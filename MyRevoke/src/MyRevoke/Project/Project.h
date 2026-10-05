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

	private:
		void Save() const;

		std::string m_Name;
		std::string m_AssetsFolder = "assets";
		std::filesystem::path m_Directory;
	};
}
