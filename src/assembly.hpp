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
    
    IRBuilder<>& builder;

    std::unordered_map<std::string, AllocaInst*> var_table;

    public:
    assembly(std::unique_ptr<Module>& module, IRBuilder<>& builder_): boost::static_visitor<Value*>(), builder(builder_) {}

    Value* operator() (int value) {
        return ConstantInt::get(builder.getInt32Ty(), value);
    }

    Value* operator() (bool value) {
        return ConstantInt::get(builder.getInt1Ty(), value);
    }

    Value* operator() (const ast::var_ref& vr) {
        return builder.CreateLoad(var_table[vr.id], vr.id);
    }

    Value* operator() (const ast::assign& as) {
        Value* val = boost::apply_visitor(*this, as.val);
        builder.CreateStore(val, var_table[as.id]);
        return val;
    }

    template<typename Op>
    Value* operator() (const ast::binary_op<Op>& op) {
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
        auto addr = builder.CreateAlloca(builder.getInt32Ty(), nullptr, id);
        builder.CreateStore(val, addr);
        var_table[id] = addr;
        return val;
    }

    Value* apply_vdec(const ast::vdec<bool>&, Value* val, std::string id) {
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

    builder.SetInsertPoint(BasicBlock::Create(context, "", func));
}

void make_function(LLVMContext& context, std::unique_ptr<Module>& module, IRBuilder<>& builder, Type* type, std::string name) {
    auto* func = Function::Create(
        FunctionType::get(type, false),
        Function::ExternalLinkage,
        name,
        module.get()
    );

    builder.SetInsertPoint(BasicBlock::Create(context, "", func));
}

#endif