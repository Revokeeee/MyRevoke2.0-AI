---
applyTo: "MyRevoke/vendor/**, **/bin/**, **/bin-int/**, .vs/**, **/*.obj, **/*.lib, **/*.pdb, **/*.idb, **/*.tlog, **/*.ipch"
excludeAgent: "code-review"
---

# Not our code — do not review

Everything matched by this file is either third-party or machine-generated:

- `MyRevoke/vendor/**` — git submodules for Box2D, GLFW, glm, imgui, ImGuizmo,
  spdlog and yaml-cpp. Upstream code, not authored or maintained in this repo.
- `bin/`, `bin-int/`, `.vs/`, and the object/library/debug artifacts
  (`*.obj`, `*.lib`, `*.pdb`, `*.idb`, `*.tlog`, `*.ipch`) — MSBuild output that
  is currently committed to the repo. Generated, never hand-written.

Reviewing any of this produces findings nobody can or should act on.
