%{
    #include "node.h"
    NBlock *programBlock; /* the top level root node of our final AST */

    extern FILE *file_out;
    extern int yylex();
    void yyerror(const char* s) {
      fprintf(file_out, "Parse error: %s\n", s);
    }
%}

/* Represents the many different ways we can access our data */
%union {
    Node *node;
    NBlock *block;
    NExpression *expr;
    NStatement *stmt;
    NIdentifier *ident;
    NVariableDeclaration *var_decl;
    std::vector<NVariableDeclaration*> *varvec;
    std::vector<NExpression*> *exprvec;
    std::string *string;
    int token;
}

/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <string> TIDENTIFIER TINTEGER TDOUBLE TSTRING
%token <token> TCEQ TCNE TCLT TCLE TCGT TCGE TEQUAL
%token <token> TLPAREN TRPAREN TLBRACE TRBRACE TCOMMA TDOT
%token <token> TPLUS TMINUS TMUL TDIV TMOD TSEMI
%token <token> TRETURN TIF TELSE TWHILE TFOR TDO TFOREACH
%token <token> TLBRACKET TRBRACKET

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <ident> ident
%type <expr> value expr 
%type <varvec> func_decl_args
%type <exprvec> call_args list_args
%type <block> program stmts block
%type <stmt> stmt var_decl func_decl
%type <token> comparison

/* Operator precedence for mathematical operators */
%left TPLUS TMINUS
%left TMUL TDIV TPLUSEQ

%start program

%%

program : stmts { programBlock = $1; }
        ;
        
stmts : stmt TSEMI { $$ = new NBlock(); $$->statements.push_back($<stmt>1); }
      | stmts stmt TSEMI { $1->statements.push_back($<stmt>2); }
      ;

stmt : var_decl | func_decl 
     | expr { $$ = new NExpressionStatement(*$1); }
     | TRETURN expr { $$ = new NReturnStatement(*$2); }
     | TIF TLPAREN expr TRPAREN block TELSE block { $$ = new NIfElseStatement(*$3, *$5, *$7); }
     | TIF TLPAREN expr TRPAREN block { $$ = new NIfStatement(*$3, *$5); }
     | TWHILE TLPAREN expr TRPAREN block { $$ = new NWhileStatement( *$3, *$5); }
     | TFOR TLPAREN expr TSEMI expr TSEMI expr TRPAREN block { $$ = new NForStatement( *$3, *$5, *$7, *$9); }
     | TDO block TWHILE TLPAREN expr TRPAREN { $$ = new NDoWhileStatement( *$5, *$2); }
     | TFOREACH TLPAREN expr TRPAREN block { $$ = new NForEachStatement( *$3, *$5); }
     ;

block : TLBRACE stmts TRBRACE { $$ = $2; }
      | TLBRACE TRBRACE { $$ = new NBlock(); }
      ;

var_decl : ident ident { $$ = new NVariableDeclaration(*$1, *$2); }
         | ident ident TEQUAL expr { $$ = new NVariableDeclaration(*$1, *$2, $4); }
         ;
        
func_decl : ident ident TLPAREN func_decl_args TRPAREN block 
            { $$ = new NFunctionDeclaration(*$1, *$2, *$4, *$6); delete $4; }
          ;
    
func_decl_args : /*blank*/  { $$ = new VariableList(); }
          | var_decl { $$ = new VariableList(); $$->push_back($<var_decl>1); }
          | func_decl_args TCOMMA var_decl { $1->push_back($<var_decl>3); }
          ;

ident : TIDENTIFIER { $$ = new NIdentifier(*$1); delete $1; }
      ;

value : TINTEGER { $$ = new NInteger(atol($1->c_str())); delete $1; }
        | TDOUBLE { $$ = new NDouble(atof($1->c_str())); delete $1; }
        | TSTRING { $$ = new NString((*$1).substr(1, (*$1).size() - 2)); delete $1; }
        ;

list_args : /*blank*/  { $$ = new ExpressionList(); }
            | expr { $$ = new ExpressionList(); $$->push_back($1); }
            | list_args TCOMMA expr { $1->push_back($3); }
            ;

expr : ident TEQUAL expr { $$ = new NAssignment(*$<ident>1, *$3); }
     | ident TLPAREN call_args TRPAREN { $$ = new NMethodCall(*$1, *$3); delete $3; }
     | ident { $<ident>$ = $1; }
     | value
     | expr comparison expr { $$ = new NBinaryOperator(*$1, $2, *$3); }
     | TLPAREN expr TRPAREN { $$ = $2; }
     | TLBRACKET list_args TRBRACKET { $$ = new NList( *$2); delete $2;}
     ;
    
call_args : /*blank*/  { $$ = new ExpressionList(); }
          | expr { $$ = new ExpressionList(); $$->push_back($1); }
          | call_args TCOMMA expr  { $1->push_back($3); }
          ;

comparison : TCEQ | TCNE | TCLT | TCLE | TCGT | TCGE 
           | TPLUS | TMINUS | TMUL | TDIV | TMOD
           ;

%%