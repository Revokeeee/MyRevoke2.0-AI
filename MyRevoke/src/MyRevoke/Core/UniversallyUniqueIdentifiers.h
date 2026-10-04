#pragma once

#include <xhash>

namespace Revoke
{
	class UUID
	{
	public:
		UUID();
		UUID(uint64_t uuid);
		UUID(const UUID&) = default;

		uint64_t Get() const { return m_UUID; }

		operator uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};
}
namespace std {

	template<>
	struct hash<Revoke::UUID>
	{
		std::size_t operator()(Revoke::UUID id) const
		{
			uint64_t uuid = id.Get();
			return hash<uint64_t>()(uuid);
		}
	};

}