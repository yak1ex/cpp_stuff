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
#include <boost/functional/forward_adapter.hpp>

#include <boost/preprocessor/cat.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing.hpp>
#include <boost/preprocessor/control/if.hpp>
#include <boost/preprocessor/repetition/repeat.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/facilities/intercept.hpp>

#include "forward_adapter_.hpp"
#if !defined(BOOST_NO_VARIADIC_TEMPLATES)
#include "variadic_bridge.hpp"
#endif

namespace yak {
namespace util {

// NOTE: ordinary operator semantics/precedence
template<typename C, typename T>
struct extender1
{
#if defined(BOOST_NO_RVALUE_REFERENCES)

	typedef yak::boostex::forward_adapter_<T, C> wrap;
	typedef yak::boostex::forward_adapter_<const T, C> wrapc;
	friend wrap operator->*(T& t, C c)
	{
		return wrap(t);
	}
	friend wrapc operator->*(const T& t, C c)
	{
		return wrapc(t);
	}

#else // defined(BOOST_NO_RVALUE_REFERENCES)

	template<typename T2>
	struct wrap
	{
		T2 &&t;
		wrap(T2&& t) : t(std::forward<T2>(t)) {}

// TODO: Not checked at GCC 4.3 and 4.4
#if BOOST_WORKAROUND(__GNUC__, == 4) && (__GNUC_MINOR__ <= 4) || defined(BOOST_NO_VARIADIC_TEMPLATES)

#define EXTENDER_PP_TEMPLATE1_1(z, n, data) std::forward<BOOST_PP_CAT(Arg, n)>(BOOST_PP_CAT(arg, n))
#define EXTENDER_PP_TEMPLATE1_2(n) \
template<BOOST_PP_ENUM_PARAMS(n, typename Arg)> \
auto operator()(BOOST_PP_ENUM_BINARY_PARAMS(n, Arg, && arg)) const -> decltype(C()(std::forward<T2>(t) BOOST_PP_ENUM_TRAILING(n, EXTENDER_PP_TEMPLATE1_1, _))) \
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

#else // BOOST_WORKAROUND(__GNUC__, == 4) && (__GNUC_MINOR__ <= 4) || defined(BOOST_NO_VARIADIC_TEMPLATES)

#if BOOST_WORKAROUND(__GNUC__, == 4) && ((__GNUC_MINOR__ == 5) || (__GNUC_MINOR__ == 6) && (__GNUC_PATCHLEVEL__ == 0))

private:
		template<typename ... Args>
		struct deduce
		{
			typedef decltype(C()(std::forward<Args>(std::declval<Args>())...)) type;
		};

public:

		template<typename ... Args>
		typename deduce<T2,Args...>::type operator()(Args && ... args) const
		{ return C()(std::forward<T2>(t), std::forward<Args>(args)...); }

#else // BOOST_WORKAROUND(__GNUC__, == 4) && (__GNUC_MINOR__ == 6) && (__GNUC_PATCHLEVEL__ == 0)

		template<typename ... Args>
		auto operator()(Args && ... args) const -> decltype(C()(std::forward<T2>(t), std::forward<Args>(args)...))
		{ return C()(std::forward<T2>(t), std::forward<Args>(args)...); }

#endif // BOOST_WORKAROUND(__GNUC__, == 4) && (__GNUC_MINOR__ == 6) && (__GNUC_PATCHLEVEL__ == 0)

#endif // BOOST_WORKAROUND(__GNUC__, == 4) && (__GNUC_MINOR__ <= 5) || defined(BOOST_NO_VARIADIC_TEMPLATES)

	};
	template<typename T2>
	friend wrap<T2> operator->*(T2&& t, C c)
	{
		return wrap<T2>(std::forward<T2>(t));
	}

#endif // defined(BOOST_NO_RVALUE_REFERENCES)

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
    !defined(BOOST_NO_DECLTYPE) && \
    !defined(BOOST_NO_RVALUE_REFERENCES)
#define YAK_UTIL_EXTENDER3_ENABLED

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
struct extender3_wrap
{
#if BOOST_WORKAROUND(__GNUC__, == 4) && (__GNUC_MINOR__ <= 6)
	typedef typename boost::fusion::result_of::as_vector<
		typename yak::util::variadic_to_vector<typename ftype<Args&&>::type...>::type
	>::type fusion_type;
#else // BOOST_WORKAROUND(__GNUC__, == 4) && (__GNUC_MINOR__ <= 5)
	typedef boost::fusion::vector<typename ftype<Args&&>::type...> fusion_type;
#endif // BOOST_WORKAROUND(__GNUC__, == 4) && (__GNUC_MINOR__ <= 5)
	fusion_type args;

