#ifndef YAK_UTIL_DINIT_HPP
#define YAK_UTIL_DINIT_HPP

#include <tuple>
#include <type_traits>
#include <utility>
#include <initializer_list>

#include <boost/fusion/include/is_sequence.hpp>
#include <boost/fusion/include/value_at.hpp>
#include <boost/fusion/include/size.hpp>

#include <boost/fusion/include/begin.hpp>
#include <boost/fusion/include/advance.hpp>
#include <boost/fusion/include/key_of.hpp>
#include <boost/fusion/include/category_of.hpp>

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

template<typename Key>
struct key
{
	constexpr key() = default;
	template<typename U>
	constexpr std::pair<key, U&&> operator=(U&& u) const
	{
		return std::pair<key, U&&>(key(), std::forward<U>(u));
	}
	typedef Key type;
};

template<typename Key, typename U>
constexpr std::pair<key<Key>, U&&> key_(U&& u)
{
	return std::pair<key<Key>, U&&>(key<Key>(), std::forward<U>(u));
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
struct max_idx<std::tuple<T>> : std::integral_constant<std::size_t, get_first<T>::type::value>
{
};

template<typename T0, typename ... T> 
struct max_idx<std::tuple<T0, T...>> :
  std::enable_if<(sizeof...(T)>0),
    std::integral_constant<std::size_t, 
      (max_idx<std::tuple<T...>>::type::value > get_first<T0>::type::value ? 
       max_idx<std::tuple<T...>>::type::value : get_first<T0>::type::value)
  >>::type
{
};

///////////////////////////////////////////////////////////////////////
//
// get_value_imp for not fusion sequence
//

template<std::size_t idxM, typename Tuple, std::size_t idxN>
struct found_by_idx :
  std::is_same<typename get_first_with_idx<idxN, Tuple>::type, idx<idxM>>
{
};

// Found at the last
template<std::size_t idxM, typename Tuple, std::size_t idxN>
constexpr auto get_value_imp(Tuple t, indices<idxN>)
  -> typename std::enable_if<found_by_idx<idxM, Tuple, idxN>::value,
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
  -> typename std::enable_if<!found_by_idx<idxM, Tuple, idxN>::value,
       typename std::remove_reference<typename get_second_with_idx<idxN, Tuple>::type>::type>::type
{
// TODO: check if element_type is not-reference type
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
  -> typename std::enable_if<(sizeof...(idxN) > 0) && found_by_idx<idxM, Tuple, idxN0>::value,
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
  -> typename std::enable_if<(sizeof...(idxN) > 0) && !found_by_idx<idxM, Tuple, idxN0>::value,
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

///////////////////////////////////////////////////////////////////////
//
// get_value_imp for fusion sequence with key
//

template<bool is_assoc_seq, typename U, std::size_t idxM>
struct get_key_imp
{
	typedef typename boost::fusion::result_of::key_of<
	  typename boost::fusion::result_of::advance_c<
	    typename boost::fusion::result_of::begin<U>::type, idxM
	  >::type
	>::type type;
};

template<typename U, std::size_t idxM>
struct get_key_imp<false, U, idxM>
{
	typedef void type;
};

template<typename U, std::size_t idxM>
struct get_key
{
	typedef typename get_key_imp<std::is_base_of<
	  boost::fusion::associative_tag, 
	  typename boost::fusion::traits::category_of<U>::type
	>::type::value, U, idxM>::type type;
};

template<typename U, std::size_t idxM, typename Tuple, std::size_t idxN>
struct found_by_key :
  std::is_same<typename get_first_with_idx<idxN, Tuple>::type, key<typename get_key<U, idxM>::type>>
{
};

// Found at the last
template<typename U, std::size_t idxM, typename Tuple, std::size_t idxN>
constexpr auto get_value_imp(Tuple t, indices<idxN>)
  -> typename std::enable_if<found_by_key<U, idxM, Tuple, idxN>::value,
       typename boost::fusion::result_of::value_at_c<U, idxM>::type>::type
{
	typedef typename boost::fusion::result_of::value_at_c<U, idxM>::type element_type;
	return std::forward<element_type>(std::get<idxN>(t).second);
}

// Not found at the last, just call with idx version

// Found not at the last
template<typename U, std::size_t idxM, typename Tuple, std::size_t idxN0, std::size_t ... idxN>
constexpr auto get_value_imp(Tuple t, indices<idxN0, idxN...>)
  -> typename std::enable_if<(sizeof...(idxN) > 0) && found_by_key<U, idxM, Tuple, idxN0>::value,
       typename boost::fusion::result_of::value_at_c<U, idxM>::type>::type
{
	return std::get<idxN0>(t).second;
}

// Not found not at the last, just call with idx version

///////////////////////////////////////////////////////////////////////
//
// get_value_imp for fusion sequence with idx
//

// Found at the last
template<typename U, std::size_t idxM, typename Tuple, std::size_t idxN>
constexpr auto get_value_imp(Tuple t, indices<idxN>)
  -> typename std::enable_if<found_by_idx<idxM, Tuple, idxN>::value,
       typename boost::fusion::result_of::value_at_c<U, idxM>::type>::type
{
	typedef typename boost::fusion::result_of::value_at_c<U, idxM>::type element_type;
	return std::forward<element_type>(std::get<idxN>(t).second);
}

// Not found at the last
template<typename U, std::size_t idxM, typename Tuple, std::size_t idxN>
constexpr auto get_value_imp(Tuple t, indices<idxN>)
  -> typename std::enable_if<!found_by_idx<idxM, Tuple, idxN>::value &&
         !found_by_key<U, idxM, Tuple, idxN>::value,
       typename std::remove_reference<typename boost::fusion::result_of::value_at_c<U, idxM>::type>::type>::type
{
// TODO: check if element_type is not-reference type
	typedef typename boost::fusion::result_of::value_at_c<U, idxM>::type element_type;
	return element_type{};
}

// Found not at the last
template<typename U, std::size_t idxM, typename Tuple, std::size_t idxN0, std::size_t ... idxN>
constexpr auto get_value_imp(Tuple t, indices<idxN0, idxN...>)
  -> typename std::enable_if<(sizeof...(idxN) > 0) && found_by_idx<idxM, Tuple, idxN0>::value,
       typename boost::fusion::result_of::value_at_c<U, idxM>::type>::type
{
	return std::get<idxN0>(t).second;
}

// Not found not at the last
template<typename U, std::size_t idxM, typename Tuple, std::size_t idxN0, std::size_t ... idxN>
constexpr auto get_value_imp(Tuple t, indices<idxN0, idxN...>)
  -> typename std::enable_if<(sizeof...(idxN) > 0) && !found_by_idx<idxM, Tuple, idxN0>::value &&
         !found_by_key<U, idxM, Tuple, idxN0>::value,
       typename boost::fusion::result_of::value_at_c<U, idxM>::type>::type
{
	return get_value_imp<U, idxM>(t, indices<idxN...>());
}

///////////////////////////////////////////////////////////////////////
//
// get_value and its caller
//

template<typename U, std::size_t idxM, typename ... T>
constexpr auto get_value(std::tuple<T...> t)
  -> typename boost::fusion::result_of::value_at_c<U, idxM>::type
{
	typedef typename make_indices<0, sizeof...(T)>::type indicesN_type;
	return detail::get_value_imp<U, idxM>(t, indicesN_type());
}

template<bool is_sequence, typename U, typename Tuple>
struct size : std::integral_constant<std::size_t, max_idx<Tuple>::type::value + 1>
{
};

template<typename U, typename Tuple>
struct size<true, U, Tuple> : boost::fusion::result_of::size<U>::type
{
};

template<typename Tuple, std::size_t ... idxN>
struct Initer
{
	constexpr Initer(const Tuple& t) : args(t) {}
	constexpr Initer(const Initer& it) : args(it.args) {}
	template<typename U, std::size_t ... idxM>
	constexpr typename std::enable_if<!boost::fusion::traits::is_sequence<U>::type::value, U>::type
	convert(indices<idxM...>) const
	{
		return { get_value<idxM>(args)... };
	}
	template<typename U, std::size_t ... idxM>
	constexpr typename std::enable_if<boost::fusion::traits::is_sequence<U>::type::value, U>::type
	convert(indices<idxM...>) const
	{
		// U requires for array
		return U{ get_value<U, idxM>(args)... };
	}
	template<typename U>
	constexpr operator U() const
	{
		typedef typename size<boost::fusion::traits::is_sequence<U>::value, U, Tuple>::type size_type;
		typedef typename make_indices<0, size_type::value>::type indicesM_type;
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
constexpr const auto di(T&& ... t) -> 
  decltype(detail::init_imp(std::declval<typename make_indices<0, sizeof...(T)>::type>(), std::forward_as_tuple(t...)))
{
	typedef typename make_indices<0, sizeof...(T)>::type indicesN_type;
	return detail::init_imp(indicesN_type(), std::forward_as_tuple(t...));
}

} // namespace dinit
} // namespace util
} // namespace yak

#endif
