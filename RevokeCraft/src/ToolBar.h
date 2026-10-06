#pragma once

#include "MyRevoke/Renderer/Texture.h"
#include "MyRevoke/Core/Core.h"
#include <functional>

namespace Revoke
{
	enum class SceneState
	{
		Editor = 0,
		Runtime = 1
	};

	class ToolBar
	{
	public:
		ToolBar();
		void OnImGuiRender();

		void OnScenePlay();
		void OnSceneStop();

		// The owner creates and discards the play-mode scene; the toolbar only tracks the state.
		void SetPlayCallbacks(std::function<void()> onPlay, std::function<void()> onStop);
		void SetGuizmo(int* guizmo);

		SceneState GetSceneState() const { return m_SceneState; }
	private:
	
		SceneState m_SceneState = SceneState::Editor;

		Shared<Texture> m_PlayIcon;
		Shared<Texture> m_StopIcon;

		std::function<void()> m_OnPlay;
		std::function<void()> m_OnStop;

		int* m_Guizmo;

	};
}


