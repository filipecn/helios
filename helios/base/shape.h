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
///\file core/shape.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-01
///
///\brief

#ifndef HELIOS_CORE_SHAPE_H
#define HELIOS_CORE_SHAPE_H

#include <helios/common/bitmask_operators.h>
#include <helios/common/globals.h>
#include <helios/geometry/ray.h>
#include <helios/core/interaction.h>
#include <helios/geometry/transform.h>
#include <helios/core/mem.h>
#include <helios/core/interaction.h>
#include <hermes/common/optional.h>

#include <memory>
#include <vector>

namespace helios {

// *********************************************************************************************************************
//                                                                                                  ShapeIntersection
// *********************************************************************************************************************
struct ShapeIntersection {
  SurfaceInteraction interaction;
  real_t t_hit{};
};

using ShapeIntersectionReturn = hermes::Optional<ShapeIntersection>;

struct QuadricIntersection {
  real_t t_hit;
  hermes::point3 p_obj;
  real_t phi;
};

using QuadricIntersectionReturn = hermes::Optional<QuadricIntersection>;

enum class ShapeType {
  SPHERE,
  MESH,
  CUSTOM
};

enum class shape_flags : u8 {
  NONE = 0x0,
  REVERSE_ORIENTATION = 0x1,         //!< indicates whether surface normals should be inverted from default
  TRANSFORM_SWAP_HANDEDNESS = 0x2    //!< precomputed from **o2w** transform
};

HELIOS_ENABLE_BITMASK_OPERATORS(shape_flags);

// *********************************************************************************************************************
//                                                                                                              Shape
// *********************************************************************************************************************
/// Geometric shape Interface.
/// The shape is a fixed-size struct that holds information about the location and material of a object in the scene.
/// Each shape has a type that tells helios how to render its geometry. The geometry of a shape is represented by a
/// set of primitives referenced by the shape.
/// All shapes receive a unique id.
struct Shape {
  // *******************************************************************************************************************
  //                                                                                                            SETUP
  // *******************************************************************************************************************
  Shape &withFlags(shape_flags new_flags) {
    flags = new_flags;
    return *this;
  }
  Shape &withTransform(const hermes::Transform object2world) {
    o2w = object2world;
    auto o_bounds = w2o(bounds);
    w2o = hermes::inverse(object2world);
    bounds = o2w(o_bounds);
    return *this;
  }
  // *******************************************************************************************************************
  //                                                                                                        OPERATORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE explicit operator bool() const { return (bool) data_ptr; }
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
  hermes::Transform o2w;                 //!< object space to world space transform
  hermes::Transform w2o;                 //!< world space to object space transform
  hermes::bbox3 bounds;                  //!< world space bounds
  mem::Ptr data_ptr;                     //!<
  ShapeType type{ShapeType::CUSTOM};     //!<
  shape_flags flags{shape_flags::NONE};  //!<
};

} // namespace helios

#endif
