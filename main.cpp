#include "guu.h"

// TODO: catch compilation errors

int main(int argc, char *argv[])
{
    guu::Program program(std::cin);
    program.run(std::cout);

    return 0;
}
