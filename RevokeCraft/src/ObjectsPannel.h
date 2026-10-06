#pragma once
#include "MyRevoke.h"
#include "MyRevoke/Core/Core.h"

#include <filesystem>
#include <string>

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

		void PropertiesWindow();

		void SetSelectedEntity(Entity entity);
		Entity GetSelectedEntity() const { return m_SelectedEntity; }

		// Act on the selected entity; the hierarchy and the viewport share these shortcuts.
		void DeleteSelectedEntity();
		void DuplicateSelectedEntity();
		void RenameSelectedEntity();
	private:
		void HierarchyWindow();
		void DrawEntityRow(Entity entity);
		void DrawCreateEntityMenu();
	private:
		Shared<Scene> m_CurrentScene;
		std::filesystem::path m_AssetsDirectory;
		Entity m_SelectedEntity;

		std::string m_HierarchyFilter;
		// The entity whose name is being edited in place, if any.
		Entity m_RenamingEntity;
		char m_RenameBuffer[256] = {};
		bool m_FocusRename = false;
		Entity m_PendingDelete;
	};
}
