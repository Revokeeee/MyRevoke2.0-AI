#pragma once

#include "glm/glm.hpp"

#include "MyRevoke/Renderer/BuffersAPI.h"

namespace Revoke
{
	class RendererAPI
	{
	public:
		static void SetClearColor(const glm::vec4 color);
		static void Clear();
		static void DrawElements(const Shared<VertexArray>& vertrexArray, uint32_t count = 0);
		static void EnableBlending();
		static void DisableBlending();
		static void WindowResize(float width, float height);
		 
		static void Init();
	};
}