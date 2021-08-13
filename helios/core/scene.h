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
///\file scene.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-08-10
///
///\brief

#ifndef HELIOS_HELIOS_CORE_SCENE_H
#define HELIOS_HELIOS_CORE_SCENE_H

#include <helios/core/interaction.h>
#include <helios/core/light.h>
#include <helios/geometry/bounds.h>
#include <helios/core/aggregate.h>
#include <hermes/storage/array.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                              Scene
// *********************************************************************************************************************
class Scene {
public:
  class View {
    friend class Scene;
  public:
    /// \return
    [[nodiscard]] HERMES_DEVICE_CALLABLE const bounds3 &worldBound() const;
    /// \param ray
    /// \param isect
    /// \return
    HERMES_DEVICE_CALLABLE bool intersect(const Ray &ray, SurfaceInteraction *isect) const;
    /// \param ray
    /// \return
    [[nodiscard]] HERMES_DEVICE_CALLABLE bool intersectP(const Ray &ray) const;
    //  HERMES_DEVICE_CALLABLE bool intersectTr(Ray ray,
    //  Sampler &sampler, SurfaceInteraction *isect, Spectrum *transmittance) const;
    hermes::ConstArrayView<Light> lights;
  private:
    View(const Aggregate &aggregate, const bounds3 &world_bounds, const hermes::ConstArrayView<Light> &lights)
        : aggregate_{aggregate},
          world_bounds_(world_bounds),
          lights(lights) {}
    Aggregate aggregate_;
    bounds3 world_bounds_;
  };
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  Scene();
  ~Scene();
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  View view() const;
  void setAggregate(const Aggregate &aggregate);
  void setLights(const hermes::Array<Light> &lights);
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
private:
  hermes::DeviceArray<Light> lights_;
  Aggregate aggregate_;
  bounds3 world_bounds_;
};

}

#endif //HELIOS_HELIOS_CORE_SCENE_H
