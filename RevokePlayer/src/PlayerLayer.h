#pragma once

#include "MyRevoke.h"

#include "MyRevoke/Project/Project.h"

namespace Revoke
{
	// Runs a project's startup scene the way the editor's Play mode does, with no editor UI.
	class PlayerLayer : public Layer
	{
	public:
		PlayerLayer();

		void OnAttach() override;
		void OnDetach() override;

		void OnUpdate(Timestep deltaTime) override;
		bool OnCloseRequested() override;

	private:
		void StopRuntime();

		// Null when the project or its startup scene could not be loaded.
		Shared<Scene> m_Scene;
		uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;
	};
}
