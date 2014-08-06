#ifndef GENERATORS_TRANSLATOR_H
#define GENERATORS_TRANSLATOR_H

namespace meta {

class Function;
class VarDecl;
class Return;
class Call;
class Number;
class Var;
class Assigment;
class BinaryOp;

} // namespace meta

namespace generators {

template<typename Value>
class Translator
{
public:
    virtual ~Translator() {}

    virtual void startFunction(meta::Function *node) = 0;
    // Value consumers
    virtual void declareVar(meta::VarDecl *node, Value initialVal) = 0;
    virtual void returnValue(meta::Return *node, Value val) = 0;
    // Value providers
    virtual Value number(meta::Number *node) = 0;
    virtual Value var(meta::Var *node) = 0;
    // Operations on values
    virtual Value call(meta::Call *node, const std::vector<Value> &args) = 0;
    virtual Value assign(meta::Assigment *node, Value val) = 0;
    virtual Value binaryOp(meta::BinaryOp *node, Value left, Value right) = 0;
};

} // namespace generators

#endif // TRANSLATOR_H
