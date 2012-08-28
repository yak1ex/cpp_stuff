#include <iostream>
#include "extender.hpp"

namespace test {

struct A
{
	int n;
};

}

namespace ext {

struct func1_ : public yak::util::extender1<func1_, test::A, bool(int)>
{
	static bool func(test::A& a, int n) // "func" is fixed name
	{
		a.n += n;
		std::cout << a.n << ':' << n << std::endl;
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

BEGIN_EXTENDER(test::A, (test::A&)(func3)(((test::A&, a))((int, n))))
{
	a.n += n;
	std::cout << a.n << ':' << n << std::endl;
	return a;
}
END_EXTENDER()

}

int main(void)
{
	using ext::func1;
	using ext::func2;
	using ext::func3;

	test::A a = { 0 };
	(a->*func1)(1);
// NOTE: different from ordinary operator semantics/precedence
	a->*func2::_(3)->*func3(5);
	return 0;
}
