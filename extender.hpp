#ifndef YAK_UTIL_EXTENDER
#define YAK_UTIL_EXTENDER

#include <utility>

#include <boost/detail/workaround.hpp>

#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/function_arity.hpp>
#include <boost/mpl/at.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <boost/fusion/include/push_front.hpp>
#include <boost/fusion/include/at_c.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum.hpp>

#include "forward_adapter_.hpp"
#include "variadic_bridge.hpp"

namespace yak {
namespace util {

// NOTE: ordinary operator semantics/precedence
template<typename C, typename T>
struct extender1
{
#if defined(BOOST_NO_RVALUE_REFERENCES)

	typedef yak::boostex::forward_adapter_<T, C> wrap;

#else // defined(BOOST_NO_RVALUE_REFERENCES)

	struct wrap
	{
		T &t;
		wrap(T& t) : t(t) {}
#if BOOST_WORKAROUND(__GNUC__, == 4) && (__GNUC_MINOR__ <= 5) || defined(BOOST_NO_VARIADIC_TEMPLATES)

#define EXTENDER_PP_TEMPLATE1_1(z, n, data) std::forward<BOOST_PP_CAT(Arg, n)>(BOOST_PP_CAT(arg, n))
#define EXTENDER_PP_TEMPLATE1_2(n) \
template<BOOST_PP_ENUM_PARAMS(n, typename Arg)> \
auto operator()(BOOST_PP_ENUM_BINARY_PARAMS(n, Arg, && arg)) const -> decltype(C()(t BOOST_PP_ENUM_TRAILING(n, EXTENDER_PP_TEMPLATE1_1, _))) \
{ return C()(t BOOST_PP_ENUM_TRAILING(n, EXTENDER_PP_TEMPLATE1_1, _)); }
#define EXTENDER_PP_TEMPLATE1_3(n) \
/* It seems that we can't define member fuction with appropriate return type lazily */ \
/* decltype(C()(t)) operator()() const */ \
/* { return C()(t); } */
#define EXTENDER_PP_TEMPLATE1(z, n, data) BOOST_PP_IF(n, EXTENDER_PP_TEMPLATE1_2, EXTENDER_PP_TEMPLATE1_3)(n)

		BOOST_PP_REPEAT(4, EXTENDER_PP_TEMPLATE1, _)

#undef EXTENDER_PP_TEMPLATE1_1
#undef EXTENDER_PP_TEMPLATE1_2
#undef EXTENDER_PP_TEMPLATE1_3
#undef EXTENDER_PP_TEMPLATE1

#else // BOOST_WORKAROUND(__GNUC__, == 4) && (__GNUC_MINOR__ <= 5) || defined(BOOST_NO_VARIADIC_TEMPLATES)

		template<typename ... Args>
		auto operator()(Args && ... args) const -> decltype(C()(t, std::forward<Args>(args)...))
		{ return C()(t, std::forward<Args>(args)...); }

#endif // BOOST_WORKAROUND(__GNUC__, == 4) && (__GNUC_MINOR__ <= 5) || defined(BOOST_NO_VARIADIC_TEMPLATES)

	};

#endif // defined(BOOST_NO_RVALUE_REFERENCES)

