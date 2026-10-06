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

			// Every entity, oldest first. The registry itself iterates newest first, which made the
			// hierarchy and the scene file flip order on every save and reload.
			std::vector<Entity> GetEntities();

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

			void SetName(std::string name) { m_Name = name; }
			std::string GetName() { return m_Name; }

			void SetGravityStats(int positionIteration, int velocityIteration) { SetGravityPositionIteration(positionIteration); SetGravityVelocityIteration(velocityIteration); }
			void SetGravityPositionIteration(int positionIteration) { m_PositionIteration = positionIteration; }
			void SetGravityVelocityIteration(int velocityIteration) { m_VelocityIteration = velocityIteration; }
			int GetGravityPositionIteration() const { return m_PositionIteration; }
			int GetGravityVelocityIteration() const { return m_VelocityIteration; }

			void OnSceneClose();

			Entity GetMainCamera() ;

	private:
		template<typename T>
		void OnComponentAdded(Entity entity, T& component);
	private:
		entt::registry m_Registry;
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