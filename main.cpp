#include <cstring>
#include <fstream>

#include "guu.h"

#define HELP "Usage: " << argv[0] << " [FILE] [-d|--debug] [-l|--log]\n"

int main(int argc, char *argv[])
{
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(0);

    if (argc < 2) {
        std::cerr << "No file specified.\n";
        std::cerr << HELP;
        return 2;
    }

    bool debug = false;
    bool log = false;
    for (int i = 2; i < argc; ++i) {
        if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")) {
            debug = true;
        } else if (!strcmp(argv[i], "-l") || !strcmp(argv[i], "--log")) {
            log = true;
        } else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
            std::cerr << HELP;
            return 0;
        } else {
            std::cerr << "Unknown argument: `" << argv[i] << "'.\n";
            std::cerr << HELP;
            return 2;
        }
    }

    std::ifstream fileStream(argv[1]);
    if (!fileStream.is_open()) {
        std::cerr << "Can't open file `" << argv[1] << "'.\n";
        return 1;
    }

    try {
        guu::Program program(fileStream);
        try {
            program.run(std::cout, std::cerr, std::cin, debug, log);
        } catch (std::runtime_error &e) {
            std::cerr << "Runtime error: " << e.what() << ".\n";
            return 1;
        }
    } catch (std::runtime_error &e) {
        std::cerr << "Compilation error: " << e.what() << ".\n";
        return 1;
    }

    return 0;
}
