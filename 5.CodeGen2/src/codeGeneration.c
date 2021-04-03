#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "header.h"
#include "symbolTable.h"
#include "codeGeneration.h"

#define ELEM_SIZE 4

/*
 [msg] int to float conversion
    -- fcvt.s.w ft0, t0

 [msg] float to int conversion (檢查 condition 會用到)
    -- fcvt.w.s t0, ft0, rtz
*/

FILE *output;
int int_reg_t[7] = {0}, int_reg_s[12] = {0};
int float_reg[8] = {0};

int g_name_cnt = 0;
int g_ar_offset = 0;

union Float2UInt{
    float fval;
    unsigned int uval;
};

unsigned int convert_float_to_uint(float f) {
    union Float2UInt fu;
    fu.fval = f;
    return fu.uval;
}

void free_reg(char *str){
    char typ = str[0];
    int num;
    if (typ == 'f') {
        num = atoi(str + 2);
    } else {
        num = atoi(str + 1);
    }
    printf("[freed reg] %c%d.\n", typ, num);

    switch (typ){
    case 's':
        int_reg_s[num] = 0;
        break;
    case 't':
        int_reg_t[num] = 0;
        break;
    case 'f':
        float_reg[num] = 0;
    default:
        break;
    }
}

void regain_reg(char *str){
    char typ = str[0];
    int num;
    if (typ == 'f') {
        num = atoi(str + 2);

    } else {
        num = atoi(str + 1);
    }

    switch (typ){
    case 's':
        int_reg_s[num] = 1;
        break;
    case 't':
        int_reg_t[num] = 1;
        break;
    case 'f':
        float_reg[num] = 1;
    default:
        break;
    }
    printf("[regained reg] %c%d.\n", typ, num);
}

void get_f_reg(char *ans){
    int i = 0;
    while(i < 8){
        if(float_reg[i] == 0){
            float_reg[i] = 1;
            sprintf(ans, "ft%d\0", i);
            return;
        }
        i++;
    }
}

void get_reg(char *ans){
    int i = 0;
    while(i < 7){
        if(int_reg_t[i] == 0){
            int_reg_t[i] = 1;
            sprintf(ans, "t%d\0", i);
            return;
        }
        i++;
    }
    i = 0;
    while(i < 12){
        if(int_reg_s[i] == 0){
            int_reg_s[i] = 1;
            sprintf(ans, "s%d\0", i);
            return;
        }
        i++;
    }
}

void genCallerSave (CALLER_SAVE_ACTION action) {
    char save_or_load = (action == CALLER_RESTORE)? 'l' : 's';
    // if (save_or_load == 's') {
    //     fprintf(output, "add sp, sp, -100\n");
    // }
    // else{
    //     fprintf(output, "ld fp, 8(sp)\n");
    // }
    int save_offset = 8;
    for (int reg_a = 1; reg_a < 8; reg_a++) {
        save_offset -= 8;
        fprintf(output, "%cd a%d, %d(sp)\n", save_or_load, reg_a, save_offset);
    }
    for (int reg_fa = 1; reg_fa < 8; reg_fa++) {
        save_offset -= 4;
        fprintf(output, "f%cw fa%d, %d(sp)\n", save_or_load, reg_fa, save_offset);
    }
    // if (save_or_load == 's') {
    //     fprintf(output, "sd fp, 8(sp)\n");
    // }
    // else {
    //     fprintf(output, "add sp, sp, 100\n");
    // }
}

void genPrologue(char *funcIDName) {
    fprintf(output, ".text\n");
    fprintf(output, "_start_%s:\n", funcIDName);
    fprintf(output, "add sp, sp, -84\n");
    fprintf(output, "sd ra, 0(sp)\n");
    fprintf(output, "sd fp, -8(sp)\n");
    fprintf(output, "add fp, sp, -8\n");
    fprintf(output, "add sp, sp, -16\n");
    fprintf(output, "la ra, _frameSize_%s\n", funcIDName);
    fprintf(output, "lw ra, 0(ra)\n");
    fprintf(output, "sub sp, sp, ra\n");

    g_ar_offset = 8;
    for (int reg_t = 0; reg_t < 7; reg_t++) {
        fprintf(output, "sd t%d, %d(sp)\n", reg_t, g_ar_offset);
        g_ar_offset += 8;
    }
    for (int reg_s = 2; reg_s < 12; reg_s++) {
        fprintf(output, "sd s%d, %d(sp)\n", reg_s, g_ar_offset);
        g_ar_offset += 8;
    }
    fprintf(output, "sd fp, %d(sp)\n", g_ar_offset);
    g_ar_offset += 8;
    for (int reg_ft = 0; reg_ft < 8; reg_ft++) {
        fprintf(output, "fsw ft%d, %d(sp)\n", reg_ft, g_ar_offset);
        g_ar_offset += 4;
    }

    return;
}

void genEpilogue(char *funcIDName) {
    fprintf(output, "_end_%s:\n", funcIDName);

    int epilogue_offset = 8;
    for (int reg_t = 0; reg_t < 7; reg_t++) {
        fprintf(output, "ld t%d, %d(sp)\n", reg_t, epilogue_offset);
        epilogue_offset += 8;
    }
    for (int reg_s = 2; reg_s < 12; reg_s++) {
        fprintf(output, "ld s%d, %d(sp)\n", reg_s, epilogue_offset);
        epilogue_offset += 8;
    }
    fprintf(output, "ld fp, %d(sp)\n", epilogue_offset);
    epilogue_offset += 8;
    for (int reg_ft = 0; reg_ft < 8; reg_ft++) {
        fprintf(output, "flw ft%d, %d(sp)\n", reg_ft, epilogue_offset);
        epilogue_offset += 4;
    }

    fprintf(output, "ld ra, 8(fp)\n");
    fprintf(output, "mv sp, fp\n");
    fprintf(output, "add sp, sp, 8\n");
    fprintf(output, "ld fp, 0(fp)\n");
    fprintf(output, "add sp, sp, 84\n");
    fprintf(output, "jr ra\n");

    fprintf(output, ".data\n");
    fprintf(output, "_frameSize_%s: .word %d\n", funcIDName, g_ar_offset);

    return;
}

void genOffsetSafeStoreLoad (char *instr, char *reg, int offset) {
    if ( offset >= -2048 && offset < 2048 ) {
        fprintf(output, "%s %s, %d(sp)\n", instr, reg, offset);
    }
    else {
        char reg_aux[8];
        get_reg(reg_aux);
        fprintf(output, "li %s, %d\n", reg_aux, offset);
        fprintf(output, "add %s, %s, sp\n", reg_aux, reg_aux);
        fprintf(output, "%s %s, 0(%s)\n", instr, reg, reg_aux);
        free_reg(reg_aux);
    }

    return;
}

