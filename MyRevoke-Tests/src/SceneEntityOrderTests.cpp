#include <doctest/doctest.h>

// The scene headers pull in windows.h; keep its min/max macros out of entt.
#define NOMINMAX

#include "MyRevoke/Scene/Scene.h"
#include "MyRevoke/Scene/Components.h"
#include "MyRevoke/Scene/Entity.h"
#include "MyRevoke/Scene/Serializer.h"

#include <string>
#include <vector>

using namespace Revoke;

// The hierarchy lists entities with GetEntities(), and the scene file is written in that order.
// Both used to follow the registry's newest-first order, so every save and reload flipped them.

static std::vector<std::string> NamesOf(Scene& scene)
{
	std::vector<std::string> names;
	for (Entity entity : scene.GetEntities())
		names.push_back(entity.GetComponent<NameComponent>().Name);
	return names;
}

TEST_CASE("GetEntities lists entities in the order they were created")
{
	auto scene = std::make_shared<Scene>("test");
	scene->CreateEntity("First");
	scene->CreateEntity("Second");
	scene->CreateEntity("Third");

	CHECK(NamesOf(*scene) == std::vector<std::string>{ "First", "Second", "Third" });
}

TEST_CASE("Deleting an entity keeps the others in creation order")
{
	// The registry fills a destroyed entity's slot with its last entity, so the order can't
	// come from the registry's storage.
	auto scene = std::make_shared<Scene>("test");
	scene->CreateEntity("A");
	Entity b = scene->CreateEntity("B");
	scene->CreateEntity("C");
	scene->CreateEntity("D");

	scene->RemoveEntity(b);
	CHECK(NamesOf(*scene) == std::vector<std::string>{ "A", "C", "D" });
	CHECK(scene->GetEntityCount() == 3);

	scene->CreateEntity("E");
	CHECK(NamesOf(*scene) == std::vector<std::string>{ "A", "C", "D", "E" });
}

TEST_CASE("Saving and loading a scene keeps its entity order")
{
	auto scene = std::make_shared<Scene>("test");
	scene->CreateEntity("First");
	scene->CreateEntity("Second");
	scene->CreateEntity("Third");

	std::string text = Serializer(scene).SerializeToString();

	auto loaded = std::make_shared<Scene>();
	REQUIRE(Serializer(loaded).DeserializeFromString(text));
	CHECK(NamesOf(*loaded) == std::vector<std::string>{ "First", "Second", "Third" });

	// And the reloaded scene saves to the same text, so it doesn't look modified.
	CHECK(Serializer(loaded).SerializeToString() == text);
}

TEST_CASE("Duplicating an entity copies its components under a new id")
{
	auto scene = std::make_shared<Scene>("test");
	Entity source = scene->CreateEntity("Crate");
	source.GetComponent<TransformComponent>().Position = { 1.0f, 2.0f, 3.0f };
	source.AddComponent<SpriteRendererComponent>().Color = { 1.0f, 0.0f, 0.0f, 1.0f };
	source.AddComponent<RigidBodyComponent>().Type = RigidBodyComponent::BodyType::DynamicBody;
	source.AddComponent<NativeScriptComponent>().scriptClassName = "Player";

	Entity copy = scene->DuplicateEntity(source);

	REQUIRE(copy);
	CHECK(copy != source);
	CHECK(copy.GetComponent<IdComponent>().ID != source.GetComponent<IdComponent>().ID);
	CHECK(copy.GetComponent<NameComponent>().Name == "Crate");
	CHECK(copy.GetComponent<TransformComponent>().Position == glm::vec3(1.0f, 2.0f, 3.0f));
	CHECK(copy.GetComponent<SpriteRendererComponent>().Color == glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
	CHECK(copy.GetComponent<RigidBodyComponent>().Type == RigidBodyComponent::BodyType::DynamicBody);
	CHECK(copy.GetComponent<NativeScriptComponent>().scriptClassName == "Player");
	CHECK_FALSE(copy.HasComponent<CameraComponent>());

	// Editing the copy leaves the source alone.
	copy.GetComponent<TransformComponent>().Position.x = 10.0f;
	CHECK(source.GetComponent<TransformComponent>().Position.x == 1.0f);
}
