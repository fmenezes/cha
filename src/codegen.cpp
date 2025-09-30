#include "codegen.hpp"
#include "log.hpp"

#include <optional>
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/CodeGen/TargetPassConfig.h>



namespace cha {

CodeGenerator::CodeGenerator() 
    : context_(std::make_unique<llvm::LLVMContext>())
    , module_(std::make_unique<llvm::Module>("cha_module", *context_))
    , builder_(std::make_unique<llvm::IRBuilder<>>(*context_)) {
    
    // Initialize only the targets we have linked
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
}

int CodeGenerator::generate(const AstNodeList& ast, CompileFormat format, const std::string& output_file) {
    has_error_ = false;
    
    // Visit all top-level nodes
    for (const auto& node : ast) {
        visit_node(*node);
        if (has_error_) {
            return 1;
        }
    }
    
    // Create main wrapper if we don't have a main function
    if (functions_.find("main") == functions_.end()) {
        create_main_wrapper();
    }
    
    // Verify the module
    std::string error_str;
    llvm::raw_string_ostream error_stream(error_str);
    if (llvm::verifyModule(*module_, &error_stream)) {
        log_error("LLVM module verification failed: " + error_str);
        return 1;
    }
    
    // Write output
    return write_output(format, output_file);
}

void CodeGenerator::visit_node(const AstNode& node) {
    node.accept(*this);
}

llvm::Type* CodeGenerator::get_llvm_type(const AstType& type) {
    if (type.is_primitive()) {
        return primitive_to_llvm_type(type.as_primitive().type);
    } else if (type.is_array()) {
        llvm::Type* element_type = get_llvm_type(*type.as_array().element_type);
        if (!element_type) return nullptr;
        return llvm::ArrayType::get(element_type, type.as_array().size);
    } else if (type.is_identifier()) {
        // For now, treat identifiers as unknown - would need a symbol table for custom types
        log_error("Custom types not yet supported: " + type.as_identifier().name);
        return nullptr;
    }
    return nullptr;
}

llvm::Type* CodeGenerator::primitive_to_llvm_type(PrimitiveType prim_type) {
    switch (prim_type) {
        case PrimitiveType::BOOL:
            return llvm::Type::getInt1Ty(*context_);
        case PrimitiveType::INT8:
            return llvm::Type::getInt8Ty(*context_);
        case PrimitiveType::INT16:
            return llvm::Type::getInt16Ty(*context_);
        case PrimitiveType::INT32:
        case PrimitiveType::INT:
            return llvm::Type::getInt32Ty(*context_);
        case PrimitiveType::INT64:
        case PrimitiveType::CONST_INT:
            return llvm::Type::getInt64Ty(*context_);
        case PrimitiveType::UINT8:
            return llvm::Type::getInt8Ty(*context_);
        case PrimitiveType::UINT16:
            return llvm::Type::getInt16Ty(*context_);
        case PrimitiveType::UINT32:
        case PrimitiveType::UINT:
            return llvm::Type::getInt32Ty(*context_);
        case PrimitiveType::UINT64:
        case PrimitiveType::CONST_UINT:
            return llvm::Type::getInt64Ty(*context_);
        case PrimitiveType::FLOAT16:
            return llvm::Type::getHalfTy(*context_);
        case PrimitiveType::FLOAT32:
            return llvm::Type::getFloatTy(*context_);
        case PrimitiveType::FLOAT64:
        case PrimitiveType::CONST_FLOAT:
            return llvm::Type::getDoubleTy(*context_);
        case PrimitiveType::UNDEF:
            return llvm::Type::getVoidTy(*context_);
        default:
            return nullptr;
    }
}

int CodeGenerator::write_output(CompileFormat format, const std::string& output_file) {
    std::error_code ec;
    
    switch (format) {
        case CompileFormat::LLVM_IR: {
            llvm::raw_fd_ostream dest(output_file, ec, llvm::sys::fs::OF_None);
            if (ec) {
                log_error("Could not open file: " + ec.message());
                return 1;
            }
            module_->print(dest, nullptr);
            break;
        }
        
        case CompileFormat::ASSEMBLY_FILE:
        case CompileFormat::OBJECT_FILE:
        case CompileFormat::BINARY_FILE: {
            // Initialize target
            auto target_triple = llvm::sys::getDefaultTargetTriple();
            std::string error;
            auto target = llvm::TargetRegistry::lookupTarget(target_triple, error);
            
            if (!target) {
                log_error("Target lookup failed: " + error);
                return 1;
            }
            
            auto cpu = "generic";
            auto features = "";
            
            llvm::TargetOptions opt;
            std::optional<llvm::Reloc::Model> reloc_model;
            auto target_machine = target->createTargetMachine(
                llvm::Triple(target_triple), cpu, features, opt, reloc_model);
            
            module_->setDataLayout(target_machine->createDataLayout());
            module_->setTargetTriple(llvm::Triple(target_triple));
            
            llvm::raw_fd_ostream dest(output_file, ec, llvm::sys::fs::OF_None);
            if (ec) {
                log_error("Could not open file: " + ec.message());
                return 1;
            }
            
            llvm::legacy::PassManager pass;
            auto file_type = (format == CompileFormat::ASSEMBLY_FILE) ? 
                llvm::CodeGenFileType::AssemblyFile : llvm::CodeGenFileType::ObjectFile;
            
            if (target_machine->addPassesToEmitFile(pass, dest, nullptr, file_type)) {
                log_error("Target machine can't emit a file of this type");
                return 1;
            }
            
            pass.run(*module_);
            dest.flush();
            
            // For binary files, we'd need a linker step - not implemented for now
            if (format == CompileFormat::BINARY_FILE) {
                log_error("Binary file generation not yet implemented - produced object file instead");
            }
            break;
        }
        
        default:
            log_error("Unsupported output format");
            return 1;
    }
    
    return 0;
}

void CodeGenerator::create_main_wrapper() {
    // Create a simple main function that returns 0
    llvm::FunctionType* main_type = llvm::FunctionType::get(
        llvm::Type::getInt32Ty(*context_), false);
    
    llvm::Function* main_func = llvm::Function::Create(
        main_type, llvm::Function::ExternalLinkage, "main", module_.get());
    
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context_, "entry", main_func);
    builder_->SetInsertPoint(bb);
    
