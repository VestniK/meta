#include <iostream>
#include <fstream>

#include "utils/contract.h"

#include "typesystem/type.h"

#include "parser/metaparser.h"
#include "parser/function.h"
#include "parser/vardecl.h"

#include "generator.h"

namespace meta {
namespace generators{
namespace cppgen {

namespace {

bool isSubpackage(const utils::string_view& child, const utils::string_view& parent) {
    if (child.size() < parent.size())
        return false;
    if (child.size() == parent.size())
        return child == parent;
    return child.substr(0, parent.size()) == parent && child[parent.size()] == '.';
}

utils::string_view parent(const utils::string_view& package) {
    auto pos = package.rfind('.');
    if (pos == utils::string_view::npos)
        return {};
    return package.substr(0, pos);
}

std::tuple<utils::string_view, utils::string_view> split(const utils::string_view& string, char sep) {
    const auto pos = string.find(sep);
    if (pos == utils::string_view::npos)
        return std::make_tuple(string, utils::string_view{});
    if (pos == 0)
        return std::make_tuple(utils::string_view{}, string);
    return std::make_tuple(string.substr(0, pos), string.substr(pos + 1));
}

utils::string_view stripParent(const utils::string_view& package, const utils::string_view& parent) {
    PRECONDITION(parent.empty() || isSubpackage(package, parent));
    if (parent.empty())
        return package;
    if (package.size() == parent.size())
        return {};
    return package.substr(parent.size() + 1);
}

class CppWriter {
public:
    CppWriter(std::ostream& out): mOut(&out) {}
    CppWriter(const CppWriter&) = delete;
    CppWriter& operator= (const CppWriter&) = delete;
    CppWriter(CppWriter&&) = default;
    CppWriter& operator= (CppWriter&&) = default;

    ~CppWriter() {
        for (; !mPackage.empty(); mPackage = parent(mPackage))
            (*mOut) << '}' << std::endl;
    }

    void setPackage(const utils::string_view& pkg) {
        for (; !mPackage.empty() && !isSubpackage(pkg, mPackage); mPackage = parent(mPackage))
            (*mOut) << '}' << std::endl;

        for (auto relativePkg = stripParent(pkg, mPackage); !relativePkg.empty(); ) {
            utils::string_view head;
            std::tie(head, relativePkg) = split(relativePkg, '.');
            assert(!head.empty());
            (*mOut) << "namespace " << head << " {" << std::endl;
        }
        mPackage = pkg;
    }

    void forwardDeclare(Function* func) {
        PRECONDITION(func->package() == mPackage);
        (*mOut) << func->type()->name() << " " << func->name() << "(";
        bool first = true;
        for (auto* arg: func->args()) {
            if (!first)
                (*mOut) << ",";
            else
                first = false;
            (*mOut) << arg->type()->name() << " " << arg->name();
        }
        (*mOut) << ");" << std::endl;
    }

private:
    utils::string_view mPackage;
    std::ostream* mOut;
};

} // anonymous namespace

class CppGen: public Generator
{
public:
    CppGen(const Dictionary& dict): mDict(dict) {}

    void generate(meta::AST*, const std::string& output) override
    {
        std::ofstream out;
        out.exceptions(std::fstream::badbit);
        out.open(output, std::fstream::trunc | std::fstream::binary);
        CppWriter writer(out);

        for (const auto& kv: mDict) {
            const auto& functions = kv.second;

            writer.setPackage(kv.first);
            for (const auto& func: functions) {
                Function* f = func.second;
                if (f->visibility() != Visibility::Export)
                    continue;
                writer.forwardDeclare(f);
            }
        }
    }

private:
    const Dictionary& mDict;
};

std::unique_ptr<Generator> createCppGenerator(const Dictionary& dict)
{
    return std::make_unique<CppGen>(dict);
}

}}} //namespace meta::generators::cppgen
