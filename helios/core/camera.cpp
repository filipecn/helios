/*
 * Copyright (c) 2019 FilipeCN
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <helios/core/camera.h>

using namespace hermes;

namespace helios {

/*
real_t Camera::generateRayDifferential(const CameraSample &sample,
                                       RayDifferential *rd) const {
  real_t wt = generateRay(sample, rd);
  // find ray after shifting one pixel in the x direction
  CameraSample sshift = sample;
  ++(sshift.pFilm.x);
  HRay rx;
  real_t wtx = generateRay(sshift, &rx);
  rd->rxOrigin = rx.o;
  rd->rxDirection = rx.d;
  // find ray after shifting one pixel in the y direction
  CameraSample sshift_y = sample;
  ++(sshift_y.pFilm.y);
  HRay ry;
  real_t wty = generateRay(sshift_y, &ry);
  rd->ryOrigin = ry.o;
  rd->ryDirection = ry.d;
  if (wtx == 0.f || wty == 0.f)
    return 0.f;
  rd->hasDifferentials = true;
  return wt;
}

ProjectiveCamera::ProjectiveCamera(const hermes::Transform &cam2scr,
                                   const hermes::range2 &screen_window,
                                   real_t lensr,
                                   real_t focald) {
  // initialize depth of field parameters
  lens_radius = lensr;
  focal_distance = focald;
  // compute projective camera transformations
  camera2screen = cam2scr;
  // compute projective camera screen transformations
  screen2raster =
      scale(real_t(film->fullResolution.x),
            real_t(film->fullResolution.y), 1.f) *
          Transform::scale(1.f / (screen_window.upper().i - screen_window.lower().i),
                           1.f / (screen_window.lower().j - screen_window.upper().j), 1.f) *
          Transform::translate(vec3(-screen_window.lower().i, -screen_window.upper().j, 0.f));
  raster2screen = inverse(screen2raster);
  raster2camera = inverse(camera2screen) * raster2screen;
}*/

ProjectiveCamera::ProjectiveCamera(const Transform &cam2scr, const bbox2 &screen_window,
                                   const size2 &film_resolution,
                                   real_t lensr, real_t focald) :
    camera2screen(cam2scr) {
  lens_radius = lensr;
  focal_distance = focald;
  screen2raster =
      Transform::scale(film_resolution.width, film_resolution.height, 1) *
          Transform::scale(1. / (screen_window.upper.x - screen_window.lower.x),
                           1. / (screen_window.lower.y - screen_window.upper.y), 1) *
          Transform::translate(vec3(-screen_window.lower.x, -screen_window.upper.y, 0));
  raster2screen = inverse(screen2raster);
  raster2camera = inverse(camera2screen) * raster2screen;
}

hermes::MemoryDumper::RegionLayout ProjectiveCamera::memoryDumpLayout() {
  return hermes::MemoryDumper::RegionLayout().withSizeOf<ProjectiveCamera>()
      .withSubRegion(hermes::Transform::memoryDumpLayout().withColor(ConsoleColors::red))
      .withSubRegion(hermes::Transform::memoryDumpLayout().withColor(ConsoleColors::green))
      .withSubRegion(hermes::Transform::memoryDumpLayout().withColor(ConsoleColors::blue))
      .withSubRegion(hermes::Transform::memoryDumpLayout().withColor(ConsoleColors::yellow))
      .withSubRegion(hermes::MemoryDumper::RegionLayout().withSizeOf<real_t>().withTypeFrom<real_t>())
      .withSubRegion(hermes::MemoryDumper::RegionLayout().withSizeOf<real_t>().withTypeFrom<real_t>());
}

hermes::MemoryDumper::RegionLayout Camera::memoryDumpLayout() {
  return hermes::MemoryDumper::RegionLayout().withSizeOf<Camera>()
      .withSubRegion(AnimatedTransform::memoryDumpLayout().withColor(ConsoleColors::red))
      .withSubRegion(hermes::MemoryDumper::RegionLayout().withSizeOf<real_t>().withTypeFrom<real_t>())
      .withSubRegion(hermes::MemoryDumper::RegionLayout().withSizeOf<real_t>().withTypeFrom<real_t>())
      .withSubRegion(hermes::MemoryDumper::RegionLayout().withSizeOf<real_t>().withTypeFrom<real_t>());
}

} // namespace helios
