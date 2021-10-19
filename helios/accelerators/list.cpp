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
#include <helios/shapes/intersection.h>
#include <helios/shapes.h>

namespace helios {

ListAggregate::View::View() = default;

ListAggregate::View::View(const hermes::ConstArrayView<Primitive> &primitives, const bounds3 &world_bounds)
    : primitives_(primitives), world_bounds_(world_bounds) {
}

HERMES_DEVICE_CALLABLE const helios::bounds3 &ListAggregate::View::worldBound() const {
  return world_bounds_;
}

HERMES_DEVICE_CALLABLE hermes::Optional<ShapeIntersection> ListAggregate::View::intersect(const Ray &ray) const {
  real_t t_min, t_max;

  ShapeIntersectionReturn si;

  if (!intersection::intersectP(world_bounds_, ray, &t_min, &t_max))
    return {};

  for (const auto &primitive : primitives_) {
    ShapeIntersectionReturn local_si;
    CAST_PRIMITIVE(primitive.value, primitive_ptr,
                   local_si = primitive_ptr->intersect(ray);
    );

    if (local_si) {
      if (si && si->t_hit > local_si->t_hit) {
        si = local_si;
      } else
        si = local_si;
    }
  }
  return si;
}

HERMES_DEVICE_CALLABLE bool ListAggregate::View::intersectP(const Ray &ray) const {
  bool intersected = false;
  for (const auto &primitive : primitives_) {
    CAST_PRIMITIVE(primitive.value, primitive_ptr,
                   intersected = primitive_ptr->intersectP(ray);
    );
    if (intersected)
      return true;
  }
  return intersected;
}

ListAggregate::View &ListAggregate::View::operator=(const ListAggregate::View &other) {
  if (&other != this) {
    primitives_ = other.primitives_;
    world_bounds_ = other.world_bounds_;
  }
  return *this;
}

ListAggregate::ListAggregate() = default;

ListAggregate::~ListAggregate() = default;

HeResult ListAggregate::init(const std::vector<Primitive> &primitives,
                             hermes::ConstArrayView<Primitive> d_primitives) {
  d_primitives_ = d_primitives;
  // compute world bounds
  for (const auto &primitive : primitives) CAST_PRIMITIVE(primitive, primitive_ptr,
                                                          world_bounds_ = hermes::make_union(world_bounds_,
                                                                                             primitive_ptr->worldBounds());
  );
  hermes::Log::info("Initializing Accelerator Struct (ListAggregate)");
  hermes::Log::info("... with scene bounds: {}", world_bounds_);
  return HeResult::SUCCESS;
}

ListAggregate::View ListAggregate::view() {
  return ListAggregate::View(d_primitives_, world_bounds_);
}

}