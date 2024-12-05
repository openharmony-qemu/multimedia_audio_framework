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
#ifndef LOG_TAG
#define LOG_TAG "AudioDumpPCM"
#endif

#include "audio_dump_pcm.h"
#include "audio_dump_pcm_private.h"

#include <string.h>
#include <utility>

#include "media_monitor_manager.h"
#include "callback_handler.h"
#include "audio_log.h"
#include "audio_errors.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {

constexpr uint64_t MAX_LIMIT_BUFFER_SIZE = 100 * 1024 * 1024;   // 100M
constexpr size_t EACH_CHUNK_SIZE = 1024 * 1024;                 // 1M
constexpr int64_t MEMBLOCK_RELEASE_TIME = 5 * 60 * 1000000;     // release pcm 5min ago
constexpr int64_t MEMBLOCK_CHECK_TIME_MS = 30 * 1000;           // check cached data's time every 30s
constexpr int64_t MEMORY_PRINT_TIME_MS = 30 * 1000;             // print memory info every 30s
constexpr uint16_t FILENAME_ID_MAX_INDEX = 65530;               // FileNameId 0 - 65530
constexpr size_t FILENAME_AND_ID_SIZE = 128;                    // estimate each entry size in map

MemChunk::MemChunk() : totalBufferSize_(EACH_CHUNK_SIZE), pointerOffset_(0), curFileNameId_(0)
{
    Trace trace("MemChunk::MemChunk");
    firstMemBlockTime_ = ClockTime::GetRealNano();
    lastMemBlockTime_ = firstMemBlockTime_;
    bufferPointer_ = new (std::nothrow) uint8_t[EACH_CHUNK_SIZE];
    if (bufferPointer_ == nullptr) {
        AUDIO_ERR_LOG("failed to get memory!");
    }
    memBlockDeque_ = std::make_shared<std::deque<MemBlock>>();
}

MemChunk::~MemChunk()
{
    Trace trace("MemChunk::~MemChunk");
    if (bufferPointer_ != nullptr) {
        delete bufferPointer_;
    }
    bufferPointer_ = nullptr;
}

int32_t MemChunk::GetMemBlock(size_t dataLength, std::string& dumpFileName, MemBlock& curMemBlock)
{
    CHECK_AND_RETURN_RET(totalBufferSize_ - pointerOffset_ >= dataLength, ERROR);
    Trace trace("MemChunk::GetMemBlock");

    if (fileNameIdMap_.find(dumpFileName) != fileNameIdMap_.end()) {
        curMemBlock.dumpFileNameId_ = fileNameIdMap_[dumpFileName];
    } else {
        curMemBlock.dumpFileNameId_ = (curFileNameId_ + 1 >= FILENAME_ID_MAX_INDEX) ? 0 : ++curFileNameId_;
        fileNameIdMap_[dumpFileName] = curFileNameId_;
        idFileNameMap_[curFileNameId_] = dumpFileName;
    }
    
    curMemBlock.dataPointer_ = bufferPointer_ + pointerOffset_;
    curMemBlock.dataLength_ = dataLength;
    pointerOffset_ += dataLength;
    lastMemBlockTime_ = ClockTime::GetRealNano();
    memBlockDeque_->push_back(curMemBlock);
    return SUCCESS;
}

std::shared_ptr<std::deque<MemBlock>> MemChunk::GetMemBlockDeque()
{
    return memBlockDeque_;
}

void MemChunk::GetMemChunkDuration(int64_t& startTime, int64_t& endTime)
{
    startTime = firstMemBlockTime_;
    endTime = lastMemBlockTime_;
}

int32_t MemChunk::GetCurUsedMemory(size_t& dataLength, size_t& bufferLength, size_t& structLength)
{
    dataLength = pointerOffset_;
    bufferLength = totalBufferSize_;
    structLength = sizeof(MemBlock) * memBlockDeque_->size() + sizeof(MemChunk) + 
        FILENAME_AND_ID_SIZE * idFileNameMap_.size() * 2;
    return SUCCESS;
}

