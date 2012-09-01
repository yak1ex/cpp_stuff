/*****************************************************************************/
//
//  variadic_bridge.hpp: Helper classes to circumvent limited implementation
//                                                        of variadic templates
//
//  Use modification and distribution are subject to the Boost Software 
//  License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt).  
//
/*****************************************************************************/

#ifndef YAK_UTIL_VARIADIC_BRIDGE_HPP
#define YAK_UTIL_VARIADIC_BRIDGE_HPP

#include <boost/mpl/vector.hpp>
#include <boost/mpl/push_back.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/size.hpp>

#include <boost/preprocessor/iteration/local.hpp>
#include <boost/preprocessor/repetition/enum.hpp>

namespace yak {

namespace util {

// Convert variadic type parameters into a MPL vector

namespace detail {

template<typename Result, typename ... Tn>
struct variadic_to_vector_;

template<typename Result>
struct variadic_to_vector_<Result>
{
	typedef Result type;
};

template<typename Result, typename T0, typename ... Tn>
struct variadic_to_vector_<Result, T0, Tn...>
{
	typedef typename variadic_to_vector_<
		typename boost::mpl::push_back<Result, T0>::type,
		Tn...
	>::type type;
};

} // namespace detail

template<typename ... Tn>
struct variadic_to_vector
{
	typedef typename detail::variadic_to_vector_<boost::mpl::vector<>, Tn...>::type type;
};

// Convert variadic template type parameters to fixed-number type parameters

namespace detail {

// Result of push_back could be different type from boost::mpl::vector.
// Therefore, we do not use partial specialization with vector<...>

template<typename Lambda, typename VT, int N>
struct variadic_to_fixed_2;

// Partial specialization
// Expanded by Boost PP accroding to the following template
/* 
template<typename MetaFunc, typename VT>
struct variadic_to_fixed_2<MetaFunc, VT, n>
{
	typedef typename MetaFunc::template apply<
		typename boost::mpl::at_c<VT, 0>::type,
		...
		typename boost::mpl::at_c<VT, n-1>::type
	>::type type;
};
*/
// Alternative: typedef typename boost::mpl::apply<Lambda, ...>
// We do not use this alternative due to lmitiation of arity of metafunction.

template<typename MetaFunc, typename VT>
struct variadic_to_fixed_
{
	typedef typename variadic_to_fixed_2<MetaFunc, VT, boost::mpl::size<VT>::value>::type type;
};

} // namespace detail

template<typename MetaFunc, typename ... Tn>
struct variadic_to_fixed
{
	typedef typename detail::variadic_to_fixed_<MetaFunc, typename variadic_to_vector<Tn...>::type>::type type;
};


// Boiler-plate specialization definition expanded by Boost PP

namespace detail {

#define TYPE(z, i, data) typename boost::mpl::at_c<VT, i>::type
#define ELEMENT_TYPE(z, i, data) typename boost::tuples::element<i, T>::type
#define GET(z, i, data) boost::get<i>(t)

#define BOOST_PP_LOCAL_MACRO(n) \
	template<typename MetaFunc, typename VT> \
	struct variadic_to_fixed_2<MetaFunc, VT, n> { \
		typedef typename MetaFunc::template apply< \
			BOOST_PP_ENUM(n, TYPE, _) \
		>::type type; \
	};
#define BOOST_PP_LOCAL_LIMITS (0, 10)
#include BOOST_PP_LOCAL_ITERATE()

#undef TYPE
#undef ELEMENT
#undef GET
#undef BOOST_PP_LOCAL_MACRO
#undef BOOST_PP_LOCAL_LIMITS

} // namespace detail

} // namespace util

} // namespace yak

#endif // defined(YAK_UTIL_VARIADIC_BRIDGE_HPP)
