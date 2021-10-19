#include <helios/core/film.h>
#include <helios/common/globals.h>
#include <hermes/common/cuda_utils.h>

#include <utility>

using namespace hermes;

namespace helios {

HERMES_DEVICE_CALLABLE FilmTile::FilmTile(const bounds2i &pixel_bounds, const PCF *filter)
    : pixel_bounds_(pixel_bounds), filter_(filter) {
  pixels_ = new FilmTilePixel[pixel_bounds.area()];
}

HERMES_DEVICE_CALLABLE FilmTile::~FilmTile() {
  delete[] pixels_;
}

HERMES_DEVICE_CALLABLE void FilmTile::addSample(const point2 &p, const SpectrumOld &L,
                                                real_t sample_weight) {
  // compute sample's raster bounds
  point2 p_discrete = p - vec2(0.5f);
  index2 p0 = ceil(p_discrete - filter_->radius);
  index2 p1 = floor(p_discrete + filter_->radius) + vec2(1);
  p0 = max(p0, pixel_bounds_.lower());
  p1 = min(p1, pixel_bounds_.upper());

//  printf("%d %d %d %d (%d %d %d %d)\n",
//         pixel_bounds_.lower().i,
//         pixel_bounds_.lower().j,
//         pixel_bounds_.upper().i,
//         pixel_bounds_.upper().j,
//         p0.i, p0.j, p1.i, p1.j);

  // loop over filter support and add sample to pixel arrays
  // precompute x and y filter table offsets
  int *ifx = new int[p1.i - p0.i];
  for (int x = p0.i; x < p1.i; ++x) {
    real_t fx =
        abs((x - p_discrete.x) * filter_->inv_radius.x * PCF::filter_table_width);
    ifx[x - p0.i] = min((int) floor(fx), PCF::filter_table_width - 1);
  }
  int *ify = new int[p1.j - p0.j];
  for (int y = p0.j; y < p1.j; ++y) {
    real_t fy = abs((y - p_discrete.y) * filter_->inv_radius.y * PCF::filter_table_width);
    ifx[y - p0.j] = min((int) floor(fy), PCF::filter_table_width - 1);
  }
  for (auto ij : range2(p0, p1)) {
    // evaluate filter value at (x, y) pixel
    int offset = ify[ij.j - p0.j] * PCF::filter_table_width + ifx[ij.i - p0.i];
    real_t filterWeight = filter_->table[offset];
    // update pixel values with filtered sample contribution
    FilmTilePixel &pixel = getPixel(ij);
    pixel.contrib_sum += L * sample_weight * filterWeight;
    pixel.filter_weight_sum += filterWeight;
  }
  delete[] ifx;
  delete[] ify;
}

HERMES_DEVICE_CALLABLE FilmTilePixel &FilmTile::getPixel(const index2 &p) {
  int width = pixel_bounds_.upper().i - pixel_bounds_.lower().i;
  int offset = (p.i - pixel_bounds_.lower().i) + (p.j - pixel_bounds_.lower().j) * width;
  return pixels_[offset];
}

HERMES_DEVICE_CALLABLE const FilmTilePixel &FilmTile::getPixel(const index2 &p) const {
  int width = pixel_bounds_.upper().i - pixel_bounds_.lower().i;
  int offset = (p.i - pixel_bounds_.lower().i) + (p.j - pixel_bounds_.lower().j) * width;
  return pixels_[offset];
}

HERMES_DEVICE_CALLABLE bounds2i FilmTile::getPixelBounds() const { return pixel_bounds_; }

Film::Film(const hermes::size2 &resolution, Filter *filter, real_t diagonal, const bounds2 &crop_window)
    : full_resolution(resolution),
      cropped_pixel_bounds{bounds2i(
          index2(std::ceil(full_resolution.width * crop_window.lower.x),
                 std::ceil(full_resolution.height * crop_window.lower.y)),
          index2(std::ceil(full_resolution.width * crop_window.upper.x),
                 std::ceil(full_resolution.height * crop_window.upper.y)))},
      diagonal(diagonal * .001), filter(filter) {
}

HERMES_DEVICE_CALLABLE bounds2i Film::sampleBounds() const {
  return bounds2i(floor(point2(cropped_pixel_bounds.lower()) +
                      vec2(0.5) + filter.radius),
                  ceil(point2(cropped_pixel_bounds.upper()) +
                      vec2(0.5) - filter.radius));
}

HERMES_DEVICE_CALLABLE bounds2 Film::physicalExtent() const {
  real_t aspect = (real_t) full_resolution.height / (real_t) full_resolution.width;
  real_t x = std::sqrt(diagonal * diagonal / (1 + aspect * aspect));
  real_t y = aspect * x;
  return bounds2(point2(-x / 2, -y / 2), point2(x / 2, y / 2));
}

HERMES_DEVICE_CALLABLE FilmTile Film::filmTile(const bounds2i &sample_bounds) {
  // bound image pixels_ that samples in sample_bounds contribute to
  vec2 half_pixel = vec2(0.5f);
  bounds2 floatBounds = sample_bounds;
  index2 p0 = ceil(floatBounds.lower - half_pixel - filter.radius);
  index2 p1 = floor(floatBounds.upper - half_pixel + filter.radius) + vec2(1);
  bounds2i tilePixelBounds = intersect(bounds2i(p0, p1), cropped_pixel_bounds);
  return FilmTile(tilePixelBounds, &filter);
}

FilmImageView::FilmImageView(FilmPixel *pixels, f32 *rgb, Film film, real_t scale) : film_{std::move(film)},
                                                                                     pixels_{pixels},
                                                                                     rgb_{rgb}, scale{scale} {
}

HERMES_DEVICE_FUNCTION void FilmImageView::mergeFilmTile(const FilmTile &tile) {
  for (index2 pixel : tile.getPixelBounds()) {
    const FilmTilePixel &tilePixel = tile.getPixel(pixel);
    FilmPixel &mergePixel = getPixel(pixel);
    real_t xyz[3];
    tilePixel.contrib_sum.toXYZ(xyz);
    for (int i = 0; i < 3; ++i)
      atomicAdd(&mergePixel.xyz[i], xyz[i]);
    atomicAdd(&mergePixel.filter_weight_sum, tilePixel.filter_weight_sum);
  }
}

HERMES_DEVICE_CALLABLE FilmPixel &FilmImageView::getPixel(const index2 &p) {
  int width = film_.cropped_pixel_bounds.upper().i - film_.cropped_pixel_bounds.lower().i;
  int offset = (p.i - film_.cropped_pixel_bounds.lower().i) + (p.j - film_.cropped_pixel_bounds.lower().j) * width;
  return pixels_[offset];
}

HERMES_DEVICE_CALLABLE f32 *FilmImageView::rgb(const index2 &p) {
  int width = film_.cropped_pixel_bounds.upper().i - film_.cropped_pixel_bounds.lower().i;
  int offset = (p.i - film_.cropped_pixel_bounds.lower().i) + (p.j - film_.cropped_pixel_bounds.lower().j) * width;
  return &rgb_[offset * 3];
}

HERMES_DEVICE_FUNCTION Film &FilmImageView::film() {
  return film_;
}

FilmImage::FilmImage(const Film &film, real_t scale) : film_{film}, scale_{scale} {
  // allocate film image storage
  pixels_.resize(film.cropped_pixel_bounds.area());
  rgb_.resize(3 * film.cropped_pixel_bounds.area());
}

FilmImageView FilmImage::view() {
  return FilmImageView(pixels_.data(), rgb_.data(), film_, scale_);
}

const Film &FilmImage::film() const {
  return film_;
}

HERMES_CUDA_KERNEL(film2rgb)(FilmImageView film_image, real_t splat_scale = 1) {
  bounds2i bounds = film_image.film().cropped_pixel_bounds;
  HERMES_CUDA_THREAD_INDEX_IJ_LT(bounds.upper() - bounds.lower())
  auto p = ij + bounds.lower();
  // Pointer to RGB value
  real_t *rgb = film_image.rgb(p);
  // Convert pixel XYZ color to RGB
  FilmPixel &pixel = film_image.getPixel(p);
  XYZToRGB(pixel.xyz, rgb);
  // Normalize pixel with weight sum
  real_t filterWeightSum = pixel.filter_weight_sum;
  if (filterWeightSum != 0) {
    real_t invWt = (real_t) 1 / filterWeightSum;
    rgb[0] = max((real_t) 0, rgb[0] * invWt);
    rgb[1] = max((real_t) 0, rgb[1] * invWt);
    rgb[2] = max((real_t) 0, rgb[2] * invWt);
  }
  // Add splat value at pixel
  real_t splatRGB[3];
  real_t splatXYZ[3] = {pixel.splat_XYZ[0], pixel.splat_XYZ[1], pixel.splat_XYZ[2]};
  XYZToRGB(splatXYZ, splatRGB);

  rgb[0] += splat_scale * splatRGB[0];
  rgb[1] += splat_scale * splatRGB[1];
  rgb[2] += splat_scale * splatRGB[2];

  // Scale pixel value by _scale_
  rgb[0] *= film_image.scale;
  rgb[1] *= film_image.scale;
  rgb[2] *= film_image.scale;
}

Array<real_t> FilmImage::imagePixels() {
  f32 elapsed_time = 0;
  HERMES_CUDA_TIME(
      HERMES_CUDA_LAUNCH_AND_SYNC((pixels_.size()),
                                  film2rgb_k,
                                  view()),
      elapsed_time)
  HERMES_LOG_VARIABLE(elapsed_time)
  return rgb_;
}

//void FilmImageView::setImage(const SpectrumOld *img) const {
//  int nPixels = film_.cropped_pixel_bounds.area();
//  for (int i = 0; i < nPixels; ++i) {
//    Pixel &p = pixels_[i];
//    img[i].toXYZ(p.xyz);
//    p.filter_weight_sum = 1;
//    p.splat_XYZ[0] = p.splat_XYZ[1] = p.splat_XYZ[2] = 0;
//  }
//}

//void FilmImageView::addSplat(const point2 &p, const SpectrumOld &v) {
//  if (!film_.cropped_pixel_bounds.contains(p))
//    return;
//  real_t xyz[3];
//  v.toXYZ(xyz);
//  Pixel &pixel = getPixel(p);
//  for (int i = 0; i < 3; ++i)
//    pixel.splat_XYZ[i] += xyz[i]; // TODO ATOMIC!
//}

} // namespace helios