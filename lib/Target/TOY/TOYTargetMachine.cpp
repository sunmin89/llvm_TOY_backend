//===-- TOYTargetMachine.cpp - Define TargetMachine for TOY -----------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

#include "TOYTargetMachine.h"
#include "TOY.h"
#include "TOYFrameLowering.h"
#include "TOYInstrInfo.h"
#include "TOYISelLowering.h"
#include "TOYSelectionDAGInfo.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/TargetLoweringObjectFileImpl.h"
#include "llvm/CodeGen/TargetPassConfig.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"

using namespace llvm;

static std::string computeDataLayout(const Triple &TT) {
    return  "";
}

static Reloc::Model getEffectiveRelocModel(const Triple &TT,
        Optional<Reloc::Model> RM) {
    if (!RM.hasValue())
        return Reloc::Static;
    return *RM;
}

static CodeModel::Model getEffectiveCodeModel(Optional<CodeModel::Model> CM) {
    if (CM)
        return *CM;
    return CodeModel::Small;
}


TOYTargetMachine::TOYTargetMachine(const Target &T, const Triple &TT, StringRef CPU,
        StringRef FS, const TargetOptions &Options,
        Optional<Reloc::Model> RM, Optional<CodeModel::Model> CM,
        CodeGenOpt::Level OL, bool JIT) : LLVMTargetMachine(T, 
            computeDataLayout(TT), TT, CPU, FS, Options, 
            getEffectiveRelocModel(TT, RM), 
            getEffectiveCodeModel(CM), OL),
    TLOF(make_unique<TargetLoweringObjectFileELF>()),
    Subtarget(TT, CPU, FS, *this) {
        initAsmInfo();
        // 调试输出
        llvm::errs() << "TOYTargetMachine constructor called\n";
        llvm::errs() << "Triple: " << TT.str() << "\n";
        llvm::errs() << "Vendor: " << TT.getVendorName() << "\n";
        if (TT.getVendor() == Triple::ToyVendor) {
            llvm::outs() << "hello toyvendor \n";
        }
    }

namespace {
    /// TOY Code Generator Pass Configuration Options.
    class TOYPassConfig : public TargetPassConfig {
        public:
            TOYPassConfig(TOYTargetMachine &TM, PassManagerBase &PM)
                : TargetPassConfig(TM, PM) {}

            TOYTargetMachine &getTOYTargetMachine() const {
                return getTM<TOYTargetMachine>();
            }

            bool addPreISel() override;
            bool addInstSelector() override;
            void addPreEmitPass() override;
    };
} // namespace

TargetPassConfig *TOYTargetMachine::createPassConfig(PassManagerBase &PM) {
    return new TOYPassConfig(*this, PM);
}

bool TOYPassConfig::addPreISel() { return false; }

bool TOYPassConfig::addInstSelector() {
    addPass(createTOYISelDag(getTOYTargetMachine(), getOptLevel()));
    return false;
}

void TOYPassConfig::addPreEmitPass() { return; }

// Force static initialization.
extern "C" void LLVMInitializeTOYTarget() {
    RegisterTargetMachine<TOYTargetMachine> X(getTheTOYTarget());
}

void TOYTargetMachine::addAnalysisPasses(PassManagerBase &PM) {}
