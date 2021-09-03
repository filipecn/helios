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
#include <helios/shapes/sphere.h>

namespace helios {

#define CAST_SHAPE(C) reinterpret_cast<C*>(shape_->primitive_data)

#define INTERSECT_P_SHAPE(C) CAST_SHAPE(C)->intersectP(*shape_, ray, false)

HERMES_DEVICE_CALLABLE bounds3 GeometricPrimitive::worldBounds() const {
  return shape_->bounds;
}

HERMES_DEVICE_CALLABLE bool GeometricPrimitive::intersect(const Ray &ray, SurfaceInteraction *si) const {
#define SHAPE_CASE(E, C) case ShapeType::E:  \
if(INTERSECT_P_SHAPE(C))  \
  intersected = CAST_SHAPE(C)->intersect(shape_, ray, &cur_hit, si, false); \
 break;

  bool intersected = false;
  real_t cur_hit = 0;
  switch (shape_->type) {
  SHAPE_CASE(SPHERE, Sphere)
  case ShapeType::MESH:break;
  case ShapeType::CUSTOM:break;
  }
  if (!intersected)
    return false;
  ray.max_t = cur_hit;
  si->primitive = this;
  // CHECK_GE(Dot(isect->n, isect->shading.n), 0.);
  // TODO handle medium
  return true;
#undef SHAPE_CASE
}

HERMES_DEVICE_CALLABLE bool GeometricPrimitive::intersectP(const Ray &ray) const {
#define SHAPE_CASE(E, C) case ShapeType::E: intersected = INTERSECT_P_SHAPE(C); break;
  bool intersected = false;
  switch (shape_->type) {
  SHAPE_CASE(SPHERE, Sphere)
  case ShapeType::MESH:break;
  case ShapeType::CUSTOM:break;
  }
  return intersected;
#undef SHAPE_CAS
}

#undef CAST_SHAPE

} // namespace helios
