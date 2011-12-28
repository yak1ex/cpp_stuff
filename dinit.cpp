#include <vector>
#include <iostream>

#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/define_struct.hpp>
#include <boost/array.hpp>
#include <boost/fusion/include/boost_array.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/fusion/include/boost_tuple.hpp>
#include <boost/fusion/include/adapt_assoc_struct.hpp>

#include <boost/fusion/include/out.hpp>

#include "dinit.hpp"

// Helper function

template<typename T>
void output(const T& v)
{
	for(auto val : v) {
		std::cout << ' ' << val;
	}
	std::cout << std::endl;
}

// Type definition and adapter

// Don't work for BOOST_FUSION_DEFINE_STRUCT becaues conflict with constructor

struct employee
{
	std::string name;
	int age;
	double rate;
};

BOOST_FUSION_ADAPT_STRUCT(
	employee,
	(std::string, name)
	(int, age)
	(double, rate)
)

namespace keys {
	struct name;
	struct age;
	struct rate;
}

struct employee_
{
	std::string name;
	int age;
	double rate;
};


BOOST_FUSION_ADAPT_ASSOC_STRUCT(
	employee_,
	(std::string, name, keys::name)
	(int, age, keys::age)
	(double, rate, keys::rate)
)

// Don't work for plain array because brace-enclosed initializer is required.

int main(void)
{
	using yak::util::dinit::di;
	using yak::util::dinit::idx;
	using yak::util::dinit::idx_;
	using yak::util::dinit::key;
	using yak::util::dinit::key_;

// For sequence with list constructor

	std::vector<int> v = di(idx<10>() = 1, idx<0>() = 2, idx<2>() = 3, idx<3>() = 4);
	output(v);

	std::vector<int> v2 = di(idx_<10>(1), idx_<0>(2), idx_<2>(3), idx_<3>(4));
	output(v2);

// For Fusion sequence with idx

	employee e = di(idx<2>() = 3.3, idx<1>() = 33);
	boost::fusion::out(std::cout, e); std::cout << std::endl;

	boost::array<int, 5> ar2 = di(idx<3>() = 4);
	output(ar2);

	boost::tuple<std::string, int, double> t = di(idx<1>() = 5);
	boost::fusion::out(std::cout, t); std::cout << std::endl;

// For Fusion sequence with tag

	employee_ e2 = di(key<keys::rate>() = 3.3, key<keys::age>() = 33);
	boost::fusion::out(std::cout, e2); std::cout << std::endl;

	employee_ e3 = di(key_<keys::rate>(3.3), key_<keys::age>(33));
	boost::fusion::out(std::cout, e3); std::cout << std::endl;

	return 0;
}
