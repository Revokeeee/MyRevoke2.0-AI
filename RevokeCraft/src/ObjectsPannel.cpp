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
			return std::filesystem::path(path).filename().string();
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

		// While playing, the physics world still holds the body and its fixture.
		template<>
		void ReleaseComponent<RigidBodyComponent>(Scene& scene, Entity entity)
		{
			scene.DestroyPhysicsBody(entity);
		}

		template<>
		void ReleaseComponent<BoxCollisionComponent>(Scene& scene, Entity entity)
		{
			scene.DestroyCollider(entity);
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
		m_RenamingEntity = {};
		m_PendingDelete = {};
	}

	void ObjectsPannel::OnImGuiRender()
	{
		HierarchyWindow();

		ImGui::Begin("Properties");
		PropertiesWindow();
		ImGui::End();
	}

	void ObjectsPannel::HierarchyWindow()
	{
		ImGui::Begin("Scene Hierarchy");
		if (!m_CurrentScene)
		{
			UI::EmptyState(RV_ICON_LAYERS, "No scene open.");
			ImGui::End();
			return;
		}

		// Create button and search
		if (UI::IconButton(RV_ICON_ADD, "Create entity"))
			ImGui::OpenPopup("CreateEntity");
		if (ImGui::BeginPopup("CreateEntity"))
		{
			DrawCreateEntityMenu();
			ImGui::EndPopup();
		}
		ImGui::SameLine(0.0f, ImGui::GetStyle().ItemInnerSpacing.x);
		UI::SearchBox("HierarchySearch", m_HierarchyFilter, "Search entities");

		std::vector<Entity> entities = m_CurrentScene->GetEntities();
		std::vector<Entity> visible;
		visible.reserve(entities.size());
		for (Entity entity : entities)
		{
			if (UI::MatchesFilter(entity.GetComponent<NameComponent>().Name, m_HierarchyFilter))
				visible.push_back(entity);
		}

		// The list scrolls; the entity count below it stays put.
		const float footerHeight = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
		ImGui::BeginChild("##entities", ImVec2(0.0f, -footerHeight));

		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 1.0f));
		for (Entity entity : visible)
			DrawEntityRow(entity);
		ImGui::PopStyleVar();

		if (entities.empty())
			UI::EmptyState(RV_ICON_CUBE, "No entities yet. Right-click here or press + to create one.");
		else if (visible.empty())
			UI::EmptyState(RV_ICON_SEARCH, "No entity matches the search.");

		// A click on empty space clears the selection; a right click there offers to create.
		if (ImGui::IsMouseClicked(ImGuiMouseButton_Left) && ImGui::IsWindowHovered() && !ImGui::IsAnyItemHovered())
			m_SelectedEntity = {};
		if (ImGui::BeginPopupContextWindow("HierarchyContext", ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			DrawCreateEntityMenu();
			ImGui::EndPopup();
		}

		ImGui::EndChild();

		ImGui::PushStyleColor(ImGuiCol_Text, Theme::TextDisabled);
		if (m_HierarchyFilter.empty())
			ImGui::Text("%d %s", (int)entities.size(), entities.size() == 1 ? "entity" : "entities");
		else
			ImGui::Text("%d of %d entities", (int)visible.size(), (int)entities.size());
		ImGui::PopStyleColor();

		// Keyboard, while the hierarchy has focus and no text field is taking the keys.
		if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) && !ImGui::GetIO().WantTextInput)
		{
			if (ImGui::IsKeyPressed(ImGuiKey_Delete, false))
				DeleteSelectedEntity();
			if (ImGui::IsKeyPressed(ImGuiKey_F2, false))
				RenameSelectedEntity();
			if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(ImGuiKey_D, false))
				DuplicateSelectedEntity();

			// Up / Down walk the visible list.
			int step = ImGui::IsKeyPressed(ImGuiKey_DownArrow) ? 1 : ImGui::IsKeyPressed(ImGuiKey_UpArrow) ? -1 : 0;
			if (step != 0 && !visible.empty())
			{
				auto it = std::find(visible.begin(), visible.end(), m_SelectedEntity);
				int index = it == visible.end() ? (step > 0 ? -1 : (int)visible.size()) : (int)(it - visible.begin());
				index = std::clamp(index + step, 0, (int)visible.size() - 1);
				m_SelectedEntity = visible[index];
			}
		}

		// Deleting inside the loop would pull the entity out from under the rows still being drawn.
		if (m_PendingDelete)
		{
			if (m_SelectedEntity == m_PendingDelete)
				m_SelectedEntity = {};
			if (m_RenamingEntity == m_PendingDelete)
				m_RenamingEntity = {};
			m_CurrentScene->RemoveEntity(m_PendingDelete);
			m_PendingDelete = {};
		}

		ImGui::End();
	}

	void ObjectsPannel::DrawEntityRow(Entity entity)
	{
		auto& name = entity.GetComponent<NameComponent>().Name;
		ImGui::PushID((int)(uint32_t)entity);

		if (m_RenamingEntity == entity)
		{
			if (m_FocusRename)
			{
				ImGui::SetKeyboardFocusHere();
				m_FocusRename = false;
			}
			ImGui::SetNextItemWidth(-FLT_MIN);
			ImGui::InputText("##rename", m_RenameBuffer, sizeof(m_RenameBuffer), ImGuiInputTextFlags_AutoSelectAll);

			// Enter or clicking away keeps the new name; Escape keeps the old one.
			if (ImGui::IsItemDeactivated())
			{
				if (!ImGui::IsKeyPressed(ImGuiKey_Escape) && m_RenameBuffer[0] != '\0')
					name = m_RenameBuffer;
				m_RenamingEntity = {};
			}
			ImGui::PopID();
			return;
		}

		const char* icon = RV_ICON_CUBE;
		if (entity.HasComponent<CameraComponent>())
			icon = RV_ICON_CAMERA;
		else if (entity.HasComponent<SpriteRendererComponent>())
			icon = RV_ICON_IMAGE;
		else if (entity.HasComponent<SoundComponent>())
			icon = RV_ICON_VOLUME;

		const bool selected = m_SelectedEntity == entity;
		if (selected)
		{
			ImGui::PushStyleColor(ImGuiCol_Header, Theme::Selection);
			ImGui::PushStyleColor(ImGuiCol_HeaderHovered, Theme::SelectionHovered);
			ImGui::PushStyleColor(ImGuiCol_HeaderActive, Theme::SelectionHovered);
		}
		if (ImGui::Selectable("##row", selected, ImGuiSelectableFlags_AllowDoubleClick, ImVec2(0.0f, ImGui::GetFrameHeight())))
		{
			m_SelectedEntity = entity;
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
				RenameSelectedEntity();
		}
		if (selected)
			ImGui::PopStyleColor(3);

		// Icon and name drawn over the row, so the icon can be muted.
		ImVec2 rowMin = ImGui::GetItemRectMin();
		float textY = rowMin.y + (ImGui::GetItemRectSize().y - ImGui::GetFontSize()) * 0.5f;
		float textX = rowMin.x + ImGui::GetStyle().FramePadding.x;
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		drawList->AddText(ImVec2(textX, textY), ImGui::GetColorU32(selected ? Theme::AccentHovered : Theme::TextMuted), icon);
		textX += ImGui::CalcTextSize(icon).x + ImGui::GetStyle().ItemInnerSpacing.x * 1.5f;
		drawList->AddText(ImVec2(textX, textY), ImGui::GetColorU32(ImGuiCol_Text), name.c_str());

		if (ImGui::BeginPopupContextItem("EntityContext"))
		{
			m_SelectedEntity = entity;
			if (ImGui::MenuItem(RV_ICON_PENCIL "  Rename", "F2"))
				RenameSelectedEntity();
			if (ImGui::MenuItem(RV_ICON_COPY "  Duplicate", "Ctrl+D"))
				DuplicateSelectedEntity();
			ImGui::Separator();
			if (ImGui::MenuItem(RV_ICON_DELETE "  Delete", "Del"))
				m_PendingDelete = entity;
			ImGui::EndPopup();
		}

		ImGui::PopID();
	}

	void ObjectsPannel::DrawCreateEntityMenu()
	{
		if (ImGui::MenuItem(RV_ICON_CUBE "  Empty Entity"))
			m_SelectedEntity = m_CurrentScene->CreateEntity("Entity");

		if (ImGui::MenuItem(RV_ICON_IMAGE "  Sprite"))
		{
			Entity entity = m_CurrentScene->CreateEntity("Sprite");
			entity.AddComponent<SpriteRendererComponent>();
			m_SelectedEntity = entity;
		}

		if (ImGui::MenuItem(RV_ICON_CAMERA "  Camera"))
		{
			// Only the first camera in a scene starts out as the one play mode renders through.
			bool hasPrimary = (bool)m_CurrentScene->GetMainCamera();
			Entity entity = m_CurrentScene->CreateEntity("Camera");
			entity.AddComponent<CameraComponent>().isMain = !hasPrimary;
			m_SelectedEntity = entity;
		}
	}

	void ObjectsPannel::DeleteSelectedEntity()
	{
		if (m_SelectedEntity)
			m_PendingDelete = m_SelectedEntity;
	}

	void ObjectsPannel::DuplicateSelectedEntity()
	{
		if (m_SelectedEntity && m_CurrentScene)
			m_SelectedEntity = m_CurrentScene->DuplicateEntity(m_SelectedEntity);
	}

	void ObjectsPannel::RenameSelectedEntity()
	{
		if (!m_SelectedEntity)
			return;

		m_RenamingEntity = m_SelectedEntity;
		std::snprintf(m_RenameBuffer, sizeof(m_RenameBuffer), "%s", m_SelectedEntity.GetComponent<NameComponent>().Name.c_str());
		m_FocusRename = true;
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
					script.scriptClassName = dropped.stem().string();
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
		if (m_RenamingEntity != entity)
			m_RenamingEntity = {};
	}
}
