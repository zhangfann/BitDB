#pragma once

#include "leanstore/LeanStore.hpp"
#include "leanstore/buffer-manager/AsyncWriteBuffer.hpp"
#include "leanstore/buffer-manager/BMPlainGuard.hpp"
#include "leanstore/buffer-manager/BufferFrame.hpp"
#include "leanstore/buffer-manager/FreeList.hpp"
#include "leanstore/buffer-manager/Partition.hpp"
#include "leanstore/buffer-manager/Swip.hpp"
#include "leanstore/utils/UserThread.hpp"

#include <cstdint>

#include <fcntl.h>
#include <sys/resource.h>
#include <sys/time.h>
#include <unistd.h>

namespace fandb::storage
{
    class PageEvictor
    {
    private:
        std::vector<std::unique_ptr<leanstore::storage::Partition>> &mPartitions; //??
        FreeBfList mFreeBfList; //??
        std::vector<BufferFrame* > mEvictCandidateBfs; //??

    public:
        void PickBufferFramesToCool(leanstore::storage::Partition &targetPartition);
    };
}
