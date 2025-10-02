%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ast.hpp"
#include "parser.hpp" 
#include "exceptions.hpp"

using namespace cha;
AstNodeList *parsed_ast;
const char *current_file = nullptr;

int yyerror(const char *msg) {
  AstLocation location(
    current_file ? std::string(current_file) : std::string(""),
    1, 1, 1, 1  // Default location since yylloc isn't available here  
  );
  
  // Throw parse exception instead of logging
  throw ParseException(location, std::string(msg));
}

extern "C" {
    int yylex();
    extern FILE *yyin;
}
%}

%locations

%code{
# include "ast.hpp"
AstLocation convert_location(YYLTYPE start, YYLTYPE end);
}

%union {
  char *str;
  AstNodePtr* node;
  AstTypePtr* type;
  AstNodeList* list;
}

%token KEYWORD_FUN OPEN_PAR CLOSE_PAR OPEN_CUR CLOSE_CUR COMMA KEYWORD_VAR EQUALS KEYWORD_RET ADD SUBTRACT MULTIPLY DIVIDE REFTYPE_INT8 REFTYPE_UINT8 REFTYPE_INT16 REFTYPE_UINT16 REFTYPE_INT32 REFTYPE_UINT32 REFTYPE_INT64 REFTYPE_UINT64 REFTYPE_INT REFTYPE_UINT REFTYPE_FLOAT16 REFTYPE_FLOAT32 REFTYPE_FLOAT64 REFTYPE_BOOL BOOL_TRUE BOOL_FALSE EQUALS_EQUALS NOT_EQUALS GREATER_THAN GREATER_THAN_OR_EQUALS LESS_THAN LESS_THAN_OR_EQUALS AND OR KEYWORD_CONST KEYWORD_IF KEYWORD_ELSE
%token <str> IDENTIFIER INTEGER UINTEGER FLOAT

%nterm <list> top_level block def_args call_args statements
%nterm <node> instruction const_definition function statement arg expr const_value
%nterm <type> reftype

%left ADD SUBTRACT
%left MULTIPLY DIVIDE

%start parse

%%

parse :
	top_level																		{ *parsed_ast = std::move(*$1); delete $1; }
	;

top_level :
	instruction																		{ $$ = new AstNodeList(); $$->push_back(std::move(*$1)); delete $1; }
	| top_level instruction															{ $$ = $1; $$->push_back(std::move(*$2)); delete $2; }
	;

instruction :
	const_definition																{ $$ = $1; }
	| function																		{ $$ = $1; }
	;

const_definition :
	KEYWORD_CONST IDENTIFIER EQUALS const_value										{ $$ = new AstNodePtr(std::make_unique<ConstantDeclarationNode>(convert_location(@1, @4), std::string($2), std::move(*$4))); delete $4; }
	;

function :
	KEYWORD_FUN IDENTIFIER OPEN_PAR CLOSE_PAR block									{ 
		auto void_type = std::make_unique<AstType>(convert_location(@1, @5), AstType::Primitive{PrimitiveType::UNDEF});
		AstNodeList args;
		$$ = new AstNodePtr(std::make_unique<FunctionDeclarationNode>(convert_location(@1, @5), std::string($2), std::move(void_type), std::move(args), std::move(*$5))); 
		delete $5; 
	}
	| KEYWORD_FUN IDENTIFIER OPEN_PAR def_args CLOSE_PAR block						{ 
		auto void_type = std::make_unique<AstType>(convert_location(@1, @6), AstType::Primitive{PrimitiveType::UNDEF});
		$$ = new AstNodePtr(std::make_unique<FunctionDeclarationNode>(convert_location(@1, @6), std::string($2), std::move(void_type), std::move(*$4), std::move(*$6))); 
		delete $4; delete $6; 
	}
	| KEYWORD_FUN IDENTIFIER OPEN_PAR CLOSE_PAR reftype block						{ 
		AstNodeList args;
		$$ = new AstNodePtr(std::make_unique<FunctionDeclarationNode>(convert_location(@1, @6), std::string($2), std::move(*$5), std::move(args), std::move(*$6))); 
		delete $5; delete $6; 
	}
	| KEYWORD_FUN IDENTIFIER OPEN_PAR def_args CLOSE_PAR reftype block				{ 
		$$ = new AstNodePtr(std::make_unique<FunctionDeclarationNode>(convert_location(@1, @7), std::string($2), std::move(*$6), std::move(*$4), std::move(*$7))); 
		delete $4; delete $6; delete $7; 
	}
	;

