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
    static SourceFile fake(std::string&& content, utils::fs::path&& path = "test.meta") {
        SourceFile res;
        res.mPath = std::move(path);
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

#if defined(META_UNIT_TEST)
private:
    SourceFile() = default;
#else
    SourceFile() = delete;
#endif

private:
    fs::path mPath;
    std::string mContent;
};

#if defined(META_UNIT_TEST)
namespace literals {
inline
SourceFile operator "" _fake_src (const char* content, size_t sz) {
    return SourceFile::fake(std::string{content, content + sz});
}
} // namespace literals
#endif

} // namespace meta::utils
