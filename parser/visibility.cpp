#include <cassert>
#include <iostream>

#include "parser/metaparser.h"
#include "parser/visibility.h"

namespace meta {

Visibility fromToken(const Token &token) {return fromTerm(token.termNum);}

Visibility fromTerm(int term) {
    switch (term) {
        case Extern: return Visibility::Extern;
        case Export: return Visibility::Export;
        case Public: return Visibility::Public;
        case Protected: return Visibility::Protected;
        case Private: return Visibility::Private;
        default: break;
    }
    assert(false && "unexpected term for visibility name");
    return Visibility::Default;
}

std::ostream& operator<< (std::ostream& out, Visibility val) {
    switch (val) {
    case Visibility::Export: out << "export"; break;
    case Visibility::Extern: out << "extern"; break;
    case Visibility::Public: out << "public"; break;
    case Visibility::Private: out << "private"; break;
    case Visibility::Protected: out << "protected"; break;
    case Visibility::Default: out << "default"; break;
    }
    return out;
}

} // namespace meta
