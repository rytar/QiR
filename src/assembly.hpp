#ifndef QIRASSEMBLY
#define QIRASSEMBLY

#include <fstream>
#include <sstream>
#include <vector>
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
    Value* operator() (ast::binary_op<Op> const& op) {
        Value* lhs = boost::apply_visitor(*this, op.lhs);
        Value* rhs = boost::apply_visitor(*this, op.rhs);

        return apply_op(op, lhs, rhs);
    }

    private:
    Value* apply_op(ast::binary_op<ast::lt> const&, Value* lhs, Value* rhs) {
        return builder.CreateICmpSLT(lhs, rhs);
    }
    Value* apply_op(ast::binary_op<ast::lte> const&, Value* lhs, Value* rhs) {
        return builder.CreateICmpSLE(lhs, rhs);
    }
    Value* apply_op(ast::binary_op<ast::gt> const&, Value* lhs, Value* rhs) {
        return builder.CreateICmpSGT(lhs, rhs);
    }
    Value* apply_op(ast::binary_op<ast::gte> const&, Value* lhs, Value* rhs) {
        return builder.CreateICmpSGE(lhs, rhs);
    }
    Value* apply_op(ast::binary_op<ast::eql> const&, Value* lhs, Value* rhs) {
        return builder.CreateICmpEQ(lhs, rhs);
    }
    Value* apply_op(ast::binary_op<ast::neq> const&, Value* lhs, Value* rhs) {
        return builder.CreateICmpNE(lhs, rhs);
    }
    Value* apply_op(ast::binary_op<ast::add> const&, Value* lhs, Value* rhs) {
        return builder.CreateAdd(lhs, rhs);
    }

    Value* apply_op(ast::binary_op<ast::sub> const&, Value* lhs, Value* rhs) {
        return builder.CreateSub(lhs, rhs);
    }

    Value* apply_op(ast::binary_op<ast::mul> const&, Value* lhs, Value* rhs) {
        return builder.CreateMul(lhs, rhs);
    }

    Value* apply_op(ast::binary_op<ast::div> const&, Value* lhs, Value* rhs) {
        return builder.CreateSDiv(lhs, rhs);
    }

    Value* apply_op(ast::binary_op<ast::mod> const&, Value* lhs, Value* rhs) {
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

std::string read_file(char const* filename) {
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