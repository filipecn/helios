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
///\file dielectric_bxdf.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-14
///
///\brief

#include <helios/scattering/dielectric_bxdf.h>
#include <helios/scattering/scattering.h>

namespace helios {

HERMES_DEVICE_CALLABLE DielectricBxDF::DielectricBxDF() {}

HERMES_DEVICE_CALLABLE DielectricBxDF::DielectricBxDF(real_t eta, const TrowbridgeReitzDistribution &mf_distribution)
    : eta_(eta), mf_distribution_(mf_distribution) {}

HERMES_DEVICE_CALLABLE DielectricBxDF::~DielectricBxDF() {}

HERMES_DEVICE_CALLABLE SampledSpectrum DielectricBxDF::f(const hermes::vec3 &wo,
                                                         const hermes::vec3 &wi,
                                                         TransportMode mode) const {
  if (eta_ == 1 || mf_distribution_.effectivelySmooth())
    return SampledSpectrum(0.f);
  // Evaluate rough dielectric BSDF
  // Compute generalized half vector _wm_
  real_t cosTheta_o = ShadingCoordinateSystem::cosTheta(wo), cosTheta_i = ShadingCoordinateSystem::cosTheta(wi);
  bool reflect = cosTheta_i * cosTheta_o > 0;
  float etap = 1;
  if (!reflect)
    etap = cosTheta_o > 0 ? eta_ : (1 / eta_);
  hermes::vec3 wm = wi * etap + wo;
  // TODO check wm.length2() == 0
  if (cosTheta_i == 0 || cosTheta_o == 0 || wm.length2() == 0)
    return {};
  wm = hermes::faceForward(hermes::normalize(wm), hermes::normal3(0, 0, 1));

  // Discard back-facing microfacets
  if (hermes::dot(wm, wi) * cosTheta_i < 0 || hermes::dot(wm, wo) * cosTheta_o < 0)
    return {};

  real_t F = Scattering::frDielectric(hermes::dot(wo, wm), eta_);
  if (reflect) {
    // Compute reflection at rough dielectric interface
    return SampledSpectrum(mf_distribution_.D(wm) * mf_distribution_.G(wo, wi) * F /
        fabsf(4 * cosTheta_i * cosTheta_o));

  }
  // Compute transmission at rough dielectric interface
  real_t denom = hermes::Numbers::sqr(hermes::dot(wi, wm) + hermes::dot(wo, wm) / etap);
  real_t ft =
      (1 - F) * mf_distribution_.D(wm) * mf_distribution_.G(wo, wi) *
          fabsf(hermes::dot(wi, wm) * hermes::dot(wo, wm) / (cosTheta_i * cosTheta_o * denom));
  // Account for non-symmetry with transmission to different medium
  if (mode == TransportMode::RADIANCE)
    ft /= hermes::Numbers::sqr(etap);

  return SampledSpectrum(ft);
}

HERMES_DEVICE_CALLABLE BSDFSampleReturn DielectricBxDF::sample_f(const hermes::vec3 &wo,
                                                                 real_t uc,
                                                                 const hermes::point2 &u,
                                                                 TransportMode mode,
                                                                 bxdf_refl_trans_flags sample_flags) const {
  if (eta_ == 1 || mf_distribution_.effectivelySmooth()) {
    // Sample perfectly specular dielectric BSDF
    real_t R = Scattering::frDielectric(ShadingCoordinateSystem::cosTheta(wo), eta_), T = 1 - R;
    // Compute probabilities _pr_ and _pt_ for sampling reflection and transmission
    real_t pr = R, pt = T;
    if (!((u32) sample_flags & (u32) bxdf_refl_trans_flags::REFLECTION))
      pr = 0;
    if (!((u32) sample_flags & (u32) bxdf_refl_trans_flags::TRANSMISSION))
      pt = 0;
    if (pr == 0 && pt == 0)
      return {};

    if (uc < pr / (pr + pt)) {
      // Sample perfect specular dielectric BRDF
      hermes::vec3 wi(-wo.x, -wo.y, wo.z);
      SampledSpectrum fr(R / ShadingCoordinateSystem::absCosTheta(wi));
      return BSDFSample(fr, wi, pr / (pr + pt), bxdf_flags::SPECULAR_REFLECTION);

    } else {
      // Sample perfect specular dielectric BTDF
      // Compute ray direction for specular transmission
      hermes::vec3 wi;
      real_t eta_p;
      bool valid = Scattering::refract(wo, hermes::normal3(0, 0, 1), eta_, &eta_p, &wi);
      // TODO CHECK VALID
      if (!valid)
        return {};

      SampledSpectrum ft(T / ShadingCoordinateSystem::absCosTheta(wi));
      // Account for non-symmetry with transmission to different medium
      if (mode == TransportMode::RADIANCE)
        ft /= hermes::Numbers::sqr(eta_p);

      return BSDFSample(ft, wi, pt / (pr + pt), bxdf_flags::SPECULAR_TRANSMISSION,
                        eta_p);
    }

  } else {
    // Sample rough dielectric BSDF
    hermes::vec3 wm = mf_distribution_.sample_wm(wo, u);
    real_t R = Scattering::frDielectric(hermes::dot(wo, wm), eta_);
    real_t T = 1 - R;
    // Compute probabilities _pr_ and _pt_ for sampling reflection and transmission
    real_t pr = R, pt = T;
    if (!((u32) sample_flags & (u32) bxdf_refl_trans_flags::REFLECTION))
      pr = 0;
    if (!((u32) sample_flags & (u32) bxdf_refl_trans_flags::TRANSMISSION))
      pt = 0;
    if (pr == 0 && pt == 0)
      return {};

    real_t pdf;
    if (uc < pr / (pr + pt)) {
      // Sample reflection at rough dielectric interface
      hermes::vec3 wi = Scattering::reflect(wo, wm);
      if (!Scattering::sameHemisphere(wo, wi))
        return {};
      // Compute PDF of rough dielectric reflection
      pdf = mf_distribution_.PDF(wo, wm) / (4 * abs(hermes::dot(wo, wm))) * pr / (pr + pt);

      HERMES_CHECK_EXP(!hermes::Check::is_nan(pdf))
      SampledSpectrum f(mf_distribution_.D(wm) * mf_distribution_.G(wo, wi) * R /
          (4 * ShadingCoordinateSystem::cosTheta(wi) * ShadingCoordinateSystem::cosTheta(wo)));
      return BSDFSample(f, wi, pdf, bxdf_flags::GLOSSY_REFLECTION);

    } else {
      // Sample transmission at rough dielectric interface
      real_t eta_p;
      hermes::vec3 wi;
      bool tir = !Scattering::refract(wo, (hermes::normal3) wm, eta_, &eta_p, &wi);
      // TODO CHECK_RARE(1e-5f, tir);
      if (Scattering::sameHemisphere(wo, wi) || wi.z == 0 || tir)
        return {};
      // Compute PDF of rough dielectric transmission
      real_t denom = hermes::Numbers::sqr(hermes::dot(wi, wm) + hermes::dot(wo, wm) / eta_p);
      real_t dwm_dwi = abs(hermes::dot(wi, wm)) / denom;
      pdf = mf_distribution_.PDF(wo, wm) * dwm_dwi * pt / (pr + pt);

      HERMES_CHECK_EXP(!hermes::Check::is_nan(pdf));
      // Evaluate BRDF and return _BSDFSample_ for rough transmission
      SampledSpectrum ft(T * mf_distribution_.D(wm) * mf_distribution_.G(wo, wi) *
          std::abs(hermes::dot(wi, wm) * hermes::dot(wo, wm) /
              (ShadingCoordinateSystem::cosTheta(wi) * ShadingCoordinateSystem::cosTheta(wo) * denom)));
      // Account for non-symmetry with transmission to different medium
      if (mode == TransportMode::RADIANCE)
        ft /= hermes::Numbers::sqr(eta_p);

      return BSDFSample(ft, wi, pdf, bxdf_flags::GLOSSY_TRANSMISSION, eta_p);
    }
  }
  return helios::BSDFSampleReturn();
}

HERMES_DEVICE_CALLABLE real_t DielectricBxDF::PDF(const hermes::vec3 &wo,
                                                  const hermes::vec3 &wi,
                                                  TransportMode mode,
                                                  bxdf_refl_trans_flags sampleFlags) const {
  if (eta_ == 1 || mf_distribution_.effectivelySmooth())
    return 0.f;
  // Evaluate sampling PDF of rough dielectric BSDF
  // Compute generalized half vector _wm_
  real_t cosTheta_o = ShadingCoordinateSystem::cosTheta(wo), cosTheta_i =
      ShadingCoordinateSystem::cosTheta(wi);
  bool reflect = cosTheta_i * cosTheta_o > 0;
  float etap = 1;
  if (!reflect)
    etap = cosTheta_o > 0 ? eta_ : (1 / eta_);
  hermes::vec3 wm = wi * etap + wo;
  // TODO CHECK_RARE(1e-5f, LengthSquared(wm) == 0);
  if (cosTheta_i == 0 || cosTheta_o == 0 || wm.length2() == 0)
    return {};
  wm = hermes::faceForward(hermes::normalize(wm), hermes::normal3(0, 0, 1));

  // Discard back-facing microfacets
  if (hermes::dot(wm, wi) * cosTheta_i < 0 || hermes::dot(wm, wo) * cosTheta_o < 0)
    return {};

  // Determine Fresnel reflectance of rough dielectric boundary
  real_t R = Scattering::frDielectric(hermes::dot(wo, wm), eta_);
  real_t T = 1.f - R;

  // Compute probabilities _pr_ and _pt_ for sampling reflection and transmission
  real_t pr = R, pt = T;
  if (!((u32) sampleFlags & (u32) bxdf_refl_trans_flags::REFLECTION))
    pr = 0;
  if (!((u32) sampleFlags & (u32) bxdf_refl_trans_flags::TRANSMISSION))
    pt = 0;
  if (pr == 0 && pt == 0)
    return {};

  real_t pdf;
  if (reflect) {
    // Compute PDF of rough dielectric reflection
    pdf = mf_distribution_.PDF(wo, wm) / (4 * abs(hermes::dot(wo, wm))) * pr / (pr + pt);

  } else {
    // Compute PDF of rough dielectric transmission
    real_t denom = hermes::Numbers::sqr(hermes::dot(wi, wm) + hermes::dot(wo, wm) / etap);
    real_t dwm_dwi = abs(hermes::dot(wi, wm)) / denom;
    pdf = mf_distribution_.PDF(wo, wm) * dwm_dwi * pt / (pr + pt);
  }
  return pdf;
}

}
