#include <fstream>

#include "guu.h"

// TODO: catch compilation errors
// DONE: throw runtime error if trying to use a variable, which is not set
// DONE: implement debugger
// TODO: command line flags parsing
// TODO: add option to disable debug output about execution

int main(int argc, char *argv[])
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);
    std::cout.tie(0);

    std::ifstream fileStream(argv[1]);

    guu::Program program(fileStream);
    program.run(std::cout, std::cerr, std::cin, true, true);

    return 0;
}
