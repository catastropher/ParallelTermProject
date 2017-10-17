#include <iostream>

#include "Parser.hpp"
#include "Lexer.hpp"

Parser::Parser(std::vector<Token>& tokens_)
    : tokens(&tokens_),
    currentTokenId(0),
    lastToken(Token(TOK_INVALID, "", -1, -1)),
    nextLabelId(0)
{
    
}

Ast& Parser::parse()
{
    parseHeader();
    
    CodeBlockNode* body = parseCodeBlock(TOK_END);
    body->addStatement(ast.addEndNode());
    ast.setBody(body);
    
    return ast;
}

ExpressionNode* Parser::Parser::parseExpression()
{
    ExpressionNode* result = parseTerm();
    
    while(currentToken().type == TOK_ADD || currentToken().type == TOK_SUB)
    {
        TokenType op = currentToken().type;
        nextToken();
        ExpressionNode* right = parseTerm();
        
        result = ast.newBinaryOpNode(result, op, right);
    }
    
    return result;
}

FactorNode* Parser::parseVarFactor()
{
    VarDeclNode* var = ast.getVarByName(currentToken().value);
    FactorNode* newNode = NULL;
    
    std::string name = currentToken().value;
    
    if(!var)
        throwErrorAtCurrentLocation("No such variable " + name);
    
    bool arrayAccess = peekNextToken().type == TOK_LSQUARE_BRACKET;
    
    if(IntDeclNode* intVar = dynamic_cast<IntDeclNode*>(var))
    {
        if(arrayAccess)
            throwErrorAtCurrentLocation("Variable " + name + " does not have list type");
        
        newNode = ast.addIntVarFactor(intVar);
        nextToken();
    }
    else if(OneDimensionalListDecl* listVar = dynamic_cast<OneDimensionalListDecl*>(var))
    {
        if(!arrayAccess)
            throwErrorAtCurrentLocation("Variable " + name + " has array type - expected '['");
        
        nextToken();
        expectType(TOK_LSQUARE_BRACKET);
        nextToken();
        
        ExpressionNode* index = parseExpression();
        
        expectType(TOK_RSQUARE_BRACKET);
        nextToken();
        
        newNode = ast.addOneDimensionalListFactor(listVar, index);
    }
    else
    {
        throwErrorAtCurrentLocation("Variable " + name + " does not have integer type");
    }
    
    return newNode;
}

ExpressionNode* Parser::parseFactor()
{
    TokenType unaryOp = TOK_INVALID;
    
    if(currentToken().type == TOK_ADD || currentToken().type == TOK_SUB)
    {
        unaryOp = currentToken().type;
        nextToken();
    }
    
    ExpressionNode* newNode = NULL;
    
    if(currentToken().type == TOK_NUMBER)
    {
        newNode = ast.newIntegerNode(atoi(currentToken().value.c_str()));
        nextToken();
    }
    else if(currentToken().type == TOK_ID)
    {
        newNode = parseVarFactor();
    }
    else if(currentToken().type == TOK_LPAREN)
    {
        nextToken();
        newNode = parseExpression();
        
        expectType(TOK_RPAREN);
        nextToken();
    }
    
    if(unaryOp != TOK_INVALID)
        return ast.newUnaryOpNode(newNode, unaryOp);
    
    return newNode;
}

ExpressionNode* Parser::parseTerm()
{
    ExpressionNode* result = parseFactor();
    
    while(currentToken().type == TOK_MUL || currentToken().type == TOK_DIV || currentToken().type == TOK_MOD)
    {
        TokenType op = currentToken().type;
        nextToken();
        ExpressionNode* right = parseFactor();
        
        result = ast.newBinaryOpNode(result, op, right);
    }
    
    return result;
}

void Parser::throwErrorAtCurrentLocation(std::string errorMessage)
{
    std::cerr << "Error on line " << currentToken().line << ", col " << currentToken().col << ": " << errorMessage << std::endl;
    
    throw "Error in parsing";
}

