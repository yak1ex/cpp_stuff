#ifndef YAK_UTIL_INDEX_TUPLE_HPP
#define YAK_UTIL_INDEX_TUPLE_HPP

#include <utility>
#include <tuple>

namespace yak {
namespace util {

template<std::size_t ... args>
struct indices
{
};

namespace detail {

template<std::size_t val, typename T>
struct make_indices_impl
{
//	I'm not sure that this (dependent value postpones static_assert) is correct
	static_assert(!std::is_same<T, T>::value, "internal error: make_indices_impl instansiated with invalid arguments");
};

template<std::size_t val, std::size_t ... args>
struct make_indices_impl<val, indices<args...>>
{
	typedef indices<val, args...> type;
};

} // namespace detail

template<std::size_t beg, std::size_t end>
struct make_indices
{
	static_assert(beg < end, "invalid range");
	typedef typename detail::make_indices_impl<beg, typename make_indices<beg+1, end>::type>::type type;
};

template<std::size_t beg>
struct make_indices<beg, beg>
{
	typedef indices<> type;
};

namespace detail {

template<typename F, typename Tuple, std::size_t ... args>
constexpr auto call_imp(F f, Tuple t, indices<args...> i) -> decltype(f(std::get<args>(t)...))
{
	return f(std::get<args>(t)...);
}

} // namespace detail

template<typename F, typename Tuple>
constexpr auto call(F f, Tuple t) -> decltype(detail::call_imp(f, t, std::declval<typename make_indices<0, std::tuple_size<Tuple>::value>::type>()))
{
	typedef typename make_indices<0, std::tuple_size<Tuple>::value>::type indices_type;
	return detail::call_imp(f, t, indices_type());
}


} // namespace util
} // namespace yak

#endif
