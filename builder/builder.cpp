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
    BuildContext(Environment &env): builder(env.context) {}

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
    virtual void leave(meta::Call *node) override;

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
    buildContext(new BuildContext(env))
{
}

CodeGen::~CodeGen()
{
}

void CodeGen::visit(meta::Function *node)
{
    buildContext->varMap.clear();
    llvm::Function *func = env.module->getFunction(node->name());
    assert(func); // All functions should be registered in the environment before compilation

    astutils::ChildrenGatherer<meta::Arg> argGatherer;
    node->walk(&argGatherer);

    llvm::Function::arg_iterator it = func->arg_begin();
    for (const auto arg : argGatherer.gathered()) {
        buildContext->varMap[arg->name()] = it; /// @todo check name collisions
        assert(it != func->arg_end());
        ++it;
    }
    assert(it == func->arg_end());

    llvm::BasicBlock *body = llvm::BasicBlock::Create(env.context, "", func);
    buildContext->builder.SetInsertPoint(body);
}

void CodeGen::visit(meta::Call *)
{
    buildContext->evaluationStack.push(nullptr); // Marker for the result operation value
}

void CodeGen::leave(meta::Call *node)
{
    llvm::Function *func = env.module->getFunction(node->functionName());
    /// @todo this kind of checks should be done before compilation
    if (func == nullptr)
        throw std::runtime_error(std::string("Call of unknown function " + node->functionName()));
    std::vector<llvm::Value*> args;
    for (; buildContext->evaluationStack.top() != nullptr; buildContext->evaluationStack.pop())
        args.push_back(buildContext->evaluationStack.top());
    if (func->arg_size() != args.size())
        throw std::runtime_error(std::string("Call of the function ") + node->functionName() + " with incorrect number of arguments");
    buildContext->evaluationStack.top() = buildContext->builder.CreateCall(func, args);
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