block :
	OPEN_CUR statements CLOSE_CUR													{ $$ = $2; }
	| OPEN_CUR CLOSE_CUR															{ $$ = new AstNodeList(); }
	;

arg :
	IDENTIFIER reftype																{ $$ = new AstNodePtr(std::make_unique<ArgumentNode>(convert_location(@1, @2), std::string($1), std::move(*$2))); delete $2; }
	;

def_args :
	arg																				{ $$ = new AstNodeList(); $$->push_back(std::move(*$1)); delete $1; }
	| def_args COMMA arg															{ $$ = $1; $$->push_back(std::move(*$3)); delete $3; }
	;

call_args :
	expr																			{ $$ = new AstNodeList(); $$->push_back(std::move(*$1)); delete $1; }
	| call_args COMMA expr															{ $$ = $1; $$->push_back(std::move(*$3)); delete $3; }
	;

statements :
	statement																		{ $$ = new AstNodeList(); $$->push_back(std::move(*$1)); delete $1; }
	| statements statement															{ $$ = $1; $$->push_back(std::move(*$2)); delete $2; }
	;

statement :
	KEYWORD_VAR IDENTIFIER reftype													{ $$ = new AstNodePtr(std::make_unique<VariableDeclarationNode>(convert_location(@1, @3), std::string($2), std::move(*$3), nullptr)); delete $3; }
	| KEYWORD_VAR IDENTIFIER reftype EQUALS expr									{ $$ = new AstNodePtr(std::make_unique<VariableDeclarationNode>(convert_location(@1, @5), std::string($2), std::move(*$3), std::move(*$5))); delete $3; delete $5; }
	| IDENTIFIER EQUALS expr														{ $$ = new AstNodePtr(std::make_unique<VariableAssignmentNode>(convert_location(@1, @3), std::string($1), std::move(*$3))); delete $3; }
	| expr																			{ $$ = $1; }
	| KEYWORD_RET expr																{ $$ = new AstNodePtr(std::make_unique<FunctionReturnNode>(convert_location(@1, @2), std::move(*$2))); delete $2; }
	| KEYWORD_RET 																	{ $$ = new AstNodePtr(std::make_unique<FunctionReturnNode>(convert_location(@1, @1), nullptr)); }
	| KEYWORD_IF expr block															{ 
		AstNodeList empty_else;
		$$ = new AstNodePtr(std::make_unique<IfNode>(convert_location(@1, @3), std::move(*$2), std::move(*$3), std::move(empty_else))); 
		delete $2; delete $3; 
	}
	| KEYWORD_IF expr block KEYWORD_ELSE block										{ $$ = new AstNodePtr(std::make_unique<IfNode>(convert_location(@1, @5), std::move(*$2), std::move(*$3), std::move(*$5))); delete $2; delete $3; delete $5; }
	;

