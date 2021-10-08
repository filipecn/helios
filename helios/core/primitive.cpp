/*
 * Copyright (c) 2018 FilipeCN
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <helios/core/primitive.h>
#include <helios/shapes/shapes.h>

namespace helios {

Primitive GeometricPrimitive::createPrimitive(mem::Ptr data_ptr) {
  return {
      .type = PrimitiveType::GEOMETRIC_PRIMITIVE,
      .data_ptr = data_ptr
  };
}

Primitive GeometricPrimitive::createPrimitive(const Shape* shape) {
  auto geo_prim_data = mem::allocate<GeometricPrimitive>(*shape);
  return {
      .type = PrimitiveType::GEOMETRIC_PRIMITIVE,
      .data_ptr = geo_prim_data
  };
}

GeometricPrimitive::GeometricPrimitive(const Shape &shape) : shape(shape) {}

HERMES_DEVICE_CALLABLE bounds3 GeometricPrimitive::worldBounds() const {
  return shape.bounds;
}

HERMES_DEVICE_CALLABLE bool GeometricPrimitive::intersect(const Ray &ray, SurfaceInteraction *si) const {
  bool intersected = false;
  real_t cur_hit = 0;

  CAST_CONST_SHAPE(shape, shape_ptr,
             if (shape_ptr->intersectP(&shape, ray, false))
               intersected = shape_ptr->intersect(&shape, ray, &cur_hit, si, false);
  );

  if (!intersected)
    return false;
  ray.max_t = cur_hit;
  si->primitive = this;
  // CHECK_GE(Dot(isect->n, isect->shading.n), 0.);
  // TODO handle medium
  return true;
}

HERMES_DEVICE_CALLABLE bool GeometricPrimitive::intersectP(const Ray &ray) const {
  bool intersected = false;

  CAST_SHAPE(shape, shape_ptr,
             intersected = shape_ptr->intersectP(&shape, ray, false);
  );

  return intersected;
}

} // namespace helios