void genLocalVars(AST_NODE *declList){
    AST_NODE *cur_decl = declList->child;

    while(cur_decl != NULL){
        if ( cur_decl->semantic_value.declSemanticValue.kind == VARIABLE_DECL ) {
            AST_NODE *type_node = cur_decl->child;
            AST_NODE *cur_id_node = type_node->rightSibling;
            printf("[saw local type] %s.\n", type_node->semantic_value.identifierSemanticValue.identifierName);

            while ( cur_id_node != NULL ) {
                TypeDescriptor *id_type = cur_id_node->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
                SymbolTableEntry *id_entry = cur_id_node->semantic_value.identifierSemanticValue.symbolTableEntry;
                printf("  -- [saw local var] %s (kind: %d).\n", cur_id_node->semantic_value.identifierSemanticValue.identifierName, id_type->kind);

                if ( id_type->kind == ARRAY_TYPE_DESCRIPTOR ) {
                    printf("  -- is array.\n");

                    int array_size = 1;
                    for (int d = 0; d < id_type->properties.arrayProperties.dimension; d++) {
                        array_size *= id_type->properties.arrayProperties.sizeInEachDimension[d];
                    }
                    printf("  -- memory size: %d.\n", array_size * ELEM_SIZE);

                    array_size *= ELEM_SIZE;
                    id_entry->offset = g_ar_offset;
                    g_ar_offset += array_size;

                } else {
                    printf("  -- is scalar (dataType: %d).\n", id_type->properties.dataType);
                    id_entry->offset = g_ar_offset;
                    g_ar_offset += ELEM_SIZE;
                    
                    if(cur_id_node->semantic_value.identifierSemanticValue.kind == WITH_INIT_ID) {
                        // [msg] remember to consider <expr> RHS
                        // [msg] semantic analysis didn't handle the <expr> case well, so I give up
                        char reg[8];
                        if(id_type->properties.dataType == INT_TYPE){
                            get_reg(reg);
                            int init_value = cur_id_node->child->semantic_value.const1->const_u.intval;
                            fprintf(output, "li %s, %d\n", reg, init_value);
                            // genExprRelated(reg, cur_id_node->child);
                            // fprintf(output, "sw %s, %d(sp)\n", reg, id_entry->offset);
                            printf("[DEBUG] var:%s   offset = %d\n", id_entry->name, id_entry->offset);
                            genOffsetSafeStoreLoad("sw", reg, id_entry->offset);
                        } else {
                            get_f_reg(reg);
                            float init_value = cur_id_node->child->semantic_value.const1->const_u.fval;
                            fprintf(output, "li %s, 0x%x\n", reg, convert_float_to_uint(init_value));
                            // genExprRelated(reg, cur_id_node->child);
                            // fprintf(output, "fsw %s, %d(sp)\n", reg, id_entry->offset);
                            printf("[DEBUG] var:%s   offset = %d\n", id_entry->name, id_entry->offset);
                            genOffsetSafeStoreLoad("fsw", reg, id_entry->offset);
                        }
                        free_reg(reg);                      
                    }
                }
                cur_id_node = cur_id_node->rightSibling;
            }
        }
        else if(cur_decl->semantic_value.declSemanticValue.kind == TYPE_DECL){
            //nothing to do
        }
        cur_decl = cur_decl->rightSibling;
    }
    
    return;
}

void genIntCompareOp(char *regL, char *regR, char *cmpType) {
    fprintf(output, "b%s %s, %s, _cmpElse_%d\n", cmpType, regL, regR, g_name_cnt);
    fprintf(output, "li %s, 0\n", regL);
    fprintf(output, "j _cmpEnd_%d\n", g_name_cnt);
    fprintf(output, "_cmpElse_%d:\n", g_name_cnt);
    fprintf(output, "li %s, 1\n", regL);
    fprintf(output, "_cmpEnd_%d:\n", g_name_cnt++);
    return;
}

void genFloatCompareOp(char *regL, char *regR, char *cmpType) {
    char reg_aux[8];
    get_reg(reg_aux);

    if ( !strcmp(cmpType, "eq") ) {
        fprintf(output, "feq.s %s, %s, %s\n", reg_aux, regL, regR);
    }
    else if (!strcmp(cmpType, "ne")) {
        fprintf(output, "feq.s %s, %s, %s\n", reg_aux, regL, regR);
        fprintf(output, "subw %s, zero, %s\n", reg_aux, reg_aux);
        fprintf(output, "addiw %s, %s, 1\n", reg_aux, reg_aux);
    }
    else if (!strcmp(cmpType, "le") ) {
        fprintf(output, "fle.s %s, %s, %s\n", reg_aux, regL, regR);
    }
    else if (!strcmp(cmpType, "lt") ) {
        fprintf(output, "flt.s %s, %s, %s\n", reg_aux, regL, regR);
    }
    else if (!strcmp(cmpType, "ge") ) {
        fprintf(output, "fle.s %s, %s, %s\n", reg_aux, regR, regL);
    }
    else if (!strcmp(cmpType, "gt") ) {
        fprintf(output, "flt.s %s, %s, %s\n", reg_aux, regR, regL);
    }

    fprintf(output, "fcvt.s.w %s, %s\n", regL, reg_aux);
    free_reg(reg_aux);
    return;
}

void genIntLogicalOp(char *regL, char *regR, char *logicType) {
    if ( !strcmp(logicType, "and") ) {
        fprintf(output, "beqz %s, _logicElse_%d\n", regL, g_name_cnt);
        fprintf(output, "beqz %s, _logicElse_%d\n", regR, g_name_cnt);
        fprintf(output, "li %s, 1\n", regL);
        fprintf(output, "j _logicEnd_%d\n", g_name_cnt);
        fprintf(output, "_logicElse_%d:\n", g_name_cnt);
        fprintf(output, "li %s, 0\n", regL);
        fprintf(output, "_logicEnd_%d:\n", g_name_cnt++);
    }
    else if ( !strcmp(logicType, "or") ) {
        fprintf(output, "bnez %s, _logicElse_%d\n", regL, g_name_cnt);
        fprintf(output, "bnez %s, _logicElse_%d\n", regR, g_name_cnt);
        fprintf(output, "li %s, 0\n", regL);
        fprintf(output, "j _logicEnd_%d\n", g_name_cnt);
        fprintf(output, "_logicElse_%d:\n", g_name_cnt);
        fprintf(output, "li %s, 1\n", regL);
        fprintf(output, "_logicEnd_%d:\n", g_name_cnt++);
    } 
    else if ( !strcmp(logicType, "not") ) {
        fprintf(output, "bnez %s, _logicElse_%d\n", regL, g_name_cnt);
        fprintf(output, "li %s, 1\n", regL);
        fprintf(output, "j _logicEnd_%d\n", g_name_cnt);
        fprintf(output, "_logicElse_%d:\n", g_name_cnt);
        fprintf(output, "li %s, 0\n", regL);
        fprintf(output, "_logicEnd_%d:\n", g_name_cnt++);
    }
    return;
}

