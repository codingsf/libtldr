#include <config.h>
#include "hash.hpp"

#include <cstdint>

namespace tldr {

unsigned long elf_hash(const char * name)
{
	std::uint_fast32_t h = 0;
	while (*name) {
		unsigned long g;
		h = (h << 4) + static_cast<unsigned char>(*name++);
		if (g = h & 0xf0000000)
			h ^= g >> 24;
		h &= ~g;
	}
	return h;
}

unsigned long elf_gnu_hash (const char * name)
{
	std::uint_fast32_t h = 5381;
	while (*name)
		h = h * 33 + static_cast<unsigned char>(*name++);
	return h & 0xffffffff;
}


}
