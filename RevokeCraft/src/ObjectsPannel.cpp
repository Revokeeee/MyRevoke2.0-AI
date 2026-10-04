#include "ObjectsPannel.h"

#include "imgui.h"

#include <glm/gtc/type_ptr.hpp>

namespace Revoke
{

	extern const std::filesystem::path g_AssetsDirectory;

	struct TempData
	{
		std::string CurrentScriptName = "Script";
		std::string CurrentTextureName = "Texture";
		std::string CurrentSoundName = "Sound";

		void MakeDefault()
		{
			CurrentScriptName = "Script";
			CurrentTextureName = "Texture";
			CurrentSoundName = "Sound";
		}
	};

	static TempData s_TempData;

	namespace Utils
	{
		static bool IsPayloadTexture(const wchar_t* path) {
			if (path == nullptr) {
				return false;
			}

			const wchar_t* pngExtension = L".png";
			const wchar_t* jpgExtension = L".jpg";
			size_t pathLen = wcslen(path);
			size_t pngExtLen = wcslen(pngExtension);
			size_t jpgExtLen = wcslen(jpgExtension);

			if (pathLen >= pngExtLen) {
				const wchar_t* pathPngExtension = path + pathLen - pngExtLen;
				if (wcscmp(pathPngExtension, pngExtension) == 0) {
					return true;
				}
			}

			if (pathLen >= jpgExtLen) {
				const wchar_t* pathJpgExtension = path + pathLen - jpgExtLen;
				if (wcscmp(pathJpgExtension, jpgExtension) == 0) {
					return true;
				}
			}

			return false;
		}

		static bool IsPayloadAudio(const wchar_t* path) {
			if (path == nullptr) {
				return false;
			}

			const wchar_t* pngExtension = L".wov";
			const wchar_t* jpgExtension = L".ogg";
			size_t pathLen = wcslen(path);
			size_t pngExtLen = wcslen(pngExtension);
			size_t jpgExtLen = wcslen(jpgExtension);

			if (pathLen >= pngExtLen) {
				const wchar_t* pathPngExtension = path + pathLen - pngExtLen;
				if (wcscmp(pathPngExtension, pngExtension) == 0) {
					return true;
				}
			}

			if (pathLen >= jpgExtLen) {
				const wchar_t* pathJpgExtension = path + pathLen - jpgExtLen;
				if (wcscmp(pathJpgExtension, jpgExtension) == 0) {
					return true;
				}
			}

			return false;
		}

		static bool IsPayloadScript(const wchar_t* path) {
			if (path == nullptr) {
				return false;
			}

			const wchar_t* pngExtension = L".cpp";
			const wchar_t* jpgExtension = L".h";
			size_t pathLen = wcslen(path);
			size_t pngExtLen = wcslen(pngExtension);
			size_t jpgExtLen = wcslen(jpgExtension);

			if (pathLen >= pngExtLen) {
				const wchar_t* pathPngExtension = path + pathLen - pngExtLen;
				if (wcscmp(pathPngExtension, pngExtension) == 0) {
					return true;
				}
			}

			if (pathLen >= jpgExtLen) {
				const wchar_t* pathJpgExtension = path + pathLen - jpgExtLen;
				if (wcscmp(pathJpgExtension, jpgExtension) == 0) {
					return true;
				}
			}

			return false;
		}

	}

	ObjectsPannel::ObjectsPannel(Shared<Scene> currentScene)
		:m_CurrentScene(currentScene)
	{
	}

	void ObjectsPannel::SetScene(Shared<Scene> currentScene)
	{
		m_CurrentScene = currentScene;
		m_SelectedEntity = {};
		s_TempData.MakeDefault();
	}

