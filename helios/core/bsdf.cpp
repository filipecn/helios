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
///\file core/bsdf.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-11
///
///\brief

#include <helios/core/bsdf.h>

namespace helios {

/*
BSDF::BSDF(const SurfaceInteraction &si, real_t eta)
    : eta(eta), ns(si.shading.n), ng(si.n),
      ss(ponos::normalize(si.shading.dpdu)), ts(ponos::cross(ns, ss)) {}

void BSDF::add(BxDF *b) { bxdfs[nBxDFs++] = b; }

int BSDF::numComponents(BxDF::Type types) const {}

ponos::vec3 BSDF::worldToLocal(const ponos::vec3 &v) const {
  return ponos::vec3(ponos::dot(v, ss), ponos::dot(v, ts), ponos::dot(v, ns));
}

ponos::vec3 BSDF::localToWorld(const ponos::vec3 &v) const {
  return ponos::vec3(ss.x * v.x + ts.x * v.y + ns.x * v.z,
                     ss.y * v.x + ts.y * v.y + ns.y * v.z,
                     ss.z * v.x + ts.z * v.y + ns.z * v.z);
}

SpectrumOld BSDF::f(const ponos::vec3 &woW, const ponos::vec3 &wiW,
                 BxDF::Type types) const {
  ponos::vec3 wi = worldToLocal(wiW), wo = worldToLocal(woW);
  bool reflect = ponos::dot(wiW, ng) * ponos::dot(woW, ng) > 0;
  SpectrumOld s(0.f);
  for (int i = 0; i < nBxDFs; ++i)
    if (bxdfs[i]->matchesFlags(types) &&
        ((reflect && (bxdfs[i]->type & BxDF::Type::BSDF_REFLECTION)) ||
         (!reflect && (bxdfs[i]->type & BxDF::Type::BSDF_TRANSMISSION))))
      s += bxdfs[i]->f(wo, wi);
  return s;
}
*/

HERMES_DEVICE_CALLABLE BSDF::BSDF() {}

HERMES_DEVICE_CALLABLE BSDF::BSDF(hermes::normal3 ns, hermes::vec3 dpdus, BxDF bxdf)
    : bxdf_(bxdf)/*, shadingFrame(Frame::FromXZ(Normalize(dpdus), Vector3f(ns)))*/ {}

HERMES_DEVICE_CALLABLE BSDF::~BSDF() {}

HERMES_DEVICE_CALLABLE BSDF::operator bool() const {
  return (bool) bxdf_;
}

} // namespace helios
