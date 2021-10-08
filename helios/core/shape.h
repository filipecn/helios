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

#include <memory>
#include <vector>

namespace helios {

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
  //                                                                                                          METHODS
  // *******************************************************************************************************************
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


// *******************************************************************************************************************
//                                                                                                     CONSTRUCTORS
// *******************************************************************************************************************
/// \param o2w  object to world transformation
/// \param w2o  world to object transformation
/// \param ro   reverse orientation: indicates if the surface normal directions should be reversed
///             from default (default = normals pointing outside).
//Shape(const hermes::Transform *o2w, const hermes::Transform *w2o, bool ro);

/// Shape bounding box.
/// \return bounding box of the shapet (in world space)
//[[nodiscard]] bounds3 worldBound() const;
// *******************************************************************************************************************
//                                                                                                        INTERFACE
// *******************************************************************************************************************
//                                                                                                         geometry
/// Shape bounding box.
/// \return bounding box of the shape (in object space)
//[[nodiscard]] virtual bounds3 objectBound() const = 0;
/// \return object's surface area
//virtual real_t surfaceArea() const = 0;
//                                                                                                     intersection
/// \param ray                  ray to be intersected (in world space).
/// \param tHit [out]           if an intersection is found, **tHit receives the parametric distance along
///                             the ray, between (0, tMax), to the intersection point.
/// \param isect [out]          information about the geometry of the intersection point
/// \param test_alpha_texture   true if the can be cutted away using a
/// texture
/// \return **true** if an intersection was found
//virtual bool intersect(const Ray &ray, real_t *tHit, SurfaceInteraction *isect,
//                       bool test_alpha_texture = true) const = 0;
/// Predicate that determines if an intersection occurs.
/// \param ray                  ray to be intersected (in world space).
/// \param test_alpha_texture   true if the can be cut away using a
/// texture
/// \return true if an intersection exists
//virtual bool intersectP(const Ray &ray, bool test_alpha_texture = true) const;
//const uint32_t shape_id;
//static uint32_t nextShapeId;

} // namespace helios

#endif
