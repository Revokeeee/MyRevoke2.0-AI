#pragma once

#include <filesystem>
#include <string>

#include "MyRevoke/Core/Core.h"

namespace Revoke
{
	// The user's game content lives in a project folder the user picks, never in the editor's own
	// install folder. A project is a <Name>.mrproject file plus the assets folder next to it.
	class Project
	{
	public:
		static Shared<Project> Create(const std::string& name, const std::filesystem::path& parentDirectory);
		static Shared<Project> Load(const std::filesystem::path& projectFilePath);

		const std::string& GetName() const { return m_Name; }
		const std::filesystem::path& GetDirectory() const { return m_Directory; }

		std::filesystem::path GetAssetsDirectory() const { return m_Directory / m_AssetsFolder; }
		std::filesystem::path GetScenesDirectory() const { return GetAssetsDirectory() / "Scenes"; }
		std::filesystem::path GetScriptsDirectory() const { return GetAssetsDirectory() / "Scripts"; }

		// What the shipped game is called. It is the player's window title, and it can differ from
		// the project's name, which is fixed by the .mrproject file's name.
		const std::string& GetGameName() const { return m_GameName; }
		void SetGameName(const std::string& gameName) { m_GameName = gameName; }

		// Relative to the assets folder, so the project stays valid when it is moved.
		const std::string& GetStartupScene() const { return m_StartupScene; }
		void SetStartupScene(const std::string& sceneRelativeToAssets) { m_StartupScene = sceneRelativeToAssets; }

		// The scene a game built from this project starts in. Empty when the project file names none.
		std::filesystem::path GetStartupScenePath() const;

		// The window the shipped game opens.
		unsigned int GetWindowWidth() const { return m_WindowWidth; }
		unsigned int GetWindowHeight() const { return m_WindowHeight; }
		bool IsFullscreen() const { return m_Fullscreen; }
		void SetWindowSize(unsigned int width, unsigned int height) { m_WindowWidth = width; m_WindowHeight = height; }
		void SetFullscreen(bool fullscreen) { m_Fullscreen = fullscreen; }

		// Writes the project file back. The editor calls this after every settings change.
		void Save() const;

	private:
		std::string m_Name;
		std::string m_GameName;
		std::string m_AssetsFolder = "assets";
		std::string m_StartupScene;
		std::filesystem::path m_Directory;

		unsigned int m_WindowWidth = 1280;
		unsigned int m_WindowHeight = 720;
		bool m_Fullscreen = false;
	};
}
