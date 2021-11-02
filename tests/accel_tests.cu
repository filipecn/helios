#include <catch2/catch.hpp>

#include <helios/lights/point.h>
#include <helios/core/scene.h>
#include <helios/shapes.h>
#include <hermes/common/cuda_utils.h>

using namespace helios;

HERMES_CUDA_KERNEL(checkListAggregate)(bool *r, Scene::View s) {
  HERMES_CUDA_RETURN_IF_NOT_THREAD_0
  Ray ray({0, 0, 0}, {1, 0, 0});
  s.intersectP(ray);
  *r = true;
}

TEST_CASE("ListAggregate") {
  mem::init(2048);

  // create data
  auto point_light_data = mem::allocate<PointLight>();
  auto sphere_shape_data = mem::allocate<Sphere>(Sphere::unitSphere());
  // create scene
  Scene scene;
  // add 3 identical lights with different positions
  scene.addLight(PointLight::createLight({1, 0, 0}, point_light_data));
  scene.addLight(PointLight::createLight({0, 1, 0}, point_light_data));
  scene.addLight(PointLight::createLight({0, 0, 1}, point_light_data));
  // add 10 primitives/shapes
  for (int i = 0; i < 10; ++i) {
    auto *sphere_shape = scene.addShape(
        Shapes::createFrom<Sphere>(sphere_shape_data, {1.f * i, 1, 1}, {1, 1, 1}));
    // add 2 geometrical primitives that use the same shape
    scene.addPrimitive(GeometricPrimitive::createPrimitive(sphere_shape));
  }
  // send everything to the GPU
  scene.prepare();

  // result
  hermes::UnifiedArray<bool> result(1);
  HERMES_CUDA_LAUNCH_AND_SYNC((1), checkListAggregate_k, result.data(), scene.view())
  REQUIRE(result[0]);
  std::cerr << mem::dumpMemory();
}