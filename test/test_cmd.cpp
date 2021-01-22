#include <string>
#include <iostream>
#include "cmd.hpp"

int main()
{
    auto result = tana::cmd::exec("objdump -d /usr/bin/top");
    std::cout << result << std::endl;
    return 0;
}