#pragma once

#include "MyRevoke/Core/Core.h"
#include <functional>

namespace Revoke
{
	enum class SceneState
	{
		Editor = 0,
		Runtime = 1
	};

	// What the transform gizmo does to the selected entity.
	enum class GizmoTool
	{
		Select = 0,
		Translate,
		Rotate,
		Scale
	};

	class ToolBar
	{
	public:
		ToolBar() = default;
		void OnImGuiRender();

		void OnScenePlay();
		void OnSceneStop();
		void TogglePlay();

		// The owner creates and discards the play-mode scene; the toolbar only tracks the state.
		void SetPlayCallbacks(std::function<void()> onPlay, std::function<void()> onStop);

		SceneState GetSceneState() const { return m_SceneState; }

		GizmoTool GetGizmoTool() const { return m_GizmoTool; }
		void SetGizmoTool(GizmoTool tool) { m_GizmoTool = tool; }
		// Local: the gizmo follows the entity's rotation. Otherwise it is aligned to the world axes.
		bool IsGizmoLocal() const { return m_GizmoLocal; }
		// Snapping is on while this is set, or while Ctrl is held; Ctrl inverts it.
		bool IsSnapEnabled() const { return m_SnapEnabled; }
	private:
		SceneState m_SceneState = SceneState::Editor;

		GizmoTool m_GizmoTool = GizmoTool::Translate;
		bool m_GizmoLocal = true;
		bool m_SnapEnabled = false;

		std::function<void()> m_OnPlay;
		std::function<void()> m_OnStop;
	};
}