	void ObjectsPannel::OnImGuiRender()
	{
		ImGui::Begin("Scene Hierarchy");
		if (m_CurrentScene)
		{
			m_CurrentScene->m_Registry.each([&](auto entityID)
				{
					Entity entity{ entityID , m_CurrentScene.get() };
					SceneHierarchyWindow(entity);
				});
			if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
				m_SelectedEntity = {};

			if (ImGui::BeginPopupContextWindow(0, 1 | ImGuiPopupFlags_NoOpenOverItems))
			{
				if (ImGui::MenuItem("Create Entity"))
				{
					m_CurrentScene->CreateEntity("Empty Entity");
				}
				ImGui::EndPopup();
			}
		
		}
	
		ImGui::End();

		ImGui::Begin("Properties");
		PropertiesWindow();
		ImGui::End();
	}
	void ObjectsPannel::SceneHierarchyWindow(Entity entity)
	{
				bool entityExist = true;
				auto& entityName = entity.GetComponent<NameComponent>().Name;
				ImGuiTreeNodeFlags flags = ((m_SelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
				flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

				bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, entityName.c_str());
				if (ImGui::IsItemClicked())
				{
					m_SelectedEntity = entity;
				}

				if (ImGui::BeginPopupContextItem())
				{
					if (ImGui::MenuItem("Delete Entity"))
					{
						entityExist = false;
					}
					ImGui::EndPopup();
				}

				if (opened)
				{
					ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
					bool opened = ImGui::TreeNodeEx((void*)9817239, flags, entityName.c_str());
					if (opened)
						ImGui::TreePop();
					ImGui::TreePop();
				}
				if (!entityExist)
				{
					m_CurrentScene->RemoveEntity(entity);
					if (m_SelectedEntity == entity)
						m_SelectedEntity = {};
				}

	}
	void ObjectsPannel::PropertiesWindow()
	{
		if (m_SelectedEntity)
		{
			if (m_SelectedEntity.HasComponent<NameComponent>())
			{
				auto& name = m_SelectedEntity.GetComponent<NameComponent>().Name;

				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				strcpy_s(buffer, 256, name.c_str());
				if (ImGui::InputText("Name", buffer, sizeof(buffer)))
				{
					name = std::string(buffer);
				}
			}

		

			if (m_SelectedEntity.HasComponent<TransformComponent>())
			{
				if (ImGui::TreeNodeEx((void*)typeid(TransformComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Transform"))
				{
					auto& transformComponent = m_SelectedEntity.GetComponent<TransformComponent>();

					ImGui::DragFloat3("Position", glm::value_ptr(transformComponent.Position), 0.1f);
					ImGui::DragFloat3("Rotation", glm::value_ptr(transformComponent.Rotation), 0.1f);
					ImGui::DragFloat3("Scale", glm::value_ptr(transformComponent.Scale), 0.1f);

					ImGui::TreePop();
				}
			}
			if (m_SelectedEntity.HasComponent<SpriteRendererComponent>())
			{
				if (ImGui::TreeNodeEx((void*)typeid(SpriteRendererComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Color"))
				{
					auto& spriteComponent = m_SelectedEntity.GetComponent<SpriteRendererComponent>();
					ImGui::ColorEdit4("Color", glm::value_ptr(spriteComponent.Color));

					ImGui::Button(s_TempData.CurrentTextureName.c_str());

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_PAYLOAD"))
						{
							const wchar_t* path = (const wchar_t*)payload->Data;
							std::filesystem::path texturePath = std::filesystem::path(g_AssetsDirectory) / path;

							if (Utils::IsPayloadTexture(path))
							{
								s_TempData.CurrentTextureName = texturePath.stem().string();
								spriteComponent.Texture2D = texturePath.string();
							}
							else
							{
								RV_EDITOR_ERROR("Wrong Texture File");
							}

						}

						ImGui::EndDragDropTarget();
					}

					ImGui::TreePop();
				}
			}
			if (m_SelectedEntity.HasComponent<SoundComponent>())
			{
				if (ImGui::TreeNodeEx((void*)typeid(SoundComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Sound Settings"))
				{
					auto& soundComponent = m_SelectedEntity.GetComponent<SoundComponent>();

					ImGui::Button(s_TempData.CurrentSoundName.c_str());

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_PAYLOAD"))
						{
							const wchar_t* path = (const wchar_t*)payload->Data;
							std::filesystem::path soundPath = std::filesystem::path(g_AssetsDirectory) / path;

							if (Utils::IsPayloadAudio(path))
							{
								s_TempData.CurrentSoundName = soundPath.stem().string();
								soundComponent.SetPath(soundPath.string());
							}
							else
							{
								RV_EDITOR_ERROR("Wrong Audio File");
							}

						}
						ImGui::EndDragDropTarget();
					}
					ImGui::DragFloat("Pitch", &soundComponent.Pitch, 0.1f);
					ImGui::DragFloat("Gain", &soundComponent.Gain, 0.1f);
					ImGui::DragFloat3("Position", glm::value_ptr(soundComponent.Position), 0.1f);
					ImGui::DragFloat3("Velocity", glm::value_ptr(soundComponent.Velocity), 0.1f);

					//TODO: Maybe add on propertiesUpdate

					ImGui::Checkbox("Loop Audio", &soundComponent.LoopSound);


					ImGui::TreePop();
				}
			}
			if (m_SelectedEntity.HasComponent<RigidBodyComponent>())
			{
				if (ImGui::TreeNodeEx((void*)typeid(RigidBodyComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Rigid Body"))
				{
					auto& bodyComponent = m_SelectedEntity.GetComponent<RigidBodyComponent>();

					const char* bodyType[] = { "StaticBody",  "KinematicBody", "DynamicBody" };
					const char* currentBodyType = bodyType[(int)bodyComponent.Type];

					if (ImGui::BeginCombo("Body Type", currentBodyType))
					{
						for (int i = 0; i < 3; i++)
						{
							bool isSelected = currentBodyType == bodyType[i];
							if (ImGui::Selectable(bodyType[i], isSelected))
							{
								currentBodyType = bodyType[i];
								bodyComponent.Type = (RigidBodyComponent::BodyType)i;
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}

						ImGui::EndCombo();
					}
					ImGui::Checkbox("Fixed Rotation", &bodyComponent.IsRotating);

					ImGui::TreePop();
				}
			}

			if (m_SelectedEntity.HasComponent<BoxCollisionComponent>())
			{
				if (ImGui::TreeNodeEx((void*)typeid(BoxCollisionComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Colisions"))
				{
					auto& boxCollisionComponent = m_SelectedEntity.GetComponent<BoxCollisionComponent>();

					ImGui::DragFloat2("Size", glm::value_ptr(boxCollisionComponent.Size), 0.1f);
					ImGui::DragFloat2("Offset", glm::value_ptr(boxCollisionComponent.Offset), 0.1f);

					ImGui::DragFloat("Density", &boxCollisionComponent.Density, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("Friction", &boxCollisionComponent.Friction, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("Restriction", &boxCollisionComponent.Restriction, 0.01f, 0.0f, 1.0f);
					ImGui::DragFloat("Resitution Treshhold", &boxCollisionComponent.ResitutionTreshhold, 0.01f, 0.0f);

					ImGui::Checkbox("Sensor", &boxCollisionComponent.isSensor);

					ImGui::TreePop();
				}
			}

			if (m_SelectedEntity.HasComponent<NativeScriptComponent>())
			{
				if (ImGui::TreeNodeEx((void*)typeid(NativeScriptComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Scripts"))
				{
					auto& nativeScriptComponent = m_SelectedEntity.GetComponent<NativeScriptComponent>();
					
					ImGui::Button(s_TempData.CurrentScriptName.c_str());

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_PAYLOAD"))
						{
							const wchar_t* path = (const wchar_t*)payload->Data;
							std::filesystem::path scriptName = path;

							if (Utils::IsPayloadScript(path))
							{
								s_TempData.CurrentScriptName = scriptName.stem().string();
								nativeScriptComponent.scriptClassName = s_TempData.CurrentScriptName;
								nativeScriptComponent.Instance = nullptr;
							}

							else
							{
								RV_EDITOR_ERROR("Wrong Script File");
							}
						}

						ImGui::EndDragDropTarget();
					}

					ImGui::SameLine();
					if (ImGui::Button("X", ImVec2(20, 20))) {
						s_TempData.CurrentScriptName = "Script";
						nativeScriptComponent.scriptClassName.clear();
						nativeScriptComponent.Instance = nullptr;
					}
				
					ImGui::TreePop();
				}
			}

			if (m_SelectedEntity.HasComponent<CameraComponent>())
			{
				if (ImGui::TreeNodeEx((void*)typeid(CameraComponent).hash_code(), ImGuiTreeNodeFlags_DefaultOpen, "Camera"))
				{
					auto& cameraComponent = m_SelectedEntity.GetComponent<CameraComponent>();

					ImGui::Checkbox("Primary", &cameraComponent.isMain);


					const char* projectionType[] = { "3D Camera", "2D Camera" };
					const char* currentProjectionType = projectionType[(int)cameraComponent.Camera.GetProjectionType()];

					if (ImGui::BeginCombo("Camera Type", currentProjectionType))
					{
						for (int i = 0; i < 2; i++)
						{
							bool isSelected = currentProjectionType == projectionType[i];
							if (ImGui::Selectable(projectionType[i], isSelected))
							{
								currentProjectionType = projectionType[i];
								cameraComponent.Camera.SetProjectionType((SceneCamera::Projection)i);
							}
							if (isSelected)
								ImGui::SetItemDefaultFocus();
						}

						ImGui::EndCombo();
					}

					if (cameraComponent.Camera.GetProjectionType() == SceneCamera::Projection::Perspective)
					{
						float FOV = glm::degrees(cameraComponent.Camera.GetPerspFOV());
						if (ImGui::DragFloat("FOV", &FOV))
							cameraComponent.Camera.SetPerspFOV(glm::radians(FOV));

						float nearClip = cameraComponent.Camera.GetPerspNearClip();
						if (ImGui::DragFloat("Near Clip", &nearClip))
							cameraComponent.Camera.SetPerspNearClip(nearClip);

						float farClip = cameraComponent.Camera.GetPerspFarClip();
						if (ImGui::DragFloat("Far Clip", &farClip))
							cameraComponent.Camera.SetPerspFarClipl(farClip);
					}
					if (cameraComponent.Camera.GetProjectionType() == SceneCamera::Projection::Orthographic)
					{
						float size = cameraComponent.Camera.GetOrthoSize();
						if (ImGui::DragFloat("Size", &size))
							cameraComponent.Camera.SetOrthoSize(size);

						float nearClip = cameraComponent.Camera.GetOrthoNearClip();
						if (ImGui::DragFloat("Near Clip", &nearClip))
							cameraComponent.Camera.SetOrthoNearClip(nearClip);

						float farClip = cameraComponent.Camera.GetOrthoFarClip();
						if (ImGui::DragFloat("Far Clip", &farClip))
							cameraComponent.Camera.SetOrthoFarClipl(farClip);

					}
					ImGui::TreePop();
				}
			}


			if (ImGui::Button("Add Component"))
				ImGui::OpenPopup("Add Component");

			//TODO add all components

			if (ImGui::BeginPopup("Add Component"))
			{
				if (ImGui::MenuItem("Sprite Renderer"))
				{
					if (!m_SelectedEntity.HasComponent<SpriteRendererComponent>())
					{
						m_SelectedEntity.AddComponent<SpriteRendererComponent>();
					}
					else
					{
						RV_EDITOR_WARN("Component Already Exists");
					}
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("Rigid Body"))
				{
					if (!m_SelectedEntity.HasComponent<RigidBodyComponent>())
					{
					m_SelectedEntity.AddComponent<RigidBodyComponent>();
					}
					else
					{
						RV_EDITOR_WARN("Component Already Exists");
					}
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("Box Colidor"))
				{
					if (!m_SelectedEntity.HasComponent<BoxCollisionComponent>())
					{
					m_SelectedEntity.AddComponent<BoxCollisionComponent>();
					}
					else
					{
						RV_EDITOR_WARN("Component Already Exists");
					}
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("Sound Component"))
				{
					if (!m_SelectedEntity.HasComponent<SoundComponent>())
					{
					m_SelectedEntity.AddComponent<SoundComponent>();
					}
					else
					{
						RV_EDITOR_WARN("Component Already Exists");
					}
					ImGui::CloseCurrentPopup();
				}

				if (ImGui::MenuItem("Camera"))
				{
					if (!m_SelectedEntity.HasComponent<CameraComponent>())
					{
					m_SelectedEntity.AddComponent<CameraComponent>();
					}
					else
					{
						RV_EDITOR_WARN("Component Already Exists");
					}
					ImGui::CloseCurrentPopup();
				}
				if (ImGui::MenuItem("Native Script Component"))
				{
					if (!m_SelectedEntity.HasComponent<NativeScriptComponent>())
					{
					m_SelectedEntity.AddComponent<NativeScriptComponent>();
					}
					else
					{
						RV_EDITOR_WARN("Component Already Exists");
					}
					ImGui::CloseCurrentPopup();
				}
				ImGui::EndPopup();
			}
		}
	}
	void ObjectsPannel::SetSelectedEntity(Entity entity)
	{
		m_SelectedEntity = entity;
	}
}
