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
///\file light.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-08-10
///
///\brief

#ifndef HELIOS_CORE_LIGHT_H
#define HELIOS_CORE_LIGHT_H

#include <helios/common/bitmask_operators.h>
#include <helios/geometry/transform.h>

namespace helios {

enum class LightType {
  POINT
};

enum class LightFlags : u8 {
  NONE = 0x0,
  DELTA_POSITION = 0x1,
  DELTA_DIRECTION = 0x2,
  AREA = 0x4,
  INFINITE = 0x8
};

HELIOS_ENABLE_BITMASK_OPERATORS(LightFlags);

// *********************************************************************************************************************
//                                                                                                              Light
// *********************************************************************************************************************
/// Light interface
/// The light struct is a fixed-size struct that holds information common to all types of lights
struct Light {
  HERMES_DEVICE_CALLABLE static inline bool isDeltaLight(LightFlags flags) {
    return HELIOS_MASK_BIT(flags, LightFlags::DELTA_DIRECTION) ||
        HELIOS_MASK_BIT(flags, LightFlags::DELTA_POSITION);
  }

  hermes::Transform light2world;         //!< light to world space transform
  hermes::Transform world2light;         //!< world to light space transform
  LightType type{LightType::POINT};      //!< light type enum
  void *light_data{nullptr};             //!< reference to light type data
  int n_samples{1};                      //!< used for area light sources
  LightFlags flags{LightFlags::NONE};    //!< indicates the light source type
};

// *********************************************************************************************************************
//                                                                                                   VisibilityTester
// *********************************************************************************************************************
/// Tests if two end points are mutually visible
/// \note Both end points are represented by interaction objects
class VisibilityTester {
public:
  // *******************************************************************************************************************
  //                                                                                                 FRIEND FUNCTIONS
  // *******************************************************************************************************************
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  /// \param p0
  /// \param p1
  HERMES_DEVICE_CALLABLE VisibilityTester(const Interaction &p0, const Interaction &p1);
  HERMES_DEVICE_CALLABLE ~VisibilityTester();
  //                                                                                                       assignment
  // *******************************************************************************************************************
  //                                                                                                        OPERATORS
  // *******************************************************************************************************************
  //                                                                                                           access
  HERMES_DEVICE_CALLABLE const Interaction &operator[](u32 p) const;
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  /// Traces a shadow ray between both interaction points and check if there is nothing on the way
  /// \tparam Aggregate
  /// \param scene
  /// \return
  template<class SceneType>
  HERMES_DEVICE_CALLABLE bool unoccluded(const SceneType &scene) const {
    return !scene.intersectP(p0.spawnRayTo(p1));
  }

//  template<typename Aggregate>
//  HERMES_DEVICE_CALLABLE bool tr(const typename Scene<Aggregate>::View &scene, Sampler sampler) const {}
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
private:
  Interaction p0, p1;
};

} // helios namespace

#endif
