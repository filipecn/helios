//
// Created by filipecn on 22/07/2021.
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
///\file sampling.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-22
///
///\brief

#include <helios/core/sampling.h>

using namespace hermes;

namespace helios::sampling {

HERMES_DEVICE_CALLABLE hermes::point2 sampleUniformDiskPolar(const hermes::point2 &u) {
  real_t r = std::sqrt(u[0]);
  real_t theta = 2 * hermes::Constants::pi * u[1];
  return {r * std::cos(theta), r * std::sin(theta)};
}

HERMES_DEVICE_CALLABLE hermes::point2 concentricSampleDisk(const point2 &u) {
  // Map uniform random numbers to $[-1,1]^2$
  point2 u_offset = 2.f * u - vec2(1, 1);

  // Handle degeneracy at the origin
  if (u_offset.x == 0 && u_offset.y == 0)
    return point2(0, 0);

  // Apply concentric mapping to point
  real_t theta, r;
  if (abs(u_offset.x) > abs(u_offset.y)) {
    r = u_offset.x;
    theta = Constants::pi_over_four * (u_offset.y / u_offset.x);
  } else {
    r = u_offset.y;
    theta = Constants::pi_over_four - Constants::pi_over_four * (u_offset.x / u_offset.y);
  }
  return r * point2(cos(theta), sin(theta));
}

}
