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
///\file microfacet_distributions.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-13
///
///\brief

#ifndef HELIOS_HELIOS_SCATTERING_MICROFACET_DISTRIBUTIONS_H
#define HELIOS_HELIOS_SCATTERING_MICROFACET_DISTRIBUTIONS_H

#include <helios/base/microfacet_distribution.h>
#include <helios/scattering/trowbridge_reitz_distribution.h>

namespace helios {

#define CAST_MICROFACET_DISTRIBUTION(MICROFACET_DISTRIBUTION, PTR, CODE)                                                                          \
{                                                                                                                   \
  switch(MICROFACET_DISTRIBUTION.type) {                                                                                           \
    case MicrofacetDistributionType::TROWBRIDGE_REITZ: {                                                                                \
                     auto * PTR = MICROFACET_DISTRIBUTION.data_ptr.get<TrowbridgeReitzDistribution>(); CODE break; }         \
  }                                                                                                                 \
}

#define CAST_CONST_MICROFACET_DISTRIBUTION(MICROFACET_DISTRIBUTION, PTR, CODE)                                                                    \
{                                                                                                                   \
  switch(MICROFACET_DISTRIBUTION.type) {                                                                                           \
    case MicrofacetDistributionType::TROWBRIDGE_REITZ: {                                                                                \
         const auto * PTR = MICROFACET_DISTRIBUTION.data_ptr.get<TrowbridgeReitzDistribution>(); CODE break; }         \
  }                                                                                                                 \
}

struct MicrofacetDistributions {
  ///
  /// \tparam T
  /// \return
  template<typename T>
  HERMES_DEVICE_CALLABLE static MicrofacetDistributionType enumFromType() {
    if (std::is_same_v<T, TrowbridgeReitzDistribution>)
      return MicrofacetDistributionType::TROWBRIDGE_REITZ;
    return MicrofacetDistributionType::CUSTOM;
  }
  ///
  /// \tparam T
  /// \tparam Allocator
  /// \tparam P
  /// \param allocator
  /// \param params
  /// \return
  template<typename T, typename Allocator, typename ... P>
  HERMES_DEVICE_CALLABLE static MicrofacetDistribution create(Allocator allocator, P &&... params) {
    MicrofacetDistribution mf_distribution;
    mf_distribution.data_ptr = allocator.template allocate<T>(std::forward<P>(params)...);
    mf_distribution.type = enumFromType<T>();
    return mf_distribution;
  }
};

}
#endif //HELIOS_HELIOS_SCATTERING_MICROFACET_DISTRIBUTIONS_H
