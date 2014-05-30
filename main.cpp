#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>

#include "parser/metaparser.h"
#include "fs/io.h"

int main(int argc, char **argv)
{
    if (argc != 2) {
        std::cerr << "Ussage: " << argv[0] << " SRC_FILE" << std::endl;
        return EXIT_FAILURE;
    }
    try {
        std::vector<char> src;
        readWholeFile(argv[1], src);

        meta::Parser parser;
        parser.parse(src.data());
    } catch(const std::exception &err) {
        std::cerr << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
