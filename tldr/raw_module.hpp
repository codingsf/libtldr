#ifndef TLDR_RAWMODULE_HPP_
#define TLDR_RAWMODULE_HPP_

#include <tldr/module.hpp>
#include <tldr/system_loader.hpp>

#include <cstddef>
#include <stdexcept>

namespace tldr {

class TLDR_EXPORT LoadError : public std::runtime_error
{
	using runtime_error::runtime_error;
};

TLDR_EXPORT
std::shared_ptr<Module> load_from_memory(const void * mem, std::size_t size,
                                         const ModuleResolver & resolver = system_loader);

}

#endif
