#include "header.h"
#include "symbolTable.h"

void free_reg(char *str);
void get_f_reg(char *ans);
void get_reg(char *ans);
void genPrologue(char *funcIDName);
void genEpilogue(char *funcIDName);
void genLocalVars(AST_NODE *declList);
void genStmtList(AST_NODE *stmtList);
void genBlock(AST_NODE *blockNode);
void genFunctionDecl(AST_NODE *funcDeclNode);
void genExprRelated(char *reg, AST_NODE *exprRelatedNode);
void genGlobalVars(AST_NODE *globalVarDeclListNode);
void genProgram(AST_NODE *programNode);
void codeGeneration(AST_NODE *programNode);
void genFunctionCall(AST_NODE *callNode);
void genWrite(AST_NODE *paramListNode);
void genStmt(AST_NODE *stmtNode);