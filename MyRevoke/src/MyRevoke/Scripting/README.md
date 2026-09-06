# Scripting

Two independent, unfinished paths for attaching gameplay logic to entities: hot-reloaded native C++
scripts, and an embedded Mono/C# runtime. Both are wired up (in principle) through
[`Scene`'s `NativeScriptComponent`](../Scene/README.md), which today only actually drives the native
path.

## Native C++ scripting

- **`ScriptEntity.h`** — the base class a script derives from: `class PlayerScript : public
  Revoke::ScriptEntity`, overriding the protected virtuals `OnCreate()`, `OnUpdate(Timestep)`,
  `OnDestroy()` (all empty by default). Helper templates `GetComponent<T>()`/`HasComponent<T>()`
  proxy to an underlying `Entity`, set by `Scene` (a `friend class`).
- **`NativeScript.h`/`.cpp`** — `Revoke::ScriptEngine`, the loader. `InitDll()` locates and loads
  `resourses/scripts/Native/MyRevoke-NativeScriptCore.dll`; `OnUpdate()` polls the DLL's last-write
  time each frame and hot-reloads it via `OnDllUpdate()`; `GetScritpByName(name)` calls
  `GetProcAddress` for an exported factory function (e.g. `Player`) and invokes it to construct a
  `ScriptEntity*`.
- Real example: [`RevokeCraft/assets/Scripts/ScriptExample.h`/`.cpp`](../../../../RevokeCraft/assets/Scripts)
  — a `PlayerScript` plus an exported `extern "C" __declspec(dllexport) ScriptEntity* Player()`
  factory, which is what `GetScritpByName("Player")` finds.
- The actual DLL project that gets built and hot-loaded is
  [`MyRevoke-NativeScriptCore`](../../../../MyRevoke-NativeScriptCore/README.md) — a separate Premake
  project, independent from the engine/editor build.

Hot-reload mechanism: `NativeScript.cpp`'s `patchDLL` copies the freshly-built DLL and rewrites its
embedded PDB path (renaming to e.g. `MyRevoke-NativeScriptCor_.dll`) so MSVC doesn't hold a lock on
the original PDB, letting the game keep running while scripts are rebuilt.

## Mono/C# scripting

- **`ScriptCore.h`/`.cpp`** — `ScriptCore::Init()` points Mono at `mono/lib`, calls `mono_jit_init`,
  loads the compiled managed assembly from `resourses/scripts/MyRevoke-ScriptCore.dll` (see
  [`MyRevoke-ScriptCore`](../../../../MyRevoke-ScriptCore/README.md)), then calls
  `ScriptConnector::ConnectFunctions()` and demonstrates instantiating `Revoke.Entity` and invoking a
  couple of its methods by hardcoded name. **This is demo/proof-of-concept code** — there is no
  per-`NativeScriptComponent` dispatch equivalent to the native path's `GetScritpByName`, so C#
  scripts aren't actually driven by `Scene` yet.
- **`ScriptConnector.h`/`.cpp`** — registers C++ functions Mono can call from C# via
  `mono_add_internal_call` (currently just one demo function, `Revoke.InternalCalls::CppFunction`).
  This is the mechanism for C#-calls-into-C++; it mirrors an `[MethodImplOptions.InternalCall]` extern
  declaration on the C# side.

## Known issues

- The Mono path is not runtime-usable yet — see above.
- `ScriptEngine::ShutDown()` calls `getchar()`, blocking the engine on console input during shutdown —
  almost certainly leftover debug code.
- Nothing calls `ScriptEntity::OnDestroy()` or deletes `NativeScriptComponent::Instance` — native
  script instances leak and never get their destroy hook (see also `Scene`'s "Known issues").
- `ScriptEngine::GetScritpByName` is a persistent typo for "Script" (also present in
  `Scene::OnRuntimeUpdate`'s call site and an error log: `"Failed to read calss"`).
- `loadDLL()` in `NativeScript.cpp` checks `dllPath == nullptr` *after* already calling
  `LoadLibraryA(dllPath)` — the null check can never do anything useful.
- Mono's internal-call surface is essentially empty (one demo function) — C# scripts currently have no
  access to entity/transform/input APIs.
- `MyRevoke-NativeScriptCore/src/dll.cpp`/`.h` (`tick()`, prints `"asd\n"`) is unrelated leftover
  scaffolding — it doesn't match the real hot-reload contract (`Player()`-style exported factories)
  used by `ScriptExample.cpp`.
