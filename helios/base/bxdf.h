#ifndef HELIOS_BXDF_H
#define HELIOS_BXDF_H

#include <helios/common/bitmask_operators.h>
#include <helios/core/mem.h>
#include <hermes/common/optional.h>
#include <helios/spectra/sampled_spectrum.h>
#include <hermes/geometry/vector.h>

namespace helios {

enum class TransportMode {
  RADIANCE,
  IMPORTANCE
};

HERMES_DEVICE_CALLABLE inline TransportMode operator!(TransportMode mode) {
  return (mode == TransportMode::RADIANCE) ? TransportMode::IMPORTANCE : TransportMode::RADIANCE;
}

enum class BxDFType {
  DIELECTRIC,
  CUSTOM
};

enum class bxdf_refl_trans_flags : i32 {
  UNSET = 0x0,
  REFLECTION = 0x1,
  TRANSMISSION = 0x2,
  ALL = REFLECTION | TRANSMISSION
};

HELIOS_ENABLE_BITMASK_OPERATORS(bxdf_refl_trans_flags);

enum class bxdf_flags : i32 {
  NONE = 0x0,
  REFLECTION = 0x1,
  TRANSMISSION = 0x2,
  DIFFUSE = 0x4,
  GLOSSY = 0x8,
  SPECULAR = 0x10,
  // Composite _BxDFFlags_ definitions
  DIFFUSE_REFLECTION = DIFFUSE | REFLECTION,
  DIFFUSE_TRANSMISSION = DIFFUSE | TRANSMISSION,
  GLOSSY_REFLECTION = GLOSSY | REFLECTION,
  GLOSSY_TRANSMISSION = GLOSSY | TRANSMISSION,
  SPECULAR_REFLECTION = SPECULAR | REFLECTION,
  SPECULAR_TRANSMISSION = SPECULAR | TRANSMISSION,
  ALL = DIFFUSE | GLOSSY | SPECULAR | REFLECTION | TRANSMISSION
};

HELIOS_ENABLE_BITMASK_OPERATORS(bxdf_flags);

// *********************************************************************************************************************
//                                                                                                               BXDF
// *********************************************************************************************************************
// BRDF and BTDF Interface
// BRDF (Bidirectional Reflectance Distribution Function) and BTDF (Bidirectional Transmission
// Distribution Function) are functions that describe how light scatters when it hits a particular
// material surface.
struct BxDF {
  // *******************************************************************************************************************
  //                                                                                                        OPERATORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE explicit operator bool() const { return (bool) data_ptr; }
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
  mem::Ptr data_ptr;                  //<
  bxdf_flags flags{bxdf_flags::NONE}; //<
  BxDFType type{BxDFType::CUSTOM};
};

// *********************************************************************************************************************
//                                                                                                         BSDFSample
// *********************************************************************************************************************
struct BSDFSample {
  HERMES_DEVICE_CALLABLE BSDFSample() {}
  HERMES_DEVICE_CALLABLE BSDFSample(SampledSpectrum f, hermes::vec3 wi, real_t pdf, bxdf_flags flags, real_t eta = 1,
                                    bool pdf_is_proportional = false)
      : f(f),
        wi(wi),
        pdf(pdf),
        flags(flags),
        eta(eta),
        pdf_is_proportional(pdf_is_proportional) {}

