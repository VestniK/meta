#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>

#include "parser/package.h"
#include "builder/builder.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Ussage: " << argv[0] << " SRC_FILE OUTPUT" << std::endl;
        return EXIT_FAILURE;
    }
    try {
        Package package;
        package.parse(argv[1]);
        builder::build(package, argv[2]);
    } catch(const std::exception &err) {
        std::cerr << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
