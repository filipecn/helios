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
///\file spectra.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-15
///
///\brief

#ifndef HELIOS_HELIOS_SPECTRUM_SPECTRA_H
#define HELIOS_HELIOS_SPECTRUM_SPECTRA_H

#include <helios/base/spectrum.h>
#include <helios/spectra/blackbody_spectrum.h>

namespace helios {

#define CAST_SPECTRUM(SPECTRUM, PTR, CODE)                                                                          \
{                                                                                                                   \
  switch(SPECTRUM.type) {                                                                                           \
    case SpectrumType::BLACKBODY: {                                                                                \
                     BlackbodySpectrum * PTR = (BlackbodySpectrum*)SPECTRUM.data_ptr.get(); CODE break; }         \
  }                                                                                                                 \
}

#define CAST_CONST_SPECTRUM(SPECTRUM, PTR, CODE)                                                                    \
{                                                                                                                   \
  switch(SPECTRUM.type) {                                                                                           \
    case SpectrumType::BLACKBODY: {                                                                                \
         const BlackbodySpectrum * PTR = (const BlackbodySpectrum*)SPECTRUM.data_ptr.get(); CODE break; }         \
  }                                                                                                                 \
}

}

#endif //HELIOS_HELIOS_SPECTRUM_SPECTRA_H