void MemChunk::Reset()
{
    Trace trace("MemChunk::Reset");
    pointerOffset_ = 0;
    curFileNameId_ = 0;
    firstMemBlockTime_ = ClockTime::GetRealNano();
    lastMemBlockTime_ = firstMemBlockTime_;
    idFileNameMap_ = {};
    fileNameIdMap_ = {};
    memBlockDeque_->clear();
}

AudioCacheMgr& AudioCacheMgr::GetInstance()
{
    static AudioCacheMgrInner mgr;
    return mgr;
}

AudioCacheMgrInner::AudioCacheMgrInner()
{
    Trace trace("AudioCacheMgrInner::AudioCacheMgrInner");
    totalMemChunkNums_ = MAX_LIMIT_BUFFER_SIZE / EACH_CHUNK_SIZE;
}

AudioCacheMgrInner::~AudioCacheMgrInner()
{
    std::lock_guard<std::mutex> runnerlock(runnerMutex_);
    if (callbackHandler_ != nullptr) {
        AUDIO_INFO_LOG("runner move");
        callbackHandler_->ReleaseEventRunner();
        callbackHandler_ = nullptr;
    }
}

bool AudioCacheMgrInner::Init()
{
    if (isInited_ == false) {
        InitCallbackHandler();
        isInited_ = true;
        return true;
    }
    AUDIO_WARNING_LOG("AudioCacheMgr is Inited!");
    return true;
}

bool AudioCacheMgrInner::DeInit()
{
    Trace trace("AudioCacheMgrInner::DeInit");
    std::unique_lock<std::mutex> lock(runnerMutex_);
    if (callbackHandler_ != nullptr) {
        callbackHandler_->ReleaseEventRunner();
        callbackHandler_ = nullptr;
        handler_ = nullptr;

        // clear all cached pcm
        memChunkDeque_ = {};
        AUDIO_INFO_LOG("deinit handler success");
    }
    lock.unlock();
    isInited_ = false;
    AUDIO_WARNING_LOG("AudioCacheMgr is DeInited!");
    return true;
}

void AudioCacheMgrInner::CacheData(std::string& dumpFileName, void* srcDataPointer, size_t dataLength)
{
    if (!isInited_.load()) {
        Trace trace("AudioCacheMgrInner::CacheData::NotInited");
        return;
    }
    Trace trace("AudioCacheMgrInner::CacheData");
    if (isDumpingData_.load()) {
        Trace trace("AudioCacheMgrInner::CacheData::ReturnWhenDumpingData");
        return;
    }
    if (!g_Mutex.try_lock()) {
        if (isDumpingData_.load()) {
            Trace trace("AudioCacheMgrInner::CacheData::TryLockFailedWhenDumpingData");
            return;
        }
        g_Mutex.lock();
    }

    MemBlock curMemBlock {nullptr,0,0};
    int ret = GetAvailableMemBlock(dataLength, dumpFileName, curMemBlock);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("GetAvailableMemBlock failed. Unable to cacheData!");
        g_Mutex.unlock();
        return;
    }
    g_Mutex.unlock();

    ret = memcpy_s(curMemBlock.dataPointer_, dataLength, srcDataPointer, dataLength);
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "memcpy_s failed. Unable to cacheData!");
}

int32_t AudioCacheMgrInner::GetAvailableMemBlock(size_t dataLength, std::string& dumpFileName, MemBlock& curMemBlock)
{
    if (!memChunkDeque_.empty()) {
        Trace trace1("AudioCacheMgrInner::GetAvailableMemBlock::hasNotFillMemChunk");
        std::shared_ptr<MemChunk> lastMemChunk = memChunkDeque_.back();
        if (lastMemChunk->GetMemBlock(dataLength, dumpFileName, curMemBlock) == SUCCESS) {
            return SUCCESS;
        }
    }

    if (memChunkDeque_.size() < totalMemChunkNums_) {
        Trace trace2("AudioCacheMgrInner::getAvailMemBlock::GetMemBlock");
        std::shared_ptr<MemChunk> newMemChunk = std::make_shared<MemChunk>();
        Trace::Count("UsedMemChunk", memChunkDeque_.size());
        memChunkDeque_.push_back(newMemChunk);
        if (newMemChunk->GetMemBlock(dataLength, dumpFileName, curMemBlock) == SUCCESS) {
            return SUCCESS;
        }
    }

    if (!memChunkDeque_.empty()) {
        Trace trace3("AudioCacheMgrInner::GetAvailableMemBlock::RecycleOneMemChunk");
        std::shared_ptr<MemChunk> recycleMemChunk = memChunkDeque_.front();
        memChunkDeque_.pop_front();
        recycleMemChunk->Reset();
        memChunkDeque_.push_back(recycleMemChunk);
        if (recycleMemChunk->GetMemBlock(dataLength, dumpFileName, curMemBlock) == SUCCESS) {
            return SUCCESS;
        }
    }

    AUDIO_ERR_LOG("failed to get available memBlock");
    return ERROR;
}

