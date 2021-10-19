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
///\file dielectric.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-13
///
///\brief

#ifndef HELIOS_HELIOS_BXDFS_DIELECTRIC_H
#define HELIOS_HELIOS_BXDFS_DIELECTRIC_H

#include <helios/scattering/trowbridge_reitz_distribution.h>
#include <helios/base/spectrum.h>
#include <helios/base/bxdf.h>
#include <hermes/geometry/point.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                     DielectricBxDF
// *********************************************************************************************************************
class DielectricBxDF {
public:
  // *******************************************************************************************************************
  //                                                                                                   STATIC METHODS
  // *******************************************************************************************************************
  static BxDF createBxDF(mem::Ptr data_ptr) {
    bxdf_flags flags{bxdf_flags::NONE};
    if (reinterpret_cast<DielectricBxDF *>(data_ptr.get())->eta_ == 1)
      flags = bxdf_flags::TRANSMISSION;
    flags = flags | bxdf_flags::REFLECTION;
    if (reinterpret_cast<DielectricBxDF *>(data_ptr.get())->mf_distribution_.effectivelySmooth())
      flags = flags | bxdf_flags::SPECULAR;
    else
      flags = flags | bxdf_flags::GLOSSY;
    return {
        .data_ptr = data_ptr,
        .flags = flags,
        .type = BxDFType::DIELECTRIC
    };
  }
  // *******************************************************************************************************************
  //                                                                                                 FRIEND FUNCTIONS
  // *******************************************************************************************************************
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE DielectricBxDF();
  HERMES_DEVICE_CALLABLE DielectricBxDF(real_t eta, const TrowbridgeReitzDistribution &mf_distribution);
  HERMES_DEVICE_CALLABLE ~DielectricBxDF();
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
  /// \param wo outgoing direction
  /// \param wi incident direction
  /// \param mode
  /// \return the value of the distribution function for the given pair of directions
  [[nodiscard]] HERMES_DEVICE_CALLABLE SampledSpectrum f(const hermes::vec3 &wo,
                                                         const hermes::vec3 &wi,
                                                         TransportMode mode) const;
  /// Computes the direction of incident light given an outgoing direction and computes the value of the BxDF
  /// for the final pair of directions.
  /// \param wo
  /// \param uc
  /// \param u
  /// \param mode
  /// \param sample_flags
  /// \return
  [[nodiscard]] HERMES_DEVICE_CALLABLE
  BSDFSampleReturn sample_f(const hermes::vec3 &wo,
                            real_t uc,
                            const hermes::point2 &u,
                            TransportMode mode,
                            bxdf_refl_trans_flags sample_flags = bxdf_refl_trans_flags::ALL) const;
  ///
  /// \param wo
  /// \param wi
  /// \param mode
  /// \param sampleFlags
  /// \return
  [[nodiscard]] HERMES_DEVICE_CALLABLE
  real_t PDF(const hermes::vec3 &wo, const hermes::vec3 &wi, TransportMode mode,
             bxdf_refl_trans_flags sampleFlags = bxdf_refl_trans_flags::ALL) const;
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
private:
  real_t eta_;
  TrowbridgeReitzDistribution mf_distribution_;
};

}

#endif //HELIOS_HELIOS_BXDFS_DIELECTRIC_H
