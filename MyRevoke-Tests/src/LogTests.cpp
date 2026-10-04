#include <doctest/doctest.h>

#include "MyRevoke/Core/Log.h"

TEST_CASE("Log::Init works without constructing Application")
{
	Revoke::Log::Init();

	CHECK(Revoke::Log::GetCoreLogger() != nullptr);
	CHECK(Revoke::Log::GetClientLogger() != nullptr);

	RV_ENGINE_INFO("Log::Init sanity check from MyRevoke-Tests");
	RV_CORE_ASSERT(Revoke::Log::GetCoreLogger(), "Core logger should exist after Log::Init");
}

TEST_CASE("Log::Init is safe to call more than once")
{
	Revoke::Log::Init();
	Revoke::Log::Init();

	CHECK(Revoke::Log::GetCoreLogger() != nullptr);
}
