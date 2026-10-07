#include <doctest/doctest.h>

// The scene headers pull in windows.h; keep its min/max macros out of entt.
#define NOMINMAX

#include "MyRevoke/Scene/Scene.h"
#include "MyRevoke/Scene/Components.h"
#include "MyRevoke/Scene/Entity.h"
#include "MyRevoke/Scripting/ScriptEntity.h"

using namespace Revoke;

// Removing an entity, or a component from the Properties panel, has to release what the
// components hold outside the registry.

namespace
{
	class TrackingScript : public ScriptEntity
	{
	public:
		explicit TrackingScript(bool& destroyed) : m_Destroyed(destroyed) {}
	protected:
		void OnDestroy() override { m_Destroyed = true; }
	private:
		bool& m_Destroyed;
	};
}

TEST_CASE("Removing an entity destroys its running script")
{
	auto scene = std::make_shared<Scene>("test");
	Entity entity = scene->CreateEntity("Scripted");

	bool destroyed = false;
	entity.AddComponent<NativeScriptComponent>().Instance = new TrackingScript(destroyed);

	scene->RemoveEntity(entity);
	CHECK(destroyed);
}

TEST_CASE("DestroyScriptInstance runs OnDestroy and clears the instance")
{
	auto scene = std::make_shared<Scene>("test");
	Entity entity = scene->CreateEntity("Scripted");

	bool destroyed = false;
	auto& script = entity.AddComponent<NativeScriptComponent>();
	script.Instance = new TrackingScript(destroyed);

	scene->DestroyScriptInstance(entity);
	CHECK(destroyed);
	CHECK(entity.GetComponent<NativeScriptComponent>().Instance == nullptr);

	// A second call has nothing to destroy.
	scene->DestroyScriptInstance(entity);
}

TEST_CASE("A sound component without a file owns no audio objects")
{
	SoundComponent sound;
	CHECK(sound.BufferID == 0);
	CHECK(sound.SourceID == 0);

	sound.ShutDown();
	CHECK(sound.BufferID == 0);
	CHECK(sound.SourceID == 0);
}
