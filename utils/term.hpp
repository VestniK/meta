#include <iostream>

#include "utils/term.h"

namespace meta::utils {

std::ostream& operator<< (std::ostream& out, TermColor color) {
    switch (color) {
        case TermColor::black: out << "\e[30m"; break;
        case TermColor::red: out << "\e[31m"; break;
        case TermColor::green: out << "\e[32m"; break;
        case TermColor::yellow: out << "\e[33m"; break;
        case TermColor::blue: out << "\e[34m"; break;
        case TermColor::magenta: out << "\e[35m"; break;
        case TermColor::cyan: out << "\e[36m"; break;
        case TermColor::white: out << "\e[37m"; break;
        case TermColor::none: out << "\e[0m"; break;
    }
    return out;
}

} // namespace meta::utils
