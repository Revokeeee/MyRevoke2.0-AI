# ImGui

Integrates [Dear ImGui](https://github.com/ocornut/imgui) (plus ImGuizmo) into the engine as an
overlay `Layer`: context setup/teardown, per-frame begin/end, a custom dark theme, and event-blocking
so ImGui can consume mouse/keyboard input before the rest of the app sees it.

## Files

- **`ImGuiBuild.cpp`** — a translation-unit shim that compiles the GLFW + OpenGL3 ImGui backend
  implementation files (`imgui_impl_glfw.cpp`, `imgui_impl_opengl3.cpp`) into the engine, using
  `IMGUI_IMPL_OPENGL_LOADER_GLAD`.
- **`ImGuiLayer.h`/`.cpp`** — `ImGuiLayer : public Layer`. `OnAttach()` creates the ImGui context,
  enables docking/viewports/keyboard nav, and applies a custom dark color theme. `OnEvent(Event&)`
  marks mouse/keyboard events handled when ImGui wants capture. `Begin()`/`End()` wrap ImGui's
  per-frame `NewFrame`/`Render` plus multi-viewport platform window updates. `OnImGuiDraw()` itself is
  empty — actual widgets are drawn by other layers between `Begin()`/`End()`.

`Application` creates this layer and pushes it as the last overlay, so it draws (and receives events)
last/first respectively — see `Core/README.md`.

## Known issues

- Includes `MouseEvent.h`/`KeyEvent.h`/`AppEvent.h` but only ever uses the base `Event` class and its
  category flags — likely unnecessary includes.
- `m_Time` is declared but never read or written (presumably meant for delta-time tracking, never
  wired up).
- `OnEvent` combines booleans with bitwise `&` instead of `&&` — works because both operands are 0/1,
  but easy to misread.
- The GLSL version string passed to the ImGui OpenGL3 backend is hardcoded to `"#version 410"`, tying
  it to OpenGL 4.1 with no configurability.
