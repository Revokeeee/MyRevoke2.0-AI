#include "rvpch.h"
#include "ConsolePanel.h"
#include "EditorUI.h"

#include "MyRevoke/ImGui/ImGuiLayer.h"
#include "MyRevoke/ImGui/ImGuiTheme.h"
#include "MyRevoke/ImGui/ImGuiIcons.h"

#include <imgui.h>

namespace Revoke
{
	namespace
	{
		enum class Severity { Info, Warning, Error };

		Severity GetSeverity(spdlog::level::level_enum level)
		{
			if (level >= spdlog::level::err)
				return Severity::Error;
			if (level == spdlog::level::warn)
				return Severity::Warning;
			return Severity::Info;
		}

		ImVec4 GetSeverityColor(Severity severity)
		{
			switch (severity)
			{
			case Severity::Error:   return Theme::Error;
			case Severity::Warning: return Theme::Warning;
			default:                return Theme::Info;
			}
		}

		const char* GetSeverityIcon(Severity severity)
		{
			switch (severity)
			{
			case Severity::Error:   return RV_ICON_ERROR;
			case Severity::Warning: return RV_ICON_WARNING;
			default:                return RV_ICON_INFO;
			}
		}

		// A filter button that shows how many messages it covers, lit while its level is shown.
		void LevelToggle(const char* icon, int count, bool& shown, const ImVec4& color, const char* tooltip)
		{
			char label[32];
			std::snprintf(label, sizeof(label), "%s %d", icon, count);

			ImGui::PushStyleColor(ImGuiCol_Button, shown ? Theme::SurfaceHovered : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::SurfaceActive);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::SurfaceActive);
			ImGui::PushStyleColor(ImGuiCol_Text, shown ? color : Theme::TextDisabled);
			if (ImGui::Button(label))
				shown = !shown;
			ImGui::PopStyleColor(4);
			UI::Tooltip(tooltip);
		}
	}

	void ConsolePanel::Refresh()
	{
		uint64_t version = Log::GetHistoryVersion();
		if (version == m_SeenVersion)
			return;

		m_Entries = Log::GetHistory();
		m_SeenVersion = version;
		m_NewEntries = true;

		m_InfoCount = m_WarningCount = m_ErrorCount = 0;
		for (const LogEntry& entry : m_Entries)
		{
			switch (GetSeverity(entry.Level))
			{
			case Severity::Error:   m_ErrorCount++; break;
			case Severity::Warning: m_WarningCount++; break;
			default:                m_InfoCount++; break;
			}
		}
	}

	void ConsolePanel::OnImGuiRender()
	{
		Refresh();

		if (!ImGui::Begin("Console"))
		{
			ImGui::End();
			return;
		}

		// Toolbar: clear, level filters, search, auto-scroll.
		if (UI::IconButton(RV_ICON_CLEAR, "Clear"))
			Log::ClearHistory();
		ImGui::SameLine();
		LevelToggle(RV_ICON_INFO, m_InfoCount, m_ShowInfo, Theme::Info, "Show messages");
		ImGui::SameLine(0.0f, 2.0f);
		LevelToggle(RV_ICON_WARNING, m_WarningCount, m_ShowWarnings, Theme::Warning, "Show warnings");
		ImGui::SameLine(0.0f, 2.0f);
		LevelToggle(RV_ICON_ERROR, m_ErrorCount, m_ShowErrors, Theme::Error, "Show errors");
		ImGui::SameLine();

		const float autoScrollWidth = ImGui::GetFrameHeight();
		UI::SearchBox("ConsoleSearch", m_Filter, "Filter messages", ImGui::GetContentRegionAvail().x - autoScrollWidth - ImGui::GetStyle().ItemSpacing.x);
		ImGui::SameLine();
		ImGui::PushStyleColor(ImGuiCol_Text, m_AutoScroll ? Theme::Accent : Theme::TextDisabled);
		if (UI::IconButton(RV_ICON_PIN, m_AutoScroll ? "Auto-scroll is on" : "Auto-scroll is off"))
			m_AutoScroll = !m_AutoScroll;
		ImGui::PopStyleColor();

		std::vector<int> visible;
		visible.reserve(m_Entries.size());
		for (int i = 0; i < (int)m_Entries.size(); i++)
		{
			const LogEntry& entry = m_Entries[i];
			Severity severity = GetSeverity(entry.Level);
			bool shown = (severity == Severity::Info && m_ShowInfo) || (severity == Severity::Warning && m_ShowWarnings) || (severity == Severity::Error && m_ShowErrors);
			if (shown && UI::MatchesFilter(entry.Message, m_Filter))
				visible.push_back(i);
		}

		if (visible.empty())
		{
			UI::EmptyState(RV_ICON_CONSOLE, m_Entries.empty() ? "No messages." : "No message matches the filters.");
			ImGui::End();
			m_NewEntries = false;
			return;
		}

		ImGui::PushFont(ImGuiLayer::GetFont(FontStyle::Mono));
		const ImGuiTableFlags tableFlags = ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_PadOuterX;
		if (ImGui::BeginTable("##log", 4, tableFlags))
		{
			ImGui::TableSetupScrollFreeze(0, 0);
			ImGui::TableSetupColumn("Time", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Level", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Source", ImGuiTableColumnFlags_WidthFixed);
			ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);

			// Only the rows on screen are laid out, so a long log stays cheap.
			ImGuiListClipper clipper;
			clipper.Begin((int)visible.size());
			while (clipper.Step())
			{
				for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++)
				{
					const LogEntry& entry = m_Entries[visible[row]];
					const Severity severity = GetSeverity(entry.Level);
					ImGui::PushID(row);
					ImGui::TableNextRow();

					ImGui::TableSetColumnIndex(0);
					// Spans the row so it can be right-clicked anywhere; the text sits on top.
					ImGui::Selectable("##row", false, ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowOverlap);
					if (ImGui::BeginPopupContextItem("##rowContext"))
					{
						ImGui::PushFont(ImGuiLayer::GetFont(FontStyle::Regular));
						if (ImGui::MenuItem(RV_ICON_COPY "  Copy Message"))
							ImGui::SetClipboardText(entry.Message.c_str());
						ImGui::PopFont();
						ImGui::EndPopup();
					}
					ImGui::SameLine(0.0f, 0.0f);
					ImGui::TextColored(Theme::TextDisabled, "%s", entry.Time.c_str());

					ImGui::TableSetColumnIndex(1);
					ImGui::TextColored(GetSeverityColor(severity), "%s", GetSeverityIcon(severity));

					ImGui::TableSetColumnIndex(2);
					ImGui::TextColored(Theme::TextMuted, "%s", entry.Source == "APP" ? "Editor" : "Engine");

					ImGui::TableSetColumnIndex(3);
					ImGui::PushStyleColor(ImGuiCol_Text, severity == Severity::Info ? Theme::Text : GetSeverityColor(severity));
					ImGui::TextUnformatted(entry.Message.c_str());
					ImGui::PopStyleColor();

					ImGui::PopID();
				}
			}

			// Follow new messages while scrolled to the bottom.
			if (m_AutoScroll && m_NewEntries && ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - ImGui::GetTextLineHeight() * 2.0f)
				ImGui::SetScrollHereY(1.0f);

			ImGui::EndTable();
		}
		ImGui::PopFont();

		m_NewEntries = false;
		ImGui::End();
	}
}
