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
///\file transform.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-02
///
///\brief

#ifndef HELIOS_HELIOS_GEOMETRY_TRANSFORM_H
#define HELIOS_HELIOS_GEOMETRY_TRANSFORM_H

#include <hermes/geometry/transform.h>
#include <helios/geometry/ray.h>
#include <helios/core/interaction.h>

namespace helios {

/// Applies transform to a vector carrying error and computes its absolute error
/// \param t transformation
/// \param v vector
/// \param err accumulated floating-vec round-off error
/// \return transformed vec
HERMES_DEVICE_CALLABLE hermes::vec3 transform(const hermes::Transform &t,
                                              const hermes::vec3 &v,
                                              hermes::vec3 &err);
/// Applies transform to a vector carrying error and computes its absolute error
/// \param t transformation
/// \param v vec
/// \param v_err accumulated floating-vec round-off error
/// \param t_err accumulated floating-vec round-off error by transform
/// \return transformed vec
HERMES_DEVICE_CALLABLE hermes::vec3 transform(const hermes::Transform &t,
                                              const hermes::vec3 &v,
                                              hermes::vec3 &v_err,
                                              hermes::vec3 &t_err);
/// Applies transform to a point carrying error and computes its absolute error
/// \param t transformation
/// \param p point
/// \param err accumulated floating-point round-off error
/// \return transformed point
HERMES_DEVICE_CALLABLE hermes::point3 transform(const hermes::Transform &t,
                                                const hermes::point3 &p,
                                                hermes::vec3 &err);
/// Applies transform to a point carrying error and computes its absolute error
/// \param t transformation
/// \param p point
/// \param p_err accumulated floating-point round-off error
/// \param t_err accumulated floating-point round-off error by transform
/// \return transformed point
HERMES_DEVICE_CALLABLE hermes::point3 transform(const hermes::Transform &t,
                                                const hermes::point3 &p,
                                                hermes::vec3 &p_err,
                                                hermes::vec3 &t_err);
/// Applies transform to a ray
/// \param t transformation matrix
/// \param r ray
/// \return transformed ray
HERMES_DEVICE_CALLABLE Ray transform(const hermes::Transform &t,
                                     const Ray &r);
HERMES_DEVICE_CALLABLE RayDifferential transform(const hermes::Transform &t,
                                                 const RayDifferential &r);
/// Applies transform to a ray carrying error and computes its absolute error
/// \param t transformation
/// \param ray
/// \param o_err accumulated floating-point round-off error in ray's origin
/// \param d_err accumulated floating-point round-off error in ray's direction
/// \return transformed ray
HERMES_DEVICE_CALLABLE Ray transform(const hermes::Transform &t,
                                     const Ray &ray,
                                     hermes::vec3 &o_err,
                                     hermes::vec3 &d_err);
/// Applies transform to surface interaction members
/// \param si surface interaction
/// \return surface interactions with members transformed
HERMES_DEVICE_CALLABLE SurfaceInteraction transform(const hermes::Transform &t,
                                                    const SurfaceInteraction &si);

///
/// \param fov
/// \param n
/// \param f
/// \return
HERMES_DEVICE_CALLABLE hermes::Transform perspective(real_t fov, real_t n, real_t f);

}

#endif //HELIOS_HELIOS_GEOMETRY_TRANSFORM_H
