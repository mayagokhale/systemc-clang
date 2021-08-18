#include "SplitCFGBlock.h"

#include "llvm/Support/Debug.h"

using namespace systemc_clang;

SplitCFGBlock::SplitCFGBlock() : block_{nullptr}, has_wait_{false} {}

SplitCFGBlock::SplitCFGBlock(const SplitCFGBlock& from) {
  block_ = from.block_;
  has_wait_ = from.has_wait_;
  split_elements_ = from.split_elements_;
  wait_element_ids_ = from.wait_element_ids_;
  preds_ = from.preds_;
  succs_ = from.succs_;
}

SplitCFGBlock& SplitCFGBlock::operator=(const SplitCFGBlock& from) {
  block_ = from.block_;
  has_wait_ = from.has_wait_;
  split_elements_ = from.split_elements_;
  wait_element_ids_ = from.wait_element_ids_;
  preds_ = from.preds_;
  succs_ = from.succs_;

  return *this;
}

clang::CFGBlock* SplitCFGBlock::getCFGBlock() const { return block_; }

bool SplitCFGBlock::hasWait() const { return has_wait_; }

std::size_t SplitCFGBlock::getSplitBlockSize() const {
  return split_elements_.size();
}

bool SplitCFGBlock::getSuccessorIndex(unsigned int& idx) const {
  /// There are X situations:
  //
  // 1. [code][wait]
  //       0    1
  // Assuming idx is 0, this function would return FALSE because it is beyond
  // the size of the split_elements_.
  // - Regular CFGBlock's successor should be added to the wait_stack (WAIT)
  // since there is a wait at 1.
  // - No changes to the visit_stack (VISIT)
  //
  //
  // 2. [code][wait][code]
  //       0    1      2
  //
  // Assuming idx is 0, idx will point to 2.
  // - This new block will be added to the wait_stack (nodes after waits to be
  // visited).
  // - This block will NOT be added to the visit stack since this is passing
  // over a wait
  //
  // Assuming idx is 2, this function will return FALSE.
  // - 2 should NOT be added to the wait_stack (WAIT) since the next block is
  // not a wait
  // - Regular CFGBlock's successor should be added to the visit_stack (VISIT)
  // since there is no wait in 2.
  //
  // getSuccessorIndex is FALSE && isNextBlockWait is FALSE (case 2)
  // - visit_stack: add CFGBlock's successor
  // - wait_stack: do nothing
  //
  // getSuccessorIndex is FALSE && isNextBlockWait is TRUE (case 1)
  //
  // - visit_stack: do nothing
  // - wait_stack: add CFGBlock's successor
  //
  // getSuccessorIndex is TRUE (by defn. isNextBlockWait should be TRUE)
  // - visit_stack: do nothing
  // - wait_stack: add idx to it.
  if ((idx + 2) < split_elements_.size()) {
    idx += 2;
    return true;
  }
  return false;
}

bool SplitCFGBlock::isNextBlockWait(unsigned int idx) const {
  llvm::dbgs() << "idx " << idx << " split_elements_size " <<  split_elements_.size() << "\n";
  if (idx < split_elements_.size()) {
    return false;
  }
  auto const sblock{split_elements_[++idx]};
  return sblock.second;
}

bool SplitCFGBlock::isFunctionCall(const clang::CFGElement& element) const {
  if (auto cfg_stmt = element.getAs<clang::CFGStmt>()) {
    auto stmt{cfg_stmt->getStmt()};
    // stmt->dump();

    auto* expr{llvm::dyn_cast<clang::Expr>(stmt)};
    if (auto cxx_me = llvm::dyn_cast<clang::CXXMemberCallExpr>(expr)) {
      if (auto direct_callee = cxx_me->getDirectCallee()) {
        auto name{direct_callee->getNameInfo().getAsString()};
        llvm::dbgs() << "Function call: " << name << "\n";
        if (name != std::string("wait")) {
          llvm::dbgs() << "NOT A WAIT CALL\n";
          /// Generate the CFG
          auto method{cxx_me->getMethodDecl()};
          auto fcfg{clang::CFG::buildCFG(method, method->getBody(),
                                         &method->getASTContext(),
                                         clang::CFG::BuildOptions())};

          clang::LangOptions lang_opts;
          // fcfg->dump(lang_opts, true);
        }
      }
    }
  }

  return true;
}

