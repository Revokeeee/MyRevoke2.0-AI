#include "rvpch.h"

#include "MyRevoke/Renderer/Renderer2D.h"
#include "MyRevoke/AudioManager/AudioRenderer.h"

#include "Scene.h"
#include "Components.h"
#include "Entity.h"
#include "Serializer.h"

#include <fstream>

#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

#include <box2d/b2_world.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_fixture.h>

#include <sndfile.h>

#include <AL/al.h>



namespace Revoke
{

    Scene::Scene()
    {
    }

    Scene::Scene(std::string name )
        :m_Name(name)
    {     
    }

    Scene::~Scene()
    {

    }

    Shared<Scene> Scene::Copy(const Shared<Scene>& source, const std::filesystem::path& assetsDirectory)
    {
        Serializer sourceSerializer(source, assetsDirectory);
        std::string snapshot = sourceSerializer.SerializeToString();

        Shared<Scene> copy = std::make_shared<Scene>();
        Serializer copySerializer(copy, assetsDirectory);
        copySerializer.DeserializeFromString(snapshot);

        // Scene settings the scene file does not store.
        copy->SetGravityStats(source->m_PositionIteration, source->m_VelocityIteration);
        copy->OnViewportResize(source->m_ViewportWidth, source->m_ViewportHeight);

        return copy;
    }

    Entity Scene::CreateEntity(const std::string name)
    {
        Entity entity = { m_Registry.create(), this };
        m_EntityOrder.push_back(entity);
        entity.AddComponent<IdComponent>();
        entity.AddComponent<TransformComponent>();
        auto& entName = entity.AddComponent<NameComponent>();
        
        entName.Name = name.empty() ? "UnNamed Entity" : name;
        return entity;
    }

    Entity Scene::CreateEntity(UUID id, const std::string name)
    {
        Entity entity = { m_Registry.create(), this };
        m_EntityOrder.push_back(entity);
        entity.AddComponent<IdComponent>(id);
        entity.AddComponent<TransformComponent>();
        auto& entName = entity.AddComponent<NameComponent>();

        entName.Name = name.empty() ? "UnNamed Entity" : name;
        return entity;
    }

    template<typename T>
    static void CopyComponentIfExists(Entity destination, Entity source)
    {
        if (!source.HasComponent<T>())
            return;

        // Copy out first: the destination's storage may grow while the component is added.
        T component = source.GetComponent<T>();
        if (destination.HasComponent<T>())
            destination.GetComponent<T>() = component;
        else
            destination.AddComponent<T>(component);
    }

    Entity Scene::DuplicateEntity(Entity source)
    {
        Entity copy = CreateEntity(source.GetComponent<NameComponent>().Name);

        CopyComponentIfExists<TransformComponent>(copy, source);
        CopyComponentIfExists<SpriteRendererComponent>(copy, source);
        CopyComponentIfExists<CameraComponent>(copy, source);
        CopyComponentIfExists<RigidBodyComponent>(copy, source);
        CopyComponentIfExists<BoxCollisionComponent>(copy, source);
        CopyComponentIfExists<NativeScriptComponent>(copy, source);
        CopyComponentIfExists<SoundComponent>(copy, source);

        // Duplicating the main camera would leave two entities claiming the role.
        if (copy.HasComponent<CameraComponent>())
            copy.GetComponent<CameraComponent>().isMain = false;

        // Runtime objects belong to the source; the copy gets its own or none.
        if (copy.HasComponent<RigidBodyComponent>())
            copy.GetComponent<RigidBodyComponent>().Body = nullptr;
        if (copy.HasComponent<NativeScriptComponent>())
            copy.GetComponent<NativeScriptComponent>().Instance = nullptr;
        if (copy.HasComponent<SoundComponent>())
        {
            auto& sound = copy.GetComponent<SoundComponent>();
            sound.BufferID = 0;
            sound.SourceID = 0;
            sound.SetPath(sound.AudioPath);
        }

        return copy;
    }

    std::vector<Entity> Scene::GetEntities()
    {
        std::vector<Entity> entities;
        entities.reserve(m_EntityOrder.size());
        for (entt::entity handle : m_EntityOrder)
        {
            if (m_Registry.valid(handle))
                entities.emplace_back(handle, this);
        }
        return entities;
    }

    size_t Scene::GetEntityCount()
    {
        return m_EntityOrder.size();
    }

    Entity Scene::FindEntityByUUID(UUID id)
    {
        auto view = m_Registry.view<IdComponent>();
        for (auto ent : view)
        {
            if (view.get<IdComponent>(ent).ID == id)
                return { ent, this };
        }
        return {};
    }


