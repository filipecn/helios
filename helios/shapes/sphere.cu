#include <helios/shapes/sphere.h>
#include <hermes/numeric/numeric.h>
#include <hermes/numeric/e_float.h>
#include <helios/geometry/utils.h>
#include <helios/core/mem.h>

using namespace hermes;

namespace helios {

HERMES_DEVICE_CALLABLE Sphere::Sphere(real_t rad, real_t z0, real_t z1, real_t pm) : radius_{rad}, phi_max{pm} {
  zmin = Numbers::clamp(fminf(z0, z1), -radius_, radius_);
  radius_ = rad;
  zmax = Numbers::clamp(fmaxf(z0, z1), -radius_, radius_);
  theta_min = acosf(Numbers::clamp<real_t>(zmin / radius_, -1, 1));
  theta_max = acosf(Numbers::clamp<real_t>(zmin / radius_, -1, 1));
  theta_max = Trigonometry::degrees2radians(Numbers::clamp<real_t>(pm, 0, 360));
}

HERMES_DEVICE_CALLABLE bbox3 Sphere::objectBound() const {
  // TODO use phi_max to compute a tighter bound
  return bbox3(point3(-radius_, -radius_, zmin), point3(radius_, radius_, zmax));
}

HERMES_DEVICE_CALLABLE QuadricIntersectionReturn Sphere::intersectQuadric(const Shape *shape,
                                                                          const Ray &r,
                                                                          real_t t_max) const {
  real_t phi;
  hermes::point3 pHit;
  // Transform _Ray_ origin and direction to object space
  hermes::point3i oi = transform(shape->w2o, hermes::point3i(r.o.x, r.o.y, r.o.z));
  hermes::vec3i di = transform(shape->w2o, hermes::vec3i(r.d.x, r.d.y, r.d.z));

  // Solve quadratic equation to compute sphere _t0_ and _t1_
  hermes::Interval<real_t> t0, t1;
  // Compute sphere quadratic coefficients
  hermes::Interval<real_t> a = di.x.sqr() + di.y.sqr() + di.z.sqr();
  hermes::Interval<real_t> b = 2.f * (di.x * oi.x + di.y * oi.y + di.z * oi.z);
  hermes::Interval<real_t> c = oi.x.sqr() + oi.y.sqr() + oi.z.sqr() - hermes::Interval<real_t>(radius_).sqr();

  // Compute sphere quadratic discriminant _discrim_
  hermes::vec3i v(oi - b / (2.f * a) * di);
  hermes::Interval<real_t> length = v.length();
  hermes::Interval<real_t> discrim =
      4.f * a * (hermes::Interval<real_t>(radius_) + length) * (hermes::Interval<real_t>(radius_) - length);
  if (discrim.low < 0)
    return {};

  // Compute quadratic $t$ values
  hermes::Interval<real_t> rootDiscrim = discrim.sqrt();
  hermes::Interval<real_t> q;
  if ((real_t) b < 0)
    q = -.5f * (b - rootDiscrim);
  else
    q = -.5f * (b + rootDiscrim);
  t0 = q / a;
  t1 = c / q;
  // Swap quadratic $t$ values so that _t0_ is the lesser
  if (t0.low > t1.low)
    hermes::Numbers::swap(t0, t1);

  // Check quadric shape _t0_ and _t1_ for nearest intersection
  if (t0.high > t_max || t1.low <= 0)
    return {};
  hermes::Interval<real_t> tShapeHit = t0;
  if (tShapeHit.low <= 0) {
    tShapeHit = t1;
    if (tShapeHit.high > t_max)
      return {};
  }

  // Compute sphere hit position and $\phi$
  pHit = hermes::point3(oi) + (real_t) tShapeHit * hermes::vec3(di);
  // Refine sphere intersection point
  pHit *= radius_ / hermes::distance(pHit, hermes::point3(0, 0, 0));

  if (pHit.x == 0 && pHit.y == 0)
    pHit.x = 1e-5f * radius_;
  phi = std::atan2(pHit.y, pHit.x);
  if (phi < 0)
    phi += 2 * hermes::Constants::pi;

  // Test sphere intersection against clipping parameters
  if ((zmin > -radius_ && pHit.z < zmin) || (zmax < radius_ && pHit.z > zmax) ||
      phi > phi_max) {
    if (tShapeHit == t1)
      return {};
    if (t1.high > t_max)
      return {};
    tShapeHit = t1;
    // Compute sphere hit position and $\phi$
    pHit = hermes::point3(oi) + (real_t) tShapeHit * hermes::vec3(di);
    // Refine sphere intersection point
    pHit *= radius_ / hermes::distance(pHit, hermes::point3(0, 0, 0));

    if (pHit.x == 0 && pHit.y == 0)
      pHit.x = 1e-5f * radius_;
    phi = std::atan2(pHit.y, pHit.x);
    if (phi < 0)
      phi += 2 * hermes::Constants::pi;

    if ((zmin > -radius_ && pHit.z < zmin) || (zmax < radius_ && pHit.z > zmax) ||
        phi > phi_max)
      return {};
  }

  // Return _QuadricIntersection_ for sphere intersection
  return QuadricIntersection{real_t(tShapeHit), pHit, phi};
}

HERMES_DEVICE_CALLABLE SurfaceInteraction Sphere::interactionFromIntersection(const Shape *shape,
                                                                              const QuadricIntersection &isect,
                                                                              hermes::vec3 wo,
                                                                              real_t time) const {
  hermes::point3 pHit = isect.p_obj;
  real_t phi = isect.phi;
  // Find parametric representation of sphere hit
  real_t u = phi / phi_max;
  real_t cosTheta = pHit.z / radius_;
  real_t theta = hermes::Trigonometry::safe_acos(cosTheta);
  real_t v = (theta - theta_min) / (theta_max - theta_min);
  // Compute sphere $\dpdu$ and $\dpdv$
  real_t zRadius = std::sqrt(hermes::Numbers::sqr(pHit.x) + hermes::Numbers::sqr(pHit.y));
  real_t cosPhi = pHit.x / zRadius, sinPhi = pHit.y / zRadius;
  hermes::vec3 dpdu(-phi_max * pHit.y, phi_max * pHit.x, 0);
  real_t sinTheta = hermes::Numbers::safe_sqrt(1 - hermes::Numbers::sqr(cosTheta));
  hermes::vec3 dpdv = (theta_max - theta_min) *
      hermes::vec3(pHit.z * cosPhi, pHit.z * sinPhi, -radius_ * sinTheta);

  // Compute sphere $\dndu$ and $\dndv$
  hermes::vec3 d2Pduu = -phi_max * phi_max * hermes::vec3(pHit.x, pHit.y, 0);
  hermes::vec3 d2Pduv =
      (theta_max - theta_min) * pHit.z * phi_max * hermes::vec3(-sinPhi, cosPhi, 0.);
  hermes::vec3 d2Pdvv = -hermes::Numbers::sqr(theta_max - theta_min) * hermes::vec3(pHit.x, pHit.y, pHit.z);
  // Compute coefficients for fundamental forms
  real_t E = hermes::dot(dpdu, dpdu), F = hermes::dot(dpdu, dpdv), G = hermes::dot(dpdv, dpdv);
  hermes::vec3 n = hermes::normalize(hermes::cross(dpdu, dpdv));
  real_t e = hermes::dot(n, d2Pduu), f = hermes::dot(n, d2Pduv), g = hermes::dot(n, d2Pdvv);

  // Compute $\dndu$ and $\dndv$ from fundamental form coefficients
  real_t EGF2 = hermes::Numbers::differenceOfProducts(E, G, F, F);
  real_t invEGF2 = (EGF2 == 0) ? real_t(0) : 1 / EGF2;
  hermes::normal3 dndu =
      hermes::normal3((f * F - e * G) * invEGF2 * dpdu + (e * F - f * E) * invEGF2 * dpdv);
  hermes::normal3 dndv =
      hermes::normal3((g * F - f * G) * invEGF2 * dpdu + (f * F - g * E) * invEGF2 * dpdv);

  // Compute error bounds for sphere intersection
  hermes::vec3 pError = hermes::Numbers::gamma(5) * hermes::abs((hermes::vec3) pHit);

  // Return _SurfaceInteraction_ for quadric intersection
  bool flipNormal = HELIOS_MASK_BIT(shape->flags, shape_flags::REVERSE_ORIENTATION) ^
      HELIOS_MASK_BIT(shape->flags, shape_flags::TRANSFORM_SWAP_HANDEDNESS);
  hermes::vec3 woObject = shape->w2o(wo);
  return transform(shape->o2w, SurfaceInteraction(
      hermes::point3i(pHit, pError),
      hermes::point2(u, v), woObject, dpdu, dpdv,
      dndu, dndv, time, flipNormal));
}

HERMES_DEVICE_CALLABLE ShapeIntersectionReturn Sphere::intersect(const Shape *shape, const Ray &r, real_t t_max) const {
  auto isect = intersectQuadric(shape, r, t_max);
  if (!isect)
    return {};
  auto intr = interactionFromIntersection(shape, *isect, -r.d, r.time);
  return ShapeIntersection{intr, isect->t_hit};

  real_t phi;
  point3 phit;
  // transform HRay to object space
  vec3f oErr, dErr;
  Ray ray = transform(shape->w2o, r, oErr, dErr);
  //    initialize efloat ray coordinate valyes
  EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y), oz(ray.o.z, oErr.z);
  EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y), dz(ray.d.z, dErr.z);
  // compute quadritic Sphere coefficients
  EFloat a = dx * dx + dy * dy + dz * dz;
  EFloat b = 2 * (dx * ox + dy * oy + dz * oz);
  EFloat c = ox * ox + oy * oy + oz * oz - EFloat(radius_) * EFloat(radius_);
  // solve quadritic equation for t values
  EFloat t0, t1;
  if (!solve_quadratic(a, b, c, &t0, &t1))
    return {};
  // check quadric shape t0 and t1 for nearest intersection
  if (t0.upperBound() > ray.max_t || t1.lowerBound() <= 0)
    return {};
  EFloat thit = t0;
  if (thit.lowerBound() <= 0) {
    thit = t1;
    if (thit.upperBound() > ray.max_t)
      return {};
  }
  // compute Sphere hit position and phi
  phit = ray((real_t) thit);
  // refine sphere intersection point
  phit *= radius_ / distance(phit, point3());
  if (phit.x == 0 && phit.y == 0)
    phit.x = 1e-5f * radius_;
  phi = atan2(phit.y, phit.x);
  if (phi < 0.)
    phi += 2 * Constants::pi;
  // test Sphere intersection against clipping parameters
  if ((zmin > -radius_ && phit.z < zmin) || (zmax < radius_ && phit.z > zmax) ||
      phi > phi_max) {
    if (thit == t1)
      return {};
    if (t1.upperBound() > ray.max_t)
      return {};
    thit = t1;
    // compute Sphere hit position and phi
    phit = ray((real_t) thit);
    // refine sphere intersection point
    phit *= radius_ / distance(phit, point3());
    if (phit.x == 0 && phit.y == 0)
      phit.x = 1e-5f * radius_;
    phi = atan2(phit.y, phit.x);
    if (phi < 0.)
      phi += 2 * Constants::pi;
    if ((zmin > -radius_ && phit.z < zmin) || (zmax < radius_ && phit.z > zmax) ||
        phi > phi_max)
      return {};
  }
  // find parametric representation of Sphere hit
  real_t u = phi / phi_max;
  real_t theta = acosf(Numbers::clamp<real_t>(phit.z / radius_, -1, 1));
  real_t v = (theta - theta_min) / (theta_max - theta_min);
  // compute Sphere dp/du and dp/dv
  real_t zradius = sqrt(phit.x * phit.x + phit.y * phit.y);
  real_t invradius = 1.f / zradius;
  real_t cosphi = phit.x * invradius;
  real_t sinphi = phit.y * invradius;
  vec3f dpdu(-phi_max * phit.y, phi_max * phit.x, 0);
  vec3f dpdv = (theta_max - theta_min) * vec3(phit.z * cosphi, phit.z * sinphi,
                                              -radius_ * sin(theta));
  // compute Sphere dn/du and dn/dv
  vec3 d2Pduu = -phi_max * phi_max * vec3f(phit.x, phit.y, 0);
  vec3 d2Pduv =
      (theta_max - theta_min) * phit.z * phi_max * vec3f(-sinphi, cosphi, 0.f);
  vec3f d2Pdvv = -(theta_max - theta_min) * (theta_max - theta_min) *
      vec3f(phit.x, phit.y, phit.z);
  // compute coefficients for fundamental forms
  real_t E = dot(dpdu, dpdu);
  real_t F = dot(dpdu, dpdv);
  real_t G = dot(dpdv, dpdv);
  vec3f N = normalize(cross(dpdu, dpdv));
  real_t e = dot(N, d2Pduu);
  real_t f = dot(N, d2Pduv);
  real_t g = dot(N, d2Pdvv);
  // compute dndu and dndv from fundamental form coefficients
  real_t invEFG2 = 1 / (E * G - F * F);
  normal3f dndu((f * F - e * G) * invEFG2 * dpdu +
      (e * F - f * E) * invEFG2 * dpdv);
  normal3f dndv((g * F - f * G) * invEFG2 * dpdu +
      (f * F - g * E) * invEFG2 * dpdv);
  // compute error bounds for sphere intersection
  vec3f pError = Numbers::gamma(5) * abs((vec3f) phit);

  return ShapeIntersection{
      .interaction = transform(shape->o2w, SurfaceInteraction(phit, pError, point2f(u, v),
                                                              -ray.d, dpdu, dpdv, dndu, dndv,
                                                              ray.time)),
      .t_hit = (real_t) (thit)
  };
}

