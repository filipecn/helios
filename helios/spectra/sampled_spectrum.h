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
///\file sampled_spectrum.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-15
///
///\brief

#ifndef HELIOS_HELIOS_SPECTRUM_SAMPLED_SPECTRUM_H
#define HELIOS_HELIOS_SPECTRUM_SAMPLED_SPECTRUM_H

#include <helios/base/spectrum.h>
#include <hermes/storage/array.h>
#include <hermes/storage/array_slice.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                    SampledSpectrum
// *********************************************************************************************************************
/// Represent a SPD with uniformly spaced samples over an wavelength range.
/// The wavelengths covers from 400 nm to 700 nm
class SampledSpectrum {
public:
  // *******************************************************************************************************************
  //                                                                                                   STATIC METHODS
  // *******************************************************************************************************************
  // *******************************************************************************************************************
  //                                                                                                 FRIEND FUNCTIONS
  // *******************************************************************************************************************
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE SampledSpectrum();
  HERMES_DEVICE_CALLABLE explicit SampledSpectrum(real_t v);
  HERMES_DEVICE_CALLABLE explicit SampledSpectrum(hermes::ArraySlice<const real_t> v);
  //                                                                                                       assignment
  // *******************************************************************************************************************
  //                                                                                                        OPERATORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE explicit operator bool() const;
  //                                                                                                       assignment
  HERMES_DEVICE_CALLABLE real_t &operator[](int i) { return c_[i]; }
  HERMES_DEVICE_CALLABLE real_t operator[](int i) const { return c_[i]; }
  //                                                                                                       arithmetic

#define ARITHMETIC_OP(OP) \
HERMES_DEVICE_CALLABLE SampledSpectrum &operator OP##=(const real_t &f) {                                           \
  for(int i = 0; i < Spectrum::n_samples; ++i) c_[i] OP##= f;                                                       \
  return *this;                                                                                                     \
}                                                                                                                   \
HERMES_DEVICE_CALLABLE SampledSpectrum &operator OP##=(const SampledSpectrum & s2) {                                \
  for(int i = 0; i < Spectrum::n_samples; ++i) c_[i] OP##= s2[i];                                                   \
  return *this;                                                                                                     \
}                                                                                                                   \
HERMES_DEVICE_CALLABLE SampledSpectrum operator OP(const real_t &f) const {                                         \
  SampledSpectrum r;                                                                                                \
  for(int i = 0; i < Spectrum::n_samples; ++i) r.c_[i] = c_[i] OP f;                                                \
  return r;                                                                                                         \
}                                                                                                                   \
HERMES_DEVICE_CALLABLE SampledSpectrum operator OP(const SampledSpectrum & s2) const {                              \
  SampledSpectrum r;                                                                                                \
  for(int i = 0; i < Spectrum::n_samples; ++i)  r.c_[i] =  c_[i] OP s2[i];                                          \
  return r;                                                                                                         \
}

  ARITHMETIC_OP(+)
  ARITHMETIC_OP(-)
  ARITHMETIC_OP(*)
  ARITHMETIC_OP(/)

#undef ARITHMETIC_OP
  //                                                                                                          boolean
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
private:
  hermes::CArray<real_t, Spectrum::n_samples> c_;
};

}

#endif //HELIOS_HELIOS_SPECTRUM_SAMPLED_SPECTRUM_H