void genFloatLogicalOp(char *regL, char *regR, char *logicType) {
    // [msg] not covered in the test cases
    // [msg] so I don't want to spend time on this.
    return;
}

void genExpr(char *reg, AST_NODE *exprNode) {
    if ( exprNode->semantic_value.exprSemanticValue.isConstEval ) {
        if ( exprNode->dataType == INT_TYPE ) {
            printf("[got const expr] int: %d\n", exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue);
            fprintf(output, "li %s, %d\n", reg,
                exprNode->semantic_value.exprSemanticValue.constEvalValue.iValue 
            );
        }
        else {
            printf("[got const expr] float: %lf\n", exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue);
            char reg_aux[8];
            fprintf(output, ".data\n");
            fprintf(output, "_CONSTANT_%d: .word 0x%x\n", g_name_cnt, 
                convert_float_to_uint(exprNode->semantic_value.exprSemanticValue.constEvalValue.fValue)
            );
            fprintf(output, ".align 3\n");
            fprintf(output, ".text\n");
            get_reg(reg_aux);
            fprintf(output, "la %s, _CONSTANT_%d\n", reg_aux, g_name_cnt++);
            fprintf(output, "flw %s, 0(%s)\n", reg, reg_aux);
            free_reg(reg_aux);
        }
        return;
    }

    if ( exprNode->semantic_value.exprSemanticValue.kind == BINARY_OPERATION ) {
        printf("[got binary arith op] %d\n", exprNode->semantic_value.exprSemanticValue.op.binaryOp);
        printf("[data types] L: %d, R: %d (reg for L: %s)\n", 
            exprNode->child->dataType, exprNode->child->rightSibling->dataType, reg
        );
        AST_NODE *left_node = exprNode->child;
        char reg_left[8];
        if (left_node->dataType == exprNode->dataType) {
            strcpy(reg_left, reg);
            genExprRelated(reg_left, left_node);
        }
        else {
            if (left_node->dataType == INT_TYPE) {
                get_reg(reg_left);
            } else {
                get_f_reg(reg_left);
            }
            free_reg(reg);
            genExprRelated(reg_left, left_node);
        }

        AST_NODE *right_node = left_node->rightSibling;
        char reg_right[8];
        if (right_node->dataType == INT_TYPE) {
            get_reg(reg_right);
        } else {
            get_f_reg(reg_right);
        }
        genExprRelated(reg_right, right_node);

        if ( strcmp(reg, reg_left) != 0 ) {
            regain_reg(reg);
        }

        if ( left_node->dataType == INT_TYPE && right_node->dataType == INT_TYPE ) {
            exprNode->dataType = INT_TYPE;
            // printf("[in binary op gen] op: %d\n", exprNode->semantic_value.exprSemanticValue.op.binaryOp);
            switch (exprNode->semantic_value.exprSemanticValue.op.binaryOp) {
                case BINARY_OP_ADD:
                    fprintf(output, "addw %s, %s, %s\n", reg, reg_left, reg_right);
                    break;
                case BINARY_OP_SUB:
                    fprintf(output, "subw %s, %s, %s\n", reg, reg_left, reg_right);
                    break;
                case BINARY_OP_MUL:
                    fprintf(output, "mulw %s, %s, %s\n", reg, reg_left, reg_right);
                    break;
                case BINARY_OP_DIV:
                    fprintf(output, "divw %s, %s, %s\n", reg, reg_left, reg_right);
                    break;
                case BINARY_OP_EQ:
                    genIntCompareOp(reg_left, reg_right, "eq");
                    break;
                case BINARY_OP_GE:
                    genIntCompareOp(reg_left, reg_right, "ge");
                    break;
                case BINARY_OP_LE:
                    genIntCompareOp(reg_left, reg_right, "le");
                    break;
                case BINARY_OP_NE:
                    genIntCompareOp(reg_left, reg_right, "ne");
                    break;
                case BINARY_OP_GT:
                    genIntCompareOp(reg_left, reg_right, "gt");
                    break;
                case BINARY_OP_LT:
                    genIntCompareOp(reg_left, reg_right, "lt");
                    break;
                case BINARY_OP_AND:
                    genIntLogicalOp(reg_left, reg_right, "and");
                    break;
                case BINARY_OP_OR:
                    genIntLogicalOp(reg_left, reg_right, "or");
                    break;
            }
        } 
        else { // [inf] either one is float
            exprNode->dataType = FLOAT_TYPE;
            char reg_orig[8];
            if (left_node->dataType == INT_TYPE) {
                strcpy(reg_orig, reg_left);
                get_f_reg(reg_left);
                fprintf(output, "fcvt.s.w %s, %s\n", reg_left, reg_orig);
                free_reg(reg_orig);
            }
            if (right_node->dataType == INT_TYPE) {
                strcpy(reg_orig, reg_right);
                get_f_reg(reg_right);
                fprintf(output, "fcvt.s.w %s, %s\n", reg_right, reg_orig);
                free_reg(reg_orig);
            }

            switch (exprNode->semantic_value.exprSemanticValue.op.binaryOp) {
                case BINARY_OP_ADD:
                    fprintf(output, "fadd.s %s, %s, %s\n", reg, reg_left, reg_right);
                    break;
                case BINARY_OP_SUB:
                    fprintf(output, "fsub.s %s, %s, %s\n", reg, reg_left, reg_right);
                    break;
                case BINARY_OP_MUL:
                    fprintf(output, "fmul.s %s, %s, %s\n", reg, reg_left, reg_right);
                    break;
                case BINARY_OP_DIV:
                    fprintf(output, "fdiv.s %s, %s, %s\n", reg, reg_left, reg_right);
                    break;
                case BINARY_OP_EQ:
                    genFloatCompareOp(reg_left, reg_right, "eq");
                    break;
                case BINARY_OP_GE:
                    genFloatCompareOp(reg_left, reg_right, "ge");
                    break;
                case BINARY_OP_LE:
                    genFloatCompareOp(reg_left, reg_right, "le");
                    break;
                case BINARY_OP_NE:
                    genFloatCompareOp(reg_left, reg_right, "ne");
                    break;
                case BINARY_OP_GT:
                    genFloatCompareOp(reg_left, reg_right, "gt");
                    break;
                case BINARY_OP_LT:
                    genFloatCompareOp(reg_left, reg_right, "lt");
                    break;
                case BINARY_OP_AND:
                    genFloatLogicalOp(reg_left, reg_right, "and");
                    break;
                case BINARY_OP_OR:
                    genFloatLogicalOp(reg_left, reg_right, "or");
                    break;
            }
        }
        if ( strcmp(reg_left, reg) != 0 ) {
            free_reg(reg_left);
        }
        free_reg(reg_right);
    }
    else { // [inf] unary operation
        AST_NODE *child_node = exprNode->child;
        printf("[got unary arith op] %d\n", exprNode->semantic_value.exprSemanticValue.op.unaryOp);
        printf("[data type] C: %d\n", exprNode->child->dataType);
        genExprRelated(reg, child_node);
        if ( child_node->dataType == INT_TYPE ) {
            exprNode->dataType = INT_TYPE;
            switch (exprNode->semantic_value.exprSemanticValue.op.unaryOp) {
                case UNARY_OP_NEGATIVE:
                    fprintf(output, "subw %s, zero, %s\n", reg, reg);
                    break;
                case UNARY_OP_LOGICAL_NEGATION:
                    genIntLogicalOp(reg, NULL, "not");
                    break;
            }
        }
        else { // [inf] float case
            printf("[captured float unary]\n");
            exprNode->dataType = FLOAT_TYPE;
            switch (exprNode->semantic_value.exprSemanticValue.op.unaryOp) {
                case UNARY_OP_NEGATIVE:
                    fprintf(output, "fneg.s %s, %s\n", reg, reg);
                    break;
                case UNARY_OP_LOGICAL_NEGATION:
                    genFloatLogicalOp(reg, NULL, "not");
                    break;
            }
        }
    }
    return;
}

