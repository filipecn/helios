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
///\file spectrum.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-15
///
///\brief

#ifndef HELIOS_CORE_SPECTRUM_H
#define HELIOS_CORE_SPECTRUM_H

#include <hermes/numeric/numeric.h>
#include <algorithm>
#include <cmath>
#include <map>
#include <helios/core/mem.h>

namespace helios {

enum class SpectrumType {
  BLACKBODY,
  CONSTANT,
  PIECEWISE_LINEAR,
  DENSELY_SAMPLED,
  RGBA_ALBEDO,
  RGB_UNBOUNDED,
  RGB_ILLUMINATION,
  CUSTOM
};

// *********************************************************************************************************************
//                                                                                                           Spectrum
// *********************************************************************************************************************
/// Spectrum base
/// Holds the radiometric quantities of light/reflection with a Spectral Power Distribution (SPD)
/// The SPD is a function that gives the amount of light for each visible wavelength (\lambda) of
/// electromagnetic radiation. Visible spectra comprises wavelengths between 400 nm and 700 nm.
/// Here, the SPD is approximated by a series of coefficients.
struct Spectrum {
  // *******************************************************************************************************************
  //                                                                                                        OPERATORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE explicit operator bool() const { return (bool) data_ptr; }
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
  mem::Ptr data_ptr;                                 //!< child data
  SpectrumType type{SpectrumType::CUSTOM};           //!< spectrum type
  static constexpr int n_samples = 4;                //!< number of wavelength samples
};







/// Enum to distinguish between the RGB value that represents surface reflectance
/// and the RGB value that represents an illuminant
//enum class SpectrumType { REFLECTANCE, ILLUMINANT };

static const int nCIESamples = 471;
extern const real_t CIE_X[nCIESamples];
extern const real_t CIE_Y[nCIESamples];
extern const real_t CIE_Z[nCIESamples];
extern const real_t CIE_lambda[nCIESamples];
static const real_t CIE_Y_integral = 106.856895;

static const int nRGB2SpectSamples = 32;
extern const real_t RGB2SpectLambda[nRGB2SpectSamples];
extern const real_t RGBRefl2SpectWhite[nRGB2SpectSamples];
extern const real_t RGBRefl2SpectCyan[nRGB2SpectSamples];
extern const real_t RGBRefl2SpectMagenta[nRGB2SpectSamples];
extern const real_t RGBRefl2SpectYellow[nRGB2SpectSamples];
extern const real_t RGBRefl2SpectRed[nRGB2SpectSamples];
extern const real_t RGBRefl2SpectGreen[nRGB2SpectSamples];
extern const real_t RGBRefl2SpectBlue[nRGB2SpectSamples];

extern const real_t RGBIllum2SpectWhite[nRGB2SpectSamples];
extern const real_t RGBIllum2SpectCyan[nRGB2SpectSamples];
extern const real_t RGBIllum2SpectMagenta[nRGB2SpectSamples];
extern const real_t RGBIllum2SpectYellow[nRGB2SpectSamples];
extern const real_t RGBIllum2SpectRed[nRGB2SpectSamples];
extern const real_t RGBIllum2SpectGreen[nRGB2SpectSamples];
extern const real_t RGBIllum2SpectBlue[nRGB2SpectSamples];

/// \param xyz **[in]** xyz values
/// \param rgb **[out]** returned rgb values
HERMES_DEVICE_CALLABLE inline void XYZToRGB(const real_t xyz[3], real_t rgb[3]) {
  rgb[0] = 3.240479f * xyz[0] - 1.537150f * xyz[1] - 0.498535f * xyz[2];
  rgb[1] = -0.969256f * xyz[0] + 1.875991f * xyz[1] + 0.041556f * xyz[2];
  rgb[2] = 0.055648f * xyz[0] - 0.204043f * xyz[1] + 1.057311f * xyz[2];
}
/// \param rgb **[in]** rgb values
/// \param xyz **[out]** returned xyz values
HERMES_DEVICE_CALLABLE inline void RGBToXYZ(const real_t rgb[3], real_t xyz[3]) {
  xyz[0] = 0.412453f * rgb[0] + 0.357580f * rgb[1] + 0.180423f * rgb[2];
  xyz[1] = 0.212671f * rgb[0] + 0.715160f * rgb[1] + 0.072169f * rgb[2];
  xyz[2] = 0.019334f * rgb[0] + 0.119193f * rgb[1] + 0.950227f * rgb[2];
}

// *********************************************************************************************************************
//                                                                                               Coefficient SpectrumOld
// *********************************************************************************************************************
/// Represents a spectra using a particular number of samples (given by **nSpectrumSamples** parameter) of
/// the SPD (_spectral power distribution_).
template<int nSpectrumSamples> class CoefficientSpectrum {
public:
  // *******************************************************************************************************************
  //                                                                                                 FRIEND FUNCTIONS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE friend inline CoefficientSpectrum operator*(const real_t &f, const CoefficientSpectrum &s) {
    return s * f;
  }
  HERMES_DEVICE_CALLABLE friend CoefficientSpectrum sqrt(const CoefficientSpectrum &s) {
    CoefficientSpectrum r;
    for (int i = 0; i < nSpectrumSamples; i++)
      r.c[i] = sqrtf(s.c[i]);
    return r;
  }
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  /// \param v **[in | optional]** constant value across all wave lengths
  HERMES_DEVICE_CALLABLE explicit CoefficientSpectrum(float v = 0.f) {
    for (int i = 0; i < nSpectrumSamples; i++)
      c[i] = v;
  }
  HERMES_DEVICE_CALLABLE virtual ~CoefficientSpectrum() {}
  // *******************************************************************************************************************
  //                                                                                                        OPERATORS
  // *******************************************************************************************************************
  //                                                                                                           access
  /// \param i
  /// \return
  HERMES_DEVICE_CALLABLE real_t &operator[](int i) { return c[i]; }
  HERMES_DEVICE_CALLABLE real_t operator[](int i) const { return c[i]; }
  //                                                                                                       arithmetic
#define ARITHMETIC_OP(OP) \
HERMES_DEVICE_CALLABLE CoefficientSpectrum &operator OP##=(const real_t &f) {                                       \
  for(int i = 0; i < nSpectrumSamples; ++i) c[i] OP##= f;                                                           \
  return *this;                                                                                                     \
}                                                                                                                   \
HERMES_DEVICE_CALLABLE CoefficientSpectrum &operator OP##=(const CoefficientSpectrum & s2) {                        \
  for(int i = 0; i < nSpectrumSamples; ++i) c[i] OP##= s2[i];                                                       \
  return *this;                                                                                                     \
}                                                                                                                   \
HERMES_DEVICE_CALLABLE CoefficientSpectrum operator OP(const real_t &f) const {                                     \
  CoefficientSpectrum<nSpectrumSamples> r;                                                                          \
  for(int i = 0; i < nSpectrumSamples; ++i) r.c[i] = c[i] OP f;                                                     \
  return r;                                                                                                         \
}                                                                                                                   \
HERMES_DEVICE_CALLABLE CoefficientSpectrum operator OP(const CoefficientSpectrum & s2) const {                      \
  CoefficientSpectrum<nSpectrumSamples> r;                                                                          \
  for(int i = 0; i < nSpectrumSamples; ++i)  r.c[i] =  c[i] OP s2[i];                                               \
  return r;                                                                                                         \
}

  ARITHMETIC_OP(+)
  ARITHMETIC_OP(-)
  ARITHMETIC_OP(*)
  ARITHMETIC_OP(/)

