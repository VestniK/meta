#include <cassert>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>

#include "fs/io.h"

#include "parser/metaparser.h"
#include "parser/package.h"

#include "generators/llvmgen/generator.h"

int main(int argc, char **argv)
{
    if (argc != 3) {
        std::cerr << "Ussage: " << argv[0] << " SRC_FILE OUTPUT" << std::endl;
        return EXIT_FAILURE;
    }
    const enum ErrorVerbosity {silent, brief, lineMarked, expectedTerms, parserStack} verbosity = expectedTerms;
    try {
        // read
        std::vector<char> input;
        readWholeFile(argv[1], input);
        // parse
        meta::Parser parser;
        auto ast = parser.parse(input.data(), input.size());
        auto package = ast.getChildren<meta::Package>();
        assert(package.size() == 1);
        // generate
        std::unique_ptr<generators::Generator> gen(generators::llvmgen::createLlvmGenerator());
        gen->generate(package.front(), argv[2]);
    } catch(const meta::SyntaxError &err) {
        if (verbosity > silent)
            std::cerr << argv[1] << ':' << err.token().line << ':' << err.token().column << ": " << err.what();
        if (verbosity == brief)
            std::cerr << std::endl;
        if (verbosity > brief) {
            std::cerr << ':' << std::endl;
            std::cerr << err.line() << std::endl;
        }
        if (verbosity > lineMarked)
            std::cerr << "Expected one of the following terms:" << std::endl << err.expected();
        if (verbosity > expectedTerms)
            std::cerr << "Parser stack dump:" << std::endl << err.parserStack();
        return EXIT_FAILURE;
    } catch(const std::exception &err) {
        std::cerr << "Internal compiler error: " << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