void genArrayDimList(AST_NODE *arrayIDNode, char *reg_offset) {
    TypeDescriptor *array_type =\
        arrayIDNode->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
    AST_NODE *array_dim_list_node = arrayIDNode->child;

    printf("[calc array offset] %s (dim: %d)\n", 
        arrayIDNode->semantic_value.identifierSemanticValue.identifierName,
        array_type->properties.arrayProperties.dimension
    );

    if (array_type->properties.arrayProperties.dimension == 1) {
        genExprRelated(reg_offset, array_dim_list_node);
    }
    else {
        char reg_dim[8];
        get_reg(reg_dim);
        fprintf(output, "mv %s, x0\n", reg_offset);
        for (int d = 0; d < array_type->properties.arrayProperties.dimension; d++) {
            printf("  -- got dim %d\n", d);

            genExprRelated(reg_dim, array_dim_list_node);
            fprintf(output, "add %s, %s, %s\n", reg_offset, reg_offset, reg_dim);
            if (d != array_type->properties.arrayProperties.dimension - 1) {
                fprintf(output, "li %s, %d\n", 
                    reg_dim, array_type->properties.arrayProperties.sizeInEachDimension[d+1]
                );
                fprintf(output, "mul %s, %s, %s\n", reg_offset, reg_offset, reg_dim);
            }
            array_dim_list_node = array_dim_list_node->rightSibling;
        }
        free_reg(reg_dim);
    }
                
    fprintf(output, "slli %s, %s, 2\n", reg_offset, reg_offset);
    return;
}

