# EventSystem

A lightweight, header-only, synchronous event system. Events are constructed on the stack and passed
by reference down the `Layer` stack — there's no queue.

## Files

- **`Event.h`** — the `EventType` enum and `EventCategory` bitflags, the abstract `Event` base class
  (`GetEventType`/`GetName`/`GetCategoryFlags`/`ToString`, plus a `Handled` flag), and
  `EventDispatcher::Dispatch<T>`, which type-matches an event against a handler function.
- **`AppEvent.h`** — `WindowResizeEvent`, `WindowsCloseEvent` *(sic — "WindowCloseEvent")*.
- **`KeyEvent.h`** — abstract `KeyEvent`, plus `KeyPressedEvent`, `KeyReleasedEvent`, `KeyTypedEvent`.
- **`MouseEvent.h`** — `MouseMovedEvent`, abstract `MouseButtonEvent`, plus
  `MouseButtonPressedEvent`, `MouseButtonReleasedEvent`, `MouseScrolledEvent`.

## Dependencies

Only depends on `Core/Core.h` (for the `BIT` macro) and the precompiled header. Consumed everywhere
input/window state needs to travel: `Window`'s GLFW callbacks construct these events,
`Application::OnEvent` dispatches them down the `LayerStack`, and `ImGuiLayer::OnEvent` consumes mouse/
keyboard categories to decide whether ImGui should swallow them.

## Known issues

- `WindowsCloseEvent` (extra "s") doesn't match its own `EventType::WindowClose`.
- `KeyReleasedEvent` carries a dead, never-set `m_RepeatCount` member copy-pasted from
  `KeyPressedEvent`, and its `ToString()` prints `"KeyPresedEvent"` (copy-paste bug, wrong name and
  missing a letter).
- `EventType::WindowMoved` is declared in the enum but has no corresponding event class.
