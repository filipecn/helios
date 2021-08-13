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
///\file transform.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-02
///
///\brief

#include <helios/geometry/transform.h>

using namespace hermes;

namespace helios {

HERMES_DEVICE_CALLABLE vec3 transform(const Transform &t, const vec3 &v, vec3 &err) {
  real_t x = v.x, y = v.y, z = v.z;
  real_t xv = t[0][0] * x + t[0][1] * y + t[0][2] * z;
  real_t yv = t[1][0] * x + t[1][1] * y + t[1][2] * z;
  real_t zv = t[2][0] * x + t[2][1] * y + t[2][2] * z;
  real_t xAbsSum = (std::abs(t[0][0] * x) + std::abs(t[0][1] * y) +
      std::abs(t[0][2] * z));
  real_t yAbsSum = (std::abs(t[1][0] * x) + std::abs(t[1][1] * y) +
      std::abs(t[1][2] * z));
  real_t zAbsSum = (std::abs(t[2][0] * x) + std::abs(t[2][1] * y) +
      std::abs(t[2][2] * z));
  err = Numbers::gamma(3) * vec3(xAbsSum, yAbsSum, zAbsSum);
  return {xv, yv, zv};
}

HERMES_DEVICE_CALLABLE vec3 transform(const Transform &t, const vec3 &v, vec3 &v_err, vec3 &t_err) {
  real_t x = v.x, y = v.y, z = v.z;
  real_t xv = t[0][0] * x + t[0][1] * y + t[0][2] * z;
  real_t yv = t[1][0] * x + t[1][1] * y + t[1][2] * z;
  real_t zv = t[2][0] * x + t[2][1] * y + t[2][2] * z;
  real_t xAbsSum = (std::abs(t[0][0] * x) + std::abs(t[0][1] * y) +
      std::abs(t[0][2] * z));
  real_t yAbsSum = (std::abs(t[1][0] * x) + std::abs(t[1][1] * y) +
      std::abs(t[1][2] * z));
  real_t zAbsSum = (std::abs(t[2][0] * x) + std::abs(t[2][1] * y) +
      std::abs(t[2][2] * z));
  real_t dXAbsSum = std::abs(t[0][0]) * v_err.x +
      std::abs(t[0][1]) * v_err.y +
      std::abs(t[0][2]) * v_err.z;
  real_t dYAbsSum = std::abs(t[1][0]) * v_err.x +
      std::abs(t[1][1]) * v_err.y +
      std::abs(t[1][2]) * v_err.z;
  real_t dZAbsSum = std::abs(t[2][0]) * v_err.x +
      std::abs(t[2][1]) * v_err.y +
      std::abs(t[2][2]) * v_err.z;
  t_err = (Numbers::gamma(3) + 1) * vec3(dXAbsSum, dYAbsSum, dZAbsSum) +
      Numbers::gamma(3) * vec3(xAbsSum, yAbsSum, zAbsSum);
  return {xv, yv, zv};
}

HERMES_DEVICE_CALLABLE point3 transform(const Transform &t, const point3 &p, vec3 &err) {
  real_t x = p.x, y = p.y, z = p.z;
  // compute transformed coordinates from point
  real_t xp = t[0][0] * x + t[0][1] * y + t[0][2] * z +
      t[0][3];
  real_t yp = t[1][0] * x + t[1][1] * y + t[1][2] * z +
      t[1][3];
  real_t zp = t[2][0] * x + t[2][1] * y + t[2][2] * z +
      t[2][3];
  real_t wp = t[3][0] * x + t[3][1] * y + t[3][2] * z +
      t[3][3];
  // compute absolute error for transformed point
  real_t xAbsSum = (std::abs(t[0][0] * x) + std::abs(t[0][1] * y) +
      std::abs(t[0][2] * z) + std::abs(t[0][3]));
  real_t yAbsSum = (std::abs(t[1][0] * x) + std::abs(t[1][1] * y) +
      std::abs(t[1][2] * z) + std::abs(t[1][3]));
  real_t zAbsSum = (std::abs(t[2][0] * x) + std::abs(t[2][1] * y) +
      std::abs(t[2][2] * z) + std::abs(t[2][3]));
  err = Numbers::gamma(3) * vec3(xAbsSum, yAbsSum, zAbsSum);
  if (wp == 1)
    return {xp, yp, zp};
  return point3(xp, yp, zp) / wp;
}

HERMES_DEVICE_CALLABLE point3 transform(const Transform &t, const point3 &p, vec3 &p_err, vec3 &t_err) {
  real_t x = p.x, y = p.y, z = p.z;
  // compute transformed coordinates from point
  real_t xp = t[0][0] * x + t[0][1] * y + t[0][2] * z +
      t[0][3];
  real_t yp = t[1][0] * x + t[1][1] * y + t[1][2] * z +
      t[1][3];
  real_t zp = t[2][0] * x + t[2][1] * y + t[2][2] * z +
      t[2][3];
  real_t wp = t[3][0] * x + t[3][1] * y + t[3][2] * z +
      t[3][3];
  // compute absolute error for transformed point
  real_t xAbsSum = (std::abs(t[0][0] * x) + std::abs(t[0][1] * y) +
      std::abs(t[0][2] * z) + std::abs(t[0][3]));
  real_t yAbsSum = (std::abs(t[1][0] * x) + std::abs(t[1][1] * y) +
      std::abs(t[1][2] * z) + std::abs(t[1][3]));
  real_t zAbsSum = (std::abs(t[2][0] * x) + std::abs(t[2][1] * y) +
      std::abs(t[2][2] * z) + std::abs(t[2][3]));
  real_t dXAbsSum = std::abs(t[0][0]) * p_err.x +
      std::abs(t[0][1]) * p_err.y +
      std::abs(t[0][2]) * p_err.z;
  real_t dYAbsSum = std::abs(t[1][0]) * p_err.x +
      std::abs(t[1][1]) * p_err.y +
      std::abs(t[1][2]) * p_err.z;
  real_t dZAbsSum = std::abs(t[2][0]) * p_err.x +
      std::abs(t[2][1]) * p_err.y +
      std::abs(t[2][2]) * p_err.z;
  t_err = (Numbers::gamma(3) + 1) * vec3(dXAbsSum, dYAbsSum, dZAbsSum) +
      Numbers::gamma(3) * vec3(xAbsSum, yAbsSum, zAbsSum);
  if (wp == 1)
    return {xp, yp, zp};
  return point3(xp, yp, zp) / wp;
}

HERMES_DEVICE_CALLABLE Ray transform(const Transform &t, const Ray &r) {
  vec3 oError;
  point3 o = transform(t, r.o, oError);
  vec3 d = t(r.d);
  // offset ray origin to edge of error bounds and compute max_t
  real_t lenghtSquared = d.length2();
  real_t max_t = r.max_t;
  if (lenghtSquared > 0) {
    real_t dt = dot(abs(d), oError) / lenghtSquared;
    o += d * dt;
    max_t -= dt;
  }
  return Ray(o, d, max_t, r.time /*TODO , r.medium*/);
}

HERMES_DEVICE_CALLABLE RayDifferential transform(const Transform &t, const RayDifferential &r) {
  Ray tr = transform(t, r.ray);
  RayDifferential ret(tr.o, tr.d, tr.max_t, tr.time);//, tr.medium);
  ret.has_differentials = r.has_differentials;
  ret.rx_origin = t(r.rx_origin);
  ret.ry_origin = t(r.ry_origin);
  ret.rx_direction = t(r.rx_direction);
  ret.ry_direction = t(r.ry_direction);
  return ret;
}

HERMES_DEVICE_CALLABLE Ray transform(const Transform &t, const Ray &ray, vec3 &o_err, vec3 &d_err) {
  return Ray(transform(t, ray.o, o_err), transform(t, ray.d, d_err), ray.max_t, ray.time);
}

HERMES_DEVICE_CALLABLE SurfaceInteraction transform(const Transform &t, const SurfaceInteraction &si) {
  SurfaceInteraction s;
  // transform p and pError
//  s.p = transform(t, si.p, si.pError, s.pError);
  // TODO: transform remaining members
  return s;
}

HERMES_DEVICE_CALLABLE Transform perspective(real_t fov, real_t n, real_t f) {
  // Perform projective divide for perspective projection
  mat4 persp(1, 0, 0, 0,
             0, 1, 0, 0,
             0, 0, f / (f - n), -f * n / (f - n),
             0, 0, 1, 0);

  // Scale canonical perspective view to specified field of view
  real_t inv_tan_ang = 1 / tan(Trigonometry::degrees2radians(fov) / 2);
  return Transform::scale(inv_tan_ang, inv_tan_ang, 1) * Transform(persp);
}

}