void genExprRelated(char *reg, AST_NODE *exprRelatedNode){
    switch(exprRelatedNode->nodeType){
        SymbolTableEntry *expr_id_entry;
        char reg_aux[8];
        case STMT_NODE: // [inf] function call
            printf("[got func call] %s (type: %d)\n", 
                exprRelatedNode->child->semantic_value.identifierSemanticValue.identifierName,
                exprRelatedNode->dataType
            );
            genFunctionCall(exprRelatedNode);
            if (exprRelatedNode->dataType == INT_TYPE) {
                fprintf(output, "mv %s, a0\n", reg);
            }
            else if (exprRelatedNode->dataType == FLOAT_TYPE) {
                fprintf(output, "fmv.s %s, fa0\n", reg);
            } 
            else {
                // [inf] void function
            }
            break;
        case EXPR_NODE:
            genExpr(reg, exprRelatedNode);
            break;
        case IDENTIFIER_NODE:
            // printf("[got identifier in expr] entry: %x, name: %s\n", 
            //     exprRelatedNode->semantic_value.identifierSemanticValue.symbolTableEntry,
            //     exprRelatedNode->semantic_value.identifierSemanticValue.identifierName
            // );
            expr_id_entry = exprRelatedNode->semantic_value.identifierSemanticValue.symbolTableEntry;
            if ( expr_id_entry->offset < 0 ) { // [msg] for passed function parameter
                printf("[got func param] %s: %d (type: %d)\n", 
                    exprRelatedNode->semantic_value.identifierSemanticValue.identifierName,
                    expr_id_entry->offset,
                    expr_id_entry->attribute->attr.typeDescriptor->properties.dataType
                );
                int arg_reg = -1 * expr_id_entry->offset;
                exprRelatedNode->dataType =\
                    expr_id_entry->attribute->attr.typeDescriptor->properties.dataType;
                if ( exprRelatedNode->dataType == INT_TYPE ) {
                    fprintf(output, "mv %s, a%d\n", reg, arg_reg);
                }
                else if ( exprRelatedNode->dataType == FLOAT_TYPE ) {
                    fprintf(output, "fmv.s %s, fa%d\n", reg, arg_reg);
                }
            }
            else if (exprRelatedNode->semantic_value.identifierSemanticValue.kind == NORMAL_ID) {
                // printf("[nesting level] %d\n", expr_id_entry->nestingLevel);
                exprRelatedNode->dataType =\
                    expr_id_entry->attribute->attr.typeDescriptor->properties.dataType;
                if (expr_id_entry->nestingLevel == 0) { // [inf] global var
                    get_reg(reg_aux);
                    fprintf(output, "la %s, _g_%s\n", reg_aux, 
                        exprRelatedNode->semantic_value.identifierSemanticValue.identifierName
                    );
                    if (exprRelatedNode->dataType == INT_TYPE) {
                        fprintf(output, "lw %s, 0(%s)\n", reg, reg_aux);
                    } 
                    else {
                        fprintf(output, "flw %s, 0(%s)\n", reg, reg_aux);
                    }
                    free_reg(reg_aux);
                } 
                else { // [inf] local var
                    printf("[fetch local var] %s (offset: %d).\n", 
                        exprRelatedNode->semantic_value.identifierSemanticValue.identifierName,
                        expr_id_entry->offset
                    );
                    if (exprRelatedNode->dataType == INT_TYPE) {
                        // fprintf(output, "lw %s, %d(sp)\n", reg, expr_id_entry->offset);
                        printf("[DEBUG] var:%s   offset = %d\n", expr_id_entry->name, expr_id_entry->offset);
                        genOffsetSafeStoreLoad("lw", reg, expr_id_entry->offset);
                    }
                    else {
                        // fprintf(output, "flw %s, %d(sp)\n", reg, expr_id_entry->offset);
                        printf("[DEBUG] var:%s   offset = %d\n", expr_id_entry->name, expr_id_entry->offset);
                        genOffsetSafeStoreLoad("flw", reg, expr_id_entry->offset);
                    }
                }
            }
            else {  // [msg] array case
                // [inf] calculate offset
                exprRelatedNode->dataType =\
                    expr_id_entry->attribute->attr.typeDescriptor->properties.arrayProperties.elementType;

                /* [inf] old offset calculation code */
                // AST_NODE *array_dim_list_node = exprRelatedNode->child;
                // char reg_offset[8];
                // get_reg(reg_offset);
                // genExprRelated(reg_offset, array_dim_list_node);
                // fprintf(output, "slli %s, %s, 2\n", reg_offset, reg_offset);

                char reg_offset[8];
                get_reg(reg_offset);
                genArrayDimList(exprRelatedNode, reg_offset);

                if (expr_id_entry->nestingLevel == 0) {
                    get_reg(reg_aux);
                    fprintf(output, "la %s, _g_%s\n", reg_aux, 
                        exprRelatedNode->semantic_value.identifierSemanticValue.identifierName
                    );
                    fprintf(output, "add %s, %s, %s\n", reg_aux, reg_aux, reg_offset);              
                }
                else {
                    get_reg(reg_aux);
                    fprintf(output, "li %s, %d\n", reg_aux, expr_id_entry->offset);
                    fprintf(output, "add %s, %s, %s\n", reg_aux, reg_aux, reg_offset);
                    fprintf(output, "add %s, sp, %s\n", reg_aux, reg_aux);               
                }

                if (exprRelatedNode->dataType == INT_TYPE) {
                    fprintf(output, "lw %s, 0(%s)\n", reg, reg_aux);
                } 
                else {
                    fprintf(output, "flw %s, 0(%s)\n", reg, reg_aux);
                }
                free_reg(reg_offset);
                free_reg(reg_aux);      
            }
            break;
        case CONST_VALUE_NODE:
            switch(exprRelatedNode->semantic_value.const1->const_type){
                int str_len;
                char str_cpy[1024];
                case INTEGERC:
                    // fprintf(output, ".data\n");
                    // fprintf(output, "_CONSTANT_%d: .word %d\n", g_name_cnt, exprRelatedNode->semantic_value.const1->const_u.intval);
                    // fprintf(output, ".align 3\n");
                    // fprintf(output, ".text\n");
                    // get_reg(reg_aux);
                    // fprintf(output, "la %s, _CONSTANT_%d\n", reg_aux, g_name_cnt++);
                    // fprintf(output, "lw %s, 0(%s)\n", reg, reg_aux);
                    // free_reg(reg_aux);
                    exprRelatedNode->dataType = INT_TYPE;
                    fprintf(output, "li %s, %d\n", reg, exprRelatedNode->semantic_value.const1->const_u.intval);
                    break;
                case FLOATC:
                    exprRelatedNode->dataType = FLOAT_TYPE;
                    fprintf(output, ".data\n");
                    fprintf(output, "_CONSTANT_%d: .word 0x%x\n", g_name_cnt, 
                            convert_float_to_uint(exprRelatedNode->semantic_value.const1->const_u.fval)
                    );
                    fprintf(output, ".align 3\n");
                    fprintf(output, ".text\n");
                    get_reg(reg_aux);
                    fprintf(output, "la %s, _CONSTANT_%d\n", reg_aux, g_name_cnt++);
                    fprintf(output, "flw %s, 0(%s)\n", reg, reg_aux);
                    free_reg(reg_aux);
                    break;
                case STRINGC:
                    str_len = strlen(exprRelatedNode->semantic_value.const1->const_u.sc);
                    strcpy(str_cpy, exprRelatedNode->semantic_value.const1->const_u.sc+1);
                    str_cpy[ str_len - 2 ] = '\0';
                    fprintf(output, ".data\n");
                    fprintf(output, "_CONSTANT_%d: .ascii \"%s\\000\"\n", g_name_cnt, str_cpy);
                    fprintf(output, ".align 3\n");
                    fprintf(output, ".text\n");
                    fprintf(output, "la %s, _CONSTANT_%d\n", reg, g_name_cnt++);
                    break;
            }
            break;
    }
}

void genWrite(AST_NODE *paramListNode){
    AST_NODE *param_node = paramListNode->child;
    char reg[8];
    switch (param_node->dataType){
        case INT_TYPE:
            /* code */
            get_reg(reg);
            genExprRelated(reg, param_node);
            fprintf(output, "mv a0, %s\n", reg);
            fprintf(output, "call _write_int\n");
            free_reg(reg);
            break;
        case FLOAT_TYPE:
            get_f_reg(reg);
            genExprRelated(reg, param_node);
            fprintf(output, "fmv.s fa0, %s\n", reg);
            fprintf(output, "call _write_float\n");
            free_reg(reg);
            break;
        case CONST_STRING_TYPE:
            get_reg(reg);
            genExprRelated(reg, param_node);
            fprintf(output, "mv a0, %s\n", reg);
            fprintf(output, "call _write_str\n");
            free_reg(reg);
            break;
        default:
            break;
    }
}

void genFunctionCall(AST_NODE *callNode){
    AST_NODE *id_node = callNode->child;
    genCallerSave(CALLER_SAVE);

    if ( !strcmp(id_node->semantic_value.identifierSemanticValue.identifierName, "write") ) {
        genWrite(id_node->rightSibling);
    }
    else if ( !strcmp(id_node->semantic_value.identifierSemanticValue.identifierName, "read") ) {
        fprintf(output, "call _read_int\n");
    }
    else if ( !strcmp(id_node->semantic_value.identifierSemanticValue.identifierName, "fread") ) {
        fprintf(output, "call _read_float\n");
    }
    else {
        int func_params_cnt = id_node->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.functionSignature->parametersCount;
        printf("[# func args] %s: %d\n", 
            id_node->semantic_value.identifierSemanticValue.identifierName, 
            func_params_cnt
        );
        if ( func_params_cnt == 0 ) {  
            fprintf(output, "call _start_%s\n", id_node->semantic_value.identifierSemanticValue.identifierName);
        }
        else {
            AST_NODE *param_node = id_node->rightSibling->child;
            Parameter *param = id_node->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.functionSignature->parameterList;
            for (int p = 0; p < func_params_cnt; p++) {
                char param_reg[8];
                if (param_node->dataType == INT_TYPE) {
                    get_reg(param_reg);
                } else {
                    get_f_reg(param_reg);
                }
                genExprRelated(param_reg, param_node);

                DATA_TYPE formal_param_type = param->type->properties.dataType;
                if (formal_param_type != param_node->dataType) {
                    if (formal_param_type == INT_TYPE) {
                        char reg_aux[8];
                        get_reg(reg_aux);
                        fprintf(output, "fcvt.w.s %s, %s, rtz\n", reg_aux, param_reg);
                        free_reg(param_reg);
                        strcpy(param_reg, reg_aux);
                    }
                    else if (formal_param_type == FLOAT_TYPE) {
                        char reg_aux[8];
                        get_f_reg(reg_aux);
                        fprintf(output, "fcvt.s.w %s, %s\n", reg_aux, param_reg);
                        free_reg(param_reg);
                        strcpy(param_reg, reg_aux);
                    }
                }

                if (formal_param_type == FLOAT_TYPE) {
                    fprintf(output, "fmv.s fa%d, %s\n", p + 1, param_reg);
                }
                else {
                    fprintf(output, "mv a%d, %s\n", p + 1, param_reg);
                }

                free_reg(param_reg);
                param = param->next;
                param_node = param_node->rightSibling;
            }

            fprintf(output, "call _start_%s\n", id_node->semantic_value.identifierSemanticValue.identifierName);
        }
    }
    genCallerSave(CALLER_RESTORE);
    return;
}

