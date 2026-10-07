#pragma once

#include <glm/glm.hpp>

#include "MyRevoke/Scene/Scene.h"

namespace Revoke
{

	class SceneSettingsPannel
	{
	public:
		SceneSettingsPannel() = default;

		void OnImGuiRender();

		void SetScene(Shared<Scene> currentScene);

	private:
		Shared<Scene> m_CurrentScene;

		// Renderer state, not saved with the scene.
		glm::vec3 m_ClearColor = { 0.2f, 0.2f, 0.2f };
		bool m_EnableBlending = true;
	};
}
