/**
 * @file parser.cpp
 * @author Пакуль Яўгеній Станіслававіч
 * 
 * @copyright Copyright (c) 2024 Пакуль Яўгеній Станіслававіч
 */

/*
  What will this shit even do?
    It will parse tokens from Lexer module to AST (Abstract Syntax Tree).
For example:
```
<Identifier "print">
<Lparen "(">
<Number "123">
<ArgumentSeparator ",">
<Identifier "abc">
<Operator "+">
<Number "2.4">
<Rparen ")">
```
will be parsed to
FunctionCall(name="print", args=[
    IntConstant(123),
    AddNumberOperation(
        Variable("abc"),
        FloatConstant(2.4),
    ),
])

  Why this shit even needed?
    Because then Executor module will search for FunctionCall or
IfStatement, WhileStatement, and so on, and (un/conditional) execute it.

  Is it waste of time?
    It's used in modern programming languages, so, probably not.

  Probably best implementation
    Split every node to it's own header file, make it so fucking OOP, that
even Lexer module (the most OOP shit I ever made) will be more functional.
*/

#include "belang/lexer.hpp"
#include <belang/parser.hpp>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <exception>
#include <iostream>
#include <vector>

namespace belang {

void Printable::print() {
    std::cout << this->toString() << std::endl;
}

AstNode::~AstNode() {}

AstNode::NodeType AstNode::getType() {
    return AstNode::NodeType::NODE_EMPTY;
}

SequenceNode::SequenceNode(std::vector<AstNode *> nodes) {
    this->nodes = nodes;
}

std::string SequenceNode::toString() {
    std::string str = "<SequenceNode: [";

    for (auto itr = this->nodes.cbegin(); itr != this->nodes.cend(); ++itr) {
        str += (*itr)->toString();

        if (itr + 1 != this->nodes.cend()) {
            str += ", ";
        }
    }

    str += "]>";
    return str;
}

unsigned long SequenceNode::getByteLength() {
    unsigned long totalSize = 0;

    for (auto itr = this->nodes.cbegin(); itr != this->nodes.cend(); ++itr) {
        totalSize += (*itr)->getByteLength();
    }

    return sizeof(unsigned int) + totalSize;
}

unsigned long SequenceNode::toBytes(void *buffer) {
    *((unsigned int *)buffer) = this->nodes.size();

    unsigned long offset = sizeof(unsigned int);

    for (auto itr = this->nodes.cbegin(); itr != this->nodes.cend(); ++itr) {
        offset += (*itr)->toBytes(((char *)buffer) + offset);
    }

    return offset;
}

unsigned long SequenceNode::fromBytes(void *buffer) {
    unsigned int size = *((unsigned int *)buffer);
    unsigned long i = 0;
    unsigned long offset = sizeof(unsigned int);
    this->nodes = std::vector<AstNode *>(size);

    while (i < size) {
        offset += this->nodes[i]->fromBytes((char *)buffer + offset);
        i++;
    }

    return offset;
}

AstNode::NodeType SequenceNode::getType() {
    return AstNode::NodeType::NODE_SEQUENCE;
}

std::vector<AstNode *> SequenceNode::getSequence() {
    return this->nodes;
}

SequenceNode::~SequenceNode() {
    for (auto itr = this->nodes.cbegin(); itr != this->nodes.cend(); ++itr) {
        delete (*itr);
    }
}

FunctionCallNode::FunctionCallNode(std::string name, SequenceNode *args) {
    this->name = name;
    this->args = args;
}

std::string FunctionCallNode::toString() {
    std::string str = "<FunctionCallNode name=";
    str += this->name;
    str += ", args=(";
    std::vector<AstNode *> args = this->args->getSequence();

    for (auto itr = args.cbegin(); itr != args.cend(); ++itr) {
        str += (*itr)->toString();

        if (itr + 1 != args.cend()) {
            str += ", ";
        }
    }

    str += ")>";
    return str;
}

unsigned long FunctionCallNode::getByteLength() {
    unsigned long length = sizeof(unsigned int);
    length += this->name.size();
    length += this->args->getByteLength();
    return length;
}

unsigned long FunctionCallNode::toBytes(void *buffer) {
    unsigned long offset = 0;
    *((unsigned int *)buffer) = this->name.size();
    offset += sizeof(unsigned int);
    memcpy((char *)buffer + offset, this->name.c_str(), this->name.size());
    offset += this->name.size();
    offset += this->args->toBytes((char *)buffer + offset);
    return offset;
}

unsigned long FunctionCallNode::fromBytes(void *buffer) {
    (void)buffer;
    throw std::exception(); // Not implemented yet
}

AstNode::NodeType FunctionCallNode::getType() {
    return AstNode::NodeType::NODE_FUNCTION_CALL;
}

std::string FunctionCallNode::getName() {
    return this->name;
}

SequenceNode *FunctionCallNode::getArgs() {
    return this->args;
}

FunctionCallNode::~FunctionCallNode() {
    delete this->args;
}

ProgramNode::ProgramNode(SequenceNode *body) {
    this->body = body;
}

std::string ProgramNode::toString() {
    std::string str = "<ProgramNode: ";
    str += this->body->toString();
    str += ">";
    return str;
}

unsigned long ProgramNode::getByteLength() {
    return this->body->getByteLength();
}

unsigned long ProgramNode::toBytes(void *buffer) {
    return this->body->toBytes(buffer);
}

unsigned long ProgramNode::fromBytes(void *buffer) {
    (void)buffer;
    throw std::exception(); // Not implemented yet
}

AstNode::NodeType ProgramNode::getType() {
    return AstNode::NodeType::NODE_PROGRAM;
}

SequenceNode *ProgramNode::getBody() {
    return this->body;
}

ProgramNode::~ProgramNode() {
    delete this->body;
}

IfStatementNode::IfStatementNode(AstNode *condition, SequenceNode *ifBranch, SequenceNode *elseBranch) {
    this->condition = condition;
    this->ifBranch = ifBranch;
    this->elseBranch = elseBranch;
}

std::string IfStatementNode::toString() {
    std::string str = "<IfStatementNode condition=";
    str += this->condition->toString();
    str += ", ifBranch=";
    str += this->ifBranch->toString();
    str += ", elseBranch=";
    str += this->elseBranch->toString();
    str += ">";
    return str;
}

unsigned long IfStatementNode::getByteLength() {
    return this->condition->getByteLength() + this->ifBranch->getByteLength() + this->elseBranch->getByteLength();
}

unsigned long IfStatementNode::toBytes(void *buffer) {
    unsigned long offset = this->condition->toBytes(buffer);
    offset += this->ifBranch->toBytes((char *)buffer + offset);
    offset += this->elseBranch->toBytes((char *)buffer + offset);
    return offset;
}

unsigned long IfStatementNode::fromBytes(void *buffer) {
    (void)buffer;
    throw std::exception(); // Not implemented yet
}

AstNode::NodeType IfStatementNode::getType() {
    return AstNode::NodeType::NODE_IF_STATEMENT;
}

AstNode *IfStatementNode::getCondition() {
    return this->condition;
}

SequenceNode *IfStatementNode::getBody() {
    return this->ifBranch;
}

SequenceNode *IfStatementNode::getElseBody() {
    return this->elseBranch;
}

IfStatementNode::~IfStatementNode() {
    delete this->condition;
    delete this->ifBranch;
    delete this->elseBranch;
}

WhileStatementNode::WhileStatementNode(AstNode *condition, SequenceNode *body) {
    this->condition = condition;
    this->body = body;
}

std::string WhileStatementNode::toString() {
    std::string str = "<WhileStatementNode condition=";
    str += this->condition->toString();
    str += ", body=";
    str += this->body->toString();
    str += ">";
    return str;
}

unsigned long WhileStatementNode::getByteLength() {
    return this->condition->getByteLength() + this->body->getByteLength();
}

unsigned long WhileStatementNode::toBytes(void *buffer) {
    unsigned long offset = this->condition->toBytes(buffer);
    offset += this->body->toBytes((char *)buffer + offset);
    return offset;
}

unsigned long WhileStatementNode::fromBytes(void *buffer) {
    (void)buffer;
    throw std::exception(); // Not implemented yet
}

AstNode::NodeType WhileStatementNode::getType() {
    return AstNode::NodeType::NODE_WHILE_STATEMENT;
}

AstNode *WhileStatementNode::getCondition() {
    return this->condition;
}

SequenceNode *WhileStatementNode::getBody() {
    return this->body;
}

WhileStatementNode::~WhileStatementNode() {
    delete this->condition;
    delete this->body;
}

ForStatementNode::ForStatementNode(SequenceNode *initializationBody, AstNode *condition, SequenceNode *incrementBody, SequenceNode *body) {
    this->initializationBody = initializationBody;
    this->condition = condition;
    this->incrementBody = incrementBody;
    this->body = body;
}

std::string ForStatementNode::toString() {
    std::string str = "<ForStatementNode initializationBody=";
    str += this->initializationBody->toString();
    str += ", condition=";
    str += this->condition->toString();
    str += ", incrementBody=";
    str += this->incrementBody->toString();
    str += ", body=";
    str += this->body->toString();
    str += ">";
    return str;
}

unsigned long ForStatementNode::getByteLength() {
    unsigned long length = this->condition->getByteLength();
    length += this->initializationBody->getByteLength();
    length += this->incrementBody->getByteLength();
    length += this->body->getByteLength();
    return length;
}

unsigned long ForStatementNode::toBytes(void *buffer) {
    unsigned long offset = this->condition->toBytes(buffer);
    offset += this->initializationBody->toBytes((char *)buffer + offset);
    offset += this->incrementBody->toBytes((char *)buffer + offset);
    offset += this->body->toBytes((char *)buffer + offset);
    return offset;
}

unsigned long ForStatementNode::fromBytes(void *buffer) {
    (void)buffer;
    throw std::exception(); // Not implemented yet
}

AstNode::NodeType ForStatementNode::getType() {
    return AstNode::NodeType::NODE_FOR_STATEMENT;
}

SequenceNode *ForStatementNode::getInitializationBody() {
    return this->initializationBody;
}

AstNode *ForStatementNode::getCondition() {
    return this->condition;
}

SequenceNode *ForStatementNode::getIncrementBody() {
    return this->incrementBody;
}

SequenceNode *ForStatementNode::getBody() {
    return this->body;
}

ForStatementNode::~ForStatementNode() {
    delete this->condition;
    delete this->initializationBody;
    delete this->incrementBody;
    delete this->body;
}

VariableAssignmentNode::VariableAssignmentNode(std::string name, AstNode *value) {
    this->name = name;
    this->value = value;
}

std::string VariableAssignmentNode::toString() {
    std::string str = "<VariableAssignmentNode name=";
    str += this->name;
    str += ", value=";
    str += this->value->toString();
    str += ">";
    return str;
}

unsigned long VariableAssignmentNode::getByteLength() {
    return sizeof(unsigned int) + this->name.size() + this->value->getByteLength();
}

unsigned long VariableAssignmentNode::toBytes(void *buffer) {
    *((unsigned int *)buffer) = this->name.size();
    unsigned long offset = sizeof(unsigned int);
    memcpy((char *)buffer + offset, this->name.c_str(), this->name.size());
    offset += this->name.size();
    offset += this->value->toBytes((char *)buffer + offset);
    return offset;
}

unsigned long VariableAssignmentNode::fromBytes(void *buffer) {
    (void)buffer;
    throw std::exception(); // Not implemented yet
}

AstNode::NodeType VariableAssignmentNode::getType() {
    return AstNode::NodeType::NODE_VARIABLE_ASSIGNMENT;
}

std::string VariableAssignmentNode::getName() {
    return this->name;
}

AstNode *VariableAssignmentNode::getValue() {
    return this->value;
}

VariableAssignmentNode::~VariableAssignmentNode() {
    delete this->value;
}

ListDefinitionNode::ListDefinitionNode(SequenceNode *array) {
    this->array = array;
}

std::string ListDefinitionNode::toString() {
    return "<ListDefinitionNode: [todo!()]>";
}

unsigned long ListDefinitionNode::getByteLength() {
    return this->array->getByteLength();
}

unsigned long ListDefinitionNode::toBytes(void *buffer) {
    return this->array->toBytes(buffer);
}

unsigned long ListDefinitionNode::fromBytes(void *buffer) {
    return this->array->fromBytes(buffer);
}

AstNode::NodeType ListDefinitionNode::getType() {
    return AstNode::NodeType::NODE_LIST_DEFINITION;
}

SequenceNode *ListDefinitionNode::getArray() {
    return this->array;
}

ListDefinitionNode::~ListDefinitionNode() {
    delete this->array;
}

ListSliceNode::ListSliceNode(AstNode *array, AstNode *value) {
    this->array = array;
    this->value = value;
}

std::string ListSliceNode::toString() {
    return "<ListSliceNode array=" + this->array->toString() + ", value=" + this->value->toString() + ">";
}

unsigned long ListSliceNode::getByteLength() {
    return this->array->getByteLength() + this->value->getByteLength();
}

unsigned long ListSliceNode::toBytes(void *buffer) {
    unsigned long offset = 0;
    offset += this->array->toBytes(buffer);
    offset += this->value->toBytes((char *)buffer + offset);
    return offset;
}

unsigned long ListSliceNode::fromBytes(void *buffer) {
    unsigned long offset = 0;
    offset += this->array->fromBytes(buffer);
    offset += this->value->fromBytes((char *)buffer + offset);
    return offset;
}

AstNode::NodeType ListSliceNode::getType() {
    return AstNode::NodeType::NODE_LIST_SLICE;
}

ListSliceNode::~ListSliceNode() {
    delete this->array;
    delete this->value;
}

VariableReferenceNode::VariableReferenceNode(std::string name) {
    this->name = name;
}

std::string VariableReferenceNode::toString() {
    return "<VariableReferenceNode name=\"" + this->name + "\">";
}

unsigned long VariableReferenceNode::getByteLength() {
    return sizeof(unsigned int) + this->name.size();
}

unsigned long VariableReferenceNode::toBytes(void *buffer) {
    *((unsigned int *)buffer) = this->name.size();
    std::memcpy(buffer, this->name.c_str(), this->name.size());
    return this->name.size();
}

unsigned long VariableReferenceNode::fromBytes(void *buffer) {
    (void)buffer;
    throw new std::exception(); // not implemented
}

AstNode::NodeType VariableReferenceNode::getType() {
    return AstNode::NodeType::NODE_VARIABLE_REFERENCE;
}

std::string VariableReferenceNode::getName() {
    return this->name;
}

VariableReferenceNode::~VariableReferenceNode() {}

OperationAddNode::OperationAddNode(AstNode *left, AstNode *right) {
    this->left = left;
    this->right = right;
}

std::string OperationAddNode::toString() {
    std::string str = "<OperationAddNode left=";
    str += this->left->toString();
    str += ", right=";
    str += this->right->toString();
    str += ">";
    return str;
}

unsigned long OperationAddNode::getByteLength() {
    return this->left->getByteLength() + this->right->getByteLength();
}

unsigned long OperationAddNode::toBytes(void *buffer) {
    unsigned long offset = 0;
    offset += this->left->toBytes((char *)buffer + offset);
    offset += this->right->toBytes((char *)buffer + offset);
    return offset;
}

unsigned long OperationAddNode::fromBytes(void *buffer) {
    (void)buffer;
    throw std::exception(); // Not implemented yet
}

AstNode::NodeType OperationAddNode::getType() {
    return AstNode::NodeType::NODE_OPERATION_ADD;
}

AstNode *OperationAddNode::getLeft() {
    return this->left;
}

AstNode *OperationAddNode::getRight() {
    return this->right;
}

OperationAddNode::~OperationAddNode() {
    delete this->left;
    delete this->right;
}

OperationSubtractNode::OperationSubtractNode(AstNode *left, AstNode *right) {
    this->left = left;
    this->right = right;
}

std::string OperationSubtractNode::toString() {
    std::string str = "<OperationSubtractNode left=";
    str += this->left->toString();
    str += ", right=";
    str += this->right->toString();
    str += ">";
    return str;
}

unsigned long OperationSubtractNode::getByteLength() {
    return this->left->getByteLength() + this->right->getByteLength();
}

unsigned long OperationSubtractNode::toBytes(void *buffer) {
    unsigned long offset = 0;
    offset += this->left->toBytes((char *)buffer + offset);
    offset += this->right->toBytes((char *)buffer + offset);
    return offset;
}

unsigned long OperationSubtractNode::fromBytes(void *buffer) {
    (void)buffer;
    throw std::exception(); // Not implemented yet
}

AstNode::NodeType OperationSubtractNode::getType() {
    return AstNode::NodeType::NODE_OPERATION_ADD;
}

AstNode *OperationSubtractNode::getLeft() {
    return this->left;
}

AstNode *OperationSubtractNode::getRight() {
    return this->right;
}

OperationSubtractNode::~OperationSubtractNode() {
    delete this->left;
    delete this->right;
}

OperationMultiplyNode::OperationMultiplyNode(AstNode *left, AstNode *right) {
    this->left = left;
    this->right = right;
}

std::string OperationMultiplyNode::toString() {
    std::string str = "<OperationMultiplyNode left=";
    str += this->left->toString();
    str += ", right=";
    str += this->right->toString();
    str += ">";
    return str;
}

unsigned long OperationMultiplyNode::getByteLength() {
    return this->left->getByteLength() + this->right->getByteLength();
}

unsigned long OperationMultiplyNode::toBytes(void *buffer) {
    unsigned long offset = 0;
    offset += this->left->toBytes((char *)buffer + offset);
    offset += this->right->toBytes((char *)buffer + offset);
    return offset;
}

unsigned long OperationMultiplyNode::fromBytes(void *buffer) {
    (void)buffer;
    throw std::exception(); // Not implemented yet
}

AstNode::NodeType OperationMultiplyNode::getType() {
    return AstNode::NodeType::NODE_OPERATION_ADD;
}

AstNode *OperationMultiplyNode::getLeft() {
    return this->left;
}

AstNode *OperationMultiplyNode::getRight() {
    return this->right;
}

OperationMultiplyNode::~OperationMultiplyNode() {
    delete this->left;
    delete this->right;
}

OperationDivideNode::OperationDivideNode(AstNode *left, AstNode *right) {
    this->left = left;
    this->right = right;
}

std::string OperationDivideNode::toString() {
    std::string str = "<OperationDivideNode left=";
    str += this->left->toString();
    str += ", right=";
    str += this->right->toString();
    str += ">";
    return str;
}

unsigned long OperationDivideNode::getByteLength() {
    return this->left->getByteLength() + this->right->getByteLength();
}

unsigned long OperationDivideNode::toBytes(void *buffer) {
    unsigned long offset = 0;
    offset += this->left->toBytes((char *)buffer + offset);
    offset += this->right->toBytes((char *)buffer + offset);
    return offset;
}

unsigned long OperationDivideNode::fromBytes(void *buffer) {
    (void)buffer;
    throw std::exception(); // Not implemented yet
}

AstNode::NodeType OperationDivideNode::getType() {
    return AstNode::NodeType::NODE_OPERATION_ADD;
}

AstNode *OperationDivideNode::getLeft() {
    return this->left;
}

AstNode *OperationDivideNode::getRight() {
    return this->right;
}

OperationDivideNode::~OperationDivideNode() {
    delete this->left;
    delete this->right;
}

OperationModNode::OperationModNode(AstNode *left, AstNode *right) {
    this->left = left;
    this->right = right;
}

std::string OperationModNode::toString() {
    std::string str = "<OperationModNode left=";
    str += this->left->toString();
    str += ", right=";
    str += this->right->toString();
    str += ">";
    return str;
}

unsigned long OperationModNode::getByteLength() {
    return this->left->getByteLength() + this->right->getByteLength();
}

unsigned long OperationModNode::toBytes(void *buffer) {
    unsigned long offset = 0;
    offset += this->left->toBytes((char *)buffer + offset);
    offset += this->right->toBytes((char *)buffer + offset);
    return offset;
}

unsigned long OperationModNode::fromBytes(void *buffer) {
    (void)buffer;
    throw std::exception(); // Not implemented yet
}

AstNode::NodeType OperationModNode::getType() {
    return AstNode::NodeType::NODE_OPERATION_ADD;
}

AstNode *OperationModNode::getLeft() {
    return this->left;
}

AstNode *OperationModNode::getRight() {
    return this->right;
}

OperationModNode::~OperationModNode() {
    delete this->left;
    delete this->right;
}

IntConstantNode::IntConstantNode(long long value) {
    this->value = value;
}

std::string IntConstantNode::toString() {
    std::string str = "<IntConstantNode value=";
    str += this->value;
    str += ">";
    return str;
}

unsigned long IntConstantNode::getByteLength() {
    return sizeof(unsigned long long);
}

unsigned long IntConstantNode::toBytes(void *buffer) {
    *((unsigned long long *)buffer) = this->value;
    return sizeof(unsigned long long);
}

unsigned long IntConstantNode::fromBytes(void *buffer) {
    (void)buffer;
    throw std::exception(); // Not implemented yet
}

AstNode::NodeType IntConstantNode::getType() {
    return AstNode::NodeType::NODE_INT_CONSTANT;
}

long long IntConstantNode::getValue() {
    return this->value;
}

IntConstantNode::~IntConstantNode() {}

FloatConstantNode::FloatConstantNode(double value) {
    this->value = value;
}

std::string FloatConstantNode::toString() {
    std::string str = "<FloatConstantNode value=";
    str += this->value;
    str += ">";
    return str;
}

unsigned long FloatConstantNode::getByteLength() {
    return sizeof(double);
}

unsigned long FloatConstantNode::toBytes(void *buffer) {
    *((double *)buffer) = this->value;
    return sizeof(double);
}

unsigned long FloatConstantNode::fromBytes(void *buffer) {
    (void)buffer;
    throw std::exception(); // Not implemented yet
}

AstNode::NodeType FloatConstantNode::getType() {
    return AstNode::NodeType::NODE_FLOAT_CONSTANT;
}

double FloatConstantNode::getValue() {
    return this->value;
}

FloatConstantNode::~FloatConstantNode() {}

StringConstantNode::StringConstantNode(std::string value) {
    this->value = value;
}

std::string StringConstantNode::toString() {
    std::string str = "<StringConstantNode value=\"";
    str += this->value;
    str += "\">";
    return str;
}

unsigned long StringConstantNode::getByteLength() {
    return this->value.size();
}

unsigned long StringConstantNode::toBytes(void *buffer) {
    memcpy(buffer, this->value.c_str(), this->value.size());
    return this->value.size();
}

unsigned long StringConstantNode::fromBytes(void *buffer) {
    (void)buffer;
    throw std::exception(); // Not implemented yet
}

AstNode::NodeType StringConstantNode::getType() {
    return AstNode::NodeType::NODE_STRING_CONSTANT;
}

std::string StringConstantNode::getValue() {
    return this->value;
}

StringConstantNode::~StringConstantNode() {}

ParserException::ParserException(std::string message) {
    this->message = message;
}

/*
    (1 + 2 + 3 * 4 + 5) + 8 % funcName()
    parseExpression()
    Finding Lparens. Found at position 1
    parseParen()
        #Not starting with Lparen
        #Int/float constant found, so
        Prioritized values:
        1 prior1 2 prior1 3 prior2 4 prior1 5
        Expand to:
        (1 + 2) + ((3 * 4) + 5)
        That corresponds to:
        AddNode(AddNode(1, 2), AddNode(MultiplyNode(3, 4), 5))
    Middle result:
        <AddNode(AddNode(1, 2), AddNode(MultiplyNode(3, 4), 5))> + 8 % funcName()
    Is a top-level (non-paren), but anyways
    parseParen()
        Prioritized values:
        <...> prior1 8 prior2 funcName()
        Expand to:
        (<...> + 8) % funcName()
        That corresponds to:
        ModNode(AddNode(AddNode(AddNode(1, 2), AddNode(MultiplyNode(3, 4), 5)), 8), FuncCall("funcName"))

Program(ModNode(AddNode(AddNode(AddNode(1, 2), AddNode(MultiplyNode(3, 4), 5)), 8), FuncCall("funcName")))

Hardest part: How to transform ... prior* ... results to AST?

Given: 1 prior1 2 prior1 3 prior2 4 prior1 5

Find first highest priority operator: third
    Group left and right numbers to one node:
        MultiplyNode(3, 4)
Middle result: 1 prior1 2 prior1 <MultiplyNode(3, 4)> prior1 5
Find first highest priority operator: first
    Group left and right numbers to one node:
        AddNode(1, 2)
Middle result: <AddNode(1, 2)> prior1 <MultiplyNode(3, 4)> prior1 5
Find first highest priority operator: first
    Group left and right numbers to one node:
        AddNode(AddNode(1, 2), MultiplyNode(3, 4))
Middle result: <AddNode(AddNode(1, 2), MultiplyNode(3, 4))> prior1 5
Find first highest priority operator: first
    Group left and right numbers to one node:
        AddNode(AddNode(AddNode(1, 2), MultiplyNode(3, 4)), 5)
Middle result: <AddNode(AddNode(AddNode(1, 2), MultiplyNode(3, 4)), 5)>
No more operators left.
ez

Next hard part: How to mix result and operators, do that left/right shit with operators?

Simple parse all as parser nodes (ints to IntConstant, floats to FloatConstant, etc.)
Then there are 2 vectors: numbers and operators.
Length of numbers is always equal to length of operators + 1
To get left number from operator, simply do numbers[operator_index]
To get right number from operator, simply do numbers[operator_index + 1]
When parsing numbers:
    remove parsed operator
    remove right number
    change left number to parsed result
When there are no operators left (or (same because of length + 1) - there is only 1 number), return last number.
*/

/*
Code implementation

Given code:
```
array = [1, 2, GetTimeSeconds()]
SetTimeSeconds(array[2] + 1)

if (array[0] == 1) {
    while (array[1] >= 2) {
        Print(array[0])
        array[1] = array[1] - 1
    }
} else {
    for ({i = 0}, i < 2, {i = i + 1}) {
        Print(1 + 8 * 2 * i - i / 5)
    }
}
```

That corresponds to next lexer tokens:
[
    0. ident(array), oper(=), lbracket, int(1), comma, int(2), comma, ident(GetTimeSeconds), lparen, rparen, rbracket,
    11. ident(SetTimeSeconds), lparen, ident(array), lbracket, int(2), rbracket, oper(+), int(1), rparen,
    20. keywd(if), lparen, ident(array), lbracket, int(0), rbracket, oper(==), int(1), rparen, lbrace,
    30. keywd(while), lparen, ident(array), lbracket, int(1), rbracket, oper(>=), int(2), rparen, lbrace,
    40. ident(Print), lparen, ident(array), lbracket, int(0), rbracket, rparen,
    47. ident(array), lbracket, int(1), rbracket, oper(=), ident(array), lbracket, int(1), rbracket, oper(-), int(1),
    58. rbrace,
    59. rbrace, keywd(else), lbrace,
    62. keywd(for), lparen, lbrace, ident(i), oper(=), int(0), rbrace, comma, ident(i), oper(<), int(2), comma, lbrace, ident(i), oper(=), ident(i), oper(+), int(1), rbrace, rparen, lbrace,
    83. ident(Print), lparen, int(1), oper(+), int(8), oper(*), int(2), oper(*), ident(i), oper(-), ident(i), oper(/), int(5), rparen,
    97. rbrace,
    98. rbrace
]

parse(tokens)
    parseSequence(0, end=nullptr)
        # Have lines, continue
        parseStatement(0)
            # First token is identifier, check second
            # Second token is operator "=", it's variable assignment
            # So, third token is new value of varaible, that starts with "[", so
            parseArrayDefinition(2)
                # Next token isn't rbracket, so continue
                parseExpression(3)
                    parseTerm()
                        # Found int(1), returning IntegerConstantNode(1)
                # Next token is comma, so continue
                parseExpression(5)
                    parseTerm()
                        # Found int(2), returning IntegerConstantNode(2)
                # Next token is comma, so continue
                parseExpression(7)
                    parseTerm()
                        # Found ident(GetTimeSeconds)
                        # Next token is lparen, so it's a function call
                        parseFunctionCall()
                            # Function name is GetTimeSeconds
                            parseEnclosed(stop=rparen)
                                # Next token is rparen, so return
                                # Return value: SequenceNode()
                            # Return value: FunctionCallNode("GetTimeSeconds", SequenceNode())
                        # Return value: FunctionCallNode("GetTimeSeconds", SequenceNode())
                # Next token isn't comma, maybe throw error?
                # Next token is rbracket, so don't throw error, but return
                # Return value: ArrayDefinitionNode(SequenceNode(IntegerConstantNode(1), IntegerConstantNode(2), FunctionCallNode("GetTimeSeconds", SequenceNode())))
            # Return value as in child
        # Have at least one line, continue
        parseStatement(11)
        # First token is identifier, check second
        # Second token is lparen, it's function call
            parseEnclosed(12, stop=rparen)
                # Next token isn't rparen, continue
                parseExpression(13)
                    parseTerm()
                        # Found ident, check next
                        # Next token is lbracket, it's array slice
                        parseExpression(15)
                            parseTerm()
                                # Simple int constant, returning IntConstantValue(2)
                        # Returning ArraySliceNode("array", IntConstantValue(2))
                    # Next token is math operator, so
                    # Parse every shit in between all math operators, the most left is already parsed
                    parseTerm()
                        # Found int constant, so returing IntConstantValue(1)
                    # Now get priorities by operators
                    getPriorities()
                        # Return values: [ArraySliceNode("array", IntConstantValue(2)), IntConstantValue(1)]
                        # Return operators: [PrioritizedOperator("+", 1)]
                    parseMath(values, operators)
                        # Max. priority operator is '+', so
                        # Add token OperationAddNode(ArraySliceNode("array", IntConstantValue(2)), IntConstantValue(1))
                        # No more operators left, return OperationAddNode(ArraySliceNode("array", IntConstantValue(2)), IntConstantValue(1))
            # Return value: OperationAddNode(ArraySliceNode("array", IntConstantValue(2)), IntConstantValue(1))
*/
Parser::Parser(std::vector<Token> tokens) {
    this->tokens = tokens;
}

ProgramNode *Parser::parse() {
    return new ProgramNode(std::get<0>(this->parseSequence(0, TokenType::PROGRAM_START)));
}

std::tuple<SequenceNode *, unsigned long> Parser::parseSequence(unsigned long index, TokenType stop) {
    if (this->tokens.size() <= index) {
        throw new ParserException("Index is bigger than tokens length");
    }

    unsigned long length = 0;
    std::vector<AstNode *> nodes;

    while (index < this->tokens.size()) {
        if (this->tokens[index].type == stop) {
            return { new SequenceNode(nodes), length };
        }

        std::tuple<AstNode *, unsigned long> statement = this->parseStatement(index);
        nodes.push_back(std::get<0>(statement));
        unsigned long statementLength = std::get<1>(statement);
        index += statementLength;
        length += statementLength;
    }

    return { new SequenceNode(nodes), length };
}

// (nullptr, 0) = ParserError
std::tuple<AstNode *, unsigned long> Parser::parseStatement(unsigned long index) {
    /*
    At this moment we have only statement, which result is unused.
    Result may be used in that statement (ex. function call, var assign, etc.).
    But now, we can simply omit it, to increase lang performance.
    */
    Token *token = &this->tokens[index]; // Take address as optimization

    switch ((unsigned char)token->type) {
        case TokenType::FLOAT_NUMBER:
        case TokenType::INT_NUMBER:
        case TokenType::STRING:
        {
            return { nullptr, 1 };
        }

        case TokenType::LPAREN: {
            return this->parseStatement(index + 1);
        }
        case TokenType::LBRACKET: {
            return { nullptr, std::get<1>(this->parseListDefinition(index)) };
        }

        case TokenType::IDENTIFIER: {
            Token *nextToken = &this->tokens[index + 1];

            if (nextToken->type == TokenType::LPAREN) {
                return parseFunctionCall(index);
            } else if (nextToken->type == TokenType::OPERATOR && nextToken->content == "=") {
                std::tuple<AstNode *, unsigned long> expr = parseExpression(index + 2);
                return { new VariableAssignmentNode(token->content, std::get<0>(expr)), 2 + std::get<1>(expr) };
            }

            break;
        }

        case TokenType::KEYWORD: {
            if (token->content == "if") {
                if (this->tokens[index + 1].type != TokenType::LPAREN) {
                    throw new ParserException("Expected left parentheses ('(')");
                }

                std::tuple<AstNode *, unsigned long> condition = parseExpression(index + 1);
                unsigned long length = 1 + std::get<1>(condition);

                if (this->tokens[index + length].type != TokenType::LBRACE) {
                    throw new ParserException("Expected left brace ('{')");
                }

                std::tuple<SequenceNode *, unsigned long> ifBranch = parseSequence(index + length + 2, TokenType::RBRACE);
                length += 2 + std::get<1>(ifBranch) + 2;
                Token *elseToken = &this->tokens[index + length];

                if (elseToken->type == TokenType::KEYWORD && elseToken->content == "else") {
                    if (this->tokens[index + length + 1].type == TokenType::KEYWORD && this->tokens[index + length + 1].content == "if") {
                        std::tuple<AstNode *, unsigned long> elseStatement = this->parseStatement(index + length + 1);
                        length += 1 + std::get<1>(elseStatement);
                        std::vector<AstNode *> nodes;
                        nodes.push_back(std::get<0>(elseStatement));
                        return { new IfStatementNode(std::get<0>(condition), std::get<0>(ifBranch), new SequenceNode(nodes)), length }; // FIXME: mb + 1? and mb + 1 + 2?
                    }

                    std::tuple<SequenceNode *, unsigned long> elseSequence = this->parseSequence(index + length + 2, TokenType::RBRACE);
                    length += std::get<1>(elseSequence);
                    return { new IfStatementNode(std::get<0>(condition), std::get<0>(ifBranch), std::get<0>(elseSequence)), length + 2 };
                }

                return { new IfStatementNode(std::get<0>(condition), std::get<0>(ifBranch), nullptr), length - 1 };
            }
        }
    }

    throw new ParserException("Invalid statement start");
}

std::tuple<ListDefinitionNode *, unsigned long> Parser::parseListDefinition(unsigned long index) {
    std::tuple<SequenceNode *, unsigned long> sequence = this->parseEnclosed(index + 1, TokenType::RBRACKET);
    return { new ListDefinitionNode(std::get<0>(sequence)), std::get<1>(sequence) + 1 };
}

std::tuple<AstNode *, unsigned long> Parser::parseExpression(unsigned long index) {
    std::tuple<AstNode *, unsigned long> leftTerm = Parser::parseTerm(index);
    unsigned long termTokensLength = std::get<1>(leftTerm);
    index += termTokensLength;
    unsigned long length = termTokensLength;
    AstNode *leftNode = std::get<0>(leftTerm);

    if (this->tokens[index].type == TokenType::OPERATOR) {
        if (this->tokens[index].content == "=") {
            throw new ParserException("No assignment is allowed inside an expression");
        }

        std::vector<AstNode *> terms;
        std::vector<Token> unpriOperators;

        for (unsigned long i = index; i < this->tokens.size(); i++, length++) {
            if (this->tokens[i].type == TokenType::RPAREN || this->tokens[i].type == TokenType::RBRACKET || this->tokens[i].type == TokenType::RBRACE) {
                break;
            }

            if (this->tokens[i].type == TokenType::OPERATOR) {
                unpriOperators.push_back(this->tokens[i]);
            } else {
                std::tuple<AstNode *, unsigned long> term = this->parseTerm(i);
                unsigned long tokensLength = std::get<1>(term) - 1; // - 1 because it will increment after "continue"
                i += tokensLength;
                length += tokensLength;
                terms.push_back(std::get<0>(term));
                continue;
            }
        }

        if (terms.size() != unpriOperators.size() + 1) {
            throw new ParserException("Invalid operators count");
        }

        std::vector<PrioritizedOperator> operators = getPriorities(unpriOperators);

        while (operators.size() > 0) {
            auto maxPriorityOper = operators.begin();
            unsigned short maxPriority = operators[0].priority;

            for (auto itr = operators.begin(); itr != operators.end(); ++itr) {
                if (itr->priority > maxPriority) {
                    maxPriorityOper = itr;
                    maxPriority = itr->priority;
                }
            }

            AstNode *newValue;

            switch (maxPriorityOper->type) {
                case AstNode::NodeType::NODE_OPERATION_ADD: {
                    newValue = new OperationAddNode(terms[maxPriorityOper - operators.begin()], terms[maxPriorityOper - operators.begin() + 1]);
                    break;
                }
                case AstNode::NodeType::NODE_OPERATION_SUBTRACT: {
                    newValue = new OperationSubtractNode(terms[maxPriorityOper - operators.begin()], terms[maxPriorityOper - operators.begin() + 1]);
                    break;
                }
                case AstNode::NodeType::NODE_OPERATION_MULTIPLY: {
                    newValue = new OperationMultiplyNode(terms[maxPriorityOper - operators.begin()], terms[maxPriorityOper - operators.begin() + 1]);
                    break;
                }
                case AstNode::NodeType::NODE_OPERATION_DIVIDE: {
                    newValue = new OperationDivideNode(terms[maxPriorityOper - operators.begin()], terms[maxPriorityOper - operators.begin() + 1]);
                    break;
                }
                case AstNode::NodeType::NODE_OPERATION_MOD: {
                    newValue = new OperationModNode(terms[maxPriorityOper - operators.begin()], terms[maxPriorityOper - operators.begin() + 1]);
                    break;
                }
                default: {
                    throw new ParserException("Unexpected operator type");
                }
            }

            operators.erase(maxPriorityOper);
            terms[maxPriorityOper - operators.begin()] = newValue;
            terms.erase(terms.begin() + (maxPriorityOper - operators.begin()) + 1);
        }

        return { terms[0], length };
    }

    return { leftNode, length };
}

std::tuple<AstNode *, unsigned long> Parser::parseTerm(unsigned long index) {
    // TODO: ALL INDEX IN RETURNS MUST POINT TO NEXT TOKEN AFTER LAST PROCESSED

    switch (this->tokens[index].type) {
        case TokenType::LBRACKET: {
            std::tuple<ListDefinitionNode *, unsigned long> listDefinition = parseListDefinition(index);
            return listDefinition;
        }
        case TokenType::IDENTIFIER: {
            if (this->tokens[index + 1].type == TokenType::RPAREN) {
                std::tuple<FunctionCallNode *, unsigned long> functionCall = parseFunctionCall(index);
                return functionCall;
            }

            return { new VariableReferenceNode(this->tokens[index].content), 1 };
        }
        case TokenType::INT_NUMBER: {
            return { new IntConstantNode(std::strtoll(this->tokens[index].content.c_str(), nullptr, 10)), 1 };
        }
        case TokenType::FLOAT_NUMBER: {
            return { new FloatConstantNode(std::strtod(this->tokens[index].content.c_str(), nullptr)), 1 };
        }
        case TokenType::LPAREN: {
            std::tuple<AstNode *, unsigned long> expr = parseExpression(index + 1);
            unsigned long tokensLength = std::get<1>(expr);
            return { std::get<0>(expr), tokensLength + 1 };
        }
        case TokenType::STRING: {
            return { new StringConstantNode(this->tokens[index].content), 1 };
        }
        default: {
            throw new ParserException("Unexpected token, while parsing term");
        }
    }

    throw new ParserException("Unexpected token");
}

std::tuple<FunctionCallNode *, unsigned long> Parser::parseFunctionCall(unsigned long index) {
    if (this->tokens[index].type != TokenType::IDENTIFIER) {
        throw new ParserException("Expected identifier to start function call");
    }

    std::tuple<SequenceNode *, unsigned long> sequence = this->parseEnclosed(index + 1, TokenType::RPAREN);
    return { new FunctionCallNode(this->tokens[index].content, std::get<0>(sequence)), std::get<1>(sequence) + 1 };
}

std::tuple<SequenceNode *, unsigned long> Parser::parseEnclosed(unsigned long index, TokenType stop) {
    unsigned long length = 0;
    std::vector<AstNode *> nodes;

    while (index < this->tokens.size() && this->tokens[index].type != stop) {
        std::tuple<AstNode *, unsigned long> expr = parseExpression(index);
        unsigned long tokensLength = std::get<1>(expr);
        index += tokensLength;
        length += tokensLength;
        nodes.push_back(std::get<0>(expr));
    }

    return { new SequenceNode(nodes), length };
}

std::vector<PrioritizedOperator> Parser::getPriorities(std::vector<Token> tokens) {
    std::vector<PrioritizedOperator> opers;

    for (auto itr = tokens.cbegin(); itr != tokens.cend(); ++itr) {
        if (itr->type != TokenType::OPERATOR) {
            throw new ParserException("Expected operator");
        }

        if (itr->content == "+" || itr->content == "-") {
            opers.push_back(PrioritizedOperator { .type = AstNode::NodeType::NODE_OPERATION_ADD, .priority = 101 });
        } else if (itr->content == "*" || itr->content == "/" || itr->content == "%") {
            opers.push_back(PrioritizedOperator { .type = AstNode::NodeType::NODE_OPERATION_ADD, .priority = 102 });
        } else {
            throw new ParserException("Unknown operator");
        }
    }

    return opers;
}

}
