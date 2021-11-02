///  Copyright (c) 2018 FilipeCN
///
///  The MIT License (MIT)
///
///  Permission is hereby granted, free of charge, to any person obtaining a copy
///  of this software and associated documentation files (the "Software"), to deal
///  in the Software without restriction, including without limitation the rights
///  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
///  copies of the Software, and to permit persons to whom the Software is
///  furnished to do so, subject to the following conditions:
///
///  The above copyright notice and this permission notice shall be included in
///  all copies or substantial portions of the Software.
///  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
///  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
///  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
///  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
///  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
///  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
///  THE SOFTWARE.

#include <helios/common/globals.h>
#include <helios/core/interaction.h>
#include <helios/geometry/utils.h>

using namespace hermes;

namespace helios {

HERMES_DEVICE_CALLABLE Interaction::Interaction() {}

HERMES_DEVICE_CALLABLE Interaction::Interaction(
    const point3 &point, const normal3 &normal, const vec3 &pointError,
    const vec3 &outgoingDirection,
    real_t t /*TODO , const MediumInterface& mediumInterface*/)
    : p(point), time(t), pError(pointError), wo(outgoingDirection),
      n(normal) /*TODO mediumInterface(mediumInterface)*/ {}

HERMES_DEVICE_CALLABLE Interaction::Interaction(const point3 &point, real_t time) : p(point), time(time) {}
HERMES_DEVICE_CALLABLE Interaction::Interaction(hermes::point3i pi,
                                                hermes::normal3 n,
                                                hermes::point2 uv,
                                                hermes::vec3 wo,
                                                real_t time)
    : pi(pi), n(n), uv(uv), wo(hermes::normalize(wo)), time(time) {}

HERMES_DEVICE_CALLABLE Ray Interaction::spawnRay(const vec3 &d) const {
  point3 o = Ray::offsetRayOrigin(p, pError, n, d);
  return Ray(o, d, Constants::real_infinity, time /*TODO , medium(d)*/);
}

HERMES_DEVICE_CALLABLE Ray Interaction::spawnRayTo(const point3 &p2) const {
  point3 origin = Ray::offsetRayOrigin(p, pError, n, p2 - p);
  vec3 d = p2 - origin;
  return Ray(origin, d, 1 - globals::shadowEpsilon(), time /*, medium(d)*/);
}

HERMES_DEVICE_CALLABLE Ray Interaction::spawnRayTo(const Interaction &p2) const {
  point3 origin = Ray::offsetRayOrigin(p, pError, n, p2.p - p);
  vec3 d = p2.p - origin;
  return Ray(origin, d, 1 - globals::shadowEpsilon(), time /*, medium(d)*/);
}

HERMES_DEVICE_CALLABLE SurfaceInteraction::SurfaceInteraction(
    const point3 &point, const vec3 &pointError, const point2f &uv,
    const vec3 &outgoingDirection, const vec3 &dpdu, const vec3 &dpdv,
    const normal3 &dndu, const normal3 &dndv, real_t t)
    : Interaction(point, normal3(normalize(cross(dpdu, dpdv))), pointError,
                  outgoingDirection, t),
      dpdu(dpdu), dpdv(dpdv), dndu(dndu), dndv(dndv) {
  // initialize shading geometry from true geometry
  shading.n = n;
  shading.dpdu = dpdu;
  shading.dpdv = dpdv;
  shading.dndu = dndu;
  shading.dndv = dndv;
  // adjust normal based on orientation and handedness
//  if (shape && (HELIOS_MASK_BIT(shape->flags, shape_flags::REVERSE_ORIENTATION) ||
//      HELIOS_MASK_BIT(shape->flags, shape_flags::TRANSFORM_SWAP_HANDEDNESS))) {
//    interaction.n *= -1;
//    shading.n *= -1;
//  }
}

HERMES_DEVICE_CALLABLE SurfaceInteraction::SurfaceInteraction(hermes::point3i pi,
                                                              hermes::point2 uv,
                                                              hermes::vec3 wo,
                                                              hermes::vec3 dpdu,
                                                              hermes::vec3 dpdv,
                                                              hermes::normal3 dndu,
                                                              hermes::normal3 dndv,
                                                              real_t time,
                                                              bool flipNormal)
    : Interaction(pi, hermes::normal3(hermes::normalize(hermes::cross(dpdu, dpdv))), uv, wo, time),
      dpdu(dpdu),
      dpdv(dpdv),
      dndu(dndu),
      dndv(dndv) {
  // Initialize shading geometry from true geometry
  shading.n = n;
  shading.dpdu = dpdu;
  shading.dpdv = dpdv;
  shading.dndu = dndu;
  shading.dndv = dndv;

  // Adjust normal based on orientation and handedness
  if (flipNormal) {
    n *= -1;
    shading.n *= -1;
  }
}

HERMES_DEVICE_CALLABLE void SurfaceInteraction::setShadingGeometry(const vec3 &dpdus,
                                                                   const vec3 &dpdvs,
                                                                   const normal3 &dndus,
                                                                   const normal3 &dndvs,
                                                                   bool orientationIsAuthoritative) {
  // compute shading normal
  shading.n = normal3(normalize(cross(dpdus, dpdvs)));
  if (orientationIsAuthoritative)
    n = faceForward(n, shading.n);
  else
    shading.n = faceForward(shading.n, n);
  shading.dpdu = dpdus;
  shading.dpdv = dpdvs;
  shading.dndu = dndus;
  shading.dndv = dndvs;
}

HERMES_DEVICE_CALLABLE void SurfaceInteraction::computeDifferentials(const RayDifferential &ray) const {
  dudx = dvdx = 0;
  dudy = dvdy = 0;
  dpdx = dpdy = vec3(0, 0, 0);
  // Compute auxiliary intersection points with plane
  real_t d = dot(n, vec3(p));
  real_t tx =
      -(dot(n, vec3(ray.rx_origin)) - d) / dot(n, ray.rx_direction);
  if (isinf(tx) || isnan(tx))
    return;
  point3 px = ray.rx_origin + tx * ray.rx_direction;
  real_t ty =
      -(dot(n, vec3(ray.ry_origin)) - d) / dot(n, ray.ry_direction);
  if (isinf(ty) || isnan(ty))
    return;
  point3 py = ray.ry_origin + ty * ray.ry_direction;
  dpdx = px - p;
  dpdy = py - p;

  // Compute $(u,v)$ offsets at auxiliary points

  // Choose two dimensions to use for ray offset computation
  int dim[2];
  if (abs(n.x) > abs(n.y) && abs(n.x) > abs(n.z)) {
    dim[0] = 1;
    dim[1] = 2;
  } else if (abs(n.y) > abs(n.z)) {
    dim[0] = 0;
    dim[1] = 2;
  } else {
    dim[0] = 0;
    dim[1] = 1;
  }

  // Initialize _A_, _Bx_, and _By_ matrices for offset computation
  real_t A[2][2] = {{dpdu[dim[0]], dpdv[dim[0]]},
                    {dpdu[dim[1]], dpdv[dim[1]]}};
  real_t Bx[2] = {px[dim[0]] - p[dim[0]], px[dim[1]] - p[dim[1]]};
  real_t By[2] = {py[dim[0]] - p[dim[0]], py[dim[1]] - p[dim[1]]};
  if (!numeric::soveLinearSystem(A, Bx, &dudx, &dvdx))
    dudx = dvdx = 0;
  if (!numeric::soveLinearSystem(A, By, &dudy, &dvdy))
    dudy = dvdy = 0;
}

} // namespace helios