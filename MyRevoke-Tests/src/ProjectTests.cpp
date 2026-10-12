#include <doctest/doctest.h>

#include "MyRevoke/Core/Log.h"
#include "MyRevoke/Project/Project.h"

#include <fstream>

using namespace Revoke;

// RevokePlayer picks the scene it starts a game in out of the project file, so Load has to read the
// key back and GetStartupScenePath has to resolve it against the project's own assets folder.

namespace
{
	// Project::Load logs through the engine logger when a file is missing or malformed.
	struct TestProjectFile
	{
		TestProjectFile(const std::string& contents)
		{
			Log::Init();

			Directory = std::filesystem::temp_directory_path() / "MyRevoke-ProjectTests";
			std::filesystem::create_directories(Directory);

			Path = Directory / "Game.mrproject";
			std::ofstream(Path) << contents;
		}

		~TestProjectFile()
		{
			std::error_code error;
			std::filesystem::remove_all(Directory, error);
		}

		std::filesystem::path Directory;
		std::filesystem::path Path;
	};
}

TEST_CASE("A startup scene in the project file resolves against the assets folder")
{
	TestProjectFile file("Project: Game\nAssetsFolder: assets\nStartupScene: Scenes/Level1.myrevoke\n");

	Shared<Project> project = Project::Load(file.Path);
	REQUIRE(project != nullptr);

	CHECK(project->GetStartupScenePath() == file.Directory / "assets" / "Scenes" / "Level1.myrevoke");
}

TEST_CASE("A project file without a startup scene has no startup scene path")
{
	TestProjectFile file("Project: Game\nAssetsFolder: assets\n");

	Shared<Project> project = Project::Load(file.Path);
	REQUIRE(project != nullptr);

	CHECK(project->GetStartupScenePath().empty());
}
