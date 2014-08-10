#ifndef SEMANTICERROR_H
#define SEMANTICERROR_H

#include <exception>
#include <string>

#include "parser/metalexer.h"

namespace meta {

class Node;

} // namespace meta

namespace analysers {

class SemanticError: public std::exception
{
public:
    SemanticError(meta::Node *node, const std::string &msg);
    ~SemanticError();

    virtual const char *what() const noexcept override;
    const meta::TokenSequence &tokens() const {return mTokens;}

private:
    std::string mMsg;
    std::string mErrContext;
    meta::TokenSequence mTokens;
};

} // namespace analysers

#endif // SEMANTICERROR_H
