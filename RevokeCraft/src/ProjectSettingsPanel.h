#pragma once

#include "MyRevoke/Project/Project.h"

namespace Revoke
{
	// Edits the settings a shipped game needs out of its project file: what it is called, the scene
	// it starts in, and the window RevokePlayer opens for it.
	class ProjectSettingsPanel
	{
	public:
		ProjectSettingsPanel() = default;

		void OnImGuiRender();

		void SetProject(Shared<Project> project);

	private:
		Shared<Project> m_Project;
	};
}
