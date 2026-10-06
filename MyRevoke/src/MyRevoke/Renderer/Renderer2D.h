#pragma once

#include "RendererAPI.h"

#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "EditorCamera.h"
#include "MyRevoke/Scene/Components.h"

#include "glm/glm.hpp"

namespace Revoke
{

	class Renderer2D
	{
	public:

		static void Init();
		static void Shutdown();

		static void Begin(const Camera& camera, const glm::mat4 transform);
		static void Begin(const EditorCamera& camera);
		static void End();

		

		static void DrawQuad(const glm::vec3& position, const glm::vec2 size, const glm::vec4& color, int entityID);
		static void DrawQuad(const glm::vec2& position, const glm::vec2 size, const glm::vec4& color, int entityID);
		static void DrawQuad(const glm::vec3& position, const glm::vec2 size, const Shared<Texture>& texture, int entityID);
		static void DrawQuad(const glm::vec2& position, const glm::vec2 size, const Shared<Texture>& texture, int entityID);

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID);
		static void DrawQuad(const glm::mat4& transform, const Shared<Texture>& texture, int entityID, const glm::vec4& tint = glm::vec4(1.0f));

		// Textured sprites are tinted by their Color.
		static void DrawSprite(const glm::mat4& transform, SpriteRendererComponent& sprite, int entityID);

		// Loads each file once and hands out the same texture after that. Sprites draw every
		// frame, so loading from disk per draw would stall the editor.
		static Shared<Texture> GetTexture(const std::string& path);

		static void QuadInit();
	private:
		static void NewBatch();
	public:

		// Stats
		struct Stats
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6; }
		};

		static Stats GetStats();
		static void ResetStatistics();

	};

}