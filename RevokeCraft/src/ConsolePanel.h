#pragma once

#include "MyRevoke/Core/Log.h"

#include <string>
#include <vector>

namespace Revoke
{
	// The engine's and the editor's log messages, with level filters and search.
	class ConsolePanel
	{
	public:
		void OnImGuiRender();

		// Picks up new messages. OnImGuiRender calls it; the status bar needs it even while the
		// console tab is hidden.
		void Refresh();
		// The newest message, or nullptr when there is none.
		const LogEntry* GetLatestEntry() const { return m_Entries.empty() ? nullptr : &m_Entries.back(); }

	private:
		std::vector<LogEntry> m_Entries;
		uint64_t m_SeenVersion = ~0ull;
		bool m_NewEntries = false;

		int m_InfoCount = 0;
		int m_WarningCount = 0;
		int m_ErrorCount = 0;

		std::string m_Filter;
		bool m_ShowInfo = true;
		bool m_ShowWarnings = true;
		bool m_ShowErrors = true;
		bool m_AutoScroll = true;
	};
}
