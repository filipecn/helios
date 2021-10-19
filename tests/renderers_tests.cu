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
///\file integrators_tests.cu
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-15
///
///\brief

#include <catch2/catch.hpp>

#include <helios/cameras/perspective_camera.h>
#include <helios/core/filter.h>
#include <helios/core/film.h>
#include <helios/integrators/whitted_integrator.h>
#include <helios/core/renderer.cuh>
#include <helios/shapes.h>
#include <helios/common/io.h>

using namespace helios;

TEST_CASE("SamplerRenderer") {
  mem::init(2048);
  // setup resources
  auto point_light_data = mem::allocate<PointLight>();
  auto sphere_shape_data = mem::allocate<Sphere>(Sphere::unitSphere());
  // setup scene
  Scene scene;
  scene.addLight(PointLight::createLight({-10, 0, 0}, point_light_data));
  auto sphere_shape = scene.addShape(Sphere::createShape(sphere_shape_data, {0, 0, 5}));
  scene.addPrimitive(GeometricPrimitive::createPrimitive(sphere_shape));
  REQUIRE(scene.prepare() == HeResult::SUCCESS);
  // image resolution
  hermes::size2 res(1024, 1024);
  // setup film_image
  BoxFilter filter({1, 1});
  FilmImage film_image(Film(res, &filter, 10));
  // setup camera
  PerspectiveCamera camera(AnimatedTransform(),
                           {{-1, -1}, {1, 1}},
                           film_image.film().full_resolution,
                           0, 1, 0, 1, 45);
  // setup renderer
  WhittedIntegrator integrator;
  SamplerRenderer renderer((hermes::range2(res)));
  // run
  renderer.render(camera, film_image, integrator, scene.view());
  // image
  auto image = film_image.imagePixels();
  REQUIRE(io::save(image,
                   "render.png",
                   film_image.film().cropped_pixel_bounds,
                   film_image.film().full_resolution));
}
