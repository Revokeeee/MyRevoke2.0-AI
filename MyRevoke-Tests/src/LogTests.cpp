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

TEST_CASE("Logged messages reach the history the editor's console reads")
{
	Revoke::Log::Init();
	Revoke::Log::ClearHistory();
	uint64_t versionBefore = Revoke::Log::GetHistoryVersion();

	RV_ENGINE_WARN("engine warning {}", 1);
	RV_EDITOR_ERROR("editor error");

	auto history = Revoke::Log::GetHistory();
	REQUIRE(history.size() == 2);
	CHECK(history[0].Level == spdlog::level::warn);
	CHECK(history[0].Source == "MYREVOKE");
	CHECK(history[0].Message == "engine warning 1");
	CHECK(history[0].Time.size() == 8);
	CHECK(history[1].Level == spdlog::level::err);
	CHECK(history[1].Source == "APP");
	CHECK(Revoke::Log::GetHistoryVersion() > versionBefore);

	Revoke::Log::ClearHistory();
	CHECK(Revoke::Log::GetHistory().empty());
}
