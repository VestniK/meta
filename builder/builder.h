#ifndef BUILDER_H
#define BUILDER_H

#include <string>
#include <memory>

#include "parser/metaparser.h"
#include "parser/package.h"

namespace builder {

void build(std::shared_ptr<meta::Package> pkg, const std::string &output);

} // namespace builder

#endif // CODEGEN_H
