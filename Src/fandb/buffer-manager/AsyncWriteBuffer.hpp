#pragma once

#include "leanstore/Units.hpp"
#include "leanstore/buffer-manager/BufferFrame.hpp"
#include "leanstore/utils/AsyncIo.hpp"
#include "leanstore/utils/Misc.hpp"
#include "leanstore/utils/Result.hpp"

#include <cstdint>
#include <functional>
#include <vector>

#include <libaio.h>

namespace fandb::storage
{

    class AsyncWriteBuffer
    {
    private:
        struct WriteCommand
        {
            const leanstore::storage::BufferFrame *mBf;
            PID mPageId;

            void Reset(const leanstore::storage::BufferFrame *bf, PID pageId)
            {
                mBf = bf;
                mPageId = pageId;
            }
        };

        leanstore::utils::AsyncIo mAIo;
        int mFd;
        uint64_t mPageSize;

        leanstore::utils::AlignedBuffer<512> mWriteBuffer;
        std::vector<WriteCommand> mWriteCommands;

    public:
        AsyncWriteBuffer(int fd, uint64_t pageSize, uint64_t maxBatchSize);

        ~AsyncWriteBuffer();

        bool IsFull();

        void Add(const leanstore::storage::BufferFrame &bf);

        leanstore::Result<uint64_t> SubmitAll();
        leanstore::Result<uint64_t> WaitAll();
        uint64_t GetPendingRequests();

        void IterateFlushedBfs(
            std::function<void(leanstore::storage::BufferFrame &flushedBf, uint64_t flushedPsn)> callback,
            uint64_t numFlushedBfs);

    private:
        // 将page拷贝到mWriteBuffer[slot]处
        // 返回的是mWriteBuffer中写入的地址
        void *copyToBuffer(const leanstore::storage::Page *page, size_t slot)
        {
            void *dest = getWriteBuffer(slot);
            std::memcpy(dest, page, mPageSize);
            return dest;
        }

        uint8_t *getWriteBuffer(size_t slot)
        {
            return &mWriteBuffer.Get()[slot * mPageSize];
        }
    };

}