# Utility

A grab-bag of small, unrelated platform helpers.

## Files

- **`FileExplorer.h`/`.cpp`** — `FileExplorer::OpenFile(filter)`/`SaveFile(filter)`, static methods
  wrapping the Win32 `GetOpenFileNameA`/`GetSaveFileNameA` common dialogs (via
  `GLFW_EXPOSE_NATIVE_WIN32`/`glfwGetWin32Window`). Used by the editor
  (`RevokeCraft/src/CraftLayer.cpp`) for scene open/save dialogs.
- **`KeyCodes.h`** / **`MouseBtnCodes.h`** — `RV_KEY_*`/`RV_MOUSE_BUTTON_*` macros mirroring GLFW's
  key/button codes, used by `Core/Input.cpp` and window input translation.

## Known issues

- `FileExplorer` is hardcoded to Win32 (`commdlg.h`, no `#ifdef` guard) — won't compile on other
  platforms, unlike the rest of the engine's GLFW/GLAD-based cross-platform intent.
- `OpenFile` and `SaveFile` duplicate almost identical `OPENFILENAMEA` setup code; both also pass
  `OFN_FILEMUSTEXIST`, which is an odd flag for a *save* dialog (limits "Save As" to overwriting
  existing files only) and looks like a copy-paste bug.
- `KeyCodes.h`/`MouseBtnCodes.h` use plain `#define` macros rather than a scoped enum, so they pollute
  the global namespace — a style mismatch with the rest of the C++20 codebase.
