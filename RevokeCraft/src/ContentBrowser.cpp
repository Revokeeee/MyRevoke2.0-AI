#include "rvpch.h"
#include "ContentBrowser.h"
#include "MyRevoke/Utility/ExecutablePath.h"

#include <imgui.h>


namespace Revoke
{
	ContentBrowser::ContentBrowser()
	{
		std::filesystem::path iconsDirectory = GetExecutableDirectory() / "resourses" / "icons" / "ContentBrowser";
		m_FolderIcon = std::make_shared<Texture>((iconsDirectory / "Folder_Icon.png").string());
		m_FileIcon = std::make_shared<Texture>((iconsDirectory / "File_Icon.png").string());
	}

	void ContentBrowser::SetAssetsDirectory(const std::filesystem::path& assetsDirectory)
	{
		m_AssetsDir = assetsDirectory;
		m_CurrendDir = assetsDirectory;
	}

	void ContentBrowser::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		if (!std::filesystem::is_directory(m_CurrendDir))
		{
			ImGui::Text("No project open.");
			ImGui::End();
			return;
		}

		if (m_CurrendDir != m_AssetsDir)
		{
			if (ImGui::Button("<-"))
			{
				m_CurrendDir = m_CurrendDir.parent_path();
			}
		}

		static float padding = 16.0f;
		static float thumbnailSize = 95.0f;
		float cellSize = thumbnailSize + padding;
		float panelWidth = ImGui::GetContentRegionAvail().x;
		float panelHeight = ImGui::GetContentRegionAvail().y;
		int columnCount = (int)(panelWidth / cellSize);
		if (columnCount < 1)
			columnCount = 1;

		ImGui::Columns(columnCount, 0, false);

		int i = 0;
		//Does not support multiple language!!!
		for (auto entryPath : std::filesystem::directory_iterator(m_CurrendDir))
		{
			ImGui::PushID(i++);

			std::string pathString = entryPath.path().string();
			auto relativePath = std::filesystem::relative(entryPath.path(), m_AssetsDir);
			std::string fileNameString = relativePath.filename().string();

			Shared<Texture> icon = entryPath.is_directory() ? m_FolderIcon : m_FileIcon;

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1.0f, 1.0f, 1.0f, 0.05f));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 15.0f);
			ImGui::ImageButton((ImTextureID)icon->GetID(), { thumbnailSize, thumbnailSize }, { 0,1 }, { 1, 0 });
			ImGui::PopStyleVar();
			ImGui::PopStyleColor();

			if (ImGui::BeginDragDropSource())
			{
				const wchar_t* pathForDragDrop = relativePath.c_str();
				ImGui::SetDragDropPayload("CONTENT_BROWSER_PAYLOAD", pathForDragDrop, (wcslen(pathForDragDrop) + 1) * sizeof(wchar_t), ImGuiCond_Once);
				ImGui::EndDragDropSource();
			}

			if(ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				if (entryPath.is_directory())
				{
					m_CurrendDir /= entryPath.path().filename();
				}

			}
			ImGui::TextWrapped(fileNameString.c_str());
			
			ImGui::NextColumn();
			ImGui::PopID();
		}
		ImGui::End();
	}
}
