#include "rvpch.h"
#include "Log.h"


#include"spdlog/sinks/stdout_color_sinks.h"

namespace Revoke
{
	Shared<spdlog::logger> Log::s_EngineLogger;
	Shared<spdlog::logger> Log::s_EditorLogger;

	void Log::Init()
	{
		if (s_EngineLogger)
			return;

		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_EngineLogger = spdlog::stdout_color_mt("MYREVOKE");
		s_EngineLogger->set_level(spdlog::level::trace);

		s_EditorLogger = spdlog::stdout_color_mt("APP");
		s_EditorLogger->set_level(spdlog::level::trace);
	}

}

