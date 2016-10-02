#include <config.h>
#include <tldr/system_loader.hpp>

#include <tldr/lib_module.hpp>

#include <system_error>

namespace tldr {

const SystemLoader system_loader;

std::shared_ptr<Module> SystemLoader::get_module(const std::string & name) const
{
	try {
		return std::make_shared<LibModule>(name);
	} catch (std::system_error & e) {
		return nullptr;
	}
}

}
