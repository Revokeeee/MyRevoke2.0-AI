#pragma once
#include <filesystem>

#include "MyRevoke/Renderer/Texture.h"
namespace Revoke
{

	class ContentBrowser
	{
	public:
		ContentBrowser();

		void SetAssetsDirectory(const std::filesystem::path& assetsDirectory);

		void OnImGuiRender();

	private:
		std::filesystem::path m_AssetsDir;
		std::filesystem::path m_CurrendDir;
		Shared<Texture> m_FolderIcon;
		Shared<Texture> m_FileIcon;
	};

}

