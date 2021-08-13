//
// Created by filipecn on 15/07/2021.
//


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
///\file sampling_tests.cu
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-15
///
///\brief

#include <catch2/catch.hpp>

#include <hermes/common/cuda_utils.h>
#include <hermes/storage/array.h>
#include <hermes/logging/memory_dump.h>
#include <helios/samplers/sample_pool.h>
#include <helios/samplers/stratified_sampler.h>

using namespace helios;

HERMES_CUDA_KERNEL(check_pool)(SamplePool pool, int *result) {
  HERMES_CUDA_RETURN_IF_NOT_THREAD_0
  pool.reset();
  *result = 0;
  do {
    for (int d = 0; d < 3; ++d) {
      if (!hermes::Check::is_equal(pool.get1DSample(), static_cast<real_t>(pool.currentSampleIndex() * 10 + d)))
        *result = -2;
      if (pool.get2DSample() != hermes::point2(pool.currentSampleIndex(), d))
        *result = -3;
    }
    if (pool.has1D() || pool.has2D())
      *result = -1;

    for (size_t a = 0; a < 1; ++a) {
      auto *array = pool.get1DArray();
      if (array == nullptr) {
        *result = -4;
        continue;
      }
      for (size_t i = 0; i < pool.get1DArraySize(a); ++i)
        if (!hermes::Check::is_equal(array[i], static_cast<real_t>(a * 10 + i + 1)))
          *result = -5;
    }
    for (size_t a = 0; a < 2; ++a) {
      auto *array = pool.get2DArray();
      if (array == nullptr) {
        *result = -6;
        continue;
      }
      for (size_t i = 0; i < pool.get2DArraySize(a); ++i)
        if (array[i] != hermes::point2(pool.currentSampleIndex(), a * 10 + i + 1))
          *result = -7;
    }
  } while (pool.startNextSample());
}

HERMES_CUDA_KERNEL(fill_pool)(SamplePool pool, int bounds, int *result) {
  HERMES_CUDA_THREAD_INDEX_I_LT(bounds)
  pool.setPoolIndex(i);
  pool.reset();
  *result = 0;
  do {
    for (int d = 0; d < 3; ++d) {
      pool.get1DSample() = i * 1000 + pool.currentSampleIndex() * 10 + d;
      pool.get2DSample() = hermes::point2(i * 10 + pool.currentSampleIndex(), 10 + d);
    }
    if (pool.has1D() || pool.has2D())
      *result = -1;
  } while (pool.startNextSample());
}

