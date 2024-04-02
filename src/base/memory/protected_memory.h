// Copyright 2024 The Chromium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Protected memory is memory holding security-sensitive data intended to be
// left read-only for the majority of its lifetime to avoid being overwritten
// by attackers. ProtectedMemory is a simple wrapper around platform-specific
// APIs to set memory read-write and read-only when required. Protected memory
// should be set read-write for the minimum amount of time required.
//
// Normally mutable variables are held in read-write memory and constant data
// is held in read-only memory to ensure it is not accidentally overwritten.
// In some cases we want to hold mutable variables in read-only memory, except
// when they are being written to, to ensure that they are not tampered with.
//
// ProtectedMemory is a container class intended to hold a single variable in
// read-only memory, except when explicitly set read-write. The variable can be
// set read-write by creating a scoped AutoWritableMemory object by calling
// AutoWritableMemory::Create(), the memory stays writable until the returned
// object goes out of scope and is destructed. The wrapped variable can be
// accessed using operator* and operator->.
//
// Instances of ProtectedMemory must be declared in the PROTECTED_MEMORY_SECTION
// and as global variables. Because protected memory variables are globals, the
// the same rules apply disallowing non-trivial constructors and destructors.
// Global definitions are required to avoid the linker placing statics in
// inlinable functions into a comdat section and setting the protected memory
// section read-write when they are merged.
//
// EXAMPLE:
//
//  struct Items { void* item1; };
//  static PROTECTED_MEMORY_SECTION base::ProtectedMemory<Items> items;
//  void InitializeItems() {
//    // Explicitly set items read-write before writing to it.
//    auto writer = base::AutoWritableMemory::Create(items);
//    items->item1 = /* ... */;
//    assert(items->item1 != nullptr);
//    // items is set back to read-only on the destruction of writer
//  }
//
//  using FnPtr = void (*)(void);
//  PROTECTED_MEMORY_SECTION base::ProtectedMemory<FnPtr> fnPtr;
//  FnPtr ResolveFnPtr(void) {
//    // The Initializer nested class is a helper class for creating a static
//    // initializer for a ProtectedMemory variable. It implicitly sets the
//    // variable read-write during initialization.
//    static base::ProtectedMemory<FnPtr>::Initializer I(&fnPtr,
//      reinterpret_cast<FnPtr>(dlsym(/* ... */)));
//    return *fnPtr;
//  }

#ifndef BASE_MEMORY_PROTECTED_MEMORY_H_
#define BASE_MEMORY_PROTECTED_MEMORY_H_

#include "base/check_op.h"
#include "base/logging.h"
#include "base/memory/protected_memory_buildflags.h"
#include "base/memory/raw_ref.h"
#include "base/no_destructor.h"
#include "base/synchronization/lock.h"
#include "build/build_config.h"

#if BUILDFLAG(PROTECTED_MEMORY_ENABLED)
#if BUILDFLAG(IS_WIN)
// Define a read-write prot section. The $a, $mem, and $z 'sub-sections' are
// merged alphabetically so $a and $z are used to define the start and end of
// the protected memory section, and $mem holds protected variables.
// (Note: Sections in Portable Executables are equivalent to segments in other
// executable formats, so this section is mapped into its own pages.)
#pragma section("prot$a", read, write)
#pragma section("prot$mem", read, write)
#pragma section("prot$z", read, write)

// We want the protected memory section to be read-only, not read-write so we
// instruct the linker to set the section read-only at link time. We do this
// at link time instead of compile time, because defining the prot section
// read-only would cause mis-compiles due to optimizations assuming that the
// section contents are constant.
#pragma comment(linker, "/SECTION:prot,R")

__declspec(allocate("prot$a"))
__declspec(selectany) char __start_protected_memory;
__declspec(allocate("prot$z"))
__declspec(selectany) char __stop_protected_memory;

#define PROTECTED_MEMORY_SECTION __declspec(allocate("prot$mem"))
#else
#error "Protected Memory is currently only supported on Windows."
#endif  // BUILDFLAG(IS_WIN)

#else
#define PROTECTED_MEMORY_SECTION
#endif  // BUILDFLAG(PROTECTED_MEMORY_ENABLED)

namespace base {

template <typename T>
class AutoWritableMemory;

template <typename T>
class ProtectedMemory {
 public:
  ProtectedMemory() = default;
  ProtectedMemory(const ProtectedMemory&) = delete;
  ProtectedMemory& operator=(const ProtectedMemory&) = delete;

  // Expose direct access to the encapsulated variable
  const T& operator*() const { return data; }
  const T* operator->() const { return &data; }

  // Helper class for creating simple ProtectedMemory static initializers.
  class Initializer {
   public:
    // Defined out-of-line below to break circular definition dependency between
    // ProtectedMemory and AutoWritableMemory.
    Initializer(ProtectedMemory<T>* PM, T&& Init);

    Initializer() = delete;
    Initializer(const Initializer&) = delete;
    Initializer& operator=(const Initializer&) = delete;
  };

