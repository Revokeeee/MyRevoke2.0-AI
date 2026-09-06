# RevokeCraft

The flagship editor/IDE application for the MyRevoke engine — a scene/level editor (Unity/Hazel-style)
built on top of the [`MyRevoke`](../MyRevoke/README.md) engine library. It's a Premake `ConsoleApp`
project (`startproject "RevokeCraft"` in the root `premake5.lua`) that links against `MyRevoke`.

## Entry point

`src/App.cpp` defines `class RevokeCraft : public Revoke::Application`, whose constructor pushes a
single `CraftLayer`, and implements the `Revoke::CreateApplication()` factory required by the engine's
`EntryPoint.h`. Everything else lives in `CraftLayer` and the panels it composes.

## Panels (`src/`)

| File | Draws | Talks to |
|---|---|---|
| `CraftLayer.h`/`.cpp` | Dockspace, menu bar, viewport, gizmo manipulation | Owns the `Scene`, `EditorCamera`, framebuffer (for mouse picking via `ReadPixel`), and all the other panels; scene file I/O via `Serealizer`/`FileExplorer` |
| `ObjectsPannel.h`/`.cpp` *(sic — "Panel")* | "Scene Hierarchy" + "Properties" | Iterates `Scene`'s registry, per-component property editors, "Add Component" popup |
| `ContentBrowser.h`/`.cpp` | "Content Browser" | Icon grid over the `assets/` directory tree; drag-and-drop source for textures/audio/scripts |
| `SceneSettingsPannel.h`/`.cpp` *(sic)* | "Scene Settings" | Clear color, blending toggle, physics iteration counts, "Build Scripts" (`msbuild` on `MyRevoke-NativeScriptCore`) |
| `ToolBar.h`/`.cpp` | Play/Stop + gizmo mode (Q/W/E/R) toolbar | `Scene::OnRuntimeStart`/`OnRuntimeStop`, writes into a shared gizmo-type pointer |

`CraftLayer` owns all four panels as plain members (composition, not polymorphism); `OnAttach()` wires
shared state (the `Scene`, the gizmo pointer) into them, and `OnImGuiDraw()` calls each panel's
`OnImGuiRender()` in sequence. Whenever the scene changes (new/open), `CraftLayer` manually re-pushes
the new `Scene` into every panel — there's no observer/event pattern for this.

## Assets and resources

- `assets/Shaders`, `assets/Textures`, `assets/Scenes` (`.myrevoke` serialized scenes),
  `assets/Scripts` (the reference `ScriptExample.h`/`.cpp` native script — see
  [`Scripting/README.md`](../MyRevoke/src/MyRevoke/Scripting/README.md)), `assets/Audio`.
- `resourses/` *(sic — "resources")*: `icons/` for the content browser and toolbar, and
  `scripts/Native` where the compiled `MyRevoke-NativeScriptCore` DLL is hot-loaded from at runtime.
- `mono/` — the Mono runtime distribution needed for embedded C# scripting.

## Known issues

- `SceneSettingsPannel.cpp` has a stray semicolon after an `if (ImGui::ColorEdit3(...))`, so the
  following `RendererAPI::SetClearColor(...)` runs unconditionally every frame instead of only on
  change.
- `SceneSettingsPannel::SetScene` allocates a new `char[]` for the scene name on every scene load
  without freeing the previous buffer — a leak on every "New"/"Open" — and its `ImGui::InputText` call
  uses `sizeof(sceneName)` (the pointer size, 8 bytes) instead of the buffer's real length.
- `SceneSettingsPannel.cpp` shells out via `system("msbuild ...")` to rebuild native scripts — Windows-
  only and echoes a hardcoded path.
- `ObjectsPannel.cpp`'s audio drag-drop payload check compares against extension `L".wov"`, almost
  certainly meant to be `.wav`.
- `ContentBrowser.cpp` and `CraftLayer.cpp` each independently declare
  `const std::filesystem::path g_AssetsDirectory = "assets"` at namespace scope; only one is `extern`,
  so this is duplicate global state that happens to work today.
- `CraftLayer.cpp` has a `// TODO: Fix the picking in a play mode!!!` — mouse picking is known-broken
  during Runtime scene state.
