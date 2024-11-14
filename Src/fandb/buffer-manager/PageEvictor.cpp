namespace fandb::storage
{

    void PageEvictor::PickBufferFramesToCool(leanstore::storage::Partition &targetPartition)
    {
        LS_DLOG("Phase1: PickBufferFramesToCool begins");
        SCOPED_DEFER(LS_DLOG("Phase1: PickBufferFramesToCool ended, mEvictCandidateBfs.size={}",
                             mEvictCandidateBfs.size()));

        // 当前并不需要更多free bf, 所以逻辑走不进去
        if (targetPartition.NeedMoreFreeBfs() && failedAttempts < 10)
        {
        }
    }

    void PageEvictor::PrepareAsyncWriteBuffer(leanstore::storage::Partition &targetPartition)
    {
        LS_DLOG("Phase2: PrepareAsyncWriteBuffer begins");
        SCOPED_DEFER(LS_DLOG("Phase2: PrepareAsyncWriteBuffer ended, "
                             "mAsyncWriteBuffer.PendingRequests={}",
                             mAsyncWriteBuffer.GetPendingRequests()));

        //TODO
        mFreeBfList.Reset();
        for(auto* cooledBf: mEvictCandidateBfs){
            JUMPMU_TRY() {
                BMOptimisticGuard optimisticGuard(cooledBf->mHeader.mLatch);
            }
        }

        mEvictCandidateBfs.clear();
    }
}