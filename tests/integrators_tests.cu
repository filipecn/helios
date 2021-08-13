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

#include <helios/integrators/sampler_integrator.cuh>
#include <helios/cameras/perspective_camera.h>
#include <hermes/storage/array.h>

using namespace helios;
using namespace hermes;

TEST_CASE("SamplerRenderer") {
  SECTION("sanity") {
    // setup film
    BoxFilter filter({1, 1});
    Film film({16, 16}, &filter, 10);
    // setup camera
    range2 screen_window(size2(16, 16));
    UnifiedArray<PerspectiveCamera> camera_m(1);
    camera_m[0] = PerspectiveCamera(AnimatedTransform(),
                                    screen_window, film.full_resolution,
                                    0, 1, 10, 1, 45);
    // setup integrator
    SamplerRenderer integrator(range2(size2(16, 16)));
    // run
//    integrator.render(camera_m[0].handle(), film);
  }//
}
