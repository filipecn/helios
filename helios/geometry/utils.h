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
///\file utils.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-02
///
///\brief

#ifndef HELIOS_HELIOS_GEOMETRY_UTILS_H
#define HELIOS_HELIOS_GEOMETRY_UTILS_H

#include <hermes/numeric/e_float.h>
#include <hermes/geometry/normal.h>

namespace helios {

bool HERMES_DEVICE_CALLABLE solve_quadratic(hermes::EFloat A, hermes::EFloat B, hermes::EFloat C,
                                            hermes::EFloat *t0,
                                            hermes::EFloat *t1);

template<typename T>
HERMES_DEVICE_CALLABLE inline hermes::Normal3<T> faceForward(const hermes::Normal3<T> &n,
                                                             const hermes::Normal3<T> &v) {
  return (hermes::dot((hermes::Vector3<T>) n, (hermes::Vector3<T>) v) < 0.f) ? -n : n;
}

}

#endif //HELIOS_HELIOS_GEOMETRY_UTILS_H
