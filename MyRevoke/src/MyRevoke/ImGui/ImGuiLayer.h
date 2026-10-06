#pragma once

#include "MyRevoke/Core/Layer.h"

#include "MyRevoke/EventSystem/MouseEvent.h"
#include "MyRevoke/EventSystem/KeyEvent.h"
#include "MyRevoke/EventSystem/AppEvent.h"

struct ImFont;

namespace Revoke {

	enum class FontStyle
	{
		Regular = 0,
		Bold,
		// Larger semibold text, for headings and the toolbar's icons.
		Large,
		// Fixed width, for the console and anything code-like.
		Mono,
		Count
	};

	class  ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void OnAttach() override;
		void OnDetach() override;
		void OnImGuiDraw() override;
		void OnEvent(Event& e) override;

		void Begin();
		void End();

		void BlockEvents(bool block) { m_BlockEvents = block; }

		// Never null after OnAttach: falls back to ImGui's built-in font when a font file is missing.
		static ImFont* GetFont(FontStyle style);
		// The monitor's content scale (1 at 100%), already applied to every font and style size.
		static float GetUIScale();
	private:
		bool m_BlockEvents = true;
		float m_Time = 0.0f;
	};

}