bool SplitCFGBlock::isWait(const clang::CFGElement& element) const {
  if (auto cfg_stmt = element.getAs<clang::CFGStmt>()) {
    auto stmt{cfg_stmt->getStmt()};

    // stmt->dump();
    if (auto* expr = llvm::dyn_cast<clang::Expr>(stmt)) {
      if (auto cxx_me = llvm::dyn_cast<clang::CXXMemberCallExpr>(expr)) {
        if (auto direct_callee = cxx_me->getDirectCallee()) {
          auto name{direct_callee->getNameInfo().getAsString()};
          if (name == std::string("wait")) {
            // llvm::dbgs() << "@@@@ FOUND WAIT @@@@\n";

            /// Check that there is only 1 or 0 arguments
            auto args{cxx_me->getNumArgs()};
            if (args >= 2) {
              llvm::errs() << "wait() must have either 0 or 1 argument.\n";
              return false;
            }
            return true;
          }
        }
      }
    }
  }

  return false;
};

void SplitCFGBlock::split_block(clang::CFGBlock* block) {
  assert(block != nullptr);

  block_ = block;

  llvm::dbgs() << "Checking if block " << block->getBlockID()
               << " has a wait()\n";
  // We are going to have two vectors.
  // 1. A vector of vector pointers to CFGElements.
  // 2. A vector of pointers to CFGElements that are waits.
  //
  //
  unsigned int num_elements{block_->size()};

  VectorCFGElementPtr vec_elements{};
  for (auto const& element : block->refs()) {
    // element->dump();

    //////////////////////////////////////////
    /// Test code
    //////////////////////////////////////////
    // if (isFunctionCall(*element)) {
    // llvm::dbgs() << "YEAOW generate CFG for function call\n";
    //
    // }

    /// refs() returns an iterator, which actually stores an ElementRefImpl<>
    /// interface. In order to get the correct pointer to CFGElement, we need to
    /// explicitly call operator->(). Odd!
    const clang::CFGElement* element_ptr{element.operator->()};
    /// If the element is a wait() then split it.
    if (isWait(*element)) {
      /// There is only one statement and it's a wait().
      if (num_elements == 1) {
        llvm::dbgs() << "DBG: Only one statement and it is a wait().\n";
      }

      if (vec_elements.size() != 0) {
        split_elements_.push_back(std::make_pair(vec_elements, false));
        vec_elements.clear();
      }

      /// Add the wait as a separate entry in the list.
      vec_elements.push_back(element_ptr);
      split_elements_.push_back(std::make_pair(vec_elements, true));
      vec_elements.clear();
      wait_element_ids_.push_back(split_elements_.size() - 1);

      has_wait_ = true;
    } else {
      vec_elements.push_back(element_ptr);
    }
  }

  if (vec_elements.size() != 0) {
    split_elements_.push_back(std::make_pair(vec_elements, false));
  }
}

void SplitCFGBlock::dump() const {
  if (block_) {
    unsigned int i{0};

    for (auto const& split : split_elements_) {
      llvm::dbgs() << "SB" << i++ << " wait status is ";
      if (split.second) {
        llvm::dbgs() << "true\n";
      } else {
        llvm::dbgs() << "false\n";
      }

      for (auto const& element : split.first) {
        llvm::dbgs() << "  ";
        element->dump();
      }
    }

    llvm::dbgs() << "\n";
    /// Dump the wait ids
    llvm::dbgs() << "Dump wait elements: " << wait_element_ids_.size() << " \n";
    for (auto const& id : wait_element_ids_) {
      llvm::dbgs() << "Wait element at id " << id << "\n";
    }
  }
}
