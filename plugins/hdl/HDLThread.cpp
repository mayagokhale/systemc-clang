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


using namespace systemc_clang;

namespace systemc_hdl {

  HDLThread::HDLThread(CXXMethodDecl *emd, hNodep &h_top,
		       clang::DiagnosticsEngine &diag_engine, const ASTContext &ast_context, hdecl_name_map_t &mod_vname_map )
    : h_top_{h_top}, diag_e{diag_engine}, ast_context_{ast_context}, mod_vname_map_{mod_vname_map} {
      LLVM_DEBUG(llvm::dbgs() << "Entering HDLThread constructor (thread body)\n");
      h_ret = NULL;

      thread_vname_map.insertall(mod_vname_map_);
      //xtbodyp = new HDLBody(diag_e, ast_context_, mod_vname_map_);
      xtbodyp = new HDLBody(diag_e, ast_context_, thread_vname_map);
      hthreadblocksp = new hNode(hNode::hdlopsEnum::hSwitchStmt); // body is switch, each path is case alternative
      hlocalvarsp = new hNode(hNode::hdlopsEnum::hPortsigvarlist); // placeholder to collect local vars
      
      // build SC CFG
      SplitCFG scfg{const_cast<ASTContext &>(ast_context_), emd};
      //scfg.split_wait_blocks(emd);
      // scfg.build_sccfg( method );
      scfg.generate_paths();
      scfg.dump();
      LLVM_DEBUG(scfg.dumpToDot());
	

      const llvm::SmallVectorImpl<SplitCFG::VectorSplitCFGBlock> &paths_found{ scfg.getPathsFound()};
      LLVM_DEBUG(llvm::dbgs() << "Dump sc cfg from hdlthread\n");
      unsigned int id{0};
      for (auto const& pt: paths_found) {
	LLVM_DEBUG(llvm::dbgs() << "S" << id << ": ");
	for (auto const& block : pt) {
	  LLVM_DEBUG(llvm::dbgs() << block->getBlockID() << "  ");
	}
	LLVM_DEBUG(llvm::dbgs() << "\n");
	++id;
      }
      int state_num = 0;
      for (auto const& pt: paths_found) {

	hNodep h_switchcase = new hNode(std::to_string(state_num), hNode::hdlopsEnum::hSwitchCase);
	ProcessSplitGraphBlock(pt[0], state_num, h_switchcase);
	hthreadblocksp->child_list.push_back(h_switchcase);
	state_num++;
      }
      
      //std::unique_ptr< CFG > threadcfg = clang::CFG::buildCFG(emd, emd->getBody(), &(emd->getASTContext()), clang::CFG::BuildOptions());
      clang::LangOptions LO = ast_context.getLangOpts();
      //threadcfg->dump(LO, false);
      // HDLBody instance init
      // for (auto const& pt: paths_found) {
      // 	for (auto const& block : pt) {
      // 	  ProcessBB(*(block->getCFGBlock()));
      // 	}
      // }
      h_top->child_list.push_back(hlocalvarsp);
      h_top->child_list.push_back(hthreadblocksp);
      
    }

  HDLThread::~HDLThread() {
    LLVM_DEBUG(llvm::dbgs() << "[[ Destructor HDLThread ]]\n");
  }

  bool HDLThread::IsWaitStmt(hNodep hp) {
    return ((hp->child_list.size() >=1) and ((hp->child_list.back())->getopc() == hNode::hdlopsEnum::hWait));
  }

  void HDLThread::CheckVardecls(hNodep &hp, unsigned int cfgblockid) {
    int varcnt = 0;
    for (auto oneop : hp->child_list) {
      if ((oneop->getopc() == hNode::hdlopsEnum::hVardecl) || (oneop->getopc() == hNode::hdlopsEnum::hSigdecl)) {
	LLVM_DEBUG(llvm::dbgs() << "Detected vardecl for CFG Block ID " << cfgblockid << "\n");
	if (CFGVisited[cfgblockid]==1) hlocalvarsp->child_list.push_back(oneop);
	varcnt += 1;
      }
      else break; // all vardecls are first in the list of ops
    }
    if (varcnt >=1) {
      hp->child_list.erase(hp->child_list.begin(), hp->child_list.begin()+varcnt);
      if (CFGVisited[cfgblockid]==1) thread_vname_map.insertall(xtbodyp->vname_map);
    }
  }

