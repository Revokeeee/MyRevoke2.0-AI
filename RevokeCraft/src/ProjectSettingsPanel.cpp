#include "rvpch.h"
#include "ProjectSettingsPanel.h"

#include "AssetType.h"
#include "EditorUI.h"

#include <imgui.h>

#include "MyRevoke/ImGui/ImGuiIcons.h"

namespace Revoke
{
	namespace
	{
		bool IsScene(const std::filesystem::path& path) { return GetAssetType(path) == AssetType::Scene; }
	}

	void ProjectSettingsPanel::OnImGuiRender()
	{
		ImGui::Begin("Project Settings");

		if (!m_Project)
		{
			UI::EmptyState(RV_ICON_SETTINGS, "No project open.");
			ImGui::End();
			return;
		}

		// Every edit writes the project file straight away: it is the only thing RevokePlayer reads,
		// and nothing else in the editor would remember the change.
		bool changed = false;

		UI::SectionHeader("Game");
		if (UI::BeginProperties("##game"))
		{
			std::string gameName = m_Project->GetGameName();
			if (UI::PropertyText("Name", gameName))
			{
				m_Project->SetGameName(gameName);
				changed = true;
			}

			std::filesystem::path dropped;
			switch (UI::PropertyAsset("Startup Scene", RV_ICON_GLOBE, m_Project->GetStartupScene(),
				"The scene the game opens in. Drop a .myrevoke file from the Content Browser.", IsScene, dropped))
			{
			case UI::AssetFieldResult::Dropped:
				// Kept relative to the assets folder, with forward slashes, so the project file
				// reads the same wherever the project is moved.
				m_Project->SetStartupScene(dropped.generic_string());
				changed = true;
				break;
			case UI::AssetFieldResult::Cleared:
				m_Project->SetStartupScene("");
				changed = true;
				break;
			default:
				break;
			}
			UI::EndProperties();
		}

		UI::SectionHeader("Window");
		if (UI::BeginProperties("##window"))
		{
			int width = (int)m_Project->GetWindowWidth();
			int height = (int)m_Project->GetWindowHeight();
			bool sizeChanged = UI::PropertyInt("Width", width, 1.0f, 1, 7680);
			sizeChanged |= UI::PropertyInt("Height", height, 1.0f, 1, 4320);
			if (sizeChanged)
			{
				m_Project->SetWindowSize((unsigned int)width, (unsigned int)height);
				changed = true;
			}

			bool fullscreen = m_Project->IsFullscreen();
			if (UI::PropertyBool("Fullscreen", fullscreen, "Opens the game on the whole primary monitor, at the size above."))
			{
				m_Project->SetFullscreen(fullscreen);
				changed = true;
			}
			UI::EndProperties();
		}

		if (changed)
			m_Project->Save();

		ImGui::End();
	}

	void ProjectSettingsPanel::SetProject(Shared<Project> project)
	{
		m_Project = project;
	}
}
