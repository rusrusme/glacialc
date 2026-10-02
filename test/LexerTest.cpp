//
// Copyright (c) 2026 rusrusme 
// All rights reserved.
//

#include <iomanip>
#include <iostream>
#include <string_view>

#include "Lexer.hpp"

using namespace bshc;

static constexpr std::string_view token_type_to_string(const TokenType type) {
    switch (type) {
        case TokenType::VERT : return "VERT";
        case TokenType::FRAG : return "FRAG";
        case TokenType::GEOM : return "GEOM";
        case TokenType::COMP : return "COMP";
        case TokenType::META : return "META";
        case TokenType::INCLUDE : return "INCLUDE";
        case TokenType::DEFINE : return "DEFINE";
        case TokenType::PRAGMA : return "PRAGMA";
        case TokenType::IFDEF : return "IFDEF";
        case TokenType::IFNDEF : return "IFNDEF";
        case TokenType::ENDIF : return "ENDIF";
        case TokenType::IDENTIFIER : return "IDENTIFIER";
        case TokenType::NUMBER_LITERAL : return "NUMBER_LITERAL";
        case TokenType::STRING_LITERAL : return "STRING_LITERAL";
        case TokenType::EQUALS : return "EQUALS";
        case TokenType::COLON : return "COLON";
        case TokenType::SEMICOLON : return "SEMICOLON";
        case TokenType::COMMA : return "COMMA";
        case TokenType::LPAREN : return "LPAREN";
        case TokenType::RPAREN : return "RPAREN";
        case TokenType::LBRACE : return "LBRACE";
        case TokenType::RBRACE : return "RBRACE";
        case TokenType::LBRACKET : return "LBRACKET";
        case TokenType::RBRACKET : return "RBRACKET";
        case TokenType::RAW_TEXT : return "RAW_TEXT";
        case TokenType::END_OF_FILE : return "END_OF_FILE";
        case TokenType::ERROR : return "ERROR";
    }
    return "UNKNOWN";
}

int main() {
    constexpr std::string_view test_shader = R"(
        // Sample Basis Shader File
        @depth_test
        @cull_mode = 2

        #include "common/camera.glsl"

        #vert
        layout(location = 0) in vec3 a_Pos;

        void main() {
            gl_Position = vec4(a_Pos, 1.0);
        }

        #frag
        layout(location = 0) out vec4 FragColor;

        void main() {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";

    Lexer lexer(test_shader);
    const std::vector<Token> tokens = lexer.tokenize();

    std::cout << std::left
        << std::setw(18) << "TYPE"
        << std::setw(28) << "LEXEME"
        << std::setw(8) << "LINE"
        << std::setw(8) << "COL" << "\n";
    std::cout << std::string(62, '-') << "\n";

    for (const auto& [type, lexeme, line, column] : tokens) {
        std::cout << std::left
            << std::setw(18) << token_type_to_string(type)
            << std::setw(28) << (lexeme.empty() ? "<EOF>" : lexeme)
            << std::setw(8) << line
            << std::setw(8) << column << "\n";
    }

    return 0;
}
