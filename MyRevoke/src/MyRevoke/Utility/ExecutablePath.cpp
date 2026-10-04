#include "rvpch.h"
#include "ExecutablePath.h"

namespace Revoke
{
	static std::filesystem::path ReadExecutableDirectory()
	{
		std::wstring exePath(MAX_PATH, L'\0');
		DWORD length = GetModuleFileNameW(nullptr, exePath.data(), (DWORD)exePath.size());

		// GetModuleFileNameW silently truncates instead of failing, and says so by filling the buffer
		while (length == exePath.size())
		{
			exePath.resize(exePath.size() * 2);
			length = GetModuleFileNameW(nullptr, exePath.data(), (DWORD)exePath.size());
		}
		exePath.resize(length);

		return std::filesystem::path(exePath).parent_path();
	}

	const std::filesystem::path& GetExecutableDirectory()
	{
		static const std::filesystem::path directory = ReadExecutableDirectory();
		return directory;
	}
}