bool Parser::parseVarDecl()
{    
    if(currentToken().type == TOK_INT)
    {
        nextToken();
        expectType(TOK_ID);
        ast.addIntegerVar(currentToken().value, currentToken().line, currentToken().col);
        nextToken();
        return true;
    }
    
    if(currentToken().type == TOK_LIST)
    {
        nextToken();
        expectType(TOK_LSQUARE_BRACKET);
        nextToken();
        
        expectType(TOK_NUMBER);        
        int totalElements = atoi(currentToken().value.c_str());
        nextToken();
        
        expectType(TOK_RSQUARE_BRACKET);
        nextToken();
        
        expectType(TOK_ID);
        
        ast.add1DListVar(currentToken().value, currentToken().line, currentToken().col, totalElements);
        nextToken();
        
        return true;
    }
    
    return false;
}

void Parser::parseHeader()
{
    do
    {
        if(currentToken().type == TOK_VAR)
        {
            nextToken();
            while(parseVarDecl()) ;
        }
        else if(currentToken().type == TOK_BEGIN)
        {
            nextToken();
            return;
        }
        else if(currentToken().type == TOK_TITLE)
        {
            ast.setTitle(currentToken().value);
            std::cout << "Title: " << currentToken().value << std::endl;
            nextToken();
        }
        else if(currentToken().type == TOK_REM)
        {
            nextToken();
        }
        else
        {
            throwErrorAtCurrentLocation("Unexpected token: " + currentToken().value);
        }
    } while(true);
}

StatementNode* Parser::parseStatement()
{
    if(currentToken().type == TOK_INVALID)
        return nullptr;
    
    switch(currentToken().type)
    {
        case TOK_LET:       return parseLetStatement();
        case TOK_FOR:       return transformForLoop(parseForLoop());
        case TOK_GOTO:      return parseGoto();
        case TOK_LABEL:     return parseLabel();
        case TOK_WHILE:     return parseWhileLoop();
        case TOK_IF:        return parseIf();
        case TOK_PROMPT:    return parsePrompt();
        case TOK_PRINT:     return parsePrint();
        case TOK_INPUT:     return parseInput();
        
        default:
            break;
    }
    
    throwErrorAtCurrentLocation("Expected statement");
    return nullptr;
}

LValueNode* Parser::parseLValue()
{
    // TODO: add parsing of array accesses
    expectType(TOK_ID);
    
    std::string name = currentToken().value;
    VarDeclNode* var = ast.getVarByName(name);
    
    if(!var)
        throwErrorAtCurrentLocation("No such variable " + name);
    
    bool arrayAccess = peekNextToken().type == TOK_LSQUARE_BRACKET;
    
    if(IntDeclNode* intVar = dynamic_cast<IntDeclNode*>(var))
    {
        if(arrayAccess)
            throwErrorAtCurrentLocation("Variable " + name + " does not have list type");
        
        nextToken();
        return ast.addIntLValue(intVar);
    }
    
    if(OneDimensionalListDecl* listVar = dynamic_cast<OneDimensionalListDecl*>(var))
    {
        if(!arrayAccess)
            throwErrorAtCurrentLocation("Variable " + name + " has array type - expected '['");
        
        nextToken();
        expectType(TOK_LSQUARE_BRACKET);
        nextToken();
        
        ExpressionNode* index = parseExpression();
        
        expectType(TOK_RSQUARE_BRACKET);
        nextToken();
        
        return ast.addOneDimensionalListLValueNode(listVar, index);
    }
        
    throwErrorAtCurrentLocation("Variable " + name + " is not of integer type");
    return nullptr;
}

LetStatementNode* Parser::parseLetStatement()
{
    nextToken();
    LValueNode* leftSide = parseLValue();
    
    expectType(TOK_ASSIGN);
    nextToken();
    
    ExpressionNode* rightSide = parseExpression();
    
    return ast.addLetStatementNode(leftSide, rightSide);
}

CodeBlockNode* Parser::parseCodeBlock(TokenType endToken)
{
    CodeBlockNode* block = ast.addCodeBlockNode();
    
    while(currentToken().type != endToken && currentToken().type != TOK_INVALID)
        block->addStatement(parseStatement());
    
    // TODO add name of ending token
    if(currentToken().type != endToken)
    {
        prevToken();
        throwErrorAtCurrentLocation("Expected end of code block");
    }
    
    return block;
}

