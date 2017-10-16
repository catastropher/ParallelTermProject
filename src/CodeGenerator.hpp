#pragma once

#include <stack>

#include "AstVisitor.hpp"

struct CodeGenerator : AstVisitor
{
    CodeGenerator() : currentIndent(0)
    {
        
    }
    
    void visit(IntegerNode* node)
    {
        push(std::to_string(node->value));
    }
    
    void visit(IntVarFactor* node)
    {
        push(node->var->name);
    }
    
    void visit(OneDimensionalListFactor* node)
    {
        node->index->accept(*this);
        
        std::string index = pop();
        push(node->var->name + "[" + index + "]");
    }
    
    void visit(IntLValueNode* node)
    {
        push(node->var->name);
    }
    
    void visit(OneDimensionalListLValueNode* node)
    {
        node->index->accept(*this);
        
        std::string index = pop();
        push(node->var->name + "[" + index + "]");
    }
    
    void visit(ExpressionNode* node)
    {
        throw "Bad expression type";
    }
    
    void visit(BinaryOpNode* node)
    {
        node->left->accept(*this);
        node->right->accept(*this);
        
        std::string op = Token::getTokenName(node->op);
        
        std::string right = pop();
        std::string left = pop();
        
        push("(" + left + " " + op + " " + right + ")");
    }
    
    void visit(LetStatementNode* node)
    {
        node->rightSide->accept(*this);
        node->leftSide->accept(*this);
        
        std::string left = pop();
        std::string right = pop();
        
        addLine(left + " = " + right + ";");
    }
    
    void visit(CodeBlockNode* node)
    {
        addLine("{");
        ++currentIndent;
        
        for(StatementNode* s : node->statements)
            s->accept(*this);
        
        --currentIndent;
        addLine("}");
        addLine("");
    }
    
    void visit(IfNode* node)
    {
        node->condition->accept(*this);
        
        std::string condition = pop();
        
        addLine("if(" + condition + ")");
        
        ++currentIndent;
        node->body->accept(*this);
        --currentIndent;
        
        addLine("");
    }
    
    void visit(WhileLoopNode* node)
    {
        node->condition->accept(*this);
        
        std::string condition = pop();
        
        addLine("while(" + condition + ")");
        
        node->body->accept(*this);
    }
    
    std::string pop()
    {
        std::string top = expStack.top();
        expStack.pop();
        return top;
    }
    
    void push(std::string str)
    {
        expStack.push(str);
    }
    
    void addLine(std::string line, bool useIndent = true)
    {
        std::string indent;
        
        if(useIndent)
        {
            for(int i = 0; i < currentIndent; ++i)
                indent += "    ";
        }
        
        output.push_back(indent + line);
    }

    std::vector<std::string> output;
    int currentIndent;
    
    std::stack<std::string> expStack;
};

