#include <doctest/doctest.h>

// The scene headers pull in windows.h; keep its min/max macros out of entt.
#define NOMINMAX

#include "MyRevoke/Scene/Scene.h"
#include "MyRevoke/Scene/Components.h"
#include "MyRevoke/Scene/Entity.h"
#include "MyRevoke/Scene/Serializer.h"

using namespace Revoke;

// Play mode runs on Scene::Copy and throws the copy away on Stop, so anything physics or scripts
// do while playing has to stay inside the copy.

static std::string SceneText(const Shared<Scene>& scene)
{
	Serializer serializer(scene);
	return serializer.SerializeToString();
}

TEST_CASE("A copied scene has the same entities and settings")
{
	auto scene = std::make_shared<Scene>("test");
	Entity entity = scene->CreateEntity("Box");
	entity.GetComponent<TransformComponent>().Position = { 1.0f, 2.0f, 3.0f };
	entity.AddComponent<RigidBodyComponent>().Type = RigidBodyComponent::BodyType::DynamicBody;
	scene->SetGravityStats(7, 9);

	auto copy = Scene::Copy(scene);

	CHECK(SceneText(copy) == SceneText(scene));
	CHECK(copy->GetGravityPositionIteration() == 7);
	CHECK(copy->GetGravityVelocityIteration() == 9);
}

TEST_CASE("Moving entities in the copy leaves the original where it was")
{
	auto scene = std::make_shared<Scene>("test");
	Entity entity = scene->CreateEntity("Mover");
	entity.GetComponent<TransformComponent>().Position = { 1.0f, 2.0f, 0.0f };
	entity.AddComponent<RigidBodyComponent>();
	Revoke::UUID id = entity.GetComponent<IdComponent>().ID;
	std::string before = SceneText(scene);

	auto copy = Scene::Copy(scene);
	Entity copied = copy->FindEntityByUUID(id);
	REQUIRE(copied);
	auto& copiedTransform = copied.GetComponent<TransformComponent>();
	copiedTransform.Position.y = -5.0f;
	copiedTransform.Rotation.z = 1.0f;

	CHECK(entity.GetComponent<TransformComponent>().Position.y == 2.0f);
	CHECK(entity.GetComponent<TransformComponent>().Rotation.z == 0.0f);
	CHECK(SceneText(scene) == before);
}

TEST_CASE("A copy does not share the script instance of the original")
{
	auto scene = std::make_shared<Scene>("test");
	Entity entity = scene->CreateEntity("Scripted");
	auto& script = entity.AddComponent<NativeScriptComponent>();
	script.scriptClassName = "Player";
	script.Instance = reinterpret_cast<ScriptEntity*>(0x1); // a marker, never dereferenced

	auto copy = Scene::Copy(scene);
	Entity copied = copy->FindEntityByUUID(entity.GetComponent<IdComponent>().ID);
	REQUIRE(copied);

	auto& copiedScript = copied.GetComponent<NativeScriptComponent>();
	CHECK(copiedScript.scriptClassName == "Player");
	CHECK(copiedScript.Instance == nullptr);

	script.Instance = nullptr;
}

TEST_CASE("Copying twice gives the same scene both times")
{
	auto scene = std::make_shared<Scene>("test");
	scene->CreateEntity("First").AddComponent<RigidBodyComponent>();
	scene->CreateEntity("Second");

	CHECK(SceneText(Scene::Copy(scene)) == SceneText(Scene::Copy(scene)));
}