void genAssignment(AST_NODE *assignmentNode) {
    AST_NODE *assign_id_node = assignmentNode->child;
    AST_NODE *rhs_node = assign_id_node->rightSibling;

    char rhs_reg[8];
    if (rhs_node->dataType == INT_TYPE) {
        get_reg(rhs_reg);
    } else {
        get_f_reg(rhs_reg);
    }
    genExprRelated(rhs_reg, rhs_node);

    SymbolTableEntry *assign_id_entry = assign_id_node->semantic_value.identifierSemanticValue.symbolTableEntry;
    if ( assign_id_node->semantic_value.identifierSemanticValue.kind == NORMAL_ID ) {
        if (assign_id_entry->nestingLevel == 0) { // [inf] global var
            char reg_aux[8];
            get_reg(reg_aux);
            fprintf(output, "la %s, _g_%s\n", reg_aux, 
                assign_id_node->semantic_value.identifierSemanticValue.identifierName
            );
            if (assign_id_node->dataType == INT_TYPE) {
                if (rhs_node->dataType == FLOAT_TYPE) {
                    char reg_aux_2[8];
                    get_reg(reg_aux_2);
                    fprintf(output, "fcvt.w.s %s, %s, rtz\n", reg_aux_2, rhs_reg);
                    fprintf(output, "sw %s, 0(%s)\n", reg_aux_2, reg_aux);
                    free_reg(reg_aux_2);
                }
                else {
                    fprintf(output, "sw %s, 0(%s)\n", rhs_reg, reg_aux);
                }
            } 
            else {
                if (rhs_node->dataType == INT_TYPE) {
                    char reg_aux_2[8];
                    get_f_reg(reg_aux_2);
                    fprintf(output, "fcvt.s.w %s, %s\n", reg_aux_2, rhs_reg);
                    fprintf(output, "fsw %s, 0(%s)\n", reg_aux_2, reg_aux);
                    free_reg(reg_aux_2);
                }
                else {
                    fprintf(output, "fsw %s, 0(%s)\n", rhs_reg, reg_aux);
                }
            }
            free_reg(reg_aux);
        } 
        else { // [inf] local var
            printf("[assign local var] %s (offset: %d).\n", 
                assign_id_node->semantic_value.identifierSemanticValue.identifierName,
                assign_id_entry->offset
            );
            printf("[DEBUG] var:%s   offset = %d\n", assign_id_entry->name, assign_id_entry->offset);
            if (assign_id_node->dataType == INT_TYPE) {
                if (rhs_node->dataType == FLOAT_TYPE) {
                    char reg_aux[8];
                    get_reg(reg_aux);
                    fprintf(output, "fcvt.w.s %s, %s, rtz\n", reg_aux, rhs_reg);
                    // fprintf(output, "sw %s, %d(sp)\n", reg_aux, assign_id_entry->offset);
                    genOffsetSafeStoreLoad("sw", reg_aux, assign_id_entry->offset);
                    free_reg(reg_aux);
                }
                else {
                    // fprintf(output, "sw %s, %d(sp)\n", rhs_reg, assign_id_entry->offset);
                    genOffsetSafeStoreLoad("sw", rhs_reg, assign_id_entry->offset);
                }
            }
            else {
                if (rhs_node->dataType == INT_TYPE) {
                    char reg_aux[8];
                    get_f_reg(reg_aux);
                    fprintf(output, "fcvt.s.w %s, %s\n", reg_aux, rhs_reg);
                    // fprintf(output, "fsw %s, %d(sp)\n", reg_aux, assign_id_entry->offset);
                    genOffsetSafeStoreLoad("fsw", reg_aux, assign_id_entry->offset);
                    free_reg(reg_aux);
                }
                else {
                    // fprintf(output, "fsw %s, %d(sp)\n", rhs_reg, assign_id_entry->offset);
                    genOffsetSafeStoreLoad("fsw", rhs_reg, assign_id_entry->offset);
                }
            }
        }
    }
    else { // [msg] array case
        AST_NODE *array_dim_list_node = assign_id_node->child;
        char reg_offset[8], reg_aux[8];
        /* [inf] old offset calculation code */
        // genExprRelated(reg_offset, array_dim_list_node);
        // fprintf(output, "slli %s, %s, 2\n", reg_offset, reg_offset);

        get_reg(reg_offset);
        genArrayDimList(assign_id_node, reg_offset);

        if (assign_id_entry->nestingLevel == 0) {
            get_reg(reg_aux);
            fprintf(output, "la %s, _g_%s\n", reg_aux, 
                assign_id_node->semantic_value.identifierSemanticValue.identifierName
            );
            fprintf(output, "add %s, %s, %s\n", reg_aux, reg_aux, reg_offset);                    
        }
        else {
            get_reg(reg_aux);
            fprintf(output, "li %s, %d\n", reg_aux, assign_id_entry->offset);
            fprintf(output, "add %s, %s, %s\n", reg_aux, reg_aux, reg_offset);
            fprintf(output, "add %s, sp, %s\n", reg_aux, reg_aux);                   
        }
        
        if (assign_id_node->dataType == INT_TYPE) {
            if (rhs_node->dataType == FLOAT_TYPE) {
                char reg_aux_2[8];
                get_reg(reg_aux_2);
                fprintf(output, "fcvt.w.s %s, %s, rtz\n", reg_aux_2, rhs_reg);
                fprintf(output, "sw %s, 0(%s)\n", reg_aux_2, reg_aux);
                free_reg(reg_aux_2);                
            }
            else {
                fprintf(output, "sw %s, 0(%s)\n", rhs_reg, reg_aux);
            }
        } 
        else {
            if (rhs_node->dataType == INT_TYPE) {
                char reg_aux_2[8];
                get_f_reg(reg_aux_2);
                fprintf(output, "fcvt.s.w %s, %s\n", reg_aux_2, rhs_reg);
                fprintf(output, "fsw %s, 0(%s)\n", reg_aux_2, reg_aux);
                free_reg(reg_aux_2);           
            }
            else {
                fprintf(output, "fsw %s, 0(%s)\n", rhs_reg, reg_aux);
            }
        }
        free_reg(reg_offset);
        free_reg(reg_aux);  
    }

    free_reg(rhs_reg);
}

