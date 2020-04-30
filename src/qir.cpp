#include <iostream>
#include <cstdio>
#include <cstdlib>
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/ADT/Optional.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "assembly.hpp"

int main(int argc, char **argv) {
    std::string buf;
    double result;
    parser::grammar<std::string::const_iterator> grammar;
    std::vector<ast::value> tree;
    int line = 0;
    if(argc >= 2 && split(argv[1], '.')[split(argv[1], '.').size() - 1] == "qir") {
        std::string filename = split(argv[1], '/')[split(argv[1], '/').size() - 1];
        auto const code = read_file(argv[1]);
        if(!qi::phrase_parse(code.begin(), code.end(), grammar, qi::space, tree)) {
            std::cout << argv[1] << ": " << line << "parse error" << std::endl;
            throw std::runtime_error("detected error");
        }
        
        LLVMContext context;
        std::unique_ptr<Module> module(new Module(filename, context));
        IRBuilder<> builder(context);

        make_function(context, module, builder, builder.getVoidTy(), "main");

        // std::vector<Type*> args = {builder.getInt8Ty()->getPointerTo()};
        // FunctionType* print_type = FunctionType::get(builder.getInt32Ty(), ArrayRef<Type*>(args), true);
        // FunctionCallee print_func = module->getOrInsertFunction("printf", print_type);
        // auto* format = builder.CreateGlobalStringPtr("%d\n");

        assembly asm_obj(module, builder);

        for(auto const& i : tree) {
            boost::apply_visitor(asm_obj, i);

            // std::vector<Value*> args = {
            //     format, boost::apply_visitor(asm_obj, i)
            // };
            // builder.CreateCall(print_func, ArrayRef<Value*>(args));
        }

        builder.CreateRetVoid();

        std::error_code errc;
        raw_fd_ostream stream("output.ll", errc, sys::fs::OpenFlags::F_None);
        module->print(stream, nullptr);

        InitializeAllTargetInfos();
        InitializeAllTargets();
        InitializeAllTargetMCs();
        InitializeAllAsmParsers();
        InitializeAllAsmPrinters();

        std::string targetTriple = sys::getDefaultTargetTriple();
        std::string err;
        auto target = TargetRegistry::lookupTarget(targetTriple, err);
        if(!target) {
            std::cerr << "Filed to look up target " << targetTriple << ": " << err;
            return 1;
        }

        TargetOptions opt;
        auto targetMachine = target->createTargetMachine(targetTriple, "generic", "", opt, Optional<Reloc::Model>());

        module->setDataLayout(targetMachine->createDataLayout());
        module->setTargetTriple(targetTriple);

        filename = split(filename, '.')[0];
        std::error_code err_code;
        raw_fd_ostream dest(filename + ".o", err_code, sys::fs::F_None);
        if(err_code) {
            std::cerr << "Couldn't open file: " << err_code.message();
            return 1;
        }

        legacy::PassManager pass;
        if(targetMachine->addPassesToEmitFile(pass, dest, nullptr, CodeGenFileType::CGFT_ObjectFile)) {
            std::cerr << "targetMachine can't emit a file of this type.\n";
            return 1;
        }

        pass.run(*module);
        dest.flush();

        char cmd[512] = "gcc ";
        for(int i = 2; i < argc; i++) {
            strcat(cmd, argv[i]);
            strcat(cmd, " ");
        }
        strcat(cmd, ("./" + filename +  + ".o").data());
        system(cmd);
        if(remove((filename + ".o").data())) {
            std::cerr << "process didn't complete successfully.\n";
            return 1;
        }
        return 0;
    }
}