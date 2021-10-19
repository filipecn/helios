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
///\file bxdfs.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-13
///
///\brief

#ifndef HELIOS_HELIOS_BXDFS_BXDFS_H
#define HELIOS_HELIOS_BXDFS_BXDFS_H

#include <helios/base/bxdf.h>
#include <helios/scattering/dielectric_bxdf.h>

namespace helios {

#define CAST_BXDF(BXDF, PTR, CODE)                                                                                  \
{                                                                                                                   \
  switch(BXDF.type) {                                                                                               \
    case BxDFType::DIELECTRIC: {                                                                                    \
                     DielectricBxDF * PTR = (DielectricBxDF*)BXDF.data_ptr.get(); CODE break; }                     \
  }                                                                                                                 \
}

#define CAST_CONST_BXDF(BXDF, PTR, CODE)                                                                            \
{                                                                                                                   \
  switch(BXDF.type) {                                                                                               \
    case MaterialType::DIELECTRIC: {                                                                                \
         const DielectricMaterial * PTR = (const DielectricMaterial*)BXDF.data_ptr.get(); CODE break; }             \
  }                                                                                                                 \
}

}

#endif //HELIOS_HELIOS_BXDFS_BXDFS_H
