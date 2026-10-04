#include <doctest/doctest.h>

#include <unordered_set>

#include "MyRevoke/Core/UuidGenerator.h"

using namespace Revoke;

TEST_CASE("Two generators with the same seed produce the same sequence")
{
	UuidGenerator first(0x1234567890ABCDEF);
	UuidGenerator second(0x1234567890ABCDEF);

	for (int i = 0; i < 16; i++)
		CHECK(first.Generate().Get() == second.Generate().Get());
}

TEST_CASE("Generators with different seeds produce different sequences")
{
	UuidGenerator first(1);
	UuidGenerator second(2);

	CHECK(first.Generate().Get() != second.Generate().Get());
}

TEST_CASE("A single generator hands out unique ids")
{
	UuidGenerator generator(42);

	std::unordered_set<uint64_t> ids;
	for (int i = 0; i < 1000; i++)
		ids.insert(generator.Generate().Get());

	CHECK(ids.size() == 1000);
}

TEST_CASE("Default-constructed UUIDs are unique")
{
	std::unordered_set<uint64_t> ids;
	for (int i = 0; i < 1000; i++)
		ids.insert(UUID().Get());

	CHECK(ids.size() == 1000);
}

TEST_CASE("A UUID built from an explicit value keeps that value")
{
	CHECK(UUID(7).Get() == 7);
}
