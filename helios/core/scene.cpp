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
#include <hermes/common/cuda_utils.h>

namespace helios {

HERMES_DEVICE_CALLABLE const bounds3 &Scene::View::worldBound() const {
  if (aggregate_.type == AggregateType::LIST)
    return reinterpret_cast<const ListAggregate::View *>(aggregate_.data_ptr.get())->worldBound();
  return {};
}

HERMES_DEVICE_CALLABLE bool Scene::View::intersect(const Ray &ray, SurfaceInteraction *isect) const {
  // TODO check ray direction not null
  if (aggregate_.type == AggregateType::LIST)
    return reinterpret_cast<const ListAggregate::View *>(aggregate_.data_ptr.get())->intersect(ray, isect);
  return false;
}

HERMES_DEVICE_CALLABLE bool Scene::View::intersectP(const Ray &ray) const {
  // TODO check ray direction not null
  if (aggregate_.type == AggregateType::LIST)
    return reinterpret_cast<const ListAggregate::View *>(aggregate_.data_ptr.get())->intersectP(ray);
  return false;
}

Scene::Scene() = default;

Scene::~Scene() = default;

HERMES_CUDA_KERNEL(updatePointers)(hermes::ArrayView<Light> a, hermes::StackAllocatorView m) {
  HERMES_CUDA_THREAD_INDEX_I
  a[i].data_ptr.update(m);
}

HERMES_CUDA_KERNEL(updatePointers)(hermes::ArrayView<Shape> a, hermes::StackAllocatorView m) {
  HERMES_CUDA_THREAD_INDEX_I
  a[i].data_ptr.update(m);
}

HERMES_CUDA_KERNEL(updatePointers)(hermes::ArrayView<Primitive> a, hermes::StackAllocatorView m) {
  HERMES_CUDA_THREAD_INDEX_I
  a[i].data_ptr.update(m);
  CAST_PRIMITIVE(a[i], ptr,
                 ptr->shape.data_ptr.update(m);)
}

HeResult Scene::prepare() {
  // chose default struct if none
  if (!aggregate_.data_ptr.get()) {
    aggregate_ = {
        .data_ptr = mem::allocate<ListAggregate>(),
        .type = AggregateType::LIST
    };
    aggregate_view_ = {
        .data_ptr = mem::allocate<ListAggregate::View>(),
        .type = AggregateType::LIST
    };
  }

  // send data to gpu
  d_lights_ = lights_;
  d_shapes_ = shapes_;
  d_primitives_ = primitives_;

  // setup acceleration structure
  reinterpret_cast<ListAggregate *>(aggregate_.data_ptr.get())->init(primitives_, d_primitives_.constView());
  *reinterpret_cast<ListAggregate::View *>(aggregate_view_.data_ptr.get()) =
      reinterpret_cast<ListAggregate *>(aggregate_.data_ptr.get())->view();

  // send resources memory to gpu
  mem::sendToGPU();

  // update pointers
  HERMES_CUDA_LAUNCH_AND_SYNC((d_lights_.size()), updatePointers_k, d_lights_.view(), mem::gpuView());
  HERMES_CUDA_LAUNCH_AND_SYNC((d_shapes_.size()), updatePointers_k, d_shapes_.view(), mem::gpuView());
  HERMES_CUDA_LAUNCH_AND_SYNC((d_primitives_.size()), updatePointers_k, d_primitives_.view(), mem::gpuView());
  aggregate_.data_ptr.update(mem::gpuView());
  aggregate_view_.data_ptr.update(mem::gpuView());

  return HeResult::SUCCESS;
}

Scene::View Scene::view() const {
  return View(aggregate_view_, d_lights_.view(), d_primitives_.view(), d_shapes_.view());
}

}