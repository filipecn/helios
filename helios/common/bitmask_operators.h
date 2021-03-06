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
///\file bitmask_operators.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-07-05
///\brief Add support for enum classes that allow bitwise operations
/// Usage:
/// Suppose have an enum class object and want to perform bitwise operations
/// with its values:
/// enum class Permissions {
///    Readable   = 0x4,
///    Writeable  = 0x2,
///    Executable = 0x1
/// };
/// In order to allow such operations as
/// Permissions p = Permissions::Readable | Permissions::Writable;
/// just add the macro call after declaration:
/// enum class Permissions {..};
/// HELIOS_ENABLE_BITMASK_OPERATORS(Permissions);

#ifndef HELIOS_HELIOS_COMMON_BITMASK_OPERATORS_H
#define HELIOS_HELIOS_COMMON_BITMASK_OPERATORS_H

#include <type_traits>

namespace helios {

#define HELIOS_ENABLE_BITMASK_OPERATORS(x) \
template<>                           \
struct EnableBitMaskOperators<x>     \
{                                    \
    static const bool enable = true;       \
}

#define HELIOS_MASK_BIT(MASK, BIT) ((u32)(MASK) & (u32)(BIT)) == (u32)(BIT)

template<typename Enum>
struct EnableBitMaskOperators {
  static const bool enable = false;
};

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator|(Enum lhs, Enum rhs) {
  using underlying = typename std::underlying_type<Enum>::type;
  return static_cast<Enum> (
      static_cast<underlying>(lhs) |
          static_cast<underlying>(rhs)
  );
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator&(Enum lhs, Enum rhs) {
  using underlying = typename std::underlying_type<Enum>::type;
  return static_cast<Enum> (
      static_cast<underlying>(lhs) &
          static_cast<underlying>(rhs)
  );
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator^(Enum lhs, Enum rhs) {
  using underlying = typename std::underlying_type<Enum>::type;
  return static_cast<Enum> (
      static_cast<underlying>(lhs) ^
          static_cast<underlying>(rhs)
  );
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator~(Enum rhs) {
  using underlying = typename std::underlying_type<Enum>::type;
  return static_cast<Enum> (
      ~static_cast<underlying>(rhs)
  );
}

}

#endif //HELIOS_HELIOS_COMMON_BITMASK_OPERATORS_H
