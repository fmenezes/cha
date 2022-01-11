#include <iostream>
#include <string>
#include "nodes.h"
#include "parser.tab.h"

int main()
{
   yyparse();
   return 0;
}
