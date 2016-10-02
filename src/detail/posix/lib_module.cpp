#include <config.h>
#include <tldr/lib_module.hpp>

#include <dlfcn.h>

#include <cerrno>
#include <system_error>

namespace tldr {

LibModule::LibModule(module_handle_t handle)
	: handle_ { handle } {}

LibModule::LibModule(const std::string & name)
{
	if (!(handle_ = dlopen(name.c_str(), RTLD_LAZY)))
		throw std::runtime_error(dlerror());
}

LibModule::LibModule(LibModule && other) noexcept
	: handle_ { other.handle_ }
{
	other.handle_ = nullptr;
}

LibModule::~LibModule()
{
	if (handle_)
		dlclose(handle_);
}

data_ptr_t LibModule::get_raw_data(const std::string & name) const
{
	return dlsym(handle_, name.c_str());
}

fn_ptr_t LibModule::get_raw_proc(const std::string & name) const
{
	return reinterpret_cast<fn_ptr_t>(get_raw_data(name));
}

}
