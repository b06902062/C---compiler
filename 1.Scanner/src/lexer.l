%option noyywrap 
%{
#include <stdio.h>
#include<stdbool.h>
#include "header.h" 
int linenumber = 0;
symtab * lookup();
symtab * ptr;
void  insertID();
void  printSymTab();
void  insertionSort();
void  printIdList();
bool  isReserved();
void  printFreqOfId();
void  printFreqOfIdNew();
/* You need to define for all tokens in C--, here are some examples */
#define INT		10
#define FLOAT 		11 
#define OP_ASSIGN 	12 
#define OP_OR 		13 
#define MK_LPAREN 	14 
#define MK_RPAREN 	15 
#define ERROR 		100 

%}

letter   [A-Za-z]
digit    [0-9]
ID	 {letter}({letter}|{digit}|"_")*
invalid_ID {digit}+({letter}|"_")+
WS	 [ \t\r]+
Int_constant {digit}+

/* You need to define the following RE's */
Float_constant   [+-]?({digit}+([.]{digit}*)?([eE][+-]?{digit}+)?|[.]{digit}+([eE][+-]?{digit}+)?)
String_constant  \"(\\.|[^\\"])*\"
comment          \/\*([^*]|\*+[^*/])*\*+\/



/* Other operators appear here */ 
OP_or   "||"
OP_and  "&&"
OP_geq  ">="
OP_leq  "<="
OP_neq  "!="
OP_eq   "=="

OP_mul   "*"
OP_div   "/"
OP_add   "+"
OP_minus         "-"
OP_assign        "="
OP_gt    ">"
OP_lt    "<"
OP_not   "!"


/* Other separators appear here */
DL_lparen "("
DL_rparen ")"
DL_lbrack "["
DL_rbrack "]"
DL_lbrace "{"
DL_rbrace "}"
DL_comma  ","
DL_semicol ";"
DL_dot  "."

newline  "\n"

error    .

%%

{WS}            {}
{Int_constant}         {}
{Float_constant}       {/* return FLOAT; */}
{String_constant}      {}
{comment}       {printf("%s\n", yytext);}
{ID}            { 
			ptr = lookup(yytext);
			if (ptr == NULL)
			     insertID(yytext);	
			else 
			     ptr->counter++;
		}

{OP_or}         /* return OP_OR; */
{OP_and}  
{OP_geq}  
{OP_leq} 
{OP_neq} 
{OP_eq}   
{OP_mul}
{OP_div}   
{OP_add}   
{OP_minus}
{OP_assign}     /* return OP_ASSIGN; */
{OP_gt}
{OP_lt}
{OP_not}         
{DL_lparen}     /* return MK_LPAREN; */
{DL_rparen}     /* return MK_RPAREN; */
{DL_lbrack}
{DL_rbrack}
{DL_lbrace}     /* return MK_LBRACE; */
{DL_rbrace}     /* return MK_RBRACE; */
{DL_comma}      {}
{DL_semicol}    {}
{DL_dot}        {}
{newline}       linenumber += 1;
{invalid_ID}    {
   printf("ERR : line %d, invalid_ID (w/ leading digits): %s\n", linenumber+1, yytext);
   exit(1);
}
{error}         { 
   printf("ERR : line %d, unrecognizable token: %s.\n", linenumber+1, yytext);
   exit(1);
}


%%

main(int argc, char **argv)
{
   argc--; ++argv;
   if (argc > 0)
      yyin = fopen(argv[0], "r");
   else
      yyin = stdin;
   yylex();
   // printSymTab();
   printFreqOfIdNew();
}
