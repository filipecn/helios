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
///\file point.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-08-12
///
///\brief

#include <helios/lights/point.h>

using namespace hermes;

namespace helios {

Light PointLight::createLight(const hermes::Transform &l2w) {
  return {
      .light2world = l2w,
      .world2light = inverse(l2w),
      .type = LightType::POINT,
      .light_data = nullptr,
      .n_samples = 1,
      .flags =LightFlags::DELTA_POSITION
  };
}

PointLight::PointLight(const Light &light, const Spectrum &I) : p_light_(light.light2world(point3())), I_(I) {}

HERMES_DEVICE_CALLABLE Spectrum PointLight::sampleLi(const Interaction &ref,
                                                     const point2 &u,
                                                     hermes::vec3 *wi,
                                                     real_t *pdf,
                                                     VisibilityTester *vis) const {
  *wi = normalize(p_light_ - ref.p);
  *pdf = 1.f;
  *vis = VisibilityTester(ref, Interaction(p_light_, ref.time));
  return I_ / distance2(p_light_, ref.p);
}

HERMES_DEVICE_CALLABLE Spectrum PointLight::power() const {
  return 4 * Constants::pi * I_;
}

PointLight::~PointLight() = default;

}
