#include <MyRevoke.h>
#include "MyRevoke/Core/EntryPoint.h"
#include "MyRevoke/Core/Log.h"
#include "MyRevoke/Project/Project.h"
#include "MyRevoke/Utility/ExecutablePath.h"

#include "PlayerLayer.h"

namespace Revoke {

	// An exported game keeps its project file next to the exe. Running the player straight out of the
	// repo has none there, so fall back to the example project the editor opens.
	static std::filesystem::path FindProjectFile()
	{
		for (const auto& entry : std::filesystem::directory_iterator(GetExecutableDirectory()))
		{
			if (entry.path().extension() == ".mrproject")
				return entry.path();
		}

		return GetExecutableDirectory() / "projects" / "Example" / "Example.mrproject";
	}

	static WindowSettings WindowSettingsFor(const Shared<Project>& project)
	{
		if (!project)
			return WindowSettings();

		return WindowSettings(project->GetGameName(), project->GetWindowWidth(), project->GetWindowHeight(), false, project->IsFullscreen());
	}

	class RevokePlayer : public Revoke::Application
	{
	public:
		RevokePlayer(const Shared<Project>& project)
			: Application(WindowSettingsFor(project))
		{
			PushLayer(new PlayerLayer(project));
		}
	};

	Revoke::Application* Revoke::CreateApplication()
	{
		// The Application constructor creates the window, so the settings it takes have to be read
		// out of the project before it runs - including the logging that reading does.
		Log::Init();
		return new RevokePlayer(Project::Load(FindProjectFile()));
	}
};
