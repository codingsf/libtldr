#include <gtest/gtest.h>
#include <tldr/loader.hpp>

#include "loader.hpp"
#include "module.hpp"

#include <memory>

struct LoaderTests : testing::Test {
	tldr::Loader _loader;
};

TEST_F(LoaderTests, GetModuleUsesResolverIfModuleNotSet) {
	const MockModuleResolver mock_resolver;
	_loader.set_module_resolver(&mock_resolver);
	EXPECT_CALL(mock_resolver, get_module("foo")).Times(1);
	_loader.get_module("foo");
}

TEST_F(LoaderTests, GetModuleReturnsNullIfModuleAndResolverNotSet) {
	ASSERT_TRUE(_loader.get_module("foo") == nullptr);
}

TEST_F(LoaderTests, GetModuleReturnsModuleIfModuleIsSet) {
	const auto module_ptr = std::make_shared<MockModule>();
	_loader.set_module("foo", module_ptr);
	ASSERT_EQ(_loader.get_module("foo"), module_ptr);
}

TEST_F(LoaderTests, GetModuleReturnsNullIfModuleUnloaded) {
	auto module_ptr = std::make_shared<MockModule>();
	_loader.set_module("foo", module_ptr);
	module_ptr.reset();
	ASSERT_TRUE(_loader.get_module("foo") == nullptr);
}

TEST_F(LoaderTests, GetModuleUsesResolverIfModuleUnloaded) {
	const MockModuleResolver mock_resolver;
	_loader.set_module_resolver(&mock_resolver);
	auto module_ptr = std::make_shared<MockModule>();
	_loader.set_module("foo", module_ptr);
	module_ptr.reset();
	EXPECT_CALL(mock_resolver, get_module("foo")).Times(1);
	_loader.get_module("foo");
}

TEST_F(LoaderTests, SetModuleDoesNotIncreaseModuleRefCount) {
	const auto module_ptr = std::make_shared<MockModule>();
	_loader.set_module("foo", module_ptr);
	ASSERT_EQ(module_ptr.use_count(), 1);
}

TEST_F(LoaderTests, SetModuleReplacesModuleIfAlreadySet) {
	const auto module_a = std::make_shared<MockModule>();
	const auto module_b = std::make_shared<MockModule>();
	_loader.set_module("foo", module_a);
	_loader.set_module("foo", module_b);
	ASSERT_EQ(_loader.get_module("foo"), module_b);
}

TEST_F(LoaderTests, RemoveModuleRemovesModuleFromLoader) {
	const auto module_ptr = std::make_shared<MockModule>();
	_loader.set_module("foo", module_ptr);
	_loader.remove_module("foo");
	ASSERT_TRUE(_loader.get_module("foo") == nullptr);
}
