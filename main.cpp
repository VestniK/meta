#include <cassert>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "parser/metaparser.h"

void readWholeFile(const std::string &path, std::vector<char> &dst)
{
    std::ifstream in(path, std::ifstream::in | std::ifstream::binary);
    if (!in)
        throw std::runtime_error("Failed to open file: " + path);
    in.seekg(0, std::ifstream::end);
    std::streampos fileSize = in.tellg();
    dst.resize(std::size_t(fileSize) + 1);
    in.seekg(0, std::ifstream::beg);
    in.read(dst.data(), fileSize);
    if (!in.good())
        throw std::runtime_error("Failed to read file content: " + path);
    assert(in.gcount() == fileSize);
    dst[fileSize] = 0;
}

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