	friend wrap operator->*(T& t, C c)
	{
		return wrap(t);
	}
};

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

#if !defined(BOOST_NO_VARIADIC_TEMPLATES) && \
    !defined(BOOST_NO_STATIC_ASSERT) && \
    !defined(BOOST_NO_RVALUE_REFERENCES)
#define YAK_UTIL_EXTENDER3_ENABLED

// NOTE: Maybe alias template to extender2 can be used
// NOTE: irregular operator semantics/precedence with macro support
template<typename C, typename T, typename F>
struct extender3
{
	typedef typename boost::function_types::result_type<F>::type result_type;
	typedef typename boost::function_types::parameter_types<F> param_types;
	typedef typename boost::fusion::result_of::as_vector<param_types>::type fusion_type;
	// TODO: Guard by parameter type
	fusion_type ft;
	template<typename ... Args>
	extender3(Args && ... args) : ft(std::forward<Args>(args)...)
	{
		static_assert(boost::function_types::function_arity<F>::value == sizeof ... (args), "the number of arguments mismatches with function arity.");
	}
	friend result_type operator->*(T& t, const extender3& c)
	{
		return boost::fusion::invoke(&C::func, boost::fusion::push_front(c.ft, boost::ref(t)));
	}
};

namespace detail {

// For limitation of Boost.Fusion with rvalue references
template<typename T> struct ftype;
template<typename T> struct ftype<T&>       { typedef T& type; };
template<typename T> struct ftype<const T&> { typedef const T& type; };
#ifdef YAK_BOOST_FUSION_HAS_RR_SUPPORT
template<typename T> struct ftype<T&&>      { typedef T&& type; };
#else // defined(YAK_BOOST_FUSION_HAS_RR_SUPPORT)
template<typename T> struct ftype<T&&>      { typedef const T& type; };
#endif // defined(YAK_BOOST_FUSION_HAS_RR_SUPPORT)

template<typename C, typename T, typename ... Args>
struct extender3_wrap_
{
#if BOOST_WORKAROUND(__GNUC__, == 4) && (__GNUC_MINOR__ <= 5)
	typedef typename boost::fusion::result_of::as_vector<
		typename yak::util::variadic_to_vector<typename ftype<Args&&>::type...>::type
	>::type fusion_type;
#else // BOOST_WORKAROUND(__GNUC__, == 4) && (__GNUC_MINOR__ <= 5)
	typedef boost::fusion::vector<typename ftype<Args&&>::type...> fusion_type;
#endif // BOOST_WORKAROUND(__GNUC__, == 4) && (__GNUC_MINOR__ <= 5)
	fusion_type args;

	extender3_wrap_(Args&& ... args) : args(std::forward<Args>(args)...) {}
	friend auto operator->*(T& t, const extender3_wrap_& c) -> decltype(boost::fusion::invoke(typename C::_(), boost::fusion::push_front(c.args, boost::ref(t))))

	{
		return boost::fusion::invoke(typename C::_(), boost::fusion::push_front(c.args, boost::ref(t)));
	}
	friend auto operator->*(const T& t, const extender3_wrap_& c) -> decltype(boost::fusion::invoke(typename C::_(), boost::fusion::push_front(c.args, boost::ref(t))))

	{
		return boost::fusion::invoke(typename C::_(), boost::fusion::push_front(c.args, boost::ref(t)));
	}
};

} // namespace detail

template<typename C, typename T>
struct extender3_
{
	template<typename ... Args>
	detail::extender3_wrap_<C, T, Args...> operator()(Args&& ... args) const
	{
		return detail::extender3_wrap_<C, T, Args...>(std::forward<Args>(args)...);
	}
};

#endif // !defined(BOOST_NO_VARIADIC_TEMPLATES) && !defined(BOOST_NO_STATIC_ASSERT) && !defined(BOOST_NO_RVALUE_REFERENCES)

} // namespace util
} // namespace yak

#ifdef YAK_UTIL_EXTENDER3_ENABLED

#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/seq.hpp>
#include <boost/preprocessor/tuple/elem.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/preprocessor/seq/transform.hpp>

// TODO: variadic style support

#define EXTENDER_PP_RESULT_TYPE(signature)   BOOST_PP_SEQ_ELEM(0, signature)
#define EXTENDER_PP_FUNC_NAME(signature)     BOOST_PP_SEQ_ELEM(1, signature)
#define EXTENDER_PP_PARAMS(signature)        BOOST_PP_SEQ_TAIL(BOOST_PP_SEQ_TAIL(signature))
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

#endif

#endif // YAK_UTIL_EXTENDER
