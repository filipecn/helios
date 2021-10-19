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
///\file trowbridge_reitz_distribution.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-13
///
///\brief

#include <helios/scattering/trowbridge_reitz_distribution.h>
#include <helios/scattering/scattering.h>
#include <hermes/numeric/interpolation.h>
#include <helios/core/sampling.h>

namespace helios {

HERMES_DEVICE_CALLABLE MicrofacetDistribution TrowbridgeReitzDistribution::createMFD(mem::Ptr data_ptr) {
  return {
      .data_ptr = data_ptr,
      .type = MicrofacetDistributionType::TROWBRIDGE_REITZ
  };
}

HERMES_DEVICE_CALLABLE real_t TrowbridgeReitzDistribution::roughness2alpha(real_t roughness) {
#if (defined(__CUDA__ARCH__) && __CUDA_ARCH__ > 0)
  return sqrtf(roughness);
#else
  return std::sqrt(roughness);
#endif
}

HERMES_DEVICE_CALLABLE TrowbridgeReitzDistribution::TrowbridgeReitzDistribution() {}

HERMES_DEVICE_CALLABLE TrowbridgeReitzDistribution::~TrowbridgeReitzDistribution() {}

HERMES_DEVICE_CALLABLE TrowbridgeReitzDistribution::TrowbridgeReitzDistribution(real_t alpha_x, real_t alpha_y)
    : alpha_x_(alpha_x), alpha_y_(alpha_y) {}

HERMES_DEVICE_CALLABLE bool TrowbridgeReitzDistribution::effectivelySmooth() const {
#if (defined(__CUDA_ARCH__) && __CUDA_ARCH__ > 0)
  return fmaxf(alpha_x_, alpha_y_) < 1e-3f;
#else
  return std::max(alpha_x_, alpha_y_) < 1e-3f;
#endif
}

HERMES_DEVICE_CALLABLE real_t TrowbridgeReitzDistribution::Lambda(const hermes::vec3 &w) const {
  real_t tan2Theta = ShadingCoordinateSystem::tan2Theta(w);
  if (isinf(tan2Theta))
    return 0;
  real_t alpha2 = hermes::Numbers::sqr(ShadingCoordinateSystem::cosPhi(w) * alpha_x_)
      + hermes::Numbers::sqr(ShadingCoordinateSystem::sinPhi(w) * alpha_y_);
  return .5f * (std::sqrt(1 + alpha2 * tan2Theta) - 1);
}

HERMES_DEVICE_CALLABLE real_t TrowbridgeReitzDistribution::D(const hermes::vec3 &wm) const {
  real_t tan2Theta = ShadingCoordinateSystem::tan2Theta(wm);
  if (isinf(tan2Theta))
    return 0;
  real_t cos4Theta = hermes::Numbers::sqr(ShadingCoordinateSystem::cos2Theta(wm));
  if (cos4Theta < 1e-16f)
    return 0;
  real_t e = tan2Theta
      * (hermes::Numbers::sqr(ShadingCoordinateSystem::cosPhi(wm) / alpha_x_)
          + hermes::Numbers::sqr(ShadingCoordinateSystem::sinPhi(wm) / alpha_y_));
  return 1 / (hermes::Constants::pi * alpha_x_ * alpha_y_ * cos4Theta * hermes::Numbers::sqr(1 + e));
}

HERMES_DEVICE_CALLABLE real_t TrowbridgeReitzDistribution::G1(const hermes::vec3 &w) const {
  return 1 / (1 + Lambda(w));
}

HERMES_DEVICE_CALLABLE real_t TrowbridgeReitzDistribution::G(const hermes::vec3 &wo, const hermes::vec3 &wi) const {
  return 1 / (1 + Lambda(wo) + Lambda(wi));
}

HERMES_DEVICE_CALLABLE hermes::vec3 TrowbridgeReitzDistribution::sample_wm(const hermes::vec3 &w,
                                                                           const hermes::point2 &u) const {
  // Transform _w_ to hemispherical configuration
  hermes::vec3 wh = hermes::normalize(hermes::vec3(alpha_x_ * w.x, alpha_y_ * w.y, w.z));
  if (wh.z < 0)
    wh = -wh;

  // Find orthonormal basis for visible normal sampling
  hermes::vec3 T1 = (wh.z < 0.99999f) ? hermes::normalize(hermes::cross(hermes::vec3(0, 0, 1), wh))
                                      : hermes::vec3(1, 0, 0);
  hermes::vec3 T2 = hermes::cross(wh, T1);

  // Generate uniformly distributed points on the unit disk
  hermes::point2 p = sampling::sampleUniformDiskPolar(u);

  // Warp hemispherical projection for visible normal sampling
  real_t h = hermes::Numbers::sqrt(1.f - hermes::Numbers::sqr(p.x));
  p.y = hermes::interpolation::lerp(.5f * (1.f + wh.z), h, p.y);

  // Reproject to hemisphere and transform normal to ellipsoid configuration
#if defined(__CUDA_ARCH__) && __CUDA_ARCH__ > 0
  real_t pz = sqrtf(fmaxf(0, 1 - hermes::vec2(p).length2()));
#else
  real_t pz = std::sqrt(std::max<real_t>(0, 1 - hermes::vec2(p).length2()));
#endif
  hermes::vec3 nh = p.x * T1 + p.y * T2 + pz * wh;
  // TODO CHECK_RARE(1e-5f, nh.z == 0);
#if defined(__CUDA_ARCH__) && __CUDA_ARCH__ > 0
  return hermes::normalize(
      hermes::vec3(alpha_x_ * nh.x, alpha_y_ * nh.y, fmaxf(1e-6f, nh.z)));
#else
  return hermes::normalize(
      hermes::vec3(alpha_x_ * nh.x, alpha_y_ * nh.y, std::max<real_t>(1e-6f, nh.z)));
#endif
}

HERMES_DEVICE_CALLABLE real_t TrowbridgeReitzDistribution::PDF(const hermes::vec3 &w, const hermes::vec3 &wm) const {
  return D(w, wm);
}

HERMES_DEVICE_CALLABLE real_t TrowbridgeReitzDistribution::D(const hermes::vec3 &w, const hermes::vec3 &wm) const {
#if defined(__CUDA_ARCH__) && __CUDA_ARCH__ > 0
  return D(wm) * G1(w) * fabsf(hermes::dot(w, wm) / ShadingCoordinateSystem::cosTheta(w));
#else
  return D(wm) * G1(w) * std::abs(hermes::dot(w, wm) / ShadingCoordinateSystem::cosTheta(w));
#endif
}

}
