#include <doctest/doctest.h>

#include "MyRevoke/Math/Math.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace Revoke;

namespace
{
	const float kTolerance = 0.0001f;

	void CheckVec3(const glm::vec3& actual, const glm::vec3& expected, float tolerance = kTolerance)
	{
		CHECK(actual.x == doctest::Approx(expected.x).epsilon(tolerance));
		CHECK(actual.y == doctest::Approx(expected.y).epsilon(tolerance));
		CHECK(actual.z == doctest::Approx(expected.z).epsilon(tolerance));
	}

	// Mirrors Revoke::TransformComponent::GetTransform, so the composed matrix matches
	// what the engine actually feeds into DecomposeTransform at runtime.
	glm::mat4 ComposeTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
	{
		glm::mat4 rotationMatrix = glm::toMat4(glm::quat(rotation));
		return glm::translate(glm::mat4(1.0f), translation) * rotationMatrix * glm::scale(glm::mat4(1.0f), scale);
	}
}

TEST_CASE("DecomposeTransform: identity matrix")
{
	glm::vec3 translation, rotation, scale;
	bool success = DecomposeTransform(glm::mat4(1.0f), translation, rotation, scale);

	CHECK(success);
	CheckVec3(translation, { 0.0f, 0.0f, 0.0f });
	CheckVec3(rotation, { 0.0f, 0.0f, 0.0f });
	CheckVec3(scale, { 1.0f, 1.0f, 1.0f });
}

TEST_CASE("DecomposeTransform: pure translation")
{
	glm::vec3 expectedTranslation = { 3.0f, -2.5f, 7.0f };
	glm::mat4 transform = glm::translate(glm::mat4(1.0f), expectedTranslation);

	glm::vec3 translation, rotation, scale;
	bool success = DecomposeTransform(transform, translation, rotation, scale);

	CHECK(success);
	CheckVec3(translation, expectedTranslation);
	CheckVec3(rotation, { 0.0f, 0.0f, 0.0f });
	CheckVec3(scale, { 1.0f, 1.0f, 1.0f });
}

TEST_CASE("DecomposeTransform: pure scale")
{
	glm::vec3 expectedScale = { 2.0f, 3.0f, 4.0f };
	glm::mat4 transform = glm::scale(glm::mat4(1.0f), expectedScale);

	glm::vec3 translation, rotation, scale;
	bool success = DecomposeTransform(transform, translation, rotation, scale);

	CHECK(success);
	CheckVec3(translation, { 0.0f, 0.0f, 0.0f });
	CheckVec3(rotation, { 0.0f, 0.0f, 0.0f });
	CheckVec3(scale, expectedScale);
}

TEST_CASE("DecomposeTransform: combined translate/rotate/scale round-trips")
{
	glm::vec3 expectedTranslation = { 1.0f, 2.0f, -3.0f };
	// Kept away from +-90 degrees on the Y axis, where the decomposition hits gimbal lock.
	glm::vec3 expectedRotation = { 0.3f, 0.4f, 0.5f };
	glm::vec3 expectedScale = { 2.0f, 1.5f, 0.5f };

	glm::mat4 transform = ComposeTransform(expectedTranslation, expectedRotation, expectedScale);

	glm::vec3 translation, rotation, scale;
	bool success = DecomposeTransform(transform, translation, rotation, scale);

	CHECK(success);
	CheckVec3(translation, expectedTranslation);
	CheckVec3(rotation, expectedRotation);
	CheckVec3(scale, expectedScale);
}