  void HDLThread::ProcessDeclStmt(const DeclStmt *declstmt, hNodep htmp) {
    //!
    //! called when a CFG declstmt is instantiated more than once
    //! can skip the decl, but need to process initializer
    //!
    
    // adapted from ProcessVarDecl in HDLBody.cpp
    for (auto *DI : declstmt->decls()) {
      if (DI) {
	auto *vardecl = dyn_cast<VarDecl>(DI);
	if (!vardecl) continue;
	if ( Expr *declinit = vardecl->getInit()) {
	   Stmt * cdeclinit = declinit;
	  //  need to generated initializer code
	  //xtbodyp->Run(const_cast<Stmt *>((const Stmt *)declinit), hinitcode, rthread);
	  hNodep varinitp = new hNode(hNode::hdlopsEnum::hVarAssign);
	  varinitp->child_list.push_back(new hNode(thread_vname_map.find_entry_newn(vardecl), hNode::hdlopsEnum::hVarref));
	  xtbodyp->Run(cdeclinit, varinitp, rthread);
	  htmp->child_list.push_back(varinitp);
	  
	}
      }
    }
  }
	  
  void HDLThread::MarkStatements(const Stmt *S, llvm::SmallDenseMap<const Stmt*, bool> &Map) {
    if (S != NULL) {
      Map[S] = true;
      for (const Stmt *K : S->children())
	MarkStatements(K, Map);
    }
  }

  // this version is no longer being used
  void HDLThread::FindStatements(const CFGBlock &B, std::vector<const Stmt *> &SS) {
    llvm::SmallDenseMap<const Stmt*, bool> Map;

    // Mark subexpressions of each element in the block.
    for (auto I = B.begin(); I != B.end(); ++I) {
      CFGElement E = *I;
      if (auto SE = E.getAs<CFGStmt>()) {
	const Stmt *S = SE->getStmt();
	for (const Stmt *K : S->children()) 
	  MarkStatements(K, Map);
      }
    }
    // mark subexpressions coming from terminator statement
    if (B.getTerminator().isValid()) {
      const Stmt *S = B.getTerminatorStmt();
      for (const Stmt *K : S->children())
	MarkStatements(K, Map);
    }
    // Any expressions not in Map are top level statements.
    for (auto I = B.begin(); I != B.end(); ++I) {
      CFGElement E = *I;
      if (auto SE = E.getAs<CFGStmt>()) {
	const Stmt *S = SE->getStmt();
	if (Map.find(S) == Map.end()) {
	  SS.push_back(S);
	}
      }
    }
  }

    void HDLThread::FindStatements(const SplitCFGBlock *B, std::vector<const Stmt *> &SS) {
    llvm::SmallDenseMap<const Stmt*, bool> Map;

    // Mark subexpressions of each element in the block.
    for (auto I : B->getElements()) {
      CFGElement E = *I;
      if (auto SE = E.getAs<CFGStmt>()) {
	const Stmt *S = SE->getStmt();
	for (const Stmt *K : S->children()) 
	  MarkStatements(K, Map);
      }
    }
    // // mark subexpressions coming from terminator statement
    if ((B->getCFGBlock())->getTerminator().isValid()) {
      const Stmt *S = (B->getCFGBlock())->getTerminatorStmt();
      for (const Stmt *K : S->children()) {
     	MarkStatements(K, Map);
      }
      //if (auto S1 = dyn_cast<WhileStmt>(S) 
      // if (Map.find(S) == Map.end()) {
      // 	  SS.push_back(S);
      // }
      LLVM_DEBUG(llvm::dbgs() << "Stmt contains terminator\n");
      //LLVM_DEBUG(S->dump(llvm::dbgs(), ast_context_));
    }
    
    // Any expressions not in Map are top level statements.
    for (auto I : B->getElements()) {
      CFGElement E = *I;
      if (auto SE = E.getAs<CFGStmt>()) {
	const Stmt *S = SE->getStmt();
	if (Map.find(S) == Map.end()) {
	  SS.push_back(S);
	}
      }
    }
  }
  
