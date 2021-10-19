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

using namespace helios;

TEST_CASE("DielectricBxDF") {
  mem::init(1024);
  auto data = mem::allocate<DielectricBxDF>();
  auto bxdf = DielectricBxDF::createBxDF(data);
  CAST_BXDF(bxdf, ptr, /**/)
}

TEST_CASE("TrowbridgeReitzDistribution") {
  mem::init(1024);
  auto data = mem::allocate<TrowbridgeReitzDistribution>();
  auto mfd = TrowbridgeReitzDistribution::createMFD(data);
  CAST_MICROFACET_DISTRIBUTION(mfd, ptr,
                               ptr->D(hermes::vec3(1.f, 1.f, 1.f));)
}
