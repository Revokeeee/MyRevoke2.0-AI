#include "rvpch.h" 
#include "ImGuiLayer.h"
#include "ImGuiTheme.h"
#include "ImGuiIcons.h"

#include <imgui.h>
#include <backends/imgui_impl_opengl3.h>
#include <backends/imgui_impl_glfw.h>

#include "MyRevoke/Core/Application.h"

//TEmp
#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <ImGuizmo.h>

#include <array>
#include <cmath>
#include <cstdlib>
#include <filesystem>

namespace
{
	std::array<ImFont*, (size_t)Revoke::FontStyle::Count> s_Fonts{};
	float s_UIScale = 1.0f;

	// The engine only builds for Windows, so the editor uses the fonts every Windows install has
	// rather than shipping its own. Missing files fall back to ImGui's built-in font.
	std::filesystem::path FindSystemFont(std::initializer_list<const char*> fileNames)
	{
		const char* windowsDirectory = std::getenv("WINDIR");
		std::filesystem::path fontsDirectory = std::filesystem::path(windowsDirectory ? windowsDirectory : "C:\\Windows") / "Fonts";

		for (const char* fileName : fileNames)
		{
			std::error_code error;
			std::filesystem::path candidate = fontsDirectory / fileName;
			if (std::filesystem::exists(candidate, error))
				return candidate;
		}
		return {};
	}

	ImFont* AddFont(const std::filesystem::path& textFont, float size, const std::filesystem::path& iconFont, float iconSize)
	{
		ImGuiIO& io = ImGui::GetIO();

		ImFont* font = nullptr;
		if (!textFont.empty())
		{
			ImFontConfig config;
			config.OversampleH = 3;
			// Latin plus Cyrillic, so asset and entity names in either show up.
			font = io.Fonts->AddFontFromFileTTF(textFont.string().c_str(), size, &config, io.Fonts->GetGlyphRangesCyrillic());
		}
		if (!font)
		{
			ImFontConfig config;
			config.SizePixels = size;
			font = io.Fonts->AddFontDefault(&config);
		}

		if (!iconFont.empty())
		{
			// Only the glyphs the editor uses. Built once; the atlas reads it when it builds.
			static ImVector<ImWchar> iconRanges;
			if (iconRanges.empty())
			{
				ImFontGlyphRangesBuilder builder;
				builder.AddText(RV_ICONS_ALL);
				builder.BuildRanges(&iconRanges);
			}

			ImFontConfig config;
			config.MergeMode = true;
			config.PixelSnapH = true;
			config.GlyphMinAdvanceX = iconSize;
			// The icon font sits higher than Segoe UI's baseline; nudge it to line up with the text.
			config.GlyphOffset.y = iconSize * 0.18f;
			io.Fonts->AddFontFromFileTTF(iconFont.string().c_str(), iconSize, &config, iconRanges.Data);
		}

		return font;
	}

	void LoadFonts(float scale)
	{
		std::filesystem::path regular = FindSystemFont({ "segoeui.ttf" });
		std::filesystem::path semibold = FindSystemFont({ "seguisb.ttf", "segoeuib.ttf", "segoeui.ttf" });
		std::filesystem::path mono = FindSystemFont({ "CascadiaMono.ttf", "consola.ttf" });
		// Segoe Fluent Icons ships with Windows 11, Segoe MDL2 Assets with Windows 10.
		std::filesystem::path icons = FindSystemFont({ "SegoeIcons.ttf", "segmdl2.ttf" });

		const float size = std::round(16.0f * scale);
		const float largeSize = std::round(20.0f * scale);

		// The first font added is ImGui's default.
		s_Fonts[(size_t)Revoke::FontStyle::Regular] = AddFont(regular, size, icons, size);
		s_Fonts[(size_t)Revoke::FontStyle::Bold] = AddFont(semibold, size, icons, size);
		s_Fonts[(size_t)Revoke::FontStyle::Large] = AddFont(semibold, largeSize, icons, largeSize);
		s_Fonts[(size_t)Revoke::FontStyle::Mono] = AddFont(mono, std::round(15.0f * scale), icons, size);
	}
}

ImFont* Revoke::ImGuiLayer::GetFont(FontStyle style)
{
	ImFont* font = s_Fonts[(size_t)style];
	return font ? font : ImGui::GetFont();
}

float Revoke::ImGuiLayer::GetUIScale()
{
	return s_UIScale;
}

Revoke::ImGuiLayer::ImGuiLayer()
	: Layer("ImGuiLayer")
{
}

Revoke::ImGuiLayer::~ImGuiLayer()
{
 
}

void Revoke::ImGuiLayer::OnAttach()
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

    // Dragging inside a panel (e.g. orbiting the viewport) must not move the panel.
    io.ConfigWindowsMoveFromTitleBarOnly = true;
    // A plain click on a drag field types a value; dragging still scrubs it.
    io.ConfigDragClickToInputText = true;

    Application& app = Application::Get();
    GLFWwindow* window = static_cast<GLFWwindow*>(app.GetWindow().GetCoreWindow());

    // Match the monitor's scaling (125%, 150%, ...) so text stays readable on high-DPI screens.
    float xScale = 1.0f, yScale = 1.0f;
    glfwGetWindowContentScale(window, &xScale, &yScale);
    float contentScale = xScale > yScale ? xScale : yScale;
    s_UIScale = contentScale > 1.0f ? contentScale : 1.0f;

    LoadFonts(s_UIScale);
    Theme::Apply(s_UIScale);

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 410");
}


void Revoke::ImGuiLayer::OnDetach()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Revoke::ImGuiLayer::OnImGuiDraw()
{
	
}

void Revoke::ImGuiLayer::OnEvent(Event& e)
{
	if (m_BlockEvents)
	{
		ImGuiIO& io = ImGui::GetIO();
		e.Handled |= e.IsCategory(EventCategoryMouse) & io.WantCaptureMouse;
		e.Handled |= e.IsCategory(EventCategoryKeyboard) & io.WantCaptureKeyboard;
	}
}


void Revoke::ImGuiLayer::Begin()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
	ImGuizmo::BeginFrame();
}

void Revoke::ImGuiLayer::End()
{
	ImGuiIO& io = ImGui::GetIO();

	Application &app = Application::Get();
	io.DisplaySize = ImVec2((float)app.GetWindow().GetWidth(), (float)app.GetWindow().GetHeight());
	
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}
