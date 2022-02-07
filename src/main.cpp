#include <iostream>
#include <string>
#include "nodes.hh"

int main(int argc, char *argv[])
{
   ni::NProgram program;
   if (argc == 2)
   {
      auto f = std::string(argv[1]);
      program.parse(f);
   }
   else
   {
      program.parse();
   }
   std::string error;
   if (program.codegen(error) == 1)
   {
      std::cerr << error << std::endl;
   }
   return 0;
}
