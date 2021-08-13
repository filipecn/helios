// Created by filipecn on 2018-12-06.
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
#include <helios/common/globals.h>
#include <helios/core/interaction.h>

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

SurfaceInteraction::SurfaceInteraction(
    const point3 &point, const vec3 &pointError, const point2f &uv,
    const vec3 &outgoingDirection, const vec3 &dpdu, const vec3 &dpdv,
    const normal3 &dndu, const normal3 &dndv, real_t t, const Shape *shape)
    : interaction(Interaction(point, normal3(normalize(cross(dpdu, dpdv))), pointError,
                              outgoingDirection, t)),
      uv(uv), dpdu(dpdu), dpdv(dpdv), dndu(dndu), dndv(dndv), shape(shape) {
  // initialize shading geometry from true geometry
  shading.n = interaction.n;
  shading.dpdu = dpdu;
  shading.dpdv = dpdv;
  shading.dndu = dndu;
  shading.dndv = dndv;
  // adjust normal based on orientation and handedness
  //  if(shape && (shape->reverseOrientation ^ shape->transformSwapsHandedness))
  //  {
  //    n *= -1;
  //    shading.n *= -1;
  //  }
}

void SurfaceInteraction::setShadingGeometry(const vec3 &dpdus,
                                            const vec3 &dpdvs,
                                            const normal3 &dndus,
                                            const normal3 &dndvs,
                                            bool orientationIsAuthoritative) {
  // compute shading.n
  shading.n = normal3(normalize(cross(dpdus, dpdvs)));
  //  if(shape && (shape->reverseOrientation ^ shape->transformSwapsHandedness))
  //  {
  //    shading.n *= -1;
  //    if(orientationIsAuthoritative)
  //        n = Faceforward(n, shading.n);
  //    else shading.n = Faceforward(shading.n, n);
  //  }
  shading.dpdu = dpdus;
  shading.dpdv = dpdvs;
  shading.dndu = dndus;
  shading.dndv = dndvs;
}

/*
void SurfaceInteraction::computeScatteringFunctions(const RayDifferential &ray,
                                                    ponos::MemoryArena &arena,
                                                    bool allowMultipleLobes,
                                                    TransportMode mode) {
  computeDifferentials(ray);
  primitive->computeScatteringFunctions(this, arena, mode, allowMultipleLobes);
}
*/

} // namespace helios