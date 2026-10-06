#include "rvpch.h"
#include "SceneSettingsPannel.h"
#include "EditorUI.h"

#include <imgui.h>

#include "MyRevoke/ImGui/ImGuiIcons.h"
#include "MyRevoke/ImGui/ImGuiLayer.h"
#include "MyRevoke/ImGui/ImGuiTheme.h"
#include "MyRevoke/Renderer/RendererAPI.h"


namespace Revoke
{
	namespace
	{
		void SectionHeader(const char* title)
		{
			ImGui::Spacing();
			ImGui::PushFont(ImGuiLayer::GetFont(FontStyle::Bold));
			ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextMuted);
			ImGui::SeparatorText(title);
			ImGui::PopStyleColor();
			ImGui::PopFont();
		}
	}

	void SceneSettingsPannel::OnImGuiRender()
	{
		ImGui::Begin("Scene Settings");

		if (!m_CurrentScene)
		{
			UI::EmptyState(RV_ICON_SETTINGS, "No scene open.");
			ImGui::End();
			return;
		}

		SectionHeader("Scene");
		if (UI::BeginProperties("##scene"))
		{
			std::string name = m_CurrentScene->GetName();
			if (UI::PropertyText("Name", name))
				m_CurrentScene->SetName(name);
			UI::EndProperties();
		}

		SectionHeader("Rendering");
		if (UI::BeginProperties("##rendering"))
		{
			if (UI::PropertyColor("Background", m_ClearColor))
				RendererAPI::SetClearColor({ m_ClearColor.r, m_ClearColor.g, m_ClearColor.b, 1.0f });

			if (UI::PropertyBool("Alpha Blending", m_EnableBlending, "Blend sprites by their alpha. Off draws transparent pixels opaque."))
			{
				if (m_EnableBlending)
					RendererAPI::EnableBlending();
				else
					RendererAPI::DisableBlending();
			}
			UI::EndProperties();
		}

		SectionHeader("Physics");
		if (UI::BeginProperties("##physics"))
		{
			// Read from and written to the scene, so the values follow it when another scene opens.
			int velocityIterations = m_CurrentScene->GetGravityVelocityIteration();
			if (UI::PropertyInt("Velocity Iterations", velocityIterations, 0.1f, 1, 100, "Box2D velocity solver passes per step. More is more accurate and slower."))
				m_CurrentScene->SetGravityVelocityIteration(velocityIterations);

			int positionIterations = m_CurrentScene->GetGravityPositionIteration();
			if (UI::PropertyInt("Position Iterations", positionIterations, 0.1f, 1, 100, "Box2D position solver passes per step. More is more accurate and slower."))
				m_CurrentScene->SetGravityPositionIteration(positionIterations);
			UI::EndProperties();
		}

		// Needs MSBuild and the repo checkout, neither of which a shipped
		// editor has, so Dist builds don't offer it (#36, #78).
#ifndef RV_DIST
		SectionHeader("Scripting");
		if (ImGui::Button(RV_ICON_BUILD "  Build Scripts", ImVec2(ImGui::GetContentRegionAvail().x, 0.0f)))
		{
#ifdef RV_DEBUG
			system("msbuild ../MyRevoke-NativeScriptCore/MyRevoke-NativeScriptCore.vcxproj /p:Configuration=Debug /p:Platform=x64");
#else
			system("msbuild ../MyRevoke-NativeScriptCore/MyRevoke-NativeScriptCore.vcxproj /p:Configuration=Release /p:Platform=x64");
#endif

		}
		UI::Tooltip("Builds MyRevoke-NativeScriptCore with MSBuild. The editor waits until it finishes.");
#endif

		ImGui::End();
	}

	void SceneSettingsPannel::SetScene(Shared<Scene> currentScene)
	{
		m_CurrentScene = currentScene;
	}

}
