#pragma once

#include "rvpch.h"
#include "Core.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Revoke{
	// One logged message, as kept for the editor's console.
	struct LogEntry
	{
		spdlog::level::level_enum Level = spdlog::level::info;
		// The logger's name: "MYREVOKE" for the engine, "APP" for the editor.
		std::string Source;
		std::string Message;
		// Local time, HH:MM:SS.
		std::string Time;
	};

	class  Log
	{
	public:

		static void Init();

		static Shared<spdlog::logger> GetCoreLogger() { return s_EngineLogger; }
		static Shared<spdlog::logger> GetClientLogger() { return s_EditorLogger; }

		// The most recent messages from both loggers, oldest first. Only the last few thousand are kept.
		static std::vector<LogEntry> GetHistory();
		// Goes up with every message and every ClearHistory(), so a reader can skip copying the
		// history when nothing changed.
		static uint64_t GetHistoryVersion();
		static void ClearHistory();

	private:
		static Shared<spdlog::logger> s_EngineLogger;
		static Shared<spdlog::logger> s_EditorLogger;
	};
}

// macros:
#define RV_ENGINE_ERROR(...)	::Revoke::Log::GetCoreLogger()->error(__VA_ARGS__)
#define RV_ENGINE_WARN(...)		::Revoke::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define RV_ENGINE_INFO(...)		::Revoke::Log::GetCoreLogger()->info(__VA_ARGS__)
#define RV_ENGINE_TRACE(...)	::Revoke::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define RV_ENGINE_FATAL(...)	::Revoke::Log::GetCoreLogger()->critical(__VA_ARGS__)

#define RV_EDITOR_ERROR(...)		::Revoke::Log::GetClientLogger()->error(__VA_ARGS__)
#define RV_EDITOR_WARN(...)			::Revoke::Log::GetClientLogger()->warn(__VA_ARGS__)
#define RV_EDITOR_INFO(...)			::Revoke::Log::GetClientLogger()->info(__VA_ARGS__)
#define RV_EDITOR_TRACE(...)		::Revoke::Log::GetClientLogger()->trace(__VA_ARGS__)
#define RV_EDITOR_FATAL(...)		::Revoke::Log::GetClientLogger()->critical(__VA_ARGS__)
