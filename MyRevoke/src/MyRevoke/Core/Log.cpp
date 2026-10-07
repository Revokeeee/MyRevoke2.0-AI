#include "rvpch.h"
#include "Log.h"


#include"spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/base_sink.h"

#include <atomic>
#include <ctime>
#include <deque>
#include <mutex>

namespace Revoke
{
	namespace
	{
		// Keeps the latest messages in memory so the editor can show them; the console window
		// is gone in Dist builds, and easy to miss in the others.
		class HistorySink : public spdlog::sinks::base_sink<std::mutex>
		{
		public:
			static constexpr size_t MaxEntries = 5000;

			std::vector<LogEntry> GetEntries()
			{
				std::lock_guard<std::mutex> lock(mutex_);
				return std::vector<LogEntry>(m_Entries.begin(), m_Entries.end());
			}

			void Clear()
			{
				std::lock_guard<std::mutex> lock(mutex_);
				m_Entries.clear();
				m_Version++;
			}

			uint64_t GetVersion() const { return m_Version.load(); }

		protected:
			// base_sink holds mutex_ while it calls this.
			void sink_it_(const spdlog::details::log_msg& message) override
			{
				LogEntry entry;
				entry.Level = message.level;
				entry.Source.assign(message.logger_name.data(), message.logger_name.size());
				entry.Message.assign(message.payload.data(), message.payload.size());

				std::time_t time = std::chrono::system_clock::to_time_t(message.time);
				std::tm localTime{};
				localtime_s(&localTime, &time);
				char buffer[16];
				std::strftime(buffer, sizeof(buffer), "%H:%M:%S", &localTime);
				entry.Time = buffer;

				m_Entries.push_back(std::move(entry));
				if (m_Entries.size() > MaxEntries)
					m_Entries.pop_front();
				m_Version++;
			}

			void flush_() override {}

		private:
			std::deque<LogEntry> m_Entries;
			std::atomic<uint64_t> m_Version{ 0 };
		};

		std::shared_ptr<HistorySink> s_History;
	}

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

		s_History = std::make_shared<HistorySink>();
		s_EngineLogger->sinks().push_back(s_History);
		s_EditorLogger->sinks().push_back(s_History);
	}

	std::vector<LogEntry> Log::GetHistory()
	{
		return s_History ? s_History->GetEntries() : std::vector<LogEntry>();
	}

	uint64_t Log::GetHistoryVersion()
	{
		return s_History ? s_History->GetVersion() : 0;
	}

	void Log::ClearHistory()
	{
		if (s_History)
			s_History->Clear();
	}

}

