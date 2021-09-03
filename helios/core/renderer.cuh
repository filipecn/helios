/// Copyright (c) 2021, FilipeCN.
///
/// The MIT License (MIT)
///
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to
/// deal in the Software without restriction, including without limitation the
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
///
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///
///\file sampler_integrator.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-11
///
///\brief

#ifndef HELIOS_HELIOS_INTEGRATORS_SAMPLER_INTEGRATOR_CUH
#define HELIOS_HELIOS_INTEGRATORS_SAMPLER_INTEGRATOR_CUH

#include <hermes/storage/memory_block.h>
#include <helios/core/film.h>
#include <helios/samplers/stratified_sampler.h>
#include <helios/cameras/perspective_camera.h>
#include <hermes/common/cuda_utils.h>
#include <hermes/storage/array.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                  RenderInfo Struct
// *********************************************************************************************************************
struct RenderInfo {
  // *******************************************************************************************************************
  //                                                                                                           FIELDS
  // *******************************************************************************************************************
  hermes::range2 pixel_bounds;         //!< image region to be rendered
  hermes::range2 sample_bounds;        //!< image region (in sampling space) to be rendered
  hermes::index2 sample_extent;        //!< diagonal of the sampling region
  u32 tile_size{};                     //!< tile size in pixels (tiles are square regions of nxn pixels)
  hermes::size2 n_tiles;               //!< total number of tiles dividing the image region
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  RenderInfo() = default;
  RenderInfo(hermes::range2 pixel_bounds, hermes::range2 sample_bounds, u32 tile_size = 16)
      : pixel_bounds(pixel_bounds),
        sample_bounds(sample_bounds),
        sample_extent(sample_bounds.upper() - sample_bounds.lower()),
        tile_size(tile_size) {
    // Compute number of tiles
    n_tiles = hermes::size2((sample_extent.i + tile_size - 1) / tile_size,
                            (sample_extent.j + tile_size - 1) / tile_size);
  }
};

struct SamplerIntegratorDebugData {
  u32 samples_per_pixel{};
  RayDifferential *debug_rays{nullptr};
};

// *********************************************************************************************************************
//                                                                                                    render_k kernel
// *********************************************************************************************************************
/// Renders a tile
/// \param render_info
template<class CameraType, class SamplerType, class IntegratorType>
HERMES_CUDA_KERNEL(render)(
    RenderInfo render_info,
    CameraType camera,
    Scene::View scene,
    SamplerType sampler,
    FilmImageView film_image,
    IntegratorType integrator,
    SamplerIntegratorDebugData ddata) {
  HERMES_CUDA_THREAD_INDEX2_LT(tile, render_info.n_tiles)
  // Compute tile region
  auto x0 = render_info.sample_bounds.lower().i + tile.i * render_info.tile_size;
  auto x1 = min(x0 + render_info.tile_size, render_info.sample_bounds.upper().i);
  auto y0 = render_info.sample_bounds.lower().j + tile.j * render_info.tile_size;
  auto y1 = min(y0 + render_info.tile_size, render_info.sample_bounds.upper().j);
  // Create tile
  bounds2i tile_bounds(hermes::index2(x0, y0), hermes::index2(x1, y1));
  auto film_tile = film_image.film().filmTile(tile_bounds);
  // Prepare sampler
  SamplerType tile_sampler = sampler;
  auto tile_index = tile.j * render_info.n_tiles.width + tile.i;
  tile_sampler.setIndex(tile_index);
//  printf("%d %d %d %d\n", tile_bounds.lower().i, tile_bounds.lower().j, tile_bounds.upper().i, tile_bounds.upper().j);
//  printf("%d %d %d %d\n",
//         render_info.sample_bounds.lower().i,
//         render_info.sample_bounds.lower().j,
//         render_info.sample_bounds.upper().i,
//         render_info.sample_bounds.upper().j);
#ifdef HELIOS_DEBUG_DATA
  u32 pixel_sample_index = 0;
  u32 tile_ray_count = render_info.tile_size * render_info.tile_size * ddata.samples_per_pixel;
  u32 ray_base_index = tile_index * tile_ray_count;
#endif
  // loop over pixels in tile
  for (auto ij : tile_bounds) {
    if (!render_info.pixel_bounds.contains(ij))
      continue;
    // generate samples for pixel
    tile_sampler.startPixel(ij);
    do {
      // camera sample
      CameraSample camera_sample = tile_sampler.cameraSample(ij);
      // compute camera ray
      RayDifferential ray;
      auto ray_weight = camera.generateRayDifferential(camera_sample, &ray);
      ray.scaleDifferentials(1 / sqrtf((real_t) tile_sampler.samplesPerPixel()));
      // save debug data
#ifdef HELIOS_DEBUG_DATA
      ddata.debug_rays[ray_base_index + pixel_sample_index++] = ray;
#endif
      // Evaluate radiance along camera ray
      Spectrum L(0.f);
      if (ray_weight > 0)
        L = integrator.Li(ray, scene, tile_sampler);
      // Add camera ray's contribution to image
      film_tile.addSample(camera_sample.film, L, ray_weight);
    } while (sampler.startNextSample());
  }
  // Merge image tile into _Film_
  film_image.mergeFilmTile(film_tile);
}

