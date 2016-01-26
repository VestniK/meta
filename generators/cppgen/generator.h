#pragma once

#include <memory>

#include <generators/generator.h>

namespace meta {
namespace generators {
namespace cppgen {

std::unique_ptr<Generator> createCppGenerator();

}}} //namespace meta::generators::cppgen
