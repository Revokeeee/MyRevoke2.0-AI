#include "rvpch.h"
#include "Project.h"

#include <fstream>
#include <yaml-cpp/yaml.h>

namespace Revoke
{
	Shared<Project> Project::Create(const std::string& name, const std::filesystem::path& parentDirectory)
	{
		Shared<Project> project = std::make_shared<Project>();
		project->m_Name = name;
		project->m_GameName = name;
		project->m_Directory = parentDirectory / name;

		std::filesystem::create_directories(project->GetScenesDirectory());
		std::filesystem::create_directories(project->GetScriptsDirectory());
		project->Save();

		return project;
	}

	Shared<Project> Project::Load(const std::filesystem::path& projectFilePath)
	{
		std::ifstream file(projectFilePath);
		if (!file)
		{
			RV_ENGINE_ERROR("Could not open project file: {0}", projectFilePath.string());
			return nullptr;
		}

		std::stringstream fileContent;
		fileContent << file.rdbuf();

		YAML::Node data = YAML::Load(fileContent.str());
		if (!data["Project"])
		{
			RV_ENGINE_ERROR("Not a project file: {0}", projectFilePath.string());
			return nullptr;
		}

		Shared<Project> project = std::make_shared<Project>();
		project->m_Name = data["Project"].as<std::string>();
		project->m_Directory = projectFilePath.parent_path();
		// Projects written before the game settings existed name no game, so they are called
		// after the project itself.
		project->m_GameName = data["GameName"] ? data["GameName"].as<std::string>() : project->m_Name;
		if (data["AssetsFolder"])
			project->m_AssetsFolder = data["AssetsFolder"].as<std::string>();
		if (data["StartupScene"])
			project->m_StartupScene = data["StartupScene"].as<std::string>();
		if (data["WindowWidth"])
			project->m_WindowWidth = data["WindowWidth"].as<unsigned int>();
		if (data["WindowHeight"])
			project->m_WindowHeight = data["WindowHeight"].as<unsigned int>();
		if (data["Fullscreen"])
			project->m_Fullscreen = data["Fullscreen"].as<bool>();

		return project;
	}

	std::filesystem::path Project::GetStartupScenePath() const
	{
		if (m_StartupScene.empty())
			return {};

		return GetAssetsDirectory() / m_StartupScene;
	}

	void Project::Save() const
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value << m_Name;
		out << YAML::Key << "GameName" << YAML::Value << m_GameName;
		out << YAML::Key << "AssetsFolder" << YAML::Value << m_AssetsFolder;
		out << YAML::Key << "StartupScene" << YAML::Value << m_StartupScene;
		out << YAML::Key << "WindowWidth" << YAML::Value << m_WindowWidth;
		out << YAML::Key << "WindowHeight" << YAML::Value << m_WindowHeight;
		out << YAML::Key << "Fullscreen" << YAML::Value << m_Fullscreen;
		out << YAML::EndMap;

		std::ofstream file(m_Directory / (m_Name + ".mrproject"));
		file << out.c_str();
	}
}
