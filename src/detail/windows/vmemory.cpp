#include <config.h>
#include "../../vmemory.hpp"

#include <windows.h>

#include <system_error>

namespace tldr {

namespace {

DWORD memory_access_flags(int access)
{
	if (access & MemAccessRead) {
		if (access & MemAccessWrite) {
			if (access & MemAccessExecute)
				return PAGE_EXECUTE_READWRITE;
			else
				return PAGE_READWRITE;
		} else {
			if (access & MemAccessExecute)
				return PAGE_EXECUTE_READ;
			else
				return PAGE_READONLY;
		}
	} else {
		return PAGE_NOACCESS;
	}
}

}

void * vmem_alloc(std::size_t size, std::uintptr_t pref_base, int access)
{
	const auto flags = MEM_RESERVE | MEM_COMMIT;
	const auto access = memory_access_flags(access);
	const auto mbase = reinterpret_cast<void *>(pref_base);
	void * mem = VirtualAlloc(mbase, size, flags, access);
	if (!mem && pref_base)
		mem = VirtualAlloc(nullptr, size, flags, access);
	if (!mem)
		throw std::system_error(GetLastError(), std::system_category());
	return mem;
}

void vmem_protect(void * mem, std::size_t size, int new_access)
{
	DWORD old_access;
	const DWORD access = memory_access_flags(new_access);
	if (!VirtualProtect(mem, size, mprot, &old_access))
		throw std::system_error(GetLastError(), std::system_category());
}

void vmem_free(void * mem, std::size_t size)
{
	if (!VirtualFree(mem, 0, MEM_RELEASE | MEM_DECOMMIT))
		throw std::system_error(GetLastError(), std::system_category());
}

}
