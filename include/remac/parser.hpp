#pragma once
#ifndef REMAC_PARSER
#define REMAC_PARSER 1

#include <remac/utf8.hpp>
#include <remac/lexer.hpp>

#include <exception>
#include <string>
#include <tuple>
#include <vector>

namespace remac {

class Printable {
public:
    virtual std::string toString() = 0;
    virtual void print();
};

class Serializable {
public:
    virtual unsigned long getByteLength() = 0;
    virtual unsigned long toBytes(void *buffer) = 0;
    virtual unsigned long fromBytes(void *buffer) = 0;
};

class AstNode : public Serializable, public Printable {
public:
    enum NodeType {
        NODE_EMPTY,
        NODE_FUNCTION_CALL,
        NODE_PROGRAM,
        NODE_IF_STATEMENT,
        NODE_WHILE_STATEMENT,
        NODE_FOR_STATEMENT,
        NODE_VARIABLE_ASSIGNMENT,
        NODE_OPERATION_ADD,
        NODE_OPERATION_SUBTRACT,
        NODE_OPERATION_MULTIPLY,
        NODE_OPERATION_DIVIDE,
        NODE_OPERATION_MOD,
        NODE_INT_CONSTANT,
        NODE_FLOAT_CONSTANT,
        NODE_STRING_CONSTANT,
        NODE_SEQUENCE,
        NODE_LIST_DEFINITION,
        NODE_LIST_SLICE,
        NODE_VARIABLE_REFERENCE,
    };

    // unsigned long getByteLength() override;
    // unsigned long toBytes(void *buffer) override;
    // unsigned long fromBytes(void *buffer) override;

    virtual NodeType getType();

    virtual bool equalTo(AstNode *node) = 0;

    bool equals(AstNode *node);

    virtual ~AstNode() = 0;
};

class SequenceNode : public AstNode {
private:
    std::vector<AstNode *> nodes;

public:
    explicit SequenceNode(std::vector<AstNode *> nodes);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    std::vector<AstNode *> getSequence();

    bool equalTo(AstNode *node) override;

    ~SequenceNode() override;
};

/**
 * Let Arg = Identifier|IntNumber|FloatNumber|Lparen|Lbracket|String;
 * Consist of <Identifier><Lparen>(<Rparen>|<Arg><Rparen>|{<Arg><ArgSep>}<Arg><Rparen>)
 */
class FunctionCallNode : public AstNode {
private:
    std::string name;
    SequenceNode *args;

public:
    FunctionCallNode(std::string name, SequenceNode *args);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    std::string getName();
    SequenceNode *getArgs();

    bool equalTo(AstNode *node) override;

    ~FunctionCallNode() override;
};

/**
 * Consist of {(<FunctionCallNode>|<IfStatementNode>|<WhileStatementNode>|<ForStatementNode>|<VariableAssignmentNode>|...)}
 */
class ProgramNode : public AstNode {
private:
    SequenceNode *body;

public:
    explicit ProgramNode(SequenceNode *body);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    SequenceNode *getBody();

    bool equalTo(AstNode *node) override;

    ~ProgramNode() override;
};

/**
 * Let Arg = Identifier|IntNumber|FloatNumber|Lparen|Lbracket|String;
 * Consist of <Keyword "if"><Lparen><Arg><Rparen><Lbrace>...<Rbrace>
 */
class IfStatementNode : public AstNode {
private:
    AstNode *condition;
    SequenceNode *ifBranch;
    SequenceNode *elseBranch;

public:
    IfStatementNode(AstNode *condition, SequenceNode *ifBranch, SequenceNode *elseBranch);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    AstNode *getCondition();
    SequenceNode *getBody();
    SequenceNode *getElseBody();

    bool equalTo(AstNode *node) override;

    ~IfStatementNode() override;
};

/**
 * Let Arg = Identifier|IntNumber|FloatNumber|Lparen|Lbracket|String;
 * Consist of <Keyword "while"><Lparen><Arg><Rparen><Lbrace>...<Rbrace>
 */
class WhileStatementNode : public AstNode {
private:
    AstNode *condition;
    SequenceNode *body;

public:
    WhileStatementNode(AstNode *condition, SequenceNode *body);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    AstNode *getCondition();
    SequenceNode *getBody();

