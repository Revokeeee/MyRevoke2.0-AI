#pragma once
#include <string>

namespace Revoke
{
	class FileExplorer
	{
	public:
		static std::string OpenFile(const char* filter);
		static std::string SaveFile(const char* filter);
		// An existing folder, picked in the Windows folder dialog. Empty when the user cancels.
		static std::string SelectFolder(const char* title);
	};
}

