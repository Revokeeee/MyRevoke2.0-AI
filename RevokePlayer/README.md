# RevokePlayer

The standalone game runtime: the same scene playback the editor's Play mode runs, in an app with no
editor UI. Game export will ship a renamed copy of this exe next to the user's project.

## Entry point

`src/App.cpp` defines `class RevokePlayer : public Revoke::Application`, pushing a single
`PlayerLayer`, and implements the `Revoke::CreateApplication()` factory required by the engine's
`EntryPoint.h` — the same shape as [`RevokeCraft`](../RevokeCraft/README.md).

## PlayerLayer (`src/PlayerLayer.h`/`.cpp`)

- `OnAttach()` loads the project (the first `*.mrproject` next to the exe, else
  `projects/Example/Example.mrproject`), deserializes its startup scene
  ([`Project::GetStartupScenePath`](../MyRevoke/src/MyRevoke/Project/README.md)) and calls
  `Scene::OnRuntimeStart()`. A failure logs and leaves the layer with no scene; the window still runs.
- `OnUpdate()` clears the window, keeps the scene's viewport in step with the window size and calls
  `Scene::OnRuntimeUpdate()`, which renders through the scene's primary camera.
- `OnCloseRequested()`/`OnDetach()` call `Scene::OnRuntimeStop()` and `Scene::OnSceneClose()`.

Native C++ scripting needs no setup here: `Application`'s constructor already calls
`ScriptEngine::InitDll()`. The editor's per-frame `ScriptEngine::OnUpdate()` is only DLL hot reload,
so the player skips it.

## Building

Premake builds it into the `RevokeCraft/` folder, because it reads `resourses/` and the project it
runs from next to its own exe. `Dist` is a `WindowedApp`, so a shipped game opens no console.
