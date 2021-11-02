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
#ifndef HELIOS_INTERACTION_H
#define HELIOS_INTERACTION_H

#include <helios/materials.h>
#include <helios/geometry/ray.h>
#include <hermes/geometry/normal.h>
#include <hermes/geometry/point.h>
#include <helios/core/bsdf.h>
#include <helios/spectra/sampled_wave_lengths.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                        Interaction
// *********************************************************************************************************************
/// Represents local information of interaction of light and elements of the scene.
class Interaction {
public:
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE Interaction();
  /// \param point point of interaction
  /// \param normal if in a surface, the normal at **point**
  /// \param pointError floating point associated with point computation
  /// \param outgoingDirection ray's outgoing direction (negative direction)
  /// \param t ray's parametric coordinate
  HERMES_DEVICE_CALLABLE Interaction(const hermes::point3 &point, const hermes::normal3 &normal,
                                     const hermes::vec3 &pointError,
                                     const hermes::vec3 &outgoingDirection,
                                     real_t t /*, const MediumInterface& mediumInterface*/);
  ///
  /// \param point
  /// \param time
  HERMES_DEVICE_CALLABLE Interaction(const hermes::point3 &point,
                                     real_t time /*, const MediumInterface& mediumInterface*/);
  ///
  /// \param pi
  /// \param n
  /// \param uv
  /// \param wo
  /// \param time
  HERMES_DEVICE_CALLABLE Interaction(hermes::point3i pi,
                                     hermes::normal3 n,
                                     hermes::point2 uv,
                                     hermes::vec3 wo,
                                     real_t time);
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  /// \param d spawn direction
  /// \return ray leaving the intersection point
  [[nodiscard]] HERMES_DEVICE_CALLABLE Ray spawnRay(const hermes::vec3 &d) const;
  /// \param p2 destination point
  /// \return ray leaving the intersection point
  [[nodiscard]] HERMES_DEVICE_CALLABLE Ray spawnRayTo(const hermes::point3 &p2) const;
  /// \param p2 destination point
  /// \return ray leaving the intersection point
  [[nodiscard]] HERMES_DEVICE_CALLABLE Ray spawnRayTo(const Interaction &p2) const;
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
  hermes::point3 p;                            //!< point of interaction TODO remove it!
  hermes::point3i pi;                          //!< point of interaction
  real_t time{0};                              //!< time of interaction (ray's parametric coordinate)
  hermes::vec3 pError;                         //!< error associated with p computation
  hermes::vec3 wo;                             //!< negative ray direction (outgoing direction)
  hermes::normal3 n;                           //!< surface's normal at p (if a surface exists)
  hermes::point2 uv;                           //!< parametric coordinated of the surface at p
};

// *********************************************************************************************************************
//                                                                                                 SurfaceInteraction
// *********************************************************************************************************************
/// The geometry of a ray-shape interaction at a particular point on its surface
class SurfaceInteraction : public Interaction {
public:
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  SurfaceInteraction() = default;
  /// \param point point of interaction
  /// \param pointError floating point associated with point computation
  /// \param uv parametric coordinated of the surface at **Interaction::p**
  /// \param outgoingDirection ray's outgoing direction (negative direction)
  /// \param dpdu partial derivative of u at p
  /// \param dpdv partial derivative of v at p
  /// \param dndu change in surface's normal at u direction
  /// \param dndv change in surface's normal at v direction
  /// \param t ray's parametric coordinate
  HERMES_DEVICE_CALLABLE SurfaceInteraction(const hermes::point3 &point,
                                            const hermes::vec3 &pointError, const hermes::point2f &uv,
                                            const hermes::vec3 &outgoingDirection,
                                            const hermes::vec3 &dpdu, const hermes::vec3 &dpdv,
                                            const hermes::normal3 &dndu, const hermes::normal3 &dndv,
                                            real_t t);
  /// \param pi
  /// \param uv
  /// \param wo
  /// \param dpdu
  /// \param dpdv
  /// \param dndu
  /// \param dndv
  /// \param time
  /// \param flipNormal
  HERMES_DEVICE_CALLABLE SurfaceInteraction(hermes::point3i pi,
                                            hermes::point2 uv,
                                            hermes::vec3 wo,
                                            hermes::vec3 dpdu,
                                            hermes::vec3 dpdv,
                                            hermes::normal3 dndu,
                                            hermes::normal3 dndv,
                                            real_t time,
                                            bool flipNormal);
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  /// \param dpdus partial derivative of u at p
  /// \param dpdvs partial derivative of v at p
  /// \param dndus change in surface's normal at u direction
  /// \param dndvs change in surface's normal at v direction
  /// \param orientationIsAuthoritative
  HERMES_DEVICE_CALLABLE void setShadingGeometry(const hermes::vec3 &dpdus, const hermes::vec3 &dpdvs,
                                                 const hermes::normal3 &dndus,
                                                 const hermes::normal3 &dndvs,
                                                 bool orientationIsAuthoritative);
  ///
  /// \param ray
  HERMES_DEVICE_CALLABLE void computeDifferentials(const RayDifferential &ray) const;
  ///
  /// \param ray
  /// \return
  template<typename Allocator>
  HERMES_DEVICE_CALLABLE BSDF bsdf(const RayDifferential &ray, SampledWaveLengths &lambda, Allocator allocator) {
    // compute differentials for intersection
    computeDifferentials(ray);
    // check if material exists
    if (!material)
      return {};
    // TODO: normal & bump map
    CAST_MATERIAL(material, material_ptr,
                  auto b = material_ptr->bxdf(allocator, lambda);
                      CAST_BXDF(b, ptr, /**/)
                      return BSDF(shading.n, shading.dpdu, material_ptr->bxdf(allocator, lambda));
    )
    return {};
  }
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
  Material material;
  hermes::vec3 dpdu;            //!< partial derivative of u at p
  hermes::vec3 dpdv;            //!< partial derivative of v at p
  hermes::normal3 dndu;         //!< change in surface's normal at u direction
  hermes::normal3 dndv;         //!< change in surface's normal at v direction
  int face_index{0};
  struct {
    hermes::normal3 n;
    hermes::vec3 dpdu, dpdv;
    hermes::normal3 dndu, dndv;
  } shading; //!< represents perturbations on the quantities of the interaction (ex: bump mapping)
  mutable hermes::vec3 dpdx, dpdy;
  mutable real_t dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;
};

} // namespace helios

#endif // HELIOS_INTERACTION_H
