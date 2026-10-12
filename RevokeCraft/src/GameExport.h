#pragma once

#include <filesystem>

#include "MyRevoke/Project/Project.h"

namespace Revoke
{
	// Writes a runnable game into outputDirectory: a copy of RevokePlayer.exe named after the game,
	// the engine runtime files from next to the editor, and the project with its assets. Overwrites
	// an earlier build in the same folder. Returns false and logs what went wrong.
	bool ExportGame(const Project& project, const std::filesystem::path& outputDirectory);
}
