#include "leanstore/btree/core/BTreeGeneric.hpp"

#include "leanstore/btree/core/BTreeWalPayload.hpp"
#include "leanstore/LeanStore.hpp"
#include "leanstore/Units.hpp"
#include "leanstore/btree/core/BTreeNode.hpp"
#include "leanstore/btree/core/PessimisticExclusiveIterator.hpp"
#include "leanstore/btree/core/PessimisticSharedIterator.hpp"
#include "leanstore/buffer-manager/BufferFrame.hpp"
#include "leanstore/buffer-manager/BufferManager.hpp"
#include "leanstore/buffer-manager/GuardedBufferFrame.hpp"
#include "leanstore/utils/Defer.hpp"
#include "leanstore/utils/Log.hpp"
#include "leanstore/utils/Misc.hpp"
#include "leanstore/utils/UserThread.hpp"
#include "leanstore/utils/ToJson.hpp"

#include <cstdint>
#include <format>

using namespace leanstore::storage;

namespace leanstore::storage::btree {

void BTreeGeneric::Init(leanstore::LeanStore* store, TREEID btreeId, BTreeConfig config) {
  this->mStore = store;
  this->mTreeId = btreeId;
  this->mConfig = std::move(config);

  mMetaNodeSwip = &mStore->mBufferManager->AllocNewPageMayJump(btreeId);
  mMetaNodeSwip.AsBufferFrame().mHeader.mKeepInMemory = true;
  LS_DCHECK(mMetaNodeSwip.AsBufferFrame().mHeader.mLatch.GetOptimisticVersion() == 0);

  auto guardedRoot = GuardedBufferFrame<BTreeNode>(
      mStore->mBufferManager.get(), &mStore->mBufferManager->AllocNewPageMayJump(btreeId));
  auto xGuardedRoot = ExclusiveGuardedBufferFrame<BTreeNode>(std::move(guardedRoot));
  xGuardedRoot.InitPayload(true);

  auto guardedMeta = GuardedBufferFrame<BTreeNode>(mStore->mBufferManager.get(), mMetaNodeSwip);
  auto xGuardedMeta = ExclusiveGuardedBufferFrame(std::move(guardedMeta));
  xGuardedMeta->mIsLeaf = false;
  xGuardedMeta->mRightMostChildSwip = xGuardedRoot.bf();

  // Record WAL
  if (mConfig.mEnableWal) {
    TXID sysTxId = mStore->AllocSysTxTs();

    auto rootWalHandler =
        xGuardedRoot.ReserveWALPayload<WalInitPage>(0, sysTxId, mTreeId, xGuardedRoot->mIsLeaf);
    rootWalHandler.SubmitWal();

    auto metaWalHandler =
        xGuardedMeta.ReserveWALPayload<WalInitPage>(0, sysTxId, mTreeId, xGuardedMeta->mIsLeaf);
    metaWalHandler.SubmitWal();

    xGuardedMeta.SyncSystemTxId(sysTxId);
    xGuardedRoot.SyncSystemTxId(sysTxId);
  }
}

PessimisticSharedIterator BTreeGeneric::GetIterator() {
  return PessimisticSharedIterator(*this);
}

PessimisticExclusiveIterator BTreeGeneric::GetExclusiveIterator() {
  return PessimisticExclusiveIterator(*this);
}

void BTreeGeneric::TrySplitMayJump(TXID sysTxId, BufferFrame& toSplit, int16_t favoredSplitPos) {
  auto parentHandler = findParentEager(*this, toSplit);
  GuardedBufferFrame<BTreeNode> guardedParent(
      mStore->mBufferManager.get(), std::move(parentHandler.mParentGuard), parentHandler.mParentBf);
  auto guardedChild = GuardedBufferFrame<BTreeNode>(mStore->mBufferManager.get(), guardedParent,
                                                    parentHandler.mChildSwip);
  if (guardedChild->mNumSlots <= 1) {
    Log::Warn(
        "Split failed, slots too less: sysTxId={}, pageId={}, favoredSplitPos={}, numSlots={}",
        sysTxId, toSplit.mHeader.mPageId, favoredSplitPos, guardedChild->mNumSlots);
    return;
  }

  // init the separator info
  BTreeNode::SeparatorInfo sepInfo;
  if (favoredSplitPos < 0 || favoredSplitPos >= guardedChild->mNumSlots - 1) {
    if (mConfig.mUseBulkInsert) {
      favoredSplitPos = guardedChild->mNumSlots - 2;
      sepInfo = BTreeNode::SeparatorInfo{guardedChild->GetFullKeyLen(favoredSplitPos),
                                         static_cast<uint16_t>(favoredSplitPos), false};
    } else {
      sepInfo = guardedChild->FindSep();
    }
  } else {
    // Split on a specified position, used by contention management
    sepInfo = BTreeNode::SeparatorInfo{guardedChild->GetFullKeyLen(favoredSplitPos),
                                       static_cast<uint16_t>(favoredSplitPos), false};
  }

  // split the root node
  if (isMetaNode(guardedParent)) {
    splitRootMayJump(sysTxId, guardedParent, guardedChild, sepInfo);
    return;
  }

  // calculate space needed for separator in parent node
  const uint16_t spaceNeededForSeparator = guardedParent->SpaceNeeded(sepInfo.mSize, sizeof(Swip));

  // split the parent node to make zoom for separator
  if (!guardedParent->HasEnoughSpaceFor(spaceNeededForSeparator)) {
    guardedParent.unlock();
    guardedChild.unlock();
    TrySplitMayJump(sysTxId, *guardedParent.mBf);
    return;
  }

  // split the non-root node
  splitNonRootMayJump(sysTxId, guardedParent, guardedChild, sepInfo, spaceNeededForSeparator);
}

//! Split the root node, 4 nodes are involved in the split:
///
//!   meta(oldRoot) -> meta(newRoot(newLeft, oldRoot)).
///
//! meta         meta
//!   |            |
//! oldRoot      newRoot
//!              |     |
//!           newLeft oldRoot
///
//! 3 WALs are generated, redo process:
//! - Redo(newLeft, WalInitPage)
//!   - create new left
//! - Redo(newRoot, WalInitPage)
//!   - create new root
//! - Redo(oldRoot, WalSplitRoot)
//!   - move half of the old root to the new left
//!   - insert separator key into new root
//!   - update meta node to point to new root
///
void BTreeGeneric::splitRootMayJump(TXID sysTxId, GuardedBufferFrame<BTreeNode>& guardedMeta,
                                    GuardedBufferFrame<BTreeNode>& guardedOldRoot,
                                    const BTreeNode::SeparatorInfo& sepInfo) {
  auto xGuardedMeta = ExclusiveGuardedBufferFrame(std::move(guardedMeta));
  auto xGuardedOldRoot = ExclusiveGuardedBufferFrame(std::move(guardedOldRoot));
  auto* bm = mStore->mBufferManager.get();

  LS_DCHECK(isMetaNode(guardedMeta), "Parent should be meta node");
  LS_DCHECK(mHeight == 1 || !xGuardedOldRoot->mIsLeaf);

  // 1. create new left, lock it exclusively, write wal on demand
  auto* newLeftBf = &bm->AllocNewPageMayJump(mTreeId);
  auto guardedNewLeft = GuardedBufferFrame<BTreeNode>(bm, newLeftBf);
  auto xGuardedNewLeft = ExclusiveGuardedBufferFrame<BTreeNode>(std::move(guardedNewLeft));
  if (mConfig.mEnableWal) {
    xGuardedNewLeft.SyncSystemTxId(sysTxId);
    xGuardedNewLeft.WriteWal<WalInitPage>(0, sysTxId, mTreeId, xGuardedOldRoot->mIsLeaf);
  }
  xGuardedNewLeft.InitPayload(xGuardedOldRoot->mIsLeaf);

  // 2. create new root, lock it exclusively, write wal on demand
  auto* newRootBf = &bm->AllocNewPageMayJump(mTreeId);
  auto guardedNewRoot = GuardedBufferFrame<BTreeNode>(bm, newRootBf);
  auto xGuardedNewRoot = ExclusiveGuardedBufferFrame<BTreeNode>(std::move(guardedNewRoot));
  if (mConfig.mEnableWal) {
    xGuardedNewRoot.SyncSystemTxId(sysTxId);
    xGuardedNewRoot.WriteWal<WalInitPage>(0, sysTxId, mTreeId, false);
  }
  xGuardedNewRoot.InitPayload(false);

  // 3.1. write wal on demand
  if (mConfig.mEnableWal) {
    xGuardedOldRoot.SyncSystemTxId(sysTxId);
    xGuardedOldRoot.WriteWal<WalSplitRoot>(0, sysTxId, xGuardedNewLeft.bf()->mHeader.mPageId,
                                           xGuardedNewRoot.bf()->mHeader.mPageId,
                                           xGuardedMeta.bf()->mHeader.mPageId, sepInfo);
  }

  // 3.2. move half of the old root to the new left,
  // 3.3. insert separator key into new root,
  xGuardedNewRoot->mRightMostChildSwip = xGuardedOldRoot.bf();
  xGuardedOldRoot->Split(xGuardedNewRoot, xGuardedNewLeft, sepInfo);

  // 3.4. update meta node to point to new root
  xGuardedMeta->mRightMostChildSwip = xGuardedNewRoot.bf();
  mHeight++;
}

//! Split a non-root node, 3 nodes are involved in the split:
//! parent(child) -> parent(newLeft, child)
///
//! parent         parent
//!   |            |   |
//! child     newLeft child
///
void BTreeGeneric::splitNonRootMayJump(TXID sysTxId, GuardedBufferFrame<BTreeNode>& guardedParent,
                                       GuardedBufferFrame<BTreeNode>& guardedChild,
                                       const BTreeNode::SeparatorInfo& sepInfo,
                                       uint16_t spaceNeededForSeparator) {
  auto xGuardedParent = ExclusiveGuardedBufferFrame(std::move(guardedParent));
  auto xGuardedChild = ExclusiveGuardedBufferFrame(std::move(guardedChild));

  LS_DCHECK(!isMetaNode(guardedParent), "Parent should not be meta node");
  LS_DCHECK(!xGuardedParent->mIsLeaf, "Parent should not be leaf node");

  // 1. create new left, lock it exclusively, write wal on demand
  auto* newLeftBf = &mStore->mBufferManager->AllocNewPageMayJump(mTreeId);
  auto guardedNewLeft = GuardedBufferFrame<BTreeNode>(mStore->mBufferManager.get(), newLeftBf);
  auto xGuardedNewLeft = ExclusiveGuardedBufferFrame<BTreeNode>(std::move(guardedNewLeft));
  if (mConfig.mEnableWal) {
    xGuardedNewLeft.SyncSystemTxId(sysTxId);
    xGuardedNewLeft.WriteWal<WalInitPage>(0, sysTxId, mTreeId, xGuardedChild->mIsLeaf);
  }
  xGuardedNewLeft.InitPayload(xGuardedChild->mIsLeaf);

  // 2.1. write wal on demand or simply mark as dirty
  if (mConfig.mEnableWal) {
    xGuardedParent.SyncSystemTxId(sysTxId);
    xGuardedChild.SyncSystemTxId(sysTxId);
    xGuardedChild.WriteWal<WalSplitNonRoot>(0, sysTxId, xGuardedParent.bf()->mHeader.mPageId,
                                            xGuardedNewLeft.bf()->mHeader.mPageId, sepInfo);
  }

  // 2.2. make room for separator key in parent node
  // 2.3. move half of the old root to the new left
  // 2.4. insert separator key into parent node
  xGuardedParent->RequestSpaceFor(spaceNeededForSeparator);
  xGuardedChild->Split(xGuardedParent, xGuardedNewLeft, sepInfo);
}

bool BTreeGeneric::TryMergeMayJump(TXID sysTxId, BufferFrame& toMerge, bool swizzleSibling) {
  auto parentHandler = findParentEager(*this, toMerge);
  GuardedBufferFrame<BTreeNode> guardedParent(
      mStore->mBufferManager.get(), std::move(parentHandler.mParentGuard), parentHandler.mParentBf);
  GuardedBufferFrame<BTreeNode> guardedChild(mStore->mBufferManager.get(), guardedParent,
                                             parentHandler.mChildSwip);
  auto posInParent = parentHandler.mPosInParent;
  if (isMetaNode(guardedParent) ||
      guardedChild->FreeSpaceAfterCompaction() < BTreeNode::UnderFullSize()) {
    guardedParent.unlock();
    guardedChild.unlock();
    return false;
  }

  if (guardedParent->mNumSlots <= 1) {
    return false;
  }

  LS_DCHECK(posInParent <= guardedParent->mNumSlots,
            "Invalid position in parent, posInParent={}, childSizeOfParent={}", posInParent,
            guardedParent->mNumSlots);
  guardedParent.JumpIfModifiedByOthers();
  guardedChild.JumpIfModifiedByOthers();

  // TODO: write WALs
  auto mergeAndReclaimLeft = [&]() {
    auto* leftSwip = guardedParent->ChildSwip(posInParent - 1);
    if (!swizzleSibling && leftSwip->IsEvicted()) {
      return false;
    }
    auto guardedLeft =
        GuardedBufferFrame<BTreeNode>(mStore->mBufferManager.get(), guardedParent, *leftSwip);
    auto xGuardedParent = ExclusiveGuardedBufferFrame(std::move(guardedParent));
    auto xGuardedChild = ExclusiveGuardedBufferFrame(std::move(guardedChild));
    auto xGuardedLeft = ExclusiveGuardedBufferFrame(std::move(guardedLeft));

    LS_DCHECK(xGuardedChild->mIsLeaf == xGuardedLeft->mIsLeaf);

    if (!xGuardedLeft->merge(posInParent - 1, xGuardedParent, xGuardedChild)) {
      guardedParent = std::move(xGuardedParent);
      guardedChild = std::move(xGuardedChild);
      guardedLeft = std::move(xGuardedLeft);
      return false;
    }

    if (mConfig.mEnableWal) {
      guardedParent.SyncSystemTxId(sysTxId);
      guardedChild.SyncSystemTxId(sysTxId);
      guardedLeft.SyncSystemTxId(sysTxId);
    }

    xGuardedLeft.Reclaim();
    guardedParent = std::move(xGuardedParent);
    guardedChild = std::move(xGuardedChild);
    return true;
  };
  auto mergeAndReclaimRight = [&]() {
    auto& rightSwip = ((posInParent + 1) == guardedParent->mNumSlots)
                          ? guardedParent->mRightMostChildSwip
                          : *guardedParent->ChildSwip(posInParent + 1);
    if (!swizzleSibling && rightSwip.IsEvicted()) {
      return false;
    }
    auto guardedRight =
        GuardedBufferFrame<BTreeNode>(mStore->mBufferManager.get(), guardedParent, rightSwip);
    auto xGuardedParent = ExclusiveGuardedBufferFrame(std::move(guardedParent));
    auto xGuardedChild = ExclusiveGuardedBufferFrame(std::move(guardedChild));
    auto xGuardedRight = ExclusiveGuardedBufferFrame(std::move(guardedRight));

    LS_DCHECK(xGuardedChild->mIsLeaf == xGuardedRight->mIsLeaf);

    if (!xGuardedChild->merge(posInParent, xGuardedParent, xGuardedRight)) {
      guardedParent = std::move(xGuardedParent);
      guardedChild = std::move(xGuardedChild);
      guardedRight = std::move(xGuardedRight);
      return false;
    }

    if (mConfig.mEnableWal) {
      guardedParent.SyncSystemTxId(sysTxId);
      guardedChild.SyncSystemTxId(sysTxId);
      guardedRight.SyncSystemTxId(sysTxId);
    }

    xGuardedChild.Reclaim();
    guardedParent = std::move(xGuardedParent);
    guardedRight = std::move(xGuardedRight);
    return true;
  };

  SCOPED_DEFER({
    if (!isMetaNode(guardedParent) &&
        guardedParent->FreeSpaceAfterCompaction() >= BTreeNode::UnderFullSize()) {
      JUMPMU_TRY() {
        TryMergeMayJump(sysTxId, *guardedParent.mBf, true);
      }
      JUMPMU_CATCH() {
      }
    }
  });

  bool succeed = false;
  if (posInParent > 0) {
    succeed = mergeAndReclaimLeft();
  }
  if (!succeed && posInParent < guardedParent->mNumSlots) {
    succeed = mergeAndReclaimRight();
  }

  return succeed;
}

// ret: 0 did nothing, 1 full, 2 partial
int16_t BTreeGeneric::mergeLeftIntoRight(ExclusiveGuardedBufferFrame<BTreeNode>& xGuardedParent,
                                         int16_t lhsSlotId,
                                         ExclusiveGuardedBufferFrame<BTreeNode>& xGuardedLeft,
                                         ExclusiveGuardedBufferFrame<BTreeNode>& xGuardedRight,
                                         bool fullMergeOrNothing) {
  // TODO: corner cases: new upper fence is larger than the older one.
  uint32_t spaceUpperBound = xGuardedLeft->MergeSpaceUpperBound(xGuardedRight);
  if (spaceUpperBound <= BTreeNode::Size()) {
    // Do a full merge TODO: threshold
    bool succ = xGuardedLeft->merge(lhsSlotId, xGuardedParent, xGuardedRight);
    static_cast<void>(succ);
    assert(succ);
    xGuardedLeft.Reclaim();
    return 1;
  }

  if (fullMergeOrNothing)
    return 0;

  // Do a partial merge
  // Remove a key at a time from the merge and check if now it fits
  int16_t tillSlotId = -1;
  for (int16_t i = 0; i < xGuardedLeft->mNumSlots; i++) {
    spaceUpperBound -=
        sizeof(BTreeNodeSlot) + xGuardedLeft->KeySizeWithoutPrefix(i) + xGuardedLeft->ValSize(i);
    if (spaceUpperBound + (xGuardedLeft->GetFullKeyLen(i) - xGuardedRight->mLowerFence.mSize) <
        BTreeNode::Size() * 1.0) {
      tillSlotId = i + 1;
      break;
    }
  }
  if (!(tillSlotId != -1 && tillSlotId < (xGuardedLeft->mNumSlots - 1))) {
    return 0; // false
  }

  assert((spaceUpperBound + (xGuardedLeft->GetFullKeyLen(tillSlotId - 1) -
                             xGuardedRight->mLowerFence.mSize)) < BTreeNode::Size() * 1.0);
  assert(tillSlotId > 0);

  uint16_t copyFromCount = xGuardedLeft->mNumSlots - tillSlotId;

  uint16_t newLeftUpperFenceSize = xGuardedLeft->GetFullKeyLen(tillSlotId - 1);
  ENSURE(newLeftUpperFenceSize > 0);
  auto newLeftUpperFenceBuf = utils::JumpScopedArray<uint8_t>(newLeftUpperFenceSize);
  auto* newLeftUpperFence = newLeftUpperFenceBuf->get();
  xGuardedLeft->CopyFullKey(tillSlotId - 1, newLeftUpperFence);

  if (!xGuardedParent->PrepareInsert(newLeftUpperFenceSize, 0)) {
    return 0; // false
  }

  auto nodeBuf = utils::JumpScopedArray<uint8_t>(BTreeNode::Size());
  {
    Slice newLowerFence{newLeftUpperFence, newLeftUpperFenceSize};
    Slice newUpperFence{xGuardedRight->GetUpperFence()};
    auto* tmp = BTreeNode::New(nodeBuf->get(), true, newLowerFence, newUpperFence);

    xGuardedLeft->CopyKeyValueRange(tmp, 0, tillSlotId, copyFromCount);
    xGuardedRight->CopyKeyValueRange(tmp, copyFromCount, 0, xGuardedRight->mNumSlots);
    memcpy(xGuardedRight.GetPagePayloadPtr(), tmp, BTreeNode::Size());
    xGuardedRight->MakeHint();

    // Nothing to do for the right node's separator
    assert(xGuardedRight->CompareKeyWithBoundaries(newLowerFence) == 1);
  }

  {
    Slice newLowerFence{xGuardedLeft->GetLowerFence()};
    Slice newUpperFence{newLeftUpperFence, newLeftUpperFenceSize};
    auto* tmp = BTreeNode::New(nodeBuf->get(), true, newLowerFence, newUpperFence);

    xGuardedLeft->CopyKeyValueRange(tmp, 0, 0, xGuardedLeft->mNumSlots - copyFromCount);
    memcpy(xGuardedLeft.GetPagePayloadPtr(), tmp, BTreeNode::Size());
    xGuardedLeft->MakeHint();

    assert(xGuardedLeft->CompareKeyWithBoundaries(newUpperFence) == 0);

    xGuardedParent->RemoveSlot(lhsSlotId);
    ENSURE(xGuardedParent->PrepareInsert(xGuardedLeft->mUpperFence.mSize, sizeof(Swip)));
    auto swip = xGuardedLeft.swip();
    Slice key = xGuardedLeft->GetUpperFence();
    Slice val(reinterpret_cast<uint8_t*>(&swip), sizeof(Swip));
    xGuardedParent->Insert(key, val);
  }
  return 2;
}

// returns true if it has exclusively locked anything
BTreeGeneric::XMergeReturnCode BTreeGeneric::XMerge(GuardedBufferFrame<BTreeNode>& guardedParent,
                                                    GuardedBufferFrame<BTreeNode>& guardedChild,
                                                    ParentSwipHandler& parentHandler) {
  if (guardedChild->FillFactorAfterCompaction() >= 0.9) {
    return XMergeReturnCode::kNothing;
  }

  const int64_t maxMergePages = mStore->mStoreOption->mXMergeK;
  GuardedBufferFrame<BTreeNode> guardedNodes[maxMergePages];
  bool fullyMerged[maxMergePages];

  int64_t pos = parentHandler.mPosInParent;
  int64_t pageCount = 1;
  int64_t maxRight;

  guardedNodes[0] = std::move(guardedChild);
  fullyMerged[0] = false;
  double totalFillFactor = guardedNodes[0]->FillFactorAfterCompaction();

  // Handle upper swip instead of avoiding guardedParent->mNumSlots -1 swip
  if (isMetaNode(guardedParent) || !guardedNodes[0]->mIsLeaf) {
    guardedChild = std::move(guardedNodes[0]);
    return XMergeReturnCode::kNothing;
  }
  for (maxRight = pos + 1;
       (maxRight - pos) < maxMergePages && (maxRight + 1) < guardedParent->mNumSlots; maxRight++) {
    if (!guardedParent->ChildSwip(maxRight)->IsHot()) {
      guardedChild = std::move(guardedNodes[0]);
      return XMergeReturnCode::kNothing;
    }

    guardedNodes[maxRight - pos] = GuardedBufferFrame<BTreeNode>(
        mStore->mBufferManager.get(), guardedParent, *guardedParent->ChildSwip(maxRight));
    fullyMerged[maxRight - pos] = false;
    totalFillFactor += guardedNodes[maxRight - pos]->FillFactorAfterCompaction();
    pageCount++;
    if ((pageCount - std::ceil(totalFillFactor)) >= (1)) {
      // we can probably save a page by merging all together so there is no need
      // to look furhter
      break;
    }
  }
  if (((pageCount - std::ceil(totalFillFactor))) < (1)) {
    guardedChild = std::move(guardedNodes[0]);
    return XMergeReturnCode::kNothing;
  }

  ExclusiveGuardedBufferFrame<BTreeNode> xGuardedParent = std::move(guardedParent);
  // TODO(zz-jason): support wal and sync system tx id
  // TXID sysTxId = utils::tlsStore->AllocSysTxTs();
  // xGuardedParent.SyncSystemTxId(sysTxId);

  XMergeReturnCode retCode = XMergeReturnCode::kPartialMerge;
  int16_t leftHand, rightHand, ret;
  while (true) {
    for (rightHand = maxRight; rightHand > pos; rightHand--) {
      if (fullyMerged[rightHand - pos]) {
        continue;
      }
      break;
    }
    if (rightHand == pos)
      break;

    leftHand = rightHand - 1;

    {
      ExclusiveGuardedBufferFrame<BTreeNode> xGuardedRight(
          std::move(guardedNodes[rightHand - pos]));
      ExclusiveGuardedBufferFrame<BTreeNode> xGuardedLeft(std::move(guardedNodes[leftHand - pos]));
      // TODO(zz-jason): support wal and sync system tx id
      // xGuardedRight.SyncSystemTxId(sysTxId);
      // xGuardedLeft.SyncSystemTxId(sysTxId);
      maxRight = leftHand;
      ret = mergeLeftIntoRight(xGuardedParent, leftHand, xGuardedLeft, xGuardedRight,
                               leftHand == pos);
      // we unlock only the left page, the right one should not be touched again
      if (ret == 1) {
        fullyMerged[leftHand - pos] = true;
        retCode = XMergeReturnCode::kFullMerge;
      } else if (ret == 2) {
        guardedNodes[leftHand - pos] = std::move(xGuardedLeft);
      } else if (ret == 0) {
        break;
      } else {
        Log::Fatal("Invalid return code from mergeLeftIntoRight");
      }
    }
  }
  if (guardedChild.mGuard.mState == GuardState::kMoved) {
    guardedChild = std::move(guardedNodes[0]);
  }
  guardedParent = std::move(xGuardedParent);
  return retCode;
}

// -------------------------------------------------------------------------------------
// Helpers
// -------------------------------------------------------------------------------------
int64_t BTreeGeneric::iterateAllPages(BTreeNodeCallback inner, BTreeNodeCallback leaf) {
  while (true) {
    JUMPMU_TRY() {
      GuardedBufferFrame<BTreeNode> guardedParent(mStore->mBufferManager.get(), mMetaNodeSwip);
      GuardedBufferFrame<BTreeNode> guardedChild(mStore->mBufferManager.get(), guardedParent,
                                                 guardedParent->mRightMostChildSwip);
      int64_t result = iterateAllPagesRecursive(guardedChild, inner, leaf);
      JUMPMU_RETURN result;
    }
    JUMPMU_CATCH() {
    }
  }
}

int64_t BTreeGeneric::iterateAllPagesRecursive(GuardedBufferFrame<BTreeNode>& guardedNode,
                                               BTreeNodeCallback inner, BTreeNodeCallback leaf) {
  if (guardedNode->mIsLeaf) {
    return leaf(guardedNode.ref());
  }
  int64_t res = inner(guardedNode.ref());
  for (uint16_t i = 0; i < guardedNode->mNumSlots; i++) {
    auto* childSwip = guardedNode->ChildSwip(i);
    auto guardedChild =
        GuardedBufferFrame<BTreeNode>(mStore->mBufferManager.get(), guardedNode, *childSwip);
    guardedChild.JumpIfModifiedByOthers();
    res += iterateAllPagesRecursive(guardedChild, inner, leaf);
  }

  Swip& childSwip = guardedNode->mRightMostChildSwip;
  auto guardedChild =
      GuardedBufferFrame<BTreeNode>(mStore->mBufferManager.get(), guardedNode, childSwip);
  guardedChild.JumpIfModifiedByOthers();
  res += iterateAllPagesRecursive(guardedChild, inner, leaf);

  return res;
}

std::string BTreeGeneric::Summary() {
  GuardedBufferFrame<BTreeNode> guardedMeta(mStore->mBufferManager.get(), mMetaNodeSwip);
  GuardedBufferFrame<BTreeNode> guardedRoot(mStore->mBufferManager.get(), guardedMeta,
                                            guardedMeta->mRightMostChildSwip);
  uint64_t numAllPages = CountAllPages();
  return std::format("entries={}, nodes={}, innerNodes={}, spacePct={:.2f}, height={}"
                     ", rootSlots={}, freeSpaceAfterCompaction={}",
                     CountEntries(), numAllPages, CountInnerPages(),
                     (numAllPages * BTreeNode::Size()) /
                         (double)mStore->mStoreOption->mBufferPoolSize,
                     GetHeight(), guardedRoot->mNumSlots, FreeSpaceAfterCompaction());
}

StringMap BTreeGeneric::Serialize() {
  LS_DCHECK(mMetaNodeSwip.AsBufferFrame().mPage.mBTreeId == mTreeId);
  auto& metaBf = mMetaNodeSwip.AsBufferFrame();
  auto metaPageId = metaBf.mHeader.mPageId;
  auto res = mStore->mBufferManager->CheckpointBufferFrame(metaBf);
  if (!res) {
    Log::Fatal("Failed to checkpoint meta node: {}", res.error().ToString());
  }
  return {{kTreeId, std::to_string(mTreeId)},
          {kHeight, std::to_string(GetHeight())},
          {kMetaPageId, std::to_string(metaPageId)}};
}

void BTreeGeneric::Deserialize(StringMap map) {
  mTreeId = std::stoull(map[kTreeId]);
  mHeight = std::stoull(map[kHeight]);
  mMetaNodeSwip.Evict(std::stoull(map[kMetaPageId]));

  // load meta node to memory
  HybridLatch dummyLatch;
  HybridGuard dummyGuard(&dummyLatch);
  dummyGuard.ToOptimisticSpin();
  while (true) {
    JUMPMU_TRY() {
      mMetaNodeSwip = mStore->mBufferManager->ResolveSwipMayJump(dummyGuard, mMetaNodeSwip);
      JUMPMU_BREAK;
    }
    JUMPMU_CATCH() {
    }
  }
  mMetaNodeSwip.AsBufferFrame().mHeader.mKeepInMemory = true;
  LS_DCHECK(mMetaNodeSwip.AsBufferFrame().mPage.mBTreeId == mTreeId,
            "MetaNode has wrong BTreeId, pageId={}, expected={}, actual={}",
            mMetaNodeSwip.AsBufferFrame().mHeader.mPageId, mTreeId,
            mMetaNodeSwip.AsBufferFrame().mPage.mBTreeId);
}

void BTreeGeneric::ToJson(BTreeGeneric& btree, rapidjson::Document* resultDoc) {
  LS_DCHECK(resultDoc->IsObject());
  auto& allocator = resultDoc->GetAllocator();

  // meta node
  GuardedBufferFrame<BTreeNode> guardedMetaNode(btree.mStore->mBufferManager.get(),
                                                btree.mMetaNodeSwip);
  rapidjson::Value metaJson(rapidjson::kObjectType);
  utils::ToJson(guardedMetaNode.mBf, &metaJson, &allocator);
  resultDoc->AddMember("metaNode", metaJson, allocator);

  // root node
  GuardedBufferFrame<BTreeNode> guardedRootNode(btree.mStore->mBufferManager.get(), guardedMetaNode,
                                                guardedMetaNode->mRightMostChildSwip);
  rapidjson::Value rootJson(rapidjson::kObjectType);
  toJsonRecursive(btree, guardedRootNode, &rootJson, allocator);
  resultDoc->AddMember("rootNode", rootJson, allocator);
}

void BTreeGeneric::toJsonRecursive(BTreeGeneric& btree, GuardedBufferFrame<BTreeNode>& guardedNode,
                                   rapidjson::Value* resultObj,
                                   rapidjson::Value::AllocatorType& allocator) {

  LS_DCHECK(resultObj->IsObject());
  // buffer frame header
  utils::ToJson(guardedNode.mBf, resultObj, &allocator);

  // btree node
  {
    rapidjson::Value nodeObj(rapidjson::kObjectType);
    utils::ToJson(guardedNode.ptr(), &nodeObj, &allocator);
    resultObj->AddMember("pagePayload(btreeNode)", nodeObj, allocator);
  }

  if (guardedNode->mIsLeaf) {
    return;
  }

  rapidjson::Value childrenJson(rapidjson::kArrayType);
  for (auto i = 0u; i < guardedNode->mNumSlots; ++i) {
    auto* childSwip = guardedNode->ChildSwip(i);
    GuardedBufferFrame<BTreeNode> guardedChild(btree.mStore->mBufferManager.get(), guardedNode,
                                               *childSwip);

    rapidjson::Value childObj(rapidjson::kObjectType);
    toJsonRecursive(btree, guardedChild, &childObj, allocator);
    guardedChild.unlock();

    childrenJson.PushBack(childObj, allocator);
  }

  if (guardedNode->mRightMostChildSwip != nullptr) {
    GuardedBufferFrame<BTreeNode> guardedChild(btree.mStore->mBufferManager.get(), guardedNode,
                                               guardedNode->mRightMostChildSwip);
    rapidjson::Value childObj(rapidjson::kObjectType);
    toJsonRecursive(btree, guardedChild, &childObj, allocator);
    guardedChild.unlock();

    childrenJson.PushBack(childObj, allocator);
  }

  // children
  resultObj->AddMember("mChildren", childrenJson, allocator);
}

} // namespace leanstore::storage::btree
