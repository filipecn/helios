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
#include <hermes/storage/stack_allocator.h>
#include <helios/lights/point.h>
#include <helios/core/mem.h>
#include <helios/core/scene.h>
#include <helios/core/primitive.h>
#include <helios/shapes/shapes.h>

using namespace helios;

#define CUDA_REQUIRE(A) if(!(A)) return;

HERMES_CUDA_KERNEL(testShape)(bool *r, hermes::ArrayView<Shape> shapes) {
  HERMES_CUDA_RETURN_IF_NOT_THREAD_0
//  printf("%f\n", reinterpret_cast<Sphere *>(shapes[0].shape_data)->surfaceArea());
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
//  shapes[0] = Sphere::createShape();{
//      .o2w = hermes::Transform(),
//      .w2o = hermes::Transform(),
//      .bounds = sphere[0].objectBound(),
//      .shape_data = d_sphere.data(),
//      .type = ShapeType::SPHERE,
//      .flags = shape_flags::NONE
//  };
  // copy shapes to device memory
  hermes::DeviceArray<Shape> d_shapes = shapes;
  // result
  hermes::UnifiedArray<bool> result(1);
  result[0] = false;
  HERMES_CUDA_LAUNCH_AND_SYNC((1), testShape_k, result.data(), d_shapes.view())
  REQUIRE(result[0]);
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

TEST_CASE("Raw Scene") {
  mem::init(1024);

  // array of scene elements
  hermes::Array<Light> lights(5);
  hermes::Array<Shape> shapes(5);
  hermes::Array<Primitive> primitives(5);

  // create data
  auto point_light_data = mem::allocate<PointLight>();
  auto sphere_shape_data = mem::allocate<Sphere>(Sphere::unitSphere());

  // create 5 identical lights
  for (int i = 0; i < 5; ++i)
    lights[i] = PointLight::createLight({}, point_light_data);

  // create 5 identical shapes (sphere)
  for (int i = 0; i < 5; ++i)
    shapes[i] = Sphere::createShape(hermes::Transform(), sphere_shape_data);

  // create 1 geometrical primitive for each shape
  for (int i = 0; i < 5; ++i) {
    auto geo_prim_data = mem::allocate<GeometricPrimitive>(shapes[i]);
    primitives[i] = GeometricPrimitive::createPrimitive(geo_prim_data);
  }

  // check storage usage
  REQUIRE(mem::availableSize() == 1024
      - sizeof(PointLight)
      - sizeof(Sphere)
      - 5 * sizeof(GeometricPrimitive)
  );

}

HERMES_CUDA_KERNEL(checkSceneElements)(bool *r, Scene::View s) {
  HERMES_CUDA_RETURN_IF_NOT_THREAD_0
  *r = false;
  CUDA_REQUIRE(s.lights.size().total() == 3)
  CUDA_REQUIRE(s.primitives.size().total() == 2)
  CUDA_REQUIRE(s.shapes.size().total() == 1)
  // check lights
  CUDA_REQUIRE(s.lights[0].type == LightType::POINT)
  CUDA_REQUIRE(s.lights[0].light2world(hermes::point3()) == hermes::point3(1, 0, 0))
  CUDA_REQUIRE(s.lights[1].type == LightType::POINT)
  CUDA_REQUIRE(s.lights[1].light2world(hermes::point3()) == hermes::point3(0, 1, 0))
  CUDA_REQUIRE(s.lights[2].type == LightType::POINT)
  CUDA_REQUIRE(s.lights[2].light2world(hermes::point3()) == hermes::point3(0, 0, 1))
  // check shapes
  CUDA_REQUIRE(s.shapes[0].type == ShapeType::SPHERE)
  CAST_CONST_SHAPE(s.shapes[0], ptr,
                   CUDA_REQUIRE(ptr->radius() == 1.);
  )
  // check primitives
  CUDA_REQUIRE(s.primitives[0].type == PrimitiveType::GEOMETRIC_PRIMITIVE)
  CAST_CONST_PRIMITIVE(s.primitives[0], ptr,
                       CUDA_REQUIRE(ptr->shape.type == ShapeType::SPHERE)
                           CAST_CONST_SHAPE(ptr->shape, sptr,
                                            CUDA_REQUIRE(sptr->radius() == 1.);
                       )
  )
  CUDA_REQUIRE(s.primitives[1].type == PrimitiveType::GEOMETRIC_PRIMITIVE)
  CAST_CONST_PRIMITIVE(s.primitives[1], ptr,
                       CUDA_REQUIRE(ptr->shape.type == ShapeType::SPHERE)
                           CAST_CONST_SHAPE(ptr->shape, sptr,
                                            CUDA_REQUIRE(sptr->radius() == 1.);
                       )
  )
  *r = true;
}

TEST_CASE("Scene") {
  mem::init(1024);

  // create data
  auto point_light_data = mem::allocate<PointLight>();
  auto sphere_shape_data = mem::allocate<Sphere>(Sphere::unitSphere());
  // create scene
  Scene scene;
  // add 3 identical lights with different positions
  scene.addLight(PointLight::createLight({1, 0, 0}, point_light_data));
  scene.addLight(PointLight::createLight({0, 1, 0}, point_light_data));
  scene.addLight(PointLight::createLight({0, 0, 1}, point_light_data));
  // add 1 shape
  auto *sphere_shape = scene.addShape(Sphere::createShape(hermes::Transform(), sphere_shape_data));
  // add 2 geometrical primitives that use the same shape
  scene.addPrimitive(GeometricPrimitive::createPrimitive(sphere_shape));
  scene.addPrimitive(GeometricPrimitive::createPrimitive(sphere_shape));
  // send everything to the GPU
  scene.prepare();

  // result
  hermes::UnifiedArray<bool> result(1);
  HERMES_CUDA_LAUNCH_AND_SYNC((1), checkSceneElements_k, result.data(), scene.view())
  REQUIRE(result[0]);
}