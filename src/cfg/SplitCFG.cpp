#include "SplitCFG.h"

#include "llvm/Support/Debug.h"
#include "llvm/ADT/PostOrderIterator.h"

using namespace systemc_clang;

/// ===========================================
/// SplitCFG
/// ===========================================
SplitCFG::SplitCFG(clang::ASTContext& context) : context_{context} {}

void SplitCFG::dfs() {
  /// G = cfg_
  const clang::CFGBlock* BB{&cfg_->getEntry()};
  if (BB->succ_empty()) {
    /// Empty CFG block
    return;
  }

  llvm::SmallPtrSet<const clang::CFGBlock*, 8> visited;
  llvm::SmallVector<
      std::pair<const clang::CFGBlock*, clang::CFGBlock::const_succ_iterator>,
      8>
      visit_stack;
  llvm::SmallVector<const clang::CFGBlock*, 8> in_stack;

  visited.insert(BB);
  visit_stack.push_back(std::make_pair(BB, BB->succ_begin()));

  do {
    std::pair<const clang::CFGBlock*, clang::CFGBlock::const_succ_iterator>&
        Top = visit_stack.back();
    const clang::CFGBlock* ParentBB = Top.first;
    clang::CFGBlock::const_succ_iterator& I = Top.second;

    llvm::dbgs() << "BB# " << ParentBB->getBlockID() << "\n";
    bool FoundNew = false;
    while (I != ParentBB->succ_end()) {
      BB = *I++;
      if ((BB != nullptr) && (visited.insert(BB).second)) {
        FoundNew = true;
        break;
      }
    }
      llvm::dbgs() << "End adding successors\n";

    if (FoundNew) {
      // Go down one level if there is a unvisited successor.
      visit_stack.push_back(std::make_pair(BB, BB->succ_begin()));
    } else {
      // Go up one level.
      llvm::dbgs() << "pop: " << visit_stack.size() << "\n";
      visit_stack.pop_back();
    }
  } while (!visit_stack.empty());
}

void SplitCFG::split_wait_blocks(const clang::CXXMethodDecl* method) {
  cfg_ = clang::CFG::buildCFG(method, method->getBody(), &context_,
                              clang::CFG::BuildOptions());

  clang::LangOptions lang_opts;
  cfg_->dump(lang_opts, true);

  // Go through all CFG blocks
  llvm::dbgs() << "Iterate through all CFGBlocks.\n";
  /// These iterators are not in clang 12.
  // for (auto const &block: CFG->const_nodes()) {
  for (auto begin_it = cfg_->nodes_begin(); begin_it != cfg_->nodes_end();
       ++begin_it) {
    auto block{*begin_it};

    /// Try to split the block.
    SplitCFGBlock sp{};
    sp.split_block(block);

    //////////////////////////////////////////
    //
    if (sp.hasWait()) {
      /// The block has been split.
      split_blocks_.insert(
          std::pair<const clang::CFGBlock*, SplitCFGBlock>(block, sp));
    }
  }
}

void SplitCFG::dump() const {
  llvm::dbgs() << "Dump all blocks that were split\n";
  for (auto const& sblock : split_blocks_) {
    llvm::dbgs() << "Block ID: " << sblock.first->getBlockID() << "\n";
    sblock.first->dump();
    const SplitCFGBlock* block_with_wait{&sblock.second};
    llvm::dbgs() << "Print all info for split block\n";
    block_with_wait->dump();
  }
}
