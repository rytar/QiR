#ifndef QIRASSEMBLY
#define QIRASSEMBLY

#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <memory>
#include "llvm-c/Core.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/SourceMgr.h"
#include "parser.hpp"

using namespace llvm;

class assembly : public boost::static_visitor<Value*> {
    
    LLVMContext& context;
    IRBuilder<>& builder;
    std::unordered_map<std::string, AllocaInst*> var_table;

    public:
    assembly(LLVMContext& context_, IRBuilder<>& builder_): boost::static_visitor<Value*>(), context(context_), builder(builder_) {}

    Value* operator() (int value) {
        // std::cout << "int: " << value << std::endl;
        return ConstantInt::get(builder.getInt32Ty(), value);
    }

    Value* operator() (bool value) {
        // std::cout << "bool: " << value << std::endl;
        return ConstantInt::get(builder.getInt1Ty(), value);
    }

    Value* operator() (const ast::var_ref& vr) {
        // std::cout << "var_ref: " << vr.id << std::endl;
        return builder.CreateLoad(var_table[vr.id], vr.id);
    }

    Value* operator() (const ast::assign& as) {
        // std::cout << "assign: " << as.id << std::endl;
        Value* val = boost::apply_visitor(*this, as.val);
        builder.CreateStore(val, var_table[as.id]);
        return val;
    }

    Value* operator() (const ast::ifstat& ifs) {
        // std::cout << "ifstat" << std::endl;
        Value* cond = boost::apply_visitor(*this, ifs.cond);
        Function* func = builder.GetInsertBlock()->getParent();
        BasicBlock* then_block = BasicBlock::Create(context, "then", func);
        BasicBlock* else_block = BasicBlock::Create(context, "else", func);
        BasicBlock* merge_block = BasicBlock::Create(context, "merge", func);
        builder.CreateCondBr(cond, then_block, else_block);
        builder.SetInsertPoint(then_block);
        // std::cout << "then_block start" << std::endl;
        if(ifs.then_stat.size() != 0) {
            for(const auto& i : ifs.then_stat) {
                boost::apply_visitor(*this, i);
            }
        }
        // std::cout << "then_block end" << std::endl;
        builder.CreateBr(merge_block);
        builder.SetInsertPoint(else_block);
        // std::cout << "else_block start" << std::endl;
        if(ifs.else_stat.size() != 0) {
            for(const auto& i : ifs.else_stat) {
                boost::apply_visitor(*this, i);
            }
        }
        // std::cout << "else_block end" << std::endl;
        builder.CreateBr(merge_block);
        builder.SetInsertPoint(merge_block);
        return 0;
    }

    template<typename Op>
    Value* operator() (const ast::binary_op<Op>& op) {
        // std::cout << "binary_op" << std::endl;
        Value* lhs = boost::apply_visitor(*this, op.lhs);
        Value* rhs = boost::apply_visitor(*this, op.rhs);

        return apply_op(op, lhs, rhs);
    }

    template<typename T>
    Value* operator() (const ast::vdec<T>& vdec) {
        Value* val = boost::apply_visitor(*this, vdec.val);
        return apply_vdec(vdec, val, vdec.id);
    }

    private:
    Value* apply_vdec(const ast::vdec<int>&, Value* val, std::string id) {
        // std::cout << "vdec<int>: " << id << std::endl;
        auto addr = builder.CreateAlloca(builder.getInt32Ty(), nullptr, id);
        builder.CreateStore(val, addr);
        var_table[id] = addr;
        return val;
    }

    Value* apply_vdec(const ast::vdec<bool>&, Value* val, std::string id) {
        // std::cout << "vdec<bool>: " << id << std::endl;
        auto addr = builder.CreateAlloca(builder.getInt1Ty(), nullptr, id);
        builder.CreateStore(val, addr);
        var_table[id] = addr;
        return val;
    }

    Value* apply_op(const ast::binary_op<ast::lt>&, Value* lhs, Value* rhs) {
        return builder.CreateICmpSLT(lhs, rhs);
    }

    Value* apply_op(const ast::binary_op<ast::lte>&, Value* lhs, Value* rhs) {
        return builder.CreateICmpSLE(lhs, rhs);
    }

    Value* apply_op(const ast::binary_op<ast::gt>&, Value* lhs, Value* rhs) {
        return builder.CreateICmpSGT(lhs, rhs);
    }

    Value* apply_op(const ast::binary_op<ast::gte>&, Value* lhs, Value* rhs) {
        return builder.CreateICmpSGE(lhs, rhs);
    }
    
    Value* apply_op(const ast::binary_op<ast::eql>&, Value* lhs, Value* rhs) {
        return builder.CreateICmpEQ(lhs, rhs);
    }
    
    Value* apply_op(const ast::binary_op<ast::neq>&, Value* lhs, Value* rhs) {
        return builder.CreateICmpNE(lhs, rhs);
    }
    
    Value* apply_op(const ast::binary_op<ast::add>&, Value* lhs, Value* rhs) {
        return builder.CreateAdd(lhs, rhs);
    }

    Value* apply_op(const ast::binary_op<ast::sub>&, Value* lhs, Value* rhs) {
        return builder.CreateSub(lhs, rhs);
    }

    Value* apply_op(const ast::binary_op<ast::mul>&, Value* lhs, Value* rhs) {
        return builder.CreateMul(lhs, rhs);
    }

    Value* apply_op(const ast::binary_op<ast::div>&, Value* lhs, Value* rhs) {
        return builder.CreateSDiv(lhs, rhs);
    }

    Value* apply_op(const ast::binary_op<ast::mod>&, Value* lhs, Value* rhs) {
        return builder.CreateSRem(lhs, rhs);
    }
};

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> elems;
    std::stringstream ss(s);
    std::string item;
    while(getline(ss, item, delim)) {
        if(!item.empty()) {
            elems.push_back(item);
        }
    }
    return elems;
}

std::string read_file(const char* filename) {
    std::ifstream ifs(filename, std::ios::in);
    if(ifs.fail()) {
        return {};
    }

    std::istreambuf_iterator<char> itr(ifs);
    std::istreambuf_iterator<char> last;
    std::string result(itr, last);

    return result;
}

void make_function(LLVMContext& context, std::unique_ptr<Module>& module, IRBuilder<>& builder, Type* type, ArrayRef<Type*> argsRef, std::string name) {
    auto* func = Function::Create(
        FunctionType::get(type, argsRef, false),
        Function::ExternalLinkage,
        name,
        module.get()
    );

    builder.SetInsertPoint(BasicBlock::Create(context, "entry", func));
}

void make_function(LLVMContext& context, std::unique_ptr<Module>& module, IRBuilder<>& builder, Type* type, std::string name) {
    auto* func = Function::Create(
        FunctionType::get(type, false),
        Function::ExternalLinkage,
        name,
        module.get()
    );

    builder.SetInsertPoint(BasicBlock::Create(context, "entry", func));
}

#endif