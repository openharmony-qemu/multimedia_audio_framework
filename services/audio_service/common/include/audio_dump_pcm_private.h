/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef AUDIO_PCM_DUMP_PRIVATE_H
#define AUDIO_PCM_DUMP_PRIVATE_H

#include "audio_dump_pcm.h"

#include <mutex>
#include <deque>
#include <atomic>
#include <unordered_map>

#include "singleton.h"
#include "event_handler.h"
#include "event_runner.h"
#include "callback_handler.h"
#include "timestamp.h"

namespace OHOS {
namespace AudioStandard {

struct MemBlock {
    uint8_t* dataPointer_ = nullptr;
    size_t dataLength_ = 0;
    uint32_t dumpFileNameId_ = 0;
};

class MemChunk {
public:
    MemChunk();
    ~MemChunk();
    int32_t GetMemBlock(size_t dataLength, std::string &dumpFileName, MemBlock &curMemBlock);
    std::shared_ptr<std::deque<MemBlock>> GetMemBlockDeque();
    void GetMemChunkDuration(int64_t &startTime, int64_t &endTime);
    int32_t GetCurUsedMemory(size_t &dataLength, size_t &bufferLength, size_t &structLength);
    void Reset();
public:
    std::unordered_map<uint16_t, std::string> idFileNameMap_{};
private:
    std::unordered_map<std::string, uint16_t> fileNameIdMap_{};
    std::shared_ptr<std::deque<MemBlock>> memBlockDeque_;

    const size_t totalBufferSize_;
    size_t pointerOffset_;
    uint16_t curFileNameId_;
    int64_t firstMemBlockTime_;
    int64_t lastMemBlockTime_;
    uint8_t *bufferPointer_ = nullptr;
};

class AudioCacheHandler : public IHandler {
public:
    AudioCacheHandler(IHandler* handler);
    void OnHandle(uint32_t code, int64_t data) override;
private:
    IHandler *handler_ = nullptr;
};

class AudioCacheMgrInner : public AudioCacheMgr, public IHandler {
public:
    AudioCacheMgrInner();
    ~AudioCacheMgrInner() override;
    void OnHandle(uint32_t code, int64_t data) override;

    bool Init() override;
    bool DeInit() override;

    void CacheData(std::string &dumpFileName, void* dataPointer, size_t dataLength) override;
    int32_t DumpAllMemBlock() override;
    void GetCachedDuration(int64_t &startTime, int64_t &endTime) override;
    void GetCurMemoryCondition(size_t &dataLength, size_t &bufferLength, size_t &structLength) override;
    bool SetDumpParameter(const std::vector<std::pair<std::string, std::string>> &params) override;
    bool GetDumpParameter(const std::vector<std::string> &subKeys,
        std::vector<std::pair<std::string, std::string>> &result) override;

private:
    void InitCallbackHandler();
    int32_t GetAvailableMemBlock(size_t dataLength, std::string &dumpFileName, MemBlock &curMemBlock);
    void SafeSendCallBackEvent(uint32_t eventCode, int64_t data, int64_t delayTime);
    void ReleaseOverTimeMemBlock();
    void PrintCurMemoryCondition();
private:
    std::atomic<bool> isDumpingData_ = {false};
    std::atomic<bool> isInited_ = {false};
    size_t totalMemChunkNums_; // MAX_LIMIT_BUFFER_SIZE / EACH_CHUNK_SIZE
    std::deque<std::shared_ptr<MemChunk>> memChunkDeque_;
    std::mutex g_Mutex;

    // Event handler
    std::shared_ptr<AudioCacheHandler> handler_ = nullptr;
    std::mutex runnerMutex_;
    std::shared_ptr<CallbackHandler> callbackHandler_ = nullptr;

    enum {
        RELEASE_OVERTIME_MEMBLOCK = 0,
        PRINT_MEMORY_CONDITION,
        RAISE_PRIORITY,
    };
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PCM_DUMP_PRIVATE_H