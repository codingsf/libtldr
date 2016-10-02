#ifndef TLDR_MODULE_HPP_
#define TLDR_MODULE_HPP_

#include <tldr/export.h>

#include <string>

namespace tldr {

typedef void * data_ptr_t;
typedef void (* fn_ptr_t)();

class TLDR_EXPORT Module
{
public:
	virtual ~Module();

	virtual fn_ptr_t get_raw_proc(const std::string & name) const = 0;
	virtual data_ptr_t get_raw_data(const std::string & name) const = 0;

	template <typename Fn>
	Fn * get_proc(const std::string & name) const;

	template <typename T>
	T * get_data(const std::string & name) const;
};

template <typename Fn>
Fn * Module::get_proc(const std::string & name) const
{
	return reinterpret_cast<Fn *>(get_raw_proc(name));
}

template <typename T>
T * Module::get_data(const std::string & name) const
{
	return static_cast<T *>(get_raw_data(name));
}

}

#endif
