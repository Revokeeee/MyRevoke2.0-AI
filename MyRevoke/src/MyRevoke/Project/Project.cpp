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
		if (data["AssetsFolder"])
			project->m_AssetsFolder = data["AssetsFolder"].as<std::string>();

		return project;
	}

	void Project::Save() const
	{
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value << m_Name;
		out << YAML::Key << "AssetsFolder" << YAML::Value << m_AssetsFolder;
		out << YAML::EndMap;

		std::ofstream file(m_Directory / (m_Name + ".mrproject"));
		file << out.c_str();
	}
}
