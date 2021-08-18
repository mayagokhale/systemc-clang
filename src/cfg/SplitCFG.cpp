#include "SplitCFG.h"

#include "llvm/Support/Debug.h"

using namespace systemc_clang;

/// ===========================================
/// SplitCFG
/// ===========================================
void SplitCFG::dfs_pop_on_wait(
    // const clang::CFGBlock* basic_block,
    const BigCFGBlock& bbb, llvm::SmallVectorImpl<BigCFGBlock>& waits_in_stack,
    llvm::SmallSet<BigCFGBlock, 8>& visited_waits) {
  const clang::CFGBlock* basic_block{bbb.getCFGBlock()};
  /// If the CFG block's successor is empty, then simply return.
  if (basic_block->succ_empty()) {
    return;
  }

  /// Stores the visited CFGBlocks.
  llvm::SmallSet<BigCFGBlock, 8> visited;
  /// This holds the CFGBlock's that need to be visited.
  llvm::SmallVector<
      std::pair<BigCFGBlock, clang::CFGBlock::const_succ_iterator>, 8>
      visit_stack;

  // BigCFGBlock insert_block{split_blocks_[basic_block], 0};
  visited.insert(bbb);
  visit_stack.push_back(std::make_pair(bbb, basic_block->succ_begin()));

  /// Path of basic blocks that constitute paths to wait() statements.
  VectorCFGBlock curr_path;
  do {
    std::pair<BigCFGBlock, clang::CFGBlock::const_succ_iterator>& Top =
        visit_stack.back();
    BigCFGBlock& parent_big_block{Top.first};
    const clang::CFGBlock* parent_bb = Top.first.getCFGBlock();
    const SplitCFGBlock* parent_splitbb = Top.first.getSplitCFGBlock();
    clang::CFGBlock::const_succ_iterator& I = Top.second;
    unsigned int split_index{Top.first.getSplitIndex()};
    // llvm::dbgs() << "BB# " << parent_bb->getBlockID() << "." << split_index
    //            << "\n";
    parent_big_block.dump();

    /// This basic block has a wait in it.  So, we need to go through it to add
    /// successors specifically embedded within the SpligCFGBlock.
    bool bb_has_wait{split_blocks_[parent_bb].hasWait()};

    if (bb_has_wait) {
      parent_big_block.dump();
      llvm::dbgs() << "\t has a wait in it\n";
      curr_path.push_back(parent_big_block);
      /// The wait has not been processed yet so add it to the visited_wait
      /// sets. Then add it in the stack to process the waits.
      if (visited_waits.insert(parent_big_block).second == true) {
        // Insert the successor of the block.
        /// Add the successor of the CFGBlock with a wait() in it.
        unsigned int last_idx{parent_big_block.getSplitIndex()};
        BigCFGBlock old_parent_block{parent_big_block};
        if (parent_big_block.getSuccessorIndex()) {
          llvm::dbgs() << "Add SplitCFGBlock's successor\n";
          // llvm::dbgs() << "Insert successor beyond WAIT of BB#" <<
          // parent_bb->getBlockID()
          // << "." << last_idx << " : BB#" << parent_bb->getBlockID()
          // << "." << parent_big_block.getSplitIndex() << "\n";
          llvm::dbgs() << "Previous parent block\n";
          old_parent_block.dump();
          llvm::dbgs() << "New parent block\n";
          parent_big_block.dump();

          waits_in_stack.push_back(parent_big_block);

        } else {
          llvm::dbgs() << "Add REGULAR CFGBlock's successor\n";
          while (I != parent_bb->succ_end()) {
            basic_block = *I++;
            if (basic_block) {
              llvm::dbgs() << "Insert successor of BB#"
                           << parent_bb->getBlockID() << "." << last_idx
                           << ": BB#" << basic_block->getBlockID() << ".0\n";
              BigCFGBlock insert_bb{split_blocks_[basic_block], 0};

              llvm::dbgs() << "Previous parent block, last index " << last_idx
                           << "\n";
              old_parent_block.dump();
              llvm::dbgs() << "New parent block\n";
              insert_bb.dump();

              if (parent_big_block.isNextBlockWait(last_idx)) {
                llvm::dbgs() << " insert in wait_stack\n";
                waits_in_stack.push_back(insert_bb);
              } else {
                llvm::dbgs() << " insert in visit_stack\n";
                visit_stack.push_back(
                    std::make_pair(insert_bb, basic_block->succ_begin()));
              }
              // There should only be one successor.
              break;
            }
          }
        }
      }
    }

    /// A new BB has been found.  Add its successors to be visited in the
    /// future.
    bool FoundNew = false;
    if (bb_has_wait == false) {
      llvm::dbgs() << "BB does not have wait\n";
      while (I != parent_bb->succ_end()) {
        basic_block = *I++;
        if (basic_block) {
          /// So odd that the access to split_blocks_ for a non-entry would
          /// insert a null element!
          BigCFGBlock insert_bb{split_blocks_[basic_block], 0};
          if (visited.insert(insert_bb).second) {
            FoundNew = true;
            break;
          }
        }
      }

      if (FoundNew) {
        // Go down one level if there is a unvisited successor.
        llvm::dbgs() << "FoundNew Visited BB#" << parent_bb->getBlockID() << "."
                     << split_index << "\n";
        BigCFGBlock insert_bb{split_blocks_[basic_block], 0};
        BigCFGBlock parent_block{split_blocks_[parent_bb], 0};
        visit_stack.push_back(
            std::make_pair(insert_bb, basic_block->succ_begin()));
        curr_path.push_back(parent_block);
      } else {
        // Go up one level.
        llvm::dbgs() << "pop: " << visit_stack.size() << "\n";
        visit_stack.pop_back();
      }
    }

    /// Dump contents of visit stack.
    unsigned int x{0};
    llvm::dbgs() << "visit_stack ";
    for (auto const& vs : visit_stack) {
      llvm::dbgs() << "[" << x++ << ", " << vs.first.getCFGBlock()->getBlockID()
                   << "]; ";
    }
    llvm::dbgs() << "\n";

  } while (!visit_stack.empty());

  /// Insert the path constructed.
  paths_found_.push_back(curr_path);
}

