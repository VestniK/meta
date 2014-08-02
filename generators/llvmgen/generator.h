#ifndef BUILDER_H
#define BUILDER_H

#include <string>
#include <memory>

#include "parser/metaparser.h"
#include "parser/package.h"

namespace generators {
namespace llvmgen {

void generate(std::shared_ptr<meta::Package> pkg, const std::string &output);

} // namespace llvmgen
} // namespace generators

#endif // CODEGEN_H
