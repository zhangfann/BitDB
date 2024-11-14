
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <format>
#include <vector>
#include <gtest/gtest.h>
#include <fcntl.h>

#include "leanstore/buffer-manager/BufferFrame.hpp"
#include "leanstore/buffer-manager/Swip.hpp"
#include "leanstore/utils/Defer.hpp"
#include "leanstore/utils/Log.hpp"
#include "leanstore/utils/Misc.hpp"
#include "leanstore/utils/RandomGenerator.hpp"

#include "fandb/buffer-manager/AsyncWriteBuffer.hpp"

namespace fandb::storage::test
{

    class AsyncWriteBufferTest : public ::testing::Test
    {

    protected:
        std::string mTestDir = "/tmp/leanstore/AsyncWriteBufferTest";
        struct BufferFrameHolder
        {
            leanstore::utils::AlignedBuffer<512> mBuffer;
            leanstore::storage::BufferFrame *mBf;
            BufferFrameHolder(size_t pageSize, PID pageId)
                : mBuffer(512 + pageSize),
                  mBf(new(mBuffer.Get()) leanstore::storage::BufferFrame()) // 在mBuffer的位置上 构造一个BufferFrame
            {
                mBf->Init(pageId);
            }
        };

        void SetUp() override
        {
        }

        void TearDown() override
        {
        }

        std::string getRandTestFile()
        {
            return std::format("{}/{}", mTestDir, leanstore::utils::RandomGenerator::RandAlphString(8));
        }

        int openFile(const std::string &fileName)
        {
            // open the file
            auto flag = O_TRUNC | O_CREAT | O_RDWR | O_DIRECT;
            int fd = open(fileName.c_str(), flag, 0666);
            EXPECT_NE(fd, -1) << std::format("Failed to open file, fileName={}, errno={}, error={}",
                                             fileName, errno, strerror(errno));

            return fd;
        }

        void closeFile(int fd)
        {
            ASSERT_EQ(close(fd), 0) << std::format("Failed to close file, fd={}, errno={}, error={}", fd,
                                                   errno, strerror(errno));
        }

        void removeFile(const std::string &fileName)
        {
            ASSERT_EQ(remove(fileName.c_str()), 0) << std::format(
                "Failed to remove file, fileName={}, errno={}, error={}", fileName, errno, strerror(errno));
        }
    };

    TEST_F(AsyncWriteBufferTest, Basic)
    {

        auto testFile = getRandTestFile();
        auto testFd = openFile(testFile);
        SCOPED_DEFER({
            closeFile(testFd);
            leanstore::Log::Info("Test file={}", testFile);
        })

            auto testMaxBatchSize = 8;
        auto testPageSize = 512;
        std::vector<std::unique_ptr<BufferFrameHolder>> bfHolders; // 存放buffer frame用于之后比较
        AsyncWriteBuffer testWriteBuffer(testFd, testPageSize, testMaxBatchSize);
        // 写满buffer
        for (int i = 0; i < testMaxBatchSize; i++)
        {
            bfHolders.push_back(std::make_unique<BufferFrameHolder>(testPageSize, i)); // !? 一个buffer页面的大小为512+ 512byte, i是pageId
            EXPECT_FALSE(testWriteBuffer.IsFull());
            *reinterpret_cast<int64_t *>(bfHolders[i]->mBf->mPage.mPayload) = i; // 这里应该是写入页面内容为i
            testWriteBuffer.Add(*bfHolders[i]->mBf);
        }
        EXPECT_TRUE(testWriteBuffer.IsFull());

        // 提交io
        auto result = testWriteBuffer.SubmitAll();
        EXPECT_EQ(result.value(), testMaxBatchSize);

        // 等待io请求完成
        result = testWriteBuffer.WaitAll();
        auto doneRequests = result.value();
        EXPECT_EQ(doneRequests, testMaxBatchSize);
        EXPECT_EQ(testWriteBuffer.GetPendingRequests(), 0);

        // 检查当前buffer
        testWriteBuffer.IterateFlushedBfs(
            [](leanstore::storage::BufferFrame &flushedBf, uint64_t flushedPsn)
            {
                EXPECT_FALSE(flushedBf.IsDirty());
                EXPECT_FALSE(flushedBf.IsFree());
                EXPECT_EQ(flushedPsn, 0);
            },
            testMaxBatchSize);

        // 读出缓存内容
        for (int i = 0; i < testMaxBatchSize; i++)
        {
            BufferFrameHolder bfHolder(testPageSize, i);
            // 从testFd 读取到 bfHolder.mBuffer
            auto ret = pread(testFd, reinterpret_cast<void *>(bfHolder.mBuffer.Get() + 512), testPageSize, testPageSize * i);
            EXPECT_EQ(ret, testPageSize); // 读出来数据大小 等于页面大小
            // 比较页面内容
            auto payload = *reinterpret_cast<int64_t *>(bfHolder.mBf->mPage.mPayload);
            EXPECT_EQ(payload, i);
        }
    }
}
