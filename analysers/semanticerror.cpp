#include "parser/metaparser.h"

#include "analysers/semanticerror.h"

namespace analysers {

SemanticError::SemanticError(meta::Node *node, const std::string &msg):
    mMsg(msg),
    mTokens(node->tokens())
{
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