#undef ARITHMETIC_OP

  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  /// \note Useful to ray tracing algorithms to avoid the computation of reflected
  /// rays of zero reflectance surfaces.
  /// \return true
  /// \return false
  [[nodiscard]] HERMES_DEVICE_CALLABLE bool isBlack() const {
    for (int i = 0; i < nSpectrumSamples; i++)
      if (c[i] != 0.)
        return false;
    return true;
  }
  /// \param l **[in | optional]** low (default = 0)
  /// \param h **[in | optional]** high (default = INFINITY)
  /// \return CoefficientSpectrum the spectra with values clamped to be in **[l, h]**
  [[nodiscard]] CoefficientSpectrum clamp(real_t l = 0, real_t h = hermes::Constants::real_infinity) const {
    CoefficientSpectrum r;
    for (int i = 0; i < nSpectrumSamples; i++)
      r.c[i] = hermes::Numbers::clamp(c[i], l, h);
    return r;
  }
  [[nodiscard]] bool hasNaNs() const {
    for (int i = 0; i < nSpectrumSamples; i++)
      if (std::isnan(c[i]))
        return true;
    return false;
  }

  const int nSamples = nSpectrumSamples;

protected:
  real_t c[nSpectrumSamples]{};
};

/// Computes the average value of a list of samples.
/// \param lambda **[in]** coordinates
/// \param vals **[in]** values on coordinates
/// \param n **[in]** number of samples
/// \param lambdaStart **[in]** first coordinate
/// \param lambdaEnd **[in]** last coordinate
/// \return real_t
real_t averageSpectrumSamples(const real_t *lambda, const real_t *vals, int n,
                              real_t lambdaStart, real_t lambdaEnd);
