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
///\file io.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-08-09
///
///\brief

#include <helios/common/io.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <hermes/common/file_system.h>
#include <helios/common/globals.h>

using namespace hermes;

namespace helios::io {

bool save(const Array<real_t> &data,
          const Path &filename,
          const Index2Range<i32> &output_bounds,
          const size2 &full_resolution) {
  auto resolution = output_bounds.upper() - output_bounds.lower();
  auto file_extension = filename.extension();
  if (file_extension == "exr") {
    HERMES_NOT_IMPLEMENTED
    return false;
//    WriteImageEXR(name, rgb, resolution.x, resolution.y, totalResolution.x,
//                  totalResolution.y, outputBounds.pMin.x,
//                  outputBounds.pMin.y);
  } else if (file_extension == "tga" || file_extension == "png") {
    // 8-bit formats; apply gamma
    std::unique_ptr<uint8_t[]> rgb8(
        new uint8_t[3 * resolution.i * resolution.j]);
    uint8_t *dst = rgb8.get();
    for (int y = 0; y < resolution.j; ++y) {
      for (int x = 0; x < resolution.i; ++x) {
#define TO_BYTE(v) (uint8_t) Numbers::clamp(255.f * globals::gammaCorrect(v) + 0.5f, 0.f, 255.f)
        dst[0] = TO_BYTE(data[3 * (y * resolution.i + x) + 0]);
        dst[1] = TO_BYTE(data[3 * (y * resolution.i + x) + 1]);
        dst[2] = TO_BYTE(data[3 * (y * resolution.i + x) + 2]);
#undef TO_BYTE
        dst += 3;
      }
    }

    if (file_extension == "tga") {
      HERMES_NOT_IMPLEMENTED
      return false;
//      WriteImageTGA(name, rgb8.get(), resolution.x, resolution.y,
//                    totalResolution.x, totalResolution.y,
//                    outputBounds.pMin.x, outputBounds.pMin.y);
    } else {
      auto err =
          stbi_write_png(filename.fullName().c_str(), resolution.i, resolution.j, 3, rgb8.get(), resolution.i * 3);
//      unsigned int error = lodepng_encode24_file(
//          name.c_str(), rgb8.get(), resolution.x, resolution.y);
//      if (error != 0)
//        Error("Error writing PNG \"%s\": %s", name.c_str(),
//              lodepng_error_text(error));
    }
  } else {
    Log::error("Can't determine image file type from suffix of filename \"{}\"", filename);
  }
  return true;
}

}