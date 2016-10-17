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
        unsigned firstLineNum = __builtin_LINE()
    ):
        mPath(path), mContent(inlinedContent), mFirstLineNum(firstLineNum)
    {}

    static SourceFile fake(const utils::fs::path& path, std::string&& content) {
        SourceFile res;
        res.mPath = path;
        res.mContent = std::move(content);
        return res;
    }

    // Should be move-only but gtest TestWithParam requires copy constructor
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
    unsigned firstLineNum() const {return mFirstLineNum;}

#if defined(META_UNIT_TEST)
private:
    SourceFile() = default;
#else
    SourceFile() = delete;
#endif

private:
    fs::path mPath;
    std::string mContent;
    unsigned mFirstLineNum = 1;
};

} // namespace meta::utils
