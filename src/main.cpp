#include <iostream>
#include <string>
#include "nodes.hh"

void printUsage(const std::string& app)
{
   std::cerr << "Usage: " << app << " <format> <srcfile> <destfile>" << std::endl;
   std::cerr << "format: -ll for LLVM IR or -asm for Assembly" << std::endl;
}

int main(int argc, char *argv[])
{
   ni::NProgram program;
   int op;
   if (argc != 4)
   {
      printUsage(argv[0]);
      exit(1);
   }

   auto format = std::string(argv[1]);
   auto input = std::string(argv[2]);
   auto output = std::string(argv[3]);

   auto f = std::string(argv[2]);
   op = program.parse(f);
   if (op != 0)
   {
      exit(1);
   }

   std::string error;
   ni::Codegen *c;
   if (format.compare("-ll") == 0)
   {
      c = new ni::LLVMCodegen(program);
   }
   else if (format.compare("-asm") == 0)
   {
      c = new ni::ASMCodegen(program);
   }
   else
   {
      std::cerr << "Error: invalid format"
                << std::endl
                << std::endl;
      printUsage(argv[0]);
      exit(1);
   }
   if (c->codegen(output, error) == 1)
   {
      delete c;
      std::cerr << error << std::endl;
      exit(1);
   }
   delete c;
   return 0;
}
