# MyRevoke-ScriptCore

The C# side of the engine's [Mono scripting system](../MyRevoke/src/MyRevoke/Scripting/README.md). A
Premake `SharedLib` C# project (`dotnetframework "4.7.2"`) that compiles to
`RevokeCraft/resourses/scripts/MyRevoke-ScriptCore.dll`, which `ScriptCore::Init()` loads into the
embedded Mono runtime at startup.

## Files

- **`Source/main.cs`** — the C# API surface exposed to (future) user scripts:
  - `static class InternalCalls` — declares `CppFunction()` with
    `[MethodImplAttribute(MethodImplOptions.InternalCall)]`, the C# half of the internal-call binding
    registered on the C++ side by `Scripting/ScriptConnector.cpp`.
  - `class Entity` — a `FloatVar` property, a constructor that logs and calls
    `InternalCalls.CppFunction()`, and demo methods `PrintMessege()`/`PrintCustomMessege(int)`.

This is currently scaffolding/proof-of-concept rather than a finished scripting API: there's no
`GetComponent<T>`, transform access, or per-entity dispatch equivalent to the native scripting path's
`ScriptEntity` (see the Scripting module README for the full picture, including why the Mono path
isn't runtime-usable yet).
