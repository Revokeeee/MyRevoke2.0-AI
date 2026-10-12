#include "rvpch.h"
#include "FileExplorer.h"

#include <commdlg.h>
#include <shobjidl.h>
#include <filesystem>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "MyRevoke/Core/Application.h"

namespace Revoke {

	std::string FileExplorer::OpenFile(const char* filter)
    {
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetCoreWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
    }

	std::string FileExplorer::SaveFile(const char* filter)
    {
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetCoreWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}
		return std::string();
    }

	// Shows the folder dialog and returns the picked folder. The caller has already initialised COM.
	static std::string ShowFolderDialog(const char* title)
	{
		IFileDialog* dialog = nullptr;
		if (FAILED(CoCreateInstance(CLSID_FileOpenDialog, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&dialog))))
		{
			RV_ENGINE_ERROR("Could not create the folder dialog");
			return std::string();
		}

		std::string narrowTitle(title);
		std::wstring wideTitle(narrowTitle.begin(), narrowTitle.end());
		dialog->SetTitle(wideTitle.c_str());
		dialog->SetOptions(FOS_PICKFOLDERS | FOS_FORCEFILESYSTEM | FOS_PATHMUSTEXIST);

		std::string folder;
		if (SUCCEEDED(dialog->Show(glfwGetWin32Window((GLFWwindow*)Application::Get().GetWindow().GetCoreWindow()))))
		{
			IShellItem* item = nullptr;
			if (SUCCEEDED(dialog->GetResult(&item)))
			{
				PWSTR widePath = nullptr;
				if (SUCCEEDED(item->GetDisplayName(SIGDN_FILESYSPATH, &widePath)))
				{
					folder = std::filesystem::path(widePath).string();
					CoTaskMemFree(widePath);
				}
				item->Release();
			}
		}

		dialog->Release();
		return folder;
	}

	std::string FileExplorer::SelectFolder(const char* title)
	{
		// COM is already up by the time the editor draws a menu, but the dialog must not depend on
		// that. S_FALSE means it was already initialised and still has to be balanced; the failure
		// RPC_E_CHANGED_MODE must not be.
		HRESULT initialized = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

		std::string folder = ShowFolderDialog(title);

		if (SUCCEEDED(initialized))
			CoUninitialize();

		return folder;
	}
}
