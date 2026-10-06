#pragma once
#include <memory>
#include <string>

namespace Revoke
{
	class Texture 
	{
	public:
		Texture(const std::string& path);
		Texture(int width, int height);
		~Texture();

		uint32_t GetWidth() const { return m_Width; };
		uint32_t GetHeight() const { return m_Height; };
		uint32_t GetID() const { return m_RendererID; }
		// False when the file could not be read; the texture then holds a placeholder.
		bool IsLoaded() const { return m_Loaded; }

		void Bind(uint32_t slot) const;
		void UnBind() const;

		void SetData(void* data, uint32_t size);

	private:
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		bool m_Loaded = true;
	};


}