# MyRevoke (engine library)

This is the engine itself: a static library (`kind "StaticLib"` in `premake5.lua`) that every
application in the repo (`RevokeCraft`, `SandBox`) links against. It owns the application
lifecycle, windowing, rendering, the ECS scene layer, scripting, and audio.

## Layout

| Directory | Responsibility |
|---|---|
| [`src/MyRevoke/Core`](src/MyRevoke/Core/README.md) | Application entry point, main loop, window abstraction, layer stack, logging, input, UUIDs |
| [`src/MyRevoke/EventSystem`](src/MyRevoke/EventSystem/README.md) | Header-only, synchronous event types (window/keyboard/mouse) and dispatch helper |
| [`src/MyRevoke/Renderer`](src/MyRevoke/Renderer/README.md) | OpenGL abstraction: buffers, shaders, textures, framebuffers, the `Renderer2D` batch renderer, cameras |
| [`src/MyRevoke/Scene`](src/MyRevoke/Scene/README.md) | EnTT-based ECS: `Scene`/`Entity` wrapper, component definitions, YAML serialization, Box2D physics step |
| [`src/MyRevoke/Scripting`](src/MyRevoke/Scripting/README.md) | Two scripting paths: hot-reloaded native C++ scripts and an embedded Mono/C# runtime |
| [`src/MyRevoke/AudioManager`](src/MyRevoke/AudioManager/README.md) | OpenAL + libsndfile wrapper for positional sound playback |
| [`src/MyRevoke/ImGui`](src/MyRevoke/ImGui/README.md) | Dear ImGui integration as an overlay `Layer` |
| [`src/MyRevoke/Math`](src/MyRevoke/Math/README.md) | Small glm-based matrix decomposition helper used by the editor |
| [`src/MyRevoke/Utility`](src/MyRevoke/Utility/README.md) | Win32 file-picker wrapper and GLFW-derived key/mouse code constants |
| `vendor/` | Third-party dependencies, mostly as git submodules (see root `.gitmodules`) |

`src/rvpch.h`/`rvpch.cpp` is the precompiled header (`pchheader`/`pchsource` in `premake5.lua`); most
`.cpp` files in this project include it first.

## How the pieces fit together

1. A client application (e.g. `RevokeCraft`) implements `Revoke::CreateApplication()` and is linked
   against this library (see [`Core/README.md`](src/MyRevoke/Core/README.md) for the full boot
   sequence via `EntryPoint.h`).
2. `Application` creates the `Window` (which owns the GLFW window and an OpenGL `RenderContex`),
   initializes `RendererAPI`/`Renderer2D`, `AudioRenderer`, and the native-script `ScriptEngine`, and
   pushes an `ImGuiLayer` overlay.
3. Client code pushes its own `Layer`s (e.g. `CraftLayer` in RevokeCraft) onto the `LayerStack`.
   Each layer typically owns a `Scene`, which holds the actual game/editor entities via EnTT.
4. Per frame, `Application::Run()` updates every layer, renders the ImGui pass, then polls window
   events and swaps buffers. Window input callbacks turn into `EventSystem` events that flow back
   down the layer stack (reverse order, so overlays like ImGui see them first).
5. `Scene::OnRuntimeUpdate` drives per-frame gameplay: native script callbacks, Box2D physics
   stepping, audio source updates, and handing entities to `Renderer2D` for drawing.

## Dependencies

Declared as submodules under `vendor/` and wired up via `IncludeDir`/`LibraryDir` tables in the root
`premake5.lua`: GLFW (windowing), GLAD (OpenGL loading), glm (math), Dear ImGui + ImGuizmo (UI/gizmos),
EnTT (ECS), yaml-cpp (serialization), Box2D (2D physics), spdlog (logging), stb_image (image loading),
OpenAL + libsndfile (audio), Mono (embedded C# runtime).

## Known quirks worth knowing about before you go digging

A handful of names in this tree don't match what they mean — useful to know so a search for the
"real" name doesn't come up empty:

- `Renderer/Cmaera.h` (base `Camera` class) and `Renderer/GraphicContex.h`/`RenderContex` (should read
  "Camera"/"GraphicsContext"/"RenderContext").
- `Scene/Serealizer.h`/`.cpp` (should read "Serializer") and `Scene/Components.h`'s
  `BoxColisionComponent` (should read "Collision").
- `Core/Log.h`'s `s_EditortLogger` (should read "Editor").

None of these are functional bugs — see each module's README for details and for the handful of
actual bugs found while exploring (dangling pointers, leaked buffers, etc.), which are called out
separately as "Known issues" rather than lumped in with naming.
