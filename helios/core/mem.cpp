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
///\file mem.cpp
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-04
///
///\brief

#include <helios/core/mem.h>

namespace helios {

mem::Ptr::Ptr() = default;

mem::Ptr::Ptr(hermes::AddressIndex address_index) : address_index(address_index) {
  update();
}

HERMES_DEVICE_CALLABLE void *mem::Ptr::get() {
  return ptr;
}

HERMES_DEVICE_CALLABLE const void *mem::Ptr::get() const {
  return ptr;
}

void mem::Ptr::update() {
  ptr = mem::get<void>(*this);
}

HERMES_DEVICE_CALLABLE void mem::Ptr::update(hermes::StackAllocatorView m) {
  ptr = m.get<void>(address_index);
}

HeResult mem::init(std::size_t size_in_bytes) {
  return mem::get().allocator_.resize(size_in_bytes);
}

HeResult mem::sendToGPU() {
  mem::get().d_allocator_ = mem::get().allocator_;
  return HeResult::SUCCESS;
}

hermes::StackAllocatorView mem::gpuView() {
  return mem::get().d_allocator_.view();
}

std::size_t mem::availableSize() {
  return mem::get().allocator_.availableSizeInBytes();
}

}
