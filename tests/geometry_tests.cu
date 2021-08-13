#include <catch2/catch.hpp>

#include <helios/geometry/ray.h>

using namespace helios;

TEST_CASE("Ray", "[geometry]") {
  SECTION("Sanity") {
    Ray ray({0, 0, 0}, {1, 0, 0});
    REQUIRE(ray.o == hermes::point3());
    REQUIRE(ray.d == hermes::vec3(1, 0, 0));
  } //
}

TEST_CASE("RayDifferential", "[geometry]") {
  SECTION("Sanity") {
  } //
}