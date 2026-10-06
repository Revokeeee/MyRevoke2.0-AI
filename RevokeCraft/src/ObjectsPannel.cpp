#include "ObjectsPannel.h"

#include "AssetType.h"
#include "EditorUI.h"

#include "MyRevoke/ImGui/ImGuiLayer.h"
#include "MyRevoke/ImGui/ImGuiTheme.h"
#include "MyRevoke/ImGui/ImGuiIcons.h"
#include "MyRevoke/Renderer/Renderer2D.h"

#include "imgui.h"

#include <glm/gtc/type_ptr.hpp>

namespace Revoke
{
	namespace
	{
		std::string FileNameOf(const std::string& path)
		{
			if (path.empty())
				return {};
			auto name = std::filesystem::path(path).filename().u8string();
			return std::string(name.begin(), name.end());
		}

		bool IsTexture(const std::filesystem::path& path) { return GetAssetType(path) == AssetType::Texture; }
		bool IsAudio(const std::filesystem::path& path) { return GetAssetType(path) == AssetType::Audio; }
		bool IsScript(const std::filesystem::path& path) { return GetAssetType(path) == AssetType::Script; }

		// Frees what a component holds outside the registry before it is removed or reset.
		template<typename T>
		void ReleaseComponent(Scene&, Entity) {}

		template<>
		void ReleaseComponent<SoundComponent>(Scene&, Entity entity)
		{
			entity.GetComponent<SoundComponent>().ShutDown();
		}

		template<>
		void ReleaseComponent<NativeScriptComponent>(Scene& scene, Entity entity)
		{
			scene.DestroyScriptInstance(entity);
		}