    // Return 0
    builder_->CreateRet(llvm::ConstantInt::get(*context_, llvm::APInt(32, 0)));
    
    functions_["main"] = main_func;
}

// Visitor implementations
void CodeGenerator::visit(const ConstantIntegerNode& node) {
    // Parse the string value to integer
    try {
        int64_t value = std::stoll(node.value());
        current_value_ = llvm::ConstantInt::get(*context_, llvm::APInt(32, value, true));
    } catch (const std::exception& e) {
        log_error("Invalid integer constant: " + node.value());
        has_error_ = true;
    }
}

void CodeGenerator::visit(const ConstantUnsignedIntegerNode& node) {
    // Parse the string value to unsigned integer
    try {
        uint64_t value = std::stoull(node.value());
        current_value_ = llvm::ConstantInt::get(*context_, llvm::APInt(64, value, false));
    } catch (const std::exception& e) {
        log_error("Invalid unsigned integer constant: " + node.value());
        has_error_ = true;
    }
}

void CodeGenerator::visit(const ConstantFloatNode& node) {
    current_value_ = llvm::ConstantFP::get(*context_, llvm::APFloat(node.value()));
}

void CodeGenerator::visit(const ConstantBoolNode& node) {
    current_value_ = llvm::ConstantInt::get(*context_, llvm::APInt(1, node.value() ? 1 : 0));
}

