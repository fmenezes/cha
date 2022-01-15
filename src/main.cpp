#include <iostream>
#include <string>
#include "nodes.h"

ni::NProgram *program;

int main()
{
   program->parse();
   std::string error;
   std::cout << program->to_string() << std::endl;
   if (program->codegen(error) == 1)
   {
      std::cerr << error;
   }
   return 0;
}
