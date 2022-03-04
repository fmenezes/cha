#include <fstream>
#include <iostream>

#include "nodes.hh"

const std::vector<std::string> REGS({"edi", "esi", "edx", "ecx", "r8d", "r9d"});
int ni::ASMCodegen::internalCodegen(const ni::Node &node,
                                    std::string &returnAddr) {
  auto i = dynamic_cast<const ni::NInteger *>(&node);
  if (i != nullptr) {
    return this->internalCodegen(*i, returnAddr);
  }

  auto b = dynamic_cast<const ni::NBinaryOperation *>(&node);
  if (b != nullptr) {
    return this->internalCodegen(*b, returnAddr);
  }

  auto d = dynamic_cast<const ni::NVariableDeclaration *>(&node);
  if (d != nullptr) {
    return this->internalCodegen(*d, returnAddr);
  }

  auto a = dynamic_cast<const ni::NVariableAssignment *>(&node);
  if (a != nullptr) {
    return this->internalCodegen(*a, returnAddr);
  }

  auto l = dynamic_cast<const ni::NVariableLookup *>(&node);
  if (l != nullptr) {
    return this->internalCodegen(*l, returnAddr);
  }

  auto fd = dynamic_cast<const ni::NFunctionDeclaration *>(&node);
  if (fd != nullptr) {
    return this->internalCodegen(*fd, returnAddr);
  }

  auto fc = dynamic_cast<const ni::NFunctionCall *>(&node);
  if (fc != nullptr) {
    return this->internalCodegen(*fc, returnAddr);
  }

  auto fr = dynamic_cast<const ni::NFunctionReturn *>(&node);
  if (fr != nullptr) {
    return this->internalCodegen(*fr, returnAddr);
  }

  auto p = dynamic_cast<const ni::NProgram *>(&node);
  if (p != nullptr) {
    return this->internalCodegen(*p, returnAddr);
  }

  throw std::runtime_error("Unkown node type");
}

int ni::ASMCodegen::internalCodegen(const ni::NInteger &node,
                                    std::string &returnAddr) {
  returnAddr = "$" + node.value;
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NBinaryOperation &node,
                                    std::string &returnAddr) {
  std::string laddr, raddr;
  int ret = this->internalCodegen(*node.left.get(), laddr);
  if (ret != 0) {
    return ret;
  }
  if (laddr.compare("%eax") == 0) {
    *this->outputFile << "\tmovl\t" << laddr << ", %ebx" << std::endl;
    laddr = "%ebx";
  }

  ret = this->internalCodegen(*node.right.get(), raddr);
  if (ret != 0) {
    return ret;
  }
  if (raddr.compare("%eax") != 0) {
    *this->outputFile << "\tmovl\t" << raddr << ", %eax" << std::endl;
    raddr = "%eax";
  }

  if (node.op.compare("+") == 0) {
    *this->outputFile << "\taddl\t" << laddr << ", " << raddr << std::endl;
    returnAddr = raddr;
    return 0;
  } else if (node.op.compare("-") == 0) {
    *this->outputFile << "\tsubl\t" << laddr << ", " << raddr << std::endl;
    returnAddr = raddr;
    return 0;
  } else if (node.op.compare("*") == 0) {
    *this->outputFile << "\timull\t" << laddr << ", " << raddr << std::endl;
    returnAddr = raddr;
    return 0;
  }

  std::cerr << "Invalid operation " << node.op << std::endl;
  return 1;
}

int ni::ASMCodegen::internalCodegen(const ni::NVariableLookup &node,
                                    std::string &returnAddr) {
  auto s = this->vars.find(node.identifier);
  if (s == this->vars.end()) {
    std::cerr << node.identifier << " not found." << std::endl;
    return 1;
  }
  returnAddr = s->second;
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NVariableAssignment &node,
                                    std::string &returnAddr) {
  auto s = this->vars.find(node.identifier);
  if (s == this->vars.end()) {
    std::cerr << node.identifier << " not found." << std::endl;
    return 1;
  }
  std::string addr;
  int ret = this->internalCodegen(*node.value.get(), addr);
  if (ret != 0) {
    return ret;
  }

  *this->outputFile << "\tmovl\t" << addr << ", " << s->second << std::endl;
  returnAddr = s->second;
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NVariableDeclaration &node,
                                    std::string &returnAddr) {
  auto s = this->vars.find(node.identifier);
  if (s != this->vars.end()) {
    std::cerr << node.identifier << " already exists." << std::endl;
    return 1;
  }

  this->currentStackPosition -= 4;
  returnAddr = std::to_string(this->currentStackPosition) + "(%rbp)";
  this->vars[node.identifier] = returnAddr;
  return 0;
}
int calculateMemorySize(const ni::Node *node) {
  auto b = dynamic_cast<const ni::NBinaryOperation *>(node);
  if (b != nullptr) {
    return calculateMemorySize(b->left.get()) +
           calculateMemorySize(b->right.get());
  }

  auto d = dynamic_cast<const ni::NVariableDeclaration *>(node);
  if (d != nullptr) {
    return 4;
  }

  auto fd = dynamic_cast<const ni::NFunctionDeclaration *>(node);
  if (fd != nullptr) {
    int ret = fd->args.size() * 4;
    if (fd->args.size() > REGS.size()) {
      ret = REGS.size() * 4;
    }
    ret += 4;
    for (auto &it : fd->body) {
      ret += calculateMemorySize(it.get());
    }
    return ret;
  }

  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NFunctionDeclaration &node,
                                    std::string &returnAddr) {
  this->currentFunctionName = this->generateFunctionName(node.identifier);

  int ret = this->generateFunction(this->currentFunctionName);
  if (ret != 0) {
    return ret;
  }
  int memorySize = calculateMemorySize(&node);
  ret = this->generateFunctionPrologue(memorySize);
  if (ret != 0) {
    return ret;
  }
  this->resetStackFrame();

  int argc = node.args.size();
  if (argc > REGS.size()) {
    argc = REGS.size();
  }
  for (int a = 0; a < argc; a++) {
    auto arg = node.args[a];
    this->currentStackPosition -= 4;
    this->vars[arg] = std::to_string(this->currentStackPosition) + "(%rbp)";
    auto reg = REGS[a];
    *this->outputFile << "\tmovl\t%" << reg << ", " << this->vars[arg]
                      << std::endl;
  }

  if (node.args.size() > 6) {
    int offset = 16;
    for (int a = 6; a < node.args.size(); a++) {
      auto arg = node.args[a];
      this->vars[arg] = std::to_string(offset) + "(%rbp)";
      offset += 8;
    }
  }

  for (const auto &it : node.body) {
    int ret = this->internalCodegen(*it.get(), returnAddr);
    if (ret != 0) {
      return ret;
    }
  }
  returnAddr = this->currentFunctionName;
  return this->generateFunctionEpilogue(this->currentFunctionName, memorySize);
}