 private:
  friend class AutoWritableMemory<T>;
  T data;
};

namespace internal {
#if BUILDFLAG(PROTECTED_MEMORY_ENABLED)
// CHECK that the byte at |ptr| is read-only.
BASE_EXPORT void AssertMemoryIsReadOnly(const void* ptr);

// Abstract out platform-specific memory APIs. |end| points to the byte past
// the end of the region of memory having its memory protections changed.
BASE_EXPORT bool SetMemoryReadWrite(void* start, void* end);
BASE_EXPORT bool SetMemoryReadOnly(void* start, void* end);

// Abstract out platform-specific methods to get the beginning and end of the
// PROTECTED_MEMORY_SECTION. ProtectedMemoryEnd returns a pointer to the byte
// past the end of the PROTECTED_MEMORY_SECTION.
inline constexpr void* ProtectedMemoryStart = &__start_protected_memory;
inline constexpr void* ProtectedMemoryEnd = &__stop_protected_memory;
#endif  // BUILDFLAG(PROTECTED_MEMORY_ENABLED)

#if defined(COMPONENT_BUILD)
// For component builds we want to define a separate global writers variable
// (explained below) in every dynamic shared object (DSO) that includes this
// header. To do that we use this template to define a global without duplicate
// symbol errors.
template <typename T>
struct DsoSpecific {
  static T value;
};
template <typename T>
T DsoSpecific<T>::value = 0;
#endif  // defined(COMPONENT_BUILD)
}  // namespace internal

// A class that sets a given ProtectedMemory variable writable while the
// AutoWritableMemory is in scope. This class implements the logic for setting
// the protected memory region read-only/read-write in a thread-safe manner.
template <typename T>
class AutoWritableMemory {
 public:
  // If this is the first writer (e.g. writers == 0) set the writers variable
  // read-write. Next, increment writers and set the requested memory writable.
  explicit AutoWritableMemory(ProtectedMemory<T>& protected_memory)
      : protected_memory_(protected_memory) {
#if BUILDFLAG(PROTECTED_MEMORY_ENABLED)
    T* ptr = &(protected_memory.data);
    T* ptr_end = ptr + 1;
    CHECK(ptr >= internal::ProtectedMemoryStart &&
          ptr_end <= internal::ProtectedMemoryEnd);

    {
      base::AutoLock auto_lock(writers_lock());
      if (writers == 0) {
        internal::AssertMemoryIsReadOnly(ptr);
#if !defined(COMPONENT_BUILD)
        internal::AssertMemoryIsReadOnly(&writers);
        CHECK(internal::SetMemoryReadWrite(&writers, &writers + 1));
#endif  // !defined(COMPONENT_BUILD)
      }

      writers++;
    }

    CHECK(internal::SetMemoryReadWrite(ptr, ptr_end));
#endif  // BUILDFLAG(PROTECTED_MEMORY_ENABLED)
  }

  // On destruction decrement writers, and if no other writers exist, set the
  // entire protected memory region read-only.
  ~AutoWritableMemory() {
#if BUILDFLAG(PROTECTED_MEMORY_ENABLED)
    base::AutoLock auto_lock(writers_lock());
    CHECK_GT(writers, 0);
    writers--;

    if (writers == 0) {
      CHECK(internal::SetMemoryReadOnly(internal::ProtectedMemoryStart,
                                        internal::ProtectedMemoryEnd));
#if !defined(COMPONENT_BUILD)
      internal::AssertMemoryIsReadOnly(&writers);
#endif  // !defined(COMPONENT_BUILD)
    }
#endif  // BUILDFLAG(PROTECTED_MEMORY_ENABLED)
  }

  AutoWritableMemory(AutoWritableMemory& original) = delete;
  AutoWritableMemory& operator=(AutoWritableMemory& original) = delete;
  AutoWritableMemory(AutoWritableMemory&& original) = delete;
  AutoWritableMemory& operator=(AutoWritableMemory&& original) = delete;

  T& GetProtectedData() { return protected_memory_->data; }
  T* GetProtectedDataPtr() { return &(protected_memory_->data); }

 private:
  // 'writers' is a global holding the number of ProtectedMemory instances set
  // writable, used to avoid races setting protected memory readable/writable.
  // When this reaches zero the protected memory region is set read only.
  // Access is controlled by writers_lock.
#if defined(COMPONENT_BUILD)
  // For component builds writers is a reference to an int defined separately in
  // every DSO.
  static constexpr int& writers = internal::DsoSpecific<int>::value;
#else
  // Otherwise, we declare writers in the protected memory section to avoid the
  // scenario where an attacker could overwrite it with a large value and invoke
  // code that constructs and destructs an AutoWritableMemory. After such a call
  // protected memory would still be set writable because writers > 0.
  static int writers;
#endif  // defined(COMPONENT_BUILD)

  // Synchronizes access to the writers variable and the simultaneous actions
  // that need to happen alongside writers changes, e.g. setting the protected
  // memory region readable when writers is decremented to 0.
  static Lock& writers_lock() {
    static NoDestructor<Lock> writers_lock;
    return *writers_lock;
  }

  const raw_ref<ProtectedMemory<T>> protected_memory_;
};

#if !defined(COMPONENT_BUILD)
template <typename T>
PROTECTED_MEMORY_SECTION int AutoWritableMemory<T>::writers = 0;
#endif  // !defined(COMPONENT_BUILD)

template <typename T>
ProtectedMemory<T>::Initializer::Initializer(
    ProtectedMemory<T>* protected_memory,
    T&& initial_value) {
  AutoWritableMemory<T> writer(*protected_memory);
  writer.GetProtectedData() = std::forward<T>(initial_value);
}

}  // namespace base

#endif  // BASE_MEMORY_PROTECTED_MEMORY_H_