expr :
	const_value																		{ $$ = $1; }
	| IDENTIFIER																	{ $$ = new AstNodePtr(std::make_unique<VariableLookupNode>(convert_location(@1, @1), std::string($1))); }
	| IDENTIFIER OPEN_PAR CLOSE_PAR													{ 
		AstNodeList empty_args;
		$$ = new AstNodePtr(std::make_unique<FunctionCallNode>(convert_location(@1, @3), std::string($1), std::move(empty_args))); 
	}
	| IDENTIFIER OPEN_PAR call_args CLOSE_PAR										{ $$ = new AstNodePtr(std::make_unique<FunctionCallNode>(convert_location(@1, @4), std::string($1), std::move(*$3))); delete $3; }
	| expr ADD expr																	{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), Operator::ADD, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr SUBTRACT expr															{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), Operator::SUBTRACT, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr MULTIPLY expr															{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), Operator::MULTIPLY, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr DIVIDE expr																{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), Operator::DIVIDE, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr EQUALS_EQUALS expr														{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), Operator::EQUALS_EQUALS, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr NOT_EQUALS expr															{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), Operator::NOT_EQUALS, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr GREATER_THAN expr														{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), Operator::GREATER_THAN, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr GREATER_THAN_OR_EQUALS expr												{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), Operator::GREATER_THAN_OR_EQUALS, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr LESS_THAN expr															{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), Operator::LESS_THAN, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr LESS_THAN_OR_EQUALS expr													{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), Operator::LESS_THAN_OR_EQUALS, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr AND expr																	{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), Operator::AND, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr OR expr																	{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), Operator::OR, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| OPEN_PAR expr CLOSE_PAR														{ $$ = $2; }
	;

reftype :
	REFTYPE_INT																		{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::INT})); }
	| REFTYPE_UINT																	{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::UINT})); }
	| REFTYPE_INT8																	{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::INT8})); }
	| REFTYPE_UINT8																	{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::UINT8})); }
	| REFTYPE_INT16																	{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::INT16})); }
	| REFTYPE_UINT16																{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::UINT16})); }
	| REFTYPE_INT32																	{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::INT32})); }
	| REFTYPE_UINT32																{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::UINT32})); }
	| REFTYPE_INT64																	{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::INT64})); }
	| REFTYPE_UINT64																{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::UINT64})); }
	| REFTYPE_FLOAT16																{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::FLOAT16})); }
	| REFTYPE_FLOAT32																{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::FLOAT32})); }
	| REFTYPE_FLOAT64																{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::FLOAT64})); }
	| REFTYPE_BOOL																	{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::BOOL})); }
	;

const_value :
	INTEGER																			{ $$ = new AstNodePtr(std::make_unique<ConstantIntegerNode>(convert_location(@1, @1), std::string($1))); }
	| UINTEGER																		{ $$ = new AstNodePtr(std::make_unique<ConstantUnsignedIntegerNode>(convert_location(@1, @1), std::string($1))); }
	| FLOAT																			{ $$ = new AstNodePtr(std::make_unique<ConstantFloatNode>(convert_location(@1, @1), atof($1))); }
	| BOOL_TRUE																		{ $$ = new AstNodePtr(std::make_unique<ConstantBoolNode>(convert_location(@1, @1), true)); }
	| BOOL_FALSE																	{ $$ = new AstNodePtr(std::make_unique<ConstantBoolNode>(convert_location(@1, @1), false)); }
	;

%%

// yyerror is already defined in the header section above

AstLocation convert_location(YYLTYPE start, YYLTYPE end) {
  return AstLocation(
    current_file ? std::string(current_file) : std::string(""),
    start.first_line,
    start.first_column,
    end.last_line,
    end.last_column
  );
}

// Main parser function (C++ interface)
namespace cha {

void parse(const char *file, AstNodeList &out) {
  current_file = file;
  FILE *f = fopen(file, "r");
  if (f == NULL) {
    throw ParseException(
      AstLocation(std::string(file), 1, 1, 1, 1),
      std::string("Could not open file")
    );
  }

  yyin = f;
  parsed_ast = &out;
  
  try {
    int ret = yyparse();
    fclose(f);
    
    current_file = nullptr;
    parsed_ast = nullptr;
    
    if (ret != 0) {
      throw ParseException(
        AstLocation(std::string(file), 1, 1, 1, 1),
        "Parse failed"
      );
    }
  } catch (...) {
    // Clean up and re-throw
    fclose(f);
    current_file = nullptr;
    parsed_ast = nullptr;
    throw;
  }
}

} // namespace cha
