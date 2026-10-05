# Project

The user's game content lives in a project folder the user picks, so an installed editor never has to
write into its own install folder. A project is a `<Name>.mrproject` file plus the assets folder next
to it.

## Files

- **`Project.h`/`.cpp`** — `Revoke::Project`. `Create(name, parentDirectory)` makes
  `<parentDirectory>/<name>/` with `assets/Scenes` and `assets/Scripts` inside it and writes the
  project file; `Load(projectFilePath)` reads one back and returns `nullptr` if the file is missing or
  isn't a project. `GetAssetsDirectory()`/`GetScenesDirectory()`/`GetScriptsDirectory()` are what the
  editor's content browser, scene save/load and drag-and-drop use instead of a path next to the exe.

The project file is YAML, like the `.myrevoke` scene files:

```yaml
Project: Example
AssetsFolder: assets
```

`RevokeCraft` owns the open `Project` (`CraftLayer::m_Project`) and pushes its assets directory into
the panels that need it — there is no global "current project". The editor opens
`projects/Example/Example.mrproject` next to the exe on startup.

## Known issues

- Scene files store asset paths relative to the assets folder, but `Serializer` only does the
  conversion when it is given an assets directory; built without one it passes paths through
  unchanged.
- Nothing stops a project from being renamed or moved out from under the editor; `Load` is only ever
  called explicitly.
- The native script DLL still compiles `RevokeCraft/projects/Example/assets/Scripts/**` from
  `premake5.lua`, so scripts in other projects are not built.
