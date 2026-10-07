//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include "Lexer.hpp"

#include <cctype>

namespace bshc {
    Lexer::Lexer(const std::string_view source) : source_(source) {}

    std::vector<Token> Lexer::tokenize() {
        std::vector<Token> tokens;

        while (!is_at_end()) {
            skip();
            if (is_at_end()) break;

            start_ = current_;
            const char c = peek();

            if (c == '@') {
                tokens.push_back(lex_metadata());
            }
            else if (c == '#') {
                tokens.push_back(lex_hash_directive());
            }
            else if (std::isalpha(c) || c == '_') {
                tokens.push_back(lex_identifier());
            }
            else if (std::isdigit(c)) {
                tokens.push_back(lex_number());
            }
            else if (c == '"') {
                tokens.push_back(lex_string());
            }
            else {
                switch (advance()) {
                    case '=' : tokens.push_back(make_token(TokenType::EQUALS, 1));
                        break;
                    case ':' : tokens.push_back(make_token(TokenType::COLON, 1));
                        break;
                    case ';' : tokens.push_back(make_token(TokenType::SEMICOLON, 1));
                        break;
                    case ',' : tokens.push_back(make_token(TokenType::COMMA, 1));
                        break;
                    case '(' : tokens.push_back(make_token(TokenType::LPAREN, 1));
                        break;
                    case ')' : tokens.push_back(make_token(TokenType::RPAREN, 1));
                        break;
                    case '{' : tokens.push_back(make_token(TokenType::LBRACE, 1));
                        break;
                    case '}' : tokens.push_back(make_token(TokenType::RBRACE, 1));
                        break;
                    case '[' : tokens.push_back(make_token(TokenType::LBRACKET, 1));
                        break;
                    case ']' : tokens.push_back(make_token(TokenType::RBRACKET, 1));
                        break;
                    default :
                        tokens.push_back(make_token(TokenType::RAW_TEXT, 1));
                        break;
                }
            }
        }

        start_ = current_;
        tokens.push_back(make_token(TokenType::END_OF_FILE, 0));
        return tokens;
    }

    char Lexer::peek() const {
        if (is_at_end()) return '\0';
        return source_[current_];
    }

    char Lexer::peek_next() const {
        if (current_ + 1 >= source_.length()) return '\0';
        return source_[current_ + 1];
    }

    char Lexer::advance() {
        const char c = source_[current_++];
        column_++;
        if (c == '\n') {
            line_++;
            column_ = 1;
        }
        return c;
    }

    bool Lexer::is_at_end() const { return current_ >= source_.length(); }

    void Lexer::skip() {
        while (!is_at_end()) {
            switch (peek()) {
                case ' ' :
                case '\r' :
                case '\t' :
                case '\n' :
                    advance();
                    break;
                case '/' :
                    if (peek_next() == '/') {
                        while (peek() != '\n' && !is_at_end()) advance();
                    }
                    else if (peek_next() == '*') {
                        advance(); // Consume '/'
                        advance(); // Consume '*'
                        while (!is_at_end()) {
                            if (peek() == '*' && peek_next() == '/') {
                                advance(); // Consume '*'
                                advance(); // Consume '/'
                                break;
                            }
                            advance();
                        }
                    }
                    else {
                        return;
                    }
                    break;
                default :
                    return;
            }
        }
    }

    Token Lexer::make_token(const TokenType type, const size_t length) const {
        return Token{
            .type   = type,
            .lexeme = source_.substr(start_, length),
            .line   = line_,
            .column = column_ - length
        };
    }

    Token Lexer::error_token(const std::string_view message) const {
        return Token{
            .type   = TokenType::ERROR,
            .lexeme = message,
            .line   = line_,
            .column = column_
        };
    }

    Token Lexer::lex_metadata() {
        advance(); // Consume '@'
        while (std::isalnum(peek()) || peek() == '_') advance();
        return make_token(TokenType::META, current_ - start_);
    }

    Token Lexer::lex_hash_directive() {
        advance(); // Consume '#'
        const size_t kw_start = current_;
        while (std::isalpha(peek())) advance();

        const std::string_view kw = source_.substr(kw_start, current_ - kw_start);

        // Stage Directives
        if (kw == "vert") return make_token(TokenType::VERT, current_ - start_);
        if (kw == "frag") return make_token(TokenType::FRAG, current_ - start_);
        if (kw == "geom") return make_token(TokenType::GEOM, current_ - start_);
        if (kw == "comp") return make_token(TokenType::COMP, current_ - start_);

        // Preprocessor Directives
        if (kw == "include") return make_token(TokenType::INCLUDE, current_ - start_);
        if (kw == "define") return make_token(TokenType::DEFINE, current_ - start_);
        if (kw == "pragma") return make_token(TokenType::PRAGMA, current_ - start_);
        if (kw == "ifdef") return make_token(TokenType::IFDEF, current_ - start_);
        if (kw == "ifndef") return make_token(TokenType::IFNDEF, current_ - start_);
        if (kw == "endif") return make_token(TokenType::ENDIF, current_ - start_);

        return make_token(TokenType::RAW_TEXT, current_ - start_);
    }

    Token Lexer::lex_identifier() {
        while (std::isalnum(peek()) || peek() == '_') advance();
        return make_token(TokenType::IDENTIFIER, current_ - start_);
    }

    Token Lexer::lex_number() {
        while (std::isdigit(peek()) || peek() == '.') advance();
        if (peek() == 'f' || peek() == 'F') advance();
        return make_token(TokenType::NUMBER_LITERAL, current_ - start_);
    }

    Token Lexer::lex_string() {
        advance(); // Consume leading '"'
        while (peek() != '"' && !is_at_end()) advance();
        if (is_at_end()) return error_token("Unterminated string literal");
        advance(); // Consume trailing '"'
        return make_token(TokenType::STRING_LITERAL, current_ - start_);
    }
} // bshc
