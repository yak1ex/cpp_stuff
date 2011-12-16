#include <iostream>

// CAUTION: THIS CODE IS FOR GCC ONLY BECAUSE IT DEPENDS ON __builtin_scalbn()
#include "hexfloat.hpp"

int main(void)
{
#if __GNUC__ > 4 || __GNUC__ == 4 && __GNUC_MINOR__ >= 7
    using yak::util::hexfloat::operator "" _hf;

    std::cout << 0xF.ap+1_hf << std::endl;
    std::cout << 0xF.ap+2_hf << std::endl;
    std::cout << 0xF.a0p+1_hf << std::endl;
    std::cout << 0Xf.AP+1_hf << std::endl;
    std::cout << 0Xf.AP-0_hf << std::endl;
    std::cout << 0Xf.AP+0_hf << std::endl;
    std::cout << 0x0.AP+0_hf << std::endl;
    std::cout << 0X.AP+0_hf << std::endl;
    std::cout << 0xF.p0_hf << std::endl;
#else
    using yak::util::hexfloat::parse;

    std::cout << parse<'0','x','F','.','a','p','+','1'>() << std::endl;
    std::cout << parse<'0','x','F','.','a','p','+','2'>() << std::endl;
    std::cout << parse<'0','x','F','.','a','0','p','+','1'>() << std::endl;
    std::cout << parse<'0','X','f','.','A','P','+','1'>() << std::endl;
    std::cout << parse<'0','X','f','.','A','P','-','0'>() << std::endl;
    std::cout << parse<'0','X','f','.','A','P','+','0'>() << std::endl;
    std::cout << parse<'0','x','0','.','A','P','+','0'>() << std::endl;
    std::cout << parse<'0','X','.','A','P','+','0'>() << std::endl;
    std::cout << parse<'0','x','F','.','p','0'>() << std::endl;
#endif

    return 0;
}
