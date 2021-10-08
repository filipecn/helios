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

#ifndef HELIOS_CORE_CAMERA_H
#define HELIOS_CORE_CAMERA_H

#include <helios/geometry/animated_transform.h>
#include <helios/geometry/ray.h>

namespace helios {

enum class CameraType {
  PERSPECTIVE,
  CUSTOM
};

// *********************************************************************************************************************
//                                                                                                      CameraSampler
// *********************************************************************************************************************
/// Stores the sampled values needed for generating camera rays
struct CameraSample {
  hermes::point2 film;     //!< point on the film to which the ray carries radiance
  hermes::point2 lens;     //!< point on the lens the ray passes through
  real_t time{};           //!< time at which the ray should sample the scene
};

// *********************************************************************************************************************
//                                                                                                             Camera
// *********************************************************************************************************************
/// Holds basic information for several types of cameras
struct Camera {
  Camera() = default;
  /* Generates ray for a given image sample.
   * \param sample
   * \param ray
   * the camera must normalize the direction of <ray>.
   * \return a weight of how much light is arriving at the film plane.
   */
//  virtual real_t generateRay(const CameraSample &sample, HRay *ray) const = 0;
  ///
  /// \param sample
  /// \param rd
  /// \note Also computes the corresponding rays for pixels shifted one pixel in the x
  /// and y directions on the plane.
  /// \return a weight of how much light is arriving at the film plane.
  real_t generateRayDifferential(const CameraSample &sample, RayDifferential *rd) const;

  static hermes::MemoryDumper::RegionLayout memoryDumpLayout();

  AnimatedTransform camera2world; //!< camera to world transform
  real_t shutter_open_time;
  real_t shutter_close_time;
};

// *********************************************************************************************************************
//                                                                                                   ProjectiveCamera
// *********************************************************************************************************************
/// Base for models that use a 4x4 projective transformation matrix.
struct ProjectiveCamera {
  ProjectiveCamera() = default;
  /// \param cam2scr              camera to screen space transform
  /// \param screen_window        screen space extent of the film
  /// \param film_resolution      resolution of film in pixels
  /// \param lensr                lens radius_
  /// \param focald               focal distance
  ProjectiveCamera(const hermes::Transform &cam2scr, const hermes::bbox2 &screen_window,
                   const hermes::size2 &film_resolution,
                   real_t lensr, real_t focald);

  static hermes::MemoryDumper::RegionLayout memoryDumpLayout();

  hermes::Transform camera2screen;
  hermes::Transform raster2camera;
  hermes::Transform screen2raster;
  hermes::Transform raster2screen;
  real_t focal_distance{};
  real_t lens_radius{};
};

} // namespace helios

#endif // HELIOS_CORE_CAMERA_H
