#pragma once

#include "entt.hpp"

#include <filesystem>

#include "MyRevoke/Core/Time.h"
#include "MyRevoke/Renderer/EditorCamera.h"
#include "MyRevoke/Core/UniversallyUniqueIdentifiers.h"

#include "MyRevoke/Scripting/NativeScript.h"

class b2World;

namespace Revoke
{
	class Entity;


	class Scene
	{
		public:
			Scene();
			Scene(std::string name);
			~Scene();

			// Builds an independent scene with the same entities by saving the source to text and
			// loading it back. Play mode runs on such a copy so the edited scene is never touched.
			static Shared<Scene> Copy(const Shared<Scene>& source, const std::filesystem::path& assetsDirectory = {});
			
			Entity CreateEntity(const std::string name = std::string());
			Entity CreateEntity(UUID id, const std::string name = std::string());
			// A new entity (with a new id) holding copies of the source's components.
			Entity DuplicateEntity(Entity source);

			// Every entity, oldest first. The hierarchy and the scene file both use this order, so a
			// save and reload keeps it.
			std::vector<Entity> GetEntities();
			size_t GetEntityCount();

			// Returns an empty Entity when no entity has this id.
			Entity FindEntityByUUID(UUID id);

			void OnRuntimeStart();

			void OnRuntimeUpdate(Timestep ts);
			void OnEditorUpdate(Timestep ts, EditorCamera& camera);

			void OnRuntimeStop();

			void OnViewportResize(uint32_t width, uint32_t height);

			// Also frees what the entity's components hold outside the registry (sound, script instance).
			void RemoveEntity(Entity ent);
			// Calls OnDestroy on the entity's running script, if it has one, and frees it.
			void DestroyScriptInstance(Entity entity);
			// While playing: take the entity's body (and its collider with it) out of the physics
			// world, or only its collider. For removing those components; no-ops outside play mode.
			void DestroyPhysicsBody(Entity entity);
			void DestroyCollider(Entity entity);

			void SetName(std::string name) { m_Name = name; }
			std::string GetName() { return m_Name; }

			void SetGravityStats(int positionIteration, int velocityIteration) { SetGravityPositionIteration(positionIteration); SetGravityVelocityIteration(velocityIteration); }
			void SetGravityPositionIteration(int positionIteration) { m_PositionIteration = positionIteration; }
			void SetGravityVelocityIteration(int velocityIteration) { m_VelocityIteration = velocityIteration; }
			int GetGravityPositionIteration() const { return m_PositionIteration; }
			int GetGravityVelocityIteration() const { return m_VelocityIteration; }

			// Releases what the scene holds outside the registry (sounds, script instances). Call
			// before dropping a scene, while the script DLL is still loaded.
			void OnSceneClose();

			Entity GetMainCamera() ;

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);

		// Every running script in the scene, destroyed the same way as a single one.
		void DestroyAllScriptInstances();
	private:
		entt::registry m_Registry;
		// Live entities in creation order. The registry's own storage can't be used for this:
		// destroying an entity moves the last one into its slot.
		std::vector<entt::entity> m_EntityOrder;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
		std::string m_Name = "Unnamed";

		b2World* m_B2World = nullptr;


		int m_PositionIteration = 4;
		int m_VelocityIteration = 6;

		friend class Entity;
		friend class ObjectsPannel;
		friend class Serializer;
	};

}