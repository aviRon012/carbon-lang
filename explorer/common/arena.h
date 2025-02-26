// Part of the Carbon Language project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#ifndef CARBON_EXPLORER_COMMON_ARENA_H_
#define CARBON_EXPLORER_COMMON_ARENA_H_

#include <memory>
#include <type_traits>
#include <vector>

#include "explorer/common/nonnull.h"

namespace Carbon {

class Arena {
 public:
  // Values of this type can be passed as the first argument to New in order to
  // have the address of the created object written to the given pointer before
  // the constructor is run. This is used during cloning to support pointer
  // cycles within the AST.
  template <typename T>
  struct WriteAddressTo {
    Nonnull<T**> target;
  };
  template <typename T>
  WriteAddressTo(T** target) -> WriteAddressTo<T>;

  // Allocates an object in the arena, returning a pointer to it.
  template <
      typename T, typename... Args,
      typename std::enable_if_t<std::is_constructible_v<T, Args...>>* = nullptr>
  auto New(Args&&... args) -> Nonnull<T*> {
    auto smart_ptr =
        std::make_unique<ArenaEntryTyped<T>>(std::forward<Args>(args)...);
    Nonnull<T*> ptr = smart_ptr->Instance();
    arena_.push_back(std::move(smart_ptr));
    return ptr;
  }

  // Allocates an object in the arena, writing its address to the given pointer.
  template <
      typename T, typename U, typename... Args,
      typename std::enable_if_t<std::is_constructible_v<T, Args...>>* = nullptr>
  void New(WriteAddressTo<U> addr, Args&&... args) {
    arena_.push_back(std::make_unique<ArenaEntryTyped<T>>(
        addr, std::forward<Args>(args)...));
  }

 private:
  // Virtualizes arena entries so that a single vector can contain many types,
  // avoiding templated statics.
  class ArenaEntry {
   public:
    virtual ~ArenaEntry() = default;
  };

  // Templated destruction of a pointer.
  template <typename T>
  class ArenaEntryTyped : public ArenaEntry {
   public:
    struct WriteAddressHelper {};

    template <typename... Args>
    explicit ArenaEntryTyped(Args&&... args)
        : instance_(std::forward<Args>(args)...) {}

    template <typename... Args>
    explicit ArenaEntryTyped(WriteAddressHelper, Args&&... args)
        : ArenaEntryTyped(std::forward<Args>(args)...) {}

    template <typename U, typename... Args>
    explicit ArenaEntryTyped(WriteAddressTo<U> write_address, Args&&... args)
        : ArenaEntryTyped(
              (*write_address.target = &instance_, WriteAddressHelper{}),
              std::forward<Args>(args)...) {}

    auto Instance() -> Nonnull<T*> { return Nonnull<T*>(&instance_); }

   private:
    T instance_;
  };

  // Manages allocations in an arena for destruction at shutdown.
  std::vector<std::unique_ptr<ArenaEntry>> arena_;
};

}  // namespace Carbon

#endif  // CARBON_EXPLORER_COMMON_ARENA_H_
