#include "PlayerLayer.h"

namespace Revoke
{
	PlayerLayer::PlayerLayer(Shared<Project> project)
		: Layer("PlayerLayer"), m_Project(std::move(project))
	{
	}

	void PlayerLayer::OnAttach()
	{
		// The same background the editor viewport clears to, so Play mode and the player match.
		RendererAPI::SetClearColor({ 0.2f, 0.2f, 0.2f, 1.0f });
		RendererAPI::EnableBlending();

		if (!m_Project)
			return;

		std::filesystem::path startupScene = m_Project->GetStartupScenePath();
		if (startupScene.empty())
		{
			RV_ENGINE_ERROR("Project {0} names no startup scene", m_Project->GetName());
			return;
		}

		Shared<Scene> scene = std::make_shared<Scene>();
		Serializer serializer(scene, m_Project->GetAssetsDirectory());
		if (!serializer.DeSerealize(startupScene.string()))
		{
			RV_ENGINE_ERROR("Could not load the startup scene: {0}", startupScene.string());
			return;
		}

		m_Scene = scene;

		Window& window = Application::Get().GetWindow();
		m_ViewportWidth = window.GetWidth();
		m_ViewportHeight = window.GetHeight();
		m_Scene->OnViewportResize(m_ViewportWidth, m_ViewportHeight);

		m_Scene->OnRuntimeStart();
	}

	void PlayerLayer::OnDetach()
	{
		StopRuntime();
	}

	void PlayerLayer::OnUpdate(Timestep deltaTime)
	{
		RendererAPI::Clear();

		if (!m_Scene)
			return;

		// The scene draws straight to the window, so the window is the viewport. Resize events never
		// reach a layer (Application marks them handled), so the size is polled instead.
		Window& window = Application::Get().GetWindow();
		if (window.GetWidth() != m_ViewportWidth || window.GetHeight() != m_ViewportHeight)
		{
			m_ViewportWidth = window.GetWidth();
			m_ViewportHeight = window.GetHeight();
			m_Scene->OnViewportResize(m_ViewportWidth, m_ViewportHeight);
		}

		m_Scene->OnRuntimeUpdate(deltaTime);
	}

	bool PlayerLayer::OnCloseRequested()
	{
		// Stop while the audio device and the script DLL are still up: Application shuts those down
		// before it destroys its layers.
		StopRuntime();
		return true;
	}

	void PlayerLayer::StopRuntime()
	{
		if (!m_Scene)
			return;

		m_Scene->OnRuntimeStop();
		m_Scene->OnSceneClose();
		m_Scene.reset();
	}
}
