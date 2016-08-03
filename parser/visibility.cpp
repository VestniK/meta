#include <cassert>

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

} // namespace meta
