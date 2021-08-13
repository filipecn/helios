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
///\file point.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-08-12
///
///\brief

#ifndef HELIOS_LIGHTS_POINT_H
#define HELIOS_LIGHTS_POINT_H

#include <helios/core/spectrum.h>
#include <helios/core/light.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                         PointLight
// *********************************************************************************************************************
/// Represents an isotropic point light source that emits the same amount of light in all directions
class PointLight {
public:
  // *******************************************************************************************************************
  //                                                                                                   STATIC METHODS
  // *******************************************************************************************************************
  static Light createLight(const hermes::Transform &l2w);
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  PointLight();
  ///
  /// \param light
  /// \param I
  PointLight(const Light &light, /*const MediumInterface,*/ const Spectrum &I);
  ~PointLight();
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  [[nodiscard]] HERMES_DEVICE_CALLABLE Spectrum Le(const RayDifferential &ray) const { return Spectrum(0.f); }
  ///
  /// \param ref
  /// \param u
  /// \param wi
  /// \param pdf
  /// \param vis
  /// \return
  HERMES_DEVICE_CALLABLE Spectrum sampleLi(const Interaction &ref,
                                           const hermes::point2 &u,
                                           hermes::vec3 *wi,
                                           real_t *pdf,
                                           VisibilityTester *vis) const;
  /// \return total power emitted by this light
  [[nodiscard]] HERMES_DEVICE_CALLABLE Spectrum power() const;
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
private:
  hermes::point3 p_light_;
  Spectrum I_;
};

}

#endif //HELIOS_HELIOS_LIGHTS_POINT_H
