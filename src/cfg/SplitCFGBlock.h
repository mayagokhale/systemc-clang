//===- SplitCFGBlock.h - Split the CFGBlock separated by waits -*- C++
//-*-=====//
//
// Part of the systemc-clang project.
// See License.rst
//
//===----------------------------------------------------------------------===//
//
/// \file
/// Split the CFG block in chunks separted by wait() statements.
//
/// \author Hiren Patel
//===----------------------------------------------------------------------===//

#ifndef _SPLIT_CFG_BLOCK_H_
#define _SPLIT_CFG_BLOCK_H_

#include "clang/Analysis/CFG.h"
#include <vector>

namespace systemc_clang {

///
/// This class  represents information that is stored to split a single CFGBlock
/// into elements that are wait() calls versus others.
///
///
class SplitCFGBlock {
 private:
  using VectorCFGElementPtr = llvm::SmallVector<const clang::CFGElement *>;
  using VectorSplitCFGBlockPtr = llvm::SmallVector<const SplitCFGBlock*>;

  friend class SplitCFG;

  clang::CFGBlock *block_;
  bool has_wait_;
  unsigned int id_;

  /// Split the elements into blocks separated by wait() statements
  llvm::SmallVector<VectorCFGElementPtr> split_elements_;

  /// This holds the ids in split_elements_ that correspond to the wait
  /// statements.  This will be a single vector with just the wait() element.
  llvm::SmallVector<unsigned int> wait_element_ids_;

  VectorCFGElementPtr elements_;
  /// Predecessors and successors.
  llvm::SmallVector<SplitCFGBlock *> predecessors_;
  llvm::SmallVector<SplitCFGBlock *> successors_;

 public:

  struct SuccessorIterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = SplitCFGBlock;
    using pointer = SplitCFGBlock *;    // or also value_type*
    using reference = SplitCFGBlock &;  // or also value_type&

    SuccessorIterator(std::size_t idx, llvm::SmallVector<SplitCFGBlock*> &succ) : index_(idx), succs_{succ} {}
    reference operator*() const { return *succs_[index_]; }
    pointer operator->() { return succs_[index_]; }

    // Prefix increment
    SuccessorIterator &operator++() {
      index_++;
      return *this;
    }

    // Postfix increment
    SuccessorIterator operator++(int) {
      SuccessorIterator tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool operator==(const SuccessorIterator &a, const SuccessorIterator &b) {
      return a.index_ == b.index_;
    };
    friend bool operator!=(const SuccessorIterator &a, const SuccessorIterator &b) {
      return a.index_!= b.index_;
    };

   private:
    std::size_t index_;
    const llvm::SmallVector<SplitCFGBlock*> &succs_;
  };

  SuccessorIterator succs_begin() { return SuccessorIterator{0, successors_}; }
  SuccessorIterator succs_end() { return SuccessorIterator{successors_.size(), successors_}; }

 private:
  bool isWait(const clang::CFGElement &element) const;
  bool isFunctionCall(const clang::CFGElement &element) const;

 public:
  SplitCFGBlock();
  SplitCFGBlock(const SplitCFGBlock &from);

  clang::CFGBlock *getCFGBlock() const;
  std::size_t getSplitBlockSize() const;
  bool hasWait() const;
  unsigned int getID() const;

  void insertElements(VectorCFGElementPtr & elements);
  void split_block(clang::CFGBlock *block);

  void dump() const;
};

};  // namespace systemc_clang

#endif
