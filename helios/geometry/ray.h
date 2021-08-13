/*
 * Copyright (c) 2018 FilipeCN
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

#ifndef HELIOS_GEOMETRY_H_RAY_H
#define HELIOS_GEOMETRY_H_RAY_H

#include <hermes/geometry/point.h>
#include <hermes/geometry/normal.h>
#include <hermes/logging/memory_dump.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                                Ray
// *********************************************************************************************************************
class Ray {
public:
  // *******************************************************************************************************************
  //                                                                                                   STATIC METHODS
  // *******************************************************************************************************************
  /// Spawns ray origin by offsetting along the normal. Helps to avoid
  /// re-intersection of rays and surfaces.
  /// \param p  ray origin point
  /// \param pError absolute error carried by **p** \param n normal
  /// \param w spawn direction
  /// \return spawned point
  HERMES_DEVICE_CALLABLE static hermes::point3 offsetRayOrigin(const hermes::point3 &p,
                                                               const hermes::vec3 &pError,
                                                               const hermes::normal3 &n, const hermes::vec3 &w);
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE Ray();
  /// \param origin
  /// \param direction
  /// \param tMax max parametric coordinate allowed for this ray
  /// \param time current parametric coordinate (time value)
  HERMES_DEVICE_CALLABLE Ray(const hermes::point3 &origin, const hermes::vec3 &direction,
                             real_t tMax = hermes::Constants::real_infinity,
                             real_t time = 0.f /*, const Medium* medium = nullptr*/);
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  /// \param t
  /// \return o + t * d
  HERMES_DEVICE_CALLABLE hermes::point3 operator()(real_t t) const;
  // *******************************************************************************************************************
  //                                                                                                            DEBUG
  // *******************************************************************************************************************
  static hermes::MemoryDumper::RegionLayout memoryDumpLayout();

  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
  hermes::point3 o;     //!< ray's origin
  hermes::vec3 d;       //!< ray's direction
  mutable real_t max_t; //!< max parametric distance allowed
  //  const Medium* medium; //!< medium containing **o**
  real_t time; //!< current parametric distance
};

// *********************************************************************************************************************
//                                                                                                    RayDifferential
// *********************************************************************************************************************
/// Carries 2 auxiliary rays that can be used to determine projected areas over
/// objects being shaded, helping algorithms such as antialiasing.
class RayDifferential {
public:
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE RayDifferential();
  /// \param origin
  /// \param direction
  /// \param end parametric coordinate end (how far this ray can go)
  /// \param t parametric coordinate
  /// \param d depth of recursion
  HERMES_DEVICE_CALLABLE RayDifferential(const hermes::point3 &origin, const hermes::vec3 &direction,
                                         real_t tMax = hermes::Constants::real_infinity,
                                         real_t time = 0.f /*, const Medium* medium = nullptr*/);
  //                                                                                                       assignment
  HERMES_DEVICE_CALLABLE explicit RayDifferential(const Ray &ray) : ray(ray) {
    has_differentials = false;
  }
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  /// Updates the differential rays for a sample spacing <s>.
  /// \param s smaple spacing
  HERMES_DEVICE_CALLABLE void scaleDifferentials(float s);
  // *******************************************************************************************************************
  //                                                                                                            DEBUG
  // *******************************************************************************************************************
  static hermes::MemoryDumper::RegionLayout memoryDumpLayout();
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
  Ray ray;
  hermes::point3 rx_origin, ry_origin;
  hermes::vec3 rx_direction, ry_direction;
  bool has_differentials{false};
};

} // namespace helios

#endif
