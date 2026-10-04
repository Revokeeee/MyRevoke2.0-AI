#pragma once

#include <random>

#include "MyRevoke/Core/UniversallyUniqueIdentifiers.h"

namespace Revoke
{
	class UuidGenerator
	{
	public:
		UuidGenerator();
		explicit UuidGenerator(uint64_t seed);

		UUID Generate();
	private:
		std::mt19937_64 m_Engine;
		std::uniform_int_distribution<uint64_t> m_Distribution;
	};
}
