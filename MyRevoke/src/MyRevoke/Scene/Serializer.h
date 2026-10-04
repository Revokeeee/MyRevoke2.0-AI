#pragma once

#include <filesystem>

#include "Scene.h"


namespace Revoke {

	class Serializer
	{
	public:
		// Asset paths are stored relative to the assets folder so a project stays valid when it is
		// moved. An empty folder leaves them as they are.
		Serializer(const Shared<Scene> scene, const std::filesystem::path& assetsDirectory = {});

		// File-based API, unchanged for existing callers.
		void Serealize(const std::string& path);
		bool DeSerealize(const std::string& path);

		// Pure conversion API: no filesystem access, usable from tests.
		std::string SerializeToString();
		bool DeserializeFromString(const std::string& yamlSource);

	private:
		Shared<Scene> m_Scene;
		std::filesystem::path m_AssetsDirectory;
	};

}
