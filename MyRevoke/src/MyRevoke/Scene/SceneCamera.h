#pragma once
#include "MyRevoke/Renderer/Camera.h"

namespace Revoke {

	class SceneCamera : public Camera
	{
	public:
		enum class Projection
		{
			Perspective = 0,
			Orthographic = 1
		};
	public:
		SceneCamera();
		virtual ~SceneCamera() = default;

		void SetOrthographic(float size, float nearClip, float farClip);
		void SetPerspective(float FOW, float nearClip, float farClip);

		void SetViewportSize(uint32_t width, uint32_t height);

		float GetOrthoSize() const { return m_OrthographicSize; }
		void SetOrthoSize(float size) { m_OrthographicSize = size; RecalculateProjection(); }
		float GetOrthoNearClip() const { return m_OrthographicNear; }
		float GetOrthoFarClip() const { return m_OrthographicFar; }
		void SetOrthoNearClip(float nearClip) { m_OrthographicNear = nearClip; RecalculateProjection(); }
		void SetOrthoFarClipl(float farClip) { m_OrthographicFar = farClip; RecalculateProjection(); }

		float GetPerspFOV() const { return m_PerspectiveFOV; }
		void  SetPerspFOV(float FOW) { m_PerspectiveFOV = FOW; RecalculateProjection(); }
		float GetPerspNearClip() const { return m_PerspectiveNear; }
		float GetPerspFarClip() const { return m_PerspectiveFar; }
		void  SetPerspNearClip(float nearClip) { m_PerspectiveNear = nearClip; RecalculateProjection(); }
		void  SetPerspFarClipl(float farClip) { m_PerspectiveFar = farClip; RecalculateProjection(); }

		Projection GetProjectionType() const { return m_ProjectionType; }
		void SetProjectionType(const Projection& projectionType) { m_ProjectionType = projectionType; RecalculateProjection(); }

	
		void RecalculateProjection();
	private:
		Projection m_ProjectionType = Projection::Orthographic;

		float m_OrthographicSize = 10.0f;
		float m_OrthographicNear = -1.0f,
			m_OrthographicFar = 1.0f;

		float m_PerspectiveFOV = glm::radians(45.0f);
		float m_PerspectiveNear = 0.01f,
			m_PerspectiveFar = 1000.0f;

		float m_AspectRatio = 0.0f;
	};

}