  void HDLThread::ProcessSplitGraphBlock(const SplitCFGBlock *sgb, int state_num, hNodep h_switchcase) {
    bool iswait = false;

    if (sgb != NULL) {
      string blkid = "S" + std::to_string(state_num) + "_" + std::to_string(sgb->getBlockID());

      if (SGVisited.find(blkid) == SGVisited.end()) {
	SGVisited[blkid] = true;
	CFGVisited[(sgb->getCFGBlock())->getBlockID()]+= 1;
      }
      else return; // already visited this block
      
      LLVM_DEBUG(llvm::dbgs() << "Split Graph num ele, blockid are " << sgb->getNumOfElements() << " " << blkid << "\n");

      if ((sgb->getCFGBlock())->getTerminator().isValid()) {
	hNodep hcondstmt = new hNode(hNode::hdlopsEnum::hIfStmt);
	auto spgsucc = sgb->getSuccessors();
	const Stmt * S = sgb->getCFGBlock()->getTerminatorStmt();

	if (const WhileStmt *S1 = dyn_cast<WhileStmt> (S)) {
	  LLVM_DEBUG(llvm::dbgs() << "Terminator for block " <<blkid << " is a while stmt\n");
	  xtbodyp->Run((Stmt *)S1->getCond(), hcondstmt, rthread);
	}
	else if (const ForStmt *S1 = dyn_cast<ForStmt> (S)) {
	  LLVM_DEBUG(llvm::dbgs() << "Terminator for block " << blkid << " is a for stmt\n");
	  xtbodyp->Run((Stmt *)S1->getCond(), hcondstmt, rthread);
	}
	else if (const IfStmt *S1 = dyn_cast<IfStmt> (S)) {
	  LLVM_DEBUG(llvm::dbgs() << "Terminator for block " << blkid << " is an if stmt\n");
	  xtbodyp->Run((Stmt *)S1->getCond(), hcondstmt, rthread);
	}
	else {
	  LLVM_DEBUG(llvm::dbgs() << "Terminator for block " << blkid << " is not handled\n");
	}

	// for each successor need to make a compound statement

	for (auto succ: spgsucc) {
	  hNodep hcstmt = new hNode(hNode::hdlopsEnum::hCStmt);
	  ProcessSplitGraphBlock(succ, state_num, hcstmt);
	  hcondstmt->child_list.push_back(hcstmt);
	  
	}
	h_switchcase->child_list.push_back(hcondstmt);
	return;
      }

      if (sgb->getNumOfElements() > 0) {
	
	// from http://clang-developers.42468.n3.nabble.com/Visiting-statements-of-a-CFG-one-time-td4069440.html#a4069447
	// had to add recursive traversal of AST node children
	std::vector<const Stmt *> SS;
	FindStatements(sgb, SS);
	hNodep htmp = new hNode(h_top_->getname(), hNode::hdlopsEnum::hNoop); 
	//for (auto E : sgb->getElements()) {
	//if (auto SE = E->getAs<CFGStmt>()) {
	for (auto S : SS) {
	  //const Stmt *S = SE->getStmt();
	  if (sgb->hasWait()) iswait = true;
	  LLVM_DEBUG(llvm::dbgs() << "Split Graph Stmt follows\n");
	  LLVM_DEBUG(S->dump(llvm::dbgs(), ast_context_));
	  
	  htmp->child_list.clear();

	  // Check if this CFG block has already been generated
	  // if so, skip the var decls. they were done on the
	  // first instantiation
	  // However, still need to generate code for their initializers
	  const DeclStmt *declstmt = dyn_cast<DeclStmt>(S);
	  if ((declstmt!=NULL) &&  (CFGVisited[(sgb->getCFGBlock()->getBlockID()) > 1]))
	    ProcessDeclStmt(declstmt, htmp);
	  else xtbodyp->Run(const_cast<Stmt *>(S), htmp, rthread); // no initializer, so normal

	  LLVM_DEBUG(llvm::dbgs() << "after Run, htmp follows\n");
	  htmp->dumphcode();
	  CheckVardecls(htmp, (sgb->getCFGBlock())->getBlockID());
	  if (IsWaitStmt(htmp)) {
	    htmp->child_list.back()->set(std::to_string(sgb->getNextState()));
	  }
	  if (htmp->child_list.size() >0) 
	    h_switchcase->child_list.insert(h_switchcase->child_list.end(), htmp->child_list.begin(), htmp->child_list.end());
	    
	  //htmp->child_list.clear();
	    
	  methodecls.insertall(xtbodyp->methodecls);
	}
	//hthreadblocksp->child_list.push_back(h_switchcase);
      }
	
	
      if (!iswait)  {
	for (auto spgsucc : sgb->getSuccessors()) {
	  ProcessSplitGraphBlock(spgsucc, state_num, h_switchcase);
	}
      }
    }
  }

