# SandBox

A minimal, throwaway [`MyRevoke`](../MyRevoke/README.md) application used to smoke-test engine
features in isolation, outside the full `RevokeCraft` editor. Its own Premake `ConsoleApp` project,
with `Shaders/` and `Textures/` asset folders for exercising 2D rendering directly.

## Files

- **`src/SandBoxApp.cpp`** — a trivial `Revoke::Application` subclass (`SandBox`) that pushes one
  layer, `Renderer2DTest`, and implements `Revoke::CreateApplication()`.
- **`src/Sandbox2D.h`/`.cpp`** — the `Renderer2DTest` layer. Currently every lifecycle override
  (`OnAttach`, `OnDetach`, `OnUpdate`, `OnImGuiDraw`, `OnEvent`) is an empty stub — the sandbox
  presently renders nothing. `Shaders/Main.shader` (a batch-renderer-style shader supporting per-vertex
  color, texcoords, and up to 32 indexed samplers) and the `Textures/` PNGs suggest the intent was to
  exercise `Renderer2D`'s batching and texturing, but that code hasn't been written yet.

## Known issues

- `SandBoxApp.cpp` includes `"SandBox2D.h"` (capital B) while the file on disk is `Sandbox2D.h`
  (lowercase) — compiles today only because Windows/MSVC filesystems are case-insensitive; would break
  on a case-sensitive filesystem.
- `Main.shader`'s fragment shader dynamically indexes `u_Textures[int(v_TexIndex)]` with a
  non-constant index, which isn't guaranteed to work under GLSL 330 core without GL 4.0+/
  `GL_ARB_gpu_shader5` — a latent portability issue once the sandbox actually renders.
