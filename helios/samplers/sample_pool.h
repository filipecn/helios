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
///\file sample_pool.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-15
///
///\brief

#ifndef HELIOS_SAMPLERS_SAMPLE_POOL_H
#define HELIOS_SAMPLERS_SAMPLE_POOL_H

#include <hermes/geometry/point.h>
#include <hermes/logging/memory_dump.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                         SamplePool
// *********************************************************************************************************************
/// Manages memory containing pre-computed samples from samplers. Different algorithms require different
/// sets of samples in a variable number of dimensions. The SamplePool keeps track of sample requisitions
/// by consuming items in the pool.
/// \note The SamplePool does not own the memory used by its items. It just points a pre-allocated memory.
class SamplePool {
public:
  // *******************************************************************************************************************
  //                                                                                                            TYPES
  // *******************************************************************************************************************
  typedef size_t size_type;
  typedef size_t index_type;
  // *******************************************************************************************************************
  //                                                                                                 Descriptor class
  // *******************************************************************************************************************
  /// Defines the data stored in a sample pool
  /// \note A sample pool data layout considers 4 types of data: 1D sample, 2D sample,
  /// \note 1D samples array and 2D samples array.
  /// \note A pair of one 1D sample and one 2D sample is stored for each dimension and
  /// \note the data for the dynamic arrays comes after. The pattern is repeated for pixel-samples count.
  struct PoolDescriptor {
    size_type samples_per_pixel{0};      //!< total count of samples (1D and 2D samples)
    size_type dimensions{0};             //!< number of sampled dimensions
    std::vector<size_type> array1_sizes; //!< sizes of requested arrays of 1D samples
    std::vector<size_type> array2_sizes; //!< sizes of requested arrays of 2D samples
    [[nodiscard]] size_type memory_size_in_bytes() const {
      size_type size = samples_per_pixel * dimensions * (sizeof(real_t) + sizeof(hermes::point2));
      size += array1_sizes.size() * sizeof(size_type);
      size += array2_sizes.size() * sizeof(size_type);
      for (auto s : array1_sizes)
        size += sizeof(real_t) * s * samples_per_pixel;
      for (auto s : array2_sizes)
        size += sizeof(hermes::point2) * s * samples_per_pixel;
      return size;
    }
    /// Stores the sample pool description into memory
    /// \param data pointer to memory region with at least memory_size_in_bytes() size
    /// \param init_with_zeros initiates zeros on all sample positions
    void write_layout(byte *pool_data, bool init_with_zeros = false) {
      if (init_with_zeros)
        std::memset(pool_data, 0, memory_size_in_bytes());
      // we just need to put a header containing the sizes of sample arrays
      byte *ptr = pool_data;
      for (auto s : array1_sizes) {
        *reinterpret_cast<size_type *>(ptr) = s;
        ptr += sizeof(size_type);
      }
      for (auto s : array2_sizes) {
        *reinterpret_cast<size_type *>(ptr) = s;
        ptr += sizeof(size_type);
      }
    }
  };
  // *******************************************************************************************************************
  //                                                                                                 STATIC FUNCTIONS
  // *******************************************************************************************************************
  static std::string dumpMemory(const SamplePool &pool, size_type size,
                                hermes::memory_dumper_options options =
                                hermes::memory_dumper_options::type_values
                                    | hermes::memory_dumper_options::colored_output);
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  SamplePool() = default;
  SamplePool(const PoolDescriptor &descriptor, byte *data);
  HERMES_DEVICE_CALLABLE SamplePool(const SamplePool &other);
  HERMES_DEVICE_CALLABLE ~SamplePool();
  // *******************************************************************************************************************
  //                                                                                                          METHODS
  // *******************************************************************************************************************
  void set(const PoolDescriptor &descriptor, byte *data = nullptr);
  HERMES_DEVICE_CALLABLE void setData(byte *data);
  HERMES_DEVICE_CALLABLE void setPoolIndex(index_type i);
  [[nodiscard]] HERMES_DEVICE_CALLABLE inline index_type currentSampleIndex() const { return current_sample_index_; }
  [[nodiscard]] HERMES_DEVICE_CALLABLE inline bool has1D() const {
    return current_1d_dimension_ < sampled_dimensions_count_;
  }
  [[nodiscard]] HERMES_DEVICE_CALLABLE inline bool has2D() const {
    return current_2d_dimension_ < sampled_dimensions_count_;
  }
  HERMES_DEVICE_CALLABLE real_t &get1DSample();
  HERMES_DEVICE_CALLABLE real_t &get1DSample(index_type sample_index, index_type dimension_index);
  HERMES_DEVICE_CALLABLE hermes::point2 &get2DSample(index_type sample_index, index_type dimension_index);
  HERMES_DEVICE_CALLABLE hermes::point2 &get2DSample();
  HERMES_DEVICE_CALLABLE real_t *get1DArray();
  HERMES_DEVICE_CALLABLE real_t *get1DArray(index_type sample_index, index_type array_index);
  HERMES_DEVICE_CALLABLE hermes::point2 *get2DArray();
  HERMES_DEVICE_CALLABLE hermes::point2 *get2DArray(index_type sample_index, index_type array_index);
  [[nodiscard]] HERMES_DEVICE_CALLABLE size_type get1DArraySize(index_type array_index) const;
  [[nodiscard]] HERMES_DEVICE_CALLABLE size_type get2DArraySize(index_type array_index) const;
  HERMES_DEVICE_CALLABLE bool startNextSample();
  HERMES_DEVICE_CALLABLE void reset();

  [[nodiscard]] HERMES_DEVICE_CALLABLE size_type dimensionCount() const { return sampled_dimensions_count_; }
  [[nodiscard]] HERMES_DEVICE_CALLABLE size_type array1Count() const { return sample_array1_count_; }
  [[nodiscard]] HERMES_DEVICE_CALLABLE size_type array2Count() const { return sample_array2_count_; }
  [[nodiscard]] HERMES_DEVICE_CALLABLE size_type itemSize() const { return pool_item_size_; }

private:
  // pool data
  byte *data_{nullptr};
  size_type pool_item_size_{0};
  uintptr_t item_offset_{0};
  // sampling step
  index_type current_sample_index_{0};
  index_type current_sample_array1_index_{0};
  index_type current_sample_array2_index_{0};
  ptrdiff_t current_sample_array1_offset_{0};
  ptrdiff_t current_sample_array2_offset_{0};
  index_type current_1d_dimension_{0};
  index_type current_2d_dimension_{0};
  // layout goes like this:
  //  <sample_array1_count_ + sample_array2_count_> <samples_per_pixel_ * pixel_block_size_>
  //
  //                            dimension block size         1st 1d array offset
  //                               |           |               |
  //  |array sizes| |pixel sample: {dimension 0} {dimension n} {1d arrays} {2d arrays}}|
  //                |                                                      |
  //              1st pixel sample offset                                  1st 2d array offset
  size_type samples_per_pixel_{0};
  size_type sampled_dimensions_count_{0};
  size_type sample_array1_count_{0};
  size_type sample_array2_count_{0};
  size_type dimension_block_size_{0};
  size_type pixel_block_size_{0};
  ptrdiff_t first_sample_array1_offset{0};
  ptrdiff_t first_sample_array2_offset{0};
  ptrdiff_t first_pixel_sample_offset{0};

};

}

#endif // HELIOS_SAMPLERS_SAMPLE_POOL_H
