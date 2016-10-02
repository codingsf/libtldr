#ifndef TLDR_TEST_LOADER_HPP_
#define TLDR_TEST_LOADER_HPP_

#include <gmock/gmock.h>

#include <tldr/loader.hpp>

class MockModuleResolver : public tldr::ModuleResolver
{
public:
	MOCK_CONST_METHOD1(get_module, std::shared_ptr<tldr::Module>(const std::string & name));
};

#endif
