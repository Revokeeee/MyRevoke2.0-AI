#include "GameExport.h"

#include "MyRevoke/Core/Log.h"
#include "MyRevoke/Utility/ExecutablePath.h"

namespace Revoke
{
	// Import libraries, debug info and the patched copies NativeScript.cpp writes beside the script
	// DLL at runtime sit in the same folders as the files the game loads. scripts/stage-editor.ps1
	// prunes the same set out of the staged editor.
	static bool IsBuildLeftover(const std::filesystem::path& file)
	{
		std::string extension = file.extension().string();
		if (extension == ".lib" || extension == ".exp" || extension == ".pdb" || extension == ".ilk")
			return true;

		std::string name = file.filename().string();
		return name == "imgui.ini" || name.ends_with("_.dll") || name.ends_with("_dll") || name.ends_with("_pdb");
	}

	// True when directory is parent itself or lies under it. Both paths have to be absolute.
	static bool IsInside(const std::filesystem::path& directory, const std::filesystem::path& parent)
	{
		std::filesystem::path relative = directory.lexically_relative(parent);
		return !relative.empty() && *relative.begin() != "..";
	}

	// The game name is free text in the project settings, so it can hold characters Windows does not
	// allow in a file name.
	static std::string GameExeName(const std::string& gameName)
	{
		std::string name;
		for (char character : gameName)
		{
			if (std::string("\\/:*?\"<>|").find(character) == std::string::npos)
				name += character;
		}

		if (name.empty())
			name = "Game";

		return name + ".exe";
	}

	static bool CopyInto(const std::filesystem::path& file, const std::filesystem::path& destination)
	{
		std::error_code error;
		std::filesystem::copy_file(file, destination, std::filesystem::copy_options::overwrite_existing, error);
		if (error)
		{
			RV_EDITOR_ERROR("Build Game: could not copy {0}: {1}", file.string(), error.message());
			return false;
		}

		return true;
	}

	static bool CopyDirectoryInto(const std::filesystem::path& from, const std::filesystem::path& to)
	{
		if (!std::filesystem::exists(from))
		{
			RV_EDITOR_ERROR("Build Game: {0} is missing", from.string());
			return false;
		}

		std::error_code walkError;
		try
		{
			for (const auto& entry : std::filesystem::recursive_directory_iterator(from, walkError))
			{
				if (entry.is_directory() || IsBuildLeftover(entry.path()))
					continue;

				std::filesystem::path destination = to / entry.path().lexically_relative(from);

				std::error_code error;
				std::filesystem::create_directories(destination.parent_path(), error);
				if (error)
				{
					RV_EDITOR_ERROR("Build Game: could not create {0}: {1}", destination.parent_path().string(), error.message());
					return false;
				}

				if (!CopyInto(entry.path(), destination))
					return false;
			}
		}
		// The error_code passed to recursive_directory_iterator only covers opening "from" itself;
		// a later error entering a subdirectory (permission denied, a broken symlink) throws instead.
		catch (const std::filesystem::filesystem_error& exception)
		{
			RV_EDITOR_ERROR("Build Game: could not read {0}: {1}", from.string(), exception.what());
			return false;
		}

		if (walkError)
		{
			RV_EDITOR_ERROR("Build Game: could not read {0}: {1}", from.string(), walkError.message());
			return false;
		}

		return true;
	}

	bool ExportGame(const Project& project, const std::filesystem::path& outputDirectory)
	{
		const std::filesystem::path& editorDirectory = GetExecutableDirectory();

		std::error_code error;
		std::filesystem::path output = std::filesystem::weakly_canonical(outputDirectory, error);
		if (error)
			output = outputDirectory;

		// Either way the copy would read the folder it is writing into, and never finish.
		if (IsInside(output, project.GetDirectory()) || IsInside(output, editorDirectory))
		{
			RV_EDITOR_ERROR("Build Game: pick a folder outside the project and outside the editor");
			return false;
		}

		std::filesystem::create_directories(output, error);
		if (error)
		{
			RV_EDITOR_ERROR("Build Game: could not create {0}: {1}", output.string(), error.message());
			return false;
		}

		// The player reads resourses/ and its project file from next to its own exe, the same layout
		// the editor builds into, so an exported game is that folder with the editor swapped for the
		// renamed player. mono/lib is left out: C# scripting does not run yet (#36).
		std::filesystem::path player = editorDirectory / "RevokePlayer.exe";
		if (!std::filesystem::exists(player))
		{
			RV_EDITOR_ERROR("Build Game: RevokePlayer.exe is not next to the editor");
			return false;
		}

		std::string exeName = GameExeName(project.GetGameName());

		if (!CopyInto(player, output / exeName))
			return false;
		if (!CopyInto(editorDirectory / "OpenAL32.dll", output / "OpenAL32.dll"))
			return false;
		if (!CopyInto(project.GetFilePath(), output / project.GetFilePath().filename()))
			return false;
		if (!CopyDirectoryInto(editorDirectory / "resourses", output / "resourses"))
			return false;
		if (!CopyDirectoryInto(project.GetAssetsDirectory(), output / project.GetAssetsDirectory().filename()))
			return false;

		RV_EDITOR_INFO("Build Game: built {0} in {1}", exeName, output.string());
		return true;
	}
}
