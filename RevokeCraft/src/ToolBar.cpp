#include "rvpch.h"

#include "ToolBar.h"
#include "MyRevoke/Utility/ExecutablePath.h"
#include <imgui.h>

#include <ImGuizmo.h>

namespace Revoke
{
	ToolBar::ToolBar()
	{
		std::filesystem::path iconsDirectory = GetExecutableDirectory() / "resourses" / "icons" / "Toolbar";
		m_PlayIcon = std::make_shared<Texture>((iconsDirectory / "Play_Icon.png").string());
		m_StopIcon = std::make_shared<Texture>((iconsDirectory / "Stop_Icon.png").string());
	}
	void ToolBar::OnImGuiRender()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.3f, 0.3f, 0.3f, 0.5f));
		ImGui::Begin("##toolbar",nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		float size = ImGui::GetWindowHeight() - 6.0f;
		ImGui::SameLine((ImGui::GetWindowContentRegionMax().x * 0.03f) - (size * 0.5f));
		Shared<Texture> icon = m_SceneState == SceneState::Editor ? m_PlayIcon : m_StopIcon;
		if (ImGui::ImageButton((ImTextureID)icon->GetID(), ImVec2(size,size), ImVec2(0, 0), ImVec2(1,1), 0))
		{
			if (m_SceneState == SceneState::Editor)
				OnScenePlay();
			else
				OnSceneStop();
		}
		ImGui::SameLine((ImGui::GetWindowContentRegionMax().x * 0.86f) - (size * 0.5f));
		if (ImGui::Button("Q", ImVec2(size, size)))
		{
			*m_Guizmo = -1;
		}
		ImGui::SameLine((ImGui::GetWindowContentRegionMax().x * 0.895f) - (size * 0.5f));
		if (ImGui::Button("W", ImVec2(size, size)))
		{
			*m_Guizmo = ImGuizmo::OPERATION::TRANSLATE;
		}
		ImGui::SameLine((ImGui::GetWindowContentRegionMax().x * 0.925f) - (size * 0.5f));
		if (ImGui::Button("E", ImVec2(size, size)))
		{
			*m_Guizmo = ImGuizmo::OPERATION::ROTATE;
		}
		ImGui::SameLine((ImGui::GetWindowContentRegionMax().x * 0.955f) - (size * 0.5f));
		if (ImGui::Button("R", ImVec2(size, size)))
		{
			*m_Guizmo = ImGuizmo::OPERATION::SCALE;
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
		ImGui::End();

	}
	void ToolBar::OnScenePlay()
	{	
		m_CurrentScene->OnRuntimeStart();
		m_SceneState = SceneState::Runtime;
	}
	void ToolBar::OnSceneStop()
	{
		m_CurrentScene->OnRuntimeStop();
		m_SceneState = SceneState::Editor;
	}
	void ToolBar::SetScene(Shared<Scene> currentScene)
	{
		m_CurrentScene = currentScene;
	}
	void ToolBar::SetGuizmo(int* guizmo)
	{
		m_Guizmo = guizmo;
	}
}
