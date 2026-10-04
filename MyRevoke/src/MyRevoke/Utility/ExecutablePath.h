#pragma once
#include <filesystem>

namespace Revoke
{
	// Resources ship in fixed folders next to the executable, so every path to them has to be built
	// from this directory - the working directory is wherever the user happened to launch us from.
	const std::filesystem::path& GetExecutableDirectory();
}
