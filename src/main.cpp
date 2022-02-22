#include <iostream>
#include <string>
#include "nodes.hh"

int main(int argc, char *argv[])
{
   ni::NProgram program;
   int op;
   if (argc != 3)
   {
      std::cerr << "Usage: " << argv[0] << " <srcfile> <destfile>" << std::endl;
      exit(1);
   }

   auto f = std::string(argv[1]);
   op = program.parse(f);
   if (op != 0)
   {
      exit(1);
   }

   std::string error;
   ni::LLVMCodegen c(program);
   if (c.codegen(std::string(argv[2]), error) == 1)
   {
      std::cerr << error << std::endl;
      exit(1);
   }
   return 0;
}
