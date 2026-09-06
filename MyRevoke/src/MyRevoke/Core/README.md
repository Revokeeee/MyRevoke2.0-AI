# Core

The engine's foundation: application lifecycle and main loop, a GLFW window abstraction that
produces engine events, a layer/overlay stack, platform input polling, logging, frame timing, and
entity UUIDs. Everything else in the engine is bootstrapped from here.

## Files

- **`EntryPoint.h`** — declares `extern Revoke::CreateApplication()` (implemented by the client app)
  and defines `main()`: initializes `Log`, creates the client `Application`, calls `Run()`, deletes it.
- **`Application.h`/`.cpp`** — singleton (`Application::Get()`) owning the `Window`, the `LayerStack`,
  and the `ImGuiLayer`. Its constructor initializes `RendererAPI`, `Renderer2D`, `AudioRenderer`, and
  the native-script `ScriptEngine` DLL, and pushes the ImGui layer as an overlay.
- **`Input.h`/`.cpp`** — static-only key/mouse polling (`IsKeyPressed`, `GetMousePosition`, ...). Reads
  directly from `Application::Get().GetWindow().GetCoreWindow()`'s raw `GLFWwindow*`.
- **`Layer.h`/`.cpp`** — base class for anything pushed onto the stack: virtual `OnAttach`,
  `OnDetach`, `OnUpdate(Timestep)`, `OnImGuiDraw`, `OnEvent`.
- **`LayerStack.h`/`.cpp`** — `std::vector<Layer*>` with a split index: `PushLayer` inserts before the
  index (regular layers), `PushOverlay` appends after it (overlays, e.g. ImGui, always update/render
  last and receive events first). Owns the layers — its destructor deletes all of them.
- **`Log.h`/`.cpp`** — two spdlog loggers, one for the engine (`RV_ENGINE_*` macros) and one for the
  client app (`RV_EDITOR_*` macros).
- **`Time.h`** — `Timestep`, a thin wrapper around a float-seconds delta time.
- **`UniversallyUniqueIdentifiers.h`/`.cpp`** — `UUID`, a random 64-bit identifier (via
  `std::mt19937_64`) used as the persistent identity for scene entities (see `Scene/Components.h`'s
  `IdComponent`).
- **`Window.h`/`.cpp`** — wraps a `GLFWwindow*` and a `RenderContex` (see `Renderer`). Translates raw
  GLFW callbacks into `EventSystem` events and dispatches them through a `std::function` callback set
  by `Application`.
- **`Core.h`** — shared macros: `Unique<T>`/`Shared<T>` aliases (`unique_ptr`/`shared_ptr`),
  `RV_CORE_ASSERT`/`RV_ASSERT` (no-ops unless `RV_ASSERTS_ENABLE` is defined), `BIT(x)`.

## The main loop

`Application::Run()` (`Application.cpp`) does, every frame:

1. Compute a `Timestep` from `glfwGetTime()`.
2. Walk the `LayerStack` forward, calling `OnUpdate(timestep)` on each layer.
3. `ImGuiLayer::Begin()`, walk the stack again calling `OnImGuiDraw()`, then `ImGuiLayer::End()`.
4. `Window::OnUpdate()` — polls GLFW events and swaps buffers.

GLFW callbacks fire during step 4's `glfwPollEvents()` and invoke `Application::OnEvent`, which
handles `WindowsCloseEvent`/`WindowResizeEvent` itself, then walks the `LayerStack` **in reverse**
(overlays like ImGui first) until something sets `Event::Handled`.

## Cross-module dependencies

Core reaches into `EventSystem` (all event types), `Renderer` (`RendererAPI`, `Renderer2D`,
`GraphicContex`'s `RenderContex`), and `ImGui` (`ImGuiLayer`) directly. `Application`'s constructor
also bootstraps `AudioManager/AudioRenderer` and `Scripting/NativeScript`'s `ScriptEngine`, even
though nothing else in this directory otherwise references those modules.

## Known issues

- `UUID::Get()` returns `int64_t` while the underlying value is stored as `uint64_t` — a silent
  sign/narrowing mismatch.
- `Window.cpp` never calls `glfwTerminate()` on shutdown (there's a `// TODO` acknowledging it), and
  the GLFW window pointer returned by `glfwCreateWindow` isn't null-checked before use.
- `LayerStack::PopLayer`/`PopOverlay` don't `delete` the popped layer, unlike the stack's own
  destructor — inconsistent ownership if either is ever called.
- `Application.cpp` has a leftover `ALuint test = 2;` debug variable.
- `Core.h`'s `RV_CORE_ASSERT`/`RV_ASSERT` call the MSVC-only `__debugbreak()` intrinsic with no
  fallback for other compilers, and `UniversallyUniqueIdentifiers.h` includes `<xhash>`, an
  MSVC-internal header — both are portability landmines if this is ever built outside MSVC/Windows.
