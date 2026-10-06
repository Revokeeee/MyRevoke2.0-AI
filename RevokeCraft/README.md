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
| `CraftLayer.h`/`.cpp` | Dockspace, menu bar, viewport, gizmo manipulation | Owns the `Scene`, `EditorCamera`, framebuffer (for mouse picking via `ReadPixel`), and all the other panels; scene file I/O via `Serializer`/`FileExplorer` |
| `ObjectsPannel.h`/`.cpp` *(sic — "Panel")* | "Scene Hierarchy" + "Properties" | Iterates `Scene`'s registry; one collapsible section per component (`DrawComponent<T>`) with Reset/Remove, and an "Add Component" menu |
| `EditorUI.h`/`.cpp` | Shared widgets | Label/value property rows (`UI::Property*`), X/Y/Z fields, asset drop fields, search box, icon buttons, tooltips |
| `AssetType.h` | — | Maps a file extension to an `AssetType` and its icon; the Content Browser drag-and-drop payload name |
| `ContentBrowser.h`/`.cpp` | "Content Browser" | Icon grid over the open project's assets tree; drag-and-drop source for textures/audio/scripts |
| `SceneSettingsPannel.h`/`.cpp` *(sic)* | "Scene Settings" | Clear color, blending toggle, physics iteration counts, "Build Scripts" (`msbuild` on `MyRevoke-NativeScriptCore`; not in `Dist` builds) |
| `ToolBar.h`/`.cpp` | Gizmo tools (Q/W/E/R), local/world and snap toggles, Play/Stop (Ctrl+P) | Owns the gizmo state `CraftLayer` reads; calls back into `CraftLayer` to start/stop play mode |

`CraftLayer` owns all four panels as plain members (composition, not polymorphism); `OnAttach()` wires
shared state (the `Scene`, the play callbacks) into them, and `OnImGuiDraw()` calls each panel's
`OnImGuiRender()` in sequence. Whenever the scene changes (new/open), `CraftLayer` manually re-pushes
the new `Scene` into every panel — there's no observer/event pattern for this.

## Projects, assets and resources

User content lives in a [`Project`](../MyRevoke/src/MyRevoke/Project/README.md) — a `<Name>.mrproject`
file plus an `assets/` folder — in a folder the user picks ("New Project"/"Open Project" in the File
menu). `CraftLayer` owns the open project and pushes its assets directory into the content browser and
the properties panel; scene save/load goes to the project's `assets/Scenes`.

The editor's own files live next to `RevokeCraft.exe` (the Premake `targetdir` for this project is the
`RevokeCraft/` folder itself) and are opened through `GetExecutableDirectory()`, so the editor can be
started from any working directory.

- `resourses/` *(sic — "resources")*: `shaders/Main.shader` (the engine's renderer shader), `icons/`
  for the content browser, and `scripts/Native` where the compiled
  `MyRevoke-NativeScriptCore` DLL is hot-loaded from at runtime.
- `mono/` — the Mono runtime distribution needed for embedded C# scripting.
- `projects/Example/` — the example project (the old `assets/` content), opened on startup.

## Known issues

- `SceneSettingsPannel.cpp` has a stray semicolon after an `if (ImGui::ColorEdit3(...))`, so the
  following `RendererAPI::SetClearColor(...)` runs unconditionally every frame instead of only on
  change.
- `SceneSettingsPannel::SetScene` allocates a new `char[]` for the scene name on every scene load
  without freeing the previous buffer — a leak on every "New"/"Open" — and its `ImGui::InputText` call
  uses `sizeof(sceneName)` (the pointer size, 8 bytes) instead of the buffer's real length.
- `SceneSettingsPannel.cpp` shells out via `system("msbuild ...")` to rebuild native scripts — Windows-
  only and echoes a hardcoded path.
- The example project ships inside the editor's own folder, so saving a scene in it writes to the
  install folder. Only user projects created elsewhere stay out of it.
- `premake5.lua` compiles native scripts from the example project's `assets/Scripts` only, so scripts
  in other projects are not built.
- `CraftLayer.cpp` has a `// TODO: Fix the picking in a play mode!!!` — mouse picking is known-broken
  during Runtime scene state.