bool SplitCFG::isWait(const clang::CFGBlock& block) const {
  for (auto const& element : block.refs()) {
    if (auto cfg_stmt = element->getAs<clang::CFGStmt>()) {
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
  }

  return false;
};

void SplitCFG::generate_paths() {
  /// Set of visited wait blocks.
  llvm::SmallSet<BigCFGBlock, 8> visited_waits;
  llvm::SmallVector<BigCFGBlock> waits_in_stack;

  /// G = cfg_
  const clang::CFGBlock* BB{&cfg_->getEntry()};
  BigCFGBlock basic_bb{split_blocks_[BB], 0};
  waits_in_stack.push_back(basic_bb);
  do {
    const BigCFGBlock& bigBB{waits_in_stack.pop_back_val()};
    BB = bigBB.getCFGBlock();
    llvm::dbgs() << "Processing BB# " << BB->getBlockID() << "."
                 << bigBB.getSplitIndex() << "\n";
    dfs_pop_on_wait(bigBB, waits_in_stack, visited_waits);
    llvm::dbgs() << "\n";
  } while (!waits_in_stack.empty());
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
    auto block = *begin_it;
    block->dump();
    /// Try to split the block.
    SplitCFGBlock sp{};
    sp.split_block(block);
    // sp.dump();

    //  Insert all CFGblocks and their respective SplitCFGBlock.
    split_blocks_.insert(
        std::pair<const clang::CFGBlock*, SplitCFGBlock>(block, sp));
  }
}

void SplitCFG::dump() const {
  /// Dump all the paths found.
  llvm::dbgs() << "Dump all paths to wait() found in the CFG.\n";
  unsigned int i{0};
  for (auto const& block_vector : paths_found_) {
    llvm::dbgs() << "Path S" << i++ << ": ";
    for (auto const& block : block_vector) {
      llvm::dbgs() << block.getCFGBlock()->getBlockID() << "."
                   << block.getSplitIndex() << "  ";
    }
    if (i == 1) {
      llvm::dbgs() << " (reset path)";
    }
    llvm::dbgs() << "\n";
  }

  llvm::dbgs() << "Dump all blocks that were split " << split_blocks_.size()
               << "\n";
  for (auto const& sblock : split_blocks_) {
    llvm::dbgs() << "Block ID: " << sblock.first->getBlockID() << "\n";
    sblock.first->dump();
    const SplitCFGBlock* block_with_wait{&sblock.second};
    llvm::dbgs() << "Print all info for split block\n";
    block_with_wait->dump();
  }
}

SplitCFG::SplitCFG(clang::ASTContext& context) : context_{context} {}

SplitCFG::SplitCFG(clang::ASTContext& context,
                   const clang::CXXMethodDecl* method)
    : context_{context} {
  cfg_ = clang::CFG::buildCFG(method, method->getBody(), &context_,
                              clang::CFG::BuildOptions());
}
