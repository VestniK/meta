#include <cassert>

#include "parser/metaparser.h"
#include "parser/visibility.h"

namespace meta {

Visibility fromToken(const Token &token)
{
    switch (token.termNum) {
        case meta::Export: return meta::Visibility::Export;
        case meta::Public: return meta::Visibility::Public;
        case meta::Protected: return meta::Visibility::Protected;
        case meta::Private: return meta::Visibility::Private;
        default: break;
    }
    assert(false && "unexpected token for visibility name");
    return Visibility::Default;
}

} // namespace meta
