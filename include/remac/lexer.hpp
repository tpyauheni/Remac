#pragma once

#ifndef REMAC_LEXER
#define REMAC_LEXER 1

#include <remac/utf8.hpp>

#include <stack>
#include <cctype>
#include <clocale>
#include <locale>
#include <optional>
#include <string>
#include <tuple>

namespace remac {

enum TokenType : unsigned char {
    /**
     * It's not a token. Set instead of token to Lexer::prevType before first
        token.
     */
    PROGRAM_START,

    /**
     * Regular expression: `[a-zA-Z]([a-zA-Z_0-9]+)`.
     *
     * May be found after PROGRAM_START, KEYWORD, LPAREN, LBRACE, LBRACKET,
        OPERATOR, ARG_SEPARATOR.
     */
    IDENTIFIER,

    /**
     * Regular expression: `[0-9]+`.
     *
     * May be found after PROGRAM_START, KEYWORD, LPAREN, LBRACE, LBRACKET,
        OPERATOR, ARG_SEPARATOR.
     */
    INT_NUMBER,

    /**
     * Regular expression: `[0-9]+\.[0-9]+`.
     *
     * May be found after PROGRAM_START, KEYWORD, LPAREN, LBRACE, LBRACKET,
        OPERATOR, ARG_SEPARATOR.
     */
    FLOAT_NUMBER,

    /**
     * Same as IDENTIFIER.
     */
    KEYWORD,

    /**
     * Regular expression: `\(`.
     *
     * May be found after PROGRAM_START, IDENTIFIER, KEYWORD, LPAREN, RPAREN,
        LBRACE, LBRACKET, RBRACKET, OPERATOR, ARG_SEPARATOR.
     */
    LPAREN,

    /**
     * Regular expression: `\)`.
     *
     * May be found after IDENTIFIER, INT_NUMBER, FLOAT_NUMBER, LPAREN,
        RPAREN, RBRACE, RBRACKET.
     */
    RPAREN,

    /**
     * Regular expression: `{`.
     *
     * May be found after KEYWORD, RPAREN.
     */
    LBRACE,

    /**
     * Regular expression: `}`.
     *
     * May be found after IDENTIFIER, INT_NUMBER, FLOAT_NUMBER, KEYWORD,
        RPAREN, LBRACE, RBRACKET.
     */
    RBRACE,

    /**
     * Regular expression: `[`.
     *
     * May be found after PROGRAM_START, IDENTIFIER, KEYWORD, LPAREN, LBRACE,
        LBRACKET, RBRACKET, ARG_SEPARATOR.
     */
    LBRACKET,

    /**
     * Regular expression: `]`.
     *
     * May be found after IDENTIFIER, INT_NUMBER, FLOAT_NUMBER, RPAREN,
        LBRACKET, RBRACKET, ARG_SEPARATOR.
     */
    RBRACKET,

    /**
     * Regular expression: `(\=|\+|\-|\*|\/|\%|\>\=|\<\=|\>|\<|\=\=|\!\=)`.
     *
     * May be found after IDENTIFIER, INT_NUMBER, FLOAT_NUMBER, RPAREN,
        RBRACKET.
     */
    OPERATOR,

    /**
     * Regular expression: `,`.
     * 
     * May be found after IDENTIFIER, INT_NUMBER, FLOAT_NUMBER, RPAREN,
        RBRACKET.
     */
    ARG_SEPARATOR,

    /**
     * Regular expression: `"(.*?)"`.
     * 
     * May be found after PROGRAM_START, KEYWORD, LPAREN, LBRACE,
        ARG_SEPARATOR.
     */
    STRING,

    /**
     * It's not a token. Returned instead of token, when lexer error occurs.
     */
    LEXER_ERROR,
};

struct Token {
    TokenType type;
    std::string content;
    unsigned long line;
    unsigned long column;

    std::string to_string();
};

class Lexer {
private:
    enum PendingType {
        UNKNOWN,
        IDENTIFIER,
        NUMBER,
        OPERATOR,
    };

    const std::string LETTERS = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const std::string DIGITS = "0123456789";
    const std::string IDENTIFIER_CHARS_START = LETTERS;
    const std::string IDENTIFIER_CHARS = LETTERS + DIGITS + "_";
    const char FLOATING_POINT = '.';
    const char LPAREN = '(';
    const char RPAREN = ')';
    const char LBRACE = '{';
    const char RBRACE = '}';
    const char LBRACKET = '[';
    const char RBRACKET = ']';
    const char ARG_SEPARATOR = ',';
    const char ESCAPE = '\\';
    const std::string OPERATORS[12] = {
        "=",
        "+", "-", "*", "/", "%",
        "!=", "==", ">", ">=", "<", "<=",
    };
    const std::string WHITESPACE_CHARS = "\n\t ";

    std::string code;
    unsigned long index;
    PendingType type;
    unsigned long line;
    unsigned long column;
    std::stack<TokenType> parens;
    TokenType prevType = TokenType::PROGRAM_START;

public:
    explicit Lexer(std::string input);

    std::optional<Token> next();
    Token findKeyword(Token token);


private:
    Utf8Char peekChar();
    Utf8Char advanceChar();
    void skipWhitespaces();
    Token nextIdentifier();
    Token nextNumber();
    Token nextString(Utf8Char closingChar);
    std::optional<std::string> find_operator();
    std::optional<std::string> get_operator(std::string str, short depth);
};

}

#endif // REMAC_LEXER
