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
///\file microfacet_distribution.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-13
///
///\brief

#ifndef HELIOS_HELIOS_CORE_MICROFACET_DISTRIBUTION_H
#define HELIOS_HELIOS_CORE_MICROFACET_DISTRIBUTION_H

#include <helios/core/mem.h>

namespace helios {

enum class MicrofacetDistributionType {
  TROWBRIDGE_REITZ,
  CUSTOM
};

// *********************************************************************************************************************
//                                                                                             MicrofacetDistribution
// *********************************************************************************************************************
/// Microfacet Distribution Interface
/// A microfacet distribution statistically describes how the many micro surface pieces are oriented.
/// The surface that, from distance appears smooth, can be rough at a closer look, scattering light
/// in more directions. The microfacet approach models how light is scattered by a particular surface by taking
/// into account the roughness surface in the micro-surface level. Therefore, the orientation of the microfacets
/// (the small pieces of surface) define the actual surface's normal to be used in light computations.
struct MicrofacetDistribution {
  // *******************************************************************************************************************
  //                                                                                                        OPERATORS
  // *******************************************************************************************************************
  HERMES_DEVICE_CALLABLE explicit operator bool() const { return (bool) data_ptr; }
  // *******************************************************************************************************************
  //                                                                                                    PUBLIC FIELDS
  // *******************************************************************************************************************
  mem::Ptr data_ptr;                  //<
  MicrofacetDistributionType type{MicrofacetDistributionType::CUSTOM};
};

}

#endif //HELIOS_HELIOS_CORE_MICROFACET_DISTRIBUTION_H
