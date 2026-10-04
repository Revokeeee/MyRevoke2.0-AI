#include "rvpch.h"
#include "Serializer.h"

#include "Components.h"
#include <yaml-cpp/yaml.h>
#include "Entity.h"
namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

}
namespace Revoke
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

    static void SerializeEntity(YAML::Emitter& out, Entity entity)
	{
		out << YAML::BeginMap; // Entity
		out << YAML::Key << "Entity" << YAML::Value << entity.GetComponent<IdComponent>().ID; 

		if (entity.HasComponent<NameComponent>())
		{
			out << YAML::Key << "TagComponent";
			out << YAML::BeginMap; // TagComponent

			auto& tag = entity.GetComponent<NameComponent>().Name;
			out << YAML::Key << "Tag" << YAML::Value << tag;

			out << YAML::EndMap; // TagComponent
		}

		if (entity.HasComponent<TransformComponent>())
		{
			out << YAML::Key << "TransformComponent";
			out << YAML::BeginMap; // TransformComponent
		
			auto& tc = entity.GetComponent<TransformComponent>();
			out << YAML::Key << "Position" << YAML::Value << tc.Position;
			out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
			out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
		
			out << YAML::EndMap; // TransformComponent
		}

		if (entity.HasComponent<CameraComponent>())
		{
			out << YAML::Key << "CameraComponent";
			out << YAML::BeginMap; // CameraComponent

			auto& cameraComponent = entity.GetComponent<CameraComponent>();
			auto& camera = cameraComponent.Camera;

			out << YAML::Key << "Camera" << YAML::Value;
			out << YAML::BeginMap; // Camera
			out << YAML::Key << "ProjectionType" << YAML::Value << (int)camera.GetProjectionType();
			out << YAML::Key << "PerspectiveFOV" << YAML::Value << camera.GetPerspFOV();
			out << YAML::Key << "PerspectiveNear" << YAML::Value << camera.GetPerspNearClip();
			out << YAML::Key << "PerspectiveFar" << YAML::Value << camera.GetPerspFarClip();
			out << YAML::Key << "OrthographicSize" << YAML::Value << camera.GetOrthoSize();
			out << YAML::Key << "OrthographicNear" << YAML::Value << camera.GetOrthoNearClip();
			out << YAML::Key << "OrthographicFar" << YAML::Value << camera.GetOrthoFarClip();
			out << YAML::EndMap; // Camera

			out << YAML::Key << "Primary" << YAML::Value << cameraComponent.isMain;
			out << YAML::Key << "FixedAspectRatio" << YAML::Value << cameraComponent.FixedAspectRatio;

			out << YAML::EndMap; // CameraComponent
		}

		if (entity.HasComponent<SpriteRendererComponent>())
		{
			out << YAML::Key << "SpriteRendererComponent";
			out << YAML::BeginMap; 
		
			auto& spriteRendererComponent = entity.GetComponent<SpriteRendererComponent>();
			out << YAML::Key << "Color" << YAML::Value << spriteRendererComponent.Color;
			out << YAML::Key << "Texture2D" << YAML::Value << spriteRendererComponent.Texture2D;
		
			out << YAML::EndMap; 
		}


		if (entity.HasComponent<RigidBodyComponent>())
		{
			out << YAML::Key << "RigidBodyComponent";
			out << YAML::BeginMap; 

			auto& rbComponent = entity.GetComponent<RigidBodyComponent>();
			out << YAML::Key << "BodyType" << YAML::Value << (int)rbComponent.Type;
			out << YAML::Key << "IsRotating" << YAML::Value << rbComponent.IsRotating;

			out << YAML::EndMap;
		}

		if (entity.HasComponent<BoxCollisionComponent>())
		{
			out << YAML::Key << "BoxCollisionComponent";
			out << YAML::BeginMap; 

			auto& boxColisionComponent = entity.GetComponent<BoxCollisionComponent>();
			out << YAML::Key << "Offset" << YAML::Value << boxColisionComponent.Offset;
			out << YAML::Key << "Size" << YAML::Value << boxColisionComponent.Size;

			out << YAML::Key << "Density" << YAML::Value << boxColisionComponent.Density;
			out << YAML::Key << "Friction" << YAML::Value << boxColisionComponent.Friction;
			out << YAML::Key << "Restriction" << YAML::Value << boxColisionComponent.Restriction;
			out << YAML::Key << "ResitutionTreshhold" << YAML::Value << boxColisionComponent.ResitutionTreshhold;
			out << YAML::Key << "isSensor" << YAML::Value << boxColisionComponent.isSensor;

			out << YAML::EndMap; 
		}
		if (entity.HasComponent<SoundComponent>())
		{
			out << YAML::Key << "SoundComponent";
			out << YAML::BeginMap;

			auto& soundComponent = entity.GetComponent<SoundComponent>();
			out << YAML::Key << "AudioPath" << YAML::Value << soundComponent.AudioPath;

			out << YAML::Key << "Pitch" << YAML::Value << soundComponent.Pitch;
			out << YAML::Key << "Gain" << YAML::Value << soundComponent.Gain;
			out << YAML::Key << "Position" << YAML::Value << soundComponent.Position;
			out << YAML::Key << "Velocity" << YAML::Value << soundComponent.Velocity;
			out << YAML::Key << "LoopSound" << YAML::Value << soundComponent.LoopSound;
		
			out << YAML::EndMap;
		}
		if (entity.HasComponent<NativeScriptComponent>())
		{
			out << YAML::Key << "NativeScriptComponent";
			out << YAML::BeginMap;

			auto& scriptComponent = entity.GetComponent<NativeScriptComponent>();
			out << YAML::Key << "ScriptClassName" << YAML::Value << scriptComponent.scriptClassName;

			out << YAML::EndMap;
		}


		out << YAML::EndMap; // Entity
	}

    Serializer::Serializer(const Shared<Scene> scene)
        :m_Scene(scene)
    {
    }
    std::string Serializer::SerializeToString()
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << m_Scene->GetName();
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;
        m_Scene->m_Registry.each([&](auto entityID)
            {
                Entity entity = { entityID, m_Scene.get() };
                if (!entity)
                    return;

                SerializeEntity(out, entity);
            });
        out << YAML::EndSeq;
        out << YAML::EndMap;

        return out.c_str();
    }

    void Serializer::Serealize(const std::string& path)
    {
        std::ofstream fout(path);
        fout << SerializeToString();
    }


    bool Serializer::DeserializeFromString(const std::string& yamlSource)
    {
		YAML::Node data = YAML::Load(yamlSource);
		if (!data["Scene"])
			return false;

		std::string sceneName = data["Scene"].as<std::string>();
		m_Scene->SetName(sceneName);

		auto entities = data["Entities"];
		if (entities)
		{
			for (auto entity : entities)
			{
				
				uint64_t uuid = entity["Entity"].as<uint64_t>();

				std::string name;
				auto tagComponent = entity["TagComponent"];
				if (tagComponent)
					name = tagComponent["Tag"].as<std::string>();

				Entity deserializedEntity = m_Scene->CreateEntity(uuid, name);

				auto transformComponent = entity["TransformComponent"];
				if (transformComponent)
				{
					// Entities always have transforms
					auto& tc = deserializedEntity.GetComponent<TransformComponent>();
					tc.Position = transformComponent["Position"].as<glm::vec3>();
					tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
					tc.Scale = transformComponent["Scale"].as<glm::vec3>();
				}

				auto cameraComponent = entity["CameraComponent"];
				if (cameraComponent)
				{
					auto& cc = deserializedEntity.AddComponent<CameraComponent>();

					const auto& cameraProps = cameraComponent["Camera"];
					cc.Camera.SetProjectionType((SceneCamera::Projection)cameraProps["ProjectionType"].as<int>());

					cc.Camera.SetPerspFOV(cameraProps["PerspectiveFOV"].as<float>());
					cc.Camera.SetPerspNearClip(cameraProps["PerspectiveNear"].as<float>());
					cc.Camera.SetPerspFarClipl(cameraProps["PerspectiveFar"].as<float>());

					cc.Camera.SetOrthoSize(cameraProps["OrthographicSize"].as<float>());
					cc.Camera.SetOrthoNearClip(cameraProps["OrthographicNear"].as<float>());
					cc.Camera.SetOrthoFarClipl(cameraProps["OrthographicFar"].as<float>());

					cc.isMain = cameraComponent["Primary"].as<bool>();
					cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
				}

				auto spriteRendererComponent = entity["SpriteRendererComponent"];
				if (spriteRendererComponent)
				{
					auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
					src.Color = spriteRendererComponent["Color"].as<glm::vec4>();
					src.Texture2D = spriteRendererComponent["Texture2D"].as<std::string>();
				}

				auto rigitbodyComponent = entity["RigidBodyComponent"];
				if (rigitbodyComponent)
				{
					auto& src = deserializedEntity.AddComponent<RigidBodyComponent>();
					src.Type = (RigidBodyComponent::BodyType)rigitbodyComponent["BodyType"].as<int>();
					src.IsRotating = rigitbodyComponent["IsRotating"].as<bool>();
				}

				auto boxColisionComponent = entity["BoxCollisionComponent"];
				if (boxColisionComponent)
				{
					auto& src = deserializedEntity.AddComponent<BoxCollisionComponent>();
					src.Size = boxColisionComponent["Size"].as<glm::vec2>();
					src.Offset = boxColisionComponent["Offset"].as<glm::vec2>();

					src.Density = boxColisionComponent["Density"].as<float>();
					src.Friction = boxColisionComponent["Friction"].as<float>();
					src.Restriction = boxColisionComponent["Restriction"].as<float>();
					src.ResitutionTreshhold = boxColisionComponent["ResitutionTreshhold"].as<float>();
					src.isSensor = boxColisionComponent["isSensor"].as<bool>();
				}

				auto soundComponent = entity["SoundComponent"];
				if (soundComponent)
				{
					auto& src = deserializedEntity.AddComponent<SoundComponent>();

					src.Pitch = soundComponent["Pitch"].as<float>();
					src.Gain = soundComponent["Gain"].as<float>();

					src.Position = soundComponent["Position"].as<glm::vec3>();
					src.Velocity = soundComponent["Velocity"].as<glm::vec3>();

					src.LoopSound = soundComponent["LoopSound"].as<bool>();

					src.SetPath(soundComponent["AudioPath"].as<std::string>());

					
				}
				auto scriptComponent = entity["NativeScriptComponent"];
				if (scriptComponent)
				{
					auto& src = deserializedEntity.AddComponent<NativeScriptComponent>();
					src.scriptClassName = scriptComponent["ScriptClassName"].as<std::string>();

				}
			}
		}

		return true;

    }

    bool Serializer::DeSerealize(const std::string& path)
    {
		std::ifstream stream(path);
		std::stringstream strStream;
		strStream << stream.rdbuf();

		return DeserializeFromString(strStream.str());
    }

}