#ifndef FS_IO_H
#define FS_IO_H

#include <string>
#include <vector>

void readWholeFile(const std::string &path, std::vector<char> &dst);

#endif // FS_IO_H