	extender3_wrap(Args&& ... args) : args(std::forward<Args>(args)...) {}
	friend auto operator->*(T& t, const extender3_wrap& c) -> decltype(boost::fusion::invoke(typename C::_(), boost::fusion::push_front(c.args, boost::ref(t))))
	{
		return boost::fusion::invoke(typename C::_(), boost::fusion::push_front(c.args, boost::ref(t)));
	}
	friend auto operator->*(const T& t, const extender3_wrap& c) -> decltype(boost::fusion::invoke(typename C::_(), boost::fusion::push_front(c.args, boost::ref(t))))
	{
		return boost::fusion::invoke(typename C::_(), boost::fusion::push_front(c.args, boost::ref(t)));
	}
#ifdef YAK_BOOST_FUSION_HAS_RR_SUPPORT
//	might be implemented as follows:
	friend auto operator->*(T&& t, const extender3_wrap& c) -> decltype(boost::fusion::invoke(typename C::_(), boost::fusion::push_front(c.args, boost::ref(t))))
	{
		return boost::fusion::invoke(typename C::_(), boost::fusion::push_front(c.args, boost::ref(t)));
	}
#endif // defined(YAK_BOOST_FUSION_HAS_RR_SUPPORT)
};

} // namespace detail

// NOTE: Maybe PP version obsoletes extender2
// NOTE: irregular operator semantics/precedence with macro support
template<typename C, typename T>
struct extender3
{
	template<typename ... Args>
	detail::extender3_wrap<C, T, Args...> operator()(Args&& ... args) const
	{
		return detail::extender3_wrap<C, T, Args...>(std::forward<Args>(args)...);
	}
};

#else // !defined(BOOST_NO_VARIADIC_TEMPLATES) && !defined(BOOST_NO_DECLTYPE) && !defined(BOOST_NO_RVALUE_REFERENCES)

namespace detail {

#define EXTENDER_PP_TEMPLATE3_1(z, n, data) \
template<typename C, typename T BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename A)> \
struct BOOST_PP_CAT(extender3_wrap, n) \
{ \
	typedef boost::fusion::vector<BOOST_PP_ENUM_BINARY_PARAMS(n, A, & BOOST_PP_INTERCEPT)> fusion_type; \
	fusion_type args; \
\
	BOOST_PP_CAT(extender3_wrap, n)(BOOST_PP_ENUM_BINARY_PARAMS(n, A, &a)) : args(BOOST_PP_ENUM_PARAMS(n, a)) {} \
	friend typename boost::result_of<typename C::_(T& BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_BINARY_PARAMS(n, A, & BOOST_PP_INTERCEPT))>::type \
	operator->*(T& t, const BOOST_PP_CAT(extender3_wrap, n)& c) \
	{ \
		return boost::fusion::invoke(typename C::_(), boost::fusion::push_front(c.args, boost::ref(t))); \
	} \
	friend typename boost::result_of<typename C::_(const T& BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_BINARY_PARAMS(n, A, & BOOST_PP_INTERCEPT))>::type \
	operator->*(const T& t, const BOOST_PP_CAT(extender3_wrap, n)& c) \
	{ \
		return boost::fusion::invoke(typename C::_(), boost::fusion::push_front(c.args, boost::ref(t))); \
	} \
};

BOOST_PP_REPEAT(4, EXTENDER_PP_TEMPLATE3_1, _)

#define EXTENDER_PP_TEMPLATE3_2(n) \
	template<typename F BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, typename A)> \
	struct result<F(BOOST_PP_ENUM_BINARY_PARAMS(n, A, & BOOST_PP_INTERCEPT))> \
	{ \
		typedef BOOST_PP_CAT(extender3_wrap, n)<C, T BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, A)> type; \
	}; \
	template<BOOST_PP_ENUM_PARAMS(n, typename A)> \
	BOOST_PP_CAT(extender3_wrap, n)<C, T BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, A)> \
	operator()(BOOST_PP_ENUM_BINARY_PARAMS(n, A, &a)) const \
	{ \
		return BOOST_PP_CAT(extender3_wrap, n)<C, T BOOST_PP_COMMA_IF(n) BOOST_PP_ENUM_PARAMS(n, A)>(BOOST_PP_ENUM_PARAMS(n, a)); \
	}
#define EXTENDER_PP_TEMPLATE3_3(n) \
	template<typename F> \
	struct result<F()> \
	{ \
		typedef BOOST_PP_CAT(extender3_wrap, n)<C, T> type; \
	}; \
	BOOST_PP_CAT(extender3_wrap, n)<C, T> \
	operator()() const \
	{ \
		return BOOST_PP_CAT(extender3_wrap, n)<C, T>(); \
	}
#define EXTENDER_PP_TEMPLATE3_4(z, n, data) BOOST_PP_IF(n, EXTENDER_PP_TEMPLATE3_2, EXTENDER_PP_TEMPLATE3_3)(n)

template<typename C, typename T>
struct extender3_wrap_factory
{
	template<typename>
	struct result;

	BOOST_PP_REPEAT(4, EXTENDER_PP_TEMPLATE3_4, _)
};

} // namespace detail

// NOTE: irregular operator semantics/precedence with macro support
template<typename C, typename T>
struct extender3 : public boost::forward_adapter<detail::extender3_wrap_factory<C, T> >
{
};

#endif // !defined(BOOST_NO_VARIADIC_TEMPLATES) && !defined(BOOST_NO_DECLTYPE) && !defined(BOOST_NO_RVALUE_REFERENCES)

} // namespace util
} // namespace yak

#ifdef YAK_UTIL_EXTENDER3_ENABLED

// DEFINE_EXTENDER(extend_target, extension_method_name, { functor class definition without class-head });
// NOTE: Without variadic macro, you can not place comma inside the functor class definition directly.

#if defined(BOOST_NO_VARIADIC_MACROS)

#define DEFINE_EXTENDER(target, name, impl) \
struct BOOST_PP_CAT(name, _functor) : public yak::util::extender3<BOOST_PP_CAT(name, _functor), target> \
{ \
	struct _ impl; \
} name

#else

#define DEFINE_EXTENDER(target, name, ...) \
struct BOOST_PP_CAT(name, _functor) : public yak::util::extender3<BOOST_PP_CAT(name, _functor), target> \
{ \
	struct _ __VA_ARGS__; \
} name

#endif

#endif

#endif // YAK_UTIL_EXTENDER
