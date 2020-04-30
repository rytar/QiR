#ifndef QIRASSEMBLY
#define QIRASSEMBLY

#include <fstream>
#include <sstream>
#include <string>
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/SourceMgr.h"
#include "parser.hpp"

using namespace llvm;

class assembly : public boost::static_visitor<Value*> {
    
    IRBuilder<>& builder;

    public:
    assembly(IRBuilder<>& builder_): boost::static_visitor<Value*>(), builder(builder_) {}

    Value* operator() (int value) {
        return ConstantInt::get(builder.getInt32Ty(), value);
    }

    template<typename Op>
    Value* operator() (const ast::binary_op<Op>& op) {
        Value* lhs = boost::apply_visitor(*this, op.lhs);
        Value* rhs = boost::apply_visitor(*this, op.rhs);

        return apply_op(op, lhs, rhs);
    }

    Value* operator() (const ast::vdec& vdec) {
        std::string type_ = vdec.type;
        std::string name = vdec.id;
        Value* val = boost::apply_visitor(*this, vdec.val);
        Type* type;
        if(type_ == "int") {
            type = builder.getInt32Ty();
        }
        auto addr = builder.CreateAlloca(type, nullptr, name);
        return builder.CreateStore(val, addr);
    }

    private:
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