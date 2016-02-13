#pragma once

#include <memory>

#include "parser/dictionary.h"

#include "generators/generator.h"

namespace meta {
namespace generators {
namespace cppgen {

std::unique_ptr<Generator> createCppGenerator(const Dictionary& dict);

}}} //namespace meta::generators::cppgen
