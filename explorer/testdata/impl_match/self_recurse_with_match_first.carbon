// Part of the Carbon Language project, under the Apache License v2.0 with LLVM
// Exceptions. See /LICENSE for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// AUTOUPDATE
// RUN: %{explorer-run}
// RUN: %{explorer-run-trace}
// CHECK:STDOUT: result: 0

package ExplorerTest api;

interface Foo {}

__match_first {
  impl forall [T:! type where .Self == i32*] T as Foo {}
  impl forall [T:! type where .Self* is Foo] T as Foo {}
}

fn F[T:! Foo](x: T) {}

fn Main() -> i32 {
  // This is OK: we don't recursively consider the second `impl`, because the
  // first one matches.
  F(0);
  return 0;
}
