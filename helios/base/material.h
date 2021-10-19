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
///\file core/material.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-11
///
///\brief

#ifndef HELIOS_CORE_MATERIAL_H
#define HELIOS_CORE_MATERIAL_H

#include <helios/core/mem.h>

namespace helios {

enum class MaterialType {
  CoatedDiffuseMaterial,
  CoatedConductorMaterial,
  ConductorMaterial,
  DIELECTRIC,
  DiffuseMaterial,
  DiffuseTransmissionMaterial,
  HairMaterial,
  MeasuredMaterial,
  SubsurfaceMaterial,
  ThinDielectricMaterial,
  MixMaterial,
  CUSTOM
};

// *********************************************************************************************************************
//                                                                                                           Material
// *********************************************************************************************************************
/// Material Interface
struct Material {
  // *******************************************************************************************************************
  //                                                                                                        OPERATORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE explicit operator bool() const { return (bool) data_ptr; }
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
  mem::Ptr data_ptr;                       //!<
  MaterialType type{MaterialType::CUSTOM}; //!<
};

//class Material {
//public:
//  Material();
//  virtual ~Material();
//  /// Determines the reflective properties at the point and initializes the
//  /// SurfaceInteraction::bsdf and SurfaceInteraction::bssrdf members
//  /// \param si **[in/out]** surface interaction instance
//  /// \param arena used to allocate memory for BSDFs and BSSRDFs
//  /// \param mode indicates whether the surface intersection was found along a
//  /// path starting from camera or light
//  /// \param allowMultipleLobes indicates whether the material should use BxDFs
//  /// that aggregate multiple types of scattering into a single BxDF
//  virtual void computeScatteringFunctions(SurfaceInteraction *si,
//                                          ponos::MemoryArena &arena,
//                                          TransportMode mode,
//                                          bool allowMultipleLobes) const = 0;
//  /// Computes shading normals based on a displaced function represented by a
//  /// texture (bump mapping).
//  /// \param d displacement field
//  /// \param si surface interaction containing the normals
//  void bump(const std::shared_ptr<Texture<real_t>> &d, SurfaceInteraction *si);
//};

} // namespace helios

#endif