		// One collapsible section per component: a bold header with a settings menu, then the
		// component's fields in a label | value table.
		template<typename T, typename DrawFunction>
		void DrawComponent(const char* name, const char* icon, Entity entity, Scene& scene, bool removable, DrawFunction draw)
		{
			if (!entity.HasComponent<T>())
				return;

			ImGui::PushID(typeid(T).name());

			const float scale = ImGuiLayer::GetUIScale();
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(6.0f * scale, 5.0f * scale));
			ImGui::PushStyleColor(ImGuiCol_Header, Theme::Surface);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Theme::SurfaceHovered);
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, Theme::SurfaceActive);
			ImGui::PushFont(ImGuiLayer::GetFont(FontStyle::Bold));

			const ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth
				| ImGuiTreeNodeFlags_AllowOverlap | ImGuiTreeNodeFlags_FramePadding | ImGuiTreeNodeFlags_NoTreePushOnOpen;
			bool open = ImGui::TreeNodeEx("##header", flags, "%s  %s", icon, name);
			const float headerHeight = ImGui::GetItemRectSize().y;
			const ImVec2 belowHeader = ImGui::GetCursorPos();

			ImGui::PopFont();
			ImGui::PopStyleColor(3);
			ImGui::PopStyleVar();

			// Settings button over the right end of the header. Put the cursor back afterwards so
			// the button doesn't push the fields down.
			ImGui::SameLine(ImGui::GetWindowContentRegionMax().x - headerHeight);
			if (UI::IconButton(RV_ICON_MORE, "Component settings", ImVec2(headerHeight, headerHeight)))
				ImGui::OpenPopup("ComponentSettings");
			ImGui::SetCursorPos(belowHeader);

			bool reset = false;
			bool remove = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem(RV_ICON_REFRESH "  Reset"))
					reset = true;
				if (ImGui::MenuItem(RV_ICON_DELETE "  Remove Component", nullptr, false, removable))
					remove = true;
				ImGui::EndPopup();
			}

			if (open)
			{
				if (UI::BeginProperties("##properties"))
				{
					draw(entity.GetComponent<T>());
					UI::EndProperties();
				}
			}
			ImGui::Spacing();

			// After drawing, so the fields above never see a component that is gone.
			if (reset || remove)
			{
				ReleaseComponent<T>(scene, entity);
				entity.RemoveComponent<T>();
				if (reset)
					entity.AddComponent<T>();
			}

			ImGui::PopID();
		}

		template<typename T>
		void AddComponentMenuItem(Entity entity, const char* label)
		{
			bool alreadyAdded = entity.HasComponent<T>();
			if (ImGui::MenuItem(label, alreadyAdded ? RV_ICON_CHECK : nullptr, false, !alreadyAdded))
				entity.AddComponent<T>();
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
		if (!m_SelectedEntity)
		{
			UI::EmptyState(RV_ICON_SLIDERS, "Select an entity to edit its components.");
			return;
		}

		Scene& scene = *m_CurrentScene;
		Entity entity = m_SelectedEntity;

		// Name and id
		if (entity.HasComponent<NameComponent>())
		{
			auto& name = entity.GetComponent<NameComponent>().Name;

			char buffer[256];
			std::snprintf(buffer, sizeof(buffer), "%s", name.c_str());
			ImGui::PushFont(ImGuiLayer::GetFont(FontStyle::Bold));
			ImGui::SetNextItemWidth(-FLT_MIN);
			if (ImGui::InputTextWithHint("##Name", "Entity name", buffer, sizeof(buffer)))
				name = buffer;
			ImGui::PopFont();
		}
		if (entity.HasComponent<IdComponent>())
		{
			ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextDisabled);
			ImGui::Text("ID  %llu", (unsigned long long)(uint64_t)entity.GetComponent<IdComponent>().ID);
			ImGui::PopStyleColor();
		}
		ImGui::Spacing();

		DrawComponent<TransformComponent>("Transform", RV_ICON_MOVE, entity, scene, false, [](TransformComponent& transform)
			{
				UI::PropertyVec3("Position", transform.Position);

				// Stored in radians; people think in degrees.
				glm::vec3 rotation = glm::degrees(transform.Rotation);
				if (UI::PropertyVec3("Rotation", rotation, 0.0f, 1.0f))
					transform.Rotation = glm::radians(rotation);

				UI::PropertyVec3("Scale", transform.Scale, 1.0f, 0.05f);
			});

		DrawComponent<SpriteRendererComponent>("Sprite Renderer", RV_ICON_IMAGE, entity, scene, true, [this](SpriteRendererComponent& sprite)
			{
				UI::PropertyColor(sprite.Texture2D.empty() ? "Color" : "Tint", sprite.Color);

				uint32_t preview = sprite.Texture2D.empty() ? 0 : Renderer2D::GetTexture(sprite.Texture2D)->GetID();
				std::filesystem::path dropped;
				switch (UI::PropertyAsset("Texture", RV_ICON_IMAGE, FileNameOf(sprite.Texture2D), sprite.Texture2D, IsTexture, dropped, preview))
				{
				case UI::AssetFieldResult::Dropped: sprite.Texture2D = (m_AssetsDirectory / dropped).string(); break;
				case UI::AssetFieldResult::Cleared: sprite.Texture2D.clear(); break;
				default: break;
				}
			});

		DrawComponent<CameraComponent>("Camera", RV_ICON_CAMERA, entity, scene, true, [](CameraComponent& cameraComponent)
			{
				auto& camera = cameraComponent.Camera;

				UI::PropertyBool("Primary", cameraComponent.isMain, "Play mode renders through the primary camera.");

				const char* projections[] = { "Perspective", "Orthographic" };
				int projection = (int)camera.GetProjectionType();
				if (UI::PropertyCombo("Projection", projection, projections, 2))
					camera.SetProjectionType((SceneCamera::Projection)projection);

				if (camera.GetProjectionType() == SceneCamera::Projection::Perspective)
				{
					float fov = glm::degrees(camera.GetPerspFOV());
					if (UI::PropertyFloat("Field of View", fov, 0.1f, 1.0f, 179.0f, "%.1f°"))
						camera.SetPerspFOV(glm::radians(fov));

					float nearClip = camera.GetPerspNearClip();
					if (UI::PropertyFloat("Near Clip", nearClip, 0.01f))
						camera.SetPerspNearClip(nearClip);

					float farClip = camera.GetPerspFarClip();
					if (UI::PropertyFloat("Far Clip", farClip, 1.0f))
						camera.SetPerspFarClipl(farClip);
				}
				else
				{
					float size = camera.GetOrthoSize();
					if (UI::PropertyFloat("Size", size, 0.1f, 0.01f, 10000.0f, "%.2f", "Height of the view in world units."))
						camera.SetOrthoSize(size);

					float nearClip = camera.GetOrthoNearClip();
					if (UI::PropertyFloat("Near Clip", nearClip, 0.01f))
						camera.SetOrthoNearClip(nearClip);

					float farClip = camera.GetOrthoFarClip();
					if (UI::PropertyFloat("Far Clip", farClip, 0.01f))
						camera.SetOrthoFarClipl(farClip);
				}

				UI::PropertyBool("Fixed Aspect Ratio", cameraComponent.FixedAspectRatio, "Keep the aspect ratio when the viewport is resized.");
			});

		DrawComponent<RigidBodyComponent>("Rigid Body", RV_ICON_BOLT, entity, scene, true, [](RigidBodyComponent& body)
			{
				const char* bodyTypes[] = { "Static", "Kinematic", "Dynamic" };
				int type = (int)body.Type;
				if (UI::PropertyCombo("Body Type", type, bodyTypes, 3))
					body.Type = (RigidBodyComponent::BodyType)type;

				UI::PropertyBool("Fixed Rotation", body.IsRotating, "Stop collisions from rotating the body.");
			});

		DrawComponent<BoxCollisionComponent>("Box Collider", RV_ICON_FRAME, entity, scene, true, [](BoxCollisionComponent& collider)
			{
				UI::PropertyVec2("Size", collider.Size, 0.5f, 0.05f);
				UI::PropertyVec2("Offset", collider.Offset, 0.0f, 0.05f);
				UI::PropertyFloat("Density", collider.Density, 0.01f, 0.0f, 100.0f);
				UI::PropertyFloat("Friction", collider.Friction, 0.01f, 0.0f, 1.0f);
				UI::PropertyFloat("Restitution", collider.Restriction, 0.01f, 0.0f, 1.0f, "%.3f", "Bounciness: 0 never bounces, 1 bounces back at full speed.");
				UI::PropertyFloat("Restitution Threshold", collider.ResitutionTreshhold, 0.01f, 0.0f, 100.0f, "%.2f", "Hits slower than this (m/s) don't bounce.");
				UI::PropertyBool("Sensor", collider.isSensor, "Detects overlaps without colliding.");
			});

		DrawComponent<NativeScriptComponent>("Native Script", RV_ICON_CODE, entity, scene, true, [&scene, entity](NativeScriptComponent& script)
			{
				std::filesystem::path dropped;
				switch (UI::PropertyAsset("Script", RV_ICON_CODE, script.scriptClassName, "Drop a .cpp or .h file. Its file name is the script's class name.", IsScript, dropped))
				{
				case UI::AssetFieldResult::Dropped:
				{
					scene.DestroyScriptInstance(entity);
					auto className = dropped.stem().u8string();
					script.scriptClassName = std::string(className.begin(), className.end());
					break;
				}
				case UI::AssetFieldResult::Cleared:
					scene.DestroyScriptInstance(entity);
					script.scriptClassName.clear();
					break;
				default: break;
				}
			});

		DrawComponent<SoundComponent>("Sound", RV_ICON_VOLUME, entity, scene, true, [this](SoundComponent& sound)
			{
				std::filesystem::path dropped;
				switch (UI::PropertyAsset("Audio Clip", RV_ICON_MUSIC, FileNameOf(sound.AudioPath), sound.AudioPath, IsAudio, dropped))
				{
				case UI::AssetFieldResult::Dropped: sound.SetPath((m_AssetsDirectory / dropped).string()); break;
				case UI::AssetFieldResult::Cleared: sound.ShutDown(); sound.AudioPath.clear(); break;
				default: break;
				}

				UI::PropertyFloat("Pitch", sound.Pitch, 0.01f, 0.0f, 10.0f);
				UI::PropertyFloat("Gain", sound.Gain, 0.01f, 0.0f, 10.0f);
				UI::PropertyVec3("Position", sound.Position);
				UI::PropertyVec3("Velocity", sound.Velocity);
				UI::PropertyBool("Loop", sound.LoopSound);
			});

		// Add Component
		ImGui::Spacing();
		const float buttonWidth = ImGui::GetContentRegionAvail().x;
		if (ImGui::Button(RV_ICON_ADD "  Add Component", ImVec2(buttonWidth, 0.0f)))
			ImGui::OpenPopup("AddComponent");

		// Drop the menu down from the button rather than wherever the mouse happens to be.
		ImGui::SetNextWindowPos(ImVec2(ImGui::GetItemRectMin().x, ImGui::GetItemRectMax().y + ImGui::GetStyle().ItemSpacing.y * 0.5f), ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(buttonWidth, 0.0f));
		if (ImGui::BeginPopup("AddComponent"))
		{
			AddComponentMenuItem<SpriteRendererComponent>(entity, RV_ICON_IMAGE "  Sprite Renderer");
			AddComponentMenuItem<CameraComponent>(entity, RV_ICON_CAMERA "  Camera");
			AddComponentMenuItem<RigidBodyComponent>(entity, RV_ICON_BOLT "  Rigid Body");
			AddComponentMenuItem<BoxCollisionComponent>(entity, RV_ICON_FRAME "  Box Collider");
			AddComponentMenuItem<NativeScriptComponent>(entity, RV_ICON_CODE "  Native Script");
			AddComponentMenuItem<SoundComponent>(entity, RV_ICON_VOLUME "  Sound");
			ImGui::EndPopup();
		}
	}
	void ObjectsPannel::SetSelectedEntity(Entity entity)
	{
		m_SelectedEntity = entity;
	}
}