  SampledSpectrum f;
  hermes::vec3 wi;
  real_t pdf = 0;
  bxdf_flags flags;
  real_t eta = 1;
  bool pdf_is_proportional;
};

using BSDFSampleReturn = hermes::Optional<BSDFSample>;


//enum BxDFType {
//  BSDF_REFLECTION = 1 << 0,
//  BSDF_TRANSMISSION = 1 << 1,
//  BSDF_DIFFUSE = 1 << 2,
//  BSDF_GLOSSY = 1 << 3,
//  BSDF_SPECULAR = 1 << 4,
//  BSDF_ALL_TYPES = BSDF_DIFFUSE | BSDF_GLOSSY | BSDF_SPECULAR,
//  BSDF_ALL_REFLECTION = BSDF_REFLECTION | BSDF_ALL_TYPES,
//  BSDF_ALL_TRANSMISSION = BSDF_TRANSMISSION | BSDF_ALL_TYPES,
//  BSDF_ALL = BSDF_ALL_REFLECTION | BSDF_ALL_REFLECTION
//};
//
///* compute
//* @cosi **[in]** (**positive**)
//* @cost **[in]** (**positive**)
//* @etai **[in]** index of refraction for the incident media
//* @etat **[in]** index of refraction for the transmitted media
//* @return the Scattering reflection for dielectric and circularly polarized light
//*/
//SpectrumOld computeFresnelDielectric(float cosi, float cost, const SpectrumOld &etai, const SpectrumOld &etat) {
//  SpectrumOld rParl = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
//  SpectrumOld rPerp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));
//  return (SQR(rParl) + SQR(rPerp)) / 2.f;
//}
//
///* compute
//* @cosi **[in]** (**positive**)
//* @eta  **[in]** index of refraction for the conductor media
//* @k    **[in]** absorption coefficient of the conductor media
//* @return the Scattering reflection for conductors and circularly polarized light
//*/
//SpectrumOld computeFresnelConductor(float cosi, const SpectrumOld &eta, const SpectrumOld &k) {
//  SpectrumOld tmp_f = SQR(eta) + SQR(k);
//  SpectrumOld tmp = tmp_f * SQR(cosi);
//  SpectrumOld rParl2 = (tmp - (2.f * eta * cosi) + 1) / (tmp + (2.f * eta * cosi) + 1);
//  SpectrumOld rPerp2 = (tmp_f - (2.f * eta * cosi) + 1) / (tmp_f + (2.f * eta * cosi) + 1);
//  return (rParl2 + rPerp2) / 2.f;
//}
//
///* interface
//*/
//class Scattering {
//public:
//  /* compute
//  * @cosi **[in]** (**positive**)
//  */
//  virtual SpectrumOld evaluate(float cosi) const = 0;
//};
//
///* fresnel implementation
//*/
//class FresnelConductor : public Scattering {
//public:
//  /* Constructor
//  * @e  **[in]** index of refraction for the conductor media
//  * @_k **[in]** absorption coefficient of the conductor media
//  */
//  FresnelConductor(const SpectrumOld &e, const SpectrumOld &_k)
//      : eta(e), k(_k) {}
//  /* @inherit */
//  SpectrumOld evaluate(float cosi) const override {
//    return computeFresnelConductor(fabsf(cosi), eta, k);
//  }
//private:
//  SpectrumOld eta, k;
//};
//
///* fresnel implementation
//*/
//class FresnelDielectric : public Scattering {
//public:
//  /* Constructor
//  * @etai **[in]** index of refraction for the incident media
//  * @etat **[in]** index of refraction for the transmitted media
//  */
//  FresnelDielectric(float etai, float etat)
//      : eta_i(etai), eta_t(etat) {}
//  /* @inherit */
//  SpectrumOld evaluate(float cosi) const override {
//    cosi = ponos::clamp(cosi, -1.f, 1.f);
//    // Compute indices of refraction for dielectric
//    bool entering = cosi > 0.f;
//    float ei = eta_i, et = eta_t;
//    if (!entering)
//      std::swap(ei, et);
//    // Compute sint using Snell 's law
//    float sint = ei / et * sqrtf(std::max(0.f, 1.f - SQR(cosi)));
//    if (sint >= 1.f)
//      // internal reflection
//      return 1.f;
//    float cost = sqrtf(std::max(0.f, 1.f - SQR(sint)));
//    return computeFresnelDielectric(fabs(cosi), cost, ei, et);
//  }
//private:
//  float eta_i, eta_t;
//};
//
///* frestel implementation
//*/
//class FresnelNoOp : public Scattering {
//public:
//  /* @inherit */
//  SpectrumOld evaluate(float cosi) const override {
//    return SpectrumOld(1.f);
//  }
//};
//
///* interface
//* Interface for BRDF and BTDF functions.
//*/
//class BxDF {
//public:
//  BxDF(BxDFType t)
//      : type(t) {}
//  virtual ~BxDF() {}
//
//  /* distribution function
//  * @wo outgoing viewing direction
//  * @wi incident light direction
//  * @return the value of the distribution function for the given pair of directions.
//  */
//  virtual SpectrumOld f(const ponos::vec3 &wo, const ponos::vec3 &wi) const = 0;
//  /*
//  * @wo **[in]** outgoing viewing direction
//  * @wi **[out]** incident light direction
//  * @u1 **[in]**
//  * @u2 **[in]**
//  * @pdf **[out]**
//  * @return
//  */
//  virtual SpectrumOld sample_f(const ponos::vec3 &wo, ponos::vec3 *wi, float u1, float u2, float *pdf) const;
//  virtual SpectrumOld rho(const ponos::vec3 &wo, int nSamples, const float *samples) const;
//  virtual SpectrumOld rho(int nSamples, const float *samples1, const float *samples2) const;
//  bool matchesFlags(BxDFType flags) const {
//    return (type & flags) == type;
//  }
//  const BxDFType type;
//};
//
//class BRDFToBtDF : public BxDF {
//public:
//  BRDFToBtDF(BxDF *b)
//      : BxDF(BxDFType(b->type ^ (BSDF_REFLECTION | BSDF_TRANSMISSION))) {
//    brdf = b;
//  }
//  SpectrumOld f(const ponos::vec3 &wo, const ponos::vec3 &wi) const override {
//    return brdf->f(wo, ponos::otherHemisphere(wi));
//  }
//private:
//  BxDF *brdf;
//};
//
//class ScaledBxDF : public BxDF {
//public:
//  ScaledBxDF(BxDF *b, const SpectrumOld &sc)
//      : BxDF(BxDFType(b->type)), bxdf(b), s(sc) {}
//
//  SpectrumOld f(const ponos::vec3 &wo, const ponos::vec3 &wi) const override {
//    return s * bxdf->f(wo, wi);
//  }
//private:
//  BxDF *bxdf;
//  SpectrumOld s;
//};
//
///* BxDF implementation
//*/
//class SpecularReflection : public BxDF {
//public:
//  SpecularReflection(const SpectrumOld &r, Scattering *f)
//      : BxDF(BxDFType(BSDF_REFLECTION | BSDF_SPECULAR)),
//        R(r), fresnel(f) {}
//
//  /* @inherit */
//  SpectrumOld f(const ponos::vec3 &, const ponos::vec3 &) const override {
//    return SpectrumOld(0.f);
//  }
//  /* @inherit */
//  SpectrumOld sample_f(const ponos::vec3 &wo, ponos::vec3 *wi, float u1, float u2, float *pdf) const override {
//    *wi = ponos::vec3(-wo.x, -wo.y, wo.z);
//    *pdf = 1.f;
//    return fresnel->evaluate(ponos::cosTheta(wo)) * R / ponos::absCosTheta(*wi);
//  }
//private:
//  SpectrumOld R;
//  Scattering *fresnel;
//};
//
///* BxDF implementation
//*/
//class SpecularTransmission : public BxDF {
//public:
//  SpecularTransmission(const SpectrumOld &t, float ei, float et)
//      : BxDF(BxDFType(BSDF_TRANSMISSION | BSDF_SPECULAR)),
//        fresnel(ei, et) {
//    T = t;
//    etai = ei;
//    etat = et;
//  }
//
//  /* @inherit */
//  SpectrumOld f(const ponos::vec3 &, const ponos::vec3 &) const override {
//    return SpectrumOld(0.f);
//  }
//  /* @inherit */
//  SpectrumOld sample_f(const ponos::vec3 &wo, ponos::vec3 *wi, float u1, float u2, float *pdf) const override {
//    bool entering = ponos::cosTheta(wo) > 0.f;
//    float ei = etai, et = etat;
//    if (!entering)
//      std::swap(ei, et);
//    float sini2 = ponos::sinTheta2(wo);
//    float eta = ei / et;
//    float sint2 = SQR(eta) * sini2;
//    if (sint2 >= 1.f)
//      return 0.f;
//    float cost = sqrtf(std::max(0.f, 1.f - sint2));
//    if (entering)
//      cost = -cost;
//    float sint_sini = eta;
//    *wi = ponos::vec3(sint_sini * (-wo.x), sint_sini * (-wo.y), cost);
//    *pdf = 1.f;
//    SpectrumOld F = fresnel.evaluate(ponos::cosTheta(wo));
//    return SQR(et) / SQR(ei) * (SpectrumOld(1.f) - F) * T / ponos::absCosTheta(*wi);
//  }
//private:
//  SpectrumOld T;
//  float etai, etat;
//  FresnelDielectric fresnel;
//};
//
///* BxDF implementation
//*/
//class Lambertian : public BxDF {
//public:
//  /* Constructor
//  * @reflectance **[in]** the fraction of incident light that is scattered
//  */
//  Lambertian(const SpectrumOld &reflectance)
//      : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(reflectance) {}
//  /* @inherit */
//  SpectrumOld f(const ponos::vec3 &wo, const ponos::vec3 &wi) const override {
//    return R * INV_PI;
//  }
//  /* @inherit */
//  SpectrumOld rho(const ponos::vec3 &wo, int nSamples, const float *samples) const override { return R; };
//  /* @inherit */
//  SpectrumOld rho(int nSamples, const float *samples1, const float *samples2) const override { return R; };
//private:
//  SpectrumOld R;
//};
//
///* BxDF implementation
//*/
//class OrenNayar : public BxDF {
//public:
//  /* Constructor
//  * @reflectance **[in]** the fraction of incident light that is scattered
//  * @sig **[in]** sigma value
//  */
//  OrenNayar(const SpectrumOld &reflectance, float sig)
//      : BxDF(BxDFType(BSDF_REFLECTION | BSDF_DIFFUSE)), R(reflectance) {
//    float sigma = TO_RADIANS(sig);
//    float sigma2 = SQR(sigma);
//    A = 1.f - (sigma2 / (2.f * (sigma2 + 0.33f)));
//    B = 0.45f * sigma2 / (sigma2 + 0.09f);
//  }
//  /* @inherit */
//  SpectrumOld f(const ponos::vec3 &wo, const ponos::vec3 &wi) const override {
//    float sinthetai = ponos::sinTheta(wi);
//    float sinthetao = ponos::sinTheta(wo);
//    // Compute cosine term of Oren-Nayar model
//    float maxcos = 0.f;
//    if (sinthetai > 1e-4 && sinthetao > 1e-4) {
//      float sinphii = ponos::sinPhi(wi), cosphii = ponos::cosPhi(wi);
//      float sinphio = ponos::sinPhi(wo), cosphio = ponos::cosPhi(wo);
//      float dcos = cosphii * cosphio + sinphii * sinphio;
//      maxcos = std::max(0.f, dcos);
//    }
//    // Compute sine and tangent terms of Oren-Nayar model
//    float sinalpha, tanbeta;
//    if (ponos::absCosTheta(wi) > ponos::absCosTheta(wo)) {
//      sinalpha = sinthetao;
//      tanbeta = sinthetai / ponos::absCosTheta(wi);
//    } else {
//      sinalpha = sinthetai;
//      tanbeta = sinthetao / ponos::absCosTheta(wo);
//    }
//    return R * INV_PI * (A + B * maxcos * sinalpha * tanbeta);
//  }
//  /* @inherit */
//  SpectrumOld rho(const ponos::vec3 &wo, int nSamples, const float *samples) const override { return R; };
//  /* @inherit */
//  SpectrumOld rho(int nSamples, const float *samples1, const float *samples2) const override { return R; };
//private:
//  SpectrumOld R;
//  float A, B;
//};

} // helios namespace

#endif // HELIOS_BXDF_H
