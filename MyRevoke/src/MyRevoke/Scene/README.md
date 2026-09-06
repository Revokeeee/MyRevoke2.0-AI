# Scene

The ECS layer, built on [EnTT](https://github.com/skypjack/entt). `Scene` owns an `entt::registry`
and drives per-frame updates (scripting, physics, audio, rendering); `Entity` is a lightweight handle
into it; `Components.h` defines everything an entity can be made of; `Serealizer` saves/loads a scene
to/from YAML.

## Files

- **`Scene.h`/`.cpp`** — owns `entt::registry m_Registry` (private; `Entity`, `ObjectsPannel`, and
  `Serealizer` are `friend`s so they can reach into it directly). Drives `OnRuntimeStart/Update/Stop`
  and creates/destroys entities.
- **`Entity.h`/`.cpp`** — a handle pairing an `entt::entity` with a `Scene*`. Has no data of its own;
  `AddComponent`/`GetComponent`/`RemoveComponent`/`HasComponent` all forward to the owning `Scene`'s
  registry (the classic "entity is a handle, registry is the owner" pattern). `AddComponent` also
  calls a private `Scene::OnComponentAdded<T>` hook (e.g. to set a new `CameraComponent`'s viewport
  size).
- **`Components.h`** — every component struct (see below).
- **`SceneCamera.h`/`.cpp`** — runtime/gameplay camera (see `Renderer/README.md`).
- **`Serealizer.h`/`.cpp`** *(sic — "Serializer")* — YAML (yaml-cpp) scene save/load.

## Components (`Components.h`)

| Component | Purpose |
|---|---|
| `NameComponent` | Display name/tag string |
| `IdComponent` | Wraps a `UUID` — the entity's persistent identity, used by serialization |
| `TransformComponent` | Position/rotation/scale; `GetTransform()` builds the model matrix |
| `SpriteRendererComponent` | Tint color + texture path |
| `CameraComponent` | Embeds a `SceneCamera`, plus `isMain`/`FixedAspectRatio` flags |
| `RigidBodyComponent` | Box2D body type (Static/Kinematic/Dynamic) + a raw `b2Body*` set at runtime |
| `BoxColisionComponent` *(sic)* | Box collider size/offset, density/friction/restitution, `isSensor` |
| `SoundComponent` | Audio path + OpenAL buffer/source IDs, pitch/gain/position/velocity/loop |
| `MusicComponent` | **Entirely commented out** — dead code for a never-finished streamed-music component |
| `NativeScriptComponent` | Script class name + a raw `ScriptEntity*` instance, resolved lazily at runtime |

## Physics and scripting integration

- **Physics:** `Scene::OnRuntimeStart()` creates a `b2World` with gravity `(0, -9.8)`, and for every
  entity with a `RigidBodyComponent` builds a `b2Body` from its `TransformComponent` (plus a
  `b2PolygonShape`/fixture if a `BoxColisionComponent` is present). `OnRuntimeUpdate` steps the world
  and writes the resulting position/angle back into `TransformComponent`. `OnRuntimeStop` deletes the
  `b2World`.
- **Native scripting:** `OnRuntimeUpdate` iterates `NativeScriptComponent`s; if `Instance` is null it
  resolves one via `ScriptEngine::GetScritpByName` (see `Scripting/README.md`), assigns the entity
  handle, calls `OnCreate()` once, then `OnUpdate(ts)` every frame.

## Serialization

`Serealizer::Serealize()` iterates the registry and writes one YAML map per entity, keyed by its
`IdComponent` UUID, with a nested block per present component (`TagComponent`, `TransformComponent`,
`CameraComponent`, `SpriteRendererComponent`, `RigidBodyComponent`, `BoxColisionComponent`,
`SoundComponent`, `NativeScriptComponent`). `DeSerealize()` reverses this: reads the UUID + tag, calls
`Scene::CreateEntity(uuid, name)`, then conditionally re-adds and populates each component block found
in the file. Custom `YAML::convert<>` specializations handle `glm::vec2/vec3/vec4` as flow sequences.

## Known issues

- Filename/identifier typos throughout: `Serealizer`/`Serealize`/`DeSerealize` ("Serializer"),
  `BoxColisionComponent` ("Collision"), and its fields `Restriction`/`ResitutionTreshhold`
  ("Restitution"/"Threshold").
- `NativeScriptComponent::Instance` is never deleted — no `OnDestroy()` call site exists in this
  module, and `Scene::RemoveEntity` just calls `m_Registry.destroy(ent)` without releasing the script
  instance, the `b2Body`, or shutting down a `SoundComponent`'s OpenAL source — a resource leak /
  dangling-body risk on entity deletion.
- Component structs holding runtime handles (`RigidBodyComponent::Body`, `SoundComponent`'s buffer/
  source IDs) use defaulted copy constructors, so copying an entity's components would copy the raw
  Box2D/OpenAL handles, risking a double-free/double-release.
- `MusicComponent` is dead code (fully commented out) and could be removed.
