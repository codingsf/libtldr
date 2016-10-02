#ifndef TLDR_SRC_ENDIAN_HPP_
#define TLDR_SRC_ENDIAN_HPP_

#include <cassert>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <type_traits>
#include <utility>

namespace tldr {

template <typename T, typename = void>
struct impl_serializable_for;

template <typename T, typename = void>
struct impl_symmetric_serializable_for;

template <typename T, typename = void>
struct has_serializable_impl
	: public std::false_type {};

template <typename T>
struct has_serializable_impl<T, std::enable_if_t<
	(sizeof(impl_serializable_for<T>) > 0)
>> : public std::true_type {};

template <typename T>
constexpr bool has_serializable_impl_v = has_serializable_impl<T>();

template <typename T, typename = void>
struct has_symmetric_serializable_impl
	: public std::false_type {};

template <typename T>
struct has_symmetric_serializable_impl<T, std::enable_if_t<
	(sizeof(impl_symmetric_serializable_for<T>) > 0)
>> : public std::true_type {};

template <typename T>
constexpr bool has_symmetric_serializable_impl_v = has_symmetric_serializable_impl<T>();

template <typename T,
	typename = std::enable_if_t<has_serializable_impl_v<T>>
>
const void * le_read(const void * from_mem, std::size_t size, T & into_value);

template <typename T,
	typename = std::enable_if_t<has_serializable_impl_v<T>>
>
const void * be_read(const void * from_mem, std::size_t size, T & into_value);

template <typename T,
	typename = std::enable_if_t<has_serializable_impl_v<T>>
>
void * le_write(const T & value, void * into_mem, std::size_t size);

template <typename T,
	typename = std::enable_if_t<has_serializable_impl_v<T>>
>
void * be_write(const T & value, void * into_mem, std::size_t size);

namespace detail {

struct little_endian_encoding
{
	template <typename T>
	static const void * read(const void * from_mem, std::size_t size, T & into_value)
	{
		assert(size >= sizeof(T));
		return le_read(from_mem, size, into_value);
	}

	template <typename T>
	static void * write(const T & value, void * into_mem, std::size_t size)
	{
		assert(size >= sizeof(T));
		return le_write(value, into_mem, size);
	}
};

struct big_endian_encoding
{
	template <typename T>
	static const void * read(const void * from_mem, std::size_t size, T & into_value)
	{
		assert(size >= sizeof(T));
		return be_read(from_mem, size, into_value);
	}

	template <typename T>
	static void * write(const T & value, void * into_mem, std::size_t size)
	{
		assert(size >= sizeof(T));
		return be_write(value, into_mem, size);
	}
};

}

template <typename T, typename>
const void * le_read(const void * from_mem, std::size_t size, T & into_value)
{
	using encoding = detail::little_endian_encoding;
	const auto from_buf = static_cast<const char *>(from_mem);
	if (size < sizeof(T)) throw std::out_of_range("size < sizeof(T)");
	return impl_serializable_for<T>::template read<encoding>(from_buf, size, into_value);
}

template <typename T, typename>
const void * be_read(const void * from_mem, std::size_t size, T & into_value)
{
	using encoding = detail::big_endian_encoding;
	const auto from_buf = static_cast<const char *>(from_mem);
	if (size < sizeof(T)) throw std::out_of_range("size < sizeof(T)");
	return impl_serializable_for<T>::template read<encoding>(from_buf, size, into_value);
}

template <typename T, typename>
void * le_write(const T & value, void * into_mem, std::size_t size)
{
	using encoding = detail::little_endian_encoding;
	const auto into_buf = static_cast<char *>(into_mem);
	if (size < sizeof(T)) throw std::out_of_range("size < sizeof(T)");
	return impl_serializable_for<T>::template write<encoding>(value, into_buf, size);
}

template <typename T, typename>
void * be_write(const T & value, void * into_mem, std::size_t size)
{
	using encoding = detail::big_endian_encoding;
	const auto into_buf = static_cast<char *>(into_mem);
	if (size < sizeof(T)) throw std::out_of_range("size < sizeof(T)");
	return impl_serializable_for<T>::template write<encoding>(value, into_buf, size);
}

namespace detail {

template <typename T, class encoding>
struct integral_io_helper;

template <typename T>
struct integral_io_helper<T, little_endian_encoding>
{
	static const void * read(const char * from_buf, std::size_t size, T & into_value)
	{
		into_value = 0;
		assert(size >= sizeof(T));
		for (std::size_t i = 0; i < sizeof(T); ++i)
			into_value |= T(from_buf[i] & UCHAR_MAX) << i * CHAR_BIT;
		return from_buf + sizeof(T);
	}

	static void * write(const T & value, char * into_buf, std::size_t size)
	{
		assert(size >= sizeof(T));
		for (std::size_t i = 0; i < sizeof(T); ++i) {
			const auto rshift = i * CHAR_BIT;
			into_buf[i] = (value >> rshift) & UCHAR_MAX;
		}
		return into_buf + sizeof(T);
	}
};

template <typename T>
struct integral_io_helper<T, big_endian_encoding>
{
	static const void * read(const char * from_buf, std::size_t size, T & into_value)
	{
		into_value = 0;
		assert(size >= sizeof(T));
		for (std::size_t i = 0; i < sizeof(T); ++i) {
			into_value <<= CHAR_BIT;
			into_value |= from_buf[i] & UCHAR_MAX;
		}
		return from_buf + sizeof(T);
	}

