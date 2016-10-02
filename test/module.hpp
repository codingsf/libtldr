#ifndef TLDR_TEST_MODULE_HPP_
#define TLDR_TEST_MODULE_HPP_

#include <gmock/gmock.h>

#include <tldr/module.hpp>

class MockModule : public tldr::Module {
public:
	MOCK_CONST_METHOD1(get_raw_data, tldr::data_ptr_t(const std::string & name));
	MOCK_CONST_METHOD1(get_raw_proc, tldr::fn_ptr_t(const std::string & name));
};

#endif
