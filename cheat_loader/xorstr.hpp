#pragma once
#include <string>
#include <utility>

namespace
{
	constexpr int const_atoi(char c)
	{
		return c - '0';
	}
}

template<typename _string_type, size_t _length>
class _basic_xorstr
{
	using value_type = typename _string_type::value_type;
	static constexpr auto _length_minus_one = _length - 1;

public:
	constexpr __forceinline _basic_xorstr(value_type const (&str)[_length])
		: _basic_xorstr(str, std::make_index_sequence<_length_minus_one>())
	{ }

	inline auto c_str() const
	{
		decrypt();
		return data;
	}

	inline auto str() const
	{
		decrypt();
		return _string_type(data, data + _length_minus_one);
	}

	inline operator _string_type() const
	{
		return str();
	}

private:
	template<size_t... indices>
	constexpr __forceinline _basic_xorstr(value_type const (&str)[_length], std::index_sequence<indices...>)
		: data{ crypt(str[indices], indices)..., '\0' },
		encrypted(true)
	{ }

	static constexpr auto xor_key = static_cast<value_type>(
		const_atoi(__TIME__[7]) +
		const_atoi(__TIME__[6]) * 10 +
		const_atoi(__TIME__[4]) * 60 +
		const_atoi(__TIME__[3]) * 600 +
		const_atoi(__TIME__[1]) * 3600 +
		const_atoi(__TIME__[0]) * 36000
	);

	static __forceinline constexpr auto crypt(value_type c, size_t i)
	{
		return static_cast<value_type>(c ^ (xor_key + i));
	}

	inline void decrypt() const
	{
		if (encrypted)
		{
			for (size_t t = 0; t < _length_minus_one; t++)
			{
				data[t] = crypt(data[t], t);
			}
			encrypted = false;
		}
	}

	mutable value_type data[_length];
	mutable bool encrypted;
};

template<size_t _length>
using xorstr_a = _basic_xorstr<std::string, _length>;
template<size_t _length>
using xorstr_w = _basic_xorstr<std::wstring, _length>;
template<size_t _length>
using xorstr_u16 = _basic_xorstr<std::u16string, _length>;
template<size_t _length>
using xorstr_u32 = _basic_xorstr<std::u32string, _length>;

template<typename _string_type, size_t _length, size_t _length2>
inline auto operator==(const _basic_xorstr<_string_type, _length> &lhs, const _basic_xorstr<_string_type, _length2> &rhs)
{
	static_assert(_length == _length2, "XorStr== different length");
	return _length == _length2 && lhs.str() == rhs.str();
}

template<typename _string_type, size_t _length>
inline auto operator==(const _string_type &lhs, const _basic_xorstr<_string_type, _length> &rhs)
{
	return lhs.size() == _length && lhs == rhs.str();
}

template<typename _stream_type, typename _string_type, size_t _length>
inline auto& operator<<(_stream_type &lhs, const _basic_xorstr<_string_type, _length> &rhs)
{
	lhs << rhs.c_str();

	return lhs;
}

template<typename _string_type, size_t _length, size_t _length2>
inline auto operator+(const _basic_xorstr<_string_type, _length> &lhs, const _basic_xorstr<_string_type, _length2> &rhs)
{
	return lhs.str() + rhs.str();
}

template<typename _string_type, size_t _length>
inline auto operator+(const _string_type &lhs, const _basic_xorstr<_string_type, _length> &rhs)
{
	return lhs + rhs.str();
}

template<size_t _length>
constexpr __forceinline auto _xor_(char const (&str)[_length])
{
	return xorstr_a<_length>(str);
}

template<size_t _length>
constexpr __forceinline auto _xor_(wchar_t const (&str)[_length])
{
	return xorstr_w<_length>(str);
}

template<size_t _length>
constexpr __forceinline auto _xor_(char16_t const (&str)[_length])
{
	return xorstr_u16<_length>(str);
}

template<size_t _length>
constexpr __forceinline auto _xor_(char32_t const (&str)[_length])
{
	return xorstr_u32<_length>(str);
}
