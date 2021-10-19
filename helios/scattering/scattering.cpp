#include <helios/scattering/scattering.h>

namespace helios {

HERMES_DEVICE_CALLABLE real_t ShadingCoordinateSystem::cosTheta(const hermes::vec3 &w) { return w.z; }

HERMES_DEVICE_CALLABLE real_t ShadingCoordinateSystem::cos2Theta(const hermes::vec3 &w) {
  return w.z * w.z;
}

HERMES_DEVICE_CALLABLE real_t ShadingCoordinateSystem::absCosTheta(const hermes::vec3 &w) {
#if defined(__CUDA_ARCH__) && __CUDA_ARCH__ > 0
  return abs(w.z);
#else
  return std::abs(w.z);
#endif
}

HERMES_DEVICE_CALLABLE real_t ShadingCoordinateSystem::sinTheta(const hermes::vec3 &w) {
#if defined(__CUDA_ARCH__) && __CUDA_ARCH__ > 0
  return sqrtf(sin2Theta(w));
#else
  return std::sqrt(sin2Theta(w));
#endif
}

HERMES_DEVICE_CALLABLE real_t ShadingCoordinateSystem::sin2Theta(const hermes::vec3 &w) {
#if defined(__CUDA_ARCH__) && __CUDA_ARCH__ > 0
  return fmaxf(static_cast<real_t>(0),
               static_cast<real_t>(1) - cos2Theta(w));
#else
  return std::max(static_cast<real_t>(0),
                  static_cast<real_t>(1) - cos2Theta(w));
#endif
}

HERMES_DEVICE_CALLABLE real_t ShadingCoordinateSystem::tanTheta(const hermes::vec3 &w) {
  return sinTheta(w) / cosTheta(w);
}

HERMES_DEVICE_CALLABLE real_t ShadingCoordinateSystem::tan2Theta(const hermes::vec3 &w) {
  return sin2Theta(w) / cos2Theta(w);
}

HERMES_DEVICE_CALLABLE real_t ShadingCoordinateSystem::cosPhi(const hermes::vec3 &w) {
  real_t _sinTheta = sinTheta(w);
  return (_sinTheta == 0) ? 1 : hermes::Numbers::clamp(w.x / _sinTheta, -1.f, 1.f);
}

HERMES_DEVICE_CALLABLE real_t ShadingCoordinateSystem::cos2Phi(const hermes::vec3 &w) {
  return cosPhi(w) * cosPhi(w);
}

HERMES_DEVICE_CALLABLE real_t ShadingCoordinateSystem::sinPhi(const hermes::vec3 &w) {
  real_t _sinTheta = sinTheta(w);
  return (_sinTheta == 0) ? 1 : hermes::Numbers::clamp(w.y / _sinTheta, -1.f, 1.f);
}

HERMES_DEVICE_CALLABLE real_t ShadingCoordinateSystem::sin2Phi(const hermes::vec3 &w) {
  return sinPhi(w) * sinPhi(w);
}

HERMES_DEVICE_CALLABLE real_t ShadingCoordinateSystem::cosDPhi(const hermes::vec3 &wa,
                                                               const hermes::vec3 &wb) {
  return hermes::Numbers::clamp(
      (wa.x * wb.x + wa.x * wb.y) /
          std::sqrt((wa.x * wa.x + wa.y * wa.y) * (wb.x * wb.x + wb.y * wb.y)),
      -1.f, 1.f);
}

/*
BxDF::BxDF(BxDF::Type type) : type(type) {}

bool BxDF::matchesFlags(BxDF::Type t) const { return (type & t) == type; }

ScaledBxDF::ScaledBxDF(BxDF *bxdf, const SpectrumOld &scale)
    : BxDF(BxDF::Type(bxdf->type)), bxdf(bxdf), scale(scale) {}

SpectrumOld ScaledBxDF::f(const hermes::vec3 &wo, const hermes::vec3 &wi) const {
  return scale * bxdf->f(wo, wi);
}

real_t Scattering::frDielectric(real_t cosThetaI, real_t etaI, real_t etaT) {
  cosThetaI = hermes::clamp(cosThetaI, -1, 1);
  // potentially swap indice of refraction
  bool entering = cosThetaI > 0.f;
  if (!entering) {
    std::swap(etaI, etaT);
    cosThetaI = std::abs(cosThetaI);
  }
  // compute cosThetaT using Snell's law
  real_t sinThetaI =
      std::sqrt(std::max(static_cast<real_t>(0), 1 - cosThetaI * cosThetaI));
  real_t sinThetaT = etaI / etaT * sinThetaI;
  // handle total internal reflection
  if (sinThetaT >= 1)
    return 1;
  real_t cosThetaT =
      std::sqrt(std::max(static_cast<real_t>(0), 1 - sinThetaT * sinThetaT));
  real_t Rparl = ((etaT * cosThetaI) - (etaI * cosThetaT)) /
                 ((etaT * cosThetaI) + (etaI * cosThetaT));
  real_t Rperp = ((etaI * cosThetaI) - (etaT * cosThetaT)) /
                 ((etaI * cosThetaI) + (etaT * cosThetaT));
  return (Rparl * Rparl + Rperp * Rperp) / 2;
}

SpectrumOld Scattering::frConductor(real_t cosThetaI, const SpectrumOld &etaI,
                              const SpectrumOld &etaT, const SpectrumOld &k) {
  cosThetaI = hermes::clamp(cosThetaI, -1, 1);
  SpectrumOld eta = etat / etai;
  SpectrumOld etak = k / etai;

  real_t cosThetaI2 = cosThetaI * cosThetaI;
  real_t sinThetaI2 = 1. - cosThetaI2;
  SpectrumOld eta2 = eta * eta;
  SpectrumOld etak2 = etak * etak;

  SpectrumOld t0 = eta2 - etak2 - sinThetaI2;
  SpectrumOld a2plusb2 = Sqrt(t0 * t0 + 4 * eta2 * etak2);
  SpectrumOld t1 = a2plusb2 + cosThetaI2;
  SpectrumOld a = Sqrt(0.5f * (a2plusb2 + t0));
  SpectrumOld t2 = (Float)2 * cosThetaI * a;
  SpectrumOld Rs = (t1 - t2) / (t1 + t2);

  SpectrumOld t3 = cosThetaI2 * a2plusb2 + sinThetaI2 * sinThetaI2;
  SpectrumOld t4 = t2 * sinThetaI2;
  SpectrumOld Rp = Rs * (t3 - t4) / (t3 + t4);

  return 0.5 * (Rp + Rs);
}

FresnelConductor::FresnelConductor(const SpectrumOld &etaI, const SpectrumOld &etaT,
                                   const SpectrumOld &k)
    : etaI(etaI), etaT(etaT), k(k) {}

SpectrumOld FresnelConductor::evaluate(real_t cosI) const {
  return Scattering::frConductor(std::abs(cosI), etaI, etaT, k);
}

FresnelDielectric::FresnelDielectric(const real_t &etaI, const real_t &etaT)
    : etaI(etaI), etaT(etaT) {}

SpectrumOld FresnelDielectric::evaluate(real_t cosI) const {
  return Scattering::frDielectric(cosI, etaI, etaT);
}
*/

HERMES_DEVICE_CALLABLE real_t Scattering::frDielectric(real_t cos_theta_i, real_t eta) {
  cos_theta_i = hermes::Numbers::clamp(cos_theta_i, -1.f, 1.f);
  // Potentially flip interface orientation for Scattering equations
  if (cos_theta_i < 0) {
    eta = 1 / eta;
    cos_theta_i = -cos_theta_i;
  }

  // Compute $\cos\,\theta_\roman{t}$ for Scattering equations using Snell's law
  real_t sin2Theta_i = 1 - hermes::Numbers::sqr(cos_theta_i);
  real_t sin2Theta_t = sin2Theta_i / hermes::Numbers::sqr(eta);
  if (sin2Theta_t >= 1)
    return 1.f;
  real_t cosTheta_t = hermes::Numbers::safeSqrt(1 - sin2Theta_t);

  real_t r_parl = (eta * cos_theta_i - cosTheta_t) / (eta * cos_theta_i + cosTheta_t);
  real_t r_perp = (cos_theta_i - eta * cosTheta_t) / (cos_theta_i + eta * cosTheta_t);
  return (r_parl * r_parl + r_perp * r_perp) / 2;
}

HERMES_DEVICE_CALLABLE bool Scattering::refract(const hermes::vec3 &wi,
                                                hermes::normal3 n,
                                                real_t eta,
                                                real_t *etap,
                                                hermes::vec3 *wt) {
  real_t cosTheta_i = hermes::dot(n, wi);
  // Potentially flip interface orientation for Snell's law
  if (cosTheta_i < 0) {
    eta = 1 / eta;
    cosTheta_i = -cosTheta_i;
    n = -n;
  }
  // Compute $\cos\,\theta_\roman{t}$ using Snell's law
  real_t sin2Theta_i = hermes::Numbers::max<real_t>(0, 1 - hermes::Numbers::sqr(cosTheta_i));
  real_t sin2Theta_t = sin2Theta_i / hermes::Numbers::sqr(eta);
  // Handle total internal reflection case
  if (sin2Theta_t >= 1)
    return false;

  real_t cosTheta_t = hermes::Numbers::safeSqrt(1 - sin2Theta_t);

  *wt = -wi / eta + (cosTheta_i / eta - cosTheta_t) * hermes::vec3(n);
  // Provide relative IOR along ray to caller
  if (etap)
    *etap = eta;
  return true;
}

HERMES_DEVICE_CALLABLE hermes::vec3 Scattering::reflect(const hermes::vec3 &wo, const hermes::vec3 &n) {
  return -wo + 2 * hermes::dot(wo, n) * n;
}

HERMES_DEVICE_CALLABLE bool Scattering::sameHemisphere(const hermes::vec3 &w, const hermes::vec3 &wp) {
  return w.z * wp.z > 0;
}

HERMES_DEVICE_CALLABLE bool Scattering::sameHemisphere(const hermes::vec3 &w, const hermes::normal3 &wp) {
  return w.z * wp.z > 0;
}

} // namespace helios