#include "rvpch.h"
#include "Application.h"
#include "Log.h"
#include "Input.h"
#include "Core.h"

#include "MyRevoke/Renderer/Renderer2D.h"
#include "MyRevoke/AudioManager/AudioRenderer.h"
#include "MyRevoke/Scripting/NativeScript.h"

#include "GLFW/glfw3.h"

namespace Revoke
{
	
	Application* Application::s_Instance = nullptr;
	

	Application::Application(const WindowSettings& windowSettings)

	{
		Log::Init();

		ALuint test = 2;
		RV_CORE_ASSERT(!s_Instance, "Application already exist");
		s_Instance = this;

		m_Window = std::make_shared<Window>(windowSettings);
		m_Window->SetEventCallback(RV_BIND_EVENT_FUNK(Application::OnEvent));


		RendererAPI::Init();
		Renderer2D::Init();
		AudioRenderer::Init();
		ScriptEngine::InitDll();
		

		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);
	
		
	}
	Application::~Application()
	{
		AudioRenderer::Shutdown();
		Renderer2D::Shutdown();
	}
	void Application::Run()
	{
	
		while (m_Run)
		{
			float time = (float)glfwGetTime();
			Timestep timestep = time - m_LastFrame;
			m_LastFrame = time;

			for (Layer* layer : m_LayerStack)
				layer->OnUpdate(timestep);

			m_ImGuiLayer->Begin();
			for (Layer* layer : m_LayerStack)
				layer->OnImGuiDraw();
			m_ImGuiLayer->End();


			m_Window->OnUpdate();
		}
	}
	void Application::Close()
	{
		m_Run = false;
	}
	void Application::RequestClose()
	{
		for (Layer* layer : m_LayerStack)
		{
			if (!layer->OnCloseRequested())
				return;
		}
		m_Run = false;
	}
	void Application::OnEvent(Event& e)
	{
		
		EventDispatcher eDispatcher(e);
		
		eDispatcher.Dispatch<WindowsCloseEvent>(RV_BIND_EVENT_FUNK(Application::OnWindowClose));
		eDispatcher.Dispatch<WindowResizeEvent>(RV_BIND_EVENT_FUNK(Application::OnWindowResize));

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend();++it)
		{
			if (e.Handled)
				break;
			(*it)->OnEvent(e);
		}

	}
	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}
	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.PushOverlay(layer);
		layer->OnAttach();
	}
	bool Application::OnWindowClose(WindowsCloseEvent e)
	{
		RequestClose();

		// GLFW has already flagged the window to close by the time this event arrives.
		if (m_Run)
			glfwSetWindowShouldClose((GLFWwindow*)m_Window->GetCoreWindow(), GLFW_FALSE);
		return true;
	}
	bool Application::OnWindowResize(WindowResizeEvent e)
	{
		RendererAPI::WindowResize((float)e.GetWidth(), (float)e.GetHeight());
		return true;
	}
}