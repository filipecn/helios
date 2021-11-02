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

#ifndef HELIOS_SHAPES_H_SPHERE_H
#define HELIOS_SHAPES_H_SPHERE_H

#include <helios/base/shape.h>
#include <hermes/numeric/numeric.h>
#include <hermes/common/optional.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                             Sphere
// *********************************************************************************************************************
/// Implements the parametric form:
/// x = r sin theta cos phi
/// y = r sin theta sin phi
/// z = r cos theta
/// \note Angle values can be limited to tighten up the surface of the sphere
class Sphere {
public:
  // *******************************************************************************************************************
  //                                                                                                   STATIC METHODS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE static Sphere unitSphere() {
    return Sphere(1, -1, 1, hermes::Constants::two_pi);
  }
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  /// \param ro reverse orientation: indicates if the surface normal directions
  /// \param radius
  /// \param zMin lower limit of z value (up axis)
  /// \param zMax upper limit of z value (up axis)
  /// \param phiMax upper limit of phi angle
  HERMES_DEVICE_CALLABLE Sphere(real_t radius, real_t zMin, real_t zMax, real_t phiMax);
  // *******************************************************************************************************************
  //                                                                                                        INTERFACE
  // *******************************************************************************************************************
  /// \return
  [[nodiscard]] HERMES_DEVICE_CALLABLE bounds3 objectBound() const;
  /// \return
  [[nodiscard]] HERMES_DEVICE_CALLABLE real_t surfaceArea() const;
  /// \param shape
  /// \param r
  /// \param t_max
  /// \return
  [[nodiscard]] HERMES_DEVICE_CALLABLE
  QuadricIntersectionReturn intersectQuadric(const Shape *shape,
                                             const Ray &r,
                                             real_t t_max = hermes::Constants::real_infinity) const;
  /// \param shape
  /// \param isect
  /// \param wo
  /// \param time
  /// \return
  [[nodiscard]] HERMES_DEVICE_CALLABLE SurfaceInteraction interactionFromIntersection(const Shape *shape,
                                                                                      const QuadricIntersection &isect,
                                                                                      hermes::vec3 wo,
                                                                                      real_t time) const;
  /// \param r
  /// \return
  HERMES_DEVICE_CALLABLE ShapeIntersectionReturn intersect(const Shape *shape, const Ray &r,
                                                           real_t t_max = hermes::Constants::real_infinity) const;
  /// \param r
  /// \return
  [[nodiscard]] HERMES_DEVICE_CALLABLE bool intersectP(const Shape *shape, const Ray &r,
                                                       real_t t_max = hermes::Constants::real_infinity) const;
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  [[nodiscard]] HERMES_DEVICE_CALLABLE real_t radius() const;
private:
  real_t radius_;
  real_t zmin, zmax;
  real_t theta_min, theta_max, phi_max;
};

} // namespace helios

#endif
