#ifndef RESOLVER_H
#define RESOLVER_H

namespace meta {
class AST;
}

namespace analysers {

void resolve(meta::AST *ast);

} // namespace analysers

#endif // RESOLVER_H
