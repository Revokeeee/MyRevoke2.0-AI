#pragma once

#include "MyRevoke/ImGui/ImGuiIcons.h"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>

namespace Revoke
{
	// What a file in the assets folder is, judging by its extension.
	enum class AssetType
	{
		Unknown = 0,
		Folder,
		Scene,
		Texture,
		Audio,
		Script
	};

	inline AssetType GetAssetType(const std::filesystem::path& path)
	{
		std::string extension = path.extension().string();
		std::transform(extension.begin(), extension.end(), extension.begin(), [](unsigned char c) { return (char)std::tolower(c); });

		if (extension == ".myrevoke")
			return AssetType::Scene;
		// Everything stb_image can decode.
		if (extension == ".png" || extension == ".jpg" || extension == ".jpeg" || extension == ".jfif" || extension == ".bmp" || extension == ".tga")
			return AssetType::Texture;
		// Formats libsndfile reads.
		if (extension == ".wav" || extension == ".ogg" || extension == ".flac")
			return AssetType::Audio;
		if (extension == ".cpp" || extension == ".h")
			return AssetType::Script;
		return AssetType::Unknown;
	}

	inline const char* GetAssetIcon(AssetType type)
	{
		switch (type)
		{
		case AssetType::Folder:  return RV_ICON_FOLDER;
		case AssetType::Scene:   return RV_ICON_GLOBE;
		case AssetType::Texture: return RV_ICON_IMAGE;
		case AssetType::Audio:   return RV_ICON_MUSIC;
		case AssetType::Script:  return RV_ICON_CODE;
		default:                 return RV_ICON_FILE;
		}
	}

	// The drag-and-drop payload type the Content Browser sends. Its data is a null-terminated
	// wchar_t path relative to the project's assets folder.
	constexpr const char* ContentBrowserPayload = "CONTENT_BROWSER_PAYLOAD";
}
