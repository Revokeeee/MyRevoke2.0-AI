#include "CraftLayer.h"

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

namespace Revoke
{
	namespace Utils
	{
		static bool IsPayloadScene(const wchar_t* path) {
			if (path == nullptr) {
				return false;
			}

			const wchar_t* extension = L".myrevoke";
			size_t pathLen = wcslen(path);
			size_t extLen = wcslen(extension);

			if (pathLen < extLen) {
				return false;
			}

			const wchar_t* pathExtension = path + pathLen - extLen;
			return wcscmp(pathExtension, extension) == 0;
		}
	}

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

		m_GizmoType = new int(-1);

		m_ToolBar.SetGuizmo(m_GizmoType);

		RendererAPI::SetClearColor({ 0.2f, 0.2f, 0.2f, 1.0f });
		RendererAPI::EnableBlending();

		OpenProject(GetExecutableDirectory() / "projects" / "Example" / "Example.mrproject");
		MarkSceneSaved();
	}
	void CraftLayer::OnDetach()
	{
		delete m_GizmoType;
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
		// divide by zero. Skip it; the empty-dockspace check retries next frame.
		ImVec2 viewportSize = ImGui::GetMainViewport()->Size;
		if (viewportSize.x <= 0.0f || viewportSize.y <= 0.0f)
			return false;

		ImGui::DockBuilderRemoveNode(dockspaceId);
		ImGui::DockBuilderAddNode(dockspaceId, ImGuiDockNodeFlags_DockSpace);
		ImGui::DockBuilderSetNodeSize(dockspaceId, viewportSize);

		// Full-height right column (Scene Settings over Properties); the rest is
		// Hierarchy | toolbar-over-Viewport, with the Content Browser below them.
		ImGuiID mainId = dockspaceId;
		ImGuiID rightId = ImGui::DockBuilderSplitNode(mainId, ImGuiDir_Right, 0.13f, nullptr, &mainId);
		ImGuiID propertiesId = rightId;
		ImGuiID settingsId = ImGui::DockBuilderSplitNode(propertiesId, ImGuiDir_Up, 0.22f, nullptr, &propertiesId);

		ImGuiID centerId = mainId;
		ImGuiID bottomId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Down, 0.21f, nullptr, &centerId);
		ImGuiID leftId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Left, 0.21f, nullptr, &centerId);

		// The toolbar should stay a fixed strip, so size it in pixels rather than as a share of the window.
		const float toolbarHeight = 32.0f;
		float centerHeight = ImGui::DockBuilderGetNode(centerId)->Size.y;
		ImGuiID toolbarId = ImGui::DockBuilderSplitNode(centerId, ImGuiDir_Up, toolbarHeight / centerHeight, nullptr, &centerId);

		ImGui::DockBuilderDockWindow("Scene Settings", settingsId);
		ImGui::DockBuilderDockWindow("Properties", propertiesId);
		ImGui::DockBuilderDockWindow("Content Browser", bottomId);
		ImGui::DockBuilderDockWindow("Scene Hierarchy", leftId);
		ImGui::DockBuilderDockWindow("##toolbar", toolbarId);
		ImGui::DockBuilderDockWindow("Viewport", centerId);

		for (ImGuiID id : { toolbarId, centerId })
		{
			ImGuiDockNode* node = ImGui::DockBuilderGetNode(id);
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

		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
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
			// Rebuild before DockSpace() submits the node this frame, never after it.
			if (m_ResetLayoutRequested || ImGui::DockBuilderGetNode(dockspace_id) == nullptr)
			{
				if (BuildDefaultDockLayout(dockspace_id))
					m_ResetLayoutRequested = false;
			}

			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Exit")) Application::Get().RequestClose();
				if (ImGui::MenuItem("New Project")) NewProject();
				if (ImGui::MenuItem("Open Project")) OpenProject();
				if (ImGui::MenuItem("New Scene", "Ctrl+N")) NewScene();
				if (ImGui::MenuItem("Open", "Ctrl+O")) OpenScene();
				if (ImGui::MenuItem("Save", "Ctrl+S")) Save();
				if (ImGui::MenuItem("Save as", "Ctrl+Shift+S")) SaveAs();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Reset Layout")) m_ResetLayoutRequested = true;

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		DrawSavePrompt();

		//-----------Settings-------------------------------------------------
	
		//--------------------------------------------------------------------
		m_ObjPannel.OnImGuiRender();
		m_ContentBrowserPanel.OnImGuiRender();
		m_ProjectSettingsPanel.OnImGuiRender();
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

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_PAYLOAD"))
			{
				const wchar_t* path = (const wchar_t*)payload->Data;

				if (Utils::IsPayloadScene(path))
				{
					OpenScene(GetAssetsDirectory() / path);
				}
				else
				{
					RV_EDITOR_ERROR("Wrong Scene File");
				}
			}

			ImGui::EndDragDropTarget();
		}
		

		//-----------Guizmo---------------------------------------------------
		if (selectedEntity && *m_GizmoType != -1)
		{
			ImGuizmo::SetOrthographic(false);
			ImGuizmo::SetDrawlist();

			ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);


			auto mainCameraEntt = m_EditorCamera;
			const glm::mat4& cameraProjection = m_EditorCamera.GetProjectionMatrix();
			glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

			auto& entityTransformsComponent = selectedEntity.GetComponent<TransformComponent>();
			glm::mat4 tranforms = entityTransformsComponent.GetTransform();
			
			bool snap = Input::IsKeyPressed(RV_KEY_LEFT_CONTROL);
			float snapValue = 0.5f;
			if (*m_GizmoType == ImGuizmo::OPERATION::ROTATE)
				snapValue = 45.0f;

			float snapValues[3] = { snapValue, snapValue, snapValue };

		
			ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection), (ImGuizmo::OPERATION)*m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(tranforms), nullptr, snap ? snapValues : nullptr);

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


		m_ToolBar.OnImGuiRender();

		ImGui::End();
		ImGui::PopStyleVar();

		ImGui::End();
	}

	void CraftLayer::OnEvent(Revoke::Event& e)
	{
		m_EditorCamera.OnEvent(e);

		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<KeyPressedEvent>(RV_BIND_EVENT_FUNK(CraftLayer::OnKeyPressed));
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
	bool CraftLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		switch (e.GetKeyCode())
		{
		case RV_KEY_S:
		{
			if (Input::IsKeyPressed(RV_KEY_LEFT_CONTROL) && Input::IsKeyPressed(RV_KEY_LEFT_SHIFT))
			{
				SaveAs();
			}
			else if (Input::IsKeyPressed(RV_KEY_LEFT_CONTROL))
			{
				Save();
			}
			break;
		}
		case RV_KEY_N:
		{
			if (Input::IsKeyPressed(RV_KEY_LEFT_CONTROL))
			{
				NewScene();
			}
			break;
		}
		case RV_KEY_O:
		{
			if (Input::IsKeyPressed(RV_KEY_LEFT_CONTROL))
			{
				OpenScene();
			}
			break;
		}
		case RV_KEY_Q:
		{
			*m_GizmoType = -1;
			break;
		}
		case RV_KEY_W:
		{
			*m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		}
		case RV_KEY_E:
		{
			*m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		}
		case RV_KEY_R:
		{
			*m_GizmoType = ImGuizmo::OPERATION::SCALE;
			break;
		}
		}
		return false;
	}

	//------------------------------------------------------------------------
	void CraftLayer::NewProject()
	{
		std::string path = FileExplorer::SaveFile("MyRevoke Project (*.mrproject)\0*.mrproject\0");

		if (!path.empty())
		{
			std::filesystem::path chosenPath(path);
			SetProject(Project::Create(chosenPath.stem().string(), chosenPath.parent_path()));
		}
	}

	void CraftLayer::OpenProject()
	{
		std::string path = FileExplorer::OpenFile("MyRevoke Project (*.mrproject)\0*.mrproject\0");

		if (!path.empty())
		{
			OpenProject(std::filesystem::path(path));
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
		m_Scene = std::make_shared<Scene>("New scene");
		m_Scene->OnSceneClose();
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
			OpenScene(path);
		}
	}
	void CraftLayer::OpenScene(const std::filesystem::path& path)
	{
		StopPlayingScene();
		m_Scene = std::make_shared<Scene>();
		m_Scene->OnSceneClose();
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

		m_ShowSavePrompt = true;
		return false;
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
		if (!ImGui::BeginPopupModal("Save changes?", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
			return;

		ImGui::Text("Save changes to \"%s\" before closing?", m_Scene->GetName().c_str());
		if (m_LastSaveFailed)
			ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "Could not save the scene. See the log.");
		ImGui::Spacing();

		if (ImGui::Button("Save"))
		{
			// A failed or cancelled save leaves this prompt open instead of losing the changes.
			if (SaveBeforeClosing())
				Application::Get().Close();
		}
		ImGui::SameLine();
		if (ImGui::Button("Don't Save"))
		{
			Application::Get().Close();
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		ImGui::EndPopup();
	}

}

	