    void Scene::OnRuntimeStart()
    {

        b2Vec2 gravity = { 0.0f, -9.8f };
        m_B2World = new b2World(gravity);
        auto view = m_Registry.view<RigidBodyComponent>();
        for (auto ent : view)
        {
            Entity entity = { ent ,this };

            auto& rigitBody = entity.GetComponent<RigidBodyComponent>();
            auto& transforms = entity.GetComponent<TransformComponent>();

            b2BodyDef bodyDefenition;
            bodyDefenition.type = (b2BodyType)rigitBody.Type;
            bodyDefenition.position.Set(transforms.Position.x, transforms.Position.y);
            bodyDefenition.angle = transforms.Rotation.z;

            b2Body* body = m_B2World->CreateBody(&bodyDefenition);
            //TODO: add more options
            body->SetFixedRotation(rigitBody.IsRotating);

            rigitBody.Body = body;

            if (entity.HasComponent<BoxCollisionComponent>())
            {
                auto& boxColidor = entity.GetComponent<BoxCollisionComponent>();

                b2PolygonShape shape;
                // The offset was never applied, so editing it in the Properties panel did nothing.
                b2Vec2 offset(transforms.Scale.x * boxColidor.Offset.x, transforms.Scale.y * boxColidor.Offset.y);
                shape.SetAsBox(transforms.Scale.x * boxColidor.Size.x, transforms.Scale.y * boxColidor.Size.y, offset, 0.0f);

                b2FixtureDef fixture;
                fixture.shape = &shape;
                fixture.density = boxColidor.Density;
                fixture.friction = boxColidor.Friction;
                fixture.restitution = boxColidor.Restriction;
                fixture.restitutionThreshold = boxColidor.ResitutionTreshhold;
                fixture.isSensor = boxColidor.isSensor;

                //TODO: Save fixture?
                body->CreateFixture(&fixture);
            }
        }

        { // Sounds

            auto view = m_Registry.view<SoundComponent>();
            for (auto ent : view)
            {
                Entity entity = { ent, this };
                auto& soundComponent = entity.GetComponent<SoundComponent>();
                soundComponent.HasPlayed = false;

            };

        }
        
     
    }