void CodeGenerator::visit(const BinaryOpNode& node) {
    // Generate code for left operand
    visit_node(node.left());
    if (has_error_) return;
    llvm::Value* left_val = current_value_;
    
    // Generate code for right operand
    visit_node(node.right());
    if (has_error_) return;
    llvm::Value* right_val = current_value_;
    
    if (!left_val || !right_val) {
        log_error("Invalid operands for binary operation");
        has_error_ = true;
        return;
    }
    
    // Generate appropriate LLVM instruction based on operator
    switch (node.op()) {
        case Operator::ADD:
            if (left_val->getType()->isIntegerTy()) {
                current_value_ = builder_->CreateAdd(left_val, right_val, "addtmp");
            } else {
                current_value_ = builder_->CreateFAdd(left_val, right_val, "addtmp");
            }
            break;
            
        case Operator::SUBTRACT:
            if (left_val->getType()->isIntegerTy()) {
                current_value_ = builder_->CreateSub(left_val, right_val, "subtmp");
            } else {
                current_value_ = builder_->CreateFSub(left_val, right_val, "subtmp");
            }
            break;
            
        case Operator::MULTIPLY:
            if (left_val->getType()->isIntegerTy()) {
                current_value_ = builder_->CreateMul(left_val, right_val, "multmp");
            } else {
                current_value_ = builder_->CreateFMul(left_val, right_val, "multmp");
            }
            break;
            
        case Operator::DIVIDE:
            if (left_val->getType()->isIntegerTy()) {
                current_value_ = builder_->CreateSDiv(left_val, right_val, "divtmp");
            } else {
                current_value_ = builder_->CreateFDiv(left_val, right_val, "divtmp");
            }
            break;
            
        case Operator::EQUALS_EQUALS:
            if (left_val->getType()->isIntegerTy()) {
                current_value_ = builder_->CreateICmpEQ(left_val, right_val, "cmptmp");
            } else {
                current_value_ = builder_->CreateFCmpOEQ(left_val, right_val, "cmptmp");
            }
            break;
            
        case Operator::NOT_EQUALS:
            if (left_val->getType()->isIntegerTy()) {
                current_value_ = builder_->CreateICmpNE(left_val, right_val, "cmptmp");
            } else {
                current_value_ = builder_->CreateFCmpONE(left_val, right_val, "cmptmp");
            }
            break;
            
        case Operator::LESS_THAN:
            if (left_val->getType()->isIntegerTy()) {
                current_value_ = builder_->CreateICmpSLT(left_val, right_val, "cmptmp");
            } else {
                current_value_ = builder_->CreateFCmpOLT(left_val, right_val, "cmptmp");
            }
            break;
            
        case Operator::LESS_THAN_OR_EQUALS:
            if (left_val->getType()->isIntegerTy()) {
                current_value_ = builder_->CreateICmpSLE(left_val, right_val, "cmptmp");
            } else {
                current_value_ = builder_->CreateFCmpOLE(left_val, right_val, "cmptmp");
            }
            break;
            
        case Operator::GREATER_THAN:
            if (left_val->getType()->isIntegerTy()) {
                current_value_ = builder_->CreateICmpSGT(left_val, right_val, "cmptmp");
            } else {
                current_value_ = builder_->CreateFCmpOGT(left_val, right_val, "cmptmp");
            }
            break;
            
        case Operator::GREATER_THAN_OR_EQUALS:
            if (left_val->getType()->isIntegerTy()) {
                current_value_ = builder_->CreateICmpSGE(left_val, right_val, "cmptmp");
            } else {
                current_value_ = builder_->CreateFCmpOGE(left_val, right_val, "cmptmp");
            }
            break;
            
        case Operator::AND:
            current_value_ = builder_->CreateAnd(left_val, right_val, "andtmp");
            break;
            
        case Operator::OR:
            current_value_ = builder_->CreateOr(left_val, right_val, "ortmp");
            break;
            
        default:
            log_error("Unsupported binary operator");
            has_error_ = true;
            return;
    }
}

void CodeGenerator::visit(const VariableDeclarationNode& node) {
    // Get LLVM type for the variable
    llvm::Type* var_type = get_llvm_type(node.type());
    if (!var_type) {
        log_error("Unsupported type for variable: " + node.identifier());
        has_error_ = true;
        return;
    }
    
    // Create alloca instruction for the variable
    llvm::AllocaInst* alloca = builder_->CreateAlloca(var_type, nullptr, node.identifier());
    
    // Store initial value if provided
    if (node.value()) {
        visit_node(*node.value());
        if (has_error_) return;
        
        if (!current_value_) {
            log_error("Failed to generate initial value for variable: " + node.identifier());
            has_error_ = true;
            return;
        }
        
        builder_->CreateStore(current_value_, alloca);
    }
    
    // Store in symbol table
    named_values_[node.identifier()] = alloca;
    current_value_ = alloca;
}

void CodeGenerator::visit(const VariableAssignmentNode& node) {
    // Look up the variable
    auto it = named_values_.find(node.identifier());
    if (it == named_values_.end()) {
        log_error("Unknown variable name: " + node.identifier());
        has_error_ = true;
        return;
    }
    
    // Generate code for the value
    visit_node(node.value());
    if (has_error_) return;
    
    if (!current_value_) {
        log_error("Failed to generate value for assignment");
        has_error_ = true;
        return;
    }
    
    // Store the value
    builder_->CreateStore(current_value_, it->second);
}

void CodeGenerator::visit(const VariableLookupNode& node) {
    // Look up the variable
    auto it = named_values_.find(node.identifier());
    if (it == named_values_.end()) {
        log_error("Unknown variable name: " + node.identifier());
        has_error_ = true;
        return;
    }
    
    // Load the value - cast to AllocaInst to get allocated type
    llvm::AllocaInst* alloca = llvm::cast<llvm::AllocaInst>(it->second);
    current_value_ = builder_->CreateLoad(alloca->getAllocatedType(), it->second, node.identifier());
}

