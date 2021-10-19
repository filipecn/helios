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
#include <helios/base/light.h>
#include <helios/geometry/bounds.h>
#include <helios/base/aggregate.h>
#include <helios/accelerators/list.h>
#include <hermes/storage/array.h>
#include <helios/base/primitive.h>

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
    /// \return
    [[nodiscard]] HERMES_DEVICE_CALLABLE ShapeIntersectionReturn intersect(const Ray &ray) const;
    /// \param ray
    /// \return
    [[nodiscard]] HERMES_DEVICE_CALLABLE bool intersectP(const Ray &ray) const;
    //  HERMES_DEVICE_CALLABLE bool intersectTr(Ray ray,
    //  Sampler &sampler, SurfaceInteraction *isect, SpectrumOld *transmittance) const;
    hermes::ConstArrayView<Light> lights;
    hermes::ConstArrayView<Primitive> primitives;
    hermes::ConstArrayView<Shape> shapes;
  private:
    View(const Aggregate &aggregate,
         const hermes::ConstArrayView<Light> &lights,
         const hermes::ConstArrayView<Primitive> primitives,
         const hermes::ConstArrayView<Shape> shapes)
        : lights(lights),
          shapes(shapes),
          primitives(primitives),
          aggregate_{aggregate} {}
    Aggregate aggregate_;
  };
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  Scene();
  ~Scene();
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  //                                                                                                       gpu access
  HeResult prepare();
  ///
  /// \return
  View view() const;
  ///
  /// \param aggregate
  template<class A, class... P>
  void setAggregate(P &&... params) {
    if (std::is_same_v<A, ListAggregate>)
      aggregate_.type = AggregateType::LIST;
    aggregate_.data_ptr = mem::allocate<A>(std::forward<P>(params)...);
  }
  //                                                                                                   scene elements
  /// \tparam P
  /// \param params
  /// \return
  template<class... P>
  Light *addLight(P &&... params) {
    lights_.emplace_back(std::forward<P>(params)...);
    return &lights_[lights_.size() - 1];
  }
  /// \tparam P
  /// \param params
  /// \return
  template<class... P>
  Shape *addShape(P &&... params) {
    shapes_.emplace_back(std::forward<P>(params)...);
    return &shapes_[shapes_.size() - 1];
  }
  /// \tparam P
  /// \param params
  /// \return
  template<class... P>
  Primitive *addPrimitive(P &&... params) {
    primitives_.emplace_back(std::forward<P>(params)...);
    return &primitives_[primitives_.size() - 1];
  }

private:
  // CPU scene elements
  std::vector<Light> lights_;
  std::vector<Shape> shapes_;
  std::vector<Primitive> primitives_;
  // GPU scene elements
  hermes::DeviceArray<Light> d_lights_;
  hermes::DeviceArray<Shape> d_shapes_;
  hermes::DeviceArray<Primitive> d_primitives_;
  // Acceleration struct
  Aggregate aggregate_;
  Aggregate aggregate_view_;
};

}

#endif //HELIOS_HELIOS_CORE_SCENE_H
