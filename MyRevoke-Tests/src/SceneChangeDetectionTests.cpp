#include <doctest/doctest.h>

// The scene headers pull in windows.h; keep its min/max macros out of entt.
#define NOMINMAX

#include "MyRevoke/Scene/Scene.h"
#include "MyRevoke/Scene/Components.h"
#include "MyRevoke/Scene/Serializer.h"

using namespace Revoke;

// The editor decides whether to ask "save changes?" by comparing the scene's serialized text with
// a snapshot taken when it was last saved, so that text has to be stable and has to track edits.

TEST_CASE("Serializing an unchanged scene twice gives identical text")
{
	auto scene = std::make_shared<Scene>("test");
	scene->CreateEntity("First");
	scene->CreateEntity("Second");

	Serializer serializer(scene);
	CHECK(serializer.SerializeToString() == serializer.SerializeToString());
}

TEST_CASE("Editing a component changes the serialized text")
{
	auto scene = std::make_shared<Scene>("test");
	Entity entity = scene->CreateEntity("Mover");

	Serializer serializer(scene);
	std::string before = serializer.SerializeToString();

	entity.GetComponent<TransformComponent>().Position.x = 5.0f;
	CHECK(serializer.SerializeToString() != before);
}

TEST_CASE("Adding an entity changes the serialized text")
{
	auto scene = std::make_shared<Scene>("test");
	scene->CreateEntity("Existing");

	Serializer serializer(scene);
	std::string before = serializer.SerializeToString();

	scene->CreateEntity("Added");
	CHECK(serializer.SerializeToString() != before);
}

TEST_CASE("Putting a value back restores the original text")
{
	auto scene = std::make_shared<Scene>("test");
	Entity entity = scene->CreateEntity("Mover");

	Serializer serializer(scene);
	std::string before = serializer.SerializeToString();

	entity.GetComponent<TransformComponent>().Position.x = 5.0f;
	entity.GetComponent<TransformComponent>().Position.x = 0.0f;
	CHECK(serializer.SerializeToString() == before);
}
