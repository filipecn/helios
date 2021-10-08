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
///\file core/filter.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-07
///
///\brief

#ifndef HELIOS_CORE_FILTER_H
#define HELIOS_CORE_FILTER_H

#include <hermes/geometry/point.h>

namespace helios {

// *********************************************************************************************************************
//                                                                                                             Filter
// *********************************************************************************************************************
/// Base class for implementation of various types of filter functions.
class Filter {
public:
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE Filter();
  ///  The components of **radius_** define points in the axis where the function
  ///  is zero beyond. These points go to each direction, the overall extent
  ///  in each direction  (called support) is **twice** those values.
  /// \param radius filter extents
  HERMES_DEVICE_CALLABLE explicit Filter(const hermes::vec2 &radius);
  /// Evaluates this filter function at **p**
  /// \param p sample point relative to the center of the filter
  /// \return real_t filter's value
  [[nodiscard]] HERMES_DEVICE_CALLABLE virtual real_t evaluate(const hermes::point2 &p) const = 0;
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
  const hermes::vec2 radius{};        //!< filter's radius_ of support
  const hermes::vec2 inv_radius{};    //!< reciprocal of radius_
};

// *********************************************************************************************************************
//                                                                                                  PreComputedFilter
// *********************************************************************************************************************
/// Stores a table of pre-computed weights for a given filter
/// \note Generally, every image sample contributes to 16 pixels in the final image (TABLE_WIDTH = 16).
template<size_t TABLE_WIDTH>
class PreComputedFilter {
public:
  PreComputedFilter() {
    for (int y = 0, offset = 0; y < filter_table_width; ++y)
      for (int x = 0; x < filter_table_width; ++x, ++offset)
        table[offset] = 1;
  }
  explicit PreComputedFilter(const Filter *filter) : radius{filter->radius}, inv_radius{filter->inv_radius} {
    // precompute filter weight table
    for (int y = 0, offset = 0; y < filter_table_width; ++y)
      for (int x = 0; x < filter_table_width; ++x, ++offset) {
        hermes::point2 p((x + 0.5f) * filter->radius.x / filter_table_width,
                         (y + 0.5f) * filter->radius.y / filter_table_width);
        table[offset] = filter->evaluate(p);
      }
  }
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
  static constexpr int filter_table_width = TABLE_WIDTH;
  real_t table[TABLE_WIDTH * TABLE_WIDTH]{};                //!< precomputed table for filter values
  //!<                                                           to save computations of filter's
  //!<                                                           evaluate method. f = f(|x|, |y|)
  const hermes::vec2 radius;                                //!< filter's radius_ of support
  const hermes::vec2 inv_radius;                            //!< reciprocal of radius_
};

// *********************************************************************************************************************
//                                                                                                         Box Filter
// *********************************************************************************************************************
/// Equally weights all samples within a square region of the image.
/// Can cause post-aliasing even when the original image's frequencies
/// respect the Nyquist limit.
class BoxFilter : public Filter {
public:
  // *******************************************************************************************************************
  //                                                                                                     CONSTRUCTORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE BoxFilter();
  HERMES_DEVICE_CALLABLE explicit BoxFilter(const hermes::vec2 &radius);
  // *******************************************************************************************************************
  //                                                                                                        INTERFACE
  // *******************************************************************************************************************
  [[nodiscard]] HERMES_DEVICE_CALLABLE real_t evaluate(const hermes::point2 &p) const override;
};

} // namespace helios

#endif // HELIOS_CORE_FILTER_H
