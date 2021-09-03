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
///\file list.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-08-10
///
///\brief

#include <helios/accelerators/list.h>
#include <helios/shapes/sphere.h>
#include <helios/shapes/intersection.h>

namespace helios {

#define CAST_SHAPE(C) reinterpret_cast<C*>(shape.value.primitive_data)

#define INTERSECT_P_SHAPE(C) CAST_SHAPE(C)->intersectP(shape.value, ray, false)

ListAggregate::View::View(const hermes::ConstArrayView<Shape> &shapes, const bounds3 &world_bounds)
    : shapes_(shapes), world_bounds_(world_bounds) {}

HERMES_DEVICE_CALLABLE const helios::bounds3 &ListAggregate::View::worldBound() const {
  return world_bounds_;
}

HERMES_DEVICE_CALLABLE bool ListAggregate::View::intersect(const Ray &ray, SurfaceInteraction *isect) const {
#define SHAPE_CASE(E, C) case ShapeType::E:  \
if(INTERSECT_P_SHAPE(C))  \
  intersected = CAST_SHAPE(C)->intersect(&shape.value, ray, &cur_hit, &local_isect, false); \
 break;

  real_t t_min, t_max;

  if (!intersection::intersectP(world_bounds_, ray, &t_min, &t_max))
    return false;

  SurfaceInteraction local_isect;
  bool intersected = false;
  real_t min_hit = hermes::Constants::real_infinity;
  real_t cur_hit = 0;
  for (const auto &shape : shapes_) {
    switch (shape.value.type) {
    SHAPE_CASE(SPHERE, Sphere)
    case ShapeType::MESH:break;
    case ShapeType::CUSTOM:break;
    }
    if (intersected && cur_hit < min_hit) {
      *isect = local_isect;
      min_hit = cur_hit;
    }
  }
  return intersected;
#undef SHAPE_CASE
}

HERMES_DEVICE_CALLABLE bool ListAggregate::View::intersectP(const Ray &ray) const {
#define SHAPE_CASE(E, C) case ShapeType::E: intersected = INTERSECT_P_SHAPE(C); break;

  bool intersected = false;
  for (auto shape : shapes_) {
    switch (shape.value.type) {
    SHAPE_CASE(SPHERE, Sphere)
    case ShapeType::MESH:break;
    case ShapeType::CUSTOM:break;
    }
    if (intersected)
      return true;
  }
  return intersected;
#undef SHAPE_CASE
}

ListAggregate::ListAggregate(const hermes::Array<Shape> &shapes) {
  shapes_ = shapes;
  // compute world bounds
  for (const auto &shape : shapes)
    world_bounds_ = hermes::make_union(world_bounds_, shape.value.bounds);
  // transfer data to device
  d_data_.resize(sizeof(ListAggregate::View));
  auto v = view();
  d_data_.copy(&v, 0, hermes::MemoryLocation::HOST);
}

ListAggregate::~ListAggregate() = default;

ListAggregate::View ListAggregate::view() {
  return ListAggregate::View(shapes_.constView(), world_bounds_);
}

Aggregate ListAggregate::handle() {
  return {
      .aggregate = d_data_.ptr(),
      .type = AggregateType::LIST
  };
}

#undef CAST_SHAPE

}