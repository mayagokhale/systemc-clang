// clang-format off
#include "HDLThread.h"
//clang-format on

/// Different matchers may use different DEBUG_TYPE
#undef DEBUG_TYPE
#define DEBUG_TYPE "HDL"

//!
//! Entry point for SC_THREAD  generation
//!
//! Starting with outer compound statement,
//! generate CFG and then 
//! generate hcode for basic block,
//! using HDLBody class
//! 

namespace systemc_hdl {

  HDLThread::HDLThread(CXXMethodDecl *emd, hNodep &h_top,
                   clang::DiagnosticsEngine &diag_engine, const ASTContext &ast_context, hdecl_name_map_t &mod_vname_map )
    : diag_e{diag_engine}, ast_context_{ast_context} {
      LLVM_DEBUG(llvm::dbgs() << "Entering HDLThread constructor (thread body)\n");
      h_ret = NULL;

      // get CFG

      static std::unique_ptr< CFG > threadcfg = clang::CFG::buildCFG(emd, emd->getBody(), &(emd->getASTContext()), clang::CFG::BuildOptions());
      clang::LangOptions LO = ast_context.getLangOpts();
      threadcfg->dump(LO, false);
      for (CFG::const_iterator I = threadcfg->begin(), E = threadcfg->end(); I != E; ++I ) {
	unsigned j = 1;
	string blkid =  std::to_string((*I)->getBlockID());
	LLVM_DEBUG(llvm::dbgs() << "Block ID " << blkid << " size " << (*I)->size() <<"\n");
	LLVM_DEBUG(llvm::dbgs() << "Valid Terminator is " << (*I)->getTerminator().isValid() << "\n");
	hNodep h_body =  new hNode("B"+blkid, hNode::hdlopsEnum::hMethod);
	for (CFGBlock::const_iterator BI = (*I)->begin(), BEnd = (*I)->end() ;
	     BI != BEnd; ++BI, ++j ) {
	  LLVM_DEBUG(llvm::dbgs() << "Stmt kind " << BI->getKind() << "\n");
	  // from StmtPrinterHelper in CFG.cpp
	  if (Optional<CFGStmt> SE = BI->getAs<CFGStmt>()) {
	    const Stmt *stmt= SE->getStmt();
	    LLVM_DEBUG(llvm::dbgs() << "Stmt follows\n");
	    LLVM_DEBUG(stmt->dump(llvm::dbgs(), ast_context_));
	    HDLBody xmethod(const_cast<Stmt *>(stmt), h_body, diag_engine, ast_context_, mod_vname_map);
	    methodecls.insertall(xmethod.methodecls);
	  }
	  else LLVM_DEBUG(llvm::dbgs() << "Block element not stmt\n");
	}
	h_top->child_list.push_back(h_body);
      }
    }

  HDLThread::~HDLThread() {
    LLVM_DEBUG(llvm::dbgs() << "[[ Destructor HDLThread ]]\n");
  }
}
