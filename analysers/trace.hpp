#pragma once

#include <cstdlib>
#include <iostream>

#include "utils/range.h"
#include "utils/string.h"
#include "utils/types.h"

namespace meta::analysers {
namespace {

void trace(utils::string_view scopeTag, Node* node) {
    const utils::string_view traceScopes = ::getenv("META_TRACE_SCOPES");
    if (!utils::contains(utils::split(traceScopes, ':'), scopeTag))
        return;
    std::clog << node->source().path().string() << ':' << node->tokens().linenum() << ':' << node->tokens().colnum();
    utils::string_view str = node->tokens();
    const auto eol_pos = str.find('\n');
    str = str.substr(0, eol_pos);
    const char *strp = str.data();
    for (
        ;
        strp > node->source().content().data() && *(strp - 1) != '\n';
        --strp
    )
        ;
    const utils::string_view line_start = {strp, static_cast<size_t>(str.data() - strp)};

    for (
        strp = str.data() + str.size();
        strp < node->source().content().data() + node->source().content().size() && *strp != '\n';
        ++strp
    )
        ;
    const utils::string_view line_end = eol_pos != utils::string_view::npos ?
        "..."sv :
        utils::string_view{
            str.data() + str.size(),
            static_cast<size_t>(strp - str.data() - str.size())
        };
    std::clog << ": " << line_start << utils::TermColor::red << str << utils::TermColor::none << line_end << '\n';
}

} // anonymous namespace
} // namespace meta::analysers
