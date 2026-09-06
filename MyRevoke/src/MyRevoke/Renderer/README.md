# Renderer

The engine's OpenGL abstraction layer: RAII wrappers around raw GL objects, a stateless low-level
draw-call API, a 2D batch renderer, and the camera types that feed it view/projection matrices.

## Files

- **`BuffersAPI.h`/`.cpp`** — `BufferLayout`/`BufferElement` describe vertex attribute layout;
  `VertexBuffer`, `IndexBuffer`, `VertexArray` wrap `glCreateBuffers`/`glCreateVertexArrays`.
- **`Cmaera.h`** *(sic — "Camera")* — base `Camera` class, holds only a projection matrix.
- **`EditorCamera.h`/`.cpp`** — arcball-style camera used by the RevokeCraft viewport: orbits a focal
  point via pitch/yaw/distance, with mouse-driven pan/rotate/zoom.
- **`FrameBuffers.h`/`.cpp`** — FBO wrapper with configurable color/depth attachments (including a
  `RED_INTEGER` attachment used for entity-ID mouse picking), MSAA support, and `Resize`/`ReadPixel`.
- **`GraphicContex.h`/`.cpp`** *(sic — "GraphicsContext")* — `RenderContex` initializes GLAD against a
  GLFW window and performs `SwapBuffers`.
- **`Renderer2D.h`/`Renderer2d.cpp`** — static batch renderer: `Init`/`Shutdown`, `Begin(camera)`/`End`,
  several `DrawQuad`/`DrawSprite` overloads, `GetStats`.
- **`RendererAPI.h`/`.cpp`** — thin static wrapper over raw GL calls (`SetClearColor`, `Clear`,
  `DrawElements`, `EnableBlending`, `WindowResize`).
- **`Shader.h`/`.cpp`** — loads/compiles/links GLSL from a single file using `#type vertex`/
  `#type fragment` section markers, or from raw strings; caches uniform locations.
- **`Texture.h`/`.cpp`** — loads an image via stb_image, or creates a blank RGBA texture; binds via
  `glBindTextureUnit` (DSA).

Related but defined elsewhere: `Scene/SceneCamera.h` (also derives from `Camera`) models a
runtime/gameplay camera with switchable orthographic/perspective projection — unlike `EditorCamera`,
it has no view/position logic of its own (that comes from the entity's `TransformComponent`).

## Draw-call flow

`Renderer2D::Init` builds one shared quad `VertexArray` + dynamic `VertexBuffer`/`IndexBuffer` and
loads `assets/Shaders/Main.shader`. From there, per frame:

1. `Begin(camera)` binds the shader, uploads the view-projection matrix, resets the CPU-side vertex
   cursor.
2. Each `DrawQuad(...)` call writes directly into a CPU-side `QuadVertex[]` array (position, color,
   UV, texture index, entity ID) — no GL calls happen here. A texture is assigned a slot in a shared
   texture-slot array (reused if it's already bound this batch); if the batch fills up (max quads or
   max textures), `NewBatch()` flushes early via `End()`.
3. `End()` uploads the whole CPU buffer in one `glBufferSubData` call, binds every texture used this
   batch, and calls `RendererAPI::DrawElements`, which issues the actual `glDrawElements`.

So: `Renderer2D::DrawQuad` → CPU vertex buffer → `Renderer2D::End` → `Shader`/`Texture::Bind` →
`RendererAPI::DrawElements` → bound `VertexArray`/`IndexBuffer` state → OpenGL.

## Known issues

- File/class names carry a few typos: `Cmaera.h` ("Camera"), `GraphicContex.h`/`RenderContex`
  ("GraphicsContext"/"RenderContext"), `Shader::ProcesShader` ("ProcessShader").
- `FrameBuffers.cpp`'s `ToGLTextureFormat` has no `default`/fallback branch — falls off the end
  without returning for `None`/`Depth`.
- `Texture`'s constructor only handles 3- or 4-channel images; other channel counts leave
  `internalFormat`/`dataFormat` at `0` with no guard before the GL calls that use them.
- `RendererAPI::DrawElements` calls `glBindTexture(GL_TEXTURE_2D, 0)` after every draw, but textures
  are bound via `glBindTextureUnit` (DSA) — this call is a no-op.
- `Renderer2D::Shutdown()` deletes the vertex buffer base array but never deletes `s_Data` itself,
  leaking the renderer's `Data2D` struct.
- Hardcoded tuning constants throughout (`MaxElements = 10000`, `MaxTextures = 32`,
  `s_MaxFramebufferSize = 8192`, `EditorCamera`'s pan/zoom speed magic numbers) with little
  explanation of how they were derived.
