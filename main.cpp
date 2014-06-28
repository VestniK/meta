#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>

#include "parser/package.h"
#include "codegen/codegen.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Ussage: " << argv[0] << " SRC_FILE OUTPUT" << std::endl;
        return EXIT_FAILURE;
    }
    try {
        Package package;
        package.parse(argv[1]);
        CodeGen codegen(package.name);
        for (auto func : package.functions)
            func.second->walk(&codegen);
        codegen.save(argv[2]);
    } catch(const std::exception &err) {
        std::cerr << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
