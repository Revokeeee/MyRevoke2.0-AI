#include "CraftLayer.h"
#include "EditorUI.h"

#include <chrono>
#include <fstream>
#include <string>

#include "MyRevoke/Utility/FileExplorer.h"
#include "MyRevoke/Utility/ExecutablePath.h"
#include "MyRevoke/Math/Math.h"
#include "MyRevoke/AudioManager/AudioRenderer.h"
#include "MyRevoke/Scripting/NativeScript.h"
#include "MyRevoke/Core/Input.h"
#include <ImGuizmo.h>
#include <imgui_internal.h>
#include "MyRevoke/ImGui/ImGuiIcons.h"
#include "MyRevoke/ImGui/ImGuiTheme.h"
#include "MyRevoke/ImGui/ImGuiLayer.h"

namespace Revoke
{

	CraftLayer::CraftLayer()
		:Layer("CraftLayer")
	{

	}
	CraftLayer::~CraftLayer()
	{
		StopPlayingScene();
		m_Scene->OnSceneClose();
	}
	void CraftLayer::OnAttach()
	{

		FrameBufferStats frameBufferStats;
		frameBufferStats.Attachments = { FramebufferTextureFormat::RGBA8,FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::DEPTH24STENCIL8 };
		frameBufferStats.Width = 1280;
		frameBufferStats.Height = 720;
		m_FrameBuffer = std::make_shared<FrameBuffers>(frameBufferStats);
		m_Scene = std::make_shared<Scene>("Main scene");

		m_EditorCamera = EditorCamera(30.0f, 1.778f, 0.1f, 1000.0f);

		SetPanelsScene(m_Scene);
		m_ToolBar.SetPlayCallbacks([this]() { OnScenePlay(); }, [this]() { OnSceneStop(); });
		m_ContentBrowserPanel.SetOpenSceneCallback([this](const std::filesystem::path& scene) { RunAfterSavePrompt([this, scene]() { OpenScene(scene); }); });

		RendererAPI::SetClearColor({ 0.2f, 0.2f, 0.2f, 1.0f });
		RendererAPI::EnableBlending();

		OpenProject(GetExecutableDirectory() / "projects" / "Example" / "Example.mrproject");
		MarkSceneSaved();
	}
	void CraftLayer::OnDetach()
	{
	}
	void CraftLayer::OnUpdate(Timestep deltaTime)
	{
		// Resize
		if (FrameBufferStats spec = m_FrameBuffer->GetSpecification();
			m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f && 
			(spec.Width != m_ViewportSize.x || spec.Height != m_ViewportSize.y))
		{
			m_FrameBuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			m_EditorCamera.SetViewportSize(m_ViewportSize.x, m_ViewportSize.y);

			m_Scene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
			if (m_RuntimeScene)
				m_RuntimeScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		}


		// Per-frame numbers for the status bar.
		Renderer2D::ResetStatistics();

		m_FrameBuffer->Bind();

		RendererAPI::Clear();

		m_FrameBuffer->ClearColorTextureAttachment(1, -1);

		ScriptEngine::OnUpdate();

		//TODO: Fix the picking in a play mode!!!
		switch (m_ToolBar.GetSceneState())
		{
		case SceneState::Editor:
		{
			if (m_ViewportFocused)
			{
				m_EditorCamera.OnUpdate(deltaTime);
			}

			m_Scene->OnEditorUpdate(deltaTime, m_EditorCamera);
			break;
		}
		case SceneState::Runtime:
		{
			m_RuntimeScene->OnRuntimeUpdate(deltaTime);
			break;
		}
		}

		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		my = viewportSize.y - my;
		int mouseX = (int)mx;
		int mouseY = (int)my;
		if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
		{
			int pixelData = m_FrameBuffer->ReadPixel(1, mouseX, mouseY);
			m_HoveredEntity = pixelData == -1 ? Entity() : Entity((entt::entity)pixelData, GetCurrentScene().get());
		}
		
		m_FrameBuffer->UnBind();

	}

