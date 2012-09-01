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

DEFINE_EXTENDER1(test::A, func1_,
	{
		typedef void result_type;
		result_type operator()(const test::A&) { std::cout << "func1_" << std::endl; }
	}
);

DEFINE_EXTENDER2(test::A, func2, 
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
struct func2_functor : public yak::util::extender2<func2_functor, test::A>
{
	struct _
	{ specified codes };
} func2;
*/

}

int f() { return 0; }

int main(void)
{
	using ext::func1;
	using ext::func1_;
	using ext::func2;

	int n = 0;
	test::A a = { 0 };
	(a->*func1_)();
	((a->*func1)(1)->*func1)(1.1); // Cascading but unintuitive
	(const_cast<const test::A&>(a)->*func1)(1);
	(a->*func1)(2, 3);
	(a->*func1)(2, f());
	(a->*func1)(2, n);
// NOTE: different from ordinary operator semantics/precedence
	a->*func2(1)->*func2(1.1)->*func2(2, 3)->*func2(2, f())->*func2(2, n);
	const_cast<const test::A&>(a)->*func2(1);

	return 0;
}
