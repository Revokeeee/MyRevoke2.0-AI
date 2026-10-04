#include "rvpch.h"
#include "UuidGenerator.h"

namespace Revoke
{
	UuidGenerator::UuidGenerator()
		: m_Engine(std::random_device()())
	{
	}

	UuidGenerator::UuidGenerator(uint64_t seed)
		: m_Engine(seed)
	{
	}

	UUID UuidGenerator::Generate()
	{
		return UUID(m_Distribution(m_Engine));
	}
}
