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
///\file sample_pool.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-15
///
///\brief

#include <helios/samplers/sample_pool.h>

using namespace hermes;

namespace helios {

#define SAMPLE_BASE_PTR(SAMPLE_INDEX) \
  (data_ + item_offset_ + first_pixel_sample_offset + (SAMPLE_INDEX) * pixel_block_size_)

SamplePool::SamplePool(const SamplePool::PoolDescriptor &descriptor, byte *data) : data_{data} {
  set(descriptor, data);
}

HERMES_DEVICE_CALLABLE SamplePool::SamplePool(const SamplePool &other) {
  data_ = other.data_;
  pool_item_size_ = other.pool_item_size_;
  item_offset_ = other.item_offset_;
  current_sample_index_ = other.current_sample_index_;
  current_sample_array1_index_ = other.current_sample_array1_index_;
  current_sample_array2_index_ = other.current_sample_array2_index_;
  current_sample_array1_offset_ = other.current_sample_array1_offset_;
  current_sample_array2_offset_ = other.current_sample_array2_offset_;
  current_1d_dimension_ = other.current_1d_dimension_;
  current_2d_dimension_ = other.current_2d_dimension_;
  samples_per_pixel_ = other.samples_per_pixel_;
  sampled_dimensions_count_ = other.sampled_dimensions_count_;
  sample_array1_count_ = other.sample_array1_count_;
  sample_array2_count_ = other.sample_array2_count_;
  dimension_block_size_ = other.dimension_block_size_;
  pixel_block_size_ = other.pixel_block_size_;
  first_sample_array1_offset = other.first_sample_array1_offset;
  first_sample_array2_offset = other.first_sample_array2_offset;
  first_pixel_sample_offset = other.first_pixel_sample_offset;
}

HERMES_DEVICE_CALLABLE SamplePool::~SamplePool() {}

void SamplePool::set(const SamplePool::PoolDescriptor &descriptor, byte *data) {
  // pool data
  pool_item_size_ = descriptor.memory_size_in_bytes();
  // layout
  samples_per_pixel_ = descriptor.samples_per_pixel;
  sampled_dimensions_count_ = descriptor.dimensions;
  dimension_block_size_ = sizeof(real_t) + sizeof(point2);
  sample_array2_count_ = descriptor.array2_sizes.size();

  SamplePool::size_type sample_array1_sizes_sum = 0;
  for (auto s : descriptor.array1_sizes)
    sample_array1_sizes_sum += s;
  SamplePool::size_type sample_array2_sizes_sum = 0;
  for (auto s : descriptor.array2_sizes)
    sample_array2_sizes_sum += s;

  sample_array1_count_ = descriptor.array1_sizes.size();
  pixel_block_size_ =
      sampled_dimensions_count_ * dimension_block_size_         // dimension samples
          + sample_array1_sizes_sum * sizeof(real_t)            // 1d sample arrays
          + sample_array2_sizes_sum * sizeof(point2);   // 2d sample arrays

  first_pixel_sample_offset = (sample_array1_count_ + sample_array2_count_) * sizeof(SamplePool::size_type);
  first_sample_array1_offset = sampled_dimensions_count_ * dimension_block_size_;
  first_sample_array2_offset = first_sample_array1_offset + sample_array1_sizes_sum * sizeof(real_t);

  reset();
}

HERMES_DEVICE_CALLABLE void SamplePool::setData(byte *data) {
  data_ = data;
}

HERMES_DEVICE_CALLABLE void SamplePool::setPoolIndex(SamplePool::size_type i) {
  item_offset_ = i * pool_item_size_;
}

HERMES_DEVICE_CALLABLE real_t &SamplePool::get1DSample() {
  return *reinterpret_cast<real_t *>(SAMPLE_BASE_PTR(current_sample_index_)
      + dimension_block_size_ * current_1d_dimension_++);
}

HERMES_DEVICE_CALLABLE real_t &SamplePool::get1DSample(SamplePool::index_type sample_index,
                                                       SamplePool::index_type dimension_index) {
  return *reinterpret_cast<real_t *>(SAMPLE_BASE_PTR(sample_index) + dimension_block_size_ * dimension_index);
}

HERMES_DEVICE_CALLABLE point2 &SamplePool::get2DSample() {
  return *reinterpret_cast<point2 *>(SAMPLE_BASE_PTR(current_sample_index_) +
      dimension_block_size_ * current_2d_dimension_++ + sizeof(real_t));
}

HERMES_DEVICE_CALLABLE point2 &SamplePool::get2DSample(SamplePool::index_type sample_index,
                                                       SamplePool::index_type dimension_index) {
  return *reinterpret_cast<point2 *>(SAMPLE_BASE_PTR(sample_index) +
      dimension_block_size_ * dimension_index + sizeof(real_t));
}

HERMES_DEVICE_CALLABLE real_t *SamplePool::get1DArray() {
  if (current_sample_array1_index_ < sample_array1_count_) {
    auto *r = reinterpret_cast<real_t *>(SAMPLE_BASE_PTR(current_sample_index_) + current_sample_array1_offset_);
    current_sample_array1_offset_ +=
        (*reinterpret_cast<SamplePool::size_type *>(data_ + item_offset_
            + sizeof(SamplePool::size_type) * current_sample_array1_index_)) * sizeof(real_t);
    current_sample_array1_index_++;
    return r;
  }
  return nullptr;
}

HERMES_DEVICE_CALLABLE real_t *SamplePool::get1DArray(SamplePool::index_type sample_index,
                                                      SamplePool::index_type array_index) {
  if (array_index < sample_array1_count_) {
    // compute array offset
    SamplePool::index_type offset = 0;
    for (int i = 0; i < array_index; ++i)
      offset += get1DArraySize(i);
    return reinterpret_cast<real_t *>(SAMPLE_BASE_PTR(sample_index) + first_sample_array1_offset
        + offset * sizeof(real_t));
  }
  return nullptr;
}

HERMES_DEVICE_CALLABLE point2 *SamplePool::get2DArray() {
  if (current_sample_array2_index_ < sample_array2_count_) {
    auto *r = reinterpret_cast<point2 *>(SAMPLE_BASE_PTR(current_sample_index_) + current_sample_array2_offset_);
    current_sample_array2_offset_ +=
        (*reinterpret_cast<SamplePool::size_type *>(data_ + item_offset_ +
            +sizeof(SamplePool::size_type) * (sample_array1_count_ + current_sample_array2_index_))) * sizeof(point2);
    current_sample_array2_index_++;
    return r;
  }
  return nullptr;
}

HERMES_DEVICE_CALLABLE point2 *SamplePool::get2DArray(SamplePool::index_type sample_index,
                                                      SamplePool::index_type array_index) {
  if (array_index < sample_array2_count_) {
    // compute array offset
    SamplePool::index_type offset = 0;
    for (int i = 0; i < array_index; ++i)
      offset += get2DArraySize(i);
    return reinterpret_cast<point2 *>(SAMPLE_BASE_PTR(sample_index) + first_sample_array2_offset
        + offset * sizeof(point2));
  }
  return nullptr;
}

HERMES_DEVICE_CALLABLE SamplePool::size_type SamplePool::get1DArraySize(SamplePool::index_type array_index) const {
  if (array_index >= sample_array1_count_)
    return 0;
  return *reinterpret_cast<SamplePool::size_type *>(data_ + item_offset_ + sizeof(SamplePool::size_type) * array_index);
}

HERMES_DEVICE_CALLABLE SamplePool::size_type SamplePool::get2DArraySize(SamplePool::index_type array_index) const {
  if (array_index >= sample_array2_count_)
    return 0;
  return *reinterpret_cast<SamplePool::size_type *>(data_ + item_offset_
      + sizeof(SamplePool::size_type) * (sample_array1_count_ + array_index));
}

HERMES_DEVICE_CALLABLE bool SamplePool::startNextSample() {
  current_1d_dimension_ = current_2d_dimension_ = current_sample_array1_index_ = current_sample_array2_index_ = 0;
  current_sample_array1_offset_ = first_sample_array1_offset;
  current_sample_array2_offset_ = first_sample_array2_offset;
  return ++current_sample_index_ < samples_per_pixel_;
}

HERMES_DEVICE_CALLABLE void SamplePool::reset() {
  current_sample_index_ = 0;
  current_1d_dimension_ = current_2d_dimension_ = current_sample_array1_index_ = current_sample_array2_index_ = 0;
  current_sample_array1_offset_ = first_sample_array1_offset;
  current_sample_array2_offset_ = first_sample_array2_offset;

}

std::string SamplePool::dumpMemory(const SamplePool &pool,
                                   SamplePool::size_type size,
                                   memory_dumper_options options) {
  std::string r;
  SamplePool::size_type sample_array1_sizes_sum = 0;
  for (SamplePool::size_type i = 0; i < pool.sample_array1_count_; ++i)
    sample_array1_sizes_sum += pool.get1DArraySize(i);
  SamplePool::size_type sample_array2_sizes_sum = 0;
  for (SamplePool::size_type i = 0; i < pool.sample_array2_count_; ++i)
    sample_array2_sizes_sum += pool.get2DArraySize(i);
  r += MemoryDumper::dump(pool.data_, size * pool.pool_item_size_, 12,
                          {// pool item
                              0, pool.pool_item_size_, size, ConsoleColors::light_gray,
                              {
                                  { // array sizes
                                      0, sizeof(SamplePool::size_type),
                                      pool.sample_array1_count_ + pool.sample_array2_count_,
                                      ConsoleColors::blue, {}, DataType::U64
                                  },
                                  { // pixel samples
                                      static_cast<size_t>(pool.first_pixel_sample_offset),
                                      pool.pixel_block_size_,
                                      pool.samples_per_pixel_,
                                      ConsoleColors::red,
                                      {
                                          { // dimensions
                                              0, pool.dimension_block_size_,
                                              pool.sampled_dimensions_count_,
                                              ConsoleColors::yellow, {
                                                  { // 1d
                                                      0, sizeof(f32), 1,
                                                      ConsoleColors::red,
                                                      {}, DataType::F32
                                                  },
                                                  {// 2d
                                                      sizeof(f32), sizeof(f32), 2,
                                                      ConsoleColors::yellow,
                                                      {}, DataType::F32
                                                  }
                                              }},
                                          { // 1d arrays
                                              static_cast<size_t>(pool.first_sample_array1_offset),
                                              sizeof(real_t), sample_array1_sizes_sum,
                                              ConsoleColors::cyan, {}, DataType::F32},
                                          { // 2d arrays
                                              static_cast<size_t>(pool.first_sample_array2_offset),
                                              sizeof(point2),
                                              sample_array2_sizes_sum,
                                              ConsoleColors::green, {}
                                          }}}}},
                          options);
  return r;
}

}