	bool CraftLayer::BuildDefaultDockLayout(ImGuiID dockspaceId)
	{
		// A minimized window reports a zero size, which would make the splits below
		// divide by zero. Skip it; the caller retries next frame.
		ImVec2 viewportSize = ImGui::GetMainViewport()->WorkSize;
		if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f)
			return false;

		ImGui::DockBuilderRemoveNode(dockspaceId);
		ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceId, viewportSize);

		// Full-height right column (Scene Settings over Properties); the rest is
		// Hierarchy | toolbar-over-Viewport, with the Content Browser below them.
		ImGuiID mainId = dockspaceId;
		ImGuiID rightId = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Right, 0.19f, nullptr, &mainId);
		ImGuiID propertiesId = rightId;
		ImGuiID settingsId = ImGui::DockBuilderSplitNode(propertiesId, ImGuiDir_Up, 0.3f, nullptr, &propertiesId);

		ImGuiID centerId = mainId;
		ImGuiID bottomId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Down, 0.21f, nullptr, &centerId);
		ImGuiID leftId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Left, 0.21f, nullptr, &centerId);

		// The toolbar should stay a fixed strip, so size it in pixels rather than as a share of the window.
		// Capped at half so a very short window still leaves room for the Viewport.
		const float toolbarHeight = 36.0f * ImGuiLayer::GetUIScale();
		ImGuiDockNode* centerNode = ImGui::DockBuilderGetNode(centerId);
		RV_ASSERT(centerNode, "Default dock layout: Viewport node missing after split");
		// RV_ASSERT compiles out in Release, so also bail out rather than crash. Return
		// true so the caller doesn't retry a split that can never succeed.
		if (!centerNode)
		{
			ImGui::DockBuilderFinish(dockspaceId);
			return true;
		}
		float centerHeight = centerNode->Size.y;
		float toolbarRatio = ImMin(toolbarHeight / centerHeight, 0.5f);
		ImGuiID toolbarId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Up, toolbarRatio, nullptr, &centerId);

		ImGui::DockBuilderDockWindow("Scene Settings", settingsId);
		ImGui::DockBuilderDockWindow("Properties", propertiesId);
		ImGui::DockBuilderDockWindow("Console", bottomId);
		ImGui::DockBuilderDockWindow("Content Browser", bottomId);
		ImGui::DockBuilderDockWindow("Scene Hierarchy", leftId);
		ImGui::DockBuilderDockWindow("##toolbar", toolbarId);
		ImGui::DockBuilderDockWindow("Viewport", centerId);

		for (ImGuiID id : { toolbarId, centerId })
		{
			ImGuiDockNode* node = ImGui::DockBuilderGetNode(id);
			RV_ASSERT(node, "Default dock layout: node missing after split");
			if (!node)
				continue;
			node->SetLocalFlags(node->LocalFlags | ImGuiDockNodeFlags_HiddenTabBar);
		}

		ImGui::DockBuilderFinish(dockspaceId);
		return true;
	}

	void CraftLayer::OnImGuiDraw()
	{
		static bool dockspaceOpen = true;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// Before the dockspace window, so the space it takes at the bottom is known this frame.
		UpdateSceneStatus();
		DrawStatusBar();

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			// The work area is the viewport minus the status bar.
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// DockSpace
		ImGuiIO& io = ImGui::GetIO();
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			// Rebuild before DockSpace() submits the node this frame, never after it. If the
			// build is skipped (zero-size window), keep the request: DockSpace() is about to
			// create an empty node, so the "no node yet" check alone would never retry.
			if (m_ResetLayoutRequested || ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
				m_ResetLayoutRequested = !BuildDefaultDockLayout(dockspace_id);

			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem(RV_ICON_FILE_ADD "  New Scene", "Ctrl+N")) RunAfterSavePrompt([this]() { NewScene(); });
				if (ImGui::MenuItem(RV_ICON_FOLDER_OPEN "  Open Scene...", "Ctrl+O")) OpenScene();
				ImGui::Separator();
				if (ImGui::MenuItem(RV_ICON_SAVE "  Save Scene", "Ctrl+S")) Save();
				if (ImGui::MenuItem(RV_ICON_SAVE "  Save Scene As...", "Ctrl+Shift+S")) SaveAs();
				ImGui::Separator();
				if (ImGui::MenuItem(RV_ICON_FOLDER_ADD "  New Project...")) NewProject();
				if (ImGui::MenuItem(RV_ICON_LIBRARY "  Open Project...")) OpenProject();
				ImGui::Separator();
				if (ImGui::MenuItem(RV_ICON_POWER "  Exit", "Alt+F4")) Application::Get().RequestClose();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				const bool hasSelection = (bool)m_ObjPannel.GetSelectedEntity();
				if (ImGui::MenuItem(RV_ICON_PENCIL "  Rename", "F2", false, hasSelection)) m_ObjPannel.RenameSelectedEntity();
				if (ImGui::MenuItem(RV_ICON_COPY "  Duplicate", "Ctrl+D", false, hasSelection)) m_ObjPannel.DuplicateSelectedEntity();
				if (ImGui::MenuItem(RV_ICON_DELETE "  Delete", "Del", false, hasSelection)) m_ObjPannel.DeleteSelectedEntity();
				ImGui::Separator();
				const bool playing = m_ToolBar.GetSceneState() == SceneState::Runtime;
				if (ImGui::MenuItem(playing ? RV_ICON_STOP "  Stop" : RV_ICON_PLAY "  Play", "Ctrl+P")) m_ToolBar.TogglePlay();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem(RV_ICON_TILES "  Reset Layout")) m_ResetLayoutRequested = true;

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				if (ImGui::MenuItem(RV_ICON_HELP "  Keyboard Shortcuts", "F1")) m_ShowShortcuts = true;

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		DrawSavePrompt();
		DrawShortcutsWindow();
		HandleShortcuts();

		//-----------Settings-------------------------------------------------
	
		//--------------------------------------------------------------------
		m_ObjPannel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();
		m_ProjectSettingsPanel.OnImGuiRender();
		m_ConsolePanel.OnImGuiRender();
		//-----------ViewPort-------------------------------------------------
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport");

		auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
		auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
		auto viewportOffset = ImGui::GetWindowPos();
		m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
		m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

		m_ViewportFocused = ImGui::IsWindowFocused();
		m_ViewportHovered = ImGui::IsWindowHovered();
		Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportFocused && !m_ViewportHovered);

		ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
		m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };
		uint32_t textureID = m_FrameBuffer->GetColorAttachmentRendererID();
	
		Entity selectedEntity = m_ObjPannel.GetSelectedEntity();

		ImGui::Image((void*)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

		// Dropping a scene opens it; dropping an image adds a sprite showing it.
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(ContentBrowserPayload, ImGuiDragDropFlags_AcceptBeforeDelivery | ImGuiDragDropFlags_AcceptNoDrawDefaultRect))
			{
				std::filesystem::path path((const wchar_t*)payload->Data);
				AssetType type = GetAssetType(path);
				bool editing = m_ToolBar.GetSceneState() == SceneState::Editor;
				bool accepted = type == AssetType::Scene || (type == AssetType::Texture && editing);

				ImGui::GetWindowDrawList()->AddRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(),
					ImGui::GetColorU32(accepted ? Theme::Accent : Theme::Error), 0.0f, 0, 3.0f * ImGuiLayer::GetUIScale());

				if (accepted && payload->IsDelivery())
				{
					if (type == AssetType::Scene)
					{
						std::filesystem::path scene = GetAssetsDirectory() / path;
						RunAfterSavePrompt([this, scene]() { OpenScene(scene); });
					}
					else
					{
						Entity sprite = m_Scene->CreateEntity(path.stem().string());
						sprite.AddComponent<SpriteRendererComponent>((GetAssetsDirectory() / path).string());
						m_ObjPannel.SetSelectedEntity(sprite);
					}
				}
			}

			ImGui::EndDragDropTarget();
		}
		

		// While playing, the viewport shows the scene's own camera, so a gizmo placed with the
		// editor camera's matrices would not line up with anything. Outline the viewport instead,
		// so play mode is obvious at a glance.
		const bool playing = m_ToolBar.GetSceneState() == SceneState::Runtime;
		if (playing)
		{
			ImGui::GetWindowDrawList()->AddRect(ImVec2(m_ViewportBounds[0].x, m_ViewportBounds[0].y), ImVec2(m_ViewportBounds[1].x, m_ViewportBounds[1].y),
				ImGui::GetColorU32(Theme::Accent), 0.0f, 0, 2.0f * ImGuiLayer::GetUIScale());
		}

		//-----------Guizmo---------------------------------------------------
		ImGuizmo::OPERATION gizmoOperation = ImGuizmo::TRANSLATE;
		switch (m_ToolBar.GetGizmoTool())
		{
		case GizmoTool::Translate: gizmoOperation = ImGuizmo::TRANSLATE; break;
		case GizmoTool::Rotate:    gizmoOperation = ImGuizmo::ROTATE; break;
		case GizmoTool::Scale:     gizmoOperation = ImGuizmo::SCALE; break;
		default: break;
		}

		if (selectedEntity && !playing && m_ToolBar.GetGizmoTool() != GizmoTool::Select)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

			const glm::mat4& cameraProjection = m_EditorCamera.GetProjectionMatrix();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			auto& entityTransformsComponent = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 tranforms = entityTransformsComponent.GetTransform();

			// The toolbar toggle turns snapping on; Ctrl flips it either way.
			bool ctrlHeld = Input::IsKeyPressed(RV_KEY_LEFT_CONTROL) || Input::IsKeyPressed(RV_KEY_RIGHT_CONTROL);
			bool snap = m_ToolBar.IsSnapEnabled() != ctrlHeld;
			float snapValue = 0.5f;
			if (gizmoOperation == ImGuizmo::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

			ImGuizmo::MODE gizmoMode = m_ToolBar.IsGizmoLocal() ? ImGuizmo::LOCAL : ImGuizmo::WORLD;
			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), gizmoOperation, gizmoMode, glm::value_ptr(tranforms), nullptr, snap ? snapValues : nullptr);

			if (ImGuizmo::IsUsing() && !Input::IsKeyPressed(RV_KEY_LEFT_ALT))
			{
				glm::vec3 translation, rotation, scale;
				DecomposeTransform(tranforms, translation, rotation, scale);

				glm::vec3 deltaRotation = rotation - entityTransformsComponent.Rotation;
				entityTransformsComponent.Position = translation;
				entityTransformsComponent.Rotation += deltaRotation;
				entityTransformsComponent.Scale = scale;
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();

		m_ToolBar.OnImGuiRender();

		ImGui::End();
	}

	void CraftLayer::OnEvent(Revoke::Event& e)
	{
		// The camera zooms on scroll. Events reach here while the viewport merely has focus too,
		// so without this scrolling the Content Browser also zoomed the scene.
		if (m_ViewportHovered)
			m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<MouseButtonPressedEvent>(RV_BIND_EVENT_FUNK(CraftLayer::OnMouseBtnPressed));
	}
	bool CraftLayer::OnMouseBtnPressed(MouseButtonPressedEvent& e)
	{
		if (e.GetMouseButton() == RV_MOUSE_BUTTON_1 && !Input::IsKeyPressed(RV_KEY_LEFT_ALT))
		{
			if (m_ViewportHovered && !ImGuizmo::IsOver())
			{
				m_ObjPannel.SetSelectedEntity(m_HoveredEntity);
			}
			return true;
		}
		return false;
	}
	void CraftLayer::HandleShortcuts()
	{
		// Read from ImGui rather than from key events: with keyboard navigation on, ImGui claims
		// the keyboard whenever any panel has focus, so Ctrl+S only worked over the viewport.
		// A text field or an open popup (the save prompt, a menu) keeps the keys to itself.
		ImGuiIO& io = ImGui::GetIO();
		if (io.WantTextInput || ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
			return;

		auto pressed = [](ImGuiKey key) { return ImGui::IsKeyPressed(key, false); };
		const bool ctrl = io.KeyCtrl;

		if (ctrl && pressed(ImGuiKey_S))
		{
			if (io.KeyShift)
				SaveAs();
			else
				Save();
		}
		if (ctrl && pressed(ImGuiKey_N))
			RunAfterSavePrompt([this]() { NewScene(); });
		if (ctrl && pressed(ImGuiKey_O))
			OpenScene();
		if (ctrl && pressed(ImGuiKey_P))
			m_ToolBar.TogglePlay();
		if (pressed(ImGuiKey_F1))
			m_ShowShortcuts = !m_ShowShortcuts;

		if (!ctrl && !io.KeyAlt)
		{
			if (pressed(ImGuiKey_Q)) m_ToolBar.SetGizmoTool(GizmoTool::Select);
			if (pressed(ImGuiKey_W)) m_ToolBar.SetGizmoTool(GizmoTool::Translate);
			if (pressed(ImGuiKey_E)) m_ToolBar.SetGizmoTool(GizmoTool::Rotate);
			if (pressed(ImGuiKey_R)) m_ToolBar.SetGizmoTool(GizmoTool::Scale);
		}

		// The hierarchy handles these itself while it has focus; here they cover the viewport.
		if (m_ViewportFocused)
		{
			if (pressed(ImGuiKey_Delete))
				m_ObjPannel.DeleteSelectedEntity();
			if (ctrl && pressed(ImGuiKey_D))
				m_ObjPannel.DuplicateSelectedEntity();
		}
	}

	//------------------------------------------------------------------------
	void CraftLayer::NewProject()
	{
		std::string path = FileExplorer::SaveFile("MyRevoke Project (*.mrproject)\0*.mrproject\0");

		if (!path.empty())
		{
			std::filesystem::path chosenPath(path);
			RunAfterSavePrompt([this, chosenPath]() { SetProject(Project::Create(chosenPath.stem().string(), chosenPath.parent_path())); });
		}
	}

	void CraftLayer::OpenProject()
	{
		std::string path = FileExplorer::OpenFile("MyRevoke Project (*.mrproject)\0*.mrproject\0");

		if (!path.empty())
		{
			std::filesystem::path projectFile(path);
			RunAfterSavePrompt([this, projectFile]() { OpenProject(projectFile); });
		}
	}

	void CraftLayer::OpenProject(const std::filesystem::path& projectFilePath)
	{
		Shared<Project> project = Project::Load(projectFilePath);

		if (project)
		{
			SetProject(project);
		}
	}

	std::filesystem::path CraftLayer::GetAssetsDirectory() const
	{
		return m_Project ? m_Project->GetAssetsDirectory() : std::filesystem::path();
	}

	void CraftLayer::SetProject(Shared<Project> project)
	{
		if (!project)
		{
			RV_EDITOR_ERROR("Could not open the project");
			return;
		}

		m_Project = project;
		m_ContentBrowserPanel.SetAssetsDirectory(project->GetAssetsDirectory());
		m_ObjPannel.SetAssetsDirectory(project->GetAssetsDirectory());

		NewScene();
	}

	void CraftLayer::OnScenePlay()
	{
		m_RuntimeScene = Scene::Copy(m_Scene, GetAssetsDirectory());
		m_RuntimeScene->OnRuntimeStart();
		SetPanelsScene(m_RuntimeScene);
	}

	void CraftLayer::OnSceneStop()
	{
		m_RuntimeScene->OnRuntimeStop();
		m_RuntimeScene->OnSceneClose();
		m_RuntimeScene.reset();
		m_HoveredEntity = {};
		SetPanelsScene(m_Scene);
	}

	void CraftLayer::StopPlayingScene()
	{
		if (m_ToolBar.GetSceneState() == SceneState::Runtime)
			m_ToolBar.OnSceneStop();
	}

	void CraftLayer::SetPanelsScene(const Shared<Scene>& scene)
	{
		m_ObjPannel.SetScene(scene);
		m_ProjectSettingsPanel.SetScene(scene);
	}

	void CraftLayer::NewScene()
	{
		StopPlayingScene();
		// Close the scene being replaced (this used to close the new, empty one, so the old
		// scene's sounds were never freed).
		if (m_Scene)
			m_Scene->OnSceneClose();
		m_Scene = std::make_shared<Scene>("New scene");
		m_Scene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		SetPanelsScene(m_Scene);

		m_ScenePath.clear();
		MarkSceneSaved();
	}

	void CraftLayer::OpenScene()
	{
		std::string path = FileExplorer::OpenFile("MyRevoke Scene (*.myrevoke)\0*.myrevoke\0");

		if (!path.empty())
		{
			std::filesystem::path scene(path);
			RunAfterSavePrompt([this, scene]() { OpenScene(scene); });
		}
	}
	void CraftLayer::OpenScene(const std::filesystem::path& path)
	{
		StopPlayingScene();
		if (m_Scene)
			m_Scene->OnSceneClose();
		m_Scene = std::make_shared<Scene>();
		m_Scene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
		
		Serializer sceneSerializer(m_Scene, GetAssetsDirectory());
 		bool loaded = sceneSerializer.DeSerealize(path.string());

		SetPanelsScene(m_Scene);

		m_ScenePath = loaded ? path : std::filesystem::path();
		MarkSceneSaved();
	}

	bool CraftLayer::SaveAs()
	{
		std::string path = FileExplorer::SaveFile("MyRevoke Scene (*.myrevoke)\0*.myrevoke\0");

		if (path.empty())
			return false;

		return WriteScene(path + ".myrevoke");
	}

	bool CraftLayer::Save()
	{
		// A scene that came from, or was already saved to, a file goes back to that file.
		if (!m_ScenePath.empty())
			return WriteScene(m_ScenePath);

		if (!m_Project)
		{
			RV_EDITOR_ERROR("No project open - use Save as to pick a file");
			return false;
		}

		std::filesystem::path scenesDirectory = m_Project->GetScenesDirectory();
		std::filesystem::create_directories(scenesDirectory);

		return WriteScene(scenesDirectory / (m_Scene->GetName() + ".myrevoke"));
	}

	bool CraftLayer::WriteScene(const std::filesystem::path& file)
	{
		// Serialize once and keep that text as the saved snapshot; this is what Serializer::Serealize
		// does, minus a second pass over the scene.
		std::string text = SerializeScene();
		std::ofstream out(file);
		out << text;
		out.close();

		// A failed write must not count as saved, or the close prompt would never warn about it.
		m_LastSaveFailed = out.fail();
		if (m_LastSaveFailed)
		{
			RV_EDITOR_ERROR("Could not write the scene to {}", file.string());
			return false;
		}

		m_SavedSceneState = text;
		m_ScenePath = file;
		m_SceneModified = false;
		RV_EDITOR_INFO("Saved the scene to {}", file.string());
		return true;
	}

	std::string CraftLayer::SerializeScene()
	{
		Serializer sceneSerializer(m_Scene, GetAssetsDirectory());
		return sceneSerializer.SerializeToString();
	}

	void CraftLayer::MarkSceneSaved()
	{
		m_SavedSceneState = SerializeScene();
		m_SceneModified = false;
	}

	void CraftLayer::UpdateSceneStatus()
	{
		if (ImGui::GetTime() - m_LastModifiedCheck > 0.5)
		{
			m_SceneModified = HasUnsavedChanges();
			m_LastModifiedCheck = ImGui::GetTime();
		}

		// "Scene name* - Project - MyRevoke", like other editors, so the taskbar says what is open.
		std::string title = m_Scene->GetName() + (m_SceneModified ? "*" : "");
		if (m_Project)
			title += " - " + m_Project->GetName();
		title += " - MyRevoke";
		Application::Get().GetWindow().SetTitle(title);
	}

	void CraftLayer::DrawStatusBar()
	{
		const float scale = ImGuiLayer::GetUIScale();
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.0f * scale, 4.0f * scale));
		ImGui::PushStyleColor(ImGuiCol_MenuBarBg, Theme::BackgroundDark);
		const ImGuiWindowFlags flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_MenuBar;
		if (ImGui::BeginViewportSideBar("##StatusBar", ImGui::GetMainViewport(), ImGuiDir_Down, ImGui::GetFrameHeight(), flags))
		{
			if (ImGui::BeginMenuBar())
			{
				const ImVec4 separatorColor = Theme::TextDisabled;
				auto separator = [&]() { ImGui::TextColored(separatorColor, "|"); };

				// Mode
				if (m_ToolBar.GetSceneState() == SceneState::Runtime)
					ImGui::TextColored(Theme::Accent, RV_ICON_PLAY "  Playing");
				else
					ImGui::TextColored(Theme::TextMuted, RV_ICON_PENCIL "  Editing");
				separator();

				// Scene and whether it has unsaved changes
				ImGui::TextColored(Theme::TextMuted, RV_ICON_GLOBE);
				ImGui::TextUnformatted(m_Scene->GetName().c_str());
				UI::Tooltip(m_ScenePath.empty() ? "Not saved to a file yet" : m_ScenePath.string().c_str());
				if (m_SceneModified)
				{
					ImGui::TextColored(Theme::Warning, "Modified");
					UI::Tooltip("Unsaved changes. Ctrl+S saves.");
				}
				separator();

				// The latest log message; a click opens the console.
				if (const LogEntry* latest = m_ConsolePanel.GetLatestEntry())
				{
					ImVec4 color = latest->Level >= spdlog::level::err ? Theme::Error : latest->Level == spdlog::level::warn ? Theme::Warning : Theme::TextMuted;
					const char* icon = latest->Level >= spdlog::level::err ? RV_ICON_ERROR : latest->Level == spdlog::level::warn ? RV_ICON_WARNING : RV_ICON_INFO;
					ImGui::PushStyleColor(ImGuiCol_Text, color);
					ImGui::Text("%s  %s", icon, latest->Message.c_str());
					ImGui::PopStyleColor();
					if (ImGui::IsItemClicked())
						ImGui::SetWindowFocus("Console");
					UI::Tooltip("Click to open the Console");
				}

				// Stats, right-aligned
				Renderer2D::Stats stats = Renderer2D::GetStats();
				char statsText[128];
				std::snprintf(statsText, sizeof(statsText), "%d entities   %u quads   %u draw calls   %.0f FPS",
					(int)GetCurrentScene()->GetEntityCount(), stats.QuadCount, stats.DrawCalls, ImGui::GetIO().Framerate);
				float statsWidth = ImGui::CalcTextSize(statsText).x + ImGui::GetStyle().FramePadding.x * 2.0f;
				float statsX = ImGui::GetWindowContentRegionMax().x - statsWidth;
				if (statsX > ImGui::GetCursorPosX())
				{
					ImGui::SetCursorPosX(statsX);
					ImGui::TextColored(Theme::TextMuted, "%s", statsText);
				}

				ImGui::EndMenuBar();
			}
		}
		ImGui::End();
		ImGui::PopStyleColor();
		ImGui::PopStyleVar();
	}

	bool CraftLayer::HasUnsavedChanges()
	{
		return SerializeScene() != m_SavedSceneState;
	}

	// A scene that was never saved asks where to go (Save() would pick a path silently, and could
	// overwrite a file that happens to share the scene's name).
	bool CraftLayer::SaveBeforeClosing()
	{
		return m_ScenePath.empty() ? SaveAs() : Save();
	}

	bool CraftLayer::OnCloseRequested()
	{
		if (!HasUnsavedChanges())
			return true;

		RunAfterSavePrompt([]() { Application::Get().Close(); });
		return false;
	}

	void CraftLayer::RunAfterSavePrompt(std::function<void()> action)
	{
		if (!HasUnsavedChanges())
		{
			action();
			return;
		}

		m_PendingAction = std::move(action);
		m_ShowSavePrompt = true;
	}

	void CraftLayer::DrawSavePrompt()
	{
		if (m_ShowSavePrompt)
		{
			ImGui::OpenPopup("Save changes?");
			m_ShowSavePrompt = false;
			m_LastSaveFailed = false;
		}

		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		if (!ImGui::BeginPopupModal("Save changes?", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings))
			return;

		ImGui::PushFont(ImGuiLayer::GetFont(FontStyle::Bold));
		ImGui::TextColored(Theme::Warning, RV_ICON_WARNING);
		ImGui::SameLine();
		ImGui::Text("\"%s\" has unsaved changes.", m_Scene->GetName().c_str());
		ImGui::PopFont();
		ImGui::TextColored(Theme::TextMuted, "Save them first? Changes you don't save are lost.");
		if (m_LastSaveFailed)
			ImGui::TextColored(Theme::Error, RV_ICON_ERROR "  Could not save the scene. See the log.");
		ImGui::Spacing();
		ImGui::Spacing();

		const float buttonWidth = 110.0f * ImGuiLayer::GetUIScale();
		const float spacing = ImGui::GetStyle().ItemSpacing.x;
		// Right-align the buttons once the popup is wider than they are (it auto-sizes to fit them).
		const float buttonsOffset = ImGui::GetContentRegionAvail().x - buttonWidth * 3.0f - spacing * 2.0f;
		if (buttonsOffset > 0.0f)
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + buttonsOffset);

		bool proceed = false;
		ImGui::PushStyleColor(ImGuiCol_Button, Theme::Accent);
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, Theme::AccentHovered);
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, Theme::AccentActive);
		ImGui::PushStyleColor(ImGuiCol_Text, Theme::BackgroundDark);
		if (ImGui::Button(RV_ICON_SAVE "  Save", ImVec2(buttonWidth, 0.0f)))
		{
			// A failed or cancelled save leaves this prompt open instead of losing the changes.
			proceed = SaveBeforeClosing();
		}
		ImGui::PopStyleColor(4);
		ImGui::SameLine();
		if (ImGui::Button("Don't Save", ImVec2(buttonWidth, 0.0f)))
			proceed = true;
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(buttonWidth, 0.0f)) || ImGui::IsKeyPressed(ImGuiKey_Escape, false))
		{
			m_PendingAction = nullptr;
			ImGui::CloseCurrentPopup();
		}

		std::function<void()> action;
		if (proceed)
		{
			action = std::move(m_PendingAction);
			m_PendingAction = nullptr;
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();

		// Outside the popup, since the action may replace the scene this prompt names.
		if (action)
			action();
	}

	void CraftLayer::DrawShortcutsWindow()
	{
		if (!m_ShowShortcuts)
			return;

		static const char* shortcuts[][2] = {
			{ "Ctrl+N", "New scene" },
			{ "Ctrl+O", "Open scene" },
			{ "Ctrl+S", "Save scene" },
			{ "Ctrl+Shift+S", "Save scene as" },
			{ "Ctrl+P", "Play / stop" },
			{ "Q / W / E / R", "Select / move / rotate / scale tool" },
			{ "Hold Ctrl", "Toggle snapping while dragging the gizmo" },
			{ "Alt + left drag", "Orbit the camera" },
			{ "Alt + middle drag", "Pan the camera" },
			{ "Alt + right drag, wheel", "Zoom the camera" },
			{ "F2, double-click", "Rename the selected entity" },
			{ "Ctrl+D", "Duplicate the selected entity" },
			{ "Del", "Delete the selected entity" },
			{ "Up / Down", "Select the previous / next entity" },
			{ "Backspace", "Content Browser: up one folder" },
			{ "Ctrl + wheel", "Content Browser: thumbnail size" },
			{ "F1", "Show or hide this list" },
		};

		ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
		ImGui::SetNextWindowSize(ImVec2(480.0f * ImGuiLayer::GetUIScale(), 0.0f), ImGuiCond_Appearing);
		if (ImGui::Begin(RV_ICON_HELP "  Keyboard Shortcuts", &m_ShowShortcuts, ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings))
		{
			if (ImGui::BeginTable("##shortcuts", 2, ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp))
			{
				for (const auto& shortcut : shortcuts)
				{
					ImGui::TableNextRow();
					ImGui::TableSetColumnIndex(0);
					ImGui::PushFont(ImGuiLayer::GetFont(FontStyle::Bold));
					ImGui::TextUnformatted(shortcut[0]);
					ImGui::PopFont();
					ImGui::TableSetColumnIndex(1);
					ImGui::TextColored(Theme::TextMuted, "%s", shortcut[1]);
				}
				ImGui::EndTable();
			}
		}
		ImGui::End();
	}

}

	
