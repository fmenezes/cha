#include <iostream>
#include <string>
#include "nodes.hh"

int main(int argc, char *argv[])
{
   ni::NProgram program;
   int op;
   if (argc == 2)
   {
      auto f = std::string(argv[1]);
      op = program.parse(f);
   }
   else
   {
      op = program.parse();
   }
   if (op != 0) {
      exit(1);
   }
   std::string error;
   if (program.codegen(error) == 1)
   {
      std::cerr << error << std::endl;
   }
   return 0;
}