int ni::ASMCodegen::internalCodegen(const ni::NFunctionCall &node,
                                    std::string &returnAddr) {
  std::string fnName = this->generateFunctionName(node.identifier);
  int argc = node.params.size();
  if (argc > REGS.size()) {
    argc = REGS.size();
  }
  for (int i = 0; i < argc; i++) {
    auto &param = *node.params[i].get();
    auto reg = REGS[i];
    std::string addr;
    int ret = this->internalCodegen(param, addr);
    if (ret != 0) {
      return ret;
    }
    *this->outputFile << "\tmovl\t" << addr << ", %" << reg << std::endl;
  }
  for (int i = (node.params.size() - 1); i >= 6; i--) {
    auto &param = *node.params[i].get();
    std::string addr;
    int ret = this->internalCodegen(param, addr);
    if (ret != 0) {
      return ret;
    }
    *this->outputFile << "\tpushq\t" << addr << std::endl;
  }

  *this->outputFile << "\tcallq\t" << fnName << std::endl;
  returnAddr = "%eax";
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NFunctionReturn &node,
                                    std::string &returnAddr) {
  std::string addr;
  int ret = this->internalCodegen(*node.value.get(), addr);
  if (ret != 0) {
    return ret;
  }

  if (addr.compare("%eax") != 0) {
    *this->outputFile << "\tmovl\t" << addr << ", %eax" << std::endl;
  }
  *this->outputFile << "\tjmp\t" << this->currentFunctionName << "_epilogue"
                    << std::endl;
  returnAddr = "%eax";
  return 0;
}

void ni::ASMCodegen::resetStackFrame() {
  this->currentStackPosition = 0;
  this->vars.clear();
}

int ni::ASMCodegen::generateTextSection() {
  if (this->targetOS == ni::OS::MACOS) {
    *this->outputFile << ".section\t__TEXT,__text" << std::endl;
  } else {
    *this->outputFile << ".text" << std::endl;
  }
  return 0;
}

int ni::ASMCodegen::generateExitCall() {
  std::string exitCode = "60";
  if (this->targetOS == ni::OS::MACOS) {
    exitCode = "0x2000001";
  }
  *this->outputFile << "\tmovl\t$" << exitCode << ", %eax" << std::endl;
  *this->outputFile << "\tsyscall" << std::endl;
  return 0;
}

std::string
ni::ASMCodegen::generateFunctionName(const std::string &name) const {
  if (this->targetOS == ni::OS::MACOS) {
    return "_" + name;
  }
  return name;
}

int ni::ASMCodegen::generateFunction(const std::string &name) {
  *this->outputFile << ".globl\t" << name << std::endl;
  *this->outputFile << name << ":" << std::endl;
  return 0;
}

int ni::ASMCodegen::generateFunctionPrologue(const int memorySize) {
  *this->outputFile << "\tpushq\t%rbp" << std::endl;
  *this->outputFile << "\tmovq\t%rsp, %rbp" << std::endl;
  *this->outputFile << "\tsubq\t$" << memorySize << ", %rsp" << std::endl;
  return 0;
}

int ni::ASMCodegen::generateFunctionEpilogue(const std::string &name,
                                             const int memorySize) {
  *this->outputFile << name << "_epilogue:" << std::endl;
  *this->outputFile << "\taddq\t$" << memorySize << ", %rsp" << std::endl;
  *this->outputFile << "\tpopq\t%rbp" << std::endl;
  *this->outputFile << "\tretq" << std::endl << std::endl;
  return 0;
}

int ni::ASMCodegen::internalCodegen(const ni::NProgram &node,
                                    std::string &returnAddr) {
  int ret = this->generateTextSection();
  if (ret != 0) {
    return ret;
  }
  for (const auto &it : node.instructions) {
    std::string addr;
    ret = this->internalCodegen(*it.get(), addr);
    if (ret != 0) {
      return ret;
    }
  }
  return 0;
}

int ni::ASMCodegen::codegen(const std::string &output, std::string &error) {
  this->outputFile = new std::ofstream();
  this->outputFile->open(output, std::ios::trunc);
  std::string addr;
  int ret = ni::ASMCodegen::internalCodegen(this->program, addr);
  this->outputFile->close();
  this->outputFile = nullptr;

  return ret;
}
