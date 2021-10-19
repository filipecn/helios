#ifndef HELIOS_CORE_REFLECTION_H
#define HELIOS_CORE_REFLECTION_H

#include <helios/base/spectrum.h>
#include <hermes/geometry/vector.h>

namespace helios {

/// Reflections are computed in a coordinate system where the two tangent
/// vectors and the normal vector at the point being shaded are aligned with x,
/// y and z axes, respectively. All BRDF and BTDF work with vectors in this
/// system.
/// Spherical coordinates can be used to express directions here, where
/// **theta** is the angle between the direction and z axis, and **phi** is
/// formed with the x axis after projection onto the xy plane.
class ShadingCoordinateSystem {
public:
  /// \param w direction vector
  /// \return real_t cosine of angle theta (between w and z)
  HERMES_DEVICE_CALLABLE static real_t cosTheta(const hermes::vec3 &w);
  /// \param w direction vector
  /// \return real_t squared cosine of angle theta (between w and z)
  HERMES_DEVICE_CALLABLE static real_t cos2Theta(const hermes::vec3 &w);
  /// \param w direction vector
  /// \return real_t absolute cosine of angle theta (between w and z)
  HERMES_DEVICE_CALLABLE static real_t absCosTheta(const hermes::vec3 &w);
  /// \param w direction vector
  /// \return real_t sine of angle theta (between w and z)
  HERMES_DEVICE_CALLABLE static real_t sinTheta(const hermes::vec3 &w);
  /// \param w direction vector
  /// \return real_t squared sine of angle theta (between w and z)
  HERMES_DEVICE_CALLABLE static real_t sin2Theta(const hermes::vec3 &w);
  /// \param w direction vector
  /// \return real_t tangent of angle theta (between w and z)
  HERMES_DEVICE_CALLABLE static real_t tanTheta(const hermes::vec3 &w);
  /// \param w direction vector
  /// \return real_t squared tangent of angle theta (between w and z)
  HERMES_DEVICE_CALLABLE static real_t tan2Theta(const hermes::vec3 &w);
  /// \param w direction vector
  /// \return real_t cosine of angle phi (between projected w and x)
  HERMES_DEVICE_CALLABLE static real_t cosPhi(const hermes::vec3 &w);
  /// \param w direction vector
  /// \return real_t squared cosine of angle phi (between projected w and x)
  HERMES_DEVICE_CALLABLE static real_t cos2Phi(const hermes::vec3 &w);
  /// \param w direction vector
  /// \return real_t sine of angle phi (between projected w and x)
  HERMES_DEVICE_CALLABLE static real_t sinPhi(const hermes::vec3 &w);
  /// \param w direction vector
  /// \return real_t squared sine of angle phi (between projected w and x)
  HERMES_DEVICE_CALLABLE static real_t sin2Phi(const hermes::vec3 &w);
  /// Computes the cosine of the angle between two vectors in the shading
  /// coordinate system
  /// \param wa vector (in the shading coordinate system)
  /// \param wb vector (in the shading coordinate system)
  /// \return real_t cosine value
  HERMES_DEVICE_CALLABLE static real_t cosDPhi(const hermes::vec3 &wa, const hermes::vec3 &wb);
};

struct Scattering {
  /// Computes the Scattering reflection formula for dielectric materials and un-polarized light.
  /// \param cosThetaI cosine of incident angle (formed with normal)
  /// \param etaI index of refraction for the incident media
  /// \param etaT index of refraction for the transmitted media
  /// \return real_t the Scattering reflectance value
  HERMES_DEVICE_CALLABLE static real_t frDielectric(real_t cos_theta_i, real_t eta);
  ///
  /// \param wi
  /// \param n
  /// \param eta
  /// \param etap
  /// \param wt
  /// \return
  HERMES_DEVICE_CALLABLE static bool refract(const hermes::vec3 &wi,
                                             hermes::normal3 n,
                                             real_t eta,
                                             real_t *etap,
                                             hermes::vec3 *wt);
  ///
  /// \param wo
  /// \param n
  /// \return
  HERMES_DEVICE_CALLABLE static hermes::vec3 reflect(const hermes::vec3 &wo, const hermes::vec3 &n);
  ///
  /// \param w
  /// \param wp
  /// \return
  HERMES_DEVICE_CALLABLE static bool sameHemisphere(const hermes::vec3& w, const hermes::vec3& wp);
  ///
  /// \param w
  /// \param wp
  /// \return
  HERMES_DEVICE_CALLABLE static bool sameHemisphere(const hermes::vec3& w, const hermes::normal3& wp);
};
/*
class BxDF {
public:
  /// Scattering types
  enum Type {
    BSDF_REFLECTION = 1 << 0,
    BSDF_TRANSMISSION = 1 << 1,
    BSDF_DIFFUSE = 1 << 2,
    BSDF_GLOSSY = 1 << 3,
    BSDF_SPECULAR = 1 << 4,
    BSDF_ALL = BSDF_DIFFUSE | BSDF_REFLECTION | BSDF_TRANSMISSION |
               BSDF_GLOSSY | BSDF_SPECULAR,
  };
  /// \param type scattering types
  BxDF(Type type);
  virtual ~BxDF() = default;
  /// \param t scattering types
  /// \return true if t is present
  bool matchesFlags(Type t) const;
  /// \param wo outgoing viewing direction
  /// \param wi incident light direction
  /// \return the value of the distribution function for the given pair of
  /// directions.
  virtual SpectrumOld f(const hermes::vec3 &wo, const hermes::vec3 &wi) const = 0;
  /// Computes the direction of the incident light given an outgoing direction
  /// \param wo **[in]** outgoing viewing direction
  /// \param wi **[out]**  computed incident light direction
  /// \param sample **[in]**
  /// \param pdf **[out]**
  /// \return the value of the distribution function for the given pair of
  /// directions.
  virtual SpectrumOld sample_f(const hermes::vec3 &wo, hermes::vec3 *wi,
                            const hermes::point2 &sample, real_t *pdf,
                            Type *sampledType = nullptr) const;
  /// Computes the hemispherical-directional reflectance that gives the total
  /// reflection in a given direction due to constant illumination over the
  /// hemisphere
  /// \param wo **[in]** outgoing direction
  /// \param nSamples number of samples
  /// \param samples hemisphere sample positions (only needed by some
  /// algorithms)
  /// \return SpectrumOld the value of the function
  virtual SpectrumOld rho(const hermes::vec3 &wo, int nSamples,
                       const hermes::point2 *samples) const;
  /// Computes the hemispherical-hemispherical reflectance that gives the
  /// fraction of incident light reflected by the surface when incident light is
  /// the same from all directions
  /// \param nSamples
  /// \param samples1
  /// \param samples2
  /// \return SpectrumOld
  virtual SpectrumOld rho(int nSamples, const hermes::point2 *samples1,
                       const hermes::point2 *samples2) const;

  const Type type; //!< scattering types
};

/// Wrapper for BxDF. Its spectra values are scaled. Usefull to combine
/// multiple materials.
class ScaledBxDF : public BxDF {
public:
  /// \param bxdf bxdf object
  /// \param scale spectra scale factor
  ScaledBxDF(BxDF *bxdf, const SpectrumOld &scale);
  ~ScaledBxDF() = default;
  /// \param wo outgoing viewing direction
  /// \param wi incident light direction
  /// \return the value of the distribution function for the given pair of
  /// directions scaled by **scale**.
  SpectrumOld f(const hermes::vec3 &wo, const hermes::vec3 &wi) const override;

private:
  BxDF *bxdf;     //!< BxDF object
  SpectrumOld scale; //!< spectra scale factor
};

/// Provides an interface for computing Scattering reflection coefficients.
/// _Fresnel equations_ describe the amount of light reflected from a surface
/// dependent on the direction.
class Scattering {
public:
  /// Computes the Scattering reflection formula for dielectric materials and
  /// unpolarized light.
  /// \param cosThetaI cosine of incident angle (formed with normal)
  /// \param etaI index of refraction for the incident media
  /// \param etaT index of refraction for the transmitted media
  /// \return real_t the Scattering reflectance value
  static real_t frDielectric(real_t cosThetaI, real_t etaI, real_t etaT);
  /// Computes the Scattering reflection formula for conductor materials. In this
  /// case, some incident light is potentially absorbed by the material and
  /// turned into heat presenting complex indices of refraction.
  /// \param cosThetaI cosine of incident angle (formed with normal)
  /// \param etaI index of refraction for the incident media
  /// \param etaT index of refraction for the transmitted media
  /// \param k absortion coefficient
  /// \return SpectrumOld
  static SpectrumOld frConductor(real_t cosThetaI, const SpectrumOld &etaI,
                              const SpectrumOld &etaT, const SpectrumOld &k);

public:
  /// Computes the amount of light reflected by the surface
  /// \param cosI cosine of incoming direction angle (with surface normal)
  /// \return SpectrumOld the amount of light reflected by the surface
  virtual SpectrumOld evaluate(real_t cosI) const = 0;
};

// Interface for conductor materials
class FresnelConductor : public Scattering {
public:
  /// \param etaI index of refraction for the incident media
  /// \param etaT index of refraction for the transmitted media
  /// \param k absortion coefficient
  FresnelConductor(const SpectrumOld &etaI, const SpectrumOld &etaT,
                   const SpectrumOld &k);
  /// Computes the amount of light reflected by the surface
  /// \param cosI cosine of incoming direction angle (with surface normal)
  /// \return SpectrumOld the amount of light reflected by the surface
  SpectrumOld evaluate(real_t cosI) const override;

private:
  SpectrumOld etaI; //!< index of refraction for the incident media
  SpectrumOld etaT; //!< index of refraction for the transmitted media
  SpectrumOld k;    //!< absortion coefficient
};

// Interface for dielectric materials
class FresnelDielectric : public Scattering {
public:
  /// \param etaI index of refraction for the incident media
  /// \param etaT index of refraction for the transmitted media
  FresnelDielectric(const real_t &etaI, const real_t &etaT);
  /// Computes the amount of light reflected by the surface
  /// \param cosI cosine of incoming direction angle (with surface normal)
  /// \return SpectrumOld the amount of light reflected by the surface
  SpectrumOld evaluate(real_t cosI) const override;

private:
  real_t etaI; //!< index of refraction for the incident media
  real_t etaT; //!< index of refraction for the transmitted media
};
*/
} // namespace helios

#endif // HELIOS_CORE_REFLECTION_H