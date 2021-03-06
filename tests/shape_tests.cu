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

#include <helios/geometry/ray.h>
#include <helios/shapes.h>
#include <helios/shapes/intersection.h>

using namespace helios;

TEST_CASE("Sphere") {
  SECTION("shapes") {
    mem::init(1024);
    auto s = Shapes::create<Sphere>(mem::allocator(), Sphere::unitSphere());
    REQUIRE(s.bounds.lower == hermes::point3(-1, -1, -1));
    REQUIRE(s.bounds.upper == hermes::point3(1, 1, 1));
    auto s2 = Shapes::createFrom<Sphere>(s.data_ptr, {0, 0, 0}, {2, 2, 2});
    REQUIRE(s2.bounds.lower == hermes::point3(-2, -2, -2));
    REQUIRE(s2.bounds.upper == hermes::point3(2, 2, 2));
    auto r = Ray({-2, 0, 0}, {1, 0, 0});
    CAST_SHAPE(s, ptr,
               REQUIRE(ptr->intersectP(&s, r));
    )
  }//
}

TEST_CASE("bounds", "[geometry]") {
  Ray ray({0, 0, 0}, {1, 0, 0});
  bounds3 box{{2, -1, -1}, {4, 2, 2}};
  real_t h0, h1;
  REQUIRE(intersection::intersectP(box, ray, &h0, &h1));
}
