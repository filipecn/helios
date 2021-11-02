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
///\file whitted_integrator.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-08-06
///
///\brief

#ifndef HELIOS_HELIOS_INTEGRATORS_WHITTED_INTEGRATOR_H
#define HELIOS_HELIOS_INTEGRATORS_WHITTED_INTEGRATOR_H

#include <helios/geometry/ray.h>
#include <helios/base/spectrum.h>
#include <helios/core/scene.h>
#include <helios/lights/point.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                  WhittedIntegrator
// *********************************************************************************************************************
class WhittedIntegrator {
public:
  WhittedIntegrator() = default;

  template<class SamplerType, typename SceneType>
  HERMES_DEVICE_CALLABLE SpectrumOld
  Li(RayDifferential ray, const SceneType &scene, SamplerType &sampler) {
    SpectrumOld L(0.);
    // Find closest ray intersection or return background radiance
    auto si = scene.intersect(ray.ray);
    if (!si) {
      for (const auto &light : scene.lights) {
        if (light.value.type == LightType::POINT)
          L += light.value.data_ptr.template get<PointLight>()->Le(ray);
      }
      return L;
    }
    // Compute emitted and reflected light at ray intersection point
    // Initialize common variables for Whitted integrator
//    const hermes::normal3 &n = isect.shading.n;
//    hermes::vec3 wo = isect.interaction.wo;

    return SpectrumOld(1);
  }
  // *******************************************************************************************************************
  //                                                                                                   STATIC METHODS
  // *******************************************************************************************************************
  // *******************************************************************************************************************
  //                                                                                                 FRIEND FUNCTIONS
  // *******************************************************************************************************************
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  //                                                                                                       assignment
  // *******************************************************************************************************************
  //                                                                                                        OPERATORS
  // *******************************************************************************************************************
  //                                                                                                       assignment
  //                                                                                                       arithmetic
  //                                                                                                          boolean
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
};

}

#endif //HELIOS_HELIOS_INTEGRATORS_WHITTED_INTEGRATOR_H
