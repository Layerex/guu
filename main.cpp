#include "guu.h"

// TODO: catch compilation errors
// DONE: throw runtime error if trying to use a variable, which is not set
// TODO: implement call debugger
// TODO: command line flags parsing
// TODO: add option to disable debug output about execution

int main(int argc, char *argv[])
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);
    std::cout.tie(0);

    guu::Program program(std::cin);
    program.run(std::cout);

    return 0;
}
