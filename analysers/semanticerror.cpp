#include <cstdarg>
#include <cstdio>

#include "parser/metaparser.h"

#include "analysers/semanticerror.h"

namespace analysers {

SemanticError::SemanticError(meta::Node *node, const char *format, ...):
    mTokens(node->tokens())
{
    va_list args;
    va_start(args, format);
    const int size = vsnprintf(nullptr, 0, format, args) + 1;
    char buf[size];
    va_start(args, format);
    vsnprintf(buf, size, format, args);
    mMsg.assign(buf, size);
    mTokens.detach(mErrContext);
}

SemanticError::~SemanticError()
{
}

const char *SemanticError::what() const noexcept
{
    return mMsg.c_str();
}

} // namespace analysers
