#ifndef TYPES_H
#define TYPES_H

namespace meta {

struct Token;

enum class Visibility {
    Default,
    Extern,
    Export,
    Public,
    Protected,
    Private
};

/**
 * @brief Converst visibility token into Visibility value
 *
 * @note It's caller responsibility to ensure that token passed is token representing
 * visibility.
 */
Visibility fromToken(const Token &token);

} // namespace meta

#endif // TYPES_H
