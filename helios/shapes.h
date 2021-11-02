//
// Created by filipecn on 13/09/2021.
//


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
///\file shapes.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-09-13
///
///\brief

#ifndef HELIOS_HELIOS_SHAPES_SHAPES_H
#define HELIOS_HELIOS_SHAPES_SHAPES_H

#include <helios/base/shape.h>
#include <helios/shapes/sphere.h>

namespace helios {

#define CAST_SHAPE(SHAPE, PTR, CODE)                                                                                \
{                                                                                                                   \
  switch(SHAPE.type) {                                                                                              \
    case ShapeType::SPHERE: { Sphere * PTR = SHAPE.data_ptr.get<Sphere>(); CODE break; }                         \
  }                                                                                                                 \
}

#define CAST_CONST_SHAPE(SHAPE, PTR, CODE)                                                                          \
{                                                                                                                   \
  switch(SHAPE.type) {                                                                                              \
    case ShapeType::SPHERE: { const auto * PTR = SHAPE.data_ptr.get<Sphere>(); CODE break; }               \
  }                                                                                                                 \
}

struct Shapes {
  ///
  /// \tparam T
  /// \return
  template<typename T>
  HERMES_DEVICE_CALLABLE static ShapeType enumFromType() {
    if (std::is_same_v<T, Sphere>)
      return ShapeType::SPHERE;
    return ShapeType::CUSTOM;
  }
  ///
  /// \tparam Allocator
  /// \tparam T
  /// \tparam P
  /// \param allocator
  /// \param params
  /// \return
  template<typename T, typename Allocator, typename ... P>
  static Shape create(Allocator allocator, P &&... params) {
    Shape shape;
    shape.data_ptr = allocator.template allocate<T>(std::forward<P>(params)...);
    shape.type = enumFromType<T>();
    shape.bounds = shape.data_ptr.get<T>()->objectBound();
    return shape;
  }
  ///
  /// \tparam T
  /// \param data_ptr
  /// \param position
  /// \param scale
  /// \return
  template<typename T>
  static Shape createFrom(mem::Ptr data_ptr,
                          const hermes::point3 &position = {0, 0, 0},
                          const hermes::vec3 &scale = {1, 1, 1}) {
    Shape shape;
    shape.data_ptr = data_ptr;
    shape.type = enumFromType<T>();
    shape.o2w = hermes::Transform::translate(hermes::vec3(position))
        * hermes::Transform::scale(scale.x, scale.y, scale.z);
    shape.w2o = hermes::inverse(shape.o2w);
    shape.bounds = shape.o2w(shape.data_ptr.get<T>()->objectBound());
    return shape;
  }
};

}

#endif //HELIOS_HELIOS_SHAPES_SHAPES_H