  // Code below this line is obsolete
    // BFS traversal so all local decls are seen before being referenced
  
  void HDLThread::AddThreadMethod(const CFGBlock &BI) {
    std::vector<const CFGBlock *> succlist, nextsucclist;
    ProcessBB(BI);
    //CFGVisited[BI.getBlockID()]+=1;
    for (const auto &succ : BI.succs() ) { // gather successors
      const CFGBlock *SuccBlk = succ.getReachableBlock();
      if (SuccBlk!=NULL) succlist.push_back(SuccBlk);
    }
    bool changed;
    do {
      changed = false;
      for (const CFGBlock *si: succlist) { //process BB of successors at this level
	if (CFGVisited.find(si->getBlockID()) == CFGVisited.end()) {
	  CFGVisited[si->getBlockID()]+= 1;
	  LLVM_DEBUG(llvm::dbgs() << "Visiting Block " << si->getBlockID() << "\n");
	  ProcessBB(*si);
	  changed = true;
	  for (auto sii: si->succs()) {
	    const CFGBlock *SuccBlk = sii.getReachableBlock();
	    if (SuccBlk!=NULL) nextsucclist.push_back(SuccBlk); // gather successors at next level
	  }
	}
      }
      succlist = nextsucclist;
    }
    while (changed);
  }

  void HDLThread::ProcessBB(const CFGBlock &BI) {
    string blkid =  std::to_string(BI.getBlockID());
    if (BI.size() > 0) {
      hNodep h_body =  new hNode("B"+blkid, hNode::hdlopsEnum::hMethod);
      // from http://clang-developers.42468.n3.nabble.com/Visiting-statements-of-a-CFG-one-time-td4069440.html#a4069447
      // had to add recursive traversal of AST node children
      std::vector<const Stmt *> SS;
      FindStatements(BI, SS);

      hNodep htmp = new hNode(h_top_->getname(), hNode::hdlopsEnum::hNoop); // put the statement here temporarily
      for (auto stmt: SS) {
	LLVM_DEBUG(llvm::dbgs() << "Stmt follows\n");
	LLVM_DEBUG(stmt->dump(llvm::dbgs(), ast_context_));
	//generate hcode for this statement, 
	//HDLBody xmethod(const_cast<Stmt *>(stmt), h_body, diag_e, ast_context_, mod_vname_map_, false);
	xtbodyp->Run(const_cast<Stmt *>(stmt), htmp, rthread);
	CheckVardecls(htmp, BI.getBlockID());
	if (htmp->child_list.size() >0)
	  h_body->child_list.insert(h_body->child_list.end(), htmp->child_list.begin(), htmp->child_list.end());
	
	htmp->child_list.clear();
	
	methodecls.insertall(xtbodyp->methodecls);
      }
      hthreadblocksp->child_list.push_back(h_body);
    }
  }

}
