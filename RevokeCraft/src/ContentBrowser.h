#pragma once
#include <filesystem>
#include <functional>
#include <string>
#include <vector>

#include "AssetType.h"

namespace Revoke
{

	class ContentBrowser
	{
	public:
		ContentBrowser() = default;

		void SetAssetsDirectory(const std::filesystem::path& assetsDirectory);
		// Called with a scene file's full path when it is double-clicked.
		void SetOpenSceneCallback(std::function<void(const std::filesystem::path&)> callback) { m_OnOpenScene = std::move(callback); }

		void OnImGuiRender();

	private:
		struct Item
		{
			std::filesystem::path Path;
			std::string Name;
			AssetType Type = AssetType::Unknown;
		};

		void NavigateTo(const std::filesystem::path& directory);
		void Refresh();
		void DrawNavigationBar();
		void DrawItem(const Item& item, int index, float tileWidth);
		void OpenItem(const Item& item);

		std::filesystem::path m_AssetsDir;
		std::filesystem::path m_CurrentDir;

		// The listing is cached and refreshed on navigation, on a new search, or once a second,
		// rather than walking the disk every frame.
		std::vector<Item> m_Items;
		double m_LastRefreshTime = -1.0;
		bool m_NeedsRefresh = true;

		std::string m_Filter;
		std::filesystem::path m_SelectedPath;
		float m_ThumbnailSize = 80.0f;

		std::function<void(const std::filesystem::path&)> m_OnOpenScene;
	};

}
