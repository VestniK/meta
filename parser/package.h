#ifndef PACKAGE_H
#define PACKAGE_H

#include <map>
#include <string>

#include "parser/metanodes.h"

struct Package
{
    std::string name;
    std::map<std::string, std::shared_ptr<meta::Function> > functions;

    void parse(const std::string &sourcePath);
    void parse(const std::string &sourcePath, const char *content);
};

#endif // PACKAGE_H