TEST_CASE("SamplePool", "[sampling]") {
  SECTION("Descriptor") {
    SECTION("no arrays") {
      SamplePool::PoolDescriptor descriptor = {
          .samples_per_pixel = 10,
          .dimensions = 3,
          .array1_sizes = {},
          .array2_sizes = {}
      };
      REQUIRE(descriptor.memory_size_in_bytes() == 10 * 3 * (sizeof(real_t) + sizeof(hermes::point2)));
    }//
    SECTION("with arrays") {
      SamplePool::PoolDescriptor descriptor = {
          .samples_per_pixel = 10,
          .dimensions = 3,
          .array1_sizes = {1},
          .array2_sizes = {2, 1}
      };
      REQUIRE(descriptor.memory_size_in_bytes() ==
          10 * 3 * (sizeof(real_t) + sizeof(hermes::point2))     // dimension block
              + sizeof(SamplePool::size_type) * (1 + 2)          // header
              + sizeof(real_t) * 1 * 10                          // 1d arrays
              + sizeof(hermes::point2) * (2 + 1) * 10            // 2d arrays
      );
      hermes::HostMemory m(descriptor.memory_size_in_bytes());
      descriptor.write_layout(m.ptr(), true);
    }//
  }//
  SECTION("sanity") {
    SamplePool::PoolDescriptor descriptor = {
        .samples_per_pixel = 10,
        .dimensions = 3,
        .array1_sizes = {1},
        .array2_sizes = {2, 1}
    };
    hermes::UnifiedMemory m(descriptor.memory_size_in_bytes());
    descriptor.write_layout(m.ptr());
    SamplePool pool(descriptor, m.ptr());
    auto sample_count = 0;
    do {
      REQUIRE(pool.currentSampleIndex() == sample_count);
      for (size_t d = 0; d < descriptor.dimensions; ++d) {
        real_t s1 = sample_count * 10 + d;
        hermes::point2 s2 = {static_cast<real_t>(sample_count), static_cast<real_t>(d)};
        pool.get1DSample() = s1;
        pool.get2DSample() = s2;
      }
      for (size_t a = 0; a < descriptor.array1_sizes.size(); ++a) {
        auto *array = pool.get1DArray();
        REQUIRE(array != nullptr);
        for (size_t i = 0; i < pool.get1DArraySize(a); ++i)
          array[i] = a * 10 + i + 1;
      }
      for (size_t a = 0; a < descriptor.array2_sizes.size(); ++a) {
        auto *array = pool.get2DArray();
        REQUIRE(array != nullptr);
        for (size_t i = 0; i < pool.get2DArraySize(a); ++i)
          array[i] = hermes::point2(pool.currentSampleIndex(), a * 10 + i + 1);
      }
      REQUIRE(pool.get1DArraySize(0) == 1);
      REQUIRE(pool.get1DArraySize(1) == 0);
      REQUIRE(pool.get2DArraySize(0) == 2);
      REQUIRE(pool.get2DArraySize(1) == 1);
      REQUIRE(pool.get2DArraySize(2) == 0);
      REQUIRE(!pool.has1D());
      REQUIRE(!pool.has2D());
      REQUIRE(pool.get1DArray() == nullptr);
      REQUIRE(pool.get2DArray() == nullptr);
      ++sample_count;
    } while (pool.startNextSample());
    REQUIRE(sample_count == descriptor.samples_per_pixel);
//    HERMES_LOG_VARIABLE(SamplePool::dumpMemory(pool, 1))
    hermes::UnifiedArray<int> results(1);
    HERMES_CUDA_LAUNCH_AND_SYNC((1), check_pool_k, pool, results.data())
    REQUIRE(results[0] == 0);
  }//
  SECTION("multiple items") {
    SamplePool::PoolDescriptor descriptor = {
        .samples_per_pixel = 10,
        .dimensions = 3,
        .array1_sizes = {1},
        .array2_sizes = {2, 1}
    };
    const size_t pool_size = 2;
    const size_t pool_item_size = descriptor.memory_size_in_bytes();
    hermes::UnifiedMemory m(pool_size * pool_item_size);
    for (size_t i = 0; i < pool_size; ++i)
      descriptor.write_layout(m.ptr() + i * pool_item_size);
    SamplePool pool(descriptor, m.ptr());
    hermes::UnifiedArray<int> results(1);
    HERMES_CUDA_LAUNCH_AND_SYNC((pool_size), fill_pool_k, pool, pool_size, results.data())
    REQUIRE(results[0] == 0);
//    HERMES_LOG_VARIABLE(SamplePool::dumpMemory(pool, pool_size))
  }//
}

TEST_CASE("StratifiedSampler", "[sampling]") {
  SECTION("sanity") {
    StratifiedSampler sampler(hermes::size2(2, 2), true, 3);
    hermes::UnifiedMemory mem(sampler.memorySize() * 1);
    REQUIRE(mem.sizeInBytes() == sampler.memorySize());
    sampler.setDataPtr(mem.ptr());
    sampler.startPixel({});

    auto pool = sampler.samplePool();
//    HERMES_LOG_VARIABLE(SamplePool::dumpMemory(pool,1))
    for (auto ij : hermes::range2(hermes::size2(2, 2))) {
      HERMES_LOG_VARIABLE(ij)
      HERMES_LOG_VARIABLE(pool.get1DSample());
      HERMES_LOG_VARIABLE(pool.get2DSample());
    }
  }//
}