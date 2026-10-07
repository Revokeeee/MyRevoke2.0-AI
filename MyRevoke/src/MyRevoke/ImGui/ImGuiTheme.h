#pragma once

#include <imgui.h>

namespace Revoke
{
	// The editor's palette. Panels take colors from here instead of hard-coding them, so the
	// whole editor keeps one look.
	namespace Theme
	{
		constexpr ImVec4 Accent         = { 0.910f, 0.569f, 0.176f, 1.00f };
		constexpr ImVec4 AccentHovered  = { 0.949f, 0.647f, 0.294f, 1.00f };
		constexpr ImVec4 AccentActive   = { 0.816f, 0.478f, 0.086f, 1.00f };

		constexpr ImVec4 Text           = { 0.890f, 0.898f, 0.910f, 1.00f };
		constexpr ImVec4 TextMuted      = { 0.560f, 0.575f, 0.610f, 1.00f };
		constexpr ImVec4 TextDisabled   = { 0.420f, 0.435f, 0.470f, 1.00f };

		constexpr ImVec4 Background     = { 0.122f, 0.125f, 0.141f, 1.00f };
		constexpr ImVec4 BackgroundDark = { 0.078f, 0.082f, 0.094f, 1.00f };
		constexpr ImVec4 Surface        = { 0.157f, 0.161f, 0.180f, 1.00f };
		constexpr ImVec4 SurfaceHovered = { 0.196f, 0.204f, 0.227f, 1.00f };
		constexpr ImVec4 SurfaceActive  = { 0.235f, 0.243f, 0.271f, 1.00f };
		// Background of a selected row (hierarchy entity, content browser item).
		constexpr ImVec4 Selection      = { 0.910f, 0.569f, 0.176f, 0.32f };
		constexpr ImVec4 SelectionHovered = { 0.910f, 0.569f, 0.176f, 0.42f };
		constexpr ImVec4 Border         = { 0.180f, 0.188f, 0.212f, 1.00f };

		constexpr ImVec4 AxisX          = { 0.851f, 0.286f, 0.290f, 1.00f };
		constexpr ImVec4 AxisY          = { 0.392f, 0.722f, 0.290f, 1.00f };
		constexpr ImVec4 AxisZ          = { 0.247f, 0.502f, 0.886f, 1.00f };

		constexpr ImVec4 Success        = { 0.392f, 0.780f, 0.420f, 1.00f };
		constexpr ImVec4 Warning        = { 0.957f, 0.769f, 0.282f, 1.00f };
		constexpr ImVec4 Error          = { 0.937f, 0.341f, 0.341f, 1.00f };
		constexpr ImVec4 Info           = { 0.420f, 0.678f, 0.957f, 1.00f };

		// Sets every ImGui color and size. scale is the monitor's content scale (1 at 100%).
		void Apply(float scale = 1.0f);
	}
}
