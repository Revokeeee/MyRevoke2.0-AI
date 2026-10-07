# ImGui

Integrates [Dear ImGui](https://github.com/ocornut/imgui) (plus ImGuizmo) into the engine as an
overlay `Layer`: context setup/teardown, per-frame begin/end, a custom dark theme, and event-blocking
so ImGui can consume mouse/keyboard input before the rest of the app sees it.

## Files

- **`ImGuiBuild.cpp`** — a translation-unit shim that compiles the GLFW + OpenGL3 ImGui backend
  implementation files (`imgui_impl_glfw.cpp`, `imgui_impl_opengl3.cpp`) into the engine, using
  `IMGUI_IMPL_OPENGL_LOADER_GLAD`.
- **`ImGuiLayer.h`/`.cpp`** — `ImGuiLayer : public Layer`. `OnAttach()` creates the ImGui context,
  enables docking/viewports/keyboard nav, loads the fonts and applies `Theme`. `OnEvent(Event&)`
  marks mouse/keyboard events handled when ImGui wants capture. `Begin()`/`End()` wrap ImGui's
  per-frame `NewFrame`/`Render` plus multi-viewport platform window updates. `OnImGuiDraw()` itself is
  empty — actual widgets are drawn by other layers between `Begin()`/`End()`.
  `GetFont(FontStyle)` returns the Regular, Bold, Large or Mono font. They are Segoe UI, Segoe UI
  Semibold and Cascadia Mono/Consolas from `%WINDIR%\Fonts`, scaled by the monitor's content scale,
  with ImGui's built-in font as the fallback.
- **`ImGuiTheme.h`/`.cpp`** — the editor palette (`Theme::Accent`, `Theme::AxisX`, `Theme::Error`, ...)
  and `Theme::Apply()`, which sets every ImGui color and size. Panels use these constants instead of
  hard-coded colors.
- **`ImGuiIcons.h`** — `RV_ICON_*` string macros for Segoe Fluent Icons (Windows 11) / Segoe MDL2
  Assets (Windows 10) glyphs, merged into every font, so labels can mix them: `RV_ICON_SAVE " Save"`.
  Only glyphs listed in `RV_ICONS_ALL` are baked into the atlas.

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
