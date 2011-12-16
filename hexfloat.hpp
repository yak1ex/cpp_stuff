#ifndef YAK_UTIL_HEXFLOAT_HPP
#define YAK_UTIL_HEXFLOAT_HPP

#include <utility>

// CAUTION: THIS CODE IS FOR GCC ONLY BECAUSE IT DEPENDS ON __builtin_scalbn()

struct invalid_format {};

namespace yak {
namespace util {
namespace hexfloat {

namespace detail {

constexpr int hex(char c)
{
    return 
        ('0' <= c && c <= '9') ? c - '0' : 
        ('a' <= c && c <= 'f') ? c - 'a' + 10 :
        ('A' <= c && c <= 'F') ? c - 'A' + 10 :
	throw invalid_format{};
}

template<char ... c>
constexpr
typename std::enable_if<sizeof...(c) == 0, double>::type
parse_impl_3rdnum(bool positive, double value, int exp1, int exp2)
{
// CAUTION: GCC ONLY BECAUSE THIS DEPENDS ON __builtin_scalbn()
    return __builtin_scalbn(value, positive ? (-exp1 + exp2) : (-exp1 - exp2));
}

template<char c0, char ... c>
constexpr double parse_impl_3rdnum(bool positive, double value, int exp1, int exp2) 
{
    return ('0' <= c0 && c0 <= '9') ?
        parse_impl_3rdnum<c...>(positive, value, exp1, exp2 * 10 + c0 - '0') :
        throw invalid_format{};
}

template<char c0, char ... c>
constexpr double parse_impl_p(double value, int exp1)
{
    return
        c0 == '+' ? parse_impl_3rdnum<c...>(true, value, exp1, 0) :
        c0 == '-' ? parse_impl_3rdnum<c...>(false, value, exp1, 0) :
        parse_impl_3rdnum<c...>(true, value, exp1, c0 - '0');
}

template<char c0, char ... c>
constexpr
typename std::enable_if<c0 == 'p' || c0 == 'P', double>::type
parse_impl_2ndnum(double value, int exp1)
{
    return parse_impl_p<c...>(value, exp1);
}

template<char c0, char ... c>
constexpr
typename std::enable_if<c0 != 'p' && c0 != 'P', double>::type
parse_impl_2ndnum(double value, int exp1)
{
    return parse_impl_2ndnum<c...>(value * 16 + hex(c0), exp1 + 4);
}

template<char c0, char ... c>
constexpr double parse_impl_period(double value)
{
    return (c0 == 'p' || c0 == 'P') ? parse_impl_p<c...>(value, 0) : parse_impl_2ndnum<c0, c...>(value, 0);
}

template<char c0, char ... c>
constexpr
typename std::enable_if<c0 == '.' || c0 == 'p' || c0 == 'P', double>::type
parse_impl_1stnum(double value)
{
    return
        c0 == '.' ? parse_impl_period<c...>(value) :
        (c0 == 'p' || c0 == 'P') ? parse_impl_p<c...>(value, 0) : throw invalid_format{};
}

template<char c0, char ... c>
constexpr
typename std::enable_if<c0 != '.' && c0 != 'p' && c0 != 'P', double>::type
parse_impl_1stnum(double value)
{
    return parse_impl_1stnum<c...>(value * 16 + hex(c0));
}

template<char c0, char ... c>
constexpr double parse_impl_x(double value)
{
    return (c0 == 'x' || c0 == 'X') ? parse_impl_1stnum<c...>(value) : throw invalid_format{};
}

template<char c0, char ... c>
constexpr double parse_impl_init(double value)
{
    return c0 == '0' ? parse_impl_x<c...>(value) : throw invalid_format{};
}

} // namespace detail

#if __GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 7

template<char ... c>
constexpr double operator "" _hf()
{
    return detail::parse_impl_init<c...>(0);
}

#else

template<char ... c>
constexpr double parse()
{
    return detail::parse_impl_init<c...>(0);
}

#endif

} // namespace hexfloat
} // namespace util
} // namespace yak

#endif
