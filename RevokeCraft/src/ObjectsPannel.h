#pragma once
#include "MyRevoke.h"
#include "MyRevoke/Core/Core.h"

#include <filesystem>

namespace Revoke
{
	class ObjectsPannel
	{
	public:
		ObjectsPannel() = default;
		ObjectsPannel(Shared<Scene> currentScene);

		void SetScene(Shared<Scene> currentScene);
		void SetAssetsDirectory(const std::filesystem::path& assetsDirectory) { m_AssetsDirectory = assetsDirectory; }
		void OnImGuiRender();

		void SceneHierarchyWindow(Entity entity);
		void PropertiesWindow();

		void SetSelectedEntity(Entity entity);
		Entity GetSelectedEntity() const { return m_SelectedEntity; }
	private:
		Shared<Scene> m_CurrentScene;
		std::filesystem::path m_AssetsDirectory;
		Entity m_SelectedEntity;
	};
}