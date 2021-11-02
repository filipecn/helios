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
///\file core/bsdf.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-11
///
///\brief

#ifndef HELIOS_CORE_BSDF_H
#define HELIOS_CORE_BSDF_H

#include <helios/base/bxdf.h>
#include <hermes/geometry/normal.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                               BSDF
// *********************************************************************************************************************
/// Represents a Bidirectional Scattering Distribution Function
/// The BSDF is composed of a set of BRDFs and BTDFs
class BSDF {
public:
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE BSDF();
  HERMES_DEVICE_CALLABLE BSDF(hermes::normal3 ns, hermes::vec3 dpdus, BxDF bxdf);
  HERMES_DEVICE_CALLABLE ~BSDF();
  // *******************************************************************************************************************
  //                                                                                                        OPERATORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE explicit operator bool() const;

private:
  BxDF bxdf_;
};


/*
/// Represents a collection of BRDFs and BTDFs.
class BSDF {
public:
  /// \param si information about the differential geometry of points in the
  /// surface
  /// \param eta **[default = 1]** relative index of refraction over the
  /// boundary
  BSDF(const SurfaceInteraction &si, real_t eta = 1);
  /// Adds an BxDF component
  /// \param b BxDF component raw pointer
  void add(BxDF *b);
  /// Computes the number of BxDFs stored that match the set of types
  /// \param types flag
  /// \return int number of matches
  int numComponents(BxDF::Type types = BxDF::BSDF_ALL) const;
  /// \param v input vector in world space
  /// \return ponos::vec3 **v** transformed onto shading space
  ponos::vec3 worldToLocal(const ponos::vec3 &v) const;
  /// \param v input vector in local space
  /// \return ponos::vec3 **v** transformed onto world space
  ponos::vec3 localToWorld(const ponos::vec3 &v) const;
  /// Evaluates de BSDF for a given pair of directions
  /// \param woW outgoing direction in world space
  /// \param wiW incident direction in world space
  /// \param types types filter
  /// \return SpectrumOld total contribution of components
  SpectrumOld f(const ponos::vec3 &woW, const ponos::vec3 &wiW,
             BxDF::Type types) const;
  /// Computes the accumulated hemispherical-hemispherical reflectance that
  /// gives the fraction of incident light reflected by the surface when
  /// incident light is the same from all directions
  /// \param nSamples
  /// \param samples1
  /// \param samples2
  /// \param flags BxDF type filter
  /// \return SpectrumOld
  SpectrumOld rho(int nSamples, const ponos::point2 *samples1,
               const ponos::point2 *samples2,
               BxDF::Type flags = BxDF::Type::BSDF_ALL) const;
  /// Computes the accumulated hemispherical-directional reflectance that gives
  /// the total reflection in a given direction due to constant illumination
  /// over the hemisphere
  /// \param wo **[in]** outgoing direction
  /// \param nSamples number of samples
  /// \param samples hemisphere sample positions (only needed by some
  /// algorithms)
  /// \param flags BxDF type filter
  /// \return SpectrumOld the value of the function
  SpectrumOld rho(const ponos::vec3 &wo, int nSamples,
               const ponos::point2 *samples,
               BxDF::Type flags = BxDF::Type::BSDF_ALL) const;

  const real_t eta; //!< relative index of refraction over the boundary
private:
  /// Made private to force user to use MemoryArena instead of usual new/delete
  /// methods
  ~BSDF() = default;

  const ponos::normal3
      ns; //!< shading normal (given by bump mapping for example)
  const ponos::normal3 ng; //!< geometric normal
  const ponos::vec3 ss;    //!< primary tangent
  const ponos::vec3 ts;    //!< secundary tangent
  int nBxDFs = 0;          //!< number of BxDF components
  static constexpr int maxBxDFs = 8;
  BxDF *bxdfs[maxBxDFs]; //!< component array
};
*/
} // namespace helios

#endif
