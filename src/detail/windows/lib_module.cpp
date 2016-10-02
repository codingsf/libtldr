#include <config.h>
#include <tldr/lib_module.hpp>

#include <windows.h>

#include <system_error>

namespace tldr {

LibModule::LibModule(module_handle_t handle)
	: handle_ { handle } {}

LibModule::LibModule(const std::string & name)
{
	if (!(handle_ = LoadLibraryA(name.c_str())))
		throw std::system_error(GetLastError(), std::system_category());
}

LibModule::LibModule(LibModule && other) noexcept
	: handle_ { other.handle_ }
{
	other.handle_ = nullptr;
}

LibModule::~LibModule()
{
	if (handle_)
		FreeLibrary(handle_);
}

data_ptr_t LibModule::get_raw_data(const std::string & name) const
{
	return GetProcAddress(handle_, name.c_str());
}

fn_ptr_t LibModule::get_raw_proc(const std::string & name) const
{
	return reinterpret_cast<fn_ptr_t>(get_raw_data(name)));
}

}
