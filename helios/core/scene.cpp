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
///\file scene.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-08-12
///
///\brief

#include <helios/core/scene.h>
#include <helios/accelerators/list.h>

namespace helios {

HERMES_DEVICE_CALLABLE const bounds3 &Scene::View::worldBound() const {
  return world_bounds_;
}

HERMES_DEVICE_CALLABLE bool Scene::View::intersect(const Ray &ray, SurfaceInteraction *isect) const {
  if (!aggregate_.aggregate)
    return false;
  // TODO check ray direction not null
  if (aggregate_.type == AggregateType::LIST)
    return reinterpret_cast<ListAggregate::View *>(aggregate_.aggregate)->intersect(ray, isect);
  return false;
}

HERMES_DEVICE_CALLABLE bool Scene::View::intersectP(const Ray &ray) const {
  if (!aggregate_.aggregate)
    return false;
  // TODO check ray direction not null
  if (aggregate_.type == AggregateType::LIST)
    return reinterpret_cast<ListAggregate::View *>(aggregate_.aggregate)->intersectP(ray);
  return false;
}

Scene::Scene() {

}

Scene::~Scene() {

}

Scene::View Scene::view() const {
  return View(aggregate_, world_bounds_, lights_.view());
}

void Scene::setAggregate(const Aggregate &aggregate) {
  aggregate_ = aggregate;
}

void Scene::setLights(const hermes::Array<Light> &lights) {
  lights_ = lights;
}

}