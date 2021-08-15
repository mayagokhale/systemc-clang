#ifndef _SPLIT_CFG_H_
#define _SPLIT_CFG_H_

#include <unordered_map>

#include "clang/Analysis/CFG.h"
#include "SplitCFGBlock.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/ADT/SmallSet.h"
//#include "llvm/ADT/SmallPtrSet.h"

namespace systemc_clang {

struct CFGBlockContainer {
 private:
  const clang::CFGBlock *cfg_block_;
  unsigned int split_index_;

 public:
  CFGBlockContainer(const clang::CFGBlock *block, unsigned int idx)
      : cfg_block_{(block)}, split_index_{idx} {};
  const clang::CFGBlock *get_cfg_block() const { return cfg_block_; }
  unsigned int get_split_index() { return split_index_; }
  /// Needed by std::map<>
  bool operator<(const CFGBlockContainer &from) const {
    return std::tie(cfg_block_, split_index_) <
           std::tie(from.cfg_block_, from.split_index_);
  }
  bool operator==(const CFGBlockContainer &from) const {
    return std::tie(cfg_block_, split_index_) ==
           std::tie(from.cfg_block_, from.split_index_);
  }
};

/// ===========================================
/// SplitCFG
/// ===========================================
class SplitCFG {
 private:
  using BigCFGBlock = CFGBlockContainer;
  using VectorCFGBlock = llvm::SmallVector<BigCFGBlock>;

 private:
  /// \brief The context necessary to access translation unit.
  clang::ASTContext &context_;

  /// \brief The saved CFG for a given method.
  std::unique_ptr<clang::CFG> cfg_;

  /// \brief The split blocks in the CFG.
  std::unordered_map<const clang::CFGBlock *, SplitCFGBlock> split_blocks_;

  /// \brief Paths of BBs generated.
  llvm::SmallVector<VectorCFGBlock> paths_found_;

 private:
  /// \brief Checks if a CFGBlock has a wait() call in it.
  bool isWait(const clang::CFGBlock &block) const;

 public:
  SplitCFG(clang::ASTContext &context);
  SplitCFG(clang::ASTContext &context, const clang::CXXMethodDecl *cxx_decl);

  void split_wait_blocks(const clang::CXXMethodDecl *cxx_decl);
  void dfs_pop_on_wait(const clang::CFGBlock *BB,
                       llvm::SmallVectorImpl<BigCFGBlock> &waits_in_stack,
                       llvm::SmallSet<BigCFGBlock, 8> &visited_waits);

  void generate_paths();
  void dump() const;
};

};  // namespace systemc_clang

#endif /* _SPLIT_CFG_H_ */
