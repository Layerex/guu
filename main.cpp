#include "guu.h"

// TODO: catch compilation errors

int main(int argc, char *argv[])
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);
    std::cout.tie(0);

    guu::Program program(std::cin);
    program.run(std::cout);

    return 0;
}
