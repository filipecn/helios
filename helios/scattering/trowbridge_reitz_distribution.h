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
///\file trowbridge_reitz_distribution.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-13
///
///\brief

#ifndef HELIOS_HELIOS_SCATTERING_TROWBRIDGE_REITZ_DISTRIBUTION_H
#define HELIOS_HELIOS_SCATTERING_TROWBRIDGE_REITZ_DISTRIBUTION_H

#include <helios/base/microfacet_distribution.h>
#include <hermes/geometry/vector.h>
#include <hermes/geometry/point.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                        TrowbridgeReitzDistribution
// *********************************************************************************************************************
/// Microfacet distribution from Trowbridge and Reitz (1975)
/// Has the following form
///     D(w_h) =                        1
///              -----------------------------------------------------------------------------------------
///               pi * alpha_x * alpha_y * cos^4 theta_h * (1 + tan^2 theta_h *
///                                                 (cos^2 phi_h / alpha^2_x + sin^2 phi_h / alpha^2_y))^2
///
///     where alpha_* are anisotropic distribution components
class TrowbridgeReitzDistribution {
public:
  // *******************************************************************************************************************
  //                                                                                                   STATIC METHODS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE static MicrofacetDistribution createMFD(mem::Ptr data_ptr);
  ///
  /// \param roughness [0,1]
  /// \return
  HERMES_DEVICE_CALLABLE static real_t roughness2alpha(real_t roughness);
  // *******************************************************************************************************************
  //                                                                                                 FRIEND FUNCTIONS
  // *******************************************************************************************************************
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE TrowbridgeReitzDistribution();
  HERMES_DEVICE_CALLABLE TrowbridgeReitzDistribution(real_t alpha_x, real_t alpha_y);
  HERMES_DEVICE_CALLABLE ~TrowbridgeReitzDistribution();
  //                                                                                                       assignment
  // *******************************************************************************************************************
  //                                                                                                        OPERATORS
  // *******************************************************************************************************************
  //                                                                                                       assignment
  //                                                                                                       arithmetic
  //                                                                                                          boolean
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  [[nodiscard]] HERMES_DEVICE_CALLABLE bool effectivelySmooth() const;
  /// Measures invisible masked microfacet area per visible microfacet area
  /// \param w view direction
  /// \return measurement for a given view direction w
  [[nodiscard]] HERMES_DEVICE_CALLABLE real_t Lambda(const hermes::vec3 &w) const;
  /// \param wm surface normal
  /// \return the differential area of microfacets oriented with the given normal vector wm
  [[nodiscard]] HERMES_DEVICE_CALLABLE real_t D(const hermes::vec3 &wm) const;
  /// \param w
  /// \param wm
  /// \return
  [[nodiscard]] HERMES_DEVICE_CALLABLE real_t D(const hermes::vec3& w, const hermes::vec3 &wm) const;
  /// masking-shadowing function
  /// \param w view direction
  /// \return the fraction of microfacets visible from the given direction w
  [[nodiscard]] HERMES_DEVICE_CALLABLE real_t G1(const hermes::vec3 &w) const;
  /// \param wo outgoing direction
  /// \param wi incident direction
  /// \return fraction of microfacets visible from both directions wo and wi
  [[nodiscard]] HERMES_DEVICE_CALLABLE real_t G(const hermes::vec3 &wo, const hermes::vec3 &wi) const;
  ///
  /// \param w
  /// \param u
  /// \return
  [[nodiscard]] HERMES_DEVICE_CALLABLE hermes::vec3 sample_wm(const hermes::vec3 &w, const hermes::point2 &u) const;
  ///
  /// \param w
  /// \param wm
  /// \return
  [[nodiscard]] HERMES_DEVICE_CALLABLE real_t PDF(const hermes::vec3& w, const hermes::vec3& wm) const;
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
private:
  real_t alpha_x_{0};
  real_t alpha_y_{0};
};

}

#endif //HELIOS_HELIOS_SCATTERING_TROWBRIDGE_REITZ_DISTRIBUTION_H
