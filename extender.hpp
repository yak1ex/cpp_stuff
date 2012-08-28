#ifndef YAK_UTIL_EXTENDER
#define YAK_UTIL_EXTENDER

#include <utility>

#include <boost/function.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/function_types/parameter_types.hpp>
#include <boost/function_types/function_type.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/int.hpp>
#include <boost/mpl/at.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/fusion/adapted/mpl.hpp>
#include <boost/fusion/include/as_vector.hpp>
#include <boost/fusion/functional/invocation/invoke.hpp>
#include <boost/fusion/include/push_front.hpp>

namespace yak {
namespace util {

// NOTE: ordinary operator semantics/precedence
// XXX: Maybe C::func is passed, too
template<typename C, typename T, typename F>
struct extender1
{
	typedef typename boost::function_types::result_type<F>::type result_type;
	typedef typename boost::function_types::parameter_types<F> param_types;
	typedef typename boost::function_types::function_type<
		typename boost::mpl::push_front<
			typename boost::mpl::push_front<
				param_types, T&
			>::type, result_type
		>::type
	>::type func_type;
	template<typename F2>
	struct wrap
	{
		T& t;
		func_type* f;
		wrap(T& t, func_type* f) : t(t), f(f) {}
		// XXX: Maybe explicitly specify ClassTransform as identity or others
		// TODO: Make use of PP or variadic templates
		// TODO: Guard by arity
		result_type operator()()
		{
			return f(t);
		}
		result_type operator()(
			typename boost::mpl::at_c<param_types, 0>::type a0
		)
		{
			return f(t, a0);
		}
		result_type operator()(
			typename boost::mpl::at_c<param_types, 0>::type a0,
			typename boost::mpl::at_c<param_types, 1>::type a1
		)
		{
			return f(t, a0, a1);
		}
		result_type operator()(
			typename boost::mpl::at_c<param_types, 0>::type a0,
			typename boost::mpl::at_c<param_types, 1>::type a1,
			typename boost::mpl::at_c<param_types, 2>::type a2
		)
		{
			return f(t, a0, a1, a2);
		}
	};
	friend boost::function<F> operator->*(T& t, const C&)
	{
		return wrap<F>(t, C::func);
	}
};

// TODO: Unify extender2 and extender3

// NOTE: irregular operator semantics/precedence
template<typename C, typename T, typename F>
struct extender2
{
	typedef typename boost::function_types::result_type<F>::type result_type;
	typedef typename boost::function_types::parameter_types<F> param_types;
	typedef typename boost::fusion::result_of::as_vector<param_types>::type fusion_type;
	// TODO: inheriting constructor version
	// TODO: Make use of PP or variadic templates
	// TODO: Guard by arity
	struct _
	{
		fusion_type ft;
		_()
		{
		}
		_(
			typename boost::mpl::at_c<param_types, 0>::type a0
		) : ft(a0)
		{
		}
		_(
			typename boost::mpl::at_c<param_types, 0>::type a0,
			typename boost::mpl::at_c<param_types, 1>::type a1
		) : ft(a0, a1)
		{
		}
		_(
			typename boost::mpl::at_c<param_types, 0>::type a0,
			typename boost::mpl::at_c<param_types, 1>::type a1,
			typename boost::mpl::at_c<param_types, 2>::type a2
		) : ft(a0, a1, a2)
		{
		}
	};
	typedef typename extender2::_ target_type;
	friend result_type operator->*(T& t, const target_type& c)
	{
		return boost::fusion::invoke(&C::func, boost::fusion::push_front(c.ft, boost::ref(t)));
	}
};

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
