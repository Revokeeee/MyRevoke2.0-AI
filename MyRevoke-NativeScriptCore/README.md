# MyRevoke-NativeScriptCore

A separate Premake project (`kind "SharedLib"`) that builds the DLL loaded and hot-reloaded at
runtime by the engine's [native scripting system](../MyRevoke/src/MyRevoke/Scripting/README.md)
(`Scripting/NativeScript.h`'s `ScriptEngine`). It's built independently of the engine/editor and
output straight into `RevokeCraft/resourses/scripts/Native`, which is why RevokeCraft can rebuild and
hot-swap it ("Build Scripts" in `SceneSettingsPannel`) without restarting.

The build includes both `src/**` (this project) and the example project's
`RevokeCraft/projects/Example/assets/Scripts/**` (see the `premake5.lua` `files` block for this
project) — so the actual gameplay scripts, like `ScriptExample.h`/`.cpp`, are compiled as part of
this DLL, not the engine or editor. The scripts folder is hardcoded to the example project; other
projects cannot have their scripts compiled yet.

## Files

- **`src/dll.h`/`.cpp`** — declares/defines `extern "C" __declspec(dllexport) bool tick()`. This is
  leftover placeholder scaffolding (`tick()` just does `printf("asd\n")`) — it's unrelated to the real
  hot-reload contract, which is per-script exported factory functions like `Player()` in
  `ScriptExample.cpp` (see the Scripting module README for how `ScriptEngine::GetScritpByName`
  resolves those).

## Known issues

- `dll.cpp`/`dll.h` don't do anything useful and could be removed or replaced with a real shared
  entry point once one is needed.
