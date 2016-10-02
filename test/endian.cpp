#include <gtest/gtest.h>
#include "../src/endian.hpp"

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>

TEST(EndianTests, LeReadThrowsIfSizeIsLessThanRequired) {
	ASSERT_THROW({
		std::uint32_t value;
		const char buf[] = "\x01";
		tldr::le_read(buf, sizeof(buf), value);
	}, std::out_of_range);
}

TEST(EndianTests, BeReadThrowsIfSizeIsLessThanRequired) {
	ASSERT_THROW({
		std::uint32_t value;
		const char buf[] = "\x01";
		tldr::be_read(buf, sizeof(buf), value);
	}, std::out_of_range);
}

TEST(EndianTests, LeWriteThrowsIfSizeIsLessThanRequired) {
	ASSERT_THROW({
		char buf[sizeof(std::uint32_t)-1];
		const std::uint32_t value = 0x11223344;
		tldr::le_write(value, buf, sizeof(buf));
	}, std::out_of_range);
}

TEST(EndianTests, BeWriteThrowsIfSizeIsLessThanRequired) {
	ASSERT_THROW({
		char buf[sizeof(std::uint32_t)-1];
		const std::uint32_t value = 0x11223344;
		tldr::be_write(value, buf, sizeof(buf));
	}, std::out_of_range);
}

TEST(EndianTests, LeReadU8GivesExpectedValue) {
	std::uint8_t value;
	const char buf[] = "\xf0";
	tldr::le_read(buf, sizeof(buf), value);
	ASSERT_EQ(value, 0xf0);
}

TEST(EndianTests, BeReadU8GivesExpectedValue) {
	std::uint8_t value;
	const char buf[] = "\xf0";
	tldr::be_read(buf, sizeof(buf), value);
	ASSERT_EQ(value, 0xf0);
}

TEST(EndianTests, LeWriteU8GivesExpectedValue) {
	const std::uint8_t value = 0xf0;
	char buf[sizeof(std::uint8_t) + 1] = {};
	tldr::le_write(value, buf, sizeof(buf));
	ASSERT_STREQ(buf, "\xf0");
}

TEST(EndianTests, BeWriteU8GivesExpectedValue) {
	const std::uint8_t value = 0xf0;
	char buf[sizeof(std::uint8_t) + 1] = {};
	tldr::be_write(value, buf, sizeof(buf));
	ASSERT_STREQ(buf, "\xf0");
}

TEST(EndianTests, LeReadU16GivesExpectedValue) {
	std::uint16_t value;
	const char buf[] = "\xaa\xbb";
	tldr::le_read(buf, sizeof(buf), value);
	ASSERT_EQ(value, 0xbbaa);
}

TEST(EndianTests, BeReadU16GivesExpectedValue) {
	std::uint16_t value;
	const char buf[] = "\xaa\xbb";
	tldr::be_read(buf, sizeof(buf), value);
	ASSERT_EQ(value, 0xaabb);
}

TEST(EndianTests, LeWriteU16GivesExpectedValue) {
	const std::uint16_t value = 0xaabb;
	char buf[sizeof(std::uint16_t) + 1] = {};
	tldr::le_write(value, buf, sizeof(buf));
	ASSERT_STREQ(buf, "\xbb\xaa");
}

TEST(EndianTests, BeWriteU16GivesExpectedValue) {
	const std::uint16_t value = 0xaabb;
	char buf[sizeof(std::uint16_t) + 1] = {};
	tldr::be_write(value, buf, sizeof(buf));
	ASSERT_STREQ(buf, "\xaa\xbb");
}

TEST(EndianTests, LeReadU32GivesExpectedValue) {
	std::uint32_t value;
	const char buf[] = "\xaa\xbb\xcc\xdd";
	tldr::le_read(buf, sizeof(buf), value);
	ASSERT_EQ(value, 0xddccbbaa);
}

TEST(EndianTests, BeReadU32GivesExpectedValue) {
	std::uint32_t value;
	const char buf[] = "\xaa\xbb\xcc\xdd";
	tldr::be_read(buf, sizeof(buf), value);
	ASSERT_EQ(value, 0xaabbccdd);
}

TEST(EndianTests, LeWriteU32GivesExpectedValue) {
	const std::uint32_t value = 0xaabbccdd;
	char buf[sizeof(std::uint32_t) + 1] = {};
	tldr::le_write(value, buf, sizeof(buf));
	ASSERT_STREQ(buf, "\xdd\xcc\xbb\xaa");
}

TEST(EndianTests, BeWriteU32GivesExpectedValue) {
	const std::uint32_t value = 0xaabbccdd;
	char buf[sizeof(std::uint32_t) + 1] = {};
	tldr::be_write(value, buf, sizeof(buf));
	ASSERT_STREQ(buf, "\xaa\xbb\xcc\xdd");
}

TEST(EndianTests, LeReadU64GivesExpectedValue) {
	std::uint64_t value;
	const char buf[] = "\x11\x22\x33\x44"
	                   "\x55\x66\x77\x88";
	tldr::le_read(buf, sizeof(buf), value);
	ASSERT_EQ(value, 0x8877665544332211);
}

