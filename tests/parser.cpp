#include "parser.hpp"

#include <remac/parser.hpp>

void test_parser() {
    test_module("Parser");
    std::vector<remac::Token> tokens;
    tokens.push_back(remac::Token { remac::TokenType::IDENTIFIER, "Print", 1, 1 });
    tokens.push_back(remac::Token { remac::TokenType::LPAREN, "(", 1, 6 });
    tokens.push_back(remac::Token { remac::TokenType::RPAREN, ")", 1, 7 });
    remac::Parser parser(tokens);
    remac::ProgramNode *program = parser.parse();
    test_condition(program->equals(new remac::ProgramNode(
        new remac::SequenceNode({
            new remac::FunctionCallNode("Print", {})
        })
    )));
}
