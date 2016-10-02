#include <config.h>
#include "../../vmemory.hpp"

#include <sys/mman.h>

#include <cerrno>
#include <system_error>

namespace tldr {

namespace {

int memory_access_flags(int access)
{
	int flags = PROT_NONE;
	if (access & MemAccessRead) flags |= PROT_READ;
	if (access & MemAccessWrite) flags |= PROT_WRITE;
	if (access & MemAccessExecute) flags |= PROT_EXEC;
	return flags;
}

}

void * vmem_alloc(std::size_t size, std::uintptr_t pref_base, int access)
{
	const auto flags = MAP_PRIVATE | MAP_ANONYMOUS;
	const auto access = memory_access_flags(access);
	const auto mbase = reinterpret_cast<void *>(pref_base);
	void * const mem = mmap(mbase, size, access, flags, -1, 0);
	if (!mem)
		throw std::system_error(errno, std::system_category());
	return mem;
}

void vmem_protect(void * mem, std::size_t size, int new_access)
{
	const int access = memory_access_flags(new_access);
	if (mprotect(mem, size, access) == -1)
		throw std::system_error(errno, std::system_category());
}

void vmem_free(void * mem, std::size_t size)
{
	if (munmap(mem, size) == -1)
		throw std::system_error(errno, std::system_category());
}

}
