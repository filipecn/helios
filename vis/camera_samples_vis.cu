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
///\file camera_samples.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-30
///
///\brief

#include <circe/circe.h>
#include <helios/cameras/perspective_camera.h>
#include <helios/integrators/whitted_integrator.h>
#include <hermes/storage/array.h>
#include <helios/core/renderer.cuh>
#include <helios/core/scene.h>
#include <helios/accelerators/list.h>
#include <helios/common/io.h>
#include <hermes/logging/memory_dump.h>
#include <helios/core/shape.h>
#include <helios/shapes/sphere.h>
#include <helios/lights/point.h>

using namespace helios;
using namespace hermes;

struct CameraSamplesVis : public circe::gl::BaseApp {
  CameraSamplesVis() : BaseApp(800, 800) {
    //                                                                                                    setup scene
    // setup lights
    hermes::Array<Light> lights(1);
    lights[0] = PointLight::createLight(hermes::Transform());
    hermes::Array<PointLight> point_lights(1);
    point_lights[0] = PointLight(lights[0], Spectrum(0.5));
    hermes::DeviceArray<PointLight> dp_lights = point_lights;
    lights[0].light_data = dp_lights.data();

    // setup scene objects
    hermes::Array<helios::Sphere> spheres(1);
    spheres[0] = helios::Sphere::unitSphere();
    hermes::DeviceArray<helios::Sphere> d_spheres = spheres;
    hermes::Array<helios::Shape> shapes(1);
    shapes[0] = helios::Sphere::createShape(spheres[0], Transform());
    shapes[0].primitive_data = d_spheres.data();
    // setup aggregate
    ListAggregate list(shapes);
    // setup scene object
    Scene scene;
    scene.setAggregate(list.handle());
    scene.setLights(lights);

    // setup film_image
    BoxFilter filter({1, 1});
    FilmImage film_image(Film({16, 16}, &filter, 10));
    // setup camera
    PerspectiveCamera camera(AnimatedTransform(),
                             {{-1, -1}, {1, 1}}, film_image.film().full_resolution,
                             0, 1, 0, 1, 45);

    // setup renderer
    WhittedIntegrator integrator;
    SamplerRenderer renderer(range2(size2(16, 16)));
    // run
    renderer.render(camera, film_image, integrator, scene.view());
    // image
    auto image = film_image.imagePixels();
    HERMES_CHECK_EXP(io::save(image,
                              "render.png",
                              film_image.film().cropped_pixel_bounds,
                              film_image.film().full_resolution))

    {// dump camera
      MemoryDumper::dump(&camera,
                         1,
                         16,
                         PerspectiveCamera::memoryDumpLayout(),
                         memory_dumper_options::type_values | memory_dumper_options::colored_output);
    }
    {// dump samples
      HostMemory hm = renderer.debug_samples;
      renderer.debug_pool.setData(hm.ptr());
      SamplePool::dumpMemory(renderer.debug_pool, renderer.debug_render_info.n_tiles.total());
    }
    Array<RayDifferential> hm = renderer.debug_rays;
    MemoryDumper::dump(hm.data(), 4, sizeof(RayDifferential),
                       RayDifferential::memoryDumpLayout().withCount(4),
                       memory_dumper_options::colored_output | memory_dumper_options::type_values);

    // setup ray set
    // each ray is just a line segment in the mesh, and we draw with GL_LINES
    AoS aos;
    aos.pushField<vec3>("p");
//    aos.resize(hm.size().total() + 1); // origin comes in position 0
    aos.resize(4 * 16 + 1); // origin comes in position 0
    std::vector<i32> indices;

    auto pos = aos.field<vec3>("p");
    pos[0] = vec3();
    for (size_t i = 1; i < pos.size(); ++i) {
      pos[i] = hm[i - 1].ray.d;
      indices.emplace_back(0);
      indices.emplace_back(i);
    }

    circe::Model ray_set_m;
    ray_set_m = std::move(aos);
    ray_set_m = indices;
    ray_set_m.setPrimitiveType(GeometricPrimitiveType::LINES);

    ray_set = std::move(ray_set_m);

    Path shaders_path(std::string(SHADERS_PATH));
    if (!ray_set.program.link(shaders_path, "color"))
      hermes::Log::error("Failed to load model shader: " + ray_set.program.err);

    auto film_plane = film_image.film().physicalExtent();
    film_grid = circe::Shapes::plane(Plane::XY(),
                                     point3(film_plane.center().x, film_plane.center().y, 1e-2),
                                     vec3(film_plane.extends().x / 2, 0, 0), 16,
                                     circe::shape_options::wireframe);
    film_grid.program.link(shaders_path, "color");

  }
  void render(circe::CameraInterface *camera) override {
    glEnable(GL_DEPTH_TEST);
    // render model
    ray_set.program.use();
    ray_set.program.setUniform("view", camera->getViewTransform());
    ray_set.program.setUniform("model", ray_set.transform);
    ray_set.program.setUniform("projection", camera->getProjectionTransform());
    ray_set.program.setUniform("color", circe::Color::Black());
    ray_set.draw();

    film_grid.program.use();
    film_grid.program.setUniform("view", camera->getViewTransform());
    film_grid.program.setUniform("model", film_grid.transform);
    film_grid.program.setUniform("projection", camera->getProjectionTransform());
    film_grid.program.setUniform("color", circe::Color::Red());
    film_grid.draw();
  }

  circe::gl::SceneModel ray_set;
  circe::gl::SceneModel film_grid;
};

int main() {
  return CameraSamplesVis().run();
}