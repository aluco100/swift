// RUN: %empty-directory(%t)
// RUN: %empty-directory(%t/hidden)
// RUN: split-file %s %t
// RUN: sed -e "s@VFS_DIR@%{/t:regex_replacement}/vfs@g" -e "s@EXTERNAL_DIR@%{/t:regex_replacement}/hidden@g" %t/base.yaml > %t/overlay.yaml

// RUN: %host-build-swift -swift-version 5 -I %swift-host-lib-dir -L %swift-host-lib-dir -emit-library -o %t/hidden/%target-library-name(MacroDefinition) -module-name=MacroDefinition %S/Inputs/syntax_macro_definitions.swift -g -no-toolchain-stdlib-rpath -swift-version 5

// Check that loading plugins respects VFS overlays
// RUN: %target-swift-frontend -typecheck -verify -swift-version 5 -enable-experimental-feature Macros -load-plugin-library %t/vfs/%target-library-name(MacroDefinition) -I %swift-host-lib-dir -module-name MacroUser -DTEST_DIAGNOSTICS %t/macro.swift -vfsoverlay %t/overlay.yaml
// RUN: %target-swift-frontend -typecheck -verify -swift-version 5 -enable-experimental-feature Macros -plugin-path %t/vfs -I %swift-host-lib-dir -module-name MacroUser -DTEST_DIAGNOSTICS %t/macro.swift -vfsoverlay %t/overlay.yaml

// FIXME: Swift parser is not enabled on Linux CI yet.
// REQUIRES: OS=macosx

//--- macro.swift
// expected-no-diagnostics
@freestanding(expression) macro customFileID() -> String = #externalMacro(module: "MacroDefinition", type: "FileIDMacro")
let _ = #customFileID

//--- base.yaml
{
  version: 0,
  roots: [
    {
      type: "directory-remap",
      name: "VFS_DIR",
      external-contents: "EXTERNAL_DIR"
    }
  ]
}

