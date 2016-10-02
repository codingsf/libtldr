#include <config.h>
#include <gtest/gtest.h>
#include <tldr/lib_module.hpp>

TEST(LibModuleTests, ConstructFromModuleNameWorks) {
	tldr::LibModule lib_module { TLDR_TEST_MODULE_PATH };
}

TEST(LibModuleTests, GetRawDataReturnsDataAddress) {
	tldr::LibModule lib_module { TLDR_TEST_MODULE_PATH };
	ASSERT_TRUE(lib_module.get_raw_data("foo_test_data") != nullptr);
}

TEST(LibModuleTests, GetRawDataGivesNullIfSymbolNotFound) {
	tldr::LibModule lib_module { TLDR_TEST_MODULE_PATH };
	ASSERT_TRUE(lib_module.get_raw_data("unknown") == nullptr);
}

TEST(LibModuleTests, GetDataGivesCorrectSymbolAddress) {
	tldr::LibModule lib_module { TLDR_TEST_MODULE_PATH };
	const auto foo_data = lib_module.get_data<int>("foo_test_data");
	ASSERT_EQ(*foo_data, 0x11223344);
}

TEST(LibModuleTests, GetRawProcReturnsProcAddress) {
	tldr::LibModule lib_module { TLDR_TEST_MODULE_PATH };
	ASSERT_TRUE(lib_module.get_raw_proc("foo_test_proc") != nullptr);
}

TEST(RawModuleTests, GetRawProcGivesNullIfSymbolNotFound) {
	tldr::LibModule lib_module { TLDR_TEST_MODULE_PATH };
	ASSERT_TRUE(lib_module.get_raw_proc("unknown") == nullptr);
}

TEST(LibModuleTests, GetProcGivesCorrectSymbolAddress) {
	tldr::LibModule lib_module { TLDR_TEST_MODULE_PATH };
	const auto foo_fn = lib_module.get_proc<int()>("foo_test_proc");
	ASSERT_EQ(foo_fn(), 0x11223344);
}
