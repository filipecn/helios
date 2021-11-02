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
///\file texture_eval_context.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-21
///
///\brief

#include <helios/textures/texture_eval_context.h>
#include <helios/core/interaction.h>

namespace helios {

HERMES_DEVICE_CALLABLE
TextureEvalContext::TextureEvalContext(const Interaction &intr) : p(intr.p), uv(intr.uv) {}

HERMES_DEVICE_CALLABLE
TextureEvalContext::TextureEvalContext(const SurfaceInteraction &si)
    : p(si.p),
      dpdx(si.dpdx),
      dpdy(si.dpdy),
      n(si.n),
      uv(si.uv),
      dudx(si.dudx),
      dudy(si.dudy),
      dvdx(si.dvdx),
      dvdy(si.dvdy),
      faceIndex(si.face_index) {}

HERMES_DEVICE_CALLABLE
TextureEvalContext::TextureEvalContext(hermes::point3 p,
                                       hermes::vec3 dpdx,
                                       hermes::vec3 dpdy,
                                       hermes::normal3 n,
                                       hermes::point2 uv,
                                       real_t dudx,
                                       real_t dudy,
                                       real_t dvdx,
                                       real_t dvdy,
                                       int faceIndex)
    : p(p),
      dpdx(dpdx),
      dpdy(dpdy),
      n(n),
      uv(uv),
      dudx(dudx),
      dudy(dudy),
      dvdx(dvdx),
      dvdy(dvdy),
      faceIndex(faceIndex) {}
}
