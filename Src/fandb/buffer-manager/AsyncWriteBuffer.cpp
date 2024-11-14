#include "AsyncWriteBuffer.hpp"

#include "leanstore/buffer-manager/BufferFrame.hpp"
#include "leanstore/utils/Log.hpp"
#include "leanstore/utils/Result.hpp"

namespace fandb::storage
{

    AsyncWriteBuffer::AsyncWriteBuffer(int fd, uint64_t pageSize, uint64_t maxBatchSize)
        : mFd(fd),
          mPageSize(pageSize),
          mAIo(maxBatchSize),
          mWriteBuffer(pageSize* maxBatchSize),
          mWriteCommands(maxBatchSize)
    {
    }

    AsyncWriteBuffer::~AsyncWriteBuffer()
    {
    }

    bool AsyncWriteBuffer::IsFull(){
        return mAIo.IsFull();
    }

    void AsyncWriteBuffer::Add(const leanstore::storage::BufferFrame& bf)
    {

        auto pageId = bf.mHeader.mPageId;
        auto slot = mAIo.GetNumRequests();
        auto* buffer = copyToBuffer(&bf.mPage, slot); //?? 将buffer拷贝到aio中
        mWriteCommands[slot].Reset(&bf, pageId);

        // 向mFd写入buffer, 大小为mPageSize, offset: mPageSize* pageId
        mAIo.PrepareWrite(mFd, buffer, mPageSize, mPageSize* pageId);
    }

    leanstore::Result<uint64_t> AsyncWriteBuffer::SubmitAll(){
        return mAIo.SubmitAll();
    }

    leanstore::Result<uint64_t> AsyncWriteBuffer::WaitAll(){
        return mAIo.WaitAll();
    }

    uint64_t AsyncWriteBuffer::GetPendingRequests() {
        return mAIo.GetNumRequests();
    }

    void AsyncWriteBuffer::IterateFlushedBfs(
        std::function<void(leanstore::storage::BufferFrame& flushedBf, uint64_t flushedPsn)> callback,
        uint64_t numFlushedBfs
    ){
        for(uint64_t i=0; i< numFlushedBfs; i++){
            const auto slot = (reinterpret_cast<uint64_t>(mAIo.GetIoEvent(i)->data) - reinterpret_cast<uint64_t>(mWriteBuffer.Get())) / mPageSize;
            auto* flushedPage = reinterpret_cast<leanstore::storage::Page*>(getWriteBuffer(slot));
            auto flushedPsn = flushedPage->mPsn;
            auto* flushedBf = mWriteCommands[slot].mBf; // ??
            callback(*const_cast<leanstore::storage::BufferFrame*>(flushedBf), flushedPsn);
        }
    }
}