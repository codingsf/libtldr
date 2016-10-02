#include <config.h>
#include <gtest/gtest.h>
#include <tldr/module.hpp>
#include <tldr/raw_module.hpp>

#include <algorithm>
#include <iterator>
#include <fstream>
#include <vector>

class RawModuleTests : public testing::Test
{
public:
	RawModuleTests();

public:
	std::vector<char> module_data_;
};

RawModuleTests::RawModuleTests()
{
	std::ifstream ifs { TLDR_TEST_MODULE_PATH, std::ios::binary };
	ifs.unsetf(std::ios::skipws);
	std::copy(std::istream_iterator<char>(ifs),
	          std::istream_iterator<char>(),
	          std::back_inserter(module_data_));
}

TEST_F(RawModuleTests, LoadFromMemoryWorks) {
	ASSERT_TRUE(tldr::load_from_memory(module_data_.data(),
	                                   module_data_.size()) != nullptr);
}

TEST_F(RawModuleTests, GetRawDataGivesCorrectDataAddress) {
	const auto module = tldr::load_from_memory(module_data_.data(),
	                                           module_data_.size());
	ASSERT_TRUE(module->get_raw_data("foo_test_data") != nullptr);
}

TEST_F(RawModuleTests, GetRawDataGivesNullIfSymbolNotFound) {
	const auto module = tldr::load_from_memory(module_data_.data(),
	                                           module_data_.size());
	ASSERT_TRUE(module->get_raw_data("unknown") == nullptr);
}

TEST_F(RawModuleTests, GetDataGivesCorrectSymbolAddress) {
	const auto module = tldr::load_from_memory(module_data_.data(),
	                                           module_data_.size());
	const auto foo_data = module->get_data<int>("foo_test_data");
	ASSERT_EQ(*foo_data, 0x11223344);
}

TEST_F(RawModuleTests, GetRawProcGivesCorrectProcAddress) {
	const auto module = tldr::load_from_memory(module_data_.data(),
	                                           module_data_.size());
	ASSERT_TRUE(module->get_raw_proc("foo_test_proc") != nullptr);
}

TEST_F(RawModuleTests, GetRawProcGivesNullIfSymbolNotFound) {
	const auto module = tldr::load_from_memory(module_data_.data(),
	                                           module_data_.size());
	ASSERT_TRUE(module->get_raw_proc("unknown") == nullptr);
}

TEST_F(RawModuleTests, GetProcGivesCorrectSymbolAddress) {
	const auto module = tldr::load_from_memory(module_data_.data(),
	                                           module_data_.size());
	const auto foo_fn = module->get_proc<int()>("foo_test_proc");
	ASSERT_EQ(foo_fn(), 0x11223344);
}

TEST_F(RawModuleTests, LoadFromMemoryResolvesAllImports) {
	const auto module = tldr::load_from_memory(module_data_.data(),
	                                           module_data_.size());
	const auto foo_fn = module->get_proc<void()>("foo_test_imports");
	EXPECT_TRUE(foo_fn != nullptr);
	foo_fn();
}