int32_t AudioCacheMgrInner::DumpAllMemBlock()
{
    if (!isInited_.load()) {
        Trace trace("AudioCacheMgrInner::DumpAllMemBlock::NotInited");
        AUDIO_WARNING_LOG("not inited!");
        return ERR_ILLEGAL_STATE;
    }

    Trace trace("AudioCacheMgrInner::DumpAllMemBlock");
    bool targetStatus = false;
    if (!isDumpingData_.compare_exchange_strong(targetStatus, true)) {
        AUDIO_WARNING_LOG("Already in dumping data!");
    }

    std::lock_guard<std::mutex> processsLock(g_Mutex);

    std::vector<std::pair<std::string, std::string>> paramStart;
    paramStart.push_back({"BETA","true"});
    Media::MediaMonitor::MediaMonitorManager::GetInstance().SetMediaParameters(paramStart);

    while(!memChunkDeque_.empty()) {
        std::shared_ptr<MemChunk> curMemChunk = memChunkDeque_.front();
        memChunkDeque_.pop_front();

        std::shared_ptr<std::deque<MemBlock>> curMemBlockDeque = curMemChunk->GetMemBlockDeque();
        for (auto it = curMemBlockDeque->begin(); it != curMemBlockDeque->end(); ++it) {
            Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteAudioBuffer(
                    "pcm_dump_" + curMemChunk->idFileNameMap_[it->dumpFileNameId_], it->dataPointer_, it->dataLength_);
        }
        Trace::Count("UsedMemChunk", memChunkDeque_.size());
    }

    std::vector<std::pair<std::string, std::string>> paramEnd;
    paramStart.push_back({"BETA","false"});
    Media::MediaMonitor::MediaMonitorManager::GetInstance().SetMediaParameters(paramEnd);

    isDumpingData_.store(false);
    return SUCCESS;
}

void AudioCacheMgrInner::GetCachedDuration(int64_t& startTime, int64_t& endTime)
{
    Trace trace("AudioCacheMgrInner::GetCacheDuration");
    if (!isInited_.load()) {
        Trace trace("AudioCacheMgrInner::GetCachedDuration::NotInited");
        AUDIO_WARNING_LOG("not inited!");
        return;
    }
    std::lock_guard<std::mutex> processLock(g_Mutex);
    // init but no data in memchunk
    if (memChunkDeque_.size() == 0) {
        startTime = ClockTime::GetRealNano();
        endTime = startTime;
        AUDIO_WARNING_LOG("GetCachedDuration while memChunkDeque_ is empty!");
        return;
    }

    int64_t temp;
    if (memChunkDeque_.front() != nullptr) {
        memChunkDeque_.front()->GetMemChunkDuration(startTime, temp);
    }
    if (memChunkDeque_.back() != nullptr) {
        memChunkDeque_.back()->GetMemChunkDuration(temp, endTime);
    }
    AUDIO_INFO_LOG("startTime:%{public}s, endTime:%{public}s.",
        ClockTime::NanoTimeToString(startTime).c_str(), ClockTime::NanoTimeToString(endTime).c_str());
}

