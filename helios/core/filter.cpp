#include <helios/core/filter.h>

namespace helios {

HERMES_DEVICE_CALLABLE Filter::Filter() {}

HERMES_DEVICE_CALLABLE Filter::Filter(const hermes::vec2 &radius)
    : radius(radius), inv_radius(hermes::vec2(1 / radius.x, 1 / radius.y)) {}

HERMES_DEVICE_CALLABLE BoxFilter::BoxFilter(const hermes::vec2 &radius) : Filter(radius) {}

HERMES_DEVICE_CALLABLE real_t BoxFilter::evaluate(const hermes::point2 &p) const { return 1.; }

HERMES_DEVICE_CALLABLE BoxFilter::BoxFilter() {}

} // namespace helios