/// Computes the SPD at a given wavelength _lambda_
/// \param lambda **[in]** coordinates
/// \param vals **[in]** values on coordinates
/// \param n **[in]** number of samples
/// \param lambdaStart **[in]** first coordinate
/// \return SPD value
real_t interpolateSpectrumSamples(const real_t *lambda, const real_t *vals,
                                  int n, real_t l);

// *********************************************************************************************************************
//                                                                                                       RGB SpectrumOld
// *********************************************************************************************************************
/// RGB spectra implementation
class RGBSpectrum : public CoefficientSpectrum<3> {
public:
  // *******************************************************************************************************************
  //                                                                                                   STATIC METHODS
  // *******************************************************************************************************************
  /// Creates a piece-wise linear function to represent the SPD from the given
  /// set of samples.
  /// \param lambda **[in]** array of lambda coordinates
  /// \param v **[in]** array of values (one for each lambda coordinate)
  /// \param n **[in]** number of coordinates
  /// \return a RGBSpectrum object
  static RGBSpectrum fromSampled(const real_t *lambda, const real_t *v, int n);
  /// \param xyz
  /// \param type
  /// \return
  HERMES_DEVICE_CALLABLE static RGBSpectrum fromXYZ(const real_t xyz[3]);
//                                                    SpectrumType type = SpectrumType::REFLECTANCE);
  /// \param rgb **[in]**
  /// \param type **[in | optional]** denotes wether the RGB values represents a
  /// surface reflectance or an illuminant. Converts from a given **rgb** values
  /// to a full SPD.
  /// \return RGBSpectrum
  HERMES_DEVICE_CALLABLE  static RGBSpectrum fromRGB(const real_t rgb[3]);
//                                                     SpectrumType type = SpectrumType::REFLECTANCE);
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE RGBSpectrum(real_t v = 0.f);
  HERMES_DEVICE_CALLABLE RGBSpectrum(const CoefficientSpectrum<3> &v);
  // *******************************************************************************************************************
  //                                                                                                        OPERATORS
  // *******************************************************************************************************************
  //                                                                                                       assignment
  HERMES_DEVICE_CALLABLE RGBSpectrum &operator=(const RGBSpectrum &other);
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  /// \param rgb **[out]**
  HERMES_DEVICE_CALLABLE void toRGB(real_t *rgb) const;
  HERMES_DEVICE_CALLABLE void toXYZ(float xyz[3]) const;
  /// return RBG spectra object
  [[nodiscard]] HERMES_DEVICE_CALLABLE const RGBSpectrum &toRGBSpectrum() const;
  /// \return real_t  The y coordinate of XYZ color (closely related to _luminance_)
  [[nodiscard]] HERMES_DEVICE_CALLABLE real_t y() const;
};

