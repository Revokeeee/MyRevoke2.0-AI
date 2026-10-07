#include "rvpch.h"

#include "ToolBar.h"

#include "MyRevoke/ImGui/ImGuiLayer.h"
#include "MyRevoke/ImGui/ImGuiTheme.h"
#include "MyRevoke/ImGui/ImGuiIcons.h"

#include <imgui.h>

namespace Revoke
{
	namespace
	{
		void Tooltip(const char* text)
		{
			if (!ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort | ImGuiHoveredFlags_NoSharedDelay))
				return;

			// The buttons use the large icon font; the tooltip should not.
			ImGui::PushFont(ImGuiLayer::GetFont(FontStyle::Regular));
			ImGui::SetTooltip("%s", text);
			ImGui::PopFont();
		}

		// A square icon button. Active buttons get the accent so the current tool is obvious.
		bool ToolButton(const char* icon, const char* tooltip, bool active, float size)
		{
			if (active)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, Theme::Selection);
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::SelectionHovered);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::SelectionHovered);
				ImGui::PushStyleColor(ImGuiCol_Text, Theme::AccentHovered);
			}
			else
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::SurfaceHovered);
				ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::SurfaceActive);
				ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_Text));
			}

			bool pressed = ImGui::Button(icon, ImVec2(size, size));
			ImGui::PopStyleColor(4);
			Tooltip(tooltip);
			return pressed;
		}

		void GroupSeparator(float height)
		{
			ImGui::SameLine();
			ImVec2 cursor = ImGui::GetCursorScreenPos();
			float x = cursor.x + ImGui::GetStyle().ItemSpacing.x * 2.0f;
			ImGui::GetWindowDrawList()->AddLine(ImVec2(x, cursor.y + height * 0.2f), ImVec2(x, cursor.y + height * 0.8f), ImGui::GetColorU32(Theme::Border), 1.0f);
			ImGui::Dummy(ImVec2(ImGui::GetStyle().ItemSpacing.x * 4.0f, height));
			ImGui::SameLine();
		}
	}

	void ToolBar::OnImGuiRender()
	{
		const float scale = ImGuiLayer::GetUIScale();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(6.0f * scale, 3.0f * scale));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2.0f * scale, 0.0f));
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		ImGui::PushFont(ImGuiLayer::GetFont(FontStyle::Large));

		const float buttonSize = ImGui::GetContentRegionAvail().y > 16.0f * scale ? ImGui::GetContentRegionAvail().y : 16.0f * scale;
		const bool playing = m_SceneState == SceneState::Runtime;

		// Transform tools. The gizmo is hidden while playing, so they are too.
		ImGui::BeginDisabled(playing);
		if (ToolButton(RV_ICON_SELECT, "Select (Q)", m_GizmoTool == GizmoTool::Select, buttonSize))
			m_GizmoTool = GizmoTool::Select;
		ImGui::SameLine();
		if (ToolButton(RV_ICON_MOVE, "Move (W)", m_GizmoTool == GizmoTool::Translate, buttonSize))
			m_GizmoTool = GizmoTool::Translate;
		ImGui::SameLine();
		if (ToolButton(RV_ICON_ROTATE, "Rotate (E)", m_GizmoTool == GizmoTool::Rotate, buttonSize))
			m_GizmoTool = GizmoTool::Rotate;
		ImGui::SameLine();
		if (ToolButton(RV_ICON_SCALE, "Scale (R)", m_GizmoTool == GizmoTool::Scale, buttonSize))
			m_GizmoTool = GizmoTool::Scale;

		GroupSeparator(buttonSize);
		if (ToolButton(m_GizmoLocal ? RV_ICON_CUBE : RV_ICON_WORLD, m_GizmoLocal ? "Local space - click for world space" : "World space - click for local space", false, buttonSize))
			m_GizmoLocal = !m_GizmoLocal;
		ImGui::SameLine();
		if (ToolButton(RV_ICON_GRID, "Snap to 0.5 units / 45 degrees (hold Ctrl to invert)", m_SnapEnabled, buttonSize))
			m_SnapEnabled = !m_SnapEnabled;
		ImGui::EndDisabled();

		// Play / Stop in the middle, colored so play mode can't be mistaken for editing.
		ImGui::SameLine((ImGui::GetWindowContentRegionMax().x - buttonSize) * 0.5f);
		ImGui::PushStyleColor(ImGuiCol_Text, playing ? Theme::Error : Theme::Success);
		bool togglePlay = ToolButton(playing ? RV_ICON_STOP : RV_ICON_PLAY, playing ? "Stop (Ctrl+P)" : "Play (Ctrl+P)", false, buttonSize);
		ImGui::PopStyleColor();

		if (playing)
		{
			ImGui::SameLine();
			ImGui::PushFont(ImGuiLayer::GetFont(FontStyle::Bold));
			ImGui::SetCursorPosY((ImGui::GetWindowHeight() - ImGui::GetTextLineHeight()) * 0.5f);
			ImGui::TextColored(Theme::Accent, "  PLAYING");
			ImGui::PopFont();
		}

		ImGui::PopFont();
		ImGui::End();
		ImGui::PopStyleVar(3);

		// After End(), so the play callbacks never run while this window is half drawn.
		if (togglePlay)
			TogglePlay();
	}
	void ToolBar::OnScenePlay()
	{
		m_OnPlay();
		m_SceneState = SceneState::Runtime;
	}
	void ToolBar::OnSceneStop()
	{
		m_OnStop();
		m_SceneState = SceneState::Editor;
	}
	void ToolBar::TogglePlay()
	{
		if (m_SceneState == SceneState::Editor)
			OnScenePlay();
		else
			OnSceneStop();
	}
	void ToolBar::SetPlayCallbacks(std::function<void()> onPlay, std::function<void()> onStop)
	{
		m_OnPlay = onPlay;
		m_OnStop = onStop;
	}
}
