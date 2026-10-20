//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

namespace glacialc {
    enum class TokenType {
        VERT,
        FRAG,
        GEOM,
        COMP,

        INCLUDE,
        DEFINE,
        PRAGMA,
        IFDEF,
        IFNDEF,
        ENDIF,

        IDENTIFIER,
        NUMBER_LITERAL,
        STRING_LITERAL,

        EQUALS,
        COLON,
        SEMICOLON,
        COMMA,
        LPAREN,
        RPAREN,
        LBRACE,
        RBRACE,
        LBRACKET,
        RBRACKET,

        RAW_TEXT,

        END_OF_FILE,
        ERROR
    };

    struct Token {
        TokenType type = TokenType::ERROR;
        std::string_view lexeme;
        size_t line = 1;
        size_t column = 1;
    };

    class Lexer {
    public:
        explicit Lexer(std::string_view source);
        ~Lexer() = default;

        std::vector<Token> tokenize();

    private:
        [[nodiscard]] char peek() const;
        [[nodiscard]] char peek_next() const;
        char advance();
        [[nodiscard]] bool is_at_end() const;

        void skip();

        [[nodiscard]] Token make_token(TokenType type, size_t length) const;
        [[nodiscard]] Token error_token(std::string_view message) const;

        Token lex_hash_directive();
        Token lex_identifier();
        Token lex_number();
        Token lex_string();

        std::string_view source_;
        size_t start_ = 0;
        size_t current_ = 0;
        size_t line_ = 1;
        size_t column_ = 1;
    };
} // glacialc
