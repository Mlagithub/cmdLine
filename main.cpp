#include <iostream>
#include <string>

#include "cmdLine.h"


int main(int argc, char** argv)
{
    CmdLine cl{};
    cl.regist<std::string, CmdLine::MustOffer>("m", "mesh", "mesh file name."," ");
    cl.regist<size_t, CmdLine::MustOffer>("np", "npart", "number to be cutting."," ");
    cl.parse(argc, argv);

    std::cout << cl.get<int>("np") << '\n';
    
    return 0;
}