    bool equalTo(AstNode *node) override;

    ~WhileStatementNode() override;
};

/**
 * Let Arg = Identifier|IntNumber|FloatNumber|Lparen|Lbracket|String;
 * Consist of <Keyword "for"><Lparen><Lbrace>{<Arg>}<Rbrace><ArgSep><Arg><Lbrace>{<Arg>}<Rbrace><Rparen><Lbrace>...<Rbrace>
 */
class ForStatementNode : public AstNode {
/*
Correct for example:
    for ({
        i = 0
        j = 1
        initializeI(i)
    }, i > j, {
        i++
        j--
    }) {...}
*/
private:
    SequenceNode *initializationBody;
    AstNode *condition;
    SequenceNode *incrementBody;
    SequenceNode *body;

public:
    ForStatementNode(SequenceNode *initializationBody, AstNode *condition, SequenceNode *incrementBody, SequenceNode *body);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    SequenceNode *getInitializationBody();
    AstNode *getCondition();
    SequenceNode *getIncrementBody();
    SequenceNode *getBody();

    bool equalTo(AstNode *node) override;

    ~ForStatementNode() override;
};

/**
 * Let Arg = Identifier|IntNumber|FloatNumber|Lparen|Lbracket|String;
 * Consist of <Keyword "var"|"let"><Identifier><Operator "="><Arg>
 */
class VariableAssignmentNode : public AstNode {
private:
    std::string name;
    AstNode *value;

public:
    VariableAssignmentNode(std::string name, AstNode *value);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    std::string getName();
    AstNode *getValue();

    bool equalTo(AstNode *node) override;

    ~VariableAssignmentNode() override;
};

/**
 * Let Arg = Identifier|IntNumber|FloatNumber|Lparen|Lbracket|String;
 * Consist of <Lbracket>[<Arg>{<Comma><Arg>}]<Rbracket>
 */
class ListDefinitionNode : public AstNode {
private:
    SequenceNode *array;

public:
    explicit ListDefinitionNode(SequenceNode *array);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    SequenceNode *getArray();

    bool equalTo(AstNode *node) override;

    ~ListDefinitionNode() override;
};

class ListSliceNode : public AstNode {
private:
    AstNode *array;
    AstNode *value;

public:
    ListSliceNode(AstNode *array, AstNode *value);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    AstNode *getArray();
    AstNode *getValue();

    bool equalTo(AstNode *node) override;

    ~ListSliceNode() override;
};

class VariableReferenceNode : public AstNode {
private:
    std::string name;

public:
    explicit VariableReferenceNode(std::string name);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    std::string getName();

    bool equalTo(AstNode *node) override;

    ~VariableReferenceNode() override;
};

/**
 * Let Arg = Identifier|IntNumber|FloatNumber|Lparen|Lbracket|String;
 * Consist of <Arg><Operator "+"><Arg>
 *
 * Look at next token. If it is operator with more priority, add it first, before this.
 */
class OperationAddNode : public AstNode {
private:
    AstNode *left;
    AstNode *right;

public:
    OperationAddNode(AstNode *left, AstNode *right);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    AstNode *getLeft();
    AstNode *getRight();

    bool equalTo(AstNode *node) override;

    ~OperationAddNode() override;
};

/**
 * Let Arg = Identifier|IntNumber|FloatNumber|Lparen|Lbracket|String;
 * Consist of <Arg><Operator "-"><Arg>
 */
class OperationSubtractNode : public AstNode {
private:
    AstNode *left;
    AstNode *right;

public:
    OperationSubtractNode(AstNode *left, AstNode *right);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    AstNode *getLeft();
    AstNode *getRight();

    bool equalTo(AstNode *node) override;

