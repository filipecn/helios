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
///\file utils.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-02
///
///\brief

#include <helios/geometry/utils.h>

using namespace hermes;

namespace helios {

HERMES_DEVICE_CALLABLE bool solve_quadratic(EFloat A, EFloat B, EFloat C, EFloat *t0, EFloat *t1) {
  // Find quadratic discriminant
  f64 discrim = (f64) B * (f64) B - 4. * (f64) A * (f64) C;
  if (discrim < 0.)
    return false;
  double rootDiscrim = std::sqrt(discrim);

  EFloat floatRootDiscrim(rootDiscrim, Constants::machine_epsilon * rootDiscrim);

  // Compute quadratic _t_ values
  EFloat q;
  if ((float) B < 0)
    q = -.5 * (B - floatRootDiscrim);
  else
    q = -.5 * (B + floatRootDiscrim);
  *t0 = q / A;
  *t1 = C / q;
  if ((float) *t0 > (float) *t1) {
    auto tmp = *t0;
    *t0 = *t1;
    *t1 = tmp;
  }
  return true;
}

}