// *********************************************************************************************************************
//                                                                                                  SamplerRenderer
// *********************************************************************************************************************
/// Sampled Integrator interface
/// Sampled Integrators generate a stream of samples over the image Film in order to compute radiance
class SamplerRenderer {
public:
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  explicit SamplerRenderer(const hermes::range2 &pixel_bounds) : pixel_bounds_(pixel_bounds) {}
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  template<typename CameraType, class IntegratorType>
  void render(const CameraType &camera,
              FilmImage &film_image,
              const IntegratorType &integrator,
              const Scene::View &scene) {
    using namespace hermes;

    Log::info("Preparing render");

    // Compute image sample bounds
    range2 sample_bounds(film_image.film().sampleBounds());

    // Prepare render info
    RenderInfo render_info(pixel_bounds_, sample_bounds);
    Log::info("Image Resolution: {} {}", render_info.pixel_bounds.lower(), render_info.pixel_bounds.upper());
    Log::info("Sample Region: {} {}", render_info.sample_bounds.lower(), render_info.sample_bounds.upper());
    Log::info("Tile Size: {}", render_info.tile_size);
    Log::info("Total number of tiles: {}", render_info.n_tiles.total());
    Log::info("Total number of pixels: {}", render_info.pixel_bounds.area());

    // Configure Sampling
    StratifiedSampler sampler(size2(2, 2), true, 3);

    // group tiles in super tiles (large image regions) and render regions sequentially
    size2 super_tile_size(3, 3);
    size2 super_tile_pixel_size = super_tile_size * render_info.tile_size;
    auto sample_extent = sample_bounds.upper() - sample_bounds.lower();
    size2 n_super_tiles((sample_extent.i + super_tile_pixel_size.width - 1) / super_tile_pixel_size.width,
                        (sample_extent.j + super_tile_pixel_size.height - 1) / super_tile_pixel_size.height);

    Log::info("Splitting image into {} regions of {} x {} tiles",
              n_super_tiles.total(),
              super_tile_size.width,
              super_tile_size.height);

    // Prepare memory resources, device memory contains all memory used by the renderer following the layout:
    // < samples >
    size_t device_memory_size = 0;
    device_memory_size += sampler.memorySize() * super_tile_size.total();
    Log::info("Allocating device memory: {} bytes", device_memory_size);
    DeviceMemory dm(device_memory_size);

    sampler.setDataPtr(dm.ptr());

    for (auto tile_index : range2(n_super_tiles)) {
      // compute super tile film region
      range2 st_pixel_bounds(pixel_bounds_.lower() + index2(tile_index.i * super_tile_pixel_size.width,
                                                            tile_index.j * super_tile_pixel_size.height),
                             pixel_bounds_.lower() + index2(tile_index.i * super_tile_pixel_size.width,
                                                            tile_index.j * super_tile_pixel_size.height)
                                 + super_tile_pixel_size);
      range2 st_sample_bounds(sample_bounds.lower() + index2(tile_index.i * super_tile_pixel_size.width,
                                                             tile_index.j * super_tile_pixel_size.height),
                              sample_bounds.lower() + index2(tile_index.i * super_tile_pixel_size.width,
                                                             tile_index.j * super_tile_pixel_size.height)
                                  + super_tile_pixel_size);
      st_pixel_bounds = intersect(st_pixel_bounds, pixel_bounds_);
      st_sample_bounds = intersect(st_sample_bounds, sample_bounds);
      // prepare render info for super tile
      RenderInfo super_tile_render_info(st_pixel_bounds, st_sample_bounds);
      // render region
      renderFilmRegion(camera, film_image, integrator, scene, sampler, dm, super_tile_render_info);
    }
  }

#ifdef HELIOS_DEBUG_DATA
  RenderInfo debug_render_info;
  SamplePool debug_pool;
  hermes::DeviceMemory debug_samples;
  hermes::DeviceMemory debug_data;
  hermes::DeviceArray<RayDifferential> debug_rays;
#endif

private:

  template<typename CameraType, class IntegratorType, class Sampler>
  void renderFilmRegion(const CameraType &camera,
                        FilmImage &film_image,
                        const IntegratorType &integrator,
                        const Scene::View &scene,
                        const Sampler &sampler,
                        const hermes::DeviceMemory &dm,
                        const RenderInfo &render_info) {
    using namespace hermes;

    Log::info("Rendering image region ====================================");
    Log::info("Sample Region: {} {}", render_info.sample_bounds.lower(), render_info.sample_bounds.upper());

    SamplerIntegratorDebugData ddata;

#ifdef HELIOS_DEBUG_DATA
    debug_rays.resize(render_info.n_tiles.total() * render_info.tile_size * render_info.tile_size * 4);
    ddata.debug_rays = debug_rays.data();
    ddata.samples_per_pixel = 4;
#endif
    HERMES_LOG_VARIABLE(render_info.n_tiles)

    // Render tiles in parallel
    f32 elapsed_time = 0;
    HERMES_CUDA_TIME(HERMES_CUDA_LAUNCH_AND_SYNC((render_info.n_tiles),
                                                 render_k,
                                                 render_info,
                                                 camera,
                                                 scene,
                                                 sampler,
                                                 film_image.view(),
                                                 integrator,
                                                 ddata), elapsed_time);
    HERMES_LOG_VARIABLE(elapsed_time)

#ifdef HELIOS_DEBUG_DATA
    debug_render_info = render_info;
    debug_samples = dm;
    debug_pool = sampler.samplePool();
#endif
  }

  const hermes::range2 pixel_bounds_;
};

}

#endif //HELIOS_HELIOS_INTEGRATORS_SAMPLER_INTEGRATOR_CUH