    void Scene::OnRuntimeUpdate(Timestep ts)
    {
        
        { // Scripts
            m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc)
                {
                        
                    if (nsc.scriptClassName.empty())
                        return;

                    if (!nsc.Instance)
                    {
                        nsc.Instance = ScriptEngine::GetScritpByName(nsc.scriptClassName);
                        if (!nsc.Instance)
                        {
                            return;
                        }
                        nsc.Instance->m_Entity = Entity{ entity, this };

                        nsc.Instance->OnCreate();
                    }
                    nsc.Instance->OnUpdate(ts);
                   
                 
                });
        }

        {// Physics
            m_B2World->Step(ts, m_VelocityIteration, m_PositionIteration);

            auto view = m_Registry.view<RigidBodyComponent>();
            for (auto ent : view)
            {
                Entity entity = { ent, this };

                auto& rigitBody = entity.GetComponent<RigidBodyComponent>();
                auto& transforms = entity.GetComponent<TransformComponent>();

                // Bodies are created in OnRuntimeStart, so one added from the Properties panel
                // while playing has none yet.
                b2Body* body = rigitBody.Body;
                if (!body)
                    continue;
                const auto& pos = body->GetPosition();

                transforms.Position.x = pos.x;
                transforms.Position.y = pos.y;
                transforms.Rotation.z = body->GetAngle();
            };
        }
        
        
        { // Sounds

            auto view = m_Registry.view<SoundComponent>();
            for (auto ent : view)
            {
                Entity entity = { ent, this };
                auto& soundComponent = entity.GetComponent<SoundComponent>();

                soundComponent.UpdateSource();

            };

        }
            

        { // Rendering

            Camera* mainCamera = nullptr;
            glm::mat4 cameraTransform;

            auto view = m_Registry.view<TransformComponent, CameraComponent>();

            for (auto entity : view)
            {
                auto [transform, camera] = view.get<TransformComponent, CameraComponent>(entity);

                if (camera.isMain)
                {
                    mainCamera = &camera.Camera;
                    cameraTransform = transform.GetTransform();
                    break;
                }
            }


            if (mainCamera)
            {
                Renderer2D::Begin(*mainCamera, cameraTransform);

                auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
                for (auto entity : group)
                {
                    auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
                    Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);

                }
                Renderer2D::End();
            }

        }
      
    }
    void Scene::OnEditorUpdate(Timestep ts, EditorCamera& camera)
    {
        Renderer2D::Begin(camera);

        auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
        for (auto entity : group)
        {
            auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);

            Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
           
      
        }
        Renderer2D::End();
    }
    void Scene::OnRuntimeStop()
    {
        DestroyAllScriptInstances();

        m_Registry.view<RigidBodyComponent>().each([](auto entity, auto& rigidBody)
            {
                rigidBody.Body = nullptr;
            });

        delete m_B2World;
        m_B2World = nullptr;
    }
    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
        m_ViewportWidth = width;
        m_ViewportHeight = height;

        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto& cameraComponent = view.get<CameraComponent>(entity);
            if (!cameraComponent.FixedAspectRatio)
                cameraComponent.Camera.SetViewportSize(width, height);
        }

    }
    void Scene::RemoveEntity(Entity ent)
    {
        if (ent.HasComponent<SoundComponent>())
            ent.GetComponent<SoundComponent>().ShutDown();
        DestroyScriptInstance(ent);
        DestroyPhysicsBody(ent);

        m_EntityOrder.erase(std::remove(m_EntityOrder.begin(), m_EntityOrder.end(), (entt::entity)ent), m_EntityOrder.end());
        m_Registry.destroy(ent);
    }

    void Scene::DestroyPhysicsBody(Entity entity)
    {
        // Removed while playing: take the body out of the world too, or it keeps colliding.
        if (!entity.HasComponent<RigidBodyComponent>())
            return;

        auto& rigidBody = entity.GetComponent<RigidBodyComponent>();
        if (m_B2World && rigidBody.Body)
            m_B2World->DestroyBody(rigidBody.Body);
        rigidBody.Body = nullptr;
    }

    void Scene::DestroyCollider(Entity entity)
    {
        if (!m_B2World || !entity.HasComponent<RigidBodyComponent>())
            return;

        b2Body* body = entity.GetComponent<RigidBodyComponent>().Body;
        if (!body)
            return;

        // A body gets at most one fixture, from its BoxCollisionComponent.
        while (b2Fixture* fixture = body->GetFixtureList())
            body->DestroyFixture(fixture);
    }

    void Scene::DestroyScriptInstance(Entity entity)
    {
        if (!entity.HasComponent<NativeScriptComponent>())
            return;

        auto& nsc = entity.GetComponent<NativeScriptComponent>();
        if (!nsc.Instance)
            return;

        nsc.Instance->OnDestroy();
        delete nsc.Instance;
        nsc.Instance = nullptr;
    }

    void Scene::DestroyAllScriptInstances()
    {
        m_Registry.view<NativeScriptComponent>().each([this](auto entity, auto&)
            {
                DestroyScriptInstance(Entity{ entity, this });
            });
    }

    void Scene::OnSceneClose()
    {
        // A scene can be dropped without ever being stopped, and the script DLL is still
        // loaded here, so this is the last point where OnDestroy can run.
        DestroyAllScriptInstances();

        { // Sounds

            auto view = m_Registry.view<SoundComponent>();
            for (auto ent : view)
            {
                Entity entity = { ent, this };
                auto& soundComponent = entity.GetComponent<SoundComponent>();
                soundComponent.ShutDown();
            };

        }
    }

    Entity Scene::GetMainCamera()
    {
        auto view = m_Registry.view<CameraComponent>();
        for (auto ent : view)
        {
            const auto& camera = view.get<CameraComponent>(ent);
            if (camera.isMain)
            {
               
                return Entity{ ent, this };
            }
        }
        return {};
    }



    template<typename T>
    void Scene::OnComponentAdded(Entity entity, T& component)
    {
       
    }
    template<>
    void Scene::OnComponentAdded<IdComponent>(Entity entity, IdComponent& component)
    {

    }
    template<>
    void Scene::OnComponentAdded<TransformComponent>(Entity entity, TransformComponent& component)
    {

    }
    template<>
    void Scene::OnComponentAdded<CameraComponent>(Entity entity, CameraComponent& component)
    {
        component.Camera.SetViewportSize(m_ViewportWidth, m_ViewportHeight);
    }
    template<>
    void Scene::OnComponentAdded<SpriteRendererComponent>(Entity entity, SpriteRendererComponent& component)
    {
        
    }
    template<>
    void Scene::OnComponentAdded<NameComponent>(Entity entity, NameComponent& component)
    {

    }
    template<>
    void Scene::OnComponentAdded<NativeScriptComponent>(Entity entity, NativeScriptComponent& component)
    {
        
    }
    template<>
    void Scene::OnComponentAdded<RigidBodyComponent>(Entity entity, RigidBodyComponent& component)
    {

    }
    template<>
    void Scene::OnComponentAdded<BoxCollisionComponent>(Entity entity, BoxCollisionComponent& component)
    {

    }
    template<>
    void Scene::OnComponentAdded<SoundComponent>(Entity entity, SoundComponent& component)
    {

    }

    template void Scene::OnComponentAdded(Entity, IdComponent&);
    template void Scene::OnComponentAdded(Entity, TransformComponent&);
    template void Scene::OnComponentAdded(Entity, CameraComponent&);
    template void Scene::OnComponentAdded(Entity, SpriteRendererComponent&);
    template void Scene::OnComponentAdded(Entity, NameComponent&);
    template void Scene::OnComponentAdded(Entity, NativeScriptComponent&);
};

