#include <helios/geometry/ray.h>
#include <hermes/numeric/numeric.h>

using namespace hermes;

namespace helios {

HERMES_DEVICE_CALLABLE Ray::Ray() : max_t(hermes::Constants::real_infinity), time(0.f) {}

HERMES_DEVICE_CALLABLE Ray::Ray(const point3 &origin, const vec3 &direction, real_t tMax,
                                real_t time)
    : o(origin), d(direction), max_t(tMax), time(time) /*, medium(medium)*/ {}

HERMES_DEVICE_CALLABLE point3 Ray::operator()(real_t t) const { return o + d * t; }

hermes::MemoryDumper::RegionLayout Ray::memoryDumpLayout() {
  return MemoryDumper::RegionLayout().withSizeOf<Ray>()
      .withSubRegion(point3::memoryDumpLayout().withColor(ConsoleColors::red))
      .withSubRegion(vec3::memoryDumpLayout().withColor(ConsoleColors::green))
      .withSubRegion(
          MemoryDumper::RegionLayout()
              .withSizeOf<real_t>().withColor(ConsoleColors::blue).withTypeFrom<real_t>())
      .withSubRegion(
          MemoryDumper::RegionLayout()
              .withSizeOf<real_t>().withColor(ConsoleColors::magenta).withTypeFrom<real_t>());
}

HERMES_DEVICE_CALLABLE point3 Ray::offsetRayOrigin(const hermes::point3 &p, const hermes::vec3 &pError,
                                                   const hermes::normal3 &n, const hermes::vec3 &w) {
  real_t d = dot(abs(n), pError);
  vec3 offset = d * vec3(n);
  if (dot(w, n) != 0)
    offset = -offset;
  point3 po = p + offset;
  // round offset point away from p
  for (int i = 0; i < 3; i++)
    if (offset[i] < 0)
      po[i] = Numbers::nextFloatUp(po[i]);
    else if (offset[i] > 0)
      po[i] = Numbers::nextFloatDown(po[i]);
  return po;
}

HERMES_DEVICE_CALLABLE RayDifferential::RayDifferential() { has_differentials = false; }

HERMES_DEVICE_CALLABLE RayDifferential::RayDifferential(const point3 &origin, const vec3 &direction,
                                                        real_t tMax, real_t time)
    : ray(origin, direction, tMax, time) {
  has_differentials = false;
}

HERMES_DEVICE_CALLABLE void RayDifferential::scaleDifferentials(float s) {
  rx_origin = ray.o + (rx_origin - ray.o) * s;
  ry_origin = ray.o + (ry_origin - ray.o) * s;
  rx_direction = ray.d + (rx_direction - ray.d) * s;
  ry_direction = ray.d + (ry_direction - ray.d) * s;
}

hermes::MemoryDumper::RegionLayout RayDifferential::memoryDumpLayout() {
  return MemoryDumper::RegionLayout().withSizeOf<RayDifferential>()
      .withSubRegion(Ray::memoryDumpLayout())
      .withSubRegion(point3::memoryDumpLayout().withColor(ConsoleColors::cyan))
      .withSubRegion(point3::memoryDumpLayout().withColor(ConsoleColors::magenta))
      .withSubRegion(vec3::memoryDumpLayout().withColor(ConsoleColors::red))
      .withSubRegion(vec3::memoryDumpLayout().withColor(ConsoleColors::green))
      .withSubRegion(MemoryDumper::RegionLayout().withSizeOf<bool>().withTypeFrom<bool>());
}

} // namespace helios
