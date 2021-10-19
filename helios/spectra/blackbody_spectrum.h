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
///\file blackbody_spectrum.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-15
///
///\brief

#ifndef HELIOS_HELIOS_SPECTRUM_BLACKBODY_SPECTRUM_H
#define HELIOS_HELIOS_SPECTRUM_BLACKBODY_SPECTRUM_H

#include <helios/base/spectrum.h>
#include <helios/spectra/sampled_wave_lengths.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                  BlackbodySpectrum
// *********************************************************************************************************************
class BlackbodySpectrum {
public:
  // *******************************************************************************************************************
  //                                                                                                   STATIC METHODS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE static Spectrum createSpectrum(mem::Ptr data_ptr) {
    return {
        .data_ptr = data_ptr,
        .type = SpectrumType::BLACKBODY
    };
  }
  // Spectrum Function Declarations
  HERMES_DEVICE_CALLABLE inline static real_t blackbody(real_t lambda, real_t T) {
    if (T <= 0)
      return 0;
    const real_t c = 299792458.f;
    const real_t h = 6.62606957e-34f;
    const real_t kb = 1.3806488e-23f;
    // Return emitted radiance for blackbody at wavelength _lambda_
    real_t l = lambda * 1e-9f;
    real_t Le = (2 * h * c * c) / (hermes::Numbers::pow<5>(l) * (hermes::Numbers::fastExp((h * c) / (l * kb * T)) - 1));
    HERMES_CHECK_EXP(!hermes::Check::is_nan(Le))
    return Le;
  }
  // *******************************************************************************************************************
  //                                                                                                 FRIEND FUNCTIONS
  // *******************************************************************************************************************
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  /// \param temperature
  HERMES_DEVICE_CALLABLE BlackbodySpectrum(real_t temperature) : T_(temperature) {
    // Compute blackbody normalization constant for given temperature
    real_t lambda_max = 2.8977721e-3f / T_;
    norm_factor_ = 1 / blackbody(lambda_max * 1e9f, T_);
  }
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  /// \param lambda
  /// \return
  HERMES_DEVICE_CALLABLE real_t operator()(real_t lambda) const {
    return blackbody(lambda, T_) * norm_factor_;
  }
  /// \param lambda
  /// \return
  [[nodiscard]] HERMES_DEVICE_CALLABLE SampledSpectrum Sample(const SampledWaveLengths &lambda) const {
    SampledSpectrum s;
    for (int i = 0; i < Spectrum::n_samples; ++i)
      s[i] = blackbody(lambda[i], T_) * norm_factor_;
    return s;
  }
  /// \return
  [[nodiscard]] HERMES_DEVICE_CALLABLE real_t maxValue() const { return 1.f; }
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
private:
  real_t T_;
  real_t norm_factor_;
};

}

#endif //HELIOS_HELIOS_SPECTRUM_BLACKBODY_SPECTRUM_H