HERMES_DEVICE_CALLABLE bool Sphere::intersectP(const Shape *shape, const Ray &r, real_t t_max) const {
  return intersectQuadric(shape, r).hasValue();
  real_t phi;
  point3 phit;
  // transform HRay to object space
  vec3f oErr, dErr;
  Ray ray = transform(shape->w2o, r, oErr, dErr);
  //    initialize efloat ray coordinate valyes
  EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y), oz(ray.o.z, oErr.z);
  EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y), dz(ray.d.z, dErr.z);
  // compute quadratic Sphere coefficients
  EFloat a = dx * dx + dy * dy + dz * dz;
  EFloat b = 2 * (dx * ox + dy * oy + dz * oz);
  EFloat c = ox * ox + oy * oy + oz * oz - EFloat(radius_) * EFloat(radius_);
  // solve quadritic equation for t values
  EFloat t0, t1;
  if (!solve_quadratic(a, b, c, &t0, &t1))
    return false;
  HERMES_LOG_VARIABLE((float) t0.lowerBound())
  HERMES_LOG_VARIABLE((float) t0)
  HERMES_LOG_VARIABLE((float) t0.upperBound())
  HERMES_LOG_VARIABLE((float) t1.lowerBound())
  HERMES_LOG_VARIABLE((float) t1)
  HERMES_LOG_VARIABLE((float) t1.upperBound())
  // check quadric shape t0 and t1 for nearest intersection
  if (t0.upperBound() > ray.max_t || t1.lowerBound() <= 0)
    return false;
  EFloat thit = t0;
  if (thit.lowerBound() <= 0) {
    thit = t1;
    HERMES_LOG_VARIABLE((float) thit)
    HERMES_LOG_VARIABLE((float) thit.upperBound())
    HERMES_LOG_VARIABLE((float) thit.lowerBound())
    if (thit.upperBound() > ray.max_t)
      return false;
  }
  HERMES_LOG_VARIABLE((float) thit)
  // compute Sphere hit position and phi
  phit = ray((real_t) thit);
  // refine sphere intersection point
  phit *= radius_ / distance(phit, point3());
  if (phit.x == 0 && phit.y == 0)
    phit.x = 1e-5f * radius_;
  phi = atan2(phit.y, phit.x);
  if (phi < 0.)
    phi += 2 * Constants::pi;
  // test Sphere intersection against clipping parameters
  if ((zmin > -radius_ && phit.z < zmin) || (zmax < radius_ && phit.z > zmax) ||
      phi > phi_max) {
    if (thit == t1)
      return false;
    if (t1.upperBound() > ray.max_t)
      return false;
    thit = t1;
    // compute Sphere hit position and phi
    phit = ray((real_t) thit);
    // refine sphere intersection point
    phit *= radius_ / distance(phit, point3());
    if (phit.x == 0 && phit.y == 0)
      phit.x = 1e-5f * radius_;
    phi = atan2(phit.y, phit.x);
    if (phi < 0.)
      phi += 2 * Constants::pi;
    if ((zmin > -radius_ && phit.z < zmin) || (zmax < radius_ && phit.z > zmax) ||
        phi > phi_max)
      return false;
  }

  return true;
}

HERMES_DEVICE_CALLABLE real_t Sphere::surfaceArea() const { return phi_max * radius_ * (zmax - zmin); }

HERMES_DEVICE_CALLABLE real_t Sphere::radius() const {
  return radius_;
}

} // namespace helios
