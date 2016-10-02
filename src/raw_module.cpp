#include <config.h>
#include <tldr/raw_module.hpp>

#if defined(TLDR_HAS_ELF32_SUPPORT) || defined(TLDR_HAS_ELF64_SUPPORT)
#	include "elf/module.hpp"
#endif

#if defined(TLDR_HAS_PE32_SUPPORT) || defined(TLDR_HAS_PE64_SUPPORT)
#	include "pe/module.hpp"
#endif

namespace tldr {

std::shared_ptr<Module> load_from_memory(const void * mem, std::size_t size,
                                         const ModuleResolver & resolver)
{

#ifdef TLDR_HAS_ELF32_SUPPORT
	if (Elf32Module::is_valid(mem, size))
		return std::make_shared<Elf32Module>(mem, size, resolver);
#endif

#ifdef TLDR_HAS_ELF64_SUPPORT
	if (Elf64Module::is_valid(mem, size))
		return std::make_shared<Elf64Module>(mem, size, resolver);
#endif

#ifdef TLDR_HAS_PE32_SUPPORT
	if (Pe32Module::is_valid(mem, size))
		return std::make_shared<Pe32Module>(mem, size, resolver);
#endif

#ifdef TLDR_HAS_PE64_SUPPORT
	if (Pe64Module::is_valid(mem, size))
		return std::make_shared<Pe64Module>(mem, size, resolver);
#endif

	return nullptr;

}

}
