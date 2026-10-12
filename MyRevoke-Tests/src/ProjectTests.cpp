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

TEST_CASE("Game settings come back out of the project file")
{
	TestProjectFile file("Project: Game\nGameName: Space Crabs\nAssetsFolder: assets\nWindowWidth: 800\nWindowHeight: 600\nFullscreen: true\n");

	Shared<Project> project = Project::Load(file.Path);
	REQUIRE(project != nullptr);

	CHECK(project->GetGameName() == "Space Crabs");
	CHECK(project->GetWindowWidth() == 800);
	CHECK(project->GetWindowHeight() == 600);
	CHECK(project->IsFullscreen());
}

TEST_CASE("A project file written before the game settings existed still has usable ones")
{
	TestProjectFile file("Project: Game\nAssetsFolder: assets\n");

	Shared<Project> project = Project::Load(file.Path);
	REQUIRE(project != nullptr);

	CHECK(project->GetGameName() == "Game");
	CHECK(project->GetWindowWidth() == 1280);
	CHECK(project->GetWindowHeight() == 720);
	CHECK_FALSE(project->IsFullscreen());
}

// Project Settings edits only reach RevokePlayer through the project file, so what Save writes
// has to load back unchanged.
TEST_CASE("Saved game settings load back")
{
	TestProjectFile file("Project: Game\nAssetsFolder: assets\n");

	Shared<Project> project = Project::Load(file.Path);
	REQUIRE(project != nullptr);

	project->SetGameName("Space Crabs");
	project->SetStartupScene("Scenes/Level2.myrevoke");
	project->SetWindowSize(1024, 768);
	project->SetFullscreen(true);
	project->Save();

	Shared<Project> reloaded = Project::Load(file.Path);
	REQUIRE(reloaded != nullptr);

	CHECK(reloaded->GetGameName() == "Space Crabs");
	CHECK(reloaded->GetStartupScenePath() == file.Directory / "assets" / "Scenes" / "Level2.myrevoke");
	CHECK(reloaded->GetWindowWidth() == 1024);
	CHECK(reloaded->GetWindowHeight() == 768);
	CHECK(reloaded->IsFullscreen());
}
