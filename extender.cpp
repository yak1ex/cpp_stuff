#include <iostream>
#include "extender.hpp"

namespace test {

struct A
{
	int n;
};

}

namespace ext {

struct func1_ : public yak::util::extender1<func1_, test::A>
{
	typedef test::A& result_type; // MUST follow result_of protocol
	result_type operator()(test::A& a, int n) const
	{
		a.n += n;
		std::cout << "int&: " << a.n << ':' << n << std::endl;
		return a;
	}
	result_type operator()(const test::A& a, int n) const
	{
		std::cout << "[const] int&: " << a.n << ':' << n << std::endl;
		return const_cast<test::A&>(a); // Just as test purpose
	}
	result_type operator()(test::A& a, double n) const
	{
		a.n += n;
		std::cout << "double&: " << a.n << ':' << n << std::endl;
		return a;
	}
	result_type operator()(test::A& a, const int& n, int& m) const
	{
		a.n += n + m;
		std::cout << "const int&, int&: " << a.n << ':' << n << ',' << m << std::endl;
		return a;
	}
	result_type operator()(test::A& a, const int& n, const int& m) const
	{
		a.n += n + m;
		std::cout << "const int&, const int&: " << a.n << ':' << n << ',' << m << std::endl;
		return a;
	}
} func1;

struct func2 : public yak::util::extender2<func2, test::A, test::A&(int)>
{
	static test::A& func(test::A& a, int n) // "func" is fixed name
	{
		a.n += n;
		std::cout << a.n << ':' << n << std::endl;
		return a;
	}
};

#ifdef YAK_UTIL_EXTENDER3_ENABLED
DEFINE_EXTENDER(test::A, func3, 
	{
		// MUST follow result_of protocol
		template<typename>
		struct result;
		template<typename F, typename T>
		struct result<F(test::A&, T)> { typedef test::A& type; };
		template<typename F, typename T, typename U>
		struct result<F(test::A&, T, U)> { typedef test::A& type; };
		template<typename F, typename T>
		struct result<F(const test::A&, T)> { typedef const test::A& type; };
		template<typename F, typename T, typename U>
		struct result<F(const test::A&, T, U)> { typedef const test::A& type; };
//		If there is no case that const test::A& is return type, you need specify only the following line
//		typedef test::A& result_type;
		test::A& operator()(test::A& a, int n) const
		{
			a.n += n;
			std::cout << "int&: " << a.n << ':' << n << std::endl;
			return a;
		}
		const test::A& operator()(const test::A& a, int n) const
		{
			std::cout << "[const] int&: " << a.n << ':' << n << std::endl;
			return a;
		}
		test::A& operator()(test::A& a, double n) const
		{
			a.n += n;
			std::cout << "double&: " << a.n << ':' << n << std::endl;
			return a;
		}
		test::A& operator()(test::A& a, const int& n, int& m) const
		{
			a.n += n + m;
			std::cout << "const int&, int&: " << a.n << ':' << n << ',' << m << std::endl;
			return a;
		}
		test::A& operator()(test::A& a, const int& n, const int& m) const
		{
			a.n += n + m;
			std::cout << "const int&, const int&: " << a.n << ':' << n << ',' << m << std::endl;
			return a;
		}
	}
);
/* expanded as
struct func3_functor : public yak::util::extender3<func3_functor, test::A>
{
	struct _
	{ specified codes };
} func3;
*/

#endif

}

int f() { return 0; }

int main(void)
{
	using ext::func1;
	using ext::func2;
#ifdef YAK_UTIL_EXTENDER3_ENABLED
	using ext::func3;
#endif

	int n = 0;
	test::A a = { 0 };
	((a->*func1)(1)->*func1)(1.1); // Cascading but unintuitive
	(const_cast<const test::A&>(a)->*func1)(1);
	(a->*func1)(2, 3);
	(a->*func1)(2, f());
	(a->*func1)(2, n);
// NOTE: different from ordinary operator semantics/precedence
#ifdef YAK_UTIL_EXTENDER3_ENABLED
	a->*func2::_(3)->*func3(5);
	a->*func3(1)->*func3(1.1)->*func3(2, 3)->*func3(2, f())->*func3(2, n);
	const_cast<const test::A&>(a)->*func3(1);
#else
	a->*func2::_(3);
#endif

	return 0;
}
