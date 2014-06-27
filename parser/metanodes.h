#ifndef META_NODES_H
#define META_NODES_H

#include <string>

#include "parser/metaparser.h"

namespace meta {

class Function: public Node
{
meta_NODE
public:
    Function(const StackFrame *start, size_t size);

    const std::string &name() const {return mName;}
private:
    std::string mName;
};

class Arg: public Node
{
meta_NODE
public:
    Arg(const StackFrame *start, size_t size);

    const std::string &name() const {return mName;}
    const std::string &type() const {return mType;}

private:
    std::string mName, mType;
};

class BinaryOp: public Node
{
meta_NODE
public:
    BinaryOp(const StackFrame *start, size_t size);

    enum Operation {add, sub, mul, div};
    Operation operation() const {return mOp;}

private:
    Operation mOp;
};

class Number: public Node
{
meta_NODE
public:
    Number(const StackFrame *start, size_t size);

    int value() const {return mValue;}

private:
    int mValue;
};

class Var: public Node
{
meta_NODE
public:
    Var(const StackFrame *start, size_t size);

    const std::string &name() const {return mName;}

private:
    std::string mName;
};

class Call: public Node
{
meta_NODE
public:
    Call(const StackFrame *reduction, size_t size);

    const std::string &functionName() const {return mFunctionName;}

private:
    std::string mFunctionName;
};

class Return: public Node
{
meta_NODE
public:
    Return(const StackFrame *reduction, size_t size);
};

} // namespace meta

#endif // META_NODES_H
