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
///\file intersection.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-08-17
///
///\brief

#include <helios/shapes/intersection.h>

namespace helios::intersection {

HERMES_DEVICE_CALLABLE bool intersectP(const bounds3 &bounds,
                                       const Ray &ray,
                                       real_t *hitt_0,
                                       real_t *hitt_1) {
  real_t t0 = 0, t1 = ray.max_t;
  for(int i = 0; i < 3; ++i) {
    real_t inv_ray_dir = 1 / ray.d[i];
    real_t t_near = (bounds.lower[i] - ray.o[i]) * inv_ray_dir;
    real_t t_far = (bounds.upper[i] - ray.o[i]) * inv_ray_dir;
    if(t_near > t_far) {
      auto tt = t_near;
      t_near = t_far;
      t_far = tt;
    }
    t_far *= 1 + 2 * hermes::Numbers::gamma(3);
    t0 = t_near > t0 ? t_near : t0;
    t1 = t_far < t1 ? t_far : t1;
    if(t0 > t1)
      return false;
  }
  if(hitt_0) *hitt_0 = t0;
  if(hitt_1) *hitt_1 = t1;
  return true;
}

}