void genIfStmt(AST_NODE *ifStmtNode) {
    AST_NODE *test_node = ifStmtNode->child;
    AST_NODE *then_node = test_node->rightSibling;
    AST_NODE *else_node = then_node->rightSibling;

    int stmt_name_cnt = g_name_cnt++;

    if (test_node->nodeType == STMT_NODE && test_node->semantic_value.stmtSemanticValue.kind != FUNCTION_CALL_STMT) { // [inf] assignment inside condition
        genAssignment(test_node);
        test_node = test_node->child;
    }
    char test_result_reg[8];
    if (test_node->dataType == INT_TYPE) {
        get_reg(test_result_reg);
    } else {
        get_f_reg(test_result_reg);
    }
    genExprRelated(test_result_reg, test_node);

    if (test_node->dataType != INT_TYPE) {
        char reg_aux[8];
        get_reg(reg_aux);
        fprintf(output, "fcvt.w.s %s, %s, rtz\n", reg_aux, test_result_reg);
        free_reg(test_result_reg);
        strcpy(test_result_reg, reg_aux);
    }

    fprintf(output, "beqz %s, _elseLabel_%d\n", test_result_reg, stmt_name_cnt);
    free_reg(test_result_reg);
    genStmt(then_node);
    fprintf(output, "j _ifExitLabel_%d\n", stmt_name_cnt);
    fprintf(output, "_elseLabel_%d:\n", stmt_name_cnt);
    genStmt(else_node);
    fprintf(output, "_ifExitLabel_%d:\n", stmt_name_cnt);
}

void genWhileStmt(AST_NODE *whileStmtNode) {
    AST_NODE *test_node = whileStmtNode->child;
    AST_NODE *loop_content_node = test_node->rightSibling;

    int stmt_name_cnt = g_name_cnt++;
    fprintf(output, "_whileLabel_%d:\n", stmt_name_cnt);

    if( test_node->nodeType == STMT_NODE ){
        genAssignment(test_node);
        test_node = test_node->child;
    }
    char test_result_reg[8];
    if (test_node->dataType == INT_TYPE) {
        get_reg(test_result_reg);
    } else {
        get_f_reg(test_result_reg);
    }
    genExprRelated(test_result_reg, test_node);

    if (test_node->dataType != INT_TYPE) {
        char reg_aux[8];
        get_reg(reg_aux);
        fprintf(output, "fcvt.w.s %s, %s, rtz\n", reg_aux, test_result_reg);
        free_reg(test_result_reg);
        strcpy(test_result_reg, reg_aux);
    }

    fprintf(output, "beqz %s, _whileExitLabel_%d\n", test_result_reg, stmt_name_cnt);
    genStmt(loop_content_node);
    fprintf(output, "j _whileLabel_%d\n", stmt_name_cnt);
    fprintf(output, "_whileExitLabel_%d:\n", stmt_name_cnt);

    free_reg(test_result_reg);
}

void genReturnStmt(AST_NODE *retStmtNode) {
    // [msg] not covered in test cases
    AST_NODE *retValNode = retStmtNode->child;
    AST_NODE *funcNode = retStmtNode;
    while(!(funcNode->nodeType == DECLARATION_NODE && funcNode->semantic_value.declSemanticValue.kind == FUNCTION_DECL)){
        funcNode = funcNode->parent;
    }
    char *funcName = funcNode->child->rightSibling->semantic_value.identifierSemanticValue.identifierName;
    printf("[return] in func %s\n", funcName);
    if(retValNode->nodeType != NUL_NODE){
        char reg[8];
        if(retValNode->dataType == INT_TYPE){
            get_reg(reg);
            genExprRelated(reg, retValNode);
            fprintf(output, "mv a0, %s\n", reg);
        }
        else{
            get_f_reg(reg);
            genExprRelated(reg, retValNode);
            fprintf(output, "fmv.s fa0, %s\n", reg);
        }
        free_reg(reg);
    }
    fprintf(output, "j _end_%s\n", funcName);
    return;
}

void genForStmt(AST_NODE *forStmtNode) {
    AST_NODE *init_node = forStmtNode->child;
    AST_NODE *test_node = init_node->rightSibling;
    AST_NODE *iter_node = test_node->rightSibling;
    AST_NODE *block_node = iter_node->rightSibling;

    AST_NODE *tmp = init_node->child;
    while(tmp!=NULL){
        genAssignment(tmp);
        tmp = tmp->rightSibling;
    }

    int stmt_name_cnt = g_name_cnt++;
    fprintf(output, "_forLabel_%d:\n", stmt_name_cnt);

    char test_result_reg[8];
    if(test_node->nodeType != NUL_NODE){
        tmp = test_node->child;
        if (tmp->dataType == INT_TYPE) {
            get_reg(test_result_reg);
        } else {
            get_f_reg(test_result_reg);
        }
        genExprRelated(test_result_reg, tmp);

        if (tmp->dataType != INT_TYPE) {
            char reg_aux[8];
            get_reg(reg_aux);
            fprintf(output, "fcvt.w.s %s, %s, rtz\n", reg_aux, test_result_reg);
            free_reg(test_result_reg);
            strcpy(test_result_reg, reg_aux);
        }
    }
    
    fprintf(output, "beqz %s, _forExitLabel_%d\n", test_result_reg, stmt_name_cnt);
    genStmt(block_node);
    genStmtList(iter_node);
    fprintf(output, "j _forLabel_%d\n", stmt_name_cnt);
    fprintf(output, "_forExitLabel_%d:\n", stmt_name_cnt);

    free_reg(test_result_reg);
}