TEST(EndianTests, BeReadU64GivesExpectedValue) {
	std::uint64_t value;
	const char buf[] = "\x11\x22\x33\x44"
	                   "\x55\x66\x77\x88";
	tldr::be_read(buf, sizeof(buf), value);
	ASSERT_EQ(value, 0x1122334455667788);
}

TEST(EndianTests, LeWriteU64GivesExpectedValue) {
	const std::uint64_t value = 0x1122334455667788;
	char buf[sizeof(std::uint64_t) + 1] = {};
	tldr::le_write(value, buf, sizeof(buf));
	ASSERT_STREQ(buf, "\x88\x77\x66\x55"
	                  "\x44\x33\x22\x11");
}

TEST(EndianTests, BeWriteU64GivesExpectedValue) {
	const std::uint64_t value = 0x1122334455667788;
	char buf[sizeof(std::uint64_t) + 1] = {};
	tldr::be_write(value, buf, sizeof(buf));
	ASSERT_STREQ(buf, "\x11\x22\x33\x44"
	                  "\x55\x66\x77\x88");
}

TEST(EndianTests, LeReadEnumGivesExpectedValue) {
	enum class TestEnum : char { Expected };
	const char buf[] = {
		static_cast<char>(TestEnum::Expected)
	};
	TestEnum value;
	tldr::le_read(buf, sizeof(buf), value);
	ASSERT_EQ(value, TestEnum::Expected);
}

TEST(EndianTests, BeReadEnumGivesExpectedValue) {
	enum class TestEnum : char { Expected };
	const char buf[] = {
		static_cast<char>(TestEnum::Expected)
	};
	TestEnum value;
	tldr::be_read(buf, sizeof(buf), value);
	ASSERT_EQ(value, TestEnum::Expected);
}

struct FooRecord {
	std::uint64_t a;
	std::uint32_t b;
	std::uint16_t c;
	std::uint8_t  d;
};

constexpr bool operator==(const FooRecord & lhs, const FooRecord & rhs)
{
	return lhs.a == rhs.a && lhs.b == rhs.b
	    && lhs.c == rhs.c && lhs.d == rhs.d;
}

namespace tldr {

template <>
struct impl_serializable_for<FooRecord>
{
	template <class encoding>
	static void const * read(const char * from_buf, std::size_t size, FooRecord & record)
	{
		encoding::read(from_buf + offsetof(FooRecord, a), size - offsetof(FooRecord, a), record.a);
		encoding::read(from_buf + offsetof(FooRecord, b), size - offsetof(FooRecord, b), record.b);
		encoding::read(from_buf + offsetof(FooRecord, c), size - offsetof(FooRecord, c), record.c);
		encoding::read(from_buf + offsetof(FooRecord, d), size - offsetof(FooRecord, d), record.d);
		return from_buf + sizeof(FooRecord);
	}
};

}

TEST(EndianTests, LeReadSerializableRecordGivesExpectedValue) {
	const FooRecord expected {
		0x8877665544332211, 0x44332211, 0x2211, 0x11
	};
	FooRecord value;
	const char buf[] = "\x11\x22\x33\x44\x55\x66\x77\x88"
	                   "\x11\x22\x33\x44\x11\x22\x11\x00";
	tldr::le_read(buf, sizeof(buf), value);
	ASSERT_EQ(value, expected);
}

TEST(EndianTests, BeReadSerializableRecordGivesExpectedValue) {
	const FooRecord expected {
		0x1122334455667788, 0x11223344, 0x1122, 0x11
	};
	FooRecord value;
	const char buf[] = "\x11\x22\x33\x44\x55\x66\x77\x88"
	                   "\x11\x22\x33\x44\x11\x22\x11\x00";
	tldr::be_read(buf, sizeof(buf), value);
	ASSERT_EQ(value, expected);
}

struct BarRecord {
	FooRecord foo;
};

constexpr bool operator==(const BarRecord & lhs, const BarRecord & rhs)
{
	return lhs.foo == rhs.foo;
}

namespace tldr {

template <>
struct impl_symmetric_serializable_for<BarRecord>
{
	template <class io, class io_ref, class io_ptr>
	static auto transfer(io_ref && bar, io_ptr mem, std::size_t size)
	{
		io::ref(bar.foo, offsetof(BarRecord, foo)).transfer(mem, size);
		return mem + sizeof(BarRecord);
	}
};

}

TEST(EndianTests, LeReadSymmetricSerializableGivesExpectedValue) {
	const BarRecord expected {
		{ 0x8877665544332211, 0x44332211, 0x2211, 0x11 }
	};
	BarRecord value;
	const char buf[] = "\x11\x22\x33\x44\x55\x66\x77\x88"
	                   "\x11\x22\x33\x44\x11\x22\x11\x00";
	tldr::le_read(buf, sizeof(buf), value);
	ASSERT_EQ(value, expected);
}

TEST(EndianTests, BeReadSymmetricSerializableGivesExpectedValue) {
	const BarRecord expected {
		{ 0x1122334455667788, 0x11223344, 0x1122, 0x11 }
	};
	BarRecord value;
	const char buf[] = "\x11\x22\x33\x44\x55\x66\x77\x88"
	                   "\x11\x22\x33\x44\x11\x22\x11\x00";
	tldr::be_read(buf, sizeof(buf), value);
	ASSERT_EQ(value, expected);
}
