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
///\file core_tests.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-01
///
///\brief

#include <catch2/catch.hpp>

#include <hermes/common/cuda_utils.h>
#include <helios/shapes/sphere.h>
#include <hermes/storage/array.h>
#include <helios/core/spectrum.h>
#include <helios/core/film.h>
#include <helios/core/scene.h>

using namespace helios;

HERMES_CUDA_KERNEL(testShape)(bool *r, hermes::ArrayView<Shape> shapes) {
  HERMES_CUDA_RETURN_IF_NOT_THREAD_0
  printf("%f\n", reinterpret_cast<Sphere *>(shapes[0].primitive_data)->surfaceArea());
  *r = true;
}

TEST_CASE("Shapes and Primitives", "[core]") {
  // primitives
  hermes::Array<Sphere> sphere(1);
  sphere[0] = Sphere::unitSphere();
  REQUIRE(sphere[0].objectBound().lower == hermes::point3(-1, -1, -1));
  REQUIRE(sphere[0].objectBound().upper == hermes::point3(1, 1, 1));
  REQUIRE(sphere[0].surfaceArea() == Approx(4 * hermes::Constants::pi * 1 * 1));
  // copy primitives to device memory
  hermes::DeviceArray<Sphere> d_sphere = sphere;

  // shapes
  hermes::Array<Shape> shapes(1);
  shapes[0] = {
      .o2w = hermes::Transform(),
      .w2o = hermes::Transform(),
      .bounds = sphere[0].objectBound(),
      .primitive_data = d_sphere.data(),
      .type = ShapeType::SPHERE,
      .flags = shape_flags::NONE
  };
  // copy shapes to device memory
  hermes::DeviceArray<Shape> d_shapes = shapes;
  // result
  hermes::UnifiedArray<bool> result(1);
  result[0] = false;
  HERMES_CUDA_LAUNCH_AND_SYNC((1), testShape_k, result.data(), d_shapes.view())
  REQUIRE(result[0]);
}

TEST_CASE("Scene") {
  Scene
}

TEST_CASE("film") {
  BoxFilter filter({1, 1});
  Film film({16, 16}, &filter, 10);
  REQUIRE(film.full_resolution == hermes::size2(16, 16));
  REQUIRE(film.diagonal == Approx(10 / 1000.0));
  REQUIRE(film.physicalExtent().extends().length() == Approx(0.01));
}

TEST_CASE("Spectrum", "[core]") {
  Spectrum s;
  REQUIRE(s.isBlack());
}