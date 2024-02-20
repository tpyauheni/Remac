#include "belang/parser.hpp"
#include <belang/lexer.hpp>

#include <optional>
#include <string>
#include <iostream>
#include <cstdio>

#define VERSION_MAJOR 0
#define VERSION_MINOR 0
#define VERSION_PATCH 0
#define VERSION_TAG " (dev)"

int main() {
    std::string input = "Print([21, 5 * (2 + 1)])";

    std::printf(
        "Belang v.%u.%u.%u%s by Pakul Yauheni Stanislavovich\n",
        VERSION_MAJOR,
        VERSION_MINOR,
        VERSION_PATCH,
        VERSION_TAG
    );
    std::printf(">> ");
    std::fflush(stdout);
    std::cout << "WARNING: Debug mode, so input automatically filled" << std::endl; // std::cin >> input;
    belang::Lexer lexer = belang::Lexer(input);
    std::vector<belang::Token> tokens;

    std::optional<belang::Token> last_token = lexer.next();
    std::cout << "Lexical analyzer output:" << std::endl;

    while (last_token.has_value()) {
        last_token = lexer.findKeyword(*last_token);
        std::cout << last_token->to_string() << std::endl;

        if (last_token->type == belang::TokenType::LEXER_ERROR) {
            return 1;
        }

        tokens.push_back(*last_token);
        last_token = lexer.next();
    }

    std::cout << "\nParser output:" << std::endl;

    try {
        belang::Parser parser = belang::Parser(tokens);
        parser.parse()->print();
    } catch (belang::ParserException *exc) {
        std::cout << "Exception: " << exc->message << std::endl;
    }

    return 0;
}
