#include "rvpch.h"
#include "Texture.h"

#include "MyRevoke/Core/Core.h"

#include "stb_image.h"
#include "glad/glad.h"

namespace Revoke
{
	Texture::Texture(const std::string& path)
	{
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		// Always expand to RGBA: grayscale images then load too, and RGB rows with an odd
		// width don't trip GL's default 4-byte unpack alignment.
		stbi_uc* data = stbi_load(path.c_str(), &width, &height, &channels, 4);

		// A missing or broken file must not take the editor down (a scene can point at a file
		// that was moved). Show a 2x2 magenta checker instead, so the problem is visible.
		uint32_t missingData[4] = { 0xffff00ff, 0xff000000, 0xff000000, 0xffff00ff };
		m_Loaded = data != nullptr;
		if (!m_Loaded)
		{
			RV_ENGINE_ERROR("Could not load texture {}: {}", path, stbi_failure_reason() ? stbi_failure_reason() : "unknown error");
			width = 2;
			height = 2;
		}
		m_Width = width;
		m_Height = height;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, GL_RGBA8, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, m_Loaded ? GL_LINEAR : GL_NEAREST);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, m_Loaded ? (const void*)data : (const void*)missingData);

		if (data)
			stbi_image_free(data);
	}

	Texture::Texture(int width, int height)
		: m_Width(width), m_Height(height)
	{

		GLenum internalFormat = GL_RGBA8, dataFormat = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, internalFormat, m_Width, m_Height);

		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

	}

	void Texture::SetData(void* data, uint32_t size)
	{
		uint32_t bpp = 4;
		RV_CORE_ASSERT(size == m_Width * m_Height * bpp, "Data must be entire texture!");
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}

	Texture::~Texture()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void Texture::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}

	void Texture::UnBind() const
	{
		glBindTextureUnit(0, m_RendererID);
	}

}