    ~OperationSubtractNode() override;
};

/**
 * Let Arg = Identifier|IntNumber|FloatNumber|Lparen|Lbracket|String;
 * Consist of <Arg><Operator "*"><Arg>
 */
class OperationMultiplyNode : public AstNode {
private:
    AstNode *left;
    AstNode *right;

public:
    OperationMultiplyNode(AstNode *left, AstNode *right);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    AstNode *getLeft();
    AstNode *getRight();

    bool equalTo(AstNode *node) override;

    ~OperationMultiplyNode() override;
};

/**
 * Let Arg = Identifier|IntNumber|FloatNumber|Lparen|Lbracket|String;
 * Consist of <Arg><Operator "/"><Arg>
 */
class OperationDivideNode : public AstNode {
private:
    AstNode *left;
    AstNode *right;

public:
    OperationDivideNode(AstNode *left, AstNode *right);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    AstNode *getLeft();
    AstNode *getRight();

    bool equalTo(AstNode *node) override;

    ~OperationDivideNode() override;
};

/**
 * Let Arg = Identifier|IntNumber|FloatNumber|Lparen|Lbracket|String;
 * Consist of <Arg><Operator "%"><Arg>
 */
class OperationModNode : public AstNode {
private:
    AstNode *left;
    AstNode *right;

public:
    OperationModNode(AstNode *left, AstNode *right);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    AstNode *getLeft();
    AstNode *getRight();

    bool equalTo(AstNode *node) override;

    ~OperationModNode() override;
};

/**
 * Consist of <IntNumber>
 */
class IntConstantNode : public AstNode {
private:
    long long value;

public:
    explicit IntConstantNode(long long value);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    long long getValue();

    bool equalTo(AstNode *node) override;

    ~IntConstantNode() override;
};

/**
 * Consist of <FloatNumber>
 */
class FloatConstantNode : public AstNode {
private:
    double value;

public:
    explicit FloatConstantNode(double value);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    double getValue();

    bool equalTo(AstNode *node) override;

    ~FloatConstantNode() override;
};

/**
 * Consist of <String>
 */
class StringConstantNode : public AstNode {
private:
    std::string value;

public:
    explicit StringConstantNode(std::string value);

    std::string toString() override;

    unsigned long getByteLength() override;
    unsigned long toBytes(void *buffer) override;
    unsigned long fromBytes(void *buffer) override;

    NodeType getType() override;

    std::string getValue();

    bool equalTo(AstNode *node) override;

    ~StringConstantNode() override;
};

struct PrioritizedOperator {
    AstNode::NodeType type;
    unsigned short priority;
};

class ParserException : public std::exception {
public:
    std::string message;

public:
    explicit ParserException(std::string message);
};

class Parser {
private:
    std::vector<Token> tokens;
    std::vector<AstNode *> programNodes;

private:
    std::vector<AstNode *> parseTokens();

public:
    explicit Parser(std::vector<Token> tokens);

    /*
        if (x)
        if (GetStatus())
        if (123)
        if (true)
        if ("string")
        if (x + 5)
        if (x + 5 > y)
    */
    ProgramNode *parse();

    std::tuple<SequenceNode *, unsigned long> parseSequence(unsigned long index, TokenType stop);
    /*
        TODO: Check for overflow only in "if", "while", "for", VarAssign, math operations, string
    */
    std::tuple<AstNode *, unsigned long> parseStatement(unsigned long index);
    std::tuple<ListDefinitionNode *, unsigned long> parseListDefinition(unsigned long index);
    std::tuple<AstNode *, unsigned long> parseExpression(unsigned long index);
    std::tuple<AstNode *, unsigned long> parseTerm(unsigned long index);
    std::tuple<FunctionCallNode *, unsigned long> parseFunctionCall(unsigned long index);
    std::tuple<SequenceNode *, unsigned long> parseEnclosed(unsigned long index, TokenType stop);
    std::vector<PrioritizedOperator> getPriorities(std::vector<Token> tokens);
    // AstNode *parseMath(std::vector<AstNode *> values, std::vector<PrioritizedOperator> operators);
};

}

#endif // REMAC_PARSER
