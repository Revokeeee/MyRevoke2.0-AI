#include "rvpch.h"
#include "ImGuiTheme.h"

namespace Revoke
{
	namespace Theme
	{
		static ImVec4 WithAlpha(ImVec4 color, float alpha)
		{
			color.w = alpha;
			return color;
		}

		void Apply(float scale)
		{
			ImGuiStyle& style = ImGui::GetStyle();
			style = ImGuiStyle();

			style.WindowPadding = ImVec2(8.0f, 8.0f);
			style.FramePadding = ImVec2(8.0f, 4.0f);
			style.CellPadding = ImVec2(6.0f, 4.0f);
			style.ItemSpacing = ImVec2(8.0f, 6.0f);
			style.ItemInnerSpacing = ImVec2(6.0f, 4.0f);
			style.IndentSpacing = 16.0f;
			style.ScrollbarSize = 12.0f;
			style.GrabMinSize = 10.0f;

			style.WindowBorderSize = 1.0f;
			style.ChildBorderSize = 1.0f;
			style.PopupBorderSize = 1.0f;
			style.FrameBorderSize = 0.0f;
			style.TabBorderSize = 0.0f;
			style.TabBarBorderSize = 1.0f;
			style.DockingSeparatorSize = 2.0f;

			style.WindowRounding = 6.0f;
			style.ChildRounding = 4.0f;
			style.FrameRounding = 4.0f;
			style.PopupRounding = 6.0f;
			style.ScrollbarRounding = 6.0f;
			style.GrabRounding = 4.0f;
			style.TabRounding = 4.0f;

			style.WindowTitleAlign = ImVec2(0.0f, 0.5f);
			// The little dock-menu triangle in every tab bar adds clutter and nothing the tab's own
			// context menu doesn't already offer.
			style.WindowMenuButtonPosition = ImGuiDir_None;
			style.SeparatorTextBorderSize = 1.0f;
			style.SeparatorTextPadding = ImVec2(0.0f, 4.0f);

			style.ScaleAllSizes(scale);

			ImVec4* colors = style.Colors;
			colors[ImGuiCol_Text]                   = Text;
			colors[ImGuiCol_TextDisabled]           = TextDisabled;
			colors[ImGuiCol_WindowBg]               = Background;
			colors[ImGuiCol_ChildBg]                = WithAlpha(Background, 0.0f);
			colors[ImGuiCol_PopupBg]                = WithAlpha(Surface, 0.98f);
			colors[ImGuiCol_Border]                 = Border;
			colors[ImGuiCol_BorderShadow]           = { 0.0f, 0.0f, 0.0f, 0.0f };

			colors[ImGuiCol_FrameBg]                = { 0.067f, 0.071f, 0.082f, 1.00f };
			colors[ImGuiCol_FrameBgHovered]         = { 0.098f, 0.102f, 0.118f, 1.00f };
			colors[ImGuiCol_FrameBgActive]          = { 0.118f, 0.122f, 0.141f, 1.00f };

			colors[ImGuiCol_TitleBg]                = BackgroundDark;
			colors[ImGuiCol_TitleBgActive]          = BackgroundDark;
			colors[ImGuiCol_TitleBgCollapsed]       = BackgroundDark;
			colors[ImGuiCol_MenuBarBg]              = BackgroundDark;

			colors[ImGuiCol_ScrollbarBg]            = { 0.0f, 0.0f, 0.0f, 0.0f };
			colors[ImGuiCol_ScrollbarGrab]          = { 0.227f, 0.235f, 0.259f, 1.00f };
			colors[ImGuiCol_ScrollbarGrabHovered]   = { 0.290f, 0.302f, 0.329f, 1.00f };
			colors[ImGuiCol_ScrollbarGrabActive]    = { 0.337f, 0.353f, 0.384f, 1.00f };

			colors[ImGuiCol_CheckMark]              = Accent;
			colors[ImGuiCol_SliderGrab]             = WithAlpha(Accent, 0.85f);
			colors[ImGuiCol_SliderGrabActive]       = Accent;

			colors[ImGuiCol_Button]                 = SurfaceHovered;
			colors[ImGuiCol_ButtonHovered]          = { 0.235f, 0.243f, 0.271f, 1.00f };
			colors[ImGuiCol_ButtonActive]           = { 0.282f, 0.294f, 0.325f, 1.00f };

			// Header also paints CollapsingHeader, so it stays neutral. Panels push Theme::Selection
			// for rows that are actually selected.
			colors[ImGuiCol_Header]                 = SurfaceHovered;
			colors[ImGuiCol_HeaderHovered]          = SurfaceActive;
			colors[ImGuiCol_HeaderActive]           = { 0.270f, 0.282f, 0.314f, 1.00f };

			colors[ImGuiCol_Separator]              = Border;
			colors[ImGuiCol_SeparatorHovered]       = WithAlpha(Accent, 0.70f);
			colors[ImGuiCol_SeparatorActive]        = Accent;

			colors[ImGuiCol_ResizeGrip]             = { 0.0f, 0.0f, 0.0f, 0.0f };
			colors[ImGuiCol_ResizeGripHovered]      = WithAlpha(Accent, 0.50f);
			colors[ImGuiCol_ResizeGripActive]       = Accent;

			// The active tab matches the window under it, so the two read as one piece.
			colors[ImGuiCol_Tab]                    = BackgroundDark;
			colors[ImGuiCol_TabHovered]             = SurfaceHovered;
			colors[ImGuiCol_TabActive]              = Background;
			colors[ImGuiCol_TabUnfocused]           = BackgroundDark;
			colors[ImGuiCol_TabUnfocusedActive]     = Background;

			colors[ImGuiCol_DockingPreview]         = WithAlpha(Accent, 0.55f);
			colors[ImGuiCol_DockingEmptyBg]         = BackgroundDark;

			colors[ImGuiCol_PlotLines]              = Accent;
			colors[ImGuiCol_PlotLinesHovered]       = AccentHovered;
			colors[ImGuiCol_PlotHistogram]          = Accent;
			colors[ImGuiCol_PlotHistogramHovered]   = AccentHovered;

			colors[ImGuiCol_TableHeaderBg]          = Surface;
			colors[ImGuiCol_TableBorderStrong]      = Border;
			colors[ImGuiCol_TableBorderLight]       = { 0.149f, 0.157f, 0.176f, 1.00f };
			colors[ImGuiCol_TableRowBg]             = { 0.0f, 0.0f, 0.0f, 0.0f };
			colors[ImGuiCol_TableRowBgAlt]          = { 1.0f, 1.0f, 1.0f, 0.02f };

			colors[ImGuiCol_TextSelectedBg]         = WithAlpha(Accent, 0.35f);
			colors[ImGuiCol_DragDropTarget]         = Accent;
			colors[ImGuiCol_NavHighlight]           = Accent;
			colors[ImGuiCol_NavWindowingHighlight]  = { 1.0f, 1.0f, 1.0f, 0.70f };
			colors[ImGuiCol_NavWindowingDimBg]      = { 0.0f, 0.0f, 0.0f, 0.45f };
			colors[ImGuiCol_ModalWindowDimBg]       = { 0.0f, 0.0f, 0.0f, 0.55f };
		}
	}
}
