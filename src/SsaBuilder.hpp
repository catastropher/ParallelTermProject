#pragma once

#include <queue>

#include "Ast.hpp"

class SsaBuilder
{
public:
    SsaBuilder(CodeBlockNode* programBody_, Ast& ast_) : programBody(programBody_), ast(ast_)
    {
    }
    
    void buildSsa()
    {
        for(StatementNode* s : programBody->statements)
        {
            auto block = dynamic_cast<BasicBlockNode*>(s);
            if(!block)
                throw CompileError("Ssa block contains object other than basic block", "SSA building", -1, -1);
            
            workQueue.push(block);
        }
        
        while(!workQueue.empty())
        {
            auto block = workQueue.front();
            workQueue.pop();
            processBasicBlock(block);
        }
    }
    
private:
    void processBasicBlock(BasicBlockNode* basicBlock)
    {
        VarDefSet oldOutSet = basicBlock->varDefOut;
        basicBlock->recalculateInSet();
        
        basicBlock->varDefOut = basicBlock->varDefIn;
        
        for(StatementNode* s : basicBlock->statements)
        {
            if(LetStatementNode* node = dynamic_cast<LetStatementNode*>(s))
            {
                if(transformLetStatementToSsa(node))
                {
                    basicBlock->varDefOut.replaceDefinition(dynamic_cast<SsaIntLValueNode*>(node->leftSide));
                }
            }
        }
        
        if(basicBlock->varDefOut != oldOutSet)
        {
            for(auto successor : basicBlock->successors)
                workQueue.push(successor);
        }
    }
    
    bool transformLetStatementToSsa(LetStatementNode* node)
    {
        bool alreadyTransformed = dynamic_cast<SsaIntLValueNode*>(node->leftSide);
        if(alreadyTransformed)
            return true;
        
        IntLValueNode* intNode = dynamic_cast<IntLValueNode*>(node->leftSide);
        if(!intNode)
            return false;
        
        node->leftSide = ast.addSsaIntLValueNode(intNode);
        return true;
    }
    
    CodeBlockNode* programBody;
    Ast& ast;
    std::queue<BasicBlockNode*> workQueue;
};