static const int sampledLambdaStart = 400;
static const int sampledLambdaEnd = 700;
static const int nSpectralSamples = 60;
/*
// *********************************************************************************************************************
//                                                                                                   Sampled SpectrumOld
// *********************************************************************************************************************
/// Represent a SPD with uniformly spaced samples over an wavelength range.
/// The wavelengths covers from 400 nm to 700 nm
class SampledSpectrum : public CoefficientSpectrum<nSpectralSamples> {
public:
  // *******************************************************************************************************************
  //                                                                                                   STATIC METHODS
  // *******************************************************************************************************************
  /// Converts from a given **rgb** values to a full SPD.
  /// \param rgb **[in]**
  /// \param type **[in]** denotes weather the RGB values represents a surface
  /// reflectance or an illuminant.
  /// \return SampledSpectrum
  static SampledSpectrum fromRGB(const float rgb[3], SpectrumType type);
  /// surface reflectance or an illuminant. Converts from a given **xyz**
  /// values to a full SPD.
  /// \param xyz **[in]**
  /// \param type **[in | optional]** denotes weather the RGB values represents a
  /// \return SampledSpectrum
  static SampledSpectrum fromXYZ(const float xyz[3],
                                 SpectrumType type = SpectrumType::REFLECTANCE);
  /// Creates a piece-wise linear function to represent the SPD from the given
  /// set of samples.
  /// \param lambda **[in]** array of lambda coordinates
  /// \param v **[in]** array of values (one for each lambda coordinate)
  /// \param n **[in]** number of coordinates
  /// \return SampledSpectrum
  static SampledSpectrum fromSamples(const real_t *lambda, const real_t *v, int n);
  /// computes XYZ matching curves
  /// \note XYZ matching curves are used to integrate SPDs in order to represent the perceived
  /// colors for humans
  static void init();
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  /// \param v **[in | optional]** constant value across all wavelengths
  SampledSpectrum(real_t v = 0.f);
  SampledSpectrum(const CoefficientSpectrum<nSpectralSamples> &v);
  SampledSpectrum(const RGBSpectrum &r, SpectrumType t);
  virtual ~SampledSpectrum() = default;
  // *******************************************************************************************************************
  //                                                                                                        OPERATORS
  // *******************************************************************************************************************
  SampledSpectrum &operator=(const SampledSpectrum &other);
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  /// \return RBG spectra object
  [[nodiscard]] RGBSpectrum toRGBSpectrum() const;
  /// \param xyz **[out]**
  void toXYZ(float xyz[3]) const;
  /// \param rgb **[out]**
  void toRGB(float rgb[3]) const;
  /// \return real_t  The y coordinate of XYZ color (closely related to _luminance_)
  [[nodiscard]] real_t y() const;

private:
  static SampledSpectrum X, Y, Z;
  static SampledSpectrum rgbRefl2SpectWhite, rgbRefl2SpectCyan;
  static SampledSpectrum rgbRefl2SpectMagenta, rgbRefl2SpectYellow;
  static SampledSpectrum rgbRefl2SpectRed, rgbRefl2SpectGreen;
  static SampledSpectrum rgbRefl2SpectBlue;
  static SampledSpectrum rgbIllum2SpectWhite, rgbIllum2SpectCyan;
  static SampledSpectrum rgbIllum2SpectMagenta, rgbIllum2SpectYellow;
  static SampledSpectrum rgbIllum2SpectRed, rgbIllum2SpectGreen;
  static SampledSpectrum rgbIllum2SpectBlue;
};*/

typedef RGBSpectrum SpectrumOld;
//typedef SampledSpectrum SpectrumOld;

SpectrumOld lerp(real_t t, const SpectrumOld &a, const SpectrumOld &b);

} // namespace helios

#endif // HELIOS_CORE_SPECTRUM_H
