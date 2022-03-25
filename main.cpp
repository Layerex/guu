#include "guu.h"

// TODO: catch compilation errors
// TODO: using variables and calling function before declaration
// TODO: use Number by default in std::variant

int main(int argc, char *argv[])
{
    guu::Program program(std::cin);
    program.run(std::cout);

    return 0;
}
