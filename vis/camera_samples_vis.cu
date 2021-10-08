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

struct CameraSamplesVis : public circe::gl::BaseApp {
  CameraSamplesVis() : BaseApp(800, 800) {
    //                                                                                                    setup scene

    mem::init(2048);
    // setup resources
    auto point_light_data = mem::allocate<PointLight>();
    auto sphere_shape_data = mem::allocate<Sphere>(Sphere::unitSphere());
    // setup scene
    Scene scene;
    scene.addLight(PointLight::createLight({-10, 0, 0}, point_light_data));
    auto sphere_shape = scene.addShape(Sphere::createShape(sphere_shape_data, {0, 0, 5}));
    scene.addPrimitive(GeometricPrimitive::createPrimitive(sphere_shape));
    scene.prepare();
    // image resolution
    hermes::size2 res(1024, 1024);
    // setup film_image
    BoxFilter filter({1, 1});
    FilmImage film_image(Film(res, &filter, 10));
    // setup camera
    PerspectiveCamera camera(AnimatedTransform(),
                             {{-1, -1}, {1, 1}}, film_image.film().full_resolution,
                             0, 1, 0, 1, 45);
    // setup renderer
    WhittedIntegrator integrator;
    SamplerRenderer renderer((hermes::range2(res)));
    // run
    renderer.render(camera, film_image, integrator, scene.view());
    // image
    auto image = film_image.imagePixels();
    HERMES_CHECK_EXP(io::save(image,
                              "render.png",
                              film_image.film().cropped_pixel_bounds,
                              film_image.film().full_resolution))

    exit(0);
    {// dump camera
      hermes::MemoryDumper::dump(&camera,
                                 1,
                                 16,
                                 PerspectiveCamera::memoryDumpLayout(),
                                 hermes::memory_dumper_options::type_values
                                     | hermes::memory_dumper_options::colored_output);
    }

#ifdef HELIOS_DEBUG_DATA
    {// dump samples
      HostMemory hm = renderer.debug_samples;
      renderer.debug_pool.setData(hm.ptr());
      SamplePool::dumpMemory(renderer.debug_pool, renderer.debug_render_info.n_tiles.total());
    }
    Array<RayDifferential> hm;// = renderer.debug_rays;
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
      pos[i] = hm[i - 1].ray.d * 40;
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

    sphere = circe::Shapes::icosphere(4, circe::shape_options::normal);
    sphere.transform = hermes::Transform::translate({0, 0, 5});
    sphere.program.link(shaders_path, "color");
#endif
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

    sphere.program.use();
    sphere.program.setUniform("view", camera->getViewTransform());
    sphere.program.setUniform("model", sphere.transform);
    sphere.program.setUniform("projection", camera->getProjectionTransform());
    sphere.program.setUniform("color", circe::Color::Red(0.2));
    sphere.draw();
  }

  circe::gl::SceneModel ray_set;
  circe::gl::SceneModel film_grid;
  circe::gl::SceneModel sphere;
};

int main() {
  return CameraSamplesVis().run();
}