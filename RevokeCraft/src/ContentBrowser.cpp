#include "rvpch.h"
#include "ContentBrowser.h"
#include "EditorUI.h"

#include "MyRevoke/ImGui/ImGuiLayer.h"
#include "MyRevoke/ImGui/ImGuiTheme.h"
#include "MyRevoke/ImGui/ImGuiIcons.h"
#include "MyRevoke/Renderer/Renderer2D.h"

#include <imgui.h>
#include <imgui_internal.h>

// ShellExecuteW, for "Show in Explorer" and opening files in their default app.
#include <shellapi.h>


namespace Revoke
{
	namespace
	{
		std::string ToUtf8(const std::filesystem::path& path)
		{
			auto text = path.u8string();
			return std::string(text.begin(), text.end());
		}

		std::string Lowercase(std::string text)
		{
			std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return (char)std::tolower(c); });
			return text;
		}

		ImVec4 GetAssetColor(AssetType type)
		{
			switch (type)
			{
			case AssetType::Folder:  return ImVec4(0.945f, 0.729f, 0.369f, 1.0f);
			case AssetType::Scene:   return Theme::Info;
			case AssetType::Audio:   return Theme::Success;
			case AssetType::Script:  return ImVec4(0.710f, 0.553f, 0.953f, 1.0f);
			default:                 return Theme::TextMuted;
			}
		}

		const char* GetAssetTypeName(AssetType type)
		{
			switch (type)
			{
			case AssetType::Folder:  return "Folder";
			case AssetType::Scene:   return "Scene";
			case AssetType::Texture: return "Texture";
			case AssetType::Audio:   return "Audio";
			case AssetType::Script:  return "Native script";
			default:                 return "File";
			}
		}

		// Opens Explorer on a folder, or on a file's folder with the file selected.
		void ShowInExplorer(const std::filesystem::path& path)
		{
			std::error_code error;
			std::wstring arguments = std::filesystem::is_directory(path, error)
				? L"\"" + path.wstring() + L"\""
				: L"/select,\"" + path.wstring() + L"\"";
			ShellExecuteW(nullptr, L"open", L"explorer.exe", arguments.c_str(), nullptr, SW_SHOWNORMAL);
		}

		void OpenWithDefaultApp(const std::filesystem::path& path)
		{
			ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
		}
	}

	void ContentBrowser::SetAssetsDirectory(const std::filesystem::path& assetsDirectory)
	{
		m_AssetsDir = assetsDirectory;
		m_Filter.clear();
		NavigateTo(assetsDirectory);
	}

	void ContentBrowser::NavigateTo(const std::filesystem::path& directory)
	{
		m_CurrentDir = directory;
		m_SelectedPath.clear();
		m_NeedsRefresh = true;
	}

	void ContentBrowser::Refresh()
	{
		m_Items.clear();

		auto addEntry = [this](const std::filesystem::directory_entry& entry)
			{
				std::error_code error;
				Item item;
				item.Path = entry.path();
				item.Name = ToUtf8(entry.path().filename());
				item.Type = entry.is_directory(error) ? AssetType::Folder : GetAssetType(entry.path());
				m_Items.push_back(std::move(item));
			};

		std::error_code error;
		if (m_Filter.empty())
		{
			for (std::filesystem::directory_iterator it(m_CurrentDir, error), end; !error && it != end; it.increment(error))
				addEntry(*it);
		}
		else
		{
			// A search looks through every folder of the project, not only the open one.
			for (std::filesystem::recursive_directory_iterator it(m_AssetsDir, std::filesystem::directory_options::skip_permission_denied, error), end;
				!error && it != end; it.increment(error))
			{
				if (UI::MatchesFilter(ToUtf8(it->path().filename()), m_Filter))
					addEntry(*it);
			}
		}

		// Folders first, then by name, ignoring case.
		std::sort(m_Items.begin(), m_Items.end(), [](const Item& a, const Item& b)
			{
				bool aIsFolder = a.Type == AssetType::Folder;
				bool bIsFolder = b.Type == AssetType::Folder;
				if (aIsFolder != bIsFolder)
					return aIsFolder;
				return Lowercase(a.Name) < Lowercase(b.Name);
			});

		m_LastRefreshTime = ImGui::GetTime();
		m_NeedsRefresh = false;
	}

	void ContentBrowser::OnImGuiRender()
	{
		ImGui::Begin("Content Browser");

		std::error_code error;
		if (m_AssetsDir.empty() || !std::filesystem::is_directory(m_AssetsDir, error))
		{
			UI::EmptyState(RV_ICON_FOLDER_OPEN, "No project open. Use File > Open Project.");
			ImGui::End();
			return;
		}

		// The open folder may have been deleted or renamed outside the editor.
		if (!std::filesystem::is_directory(m_CurrentDir, error))
			NavigateTo(m_AssetsDir);

		if (m_NeedsRefresh || ImGui::GetTime() - m_LastRefreshTime > 1.0)
			Refresh();

		DrawNavigationBar();
		ImGui::Separator();

		ImGui::BeginChild("##grid");

		const float scale = ImGuiLayer::GetUIScale();
		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		const float tileWidth = (m_ThumbnailSize + 12.0f) * scale;
		int columns = (int)((ImGui::GetContentRegionAvail().x + spacing) / (tileWidth + spacing));
		if (columns < 1)
			columns = 1;

		for (int i = 0; i < (int)m_Items.size(); i++)
		{
			if (i % columns != 0)
				ImGui::SameLine();
			DrawItem(m_Items[i], i, tileWidth);
		}

		if (m_Items.empty())
			UI::EmptyState(m_Filter.empty() ? RV_ICON_FOLDER : RV_ICON_SEARCH, m_Filter.empty() ? "This folder is empty." : "No asset matches the search.");

		ImGuiIO& io = ImGui::GetIO();
		if (ImGui::IsWindowHovered())
		{
			// Ctrl + wheel zooms the thumbnails, like most file browsers.
			if (io.KeyCtrl && io.MouseWheel != 0.0f)
				m_ThumbnailSize = std::clamp(m_ThumbnailSize + io.MouseWheel * 8.0f, 48.0f, 192.0f);

			if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && !ImGui::IsAnyItemHovered())
				m_SelectedPath.clear();
		}

		if (ImGui::BeginPopupContextWindow("##browserContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem(RV_ICON_FOLDER_OPEN "  Show in Explorer"))
				ShowInExplorer(m_CurrentDir);
			if (ImGui::MenuItem(RV_ICON_REFRESH "  Refresh"))
				m_NeedsRefresh = true;
			ImGui::EndPopup();
		}

		ImGui::EndChild();

		// Backspace goes up a folder, Enter opens the selection.
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !io.WantTextInput)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Backspace, false) && m_CurrentDir != m_AssetsDir && m_Filter.empty())
				NavigateTo(m_CurrentDir.parent_path());

			if (ImGui::IsKeyPressed(ImGuiKey_Enter, false) && !m_SelectedPath.empty())
			{
				auto it = std::find_if(m_Items.begin(), m_Items.end(), [this](const Item& item) { return item.Path == m_SelectedPath; });
				if (it != m_Items.end())
				{
					Item selected = *it;
					OpenItem(selected);
				}
			}
		}

		ImGui::End();
	}

	void ContentBrowser::DrawNavigationBar()
	{
		const float scale = ImGuiLayer::GetUIScale();
		const bool searching = !m_Filter.empty();

		ImGui::BeginDisabled(searching || m_CurrentDir == m_AssetsDir);
		if (UI::IconButton(RV_ICON_UP, "Up one folder (Backspace)"))
			NavigateTo(m_CurrentDir.parent_path());
		ImGui::EndDisabled();
		ImGui::SameLine();

		if (searching)
		{
			ImGui::AlignTextToFramePadding();
			ImGui::TextColored(Theme::TextMuted, "Search results in all assets");
		}
		else
		{
			// Breadcrumbs: Assets > Textures > ...
			std::vector<std::filesystem::path> crumbs;
			for (std::filesystem::path directory = m_CurrentDir; ; directory = directory.parent_path())
			{
				crumbs.push_back(directory);
				if (directory == m_AssetsDir || directory == directory.parent_path())
					break;
			}
			std::reverse(crumbs.begin(), crumbs.end());

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::SurfaceHovered);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::SurfaceActive);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f * scale, ImGui::GetStyle().FramePadding.y));
			for (size_t i = 0; i < crumbs.size(); i++)
			{
				const bool last = i + 1 == crumbs.size();
				if (i > 0)
				{
					ImGui::SameLine(0.0f, 0.0f);
					ImGui::AlignTextToFramePadding();
					ImGui::TextColored(Theme::TextDisabled, RV_ICON_CHEVRON_RIGHT);
					ImGui::SameLine(0.0f, 0.0f);
				}

				std::string label = i == 0 ? std::string("Assets") : ToUtf8(crumbs[i].filename());
				ImGui::PushID((int)i);
				ImGui::PushStyleColor(ImGuiCol_Text, last ? Theme::Text : Theme::TextMuted);
				if (ImGui::Button(label.c_str()) && !last)
					NavigateTo(crumbs[i]);
				ImGui::PopStyleColor();
				ImGui::PopID();
			}
			ImGui::PopStyleVar();
			ImGui::PopStyleColor(3);
		}

		// Search and view options on the right.
		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		const float searchWidth = 220.0f * scale;
		const float rightWidth = searchWidth + spacing + ImGui::GetFrameHeight();
		const float rightX = ImGui::GetWindowContentRegionMax().x - rightWidth;
		ImGui::SameLine();
		if (ImGui::GetCursorPosX() < rightX)
			ImGui::SetCursorPosX(rightX);

		if (UI::SearchBox("AssetSearch", m_Filter, "Search assets", searchWidth))
			m_NeedsRefresh = true;

		ImGui::SameLine();
		if (UI::IconButton(RV_ICON_TILES, "Thumbnail size (Ctrl + mouse wheel)"))
			ImGui::OpenPopup("ThumbnailSize");
		if (ImGui::BeginPopup("ThumbnailSize"))
		{
			ImGui::TextColored(Theme::TextMuted, "Thumbnail size");
			ImGui::SetNextItemWidth(180.0f * scale);
			ImGui::SliderFloat("##size", &m_ThumbnailSize, 48.0f, 192.0f, "%.0f px");
			ImGui::EndPopup();
		}
	}

	void ContentBrowser::DrawItem(const Item& item, int index, float tileWidth)
	{
		const ImGuiStyle& style = ImGui::GetStyle();
		const float padding = 6.0f * ImGuiLayer::GetUIScale();
		const float thumbnailSize = tileWidth - padding * 2.0f;
		const float lineHeight = ImGui::GetTextLineHeight();
		const ImVec2 tileSize(tileWidth, padding + thumbnailSize + style.ItemInnerSpacing.y + lineHeight + padding);

		std::error_code error;
		std::filesystem::path relativePath = std::filesystem::relative(item.Path, m_AssetsDir, error);

		ImGui::PushID(index);
		const ImVec2 tileMin = ImGui::GetCursorScreenPos();
		const ImVec2 tileMax(tileMin.x + tileSize.x, tileMin.y + tileSize.y);
		ImGui::InvisibleButton("##tile", tileSize);

		const bool hovered = ImGui::IsItemHovered();
		if (ImGui::IsItemClicked(ImGuiMouseButton_Left) || ImGui::IsItemClicked(ImGuiMouseButton_Right))
			m_SelectedPath = item.Path;
		const bool selected = m_SelectedPath == item.Path;
		bool open = hovered && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left);

		// Files drag onto the viewport (scenes) or onto Properties fields (textures, audio, scripts).
		if (item.Type != AssetType::Folder && ImGui::BeginDragDropSource())
		{
			std::wstring payload = relativePath.wstring();
			ImGui::SetDragDropPayload(ContentBrowserPayload, payload.c_str(), (payload.size() + 1) * sizeof(wchar_t));
			ImGui::TextColored(GetAssetColor(item.Type), "%s", GetAssetIcon(item.Type));
			ImGui::SameLine();
			ImGui::TextUnformatted(item.Name.c_str());
			ImGui::EndDragDropSource();
		}

		if (ImGui::BeginPopupContextItem("##itemContext"))
		{
			if (item.Type == AssetType::Folder || item.Type == AssetType::Scene)
			{
				if (ImGui::MenuItem(RV_ICON_FOLDER_OPEN "  Open"))
					open = true;
			}
			else if (ImGui::MenuItem(RV_ICON_FILE "  Open in Default App"))
				OpenWithDefaultApp(item.Path);

			if (ImGui::MenuItem(RV_ICON_FOLDER_OPEN "  Show in Explorer"))
				ShowInExplorer(item.Path);
			if (ImGui::MenuItem(RV_ICON_COPY "  Copy Path"))
				ImGui::SetClipboardText(ToUtf8(relativePath).c_str());
			ImGui::EndPopup();
		}

		if (hovered && !ImGui::IsMouseDragging(ImGuiMouseButton_Left))
		{
			std::string tooltip = item.Name + "\n" + GetAssetTypeName(item.Type) + "  -  " + ToUtf8(relativePath.parent_path().empty() ? "Assets" : "Assets" / relativePath.parent_path());
			UI::Tooltip(tooltip.c_str());
		}

		ImDrawList* drawList = ImGui::GetWindowDrawList();
		if (selected || hovered)
			drawList->AddRectFilled(tileMin, tileMax, ImGui::GetColorU32(selected ? Theme::Selection : Theme::SurfaceHovered), style.FrameRounding);

		const ImVec2 thumbnailMin(tileMin.x + padding, tileMin.y + padding);
		const ImVec2 thumbnailMax(thumbnailMin.x + thumbnailSize, thumbnailMin.y + thumbnailSize);

		if (item.Type == AssetType::Texture)
		{
			// The image itself, fitted into the square without stretching.
			Shared<Texture> texture = Renderer2D::GetTexture(item.Path.string());
			float width = (float)texture->GetWidth();
			float height = (float)texture->GetHeight();
			float fit = thumbnailSize / (width > height ? width : height);
			ImVec2 imageSize(width * fit, height * fit);
			ImVec2 imageMin(thumbnailMin.x + (thumbnailSize - imageSize.x) * 0.5f, thumbnailMin.y + (thumbnailSize - imageSize.y) * 0.5f);

			drawList->AddRectFilled(thumbnailMin, thumbnailMax, ImGui::GetColorU32(ImGuiCol_FrameBg), style.FrameRounding);
			drawList->AddImage((ImTextureID)(intptr_t)texture->GetID(), imageMin, ImVec2(imageMin.x + imageSize.x, imageMin.y + imageSize.y), ImVec2(0, 1), ImVec2(1, 0));
		}
		else
		{
			ImFont* iconFont = ImGuiLayer::GetFont(FontStyle::Icons);
			const char* icon = GetAssetIcon(item.Type);
			float iconSize = thumbnailSize * 0.62f;
			ImVec2 iconExtent = iconFont->CalcTextSizeA(iconSize, FLT_MAX, 0.0f, icon);
			ImVec2 iconPos(thumbnailMin.x + (thumbnailSize - iconExtent.x) * 0.5f, thumbnailMin.y + (thumbnailSize - iconExtent.y) * 0.5f);
			drawList->AddText(iconFont, iconSize, iconPos, ImGui::GetColorU32(GetAssetColor(item.Type)), icon);
		}

		// The name, centered under the thumbnail and cut short with "..." when it doesn't fit.
		const float textTop = thumbnailMax.y + style.ItemInnerSpacing.y;
		const ImVec2 textSize = ImGui::CalcTextSize(item.Name.c_str());
		const float textLeft = tileMin.x + padding * 0.5f;
		const float textRight = tileMax.x - padding * 0.5f;
		if (textSize.x <= textRight - textLeft)
			drawList->AddText(ImVec2(tileMin.x + (tileSize.x - textSize.x) * 0.5f, textTop), ImGui::GetColorU32(ImGuiCol_Text), item.Name.c_str());
		else
			ImGui::RenderTextEllipsis(drawList, ImVec2(textLeft, textTop), ImVec2(textRight, textTop + lineHeight), textRight, textRight, item.Name.c_str(), nullptr, &textSize);

		ImGui::PopID();

		if (open)
			OpenItem(item);
	}

	void ContentBrowser::OpenItem(const Item& item)
	{
		switch (item.Type)
		{
		case AssetType::Folder:
			m_Filter.clear();
			NavigateTo(item.Path);
			break;
		case AssetType::Scene:
			if (m_OnOpenScene)
				m_OnOpenScene(item.Path);
			break;
		default:
			OpenWithDefaultApp(item.Path);
			break;
		}
	}
}
