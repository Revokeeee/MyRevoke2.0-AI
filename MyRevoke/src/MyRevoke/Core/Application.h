#pragma once

#include "Core.h"
#include "Time.h"

#include "Window.h"
#include "LayerStack.h"

#include "MyRevoke/EventSystem/Event.h"
#include "MyRevoke/EventSystem/AppEvent.h"
#include "MyRevoke/EventSystem/MouseEvent.h"
#include "MyRevoke/EventSystem/KeyEvent.h"

#include "MyRevoke/ImGui/ImGuiLayer.h"
#include "MyRevoke/Renderer/Shader.h"
#include "MyRevoke/Renderer/BuffersAPI.h"
#include "MyRevoke/Renderer/Camera.h"


namespace Revoke
{

	class  Application
	{
	public:
		Application(const WindowSettings& windowSettings = WindowSettings());
		virtual ~Application();

		void Run();

		void OnEvent(Event& e);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		// Quits immediately.
		void Close();
		// Quits unless a layer vetoes it through Layer::OnCloseRequested().
		void RequestClose();

		ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }

		static Application& Get() { 

			return *s_Instance;
		}

		static Application* GetPointer() {

			return s_Instance;
		}

		Window& GetWindow() { return *m_Window; }

	private:
		bool OnWindowClose(WindowsCloseEvent e);
		bool OnWindowResize(WindowResizeEvent e);

	private:

		Shared<Window> m_Window;
		LayerStack m_LayerStack;
		ImGuiLayer* m_ImGuiLayer;

		float m_LastFrame = 0;

		bool m_Run = true;
		
	private:
		static Application* s_Instance;
	};


	Application* CreateApplication();
}