void CodeGenerator::visit(const ArgumentNode& node) {
    // Arguments are handled in function declaration
    // This visitor is called when we need the argument type
    llvm::Type* arg_type = get_llvm_type(node.type());
    if (!arg_type) {
        log_error("Unsupported argument type: " + node.identifier());
        has_error_ = true;
    }
}

void CodeGenerator::visit(const BlockNode& node) {
    // Visit all statements in the block
    for (const auto& stmt : node.statements()) {
        visit_node(*stmt);
        if (has_error_) return;
    }
}

void CodeGenerator::visit(const FunctionDeclarationNode& node) {
    // Get return type
    llvm::Type* return_type = get_llvm_type(node.return_type());
    if (!return_type) {
        // Check if this is a void function (no explicit return type)
        if (node.return_type().is_primitive() && 
            node.return_type().as_primitive().type == PrimitiveType::UNDEF) {
            return_type = llvm::Type::getVoidTy(*context_);
        } else {
            log_error("Unsupported return type for function: " + node.identifier());
            has_error_ = true;
            return;
        }
    }
    
    // Get argument types
    std::vector<llvm::Type*> arg_types;
    for (const auto& arg : node.arguments()) {
        const ArgumentNode* arg_node = dynamic_cast<const ArgumentNode*>(arg.get());
        if (!arg_node) {
            log_error("Invalid argument in function: " + node.identifier());
            has_error_ = true;
            return;
        }
        
        llvm::Type* arg_type = get_llvm_type(arg_node->type());
        if (!arg_type) {
            log_error("Unsupported argument type in function: " + node.identifier());
            has_error_ = true;
            return;
        }
        arg_types.push_back(arg_type);
    }
    
    // Create function type
    llvm::FunctionType* func_type = llvm::FunctionType::get(return_type, arg_types, false);
    
    // Create function
    llvm::Function* function = llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, 
                                                     node.identifier(), module_.get());
    
    // Set argument names
    unsigned idx = 0;
    for (auto& arg : function->args()) {
        const ArgumentNode* arg_node = dynamic_cast<const ArgumentNode*>(node.arguments()[idx].get());
        arg.setName(arg_node->identifier());
        idx++;
    }
    
    // Create basic block
    llvm::BasicBlock* bb = llvm::BasicBlock::Create(*context_, "entry", function);
    builder_->SetInsertPoint(bb);
    
    // Save current state
    llvm::Function* prev_function = current_function_;
    std::map<std::string, llvm::Value*> prev_named_values = named_values_;
    
    current_function_ = function;
    named_values_.clear();
    
    // Create allocas for arguments
    idx = 0;
    for (auto& arg : function->args()) {
        const ArgumentNode* arg_node = dynamic_cast<const ArgumentNode*>(node.arguments()[idx].get());
        llvm::AllocaInst* alloca = builder_->CreateAlloca(arg.getType(), nullptr, arg_node->identifier());
        builder_->CreateStore(&arg, alloca);
        named_values_[arg_node->identifier()] = alloca;
        idx++;
    }
    
    // Generate function body
    for (const auto& stmt : node.body()) {
        visit_node(*stmt);
        if (has_error_) {
            // Restore state
            current_function_ = prev_function;
            named_values_ = prev_named_values;
            return;
        }
    }
    
    // If no explicit return, add default return
    if (!builder_->GetInsertBlock()->getTerminator()) {
        if (return_type->isVoidTy()) {
            builder_->CreateRetVoid();
        } else {
            // Return zero/null for non-void functions without explicit return
            builder_->CreateRet(llvm::Constant::getNullValue(return_type));
        }
    }
    
    // Restore state
    current_function_ = prev_function;
    named_values_ = prev_named_values;
    
    // Store function in symbol table
    functions_[node.identifier()] = function;
    current_value_ = function;
}

void CodeGenerator::visit(const FunctionCallNode& node) {
    // Look up the function
    auto it = functions_.find(node.identifier());
    if (it == functions_.end()) {
        log_error("Unknown function: " + node.identifier());
        has_error_ = true;
        return;
    }
    
    llvm::Function* callee = it->second;
    
    // Check argument count mismatch
    if (callee->arg_size() != node.arguments().size()) {
        log_error("Incorrect number of arguments for function: " + node.identifier());
        has_error_ = true;
        return;
    }
    
    // Generate code for arguments
    std::vector<llvm::Value*> args;
    for (const auto& arg : node.arguments()) {
        visit_node(*arg);
        if (has_error_) return;
        
        if (!current_value_) {
            log_error("Failed to generate argument for function call: " + node.identifier());
            has_error_ = true;
            return;
        }
        
        args.push_back(current_value_);
    }
    
    // Create function call
    current_value_ = builder_->CreateCall(callee, args, "calltmp");
}

