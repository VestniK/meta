#ifndef BUILDER_H
#define BUILDER_H

#include <string>
#include <memory>

#include "generators/generator.h"

namespace generators {
namespace llvmgen {

generators::Generator *createLlvmGenerator();

} // namespace llvmgen
} // namespace generators

#endif // CODEGEN_H
