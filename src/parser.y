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

extern "C" {
    int yylex();
    extern FILE *yyin;
}
%}

%locations

%code{
# include "ast.hpp"
AstLocation convert_location(YYLTYPE start, YYLTYPE end);
int yyerror(const char *msg);
}

%union {
  char *str;
  AstNodePtr* node;
  AstTypePtr* type;
  AstNodeList* list;
}

%token KEYWORD_FUN OPEN_PAR CLOSE_PAR OPEN_CUR CLOSE_CUR COMMA KEYWORD_VAR EQUALS KEYWORD_RET PLUS MINUS STAR SLASH KEYWORD_INT8 KEYWORD_UINT8 KEYWORD_INT16 KEYWORD_UINT16 KEYWORD_INT32 KEYWORD_UINT32 KEYWORD_INT64 KEYWORD_UINT64 KEYWORD_INT KEYWORD_UINT KEYWORD_FLOAT16 KEYWORD_FLOAT32 KEYWORD_FLOAT64 KEYWORD_BOOL BOOL_TRUE BOOL_FALSE EQUALS_EQUALS NOT_EQUALS GREATER_THAN GREATER_THAN_OR_EQUALS LESS_THAN LESS_THAN_OR_EQUALS AND OR KEYWORD_CONST KEYWORD_IF KEYWORD_ELSE
%token <str> IDENTIFIER INTEGER UINTEGER FLOAT

%nterm <list> top_level block def_args call_args statements
%nterm <node> instruction const_definition function statement arg expr const_value
%nterm <type> reftype

%left PLUS MINUS
%left STAR SLASH

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
	| expr PLUS expr																{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), BinaryOperator::PLUS, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr MINUS expr																{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), BinaryOperator::MINUS, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr STAR expr																{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), BinaryOperator::STAR, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr SLASH expr																{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), BinaryOperator::SLASH, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr EQUALS_EQUALS expr														{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), BinaryOperator::EQUALS_EQUALS, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr NOT_EQUALS expr															{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), BinaryOperator::NOT_EQUALS, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr GREATER_THAN expr														{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), BinaryOperator::GREATER_THAN, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr GREATER_THAN_OR_EQUALS expr												{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), BinaryOperator::GREATER_THAN_OR_EQUALS, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr LESS_THAN expr															{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), BinaryOperator::LESS_THAN, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr LESS_THAN_OR_EQUALS expr													{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), BinaryOperator::LESS_THAN_OR_EQUALS, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr AND expr																	{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), BinaryOperator::AND, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| expr OR expr																	{ $$ = new AstNodePtr(std::make_unique<BinaryOpNode>(convert_location(@1, @3), BinaryOperator::OR, std::move(*$1), std::move(*$3))); delete $1; delete $3; }
	| OPEN_PAR expr CLOSE_PAR														{ $$ = $2; }
	;

reftype :
	KEYWORD_INT																		{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::INT})); }
	| KEYWORD_UINT																	{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::UINT})); }
	| KEYWORD_INT8																	{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::INT8})); }
	| KEYWORD_UINT8																	{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::UINT8})); }
	| KEYWORD_INT16																	{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::INT16})); }
	| KEYWORD_UINT16																{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::UINT16})); }
	| KEYWORD_INT32																	{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::INT32})); }
	| KEYWORD_UINT32																{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::UINT32})); }
	| KEYWORD_INT64																	{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::INT64})); }
	| KEYWORD_UINT64																{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::UINT64})); }
	| KEYWORD_FLOAT16																{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::FLOAT16})); }
	| KEYWORD_FLOAT32																{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::FLOAT32})); }
	| KEYWORD_FLOAT64																{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::FLOAT64})); }
	| KEYWORD_BOOL																	{ $$ = new AstTypePtr(std::make_unique<AstType>(convert_location(@1, @1), AstType::Primitive{PrimitiveType::BOOL})); }
	;

const_value :
	INTEGER																			{ 
		char *endptr;
		long long value = strtoll($1, &endptr, 0);
		if (*endptr != '\0' || endptr == $1) {
			throw ParseException(convert_location(@1, @1), "Invalid integer literal: " + std::string($1));
		}
		$$ = new AstNodePtr(std::make_unique<ConstantIntegerNode>(convert_location(@1, @1), value)); 
	}
	| UINTEGER																		{ 
		// Remove the 'u' suffix before parsing
		std::string str_value = $1;
		if (!str_value.empty() && str_value.back() == 'u') {
			str_value.pop_back();
		}
		
		char *endptr;
		unsigned long long value = strtoull(str_value.c_str(), &endptr, 0);
		if (*endptr != '\0' || endptr == str_value.c_str()) {
			throw ParseException(convert_location(@1, @1), "Invalid unsigned integer literal: " + std::string($1));
		}
		$$ = new AstNodePtr(std::make_unique<ConstantUnsignedIntegerNode>(convert_location(@1, @1), value)); 
	}
	| FLOAT																			{ 
		char *endptr;
		double value = strtod($1, &endptr);
		if (*endptr != '\0' || endptr == $1) {
			throw ParseException(convert_location(@1, @1), "Invalid float literal: " + std::string($1));
		}
		$$ = new AstNodePtr(std::make_unique<ConstantFloatNode>(convert_location(@1, @1), value)); 
	}
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

int yyerror(const char *msg) {
  AstLocation location = convert_location(yylloc, yylloc);
  throw ParseException(location, std::string(msg));
}

// Main parser function (C++ interface)
namespace cha {

AstNodeList parse(const char *file) {
  current_file = file;
  FILE *f = fopen(file, "r");
  if (f == NULL) {
    throw ParseException(
      AstLocation(std::string(file), 1, 1, 1, 1),
      std::string("Could not open file")
    );
  }

  AstNodeList result;
  yyin = f;
  parsed_ast = &result;
  
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
    
    return result;
  } catch (...) {
    // Clean up and re-throw
    fclose(f);
    current_file = nullptr;
    parsed_ast = nullptr;
    throw;
  }
}

} // namespace cha
