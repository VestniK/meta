#include <cassert>
#include <fstream>
#include <stdexcept>

#include "fs/io.h"

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