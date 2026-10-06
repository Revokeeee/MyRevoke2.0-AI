#include "rvpch.h"

#include "Window.h"

#include "MyRevoke/EventSystem/AppEvent.h"
#include "MyRevoke/EventSystem/KeyEvent.h"
#include "MyRevoke/EventSystem/MouseEvent.h"

#include "MyRevoke/Renderer/GraphicContex.h"

#include <GLFW/glfw3.h>


namespace Revoke
{
	static bool s_GLFWInitialization = false;

	static void GLFWErrorCallBack(int error, const char* description)
	{

		RV_ENGINE_ERROR("GLFW Error ({0}: {1}", error, description);
	}


	Window::Window(const WindowSettings& settings)
	{
		Init(settings);
	}


	Window::~Window()
	{
		Shutdown();
	}

	void Window::Init(const WindowSettings& settings)
	{

		m_Data.Title = settings.Title;
		m_Data.Width = settings.Width;
		m_Data.Height = settings.Height;


		RV_ENGINE_INFO("Window is initialized {0} ({1}, {2})", settings.Title, settings.Width, settings.Height);

		if (!s_GLFWInitialization)
		{
			//Add glfwTerminate on system shutdown
			int success = glfwInit();
			RV_CORE_ASSERT(success, "Could not initialize GLFW");

			glfwSetErrorCallback(GLFWErrorCallBack);

			s_GLFWInitialization = true;
		}


		glfwWindowHint(GLFW_MAXIMIZED, settings.Maximized ? GLFW_TRUE : GLFW_FALSE);

		m_Window = glfwCreateWindow((int)settings.Width, (int)settings.Height, m_Data.Title.c_str(), nullptr, nullptr);

		// A maximized (or otherwise OS-clamped) window's real size differs from the
		// requested one, so read it back before anything uses m_Data.Width/Height.
		int actualWidth, actualHeight;
		glfwGetWindowSize(m_Window, &actualWidth, &actualHeight);
		m_Data.Width = actualWidth;
		m_Data.Height = actualHeight;

		m_Context = new RenderContex(m_Window);
		m_Context->Init();


		glfwSetWindowUserPointer(m_Window, &m_Data);
		SetVSync(true);

		//Set GLFW callback
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Width = width;
				data.Height = height;

				WindowResizeEvent event(width, height);
				data.EventCallback(event);
			});
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				WindowsCloseEvent event;
				data.EventCallback(event);
			});
		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancoed, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.EventCallback(event);
					break;
				}
				}
			});
		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int character)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				KeyTypedEvent event(character);
				data.EventCallback(event);

			});
		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.EventCallback(event);
					break;
				}
				}
			});
		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				MouseScrolledEvent event(xOffset, yOffset);
				data.EventCallback(event);
			});
		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				MouseMovedEvent event((float)xPos, (float)yPos);
				data.EventCallback(event);
			});
	}

	void Window::Shutdown()
	{
		glfwDestroyWindow(m_Window);
	}

	void Window::OnUpdate()
	{
		glfwPollEvents();
		m_Context->SwapBuffers();

	}

	void Window::SetVSync(bool cond)
	{
		if (cond)
			glfwSwapInterval(1);
		else
			glfwSwapInterval(0);
		m_Data.VSync = cond;
	}

	void Window::SetTitle(const std::string& title)
	{
		if (title == m_Data.Title)
			return;

		m_Data.Title = title;
		glfwSetWindowTitle(m_Window, title.c_str());
	}

	bool Window::IsVSync() const
	{
		return m_Data.VSync;
	}

}
