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

#ifndef HELIOS_HELIOS_MATERIALS_DIELECTRIC_H
#define HELIOS_HELIOS_MATERIALS_DIELECTRIC_H

#include <helios/scattering/bxdfs.h>
#include <helios/materials/material_eval_context.h>
#include <helios/spectra/sampled_wave_lengths.h>
#include <helios/scattering/trowbridge_reitz_distribution.h>
#include <helios/spectra.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                 DielectricMaterial
// *********************************************************************************************************************
/// Dielectric Material
class DielectricMaterial {
public:
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  ///
  /// \param eta
  /// \param remap_roughness
  HERMES_DEVICE_CALLABLE DielectricMaterial(Spectrum eta, bool remap_roughness) : remap_roughness_(remap_roughness),
                                                                                  eta_(eta) {}
  template<typename Allocator, typename TextureEvaluator>
  HERMES_DEVICE_CALLABLE BxDF bxdf(Allocator allocator, /*TextureEvaluator tex_ctx, MaterialEvalContext mat_ctx,*/
                                   SampledWaveLengths &lambda) const {
    // Compute index of refraction for dielectric material
    real_t sampledEta = 0;
    CAST_CONST_SPECTRUM(eta_, ptr, sampledEta = (*ptr)(lambda[0]);)

    if (eta_.type != SpectrumType::CONSTANT)
      lambda.terminateSecondary();
    // Handle edge case in case lambda[0] is beyond the wavelengths stored by the
    // Spectrum.
    if (sampledEta == 0)
      sampledEta = 1;

    // Create microfacet distribution for dielectric material
    // TODO real_t urough = texEval(uRoughness, ctx), vrough = texEval(vRoughness, ctx);
    real_t urough = 1, vrough = 1;
    if (remap_roughness_) {
      urough = TrowbridgeReitzDistribution::roughness2alpha(urough);
      vrough = TrowbridgeReitzDistribution::roughness2alpha(vrough);
    }
    TrowbridgeReitzDistribution distrib(urough, vrough);

    // Return BSDF for dielectric material
    return BxDFs::create<DielectricBxDF>(allocator, sampledEta, distrib);
  }

private:
//  Image *normalMap;
//  FloatTexture displacement;
//  FloatTexture uRoughness, vRoughness;
  bool remap_roughness_;
  Spectrum eta_;
};

}

#endif //HELIOS_HELIOS_MATERIALS_DIELECTRIC_H