void genStmt(AST_NODE *stmtNode){
    if(stmtNode->nodeType == BLOCK_NODE) {
        genBlock(stmtNode);
    }
    else if (stmtNode->nodeType == STMT_NODE) {
        switch(stmtNode->semantic_value.stmtSemanticValue.kind){
            case IF_STMT:
                genIfStmt(stmtNode);
                break;
            case WHILE_STMT:
                genWhileStmt(stmtNode);
                break;
            case ASSIGN_STMT:
                genAssignment(stmtNode);
                break;
            case FUNCTION_CALL_STMT:
                genFunctionCall(stmtNode);
                break;
            case RETURN_STMT:
                genReturnStmt(stmtNode);
                break;
            case FOR_STMT:
                genForStmt(stmtNode);
                break;
        }
    }
    return;
}

void genStmtList(AST_NODE *stmtListNode){
    AST_NODE *cur_stmt = stmtListNode->child;
    while(cur_stmt){
        switch(cur_stmt->nodeType){
            case BLOCK_NODE:
                genBlock(cur_stmt);
                break;
            case STMT_NODE:
                switch (cur_stmt->semantic_value.stmtSemanticValue.kind){
                    case WHILE_STMT:
                        genWhileStmt(cur_stmt);
                        break;
                    case IF_STMT:
                        genIfStmt(cur_stmt);
                        break;
                    case ASSIGN_STMT:
                        genAssignment(cur_stmt);
                        break;
                    case FUNCTION_CALL_STMT:
                        genFunctionCall(cur_stmt);
                        break;
                    case RETURN_STMT:
                        genReturnStmt(cur_stmt);
                        break;
                    case FOR_STMT:
                        genForStmt(cur_stmt);
                        break;
                    default:
                        printf("[error] unexpected stmtkind of stmtnode in genStmtList()\n");
                        break;
                }
            case NUL_NODE: //nothing to do
                break;
            default:
                printf("[error] unexpected nodetype of stmtnode in genStmtList()\n");
        }
        cur_stmt = cur_stmt->rightSibling;
    }

}

void genBlock(AST_NODE *blockNode){
    AST_NODE *cur_content_node = blockNode->child;
    if (cur_content_node == NULL) { // [inf] nothing in block
        return;
    }
    //[inf] block -> decl_list stmt_list | decl_list | stmt_list
    if(cur_content_node->rightSibling){ //[inf] block -> decl_list stmt_list
        genLocalVars(cur_content_node);
        genStmtList(cur_content_node->rightSibling);
    } 
    else {
        switch(cur_content_node->nodeType){
            case VARIABLE_DECL_LIST_NODE:   //[inf] block -> decl_list
                genLocalVars(cur_content_node);
                break;
            case STMT_LIST_NODE:            //[inf] block -> stmt_list
                genStmtList(cur_content_node);
                break;
            default:
                printf("[error] unexpected nodetype of block child in genFunctionDecl()\n");
                break;
        }
    }
}

void genFunctionDecl(AST_NODE *funcDeclNode) {
    AST_NODE *type_node = funcDeclNode->child;
    AST_NODE *func_id_node = type_node->rightSibling;
    AST_NODE *param_list_node = func_id_node->rightSibling;
    AST_NODE *block_node = param_list_node->rightSibling;

    char *funcIDName = func_id_node->semantic_value.identifierSemanticValue.identifierName;

    genPrologue(funcIDName);
    genBlock(block_node);
    genEpilogue(funcIDName);

    return;
}

void genGlobalVars(AST_NODE *globalVarDeclListNode) {
    AST_NODE *cur_decl = globalVarDeclListNode->child;
    fprintf(output, ".data\n");

    int last_is_arr = 0;

    while ( cur_decl != NULL ) {
        if ( cur_decl->semantic_value.declSemanticValue.kind == VARIABLE_DECL ) {
            AST_NODE *type_node = cur_decl->child;
            AST_NODE *cur_id_node = type_node->rightSibling;
            printf("[saw type] %s.\n", type_node->semantic_value.identifierSemanticValue.identifierName);

            while ( cur_id_node != NULL ) {
                TypeDescriptor *id_type = cur_id_node->semantic_value.identifierSemanticValue.symbolTableEntry->attribute->attr.typeDescriptor;
                printf("  -- [saw var] %s (kind: %d).\n", cur_id_node->semantic_value.identifierSemanticValue.identifierName, id_type->kind);

                if ( id_type->kind == ARRAY_TYPE_DESCRIPTOR ) {
                    printf("  -- is array.\n");
                    last_is_arr = 1;

                    int array_size = 1;
                    for (int d = 0; d < id_type->properties.arrayProperties.dimension; d++) {
                        array_size *= id_type->properties.arrayProperties.sizeInEachDimension[d];
                    }
                    printf("  -- memory size: %d.\n", array_size * ELEM_SIZE);
                    fprintf(output, ".comm _g_%s, %d, %d\n", 
                        cur_id_node->semantic_value.identifierSemanticValue.identifierName,
                        array_size * ELEM_SIZE,
                        ELEM_SIZE
                    );
                } else {
                    if (last_is_arr) {
                        fprintf(output, ".data\n");
                        last_is_arr = 0;
                    }

                    printf("  -- is scalar (dataType: %d).\n", id_type->properties.dataType);
                    // [msg] remember to consider <expr> RHS
                    if ( id_type->properties.dataType == INT_TYPE ) {
                        int init = (cur_id_node->semantic_value.identifierSemanticValue.kind == WITH_INIT_ID) ?\
                                    cur_id_node->child->semantic_value.const1->const_u.intval : 0;
                        fprintf(output, "_g_%s: .word %d\n",
                            cur_id_node->semantic_value.identifierSemanticValue.identifierName,
                            init
                        );
                    } else {
                        float init = (cur_id_node->semantic_value.identifierSemanticValue.kind == WITH_INIT_ID) ?\
                                    cur_id_node->child->semantic_value.const1->const_u.fval : 0;
                        fprintf(output, "_g_%s: .word 0x%x\n",
                            cur_id_node->semantic_value.identifierSemanticValue.identifierName,
                            convert_float_to_uint(init)
                        );
                    }
                }
                cur_id_node = cur_id_node->rightSibling;
            }
        }
        else if(cur_decl->semantic_value.declSemanticValue.kind == TYPE_DECL){
            //nothing to do
        }
        cur_decl = cur_decl->rightSibling;
    }

    return;
}

void genProgram(AST_NODE *programNode) {
    AST_NODE *cur_prog_child = programNode->child;
    while ( cur_prog_child != NULL ) {
        switch (cur_prog_child->nodeType) {
            case VARIABLE_DECL_LIST_NODE:
                genGlobalVars(cur_prog_child);
                break;
            case DECLARATION_NODE: // [inf] always a function
                genFunctionDecl(cur_prog_child);
                break;
            default:
                break;
        }
        cur_prog_child = cur_prog_child->rightSibling;
    }
    return;
}

void codeGeneration(AST_NODE *programNode) {
    output = fopen("output.s", "w");
    genProgram(programNode);
    fclose(output);
    return;
}