void CodeGenerator::visit(const FunctionReturnNode& node) {
    if (node.value()) {
        // Generate code for return value
        visit_node(*node.value());
        if (has_error_) return;
        
        if (!current_value_) {
            log_error("Failed to generate return value");
            has_error_ = true;
            return;
        }
        
        builder_->CreateRet(current_value_);
    } else {
        // Void return
        builder_->CreateRetVoid();
    }
}

void CodeGenerator::visit(const IfNode& node) {
    // Generate condition
    visit_node(node.condition());
    if (has_error_) return;
    
    if (!current_value_) {
        log_error("Failed to generate condition for if statement");
        has_error_ = true;
        return;
    }
    
    // Convert condition to boolean if necessary
    llvm::Value* cond_val = current_value_;
    if (!cond_val->getType()->isIntegerTy(1)) {
        // Convert to i1 (boolean)
        if (cond_val->getType()->isIntegerTy()) {
            cond_val = builder_->CreateICmpNE(cond_val, 
                llvm::ConstantInt::get(cond_val->getType(), 0), "ifcond");
        } else if (cond_val->getType()->isFloatingPointTy()) {
            cond_val = builder_->CreateFCmpONE(cond_val, 
                llvm::ConstantFP::get(cond_val->getType(), 0.0), "ifcond");
        } else {
            log_error("Invalid condition type for if statement");
            has_error_ = true;
            return;
        }
    }
    
    // Create basic blocks
    llvm::Function* function = builder_->GetInsertBlock()->getParent();
    llvm::BasicBlock* then_bb = llvm::BasicBlock::Create(*context_, "then", function);
    llvm::BasicBlock* else_bb = node.has_else() ? 
        llvm::BasicBlock::Create(*context_, "else") : nullptr;
    llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(*context_, "ifcont");
    
    // Create conditional branch
    if (node.has_else()) {
        builder_->CreateCondBr(cond_val, then_bb, else_bb);
    } else {
        builder_->CreateCondBr(cond_val, then_bb, merge_bb);
    }
    
    // Generate then block
    builder_->SetInsertPoint(then_bb);
    for (const auto& stmt : node.then_block()) {
        visit_node(*stmt);
        if (has_error_) return;
    }
    
    // Add branch to merge block if no terminator
    if (!builder_->GetInsertBlock()->getTerminator()) {
        builder_->CreateBr(merge_bb);
    }
    
    // Generate else block if present
    if (node.has_else()) {
        function->insert(function->end(), else_bb);
        builder_->SetInsertPoint(else_bb);
        
        for (const auto& stmt : node.else_block()) {
            visit_node(*stmt);
            if (has_error_) return;
        }
        
        // Add branch to merge block if no terminator
        if (!builder_->GetInsertBlock()->getTerminator()) {
            builder_->CreateBr(merge_bb);
        }
    }
    
    // Continue with merge block
    function->insert(function->end(), merge_bb);
    builder_->SetInsertPoint(merge_bb);
}

void CodeGenerator::visit(const ConstantDeclarationNode& node) {
    // Generate code for the constant value
    visit_node(node.value());
    if (has_error_) return;
    
    if (!current_value_) {
        log_error("Failed to generate constant value: " + node.identifier());
        has_error_ = true;
        return;
    }
    
    // For constants, we can store the computed value directly
    // Create a global constant
    llvm::Constant* const_val = llvm::dyn_cast<llvm::Constant>(current_value_);
    if (!const_val) {
        log_error("Constant declaration requires a constant value: " + node.identifier());
        has_error_ = true;
        return;
    }
    
    // Create global variable for the constant
    llvm::GlobalVariable* global_const = new llvm::GlobalVariable(
        *module_, const_val->getType(), true, 
        llvm::GlobalValue::InternalLinkage, const_val, node.identifier());
    
    named_values_[node.identifier()] = global_const;
}

int generate_code(const AstNodeList& ast, CompileFormat format, const std::string& output_file) {
    CodeGenerator generator;
    return generator.generate(ast, format, output_file);
}

} // namespace cha
