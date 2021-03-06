#pragma once

#include "Ast.hpp"
#include "SsaBuilder.hpp"
#include "PhiNodeBuilder.hpp"
#include "ExpressionFolder.hpp"
#include "DeadCodeEliminator.hpp"
#include "CopyPropagator.hpp"
#include "RedundantVariableRemover.hpp"

class Optimizer
{
public:
    Optimizer(CodeBlockNode* programBody_, Ast& ast_)
        : programBody(programBody_),
        ast(ast_),
        expressionFolder(programBody, ast),
        eliminator(programBody),
        copyPropagator(programBody, ast),
        varRemover(programBody)
        { }
        
    void optimize()
    {
        SsaBuilder ssaBuilder(programBody, ast);
        ssaBuilder.buildSsa();
        
        PhiNodeBuilder phiNodeBuilder(programBody, ast);
        phiNodeBuilder.buildPhiNodes();
        
        int iterationCount = 1;
        while(optimizeIteration())
            ++iterationCount;
        
        ast.eliminateUnusedVars();
        
        printf("============Optimizer stats============\n");
        printf("Total optimization passes: %d\n", iterationCount);
        expressionFolder.printStats();
        eliminator.printStats();
        copyPropagator.printStats();
        varRemover.printStats();
        printf("=======================================\n");
    }
    
private:
    bool optimizeIteration()
    {
        bool success = false;
        
        success |= expressionFolder.foldExpressions();
        success |= eliminator.eliminateDeadCode();
        success |= copyPropagator.propagateCopies();
        success |= varRemover.removeRedundantVariables();
        
        return success;
    }
    
    
    CodeBlockNode* programBody;
    Ast& ast;
    ExpressionFolder expressionFolder;
    DeadCodeEliminator eliminator;
    CopyPropagator copyPropagator;
    RedundantVariableRemover varRemover;
};
