#ifndef TLDR_SRC_VMEMORY_HPP_
#define TLDR_SRC_VMEMORY_HPP_

#include <cstddef>
#include <cstdint>

namespace tldr {

enum {
	MemAccessNone = 0,
	MemAccessRead = 1 << 0,
	MemAccessWrite = 1 << 1,
	MemAccessExecute = 1 << 2,
};

void * vmem_alloc(std::size_t size, std::uintptr_t pref_base = 0, int access = 3);
void vmem_protect(void * mem, std::size_t size, int new_access);
void vmem_free(void * mem, std::size_t size);

}

#endif
