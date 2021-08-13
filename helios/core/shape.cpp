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

#include <helios/core/shape.h>

namespace helios {

//uint32_t Shape::nextShapeId = 1;
//
//Shape::Shape(const hermes::Transform *o2w, const hermes::Transform *w2o, bool ro)
//    : objectToWorld(o2w), worldToObject(w2o), reverse_orientation(ro),
//      transform_swaps_handedness(o2w->swapsHandedness()), shape_id(nextShapeId++) {
//}
//
//bounds3 Shape::worldBound() const { return (*objectToWorld)(objectBound()); }
//
//bool Shape::intersectP(const Ray &ray, bool test_alpha_texture) const {
//  real_t tHit = ray.max_t;
//  SurfaceInteraction isect;
//  return intersect(ray, &tHit, &isect, test_alpha_texture);
//}

} // namespace helios
