#pragma once

#include "utils/io.h"
#include "utils/types.h"

namespace meta::utils {

class SourceFile {
public:
    explicit SourceFile(const fs::path& path):
        mPath(path),
        mContent(readAll(path))
    {}
    explicit SourceFile(fs::path&& path):
        mPath(std::move(path)),
        mContent(readAll(mPath))
    {}

#if defined(META_UNIT_TEST)
    template<size_t N>
    SourceFile(
        const array_t<const char, N>& inlinedContent,
        const char* path = __builtin_FILE(),
        unsigned lineOffset = __builtin_LINE()
    ):
        mPath(path), mContent(inlinedContent), mLineNumOffset(lineOffset)
    {}

    static SourceFile fake(const utils::fs::path& path, std::string&& content) {
        SourceFile res;
        res.mPath = path;
        res.mContent = std::move(content);
        return res;
    }

    SourceFile(const SourceFile&) = default;
    SourceFile& operator= (const SourceFile&) = default;
#else
    SourceFile(const SourceFile&) = delete;
    SourceFile operator= (const SourceFile&) = delete;
#endif

    SourceFile(SourceFile&&) = default;
    SourceFile& operator= (SourceFile&&) = default;

    const fs::path& path() const {return mPath;}
    string_view content() const {return mContent;}
    operator string_view () const {return mContent;} // TODO: check if needed

#if defined(META_UNIT_TEST)
private:
    SourceFile() = default;
#else
    SourceFile() = delete;
#endif

private:
    fs::path mPath;
    std::string mContent;
#if defined(META_UNIT_TEST)
    unsigned mLineNumOffset = 0;
#endif
};

} // namespace meta::utils
