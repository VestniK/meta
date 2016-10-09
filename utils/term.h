#pragma once

#include <iosfwd>

namespace meta::utils {

enum class TermColor {
    black, red, green, yellow, blue, magenta, cyan, white, none
};

std::ostream& operator<< (std::ostream& out, TermColor color);

} // namespace meta::utils
