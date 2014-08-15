/*
 * Meta language compiler
 * Copyright (C) 2014  Sergey Vidyuk <sir.vestnik@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <cassert>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>

#include "fs/io.h"

#include "parser/metaparser.h"
#include "parser/package.h"

#include "analysers/resolver.h"
#include "analysers/semanticerror.h"

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
        std::unique_ptr<meta::AST> ast(parser.parse(input.data(), input.size()));
        // analyse
        analysers::resolve(ast.get());
        // generate
        std::unique_ptr<generators::Generator> gen(generators::llvmgen::createLlvmGenerator());
        gen->generate(ast.get(), argv[2]);
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
    } catch(const analysers::SemanticError &err) {
        if (verbosity > silent)
            std::cerr << argv[1] << ':' << err.tokens().begin()->line << ':' << err.tokens().begin()->column << ": " << err.what() << (verbosity == brief ? "" : ":") << std::endl;
        if (verbosity > brief) {
            std::cerr << err.tokens().lineStr() << "..." << std::endl;
            for (int i = 1; i < err.tokens().colnum(); ++i)
                std::cerr << ' ';
            std::cerr << '^' << std::endl;
        }
        return EXIT_FAILURE;
    } catch(const std::exception &err) {
        std::cerr << "Internal compiler error: " << err.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
