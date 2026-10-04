#include "rvpch.h"
#include "UniversallyUniqueIdentifiers.h"

#include "MyRevoke/Core/UuidGenerator.h"

namespace Revoke
{
	UUID::UUID()
	{
		// All default-constructed UUIDs share one generator so the engine draws
		// from a single random sequence rather than reseeding per entity.
		static UuidGenerator s_DefaultGenerator;
		m_UUID = s_DefaultGenerator.Generate();
	}
	UUID::UUID(uint64_t uuid)
	{
		m_UUID = uuid;
	}
}