void AudioCacheMgrInner::GetCurMemoryCondition(size_t& dataLength, size_t& bufferLength, size_t& structLength)
{
    Trace trace("AudioCacheMgrInner::GetCurMemoryCondition");
    SafeSendCallBackEvent(PRINT_MEMORY_CONDITION, 0, MEMORY_PRINT_TIME_MS);
    std::lock_guard<std::mutex> processLock(g_Mutex);

    if (memChunkDeque_.empty()) {
        AUDIO_INFO_LOG("cache memory is empty");
    }

    size_t curDataLength = 0;
    size_t curBufferLength = 0;
    size_t curStructLength = 0;
    for (auto it = memChunkDeque_.begin(); it != memChunkDeque_.end(); ++it) {
        (*it)->GetCurUsedMemory(curDataLength, curBufferLength, curStructLength);
        dataLength += curDataLength;
        bufferLength += curBufferLength;
        structLength += curStructLength;
    }
}

void AudioCacheMgrInner::ReleaseOverTimeMemBlock()
{
    Trace trace("AudioCacheMgrInner::ReleaseOverTimeMemBlock");
    SafeSendCallBackEvent(RELEASE_OVERTIME_MEMBLOCK, 0, MEMBLOCK_CHECK_TIME_MS);
    std::lock_guard<std::mutex> processLock(g_Mutex);
    if (isDumpingData_.load()) {
        AUDIO_INFO_LOG("now dumping memblock, no need ReleaseOverTimeMemBlock");
        return;
    }

    int32_t recycleNums = 0;
    int64_t curTime = ClockTime::GetRealNano();
    int64_t startTime, endTime;

    while(!memChunkDeque_.empty()) {
        memChunkDeque_.front()->GetMemChunkDuration(startTime, endTime);
        if (curTime - endTime < MEMBLOCK_RELEASE_TIME * AUDIO_MS_PER_SECOND) {
            break;
        }
        memChunkDeque_.pop_front();
        ++recycleNums;
        Trace::Count("UsedMemChunk", memChunkDeque_.size());
    }
    if (recycleNums != 0) {
        AUDIO_INFO_LOG("CheckMemBlock Recycle %{public}d memBlocks", recycleNums);
    }
}

void AudioCacheMgrInner::InitCallbackHandler()
{
    Trace trace("AudioCacheMgrInner::InitCallbackHandler");
    std::unique_lock<std::mutex> lock(runnerMutex_);
    if (callbackHandler_ == nullptr) {
        handler_ = std::make_shared<AudioCacheHandler>(this);
        callbackHandler_ = CallbackHandler::GetInstance(handler_, "OS_AUDIODumpCB");
        AUDIO_INFO_LOG("init handler success");
    }
    lock.unlock();
    SafeSendCallBackEvent(RELEASE_OVERTIME_MEMBLOCK, 0, MEMBLOCK_CHECK_TIME_MS);
    SafeSendCallBackEvent(PRINT_MEMORY_CONDITION, 0, MEMORY_PRINT_TIME_MS);
}

void AudioCacheMgrInner::SafeSendCallBackEvent(uint32_t eventCode, int64_t data, int64_t delayTime)
{
    Trace trace("AudioCacheMgrInner::SafeSendCallBackEvent");
    std::lock_guard<std::mutex> lock(runnerMutex_);
    CHECK_AND_RETURN_LOG(callbackHandler_ != nullptr, "Runner is Release");

    callbackHandler_->SendCallbackEvent(eventCode, data, delayTime);
}

void AudioCacheMgrInner::OnHandle(uint32_t code, int64_t data)
{
    size_t dataLength = 0;
    size_t bufferLength = 0;
    size_t structLength = 0;

    switch (code) {
        case RELEASE_OVERTIME_MEMBLOCK:
            ReleaseOverTimeMemBlock();
            break;
        case PRINT_MEMORY_CONDITION:
            GetCurMemoryCondition(dataLength, bufferLength, structLength);
            AUDIO_INFO_LOG("dataLength:%{public}zu KB ,bufferLength:%{public}zu KB, structLength:%{public}zu KB", 
                dataLength / 1024, bufferLength / 1024, structLength / 1024);
            break;
        default:
            break;
    }
}

AudioCacheHandler::AudioCacheHandler(IHandler* handler) : handler_(handler) {}

void AudioCacheHandler::OnHandle(uint32_t code, int64_t data)
{
    CHECK_AND_RETURN_LOG(handler_ != nullptr, "handler is nullptr");
    handler_->OnHandle(code,data);
}

} // namespace AudioStandard
} // namespace OHOS





















