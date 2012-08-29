#ifndef YAK_UTIL_EXTENDER
#define YAK_UTIL_EXTENDER

#include <utility>

#include <boost/function.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <boost/fusion/include/push_front.hpp>

#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/punctuation/comma_if.hpp>

namespace yak {
namespace util {

namespace detail {

template<typename T, typename F>
struct extender1_wrap_base
{
	typedef typename boost::function_types::result_type<F>::type result_type;
	// XXX: Maybe explicitly specify ClassTransform as identity or others
	typedef typename boost::function_types::parameter_types<F> param_types;
	typedef typename boost::function_types::function_type<
		typename boost::mpl::push_front<
			typename boost::mpl::push_front<
				param_types, T&
			>::type, result_type
		>::type
	>::type func_type;
	typedef boost::function_types::function_arity<F> arity_type;
	T& t;
	func_type* f;
	extender1_wrap_base(T& t, func_type* f) : t(t), f(f) {}
};

template<int N, typename T, typename F>
struct extender1_wrap_arg;

#define EXTENDER_PP_TEMPLATE1_(z, n, data) typename boost::mpl::at_c<param_types, n>::type BOOST_PP_CAT(a, n)

#define EXTENDER_PP_TEMPLATE1(z, n, data) \
template<typename T, typename F> \
struct extender1_wrap_arg<n, T, F> : public extender1_wrap_base<T, F> \
{ \
	typedef extender1_wrap_base<T, F> base_type; \
	typedef typename base_type::func_type func_type; \
	typedef typename base_type::result_type result_type; \
	typedef typename base_type::param_types param_types; \
\
	extender1_wrap_arg(T& t, func_type* f) : base_type(t, f) {} \
	result_type operator()( \
		BOOST_PP_ENUM(n, EXTENDER_PP_TEMPLATE1_, _) \
	) \
	{ \
		return f(base_type::t BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, a)); \
	} \
};

BOOST_PP_REPEAT(4, EXTENDER_PP_TEMPLATE1, _)

}

// NOTE: ordinary operator semantics/precedence
// XXX: Maybe C::func is passed, too
template<typename C, typename T, typename F>
struct extender1
{
	struct wrap : public detail::extender1_wrap_arg<boost::function_types::function_arity<F>::value, T, F>
	{
		typedef detail::extender1_wrap_arg<boost::function_types::function_arity<F>::value, T, F> base_type;
		wrap(T& t, typename base_type::func_type* f) : base_type(t, f) {}
	};
	friend boost::function<F> operator->*(T& t, const C&)
	{
		return wrap(t, C::func);
	}
};

// TODO: Unify extender2 and extender3

namespace detail {

template<int N, typename C, typename T, typename F>
struct extender2_wrap_arg;

#define EXTENDER_PP_TEMPLATE2_(z, n, data) typename boost::mpl::at_c<param_types, n>::type BOOST_PP_CAT(a, n)

#define EXTENDER_PP_TEMPLATE2(z, n, data) \
template<typename C, typename T, typename F> \
struct extender2_wrap_arg<n, C, T, F> \
{ \
	typedef typename boost::function_types::result_type<F>::type result_type; \
	typedef typename boost::function_types::parameter_types<F> param_types; \
	typedef typename boost::fusion::result_of::as_vector<param_types>::type fusion_type; \
\
	fusion_type ft; \
	extender2_wrap_arg( \
		BOOST_PP_ENUM(n, EXTENDER_PP_TEMPLATE2_, _) \
	) : ft(BOOST_PP_ENUM_PARAMS(n, a)) \
	{ \
	} \
	friend result_type operator->*(T& t, const extender2_wrap_arg& c) \
	{ \
		return boost::fusion::invoke(&C::func, boost::fusion::push_front(c.ft, boost::ref(t))); \
	} \
};

BOOST_PP_REPEAT(4, EXTENDER_PP_TEMPLATE2, _)

} // namespace detail

// NOTE: irregular operator semantics/precedence
template<typename C, typename T, typename F>
struct extender2
{
	typedef detail::extender2_wrap_arg<boost::function_types::function_arity<F>::value, C, T, F> _;
};

// TODO: Guard by C++0x detection macro
// NOTE: irregular operator semantics/precedence with macro support
template<typename C, typename T, typename F>
struct extender3
{
	typedef typename boost::function_types::result_type<F>::type result_type;
	typedef typename boost::function_types::parameter_types<F> param_types;
	typedef typename boost::fusion::result_of::as_vector<param_types>::type fusion_type;
	// TODO: Make use of PP or variadic templates
	// TODO: Guard by arity
	fusion_type ft;
	extender3()
	{
	}
	extender3(
		typename boost::mpl::at_c<param_types, 0>::type a0
	) : ft(a0)
	{
	}
	extender3(
		typename boost::mpl::at_c<param_types, 0>::type a0,
		typename boost::mpl::at_c<param_types, 1>::type a1
	) : ft(a0, a1)
	{
	}
	extender3(
		typename boost::mpl::at_c<param_types, 0>::type a0,
		typename boost::mpl::at_c<param_types, 1>::type a1,
		typename boost::mpl::at_c<param_types, 2>::type a2
	) : ft(a0, a1, a2)
	{
	}
	typedef extender3 target_type;
	friend result_type operator->*(T& t, const target_type& c)
	{
		return boost::fusion::invoke(&C::func, boost::fusion::push_front(c.ft, boost::ref(t)));
	}
};



} // namespace util
} // namespace yak

#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/seq/size.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/transform.hpp>
#include <boost/preprocessor/tuple/elem.hpp>

#define EXTENDER_PP_RESULT_TYPE(signature)   BOOST_PP_SEQ_ELEM(0, signature)
#define EXTENDER_PP_FUNC_NAME(signature)     BOOST_PP_SEQ_ELEM(1, signature)
#define EXTENDER_PP_PARAMS(signature)        BOOST_PP_SEQ_ELEM(2, signature)
#define EXTENDER_PP_TYPE_EXTRACT(s, d, e)    BOOST_PP_TUPLE_ELEM(2, 0, e)
#define EXTENDER_PP_PARAM_TYPES(signature)   BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(EXTENDER_PP_TYPE_EXTRACT, _, BOOST_PP_SEQ_TAIL(EXTENDER_PP_PARAMS(signature))))
#define EXTENDER_PP_FUNC_TYPE(signature)     EXTENDER_PP_RESULT_TYPE(signature)(EXTENDER_PP_PARAM_TYPES(signature))
#define EXTENDER_PP_ARG_EXTRACT(s, d, e)     BOOST_PP_TUPLE_ELEM(2, 0, e) BOOST_PP_TUPLE_ELEM(2, 1, e)
#define EXTENDER_PP_FUNC_ARGS(signature)     BOOST_PP_SEQ_ENUM(BOOST_PP_SEQ_TRANSFORM(EXTENDER_PP_ARG_EXTRACT, _, EXTENDER_PP_PARAMS(signature)))

#define BEGIN_EXTENDER(target, signature) \
struct EXTENDER_PP_FUNC_NAME(signature) : public yak::util::extender3<EXTENDER_PP_FUNC_NAME(signature), target, EXTENDER_PP_FUNC_TYPE(signature)> \
{ \
	template<typename ... Args> \
	EXTENDER_PP_FUNC_NAME(signature)(Args&& ... args) : extender3(std::forward<Args>(args)...) {} \
	static typename boost::function_types::result_type<EXTENDER_PP_FUNC_TYPE(signature)>::type func(EXTENDER_PP_FUNC_ARGS(signature))
#define END_EXTENDER() \
};

#endif // YAK_UTIL_EXTENDER
