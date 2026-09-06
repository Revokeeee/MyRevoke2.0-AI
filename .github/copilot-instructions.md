# Copilot review instructions for MyRevoke 2.0

## What this project is

MyRevoke is a **learning project**: a custom C++20 game engine sandbox built to
learn real software development by building, breaking, and fixing things. It is
deliberately experimental. Unfinished features, rough edges, and messy design
choices are expected and are often intentional — they are not defects to report.

Stack: C++20, OpenGL (GLAD), ImGui, Box2D, Mono/.NET scripting, OpenAL +
libsndfile, YAML serialization, Premake5 generating VS2022 solutions.

## What to review

Focus on things that would actually break or bite:

- **Correctness bugs**: logic that doesn't do what the surrounding code clearly
  intends; off-by-one; wrong operator; inverted condition.
- **Memory safety**: use-after-free, double-free, dangling pointers/references,
  leaks, ownership confusion between raw and smart pointers.
- **Undefined behavior**: uninitialized reads, out-of-bounds indexing, invalid
  casts, aliasing violations, iterator invalidation.
- **Crashes**: null dereference on paths that can realistically be hit,
  unchecked container access, missing bounds checks on external input.
- **Real performance problems**: heap allocation inside per-frame `Render()` or
  physics-step paths, accidental O(n²) over entity/collider collections,
  copies of large structures where a reference was clearly intended.
- **Concurrency**: data races if any threading is involved.

Report these with a concrete failure scenario: what input or state triggers it,
and what goes wrong. A finding without a plausible trigger is not useful here.

## What NOT to review

Do not raise these — they generate noise without improving the project:

- **Missing tests.** The project currently has no automated test suite and no CI
  build step. This is a known, accepted gap. Do not suggest adding tests, test
  coverage, or test frameworks on individual PRs.
- **Missing documentation** on internal functions, or requests for doc comments,
  README updates, or changelog entries.
- **Style and formatting preferences**: brace placement, line length, `auto`
  usage, `const` placement, include ordering, naming that is merely different
  from your preference. Match the surrounding file instead of a global ideal.
- **Speculative future-proofing**: "consider making this configurable",
  "this might not scale", "you may want an interface here" — unless there is a
  concrete bug today.
- **Unfinished or stubbed code** that is clearly work in progress.
- **Repeating a finding that was already raised and explicitly dismissed** on an
  earlier review of the same pull request. If a previous comment on this PR was
  resolved or replied to with a rationale, treat that as settled and do not
  raise it again on subsequent pushes.
- **Anything under `MyRevoke/vendor/`** — see below.

## Third-party code is out of scope

`MyRevoke/vendor/` contains git submodules for third-party libraries (Box2D,
GLFW, glm, imgui, ImGuizmo, spdlog, yaml-cpp). This code is not authored or
maintained here. Never review, critique, or suggest changes to anything under
that path. Submodule pointer bumps are also not a code-quality concern.

Committed build output (`bin/`, `bin-int/`, `.vs/`, `*.obj`, `*.lib`, `*.pdb`,
`*.tlog`) is generated, not written. Never review it.

## Conventions in use

- Classes: `PascalCase`. Functions and variables: generally `snake_case`,
  though the codebase is not fully consistent — follow the surrounding file.
- Prefer `std::unique_ptr` / `std::shared_ptr` over raw owning pointers, but raw
  non-owning pointers and references are fine and used widely.
- Build is Premake5 → `GenerateProject.bat` → VS2022 solution. There is no
  headless/CI build, so do not assume compilation was verified.

## Calibration

Fewer, higher-confidence findings are more useful than exhaustive coverage. If
you are unsure whether something is a real bug, leave it out. A pull request
with no comments is a valid and expected outcome.
