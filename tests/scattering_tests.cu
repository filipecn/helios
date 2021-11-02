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
///\file shape_tests.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-08-19
///
///\brief

#include <catch2/catch.hpp>

#include <helios/scattering/bxdfs.h>
#include <helios/scattering/microfacet_distributions.h>
#include <helios/materials.h>
#include <helios/shapes.h>

using namespace helios;

TEST_CASE("interaction") {
  mem::init(1024);
  auto s = Shapes::create<Sphere>(mem::allocator(), Sphere::unitSphere());
  auto r = Ray({2, -1, 0}, hermes::normalize(hermes::vec3(-1, 1, 0)));
  ShapeIntersectionReturn si;
  CAST_SHAPE(s, ptr,
             auto isect = ptr->intersectQuadric(&s, r);
                 HERMES_LOG_VARIABLE((int) ((bool) isect))
                 HERMES_LOG_VARIABLE(isect->t_hit)
                 REQUIRE(ptr->intersectP(&s, r));
                 si = ptr->intersect(&s, r);
  )
  REQUIRE((bool) si);
  auto m = Materials::create<DielectricMaterial>(mem::allocator(), Spectrum(), false);

}

TEST_CASE("DielectricBxDF") {
  mem::init(1024);
  auto data = mem::allocate<DielectricBxDF>();
  auto bxdf = DielectricBxDF::createBxDF(data);
  CAST_BXDF(bxdf, ptr, /**/)
}

TEST_CASE("TrowbridgeReitzDistribution") {
  mem::init(1024);
  auto mfd = MicrofacetDistributions::create<TrowbridgeReitzDistribution>(mem::allocator());
  CAST_MICROFACET_DISTRIBUTION(mfd, ptr,
                               ptr->D(hermes::vec3(1.f, 1.f, 1.f));)
}
