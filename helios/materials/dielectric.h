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

#include <helios/base/bxdf.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                 DielectricMaterial
// *********************************************************************************************************************
/// Dielectric Material
class DielectricMaterial {
public:
  // *******************************************************************************************************************
  //                                                                                                   STATIC METHODS
  // *******************************************************************************************************************
  static Material create(mem::Ptr data_ptr) {
    return {
        .data_ptr = data_ptr,
        .type = MaterialType::DIELECTRIC
    };
  }
  template<class Allocator, class ...Args>
  static Material create(Allocator allocator, Args &&... params) {
    return {
        .data_ptr = allocator.template allocate<DielectricMaterial>(std::forward<Args>(params)...),
        .type = MaterialType::DIELECTRIC
    };
  }

  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  ///
  /// \param eta
  /// \param remap_roughness
  HERMES_DEVICE_CALLABLE DielectricMaterial(Spectrum eta, bool remap_roughness) : remap_roughness_(remap_roughness),
                                                                                  eta_(eta) {}
  HERMES_DEVICE_CALLABLE BxDF bxdf() { return {}; }
private:
//  Image *normalMap;
//  FloatTexture displacement;
//  FloatTexture uRoughness, vRoughness;
  bool remap_roughness_;
  Spectrum eta_;
};

}

#endif //HELIOS_HELIOS_MATERIALS_DIELECTRIC_H
