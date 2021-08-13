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
///\file sampling.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-22
///
///\note Part of this code is entirely based on pbrt v3 code:
///  pbrt source code is Copyright(c) 1998-2016
///                      Matt Pharr, Greg Humphreys, and Wenzel Jakob.
///  Redistribution and use in source and binary forms, with or without
///  modification, are permitted provided that the following conditions are
///  met:
///  - Redistributions of source code must retain the above copyright
///    notice, this list of conditions and the following disclaimer.
///  - Redistributions in binary form must reproduce the above copyright
///    notice, this list of conditions and the following disclaimer in the
///    documentation and/or other materials provided with the distribution.
///  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
///  IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
///  TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
///  PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
///  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
///  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
///  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
///  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
///  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
///  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
///  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


#include <helios/cameras/perspective_camera.h>
#include <helios/geometry/transform.h>
#include <hermes/numeric/interpolation.h>
#include <helios/core/sampling.h>

using namespace hermes;

namespace helios {

PerspectiveCamera::PerspectiveCamera(const AnimatedTransform &cam2world,
                                     const bbox2 &screen_window,
                                     const size2 &film_resolution,
                                     real_t sopen,
                                     real_t sclose,
                                     real_t lensr,
                                     real_t focald,
                                     real_t fov)
    : camera_({cam2world, sopen, sclose}),
      projective_camera_({perspective(fov, 1e-2f, 1000.f),
                          screen_window, film_resolution, lensr, focald}) {
  // compute differential changes in origin for perspective camera rays
  dx_camera_ = projective_camera_.raster2camera(point3(1, 0, 0))
      - projective_camera_.raster2camera(point3(0, 0, 0));
  dy_camera_ = projective_camera_.raster2camera(point3(0, 1, 0))
      - projective_camera_.raster2camera(point3(0, 0, 0));
  // compute image plane bounds at z = 1
  point3 p_min = projective_camera_.raster2camera(point3(0, 0, 0));
  point3 p_max = projective_camera_.raster2camera(point3(film_resolution.width, film_resolution.height, 0));
  p_min /= p_min.z;
  p_max /= p_max.z;
  A = abs((p_max.x - p_min.x) * (p_max.y - p_min.y));
}

HERMES_DEVICE_CALLABLE real_t PerspectiveCamera::generateRay(const CameraSample &sample,
                                                             helios::Ray *ray) const {
  // Compute raster and camera sample positions
  point3 p_film = point3(sample.film.x, sample.film.y, 0);
  point3 p_camera = projective_camera_.raster2camera(p_film);
  *ray = helios::Ray(point3(0, 0, 0), normalize(vec3(p_camera)));
  // Modify ray for depth of field
  if (projective_camera_.lens_radius > 0) {
    // Sample point on lens
    point2 p_lens = projective_camera_.lens_radius * sampling::concentricSampleDisk(sample.lens);

    // Compute point on plane of focus
    real_t ft = projective_camera_.focal_distance / ray->d.z;
    point3 p_focus = (*ray)(ft);

    // Update ray for effect of lens
    ray->o = point3(p_lens.x, p_lens.y, 0);
    ray->d = normalize(p_focus - ray->o);
  }
  ray->time = interpolation::lerp(sample.time, camera_.shutter_open_time, camera_.shutter_close_time);
//  ray->medium = medium;
  *ray = transform(camera_.camera2world, *ray);
  return 1;
}

HERMES_DEVICE_CALLABLE real_t PerspectiveCamera::generateRayDifferential(const CameraSample &sample,
                                                                         RayDifferential *ray) const {
  // Compute raster and camera sample positions
  point3 pFilm = point3(sample.film.x, sample.film.y, 0);
  point3 pCamera = projective_camera_.raster2camera(pFilm);
  vec3 dir = normalize(vec3(pCamera.x, pCamera.y, pCamera.z));
  *ray = RayDifferential(point3(0, 0, 0), dir);
  // Modify ray for depth of field
  if (projective_camera_.lens_radius > 0) {
    // Sample point on lens
    point2 lens = projective_camera_.lens_radius * sampling::concentricSampleDisk(sample.lens);

    // Compute point on plane of focus
    real_t ft = projective_camera_.focal_distance / ray->ray.d.z;
    point3 pFocus = (*ray).ray(ft);

    // Update ray for effect of lens
    ray->ray.o = point3(lens.x, lens.y, 0);
    ray->ray.d = normalize(pFocus - ray->ray.o);
  }

  if (projective_camera_.lens_radius > 0) {
    point2 lens = projective_camera_.lens_radius * sampling::concentricSampleDisk(sample.lens);
    vec3 dx = normalize(vec3(pCamera + dx_camera_));
    real_t ft = projective_camera_.focal_distance / dx.z;
    point3 pFocus = point3(0, 0, 0) + (ft * dx);
    ray->rx_origin = point3(lens.x, lens.y, 0);
    ray->rx_direction = normalize(pFocus - ray->rx_origin);

    vec3 dy = normalize(vec3(pCamera + dy_camera_));
    ft = projective_camera_.focal_distance / dy.z;
    pFocus = point3(0, 0, 0) + (ft * dy);
    ray->ry_origin = point3(lens.x, lens.y, 0);
    ray->ry_direction = normalize(pFocus - ray->ry_origin);
  } else {
    ray->rx_origin = ray->ry_origin = ray->ray.o;
    ray->rx_direction = normalize(vec3(pCamera) + dx_camera_);
    ray->ry_direction = normalize(vec3(pCamera) + dy_camera_);
  }
  ray->ray.time = interpolation::lerp(sample.time, camera_.shutter_open_time, camera_.shutter_close_time);
  //  ray->medium = medium;
  *ray = transform(camera_.camera2world, *ray);
  ray->has_differentials = true;
  return 1;
}

hermes::MemoryDumper::RegionLayout PerspectiveCamera::memoryDumpLayout() {
  return hermes::MemoryDumper::RegionLayout().withSizeOf<PerspectiveCamera>()
      .withSubRegion(Camera::memoryDumpLayout())
      .withSubRegion(ProjectiveCamera::memoryDumpLayout())
      .withSubRegion(hermes::vec3::memoryDumpLayout())
      .withSubRegion(hermes::vec3::memoryDumpLayout())
      .withSubRegion(hermes::MemoryDumper::RegionLayout().withSizeOf<real_t>().withTypeFrom<real_t>());
}

} // namespace helios
