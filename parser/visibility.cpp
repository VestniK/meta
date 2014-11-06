#include <cassert>

#include "parser/metaparser.h"
#include "parser/visibility.h"

namespace meta {

Visibility fromToken(const Token &token)
{
    switch (token.termNum) {
        case Extern: return Visibility::Extern;
        case Export: return Visibility::Export;
        case Public: return Visibility::Public;
        case Protected: return Visibility::Protected;
        case Private: return Visibility::Private;
        default: break;
    }
    assert(false && "unexpected token for visibility name");
    return Visibility::Default;
}

} // namespace meta
