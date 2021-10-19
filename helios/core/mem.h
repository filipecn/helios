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
///\file mem.h
///\author FilipeCN (filipedecn@gmail.com)
///\date 2021-10-04
///
///\brief

#ifndef HELIOS_HELIOS_CORE_MEM_H
#define HELIOS_HELIOS_CORE_MEM_H

#include <hermes/storage/stack_allocator.h>
#include <hermes/common/debug.h>

namespace helios {

/// Memory Manager Singleton
/// This memory manager implements a Stack Allocator scheme where
/// objects are allocated contiguously and it is not possible to
/// remove a object without deleting the objects on top of the
/// object
class mem {
public:
  // *******************************************************************************************************************
  //                                                                                                          Pointer
  // *******************************************************************************************************************
  struct Ptr {
    HERMES_DEVICE_CALLABLE Ptr();
    HERMES_DEVICE_CALLABLE Ptr(hermes::AddressIndex address_index);
    HERMES_DEVICE_CALLABLE explicit operator bool() const;
    HERMES_DEVICE_CALLABLE  void *get();
    [[nodiscard]] HERMES_DEVICE_CALLABLE const void *get() const;
    void update();
    HERMES_DEVICE_CALLABLE void update(hermes::StackAllocatorView m);
    hermes::AddressIndex address_index;
  private:
    void *ptr{nullptr};
  };
  // *******************************************************************************************************************
  //                                                                                                   STATIC METHODS
  // *******************************************************************************************************************
  //                                                                                                        singleton
  ///
  /// \return
  static mem &get() {
    static mem singleton;
    return singleton;
  }
  //                                                                                                         creation
  /// Allocates the memory that will be available for all allocators to use
  /// \param size_in_bytes
  /// \return
  static HeResult init(std::size_t size_in_bytes);
  ///
  /// \return
  static HeResult sendToGPU();
  static hermes::StackAllocatorView gpuView();
  //                                                                                                       allocation
  ///
  /// \tparam T
  /// \tparam P
  /// \param params
  /// \return
  template<typename T, class... P>
  static Ptr allocateAligned(P &&... params) {
    return Ptr(mem::get().allocator_.pushAligned<T>(std::forward<P>(params)...));
  }
  ///
  /// \tparam T
  /// \tparam P
  /// \param params
  /// \return
  template<typename T, class... P>
  static Ptr allocate(P &&... params) {
    return Ptr(mem::get().allocator_.push<T>(std::forward<P>(params)...));
  }
  //                                                                                                           access
  ///
  /// \tparam T
  /// \param handle
  /// \param value
  /// \return
  template<typename T>
  static HeResult set(Ptr handle, const T &value) {
    return mem::get().allocator_.set<T>(handle.address_index, value);
  }
  ///
  /// \tparam T
  /// \param handle
  /// \return
  template<typename T>
  static T *get(Ptr handle) {
    return mem::get().allocator_.get<T>(handle.address_index);
  }
  //                                                                                                             size
  ///
  /// \return
  static std::size_t availableSize();
  // *******************************************************************************************************************
  //                                                                                                        OPERATORS
  // *******************************************************************************************************************
  mem &operator=(const mem &) = delete;

  // *******************************************************************************************************************
  //                                                                                                            DEBUG
  // *******************************************************************************************************************
  static std::string dumpMemory() {
    hermes::Str r;
    const auto &ha = mem::get().allocator_.view();
    const auto &da = mem::get().d_allocator_.view();
    r.appendLine("---- Memory Manager Info ----");
    r.appendLine("Host Side -------------------");
    r.appendLine("  Size: ", ha.capacity_in_bytes);
    r.appendLine("  Available: Size ", ha.availableSizeInBytes());
    r.appendLine("  Address: ", hermes::Str::addressOf(reinterpret_cast<uintptr_t>(ha.data())));
    r.appendLine("  End addr: ", hermes::Str::addressOf(reinterpret_cast<uintptr_t>(ha.data()) + ha.capacity_in_bytes));

    r.appendLine("Device Side -----------------");
    r.appendLine("  Size ", da.capacity_in_bytes);
    r.appendLine("  Available Size ", da.availableSizeInBytes());
    r.appendLine("  Address: ", hermes::Str::addressOf(reinterpret_cast<uintptr_t>(da.data())));
    r.appendLine("  End addr: ", hermes::Str::addressOf(reinterpret_cast<uintptr_t>(da.data()) + da.capacity_in_bytes));
    return r.str();
  }

private:
  mem() = default;
  ~mem() = default;

  hermes::StackAllocator allocator_;
  hermes::DeviceStackAllocator d_allocator_;
};

}

#endif //HELIOS_HELIOS_CORE_MEM_H
