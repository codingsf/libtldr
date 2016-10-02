#ifndef TLDR_LIBMODULE_HPP_
#define TLDR_LIBMODULE_HPP_

#include <tldr/types.hpp>
#include <tldr/module.hpp>

namespace tldr {

class LibModule : public Module
{
public:
	explicit LibModule(const std::string & name);
	explicit LibModule(module_handle_t handle);
	LibModule(LibModule && other) noexcept;
	virtual ~LibModule();

	virtual data_ptr_t get_raw_data(const std::string & name) const override;
	virtual fn_ptr_t get_raw_proc(const std::string & name) const override;

private:
	module_handle_t handle_;
};

}

#endif
