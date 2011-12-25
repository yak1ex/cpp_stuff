#include <vector>
#include <iostream>

#include "dinit.hpp"

int main(void)
{
	using yak::util::dinit::di;
	using yak::util::dinit::idx;
	using yak::util::dinit::idx_;

	std::vector<int> v = di(idx<10>() = 1, idx<0>() = 2, idx<2>() = 3, idx<3>() = 4);
	for(auto val : v) {
		std::cout << val << std::endl;
	}
	std::vector<int> v2 = di(idx_<10>(1), idx_<0>(2), idx_<2>(3), idx_<3>(4));
	for(auto val : v2) {
		std::cout << val << std::endl;
	}
	return 0;
}
