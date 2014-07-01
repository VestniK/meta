#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>

#include "fs/io.h"

#include "parser/metaparser.h"
#include "parser/package.h"

#include "builder/builder.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Ussage: " << argv[0] << " SRC_FILE OUTPUT" << std::endl;
        return EXIT_FAILURE;
    }
    try {
        std::vector<char> input;
        readWholeFile(argv[1], input);
        meta::Parser parser;
        auto package = std::dynamic_pointer_cast<meta::Package>(parser.parse(input.data()));
        builder::build(package, argv[2]);
    } catch(const std::exception &err) {
        std::cerr << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
