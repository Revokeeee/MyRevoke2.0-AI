#include <doctest/doctest.h>

#include "MyRevoke/Scene/SceneCamera.h"

#include <cmath>

using namespace Revoke;

// The editor resizes scene cameras with the viewport size, which is 0 before the viewport is laid
// out. That must not leave the camera with a broken projection (or trip glm's perspective assert).

static bool IsFinite(const glm::mat4& matrix)
{
	for (int column = 0; column < 4; column++)
		for (int row = 0; row < 4; row++)
			if (!std::isfinite(matrix[column][row]))
				return false;
	return true;
}

TEST_CASE("A new scene camera has a usable projection")
{
	SceneCamera camera;
	CHECK(IsFinite(camera.GetProjectionMatrix()));

	camera.SetProjectionType(SceneCamera::Projection::Perspective);
	CHECK(IsFinite(camera.GetProjectionMatrix()));
}

TEST_CASE("A zero viewport size keeps the previous projection")
{
	SceneCamera camera;
	camera.SetProjectionType(SceneCamera::Projection::Perspective);
	camera.SetViewportSize(1280, 720);
	glm::mat4 before = camera.GetProjectionMatrix();

	camera.SetViewportSize(0, 0);
	CHECK(camera.GetProjectionMatrix() == before);

	camera.SetViewportSize(1280, 0);
	CHECK(camera.GetProjectionMatrix() == before);

	camera.SetViewportSize(0, 720);
	CHECK(camera.GetProjectionMatrix() == before);
}