	static void * write(const T & value, char * into_buf, std::size_t size)
	{
		assert(size >= sizeof(T));
		for (std::size_t i = 0; i < sizeof(T); ++i) {
			const auto rshift = (sizeof(T) - i - 1) * CHAR_BIT;
			into_buf[i] = (value >> rshift) & UCHAR_MAX;
		}
		return into_buf + sizeof(T);
	}
};

}

template <typename T>
struct impl_serializable_for<T,
	std::enable_if_t<std::is_integral<T>::value>
>
{
	template <class encoding>
	static const void * read(const char * from_buf, std::size_t size, T & into_value)
	{
		assert(size >= sizeof(T));
		return detail::integral_io_helper<T, encoding>::read(from_buf, size, into_value);
	}

	template <class encoding>
	static void * write(const T & value, char * into_buf, std::size_t size)
	{
		assert(size >= sizeof(T));
		return detail::integral_io_helper<T, encoding>::write(value, into_buf, size);
	}
};

namespace detail {

template <class encoding, typename T,
	typename = std::enable_if_t<has_serializable_impl_v<T>>
>
class ro_serializable_ref
{
public:
	explicit ro_serializable_ref(const T & value, std::size_t offset)
		: value_ { value }, offset_ { offset } {}

	template <typename R>
	ro_serializable_ref<encoding, R> rebind() const
	{
		return ro_serializable_ref<encoding, R>((R const &)value_, offset_);
	}

	void * transfer(char * into_buf, std::size_t size) const
	{
		assert(size >= sizeof(T));
		return encoding::write(value_, into_buf + offset_, size - offset_);
	}

private:
	const T & value_;
	std::size_t const offset_;
};

template <class encoding, typename T,
	typename = std::enable_if_t<has_serializable_impl_v<T>>
>
class rw_serializable_ref
{
public:
	explicit rw_serializable_ref(T & value, std::size_t offset)
		: value_ { value }, offset_ { offset } {}

	template <typename R>
	rw_serializable_ref<encoding, R> rebind()
	{
		return rw_serializable_ref<encoding, R>((R &)value_, offset_);
	}

	template <typename R>
	ro_serializable_ref<encoding, R> rebind() const
	{
		return ro_serializable_ref<encoding, R>((R const &)value_, offset_);
	}

	void * transfer(char * into_buf, std::size_t size) const
	{
		assert(size >= sizeof(T));
		return encoding::write(value_, into_buf + offset_, size - offset_);
	}

	const void * transfer(const char * from_buf, std::size_t size)
	{
		assert(size >= sizeof(T));
		return encoding::read(from_buf + offset_, size - offset_, value_);
	}

private:
	T & value_;
	std::size_t const offset_;
};

template <class encoding>
class io_services
{
public:
	using le = io_services<detail::little_endian_encoding>;
	using be = io_services<detail::big_endian_encoding>;

	template <typename T,
		typename = std::enable_if_t<has_serializable_impl_v<T>>
	>
	static ro_serializable_ref<encoding, T> ref(const T & value, std::size_t offset = 0)
	{
		return ro_serializable_ref<encoding, T>(value, offset);
	}

	template <typename T,
		typename = std::enable_if_t<has_serializable_impl_v<T>>
	>
	static rw_serializable_ref<encoding, T> ref(T & value, std::size_t offset = 0)
	{
		return rw_serializable_ref<encoding, T>(value, offset);
	}
};

}

template <typename T>
struct impl_serializable_for<T,
	std::enable_if_t<has_symmetric_serializable_impl_v<T>>
>
{
	template <typename encoding>
	static const void * read(const char * from_buf, std::size_t size, T & into_value)
	{
		assert(size >= sizeof(T));
		using io_services = detail::io_services<encoding>;
		impl_symmetric_serializable_for<T>::template transfer<io_services>(into_value, from_buf, size);
		return from_buf + sizeof(T);
	}

	template <typename encoding>
	static void * write(const T & value, char * into_buf, std::size_t size)
	{
		assert(size >= sizeof(T));
		using io_services = detail::io_services<encoding>;
		impl_symmetric_serializable_for<T>::template transfer<io_services>(value, into_buf, size);
		return into_buf + sizeof(T);
	}
};

template <typename T>
struct impl_symmetric_serializable_for<T,
	std::enable_if_t<std::is_enum<T>::value>
>
{
	template <class io, class io_ref, class io_ptr>
	static auto transfer(io_ref && value, io_ptr mem, std::size_t size)
	{
		assert(size >= sizeof(T));
		using value_type = std::underlying_type_t<T>;
		return io::ref(value).template rebind<value_type>().transfer(mem, size);
	}
};

template <typename T, std::size_t N>
struct impl_symmetric_serializable_for<T[N],
	std::enable_if_t<has_serializable_impl_v<T>>
>
{
	template <class io, class io_ref, class io_ptr>
	static auto transfer(io_ref && value, io_ptr mem, std::size_t size)
	{
		assert(size >= N * sizeof(T));
		for (std::size_t i = 0; i < N; ++i)
			io::ref(value[i], i * sizeof(T)).transfer(mem, size);
		return mem + N * sizeof(T);
	}
};

}

#endif
