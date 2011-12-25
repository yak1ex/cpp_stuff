#ifndef YAK_UTIL_CONSTEXPR_HPP
#define YAK_UTIL_CONSTEXPR_HPP

namespace yak {
namespace util {

namespace detail {

template<typename T>
constexpr T max_imp(T t)
{
	return t;
}

template<typename T0, typename T1>
constexpr auto max_imp(T0 t0, T1 t1) -> typename std::common_type<T0, T1>::type
{
	return t0 > t1 ? t0 : t1;
}

template<typename T0, typename T1, typename ... T>
constexpr auto max_imp(T0 t0, T1 t1, T ... t)
 -> typename std::enable_if<(sizeof...(T) > 0), typename std::common_type<T0, T1, T...>::type>::type
{
	return max_imp(max_imp(t0, t1), t...);
}

} // namespace detail

template<typename T0, typename ... T>
constexpr typename std::common_type<T0, T...>::type max(T0 t0, T ... t)
{
	return detail::max_imp(t0, t...);
}

} // namespace util
} // namespace yak

#endif
