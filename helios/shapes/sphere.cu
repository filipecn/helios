#include <helios/shapes/sphere.h>
#include <hermes/numeric/numeric.h>
#include <hermes/numeric/e_float.h>
#include <helios/geometry/utils.h>

using namespace hermes;

namespace helios {

Shape Sphere::createShape(const Sphere &sphere, const hermes::Transform &o2w) {
  return {
      .o2w = o2w,
      .w2o = inverse(o2w),
      .bounds = o2w(sphere.objectBound()),
      .primitive_data = nullptr,
      .type = ShapeType::SPHERE,
      .flags = shape_flags::NONE
  };
}

HERMES_DEVICE_CALLABLE Sphere::Sphere(real_t rad, real_t z0, real_t z1, real_t pm) : radius{rad}, phi_max{pm} {
  zmin = Numbers::clamp(fminf(z0, z1), -radius, radius);
  radius = rad;
  zmax = Numbers::clamp(fmaxf(z0, z1), -radius, radius);
  theta_min = acosf(Numbers::clamp<real_t>(zmin / radius, -1, 1));
  theta_max = acosf(Numbers::clamp<real_t>(zmin / radius, -1, 1));
  theta_max = Trigonometry::degrees2radians(Numbers::clamp<real_t>(pm, 0, 360));
}

HERMES_DEVICE_CALLABLE bbox3 Sphere::objectBound() const {
  // TODO use phi_max to compute a tighter bound
  return bbox3(point3(-radius, -radius, zmin), point3(radius, radius, zmax));
}

HERMES_DEVICE_CALLABLE bool Sphere::intersect(const Shape *shape, const Ray &r, real_t *tHit, SurfaceInteraction *isect,
                                              bool test_alpha_texture) const {
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
  EFloat c = ox * ox + oy * oy + oz * oz - EFloat(radius) * EFloat(radius);
  // solve quadritic equation for t values
  EFloat t0, t1;
  if (!solve_quadratic(a, b, c, &t0, &t1))
    return false;
  // check quadric shape t0 and t1 for nearest intersection
  if (t0.upperBound() > ray.max_t || t1.lowerBound() <= 0)
    return false;
  EFloat thit = t0;
  if (thit.lowerBound() <= 0) {
    thit = t1;
    if (thit.upperBound() > ray.max_t)
      return false;
  }
  // compute Sphere hit position and phi
  phit = ray((real_t) thit);
  // refine sphere intersection point
  phit *= radius / distance(phit, point3());
  if (phit.x == 0 && phit.y == 0)
    phit.x = 1e-5f * radius;
  phi = atan2(phit.y, phit.x);
  if (phi < 0.)
    phi += 2 * Constants::pi;
  // test Sphere intersection against clipping parameters
  if ((zmin > -radius && phit.z < zmin) || (zmax < radius && phit.z > zmax) ||
      phi > phi_max) {
    if (thit == t1)
      return false;
    if (t1.upperBound() > ray.max_t)
      return false;
    thit = t1;
    // compute Sphere hit position and phi
    phit = ray((real_t) thit);
    // refine sphere intersection point
    phit *= radius / distance(phit, point3());
    if (phit.x == 0 && phit.y == 0)
      phit.x = 1e-5f * radius;
    phi = atan2(phit.y, phit.x);
    if (phi < 0.)
      phi += 2 * Constants::pi;
    if ((zmin > -radius && phit.z < zmin) || (zmax < radius && phit.z > zmax) ||
        phi > phi_max)
      return false;
  }
  // find parametric representation of Sphere hit
  real_t u = phi / phi_max;
  real_t theta = acosf(Numbers::clamp<real_t>(phit.z / radius, -1, 1));
  real_t v = (theta - theta_min) / (theta_max - theta_min);
  // compute Sphere dp/du and dp/dv
  real_t zradius = sqrt(phit.x * phit.x + phit.y * phit.y);
  real_t invradius = 1.f / zradius;
  real_t cosphi = phit.x * invradius;
  real_t sinphi = phit.y * invradius;
  vec3f dpdu(-phi_max * phit.y, phi_max * phit.x, 0);
  vec3f dpdv = (theta_max - theta_min) * vec3(phit.z * cosphi, phit.z * sinphi,
                                              -radius * sin(theta));
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
  // initialize SurfaceInteraction from parametric information
  *isect = transform(shape->o2w, SurfaceInteraction(phit, pError, point2f(u, v),
                                                   -ray.d, dpdu, dpdv, dndu, dndv,
                                                   ray.time, shape));
  // update tHit for quadric intersection
  *tHit = (real_t) (thit);
  return true;
}

HERMES_DEVICE_CALLABLE bool Sphere::intersectP(const Shape &shape, const Ray &r, bool test_alpha_texture) const {
  real_t phi;
  point3 phit;
  // transform HRay to object space
  vec3f oErr, dErr;
  Ray ray = transform(shape.w2o, r, oErr, dErr);
  //    initialize efloat ray coordinate valyes
  EFloat ox(ray.o.x, oErr.x), oy(ray.o.y, oErr.y), oz(ray.o.z, oErr.z);
  EFloat dx(ray.d.x, dErr.x), dy(ray.d.y, dErr.y), dz(ray.d.z, dErr.z);
  // compute quadritic Sphere coefficients
  EFloat a = dx * dx + dy * dy + dz * dz;
  EFloat b = 2 * (dx * ox + dy * oy + dz * oz);
  EFloat c = ox * ox + oy * oy + oz * oz - EFloat(radius) * EFloat(radius);
  // solve quadritic equation for t values
  EFloat t0, t1;
  if (!solve_quadratic(a, b, c, &t0, &t1))
    return false;
  // check quadric shape t0 and t1 for nearest intersection
  if (t0.upperBound() > ray.max_t || t1.lowerBound() <= 0)
    return false;
  EFloat thit = t0;
  if (thit.lowerBound() <= 0) {
    thit = t1;
    if (thit.upperBound() > ray.max_t)
      return false;
  }
  // compute Sphere hit position and phi
  phit = ray((real_t) thit);
  // refine sphere intersection point
  phit *= radius / distance(phit, point3());
  if (phit.x == 0 && phit.y == 0)
    phit.x = 1e-5f * radius;
  phi = atan2(phit.y, phit.x);
  if (phi < 0.)
    phi += 2 * Constants::pi;
  // test Sphere intersection against clipping parameters
  if ((zmin > -radius && phit.z < zmin) || (zmax < radius && phit.z > zmax) ||
      phi > phi_max) {
    if (thit == t1)
      return false;
    if (t1.upperBound() > ray.max_t)
      return false;
    thit = t1;
    // compute Sphere hit position and phi
    phit = ray((real_t) thit);
    // refine sphere intersection point
    phit *= radius / distance(phit, point3());
    if (phit.x == 0 && phit.y == 0)
      phit.x = 1e-5f * radius;
    phi = atan2(phit.y, phit.x);
    if (phi < 0.)
      phi += 2 * Constants::pi;
    if ((zmin > -radius && phit.z < zmin) || (zmax < radius && phit.z > zmax) ||
        phi > phi_max)
      return false;
  }

  return true;
}

HERMES_DEVICE_CALLABLE real_t Sphere::surfaceArea() const { return phi_max * radius * (zmax - zmin); }

} // namespace helios
