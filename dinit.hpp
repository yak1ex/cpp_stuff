#ifndef YAK_UTIL_DINIT_HPP
#define YAK_UTIL_DINIT_HPP

#include <tuple>
#include <type_traits>
#include <utility>
#include <initializer_list>

#include "index_tuple.hpp"

namespace yak {
namespace util {
namespace dinit {

template<std::size_t N>
struct idx
{
	constexpr idx() = default;
	template<typename U>
	constexpr std::pair<idx, U&&> operator=(U&& u) const
	{
		return std::pair<idx, U&&>(idx(), std::forward<U>(u));
	}
	static constexpr std::size_t value = N;
};

template<std::size_t N, typename U>
constexpr std::pair<idx<N>, U&&> idx_(U&& u)
{
	return std::pair<idx<N>, U&&>(idx<N>(), std::forward<U>(u));
}

namespace detail {

template<typename T>
struct get_first
{
	typedef typename std::tuple_element<0, typename std::remove_reference<T>::type>::type type;
};

template<typename T>
struct get_second
{
	typedef typename std::tuple_element<1, typename std::remove_reference<T>::type>::type type;
};

template<std::size_t idxN, typename T>
struct get_first_with_idx
{
	typedef typename get_first<typename std::tuple_element<idxN, T>::type>::type type;
};

template<std::size_t idxN, typename T>
struct get_second_with_idx
{
	typedef typename get_second<typename std::tuple_element<idxN, T>::type>::type type;
};

template<typename T> 
struct max_idx;

template<typename T>
struct max_idx<std::tuple<T>>
{
	static constexpr std::size_t value = get_first<T>::type::value;
};

template<typename T0, typename ... T> 
struct max_idx<std::tuple<T0, T...>>
{
private:
	static constexpr std::size_t value1 = max_idx<std::tuple<T...>>::value;
	static constexpr std::size_t value2 = get_first<T0>::type::value;
public:
	static constexpr typename std::enable_if<(sizeof...(T)>0), std::size_t>::type value
	  = value1 > value2 ? value1 : value2;
};

// Found at the last
template<std::size_t idxM, typename Tuple, std::size_t idxN>
constexpr auto get_value_imp(Tuple t, indices<idxN>)
  -> typename std::enable_if<std::is_same<typename get_first_with_idx<idxN, Tuple>::type, idx<idxM>>::value,
       typename get_second_with_idx<idxN, Tuple>::type>::type
{
	typedef typename get_second_with_idx<idxN, Tuple>::type element_type;
	return std::forward<element_type>(std::get<idxN>(t).second);
}

// Not found at the last
//
// NOTE: Returning temporary value, thus remove_reference is necessary.
//       This may become contradictory with return type of caller using common_type.
//       (current conversion: prvalue -> prvalue / possible change: prvalue -> xvalue)
//       The current GCC implementation returns a prvalue type as a result of common_type.
//       However, it seems that conformant implementation returns an xvalue type as a result of common_type.
template<std::size_t idxM, typename Tuple, std::size_t idxN>
constexpr auto get_value_imp(Tuple t, indices<idxN>)
  -> typename std::enable_if<!std::is_same<typename get_first_with_idx<idxN, Tuple>::type, idx<idxM>>::value,
       typename std::remove_reference<typename get_second_with_idx<idxN, Tuple>::type>::type>::type
{
	typedef typename get_second_with_idx<idxN, Tuple>::type element_type;
	return element_type{};
}

// Found not at the last
//
// NOTE: The current GCC implementation returns a prvalue type as a result of common_type.
//       However, it seems that conformant implementation returns an xvalue type as a result of common_type.
//       In this case, the conversion is safe.
//       (current conversion: xvalue -> prvalue / possible change: xvalue -> xvalue)
template<std::size_t idxM, typename Tuple, std::size_t idxN0, std::size_t ... idxN>
constexpr auto get_value_imp(Tuple t, indices<idxN0, idxN...>)
  -> typename std::enable_if<(sizeof...(idxN) > 0) && std::is_same<typename get_first_with_idx<idxN0, Tuple>::type, idx<idxM>>::value,
       typename std::common_type<typename get_second_with_idx<idxN0, Tuple>::type, typename get_second_with_idx<idxN, Tuple>::type...>::type>::type
{
	return std::get<idxN0>(t).second;
}

// Not found not at the last
//
// NOTE: Returning temporary value, thus remove_reference is necessary.
//       This may become contradictory with return type of callee, which may become prvalue.
//       (current conversion: prvalue -> prvalue / possible change: prvalue -> xvalue)
//       The current GCC implementation returns a prvalue type as a result of common_type.
//       However, it seems that conformant implementation returns an xvalue type as a result of common_type.
template<std::size_t idxM, typename Tuple, std::size_t idxN0, std::size_t ... idxN>
constexpr auto get_value_imp(Tuple t, indices<idxN0, idxN...>)
  -> typename std::enable_if<(sizeof...(idxN) > 0) && !std::is_same<typename get_first_with_idx<idxN0, Tuple>::type, idx<idxM>>::value,
       typename std::common_type<typename get_second_with_idx<idxN0, Tuple>::type, typename get_second_with_idx<idxN, Tuple>::type...>::type>::type
{
	return get_value_imp<idxM>(t, indices<idxN...>());
}

template<std::size_t idxM, typename ... T>
constexpr auto get_value(std::tuple<T...> t)
  -> typename std::common_type<typename get_second<T>::type...>::type
{
	typedef typename make_indices<0, sizeof...(T)>::type indicesN_type;
	return detail::get_value_imp<idxM>(t, indicesN_type());
}

template<typename Tuple, std::size_t ... idxN>
struct Initer
{
	constexpr Initer(const Tuple& t) : args(t) {}
	constexpr Initer(const Initer& it) : args(it.args) {}
	template<typename U, std::size_t ... idxM>
	constexpr U convert(indices<idxM...>)
	{
		return { get_value<idxM>(args)... };
	}
	template<typename U>
	constexpr operator U()
	{
		typedef typename make_indices<0, max_idx<Tuple>::value + 1>::type indicesM_type;
		return convert<U>(indicesM_type());
	}
	Tuple args;
};

template<typename Tuple, std::size_t ... idxN>
constexpr Initer<Tuple, idxN...> init_imp(indices<idxN...> dummy, Tuple t)
{
	return Initer<Tuple, idxN...>(t);
}

} // namespace detail

// TODO: exclude empty arguments
template<typename ... T>
constexpr const auto di(T&& ... t) -> decltype(detail::init_imp(std::declval<typename make_indices<0, sizeof...(T)>::type>(), std::forward_as_tuple(t...)))
{
	typedef typename make_indices<0, sizeof...(T)>::type indicesN_type;
	return detail::init_imp(indicesN_type(), std::forward_as_tuple(t...));
}

} // namespace dinit
} // namespace util
} // namespace yak

#endif
