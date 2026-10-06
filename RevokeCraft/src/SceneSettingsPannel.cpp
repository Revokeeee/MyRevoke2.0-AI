#include "rvpch.h"
#include "SceneSettingsPannel.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include "MyRevoke/Renderer/RendererAPI.h"


namespace Revoke
{
	static char* sceneName;

	SceneSettingsPannel::SceneSettingsPannel()
	{
		
	}
	SceneSettingsPannel::~SceneSettingsPannel()
	{
		delete[] sceneName;
	}

	void SceneSettingsPannel::OnImGuiRender()
	{
		ImGui::Begin("Scene Settings");

		
		if (ImGui::InputText("Scene Name", sceneName, sizeof(sceneName)))
		{
			std::string str(sceneName);
			m_CurrentScene->SetName(str);
		}

		if (ImGui::ColorEdit3("Set Clear Color", glm::value_ptr(m_ClearColor)));
			RendererAPI::SetClearColor({ m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, 1.0f });

		if(ImGui::Checkbox("Eneble Blending", &m_EnebleBlending))
			RendererAPI::EnableBlending();

		if(ImGui::DragInt("Set Gravity Position Iteration", &m_GravityPositionIteration, 0, 100))
			m_CurrentScene->SetGravityPositionIteration(m_GravityPositionIteration);

		if (ImGui::DragInt("Set Gravity Velocity Iteration", &GravityVelocityIteration, 0, 100))
			m_CurrentScene->SetGravityVelocityIteration(GravityVelocityIteration);

		// Needs MSBuild and the repo checkout, neither of which a shipped
		// editor has, so Dist builds don't offer it (#36, #78).
#ifndef RV_DIST
		if (ImGui::Button("Build Scripts"))
		{
#ifdef RV_DEBUG
			system("msbuild ../MyRevoke-NativeScriptCore/MyRevoke-NativeScriptCore.vcxproj /p:Configuration=Debug /p:Platform=x64");
#else
			system("msbuild ../MyRevoke-NativeScriptCore/MyRevoke-NativeScriptCore.vcxproj /p:Configuration=Release /p:Platform=x64");
#endif

		}
#endif

		ImGui::End();
	}

	void SceneSettingsPannel::SetScene(Shared<Scene> currentScene)
	{
		m_CurrentScene = currentScene;
		const std::string& tempName = currentScene->GetName(); 
		sceneName = new char[tempName.size() + 1]; 
		strcpy(sceneName, tempName.c_str());
	}

}