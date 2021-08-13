#ifndef HELIOS_CORE_FILM_H
#define HELIOS_CORE_FILM_H

#include <helios/core/filter.h>
#include <helios/core/spectrum.h>
#include <helios/geometry/bounds.h>
#include <hermes/common/index.h>
#include <hermes/storage/array.h>
#include <memory>

namespace helios {

/// Pixel structure stored in film tiles
struct FilmTilePixel {
  Spectrum contrib_sum = 0.f;       //!< sum of weighted contributions from pixel samples
  real_t filter_weight_sum = 0.f;   //!< sum of filter weights
};

/// Pixel structure stored in film image
struct FilmPixel {
  real_t xyz[3] = {0, 0, 0};                  //!< color in XYZ color space
  real_t filter_weight_sum = 0;               //!< sum of filter weights of radiance samples
  real_t splat_XYZ[3]{};                      //!< unweighted sum of samples splats TODO atomic!
  real_t pad{};                               //!< to ensure byte alignment
};

// *********************************************************************************************************************
//                                                                                                           FilmTile
// *********************************************************************************************************************
/// Represents an image piece to be rendered
/// \note The film is split up into disjoint tiles that can be processed in parallel without
/// \note producing racing conditions
class FilmTile {
public:
  typedef PreComputedFilter<16> PCF;
  /// \param pixel_bounds the bounds of the pixels_ in the final image
  /// \param filter reconstruction filter precomputed table
  HERMES_DEVICE_CALLABLE FilmTile(const bounds2i &pixel_bounds, const PreComputedFilter<16> *filter);
  HERMES_DEVICE_CALLABLE ~FilmTile();
  /// Updates the stored image using the reconstruction filter with the pixel filtering equation
  /// \param p sample position
  /// \param L sample's radiance
  /// \param sample_weight sample's filter weight
  HERMES_DEVICE_CALLABLE void addSample(const hermes::point2 &p, const Spectrum &L, real_t sample_weight = 1.);
  /// \param p pixel coordinates with respect to overall image
  /// \return FilmTilePixel& reference to pixel inside tile
  HERMES_DEVICE_CALLABLE FilmTilePixel &getPixel(const hermes::index2 &p);
  /// \param p pixel coordinates with respect to overall image
  /// \return const FilmTilePixel& const reference to pixel inside tile
  [[nodiscard]] HERMES_DEVICE_CALLABLE const FilmTilePixel &getPixel(const hermes::index2 &p) const;
  /// \return bounds2i bound of pixels_ under the tile
  [[nodiscard]] HERMES_DEVICE_CALLABLE bounds2i getPixelBounds() const;

private:
  const bounds2i pixel_bounds_;             //!< bounds of the pixels_ in the final image
  const PreComputedFilter<16> *filter_;     //!< pointer to filter's precomputed table
  FilmTilePixel *pixels_{nullptr};          //!< rendered pixels_
};

// *********************************************************************************************************************
//                                                                                                           FilmView
// *********************************************************************************************************************
/// Dictates how the incident light is actually transformed into colors in an image.
/// \note The rendered piece of film is a cropped region defined in NDC space:
/// \note             (0,0) --------------------
/// \note              |    (x0, y0)......     |
/// \note              |    ..............     |
/// \note              |    ......(x1, y1)     |
/// \note              --------------------(1,1)
/// \note Each image pixel represents a square area of the film. Discrete coordinates of pixels are located
/// \note in the center of each pixel area:
/// \note discrete coordinates:             0             1             2
/// \note                            |------x------|------x------|------x------|
/// \note film coordinates:          0             1             2             3
struct Film {
  typedef PreComputedFilter<16> PCF;
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  Film() = default;
  /// \param resolution image's size in pixels_
  /// \param crop_window specifies a subset of the image to render (NDC space)
  /// \param filter used to compute radiance contributions to each pixel
  /// \param diagonal length of the diagonal of the film's physical area (millimeters)
  Film(const hermes::size2 &resolution,
       Filter *filter,
       real_t diagonal,
       const bounds2 &crop_window = bounds2::unitBox());
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  /// \param sample_bounds tile's region
  /// \return std::unique_ptr<FilmTile> a pointer to a FilmTile object that
  /// stores contributions for the pixels_ in its region
  HERMES_DEVICE_CALLABLE FilmTile filmTile(const bounds2i &sample_bounds);
  /// Computes the range of discrete pixel values to be sampled.
  /// \return bounds2i the area to be sampled
  [[nodiscard]] HERMES_DEVICE_CALLABLE bounds2i sampleBounds() const;
  /// \return bounds2 extent of the film in the scene
  [[nodiscard]] HERMES_DEVICE_CALLABLE bounds2 physicalExtent() const;
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
  const hermes::size2 full_resolution{};          //!< image's size in pixels
  const bounds2i cropped_pixel_bounds{};          //!< piece of image to be rendered/stored
  const real_t diagonal{};                        //!< length of the diagonal of the film's physical area in meters
  const PreComputedFilter<16> filter{};           //!< used to interpolate radiance values to pixels_
};

// *********************************************************************************************************************
//                                                                                                               Film
// *********************************************************************************************************************
/// Dictates how the incident light is actually transformed into colors in an image.
/// \note Each image pixel represents a square area of the film. Discrete coordinates of pixels are located
/// \note in the center of each pixel area:
/// \note discrete coordinates:             0             1             2
/// \note                            |------x------|------x------|------x------|
/// \note film coordinates:          0             1             2             3
class FilmImageView {
public:
  friend class FilmImage;
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  /// \param p pixel position
  /// \return Pixel& pixel's reference
  HERMES_DEVICE_CALLABLE FilmPixel &getPixel(const hermes::index2 &p);
  ///
  /// \param p
  /// \return
  HERMES_DEVICE_CALLABLE real_t *rgb(const hermes::index2 &p);
  /// Note that ownership of tile is transferred to this method, so the caller
  /// should not attempt to add contributions to the tile after.
  /// \param tile tile unique reference
  HERMES_DEVICE_FUNCTION void mergeFilmTile(const FilmTile &tile);
  /// Sets the entire image
  /// \param img pixel's XYZ values
  void setImage(const Spectrum *img) const;
  /// Splats contributions to arbitrary pixels_
  /// \param p position
  /// \param v spectrum data
  void addSplat(const hermes::point2 &p, const Spectrum &v);

  HERMES_DEVICE_FUNCTION  Film &film();

  const real_t scale;
private:
  explicit FilmImageView(FilmPixel *pixels, real_t *rgb, Film film, real_t scale);

  Film film_;
  FilmPixel *pixels_{nullptr};                               //!< image's pixel structures
  real_t *rgb_{nullptr};                                     //!< image's rgb values
};

// *********************************************************************************************************************
//                                                                                                          FilmImage
// *********************************************************************************************************************
// Stores pixels to be written into image files or buffers
class FilmImage {
public:
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  /// \param film
  explicit FilmImage(const Film &film, real_t scale = 1);
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  /// Convert film pixel information into actual rgb values
  /// \return
  hermes::Array<real_t> imagePixels();
  /// \return
  FilmImageView view();
  /// \return
  const Film &film() const;
private:
  Film film_;
  hermes::DeviceArray<FilmPixel> pixels_;
  hermes::DeviceArray<real_t> rgb_;
  real_t scale_{1};

};

} // namespace helios

#endif // HELIOS_CORE_FILM_H
