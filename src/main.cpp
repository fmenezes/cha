#include <iostream>
#include <string>
#include "nodes.h"

ni::NProgram *program;

int main()
{
   program->parse();
   std::cout << program->to_string();
   return 0;
}
