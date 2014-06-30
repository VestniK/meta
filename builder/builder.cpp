#include <cassert>
#include <map>
#include <stack>
#include <stdexcept>
#include <vector>

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Bitcode/ReaderWriter.h>
#include <llvm/Support/raw_ostream.h>

#include "parser/metanodes.h"

#include "astutils/childrengatherer.h"

#include "builder/builder.h"
#include "builder/environment.h"

namespace builder {

struct BuildContext
{
    BuildContext(): currFunc(nullptr), builder(llvm::getGlobalContext()) {}

    llvm::Function *currFunc;
    std::map<std::string, llvm::Value*> varMap;

    llvm::IRBuilder<> builder;
    std::stack<llvm::Value*> evaluationStack;
};

class CodeGen: public meta::Visitor
{
public:
    CodeGen(Environment &env);
    virtual ~CodeGen();

    virtual void visit(meta::Function *node) override;

    virtual void visit(meta::Call *) override;

    virtual void visit(meta::Number *node) override;
    virtual void visit(meta::Var *node) override;

    virtual void leave(meta::BinaryOp *node) override;
    virtual void leave(meta::Return *node) override;

    void save(const std::string &path);

private:
    Environment &env;
    std::unique_ptr<BuildContext> buildContext;
};

void build(Package& pkg, const std::string& output)
{
    Environment env(pkg.name);
    for (auto func : pkg.functions)
        env.addFunction(func.second.get());
    CodeGen codegen(env);
    for (auto func : pkg.functions)
        func.second->walk(&codegen);
    codegen.save(output);
}

CodeGen::CodeGen(Environment &env):
    env(env),
    buildContext(new BuildContext)
{
}

CodeGen::~CodeGen()
{
}

void CodeGen::visit(meta::Function *node)
{
    buildContext->varMap.clear();
    buildContext->currFunc = env.module->getFunction(node->name());

    astutils::ChildrenGatherer<meta::Arg> argGatherer;
    node->walk(&argGatherer);

    llvm::Function::arg_iterator it = buildContext->currFunc->arg_begin();
    for (const auto arg : argGatherer.gathered()) {
        buildContext->varMap[arg->name()] = it; /// @todo check name collisions
        assert(it != buildContext->currFunc->arg_end());
        ++it;
    }
    assert(it == buildContext->currFunc->arg_end());

    llvm::BasicBlock *body = llvm::BasicBlock::Create(env.context, "", buildContext->currFunc);
    buildContext->builder.SetInsertPoint(body);
}

void CodeGen::visit(meta::Call *)
{
    /// @todo add call functions support
    throw std::runtime_error("Not implemented yet");
}

void CodeGen::visit(meta::Number *node)
{
    llvm::Type *intType = llvm::Type::getInt32Ty(env.context);
    buildContext->evaluationStack.push(llvm::ConstantInt::get(intType, node->value(), true));
}

void CodeGen::visit(meta::Var *node)
{
    auto it = buildContext->varMap.find(node->name());
    if (it == buildContext->varMap.end())
        throw std::runtime_error(std::string("Undefined variable ") + node->name());
    buildContext->evaluationStack.push(it->second);
}

void CodeGen::leave(meta::BinaryOp* node)
{
    assert(buildContext->evaluationStack.size() >=2);
    llvm::Value *right = buildContext->evaluationStack.top();
    buildContext->evaluationStack.pop();
    llvm::Value *left = buildContext->evaluationStack.top();
    switch (node->operation()) {
        case meta::BinaryOp::add: buildContext->evaluationStack.top() = buildContext->builder.CreateAdd(left, right); break;
        case meta::BinaryOp::sub: buildContext->evaluationStack.top() = buildContext->builder.CreateSub(left, right); break;
        case meta::BinaryOp::mul: buildContext->evaluationStack.top() = buildContext->builder.CreateMul(left, right); break;
        case meta::BinaryOp::div: buildContext->evaluationStack.top() = buildContext->builder.CreateSDiv(left, right); break;
        default: assert(false);
    }
}

void CodeGen::leave(meta::Return* node)
{
    assert(buildContext->evaluationStack.size() == 1);
    buildContext->builder.CreateRet(buildContext->evaluationStack.top());
    buildContext->evaluationStack.pop();
}

void CodeGen::save(const std::string& path)
{
    std::string errBuf;
    llvm::raw_fd_ostream out(path.c_str(), errBuf, llvm::sys::fs::F_Binary);
    llvm::WriteBitcodeToFile(env.module.get(), out);
    out.close();
    if (out.has_error())
        throw std::runtime_error(errBuf);
}

} // namespace builder
