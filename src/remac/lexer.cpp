#include <remac/lexer.hpp>

#include <remac/utf8.hpp>

#include <algorithm>
#include <cctype>
#include <clocale>
#include <cstdio>
#include <locale>
#include <map>
#include <optional>
#include <string>
#include <cstring>
#include <stack>
#include <vector>

namespace remac {

Lexer::Lexer(std::string input) {
    this->code = input;
    this->index = 0;
    this->type = PendingType::UNKNOWN;
    this->line = 1;
    this->column = 1;
}

std::optional<Token> Lexer::next() {
    if (this->index >= this->code.size()) {
        return {};
    }

    this->skipWhitespaces();

    if (this->index >= this->code.size()) {
        return {};
    }

    Utf8Char chr = this->peekChar();

    // TODO: Remove this->type.

    if (utfCharInString(
        this->IDENTIFIER_CHARS_START.c_str(),
        this->IDENTIFIER_CHARS_START.c_str() + \
            this->IDENTIFIER_CHARS_START.size(),
        chr
    )) {
        switch (this->prevType) {
            case TokenType::PROGRAM_START:
            case TokenType::KEYWORD:
            case TokenType::LPAREN:
            case TokenType::LBRACE:
            case TokenType::LBRACKET:
            case TokenType::OPERATOR:
            case TokenType::ARG_SEPARATOR: break;

            default: {
                return { Token { .type = TokenType::LEXER_ERROR, .content = "Unexpected identifier", .line = this->line, .column = this->column } };
            }
        }

        this->prevType = TokenType::IDENTIFIER; // TODO: Add everywhere in token return
        return this->nextIdentifier();
    }

    if (utfCharInString(
        this->DIGITS.c_str(),
        this->DIGITS.c_str() + this->DIGITS.size(),
        chr
    )) {
        switch (this->prevType) {
            case TokenType::PROGRAM_START:
            case TokenType::KEYWORD:
            case TokenType::LPAREN:
            case TokenType::LBRACE:
            case TokenType::LBRACKET:
            case TokenType::OPERATOR:
            case TokenType::ARG_SEPARATOR: break;

            default: {
                return { Token { .type = TokenType::LEXER_ERROR, .content = "Unexpected number", .line = this->line, .column = this->column } };
            }
        }

        Token token = this->nextNumber();
        this->prevType = token.type;
        return token;
    }

    if (chr.bytes[0] == Lexer::LPAREN) {
        switch (this->prevType) {
            case TokenType::PROGRAM_START:
            case TokenType::IDENTIFIER:
            case TokenType::KEYWORD:
            case TokenType::LPAREN:
            case TokenType::RPAREN:
            case TokenType::LBRACE:
            case TokenType::LBRACKET:
            case TokenType::RBRACKET:
            case TokenType::OPERATOR:
            case TokenType::ARG_SEPARATOR: break;

            default: {
                return { Token { .type = TokenType::LEXER_ERROR, .content = "Unexpected left parentheses", .line = this->line, .column = this->column } };
            }
        }

        unsigned long line = this->line;
        unsigned long column = this->column;
        this->advanceChar();
        this->parens.push(TokenType::LPAREN);
        this->prevType = TokenType::LPAREN;
        return { Token { .type = TokenType::LPAREN, .content = charToString(Lexer::LPAREN), .line = line, .column = column } };
    }

    if (chr.bytes[0] == Lexer::RPAREN) {
        switch (this->prevType) {
            case TokenType::IDENTIFIER:
            case TokenType::INT_NUMBER:
            case TokenType::FLOAT_NUMBER:
            case TokenType::LPAREN:
            case TokenType::RPAREN:
            case TokenType::RBRACE:
            case TokenType::RBRACKET: break;

            default: {
                std::printf("%s\n", Token { this->prevType, "none", 0, 0 }.to_string().c_str());
                return { Token { .type = TokenType::LEXER_ERROR, .content = "Unexpected right parentheses", .line = this->line, .column = this->column } };
            }
        }

        unsigned long line = this->line;
        unsigned long column = this->column;
        this->advanceChar();

        if (this->parens.top() != TokenType::LPAREN) {
            return { Token { .type = TokenType::LEXER_ERROR, .content = "Unexpected ')'. Do you forget to close other parens?", .line = line, .column = column } };
        }

        this->parens.pop();
        this->prevType = TokenType::RPAREN;
        return { Token { .type = TokenType::RPAREN, .content = charToString(Lexer::RPAREN), .line = line, .column = column } };
    }

    if (chr.bytes[0] == Lexer::LBRACE) {
        switch (this->prevType) {
            case TokenType::KEYWORD:
            case TokenType::RPAREN: break;

            default: {
                return { Token { .type = TokenType::LEXER_ERROR, .content = "Unexpected left brace", .line = this->line, .column = this->column } };
            }
        }

        unsigned long line = this->line;
        unsigned long column = this->column;
        this->advanceChar();
        this->parens.push(TokenType::LBRACE);
        this->prevType = TokenType::LBRACE;
        return { Token { .type = TokenType::LBRACE, .content = charToString(Lexer::LBRACE), .line = line, .column = column } };
    }

    if (chr.bytes[0] == Lexer::RBRACE) {
        switch (this->prevType) {
            case TokenType::IDENTIFIER:
            case TokenType::INT_NUMBER:
            case TokenType::FLOAT_NUMBER:
            case TokenType::KEYWORD:
            case TokenType::RPAREN:
            case TokenType::LBRACE:
            case TokenType::RBRACKET: break;

            default: {
                return { Token { .type = TokenType::LEXER_ERROR, .content = "Unexpected right brace", .line = this->line, .column = this->column } };
            }
        }

        unsigned long line = this->line;
        unsigned long column = this->column;
        this->advanceChar();

        if (this->parens.top() != TokenType::LBRACE) {
            return { Token { .type = TokenType::LEXER_ERROR, .content = "Unexpected '}'. Do you forget to close other parens?", .line = line, .column = column } };
        }

        this->parens.pop();
        this->prevType = TokenType::RBRACE;
        return { Token { .type = TokenType::RBRACE, .content = charToString(Lexer::RBRACE), .line = line, .column = column } };
    }

    if (chr.bytes[0] == Lexer::LBRACKET) {
        switch (this->prevType) {
            case TokenType::PROGRAM_START:
            case TokenType::IDENTIFIER:
            case TokenType::KEYWORD:
            case TokenType::LPAREN:
            case TokenType::LBRACE:
            case TokenType::LBRACKET:
            case TokenType::RBRACKET:
            case TokenType::ARG_SEPARATOR: break;

            default: {
                return { Token { .type = TokenType::LEXER_ERROR, .content = "Unexpected left bracket", .line = this->line, .column = this->column } };
            }
        }

        unsigned long line = this->line;
        unsigned long column = this->column;
        this->advanceChar();
        this->prevType = TokenType::LBRACKET;
        return { Token { .type = TokenType::LBRACKET, .content = charToString(Lexer::LBRACKET), .line = line, .column = column } };
    }

    if (chr.bytes[0] == Lexer::RBRACKET) {
        switch (this->prevType) {
            case TokenType::IDENTIFIER:
            case TokenType::INT_NUMBER:
            case TokenType::FLOAT_NUMBER:
            case TokenType::RPAREN:
            case TokenType::LBRACKET:
            case TokenType::RBRACKET:
            case TokenType::ARG_SEPARATOR: break;

            default: {
                return { Token { .type = TokenType::LEXER_ERROR, .content = "Unexpected right bracket", .line = this->line, .column = this->column } };
            }
        }

        unsigned long line = this->line;
        unsigned long column = this->column;
        this->advanceChar();
        this->prevType = TokenType::RBRACKET;
        return { Token { .type = TokenType::RBRACKET, .content = charToString(Lexer::RBRACKET), .line = line, .column = column } };
    }

    if (chr.bytes[0] == Lexer::ARG_SEPARATOR) {
        switch (this->prevType) {
            case TokenType::IDENTIFIER:
            case TokenType::INT_NUMBER:
            case TokenType::FLOAT_NUMBER:
            case TokenType::RPAREN:
            case TokenType::RBRACKET: break;

            default: {
                return { Token { .type = TokenType::LEXER_ERROR, .content = "Unexpected argument separator", .line = this->line, .column = this->column } };
            }
        }

        unsigned long line = this->line;
        unsigned long column = this->column;
        this->advanceChar();
        this->prevType = TokenType::ARG_SEPARATOR;
        return { Token { .type = TokenType::ARG_SEPARATOR, .content = charToString(Lexer::ARG_SEPARATOR), .line = line, .column = column } };
    }

    if (chr.bytes[0] == '"' || chr.bytes[0] == '\'') {
        this->prevType = TokenType::STRING;
        return { this->nextString(chr) };
    }

    unsigned long line = this->line;
    unsigned long column = this->column;

    std::optional<std::string> operatorText = this->find_operator();

    if (operatorText.has_value()) {
        this->prevType = TokenType::OPERATOR;
        return { Token { .type = TokenType::OPERATOR, .content = *operatorText, .line = line, .column = column } };
    }

    return { Token { .type = TokenType::LEXER_ERROR, .content = "Unknown token type", .line = line, .column = column } };
}

Token Lexer::findKeyword(Token token) {
    if (token.content == "if" || token.content == "while" || token.content == "for") {
        return Token { .type = TokenType::KEYWORD, .content = token.content, .line = token.line, .column = token.column };
    }

    return token;
}

std::optional<std::string> Lexer::find_operator() {
    return this->get_operator("", 2); // Max. length of operator is 2
}

std::optional<std::string> Lexer::get_operator(std::string str, short depth) {
    if (depth == 0) {
        for (unsigned long i = 0; i < sizeof(Lexer::OPERATORS) / sizeof(Lexer::OPERATORS[0]); i++) {
            if (str == this->OPERATORS[i]) {
                this->advanceChar();
                return { str };
            }
        }
    } else {
        std::string str2;
        str2.append(str);
        utfStringAppend(&str, this->peekChar());
        std::optional<std::string> oper = get_operator(str2, depth - 1);

        if (oper.has_value()) {
            return { oper };
        }

        this->advanceChar();

        for (unsigned long i = 0; i < sizeof(Lexer::OPERATORS) / sizeof(Lexer::OPERATORS[0]); i++) {
            if (str == this->OPERATORS[i]) {
                return { str };
            }
        }
    }

    return {};
}

/**
 * Caller must ensure, that this->index + 1 < this->code.size()
*/
Utf8Char Lexer::peekChar() {
    std::tuple<unsigned char, Utf8Char> chr = getNextUnicode(this->code.c_str() + this->index);

    if (std::get<0>(chr) == UTF8_INVALID) {
        std::fprintf(stderr, "Invalid UTF-8!\n");
        throw new std::exception();
    }

    return std::get<1>(chr);
}

/**
 * Caller must ensure, that this->index + 1 < this->code.size()
*/
Utf8Char Lexer::advanceChar() {
    if (this->code[this->index] == '\n') {
        this->line++;
        this->column = 1;
    } else if (this->code[this->index] == '\r') {
        if (this->code[this->index + 1] == '\n') {
            // Windows CRLF, make sure, that its counted as only one line feed, so do nothing
        } else {
            this->line++;
            this->column = 1;
        }
    } else {
        this->column++;
    }

    std::tuple<unsigned char, Utf8Char> chrNow = getNextUnicode(this->code.c_str() + this->index);
    unsigned char prevLength = std::get<0>(chrNow);

    if (prevLength == UTF8_INVALID) {
        std::fprintf(stderr, "Invalid UTF-8!\n");
        throw new std::exception();
    }

    this->index += prevLength;
    std::tuple<unsigned char, Utf8Char> chr = getNextUnicode(this->code.c_str() + this->index);
    unsigned char length = std::get<0>(chr);

    if (length == UTF8_INVALID) {
        std::fprintf(stderr, "Invalid UTF-8!\n");
        throw new std::exception();
    }

    // this->index += length;
    return std::get<1>(chr);
}

void Lexer::skipWhitespaces() {
    Utf8Char chr = this->peekChar();

    while (utfCharInString(
        Lexer::WHITESPACE_CHARS.c_str(),
        Lexer::WHITESPACE_CHARS.c_str() + Lexer::WHITESPACE_CHARS.size(),
        chr
    )) {
        chr = this->advanceChar();
    }
}

Token Lexer::nextIdentifier() {
    std::string content = "";
    unsigned long line = this->line;
    unsigned long column = this->column;
    Utf8Char utfChar = this->peekChar();

    while (utfCharInString(this->IDENTIFIER_CHARS.c_str(), this->IDENTIFIER_CHARS.c_str() + this->IDENTIFIER_CHARS.size(), utfChar)) {
        utfStringAppend(&content, utfChar);
        utfChar = this->advanceChar();
    }

    return Token { .type = TokenType::IDENTIFIER, .content = content, .line = line, .column = column };
}

Token Lexer::nextNumber() {
    std::string content = "";
    unsigned long line = this->line;
    unsigned long column = this->column;
    bool floating = false;
    Utf8Char utfChar = this->peekChar();

    while (utfCharInString(this->DIGITS.c_str(), this->DIGITS.c_str() + this->DIGITS.size(), utfChar)) {
        utfStringAppend(&content, utfChar);
        utfChar = this->advanceChar();

        if (utfChar.bytes[0] == Lexer::FLOATING_POINT) { // This comparision is possible, because '.' is ASCII
            if (floating) {
                return Token { .type = TokenType::LEXER_ERROR, .content = "Invalid floating point number. Maybe remove second period?", .line = line, .column = column };
            }

            floating = true;
            content.push_back(Lexer::FLOATING_POINT);
            utfChar = this->advanceChar();
        }
    }

    if (content.back() == Lexer::FLOATING_POINT) { // TODO: Check if std::string.back() really returns last char or i'm stupid.
        return Token { .type = TokenType::LEXER_ERROR, .content = "Floating point number must end with digit. Maybe add '0' to end of it?", .line = line, .column = column };
    }

    if (floating) {
        return Token { .type = TokenType::FLOAT_NUMBER, .content = content, .line = line, .column = column };
    }

    return Token { .type = TokenType::INT_NUMBER, .content = content, .line = line, .column = column };
}

Token Lexer::nextString(Utf8Char closingChar) {
    Utf8Char chr = this->advanceChar(); // Gets next char after 1 open quote
    bool escaped = false;
    std::string buffer;
    unsigned long line = this->line;
    unsigned long column = this->column;

    while (chr.codePoint != closingChar.codePoint && (!escaped)) {
        if (escaped) {
            switch (chr.bytes[0]) {
                case 'n': {
                    buffer.push_back('\n');
                    break;
                }
                case 'r': {
                    buffer.push_back('\r');
                    break;
                }
                case 't': {
                    buffer.push_back('\t');
                    break;
                }
                case '\\': {
                    buffer.push_back('\\');
                    break;
                }
            }

            escaped = false;
            utfStringAppend(&buffer, chr);
        } else {
            if (chr.bytes[0] == Lexer::ESCAPE) {
                escaped = true;
            }
        }

        chr = this->advanceChar();
    }

    return Token { .type = TokenType::STRING, .content = buffer, .line = line, .column = column };
}

std::string Token::to_string() {
    if (this->type == TokenType::LEXER_ERROR) {
        return "Lexer error on line " + std::to_string(this->line) + ":" + \
            std::to_string(this->column) + ": " + this->content;
    }

    std::map<TokenType, std::string> map = {
        {TokenType::PROGRAM_START, "Program start"},
        {TokenType::IDENTIFIER, "Identifier"},
        {TokenType::INT_NUMBER, "Number"},
        {TokenType::FLOAT_NUMBER, "Number"},
        {TokenType::KEYWORD, "Keyword"},
        {TokenType::LPAREN, "Left parentheses"},
        {TokenType::RPAREN, "Right parentheses"},
        {TokenType::LBRACE, "Left brace"},
        {TokenType::RBRACE, "Right brace"},
        {TokenType::LBRACKET, "Left bracket"},
        {TokenType::RBRACKET, "Right bracket"},
        {TokenType::OPERATOR, "Operator"},
        {TokenType::ARG_SEPARATOR, "Argument separator"},
    };
    return "<Token type='" + map[this->type] + "', content='" + \
        this->content + "', line=" + std::to_string(this->line) + ":" + \
        std::to_string(this->column) + ">";
}

}