ForLoopNode* Parser::parseForLoop()
{
    nextToken();
    
    LValueNode* var = parseLValue();
    
    expectType(TOK_ASSIGN);
    nextToken();
    
    ExpressionNode* lower = parseExpression();
    
    expectType(TOK_TO);
    nextToken();
    
    ExpressionNode* upper = parseExpression();
    
    ExpressionNode* inc;
    
    if(currentToken().type == TOK_BY)
    {
        nextToken();
        inc = parseExpression();
    }
    else
    {
        inc = ast.newIntegerNode(1);
    }
    
    CodeBlockNode* body = parseCodeBlock(TOK_ENDFOR);
    nextToken();
    
    return ast.addForLoopNode(var, lower, upper, inc, body);
}

WhileLoopNode* Parser::parseWhileLoop()
{
    nextToken();
    
    expectType(TOK_LPAREN);
    nextToken();
    
    ExpressionNode* condition = parseCondition();
    
    expectType(TOK_RPAREN);
    nextToken();
    
    CodeBlockNode* body = parseCodeBlock(TOK_ENDWHILE);
    nextToken();
    
    return ast.addWhileLoopNode(condition, body);
}

GotoNode* Parser::parseGoto()
{
    nextToken();
    expectType(TOK_ID);
    
    std::string name = currentToken().value;
    
    GotoNode* newNode = ast.addGotoNode(name, currentToken().line, currentToken().col);
    nextToken();
    
    return newNode;
}

LabelNode* Parser::parseLabel()
{
    nextToken();
    expectType(TOK_ID);
    
    std::string name = currentToken().value;
    
    LabelNode* newNode = ast.addLabelNode(name, currentToken().line, currentToken().col);
    nextToken();
    
    return newNode;
}

ExpressionNode* Parser::parseCondition()
{
    ExpressionNode* leftSide = parseExpression();
    
    if(!Token::isComparisonOperator(currentToken().type))
        throwErrorAtCurrentLocation("Expected comparison operator");
    
    TokenType op = currentToken().type;
    nextToken();
    
    ExpressionNode* rightSide = parseExpression();
    
    return ast.newBinaryOpNode(leftSide, op, rightSide);
}

IfNode* Parser::parseIf()
{
    nextToken();
    
    expectType(TOK_LPAREN);
    nextToken();
    
    ExpressionNode* condition = parseCondition();
    
    expectType(TOK_RPAREN);
    nextToken();
    
    expectType(TOK_THEN);
    nextToken();
    
    StatementNode* body = parseStatement();
    
    return ast.addIfNode(condition, body);
}

PromptNode* Parser::parsePrompt()
{
    nextToken();
    expectType(TOK_STRING);
    
    std::string value = currentToken().value;
    nextToken();
    
    return ast.addPromptNode(value);
}

PrintNode* Parser::parsePrint()
{
    nextToken();
    
    ExpressionNode* expression = parseExpression();
    return ast.addPrintNode(expression);
}

InputNode* Parser::parseInput()
{
    nextToken();
    LValueNode* dest = parseLValue();
    return ast.addInputNode(dest);
}

CodeBlockNode* Parser::transformForLoop(ForLoopNode* node)
{
    CodeBlockNode* block = ast.addCodeBlockNode();
    block->disableCurlyBraces();
    
    block->addStatement(ast.addLetStatementNode(node->var, node->lowerBound));
    
    LabelNode* loopLabel = generateTempLabel();
    
    block->addStatement(loopLabel);
    
    node->body->disableCurlyBraces();
    block->addStatement(node->body);
    
    block->addStatement
    (
        ast.addLetStatementNode
        (
            node->var,
            ast.newBinaryOpNode
            (
                node->var->getFactorNode(),
                TOK_ADD,
                node->increment
            )
        )
    );
    
    block->addStatement
    (
        ast.addIfNode
        (
            ast.newBinaryOpNode
            (
                node->var->getFactorNode(),
                TOK_LE,
                node->upperBound
            ),
            ast.addGotoNode(loopLabel->name, -1, -1)
        )
    );
    
    return block;
}

CodeBlockNode* Parser::parseString(std::string str)
{
    Lexer lexer(str);
    auto newTokens = lexer.lexTokens();
    
    int saveCurrentTokenId = currentTokenId;
    auto saveTokens = tokens;
    
    CodeBlockNode* newBlock = parseCodeBlock(TOK_END);
    
    currentTokenId = saveCurrentTokenId;
    tokens = saveTokens;
    
    return newBlock;
}


