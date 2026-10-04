#pragma once

#include "Scene.h"


namespace Revoke {

	class Serializer
	{
	public:
		Serializer(const Shared<Scene> scene);

		// File-based API, unchanged for existing callers.
		void Serealize(const std::string& path);
		bool DeSerealize(const std::string& path);

		// Pure conversion API: no filesystem access, usable from tests.
		std::string SerializeToString();
		bool DeserializeFromString(const std::string& yamlSource);

	private:
		Shared<Scene> m_Scene;
	};

}
