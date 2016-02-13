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
 */
#include <cassert>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

#include "fs/io.h"

#include "parser/actions.h"
#include "parser/metaparser.h"

#include "typesystem/typesstore.h"

#include "analysers/metaprocessor.h"
#include "analysers/nodeexception.h"
#include "analysers/reachabilitychecker.h"
#include "analysers/resolver.h"
#include "analysers/semanticerror.h"
#include "analysers/typechecker.h"

#include "generators/llvmgen/generator.h"
#include "generators/cppgen/generator.h"

using namespace meta;

enum class ErrorVerbosity
{
    silent,
    brief,
    lineMarked,
    expectedTerms,
    parserStack
};

enum class Generator {llvm, cpp};

struct Options {
    ErrorVerbosity verbosity = ErrorVerbosity::expectedTerms;
    Generator generator = Generator::llvm;
    std::string output;
    std::vector<std::string> sources;
};

namespace po = boost::program_options;

std::istream& operator>> (std::istream& in, ErrorVerbosity& verbosity)
{
    std::string str;
    in >> str;
    if (str == "silent")
        verbosity = ErrorVerbosity::silent;
    else if (str == "brief")
        verbosity = ErrorVerbosity::brief;
    else if (str == "lineMarked")
        verbosity = ErrorVerbosity::lineMarked;
    else if (str == "expectedTerms")
        verbosity = ErrorVerbosity::expectedTerms;
    else if (str == "parserStack")
        verbosity = ErrorVerbosity::parserStack;
    else
        throw po::invalid_option_value(str);
    return in;
}

std::istream& operator>> (std::istream& in, Generator& generator)
{
    std::string str;
    in >> str;
    if (str == "llvm")
        generator = Generator::llvm;
    else if (str == "cpp")
        generator = Generator::cpp;
    else
        throw po::invalid_option_value(str);
    return in;
}

bool run(const Options &opts);

int main(int argc, char **argv) try
{
    Options opts;
    po::options_description desc("Command line options");
    desc.add_options()
        ("help,h", "Show help")
        ("version,v", "Show version")
        ("output,o", po::value<std::string>(&opts.output), "Specify output file path")
        ("verbosity", po::value<ErrorVerbosity>(&opts.verbosity), "Error description verbosity: silent, brief, lineMarked, expectedTerms(default), parserStack")
        ("generator,G", po::value<Generator>(&opts.generator), "Output generator: llvm(default), cpp")
        ("src", po::value<std::vector<std::string>>(&opts.sources), "Sources to compile")
    ;
    po::positional_options_description pos;
    pos.add("src", -1);
    try {
        auto parseRes = po::command_line_parser(argc, argv).options(desc).positional(pos).run();
        po::variables_map vm;
        po::store(parseRes, vm);
        po::notify(vm);
        if (vm.count("help") != 0) {
            std::cout << "Ussage: " << argv[0] << " [options] -o OUTPUT SRC_FILE..." << std::endl;
            std::cout << desc << std::endl;
            return EXIT_SUCCESS;
        } else if (vm.count("version") != 0) {
            std::cout << "0.0.0" << std::endl; // TODO: extract version from git tags
            return EXIT_SUCCESS;
        }
    } catch(std::exception &err) {
        std::cerr << "Error: " << err.what() << std::endl;
        std::cerr << "Ussage: " << argv[0] << " [options] -o OUTPUT SRC_FILE..." << std::endl;
        std::cerr << desc << std::endl;
        return EXIT_FAILURE;
    }
    return run(opts) ? EXIT_SUCCESS : EXIT_FAILURE;
} catch(const analysers::NodeException &err) {
    std::cerr <<
        err.sourcePath() << ':' << err.tokens().begin()->line <<
        ':' << err.tokens().begin()->column << ": " << err.what() <<
        ":" << std::endl
    ;
    std::cerr << err.tokens().lineStr() << "..." << std::endl;
    for (int i = 1; i < err.tokens().colnum(); ++i)
        std::cerr << ' ';
    std::cerr << '^' << std::endl;
    for (const auto &frame: err.backtrace())
        std::cerr << "\t" << frame << std::endl;
    return false;
} catch(const utils::Exception &err) {
    std::cerr << "Internal compiler error: " << err.what() << std::endl;
    for (const auto &frame: err.backtrace())
        std::cerr << "\t" << frame << std::endl;
    return EXIT_FAILURE;
}

bool run(const Options &opts) try
{
    // parse
    std::vector<std::vector<char>> input;
    input.reserve(opts.sources.size());
    Parser parser;
    Actions act;
    parser.setParseActions(&act);
    parser.setNodeActions(&act);
    for (const auto &src: opts.sources) {
        input.emplace_back(readWholeFile(src));
        parser.setSourcePath(src);
        parser.parse(input.back().data(), input.back().size());
    }
    auto ast = parser.ast();
    // analyse
    analysers::resolve(ast, act.dictionary());
    typesystem::TypesStore typestore;
    analysers::checkTypes(ast, typestore);
    analysers::checkReachability(ast);
    analysers::processMeta(ast);
    // generate
    switch (opts.generator) {
        case Generator::llvm: generators::llvmgen::createLlvmGenerator()->generate(ast, opts.output); break;
        case Generator::cpp: generators::cppgen::createCppGenerator(act.dictionary())->generate(ast, opts.output); break;
    }

    return true;
} catch(const SyntaxError &err) {
    if (opts.verbosity > ErrorVerbosity::silent) {
        std::cerr <<
            err.sourcePath() << ':' << err.token().line << ':' <<
            err.token().column << ": " << err.what()
        ;
    }
    if (opts.verbosity == ErrorVerbosity::brief)
        std::cerr << std::endl;
    if (opts.verbosity > ErrorVerbosity::brief) {
        std::cerr << ':' << std::endl;
        std::cerr << err.line() << std::endl;
    }
    if (opts.verbosity > ErrorVerbosity::lineMarked)
        std::cerr << "Expected one of the following terms:" << std::endl << err.expected();
    if (opts.verbosity > ErrorVerbosity::expectedTerms)
        std::cerr << "Parser stack dump:" << std::endl << err.parserStack();
    return false;
} catch(const analysers::SemanticError &err) {
    if (opts.verbosity > ErrorVerbosity::silent)
        std::cerr <<
            err.sourcePath() << ':' << err.tokens().begin()->line <<
            ':' << err.tokens().begin()->column << ": " << err.what() <<
            (opts.verbosity == ErrorVerbosity::brief ? "" : ":") << std::endl
        ;
    if (opts.verbosity > ErrorVerbosity::brief) {
        std::cerr << err.tokens().lineStr() << "..." << std::endl;
        for (int i = 1; i < err.tokens().colnum(); ++i)
            std::cerr << ' ';
        std::cerr << '^' << std::endl;
    }
    return false;
}
