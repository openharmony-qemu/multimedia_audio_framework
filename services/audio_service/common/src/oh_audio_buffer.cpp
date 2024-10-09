/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "OHAudioBuffer"
#endif

#include "oh_audio_buffer.h"

#include <cinttypes>
#include <climits>
#include <memory>
#include <sys/mman.h>
#include "ashmem.h"

#include "audio_errors.h"
#include "audio_service_log.h"
#include "futex_tool.h"

namespace OHOS {
namespace AudioStandard {
namespace {
    static const int INVALID_FD = -1;
    static const size_t MAX_MMAP_BUFFER_SIZE = 10 * 1024 * 1024; // 10M
    static const std::string STATUS_INFO_BUFFER = "status_info_buffer";
}
class AudioSharedMemoryImpl : public AudioSharedMemory {
public:
    uint8_t *GetBase() override;
    size_t GetSize() override;
    int GetFd() override;
    std::string GetName() override;

    AudioSharedMemoryImpl(size_t size, const std::string &name);

    AudioSharedMemoryImpl(int fd, size_t size, const std::string &name);

    ~AudioSharedMemoryImpl();

    int32_t Init();

private:
    void Close();

    uint8_t *base_;
    int fd_;
    size_t size_;
    std::string name_;
};

AudioSharedMemoryImpl::AudioSharedMemoryImpl(size_t size, const std::string &name)
    : base_(nullptr), fd_(INVALID_FD), size_(size), name_(name)
{
    AUDIO_INFO_LOG("AudioSharedMemory ctor with size: %{public}zu name: %{public}s", size_, name_.c_str());
}

AudioSharedMemoryImpl::AudioSharedMemoryImpl(int fd, size_t size, const std::string &name)
    : base_(nullptr), fd_(dup(fd)), size_(size), name_(name)
{
    AUDIO_INFO_LOG("AudioSharedMemory ctor with fd %{public}d size %{public}zu name %{public}s", fd_, size_,
        name_.c_str());
}

AudioSharedMemoryImpl::~AudioSharedMemoryImpl()
{
    AUDIO_INFO_LOG(" %{public}s enter ~AudioSharedMemoryImpl()", name_.c_str());
    Close();
}

int32_t AudioSharedMemoryImpl::Init()
{
    CHECK_AND_RETURN_RET_LOG((size_ > 0 && size_ < MAX_MMAP_BUFFER_SIZE), ERR_INVALID_PARAM,
        "Init falied: size out of range: %{public}zu", size_);
    bool isFromRemote = false;
    if (fd_ > 0) {
        isFromRemote = true;
        int size = AshmemGetSize(fd_); // hdi fd may not support
        if (size < 0 || static_cast<size_t>(size) != size_) {
            AUDIO_WARNING_LOG("AshmemGetSize faied, get %{public}d", size);
        }
    } else {
        fd_ = AshmemCreate(name_.c_str(), size_);
        CHECK_AND_RETURN_RET_LOG((fd_ > 0), ERR_OPERATION_FAILED, "Init falied: fd %{public}d", fd_);
    }

    void *addr = mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    CHECK_AND_RETURN_RET_LOG(addr != MAP_FAILED, ERR_OPERATION_FAILED, "Init falied: fd %{public}d size %{public}zu",
        fd_, size_);
    base_ = static_cast<uint8_t *>(addr);
    AUDIO_INFO_LOG("Init %{public}s <%{public}s> done.", (isFromRemote ? "remote" : "local"),
        name_.c_str());
    return SUCCESS;
}

void AudioSharedMemoryImpl::Close()
{
    if (base_ != nullptr) {
        (void)munmap(base_, size_);
        base_ = nullptr;
        size_ = 0;
        AUDIO_INFO_LOG("%{public}s munmap done", name_.c_str());
    }

    if (fd_ > 0) {
        (void)close(fd_);
        fd_ = INVALID_FD;
        AUDIO_INFO_LOG("%{public}s close fd done", name_.c_str());
    }
}

uint8_t *AudioSharedMemoryImpl::GetBase()
{
    return base_;
}

size_t AudioSharedMemoryImpl::GetSize()
{
    return size_;
}

std::string AudioSharedMemoryImpl::GetName()
{
    return name_;
}

int AudioSharedMemoryImpl::GetFd()
{
    return fd_;
}

std::shared_ptr<AudioSharedMemory> AudioSharedMemory::CreateFormLocal(size_t size, const std::string &name)
{
    std::shared_ptr<AudioSharedMemoryImpl> sharedMemory = std::make_shared<AudioSharedMemoryImpl>(size, name);
    CHECK_AND_RETURN_RET_LOG(sharedMemory->Init() == SUCCESS,
        nullptr, "CreateFormLocal failed");
    return sharedMemory;
}

std::shared_ptr<AudioSharedMemory> AudioSharedMemory::CreateFromRemote(int fd, size_t size, const std::string &name)
{
    int minfd = 2; // ignore stdout, stdin and stderr.
    CHECK_AND_RETURN_RET_LOG(fd > minfd, nullptr, "CreateFromRemote failed: invalid fd: %{public}d", fd);
    std::shared_ptr<AudioSharedMemoryImpl> sharedMemory = std::make_shared<AudioSharedMemoryImpl>(fd, size, name);
    if (sharedMemory->Init() != SUCCESS) {
        AUDIO_ERR_LOG("CreateFromRemote failed");
        return nullptr;
    }
    return sharedMemory;
}

int32_t AudioSharedMemory::WriteToParcel(const std::shared_ptr<AudioSharedMemory> &memory, MessageParcel &parcel)
{
    std::shared_ptr<AudioSharedMemoryImpl> memoryImpl = std::static_pointer_cast<AudioSharedMemoryImpl>(memory);
    CHECK_AND_RETURN_RET_LOG(memoryImpl != nullptr, ERR_OPERATION_FAILED, "invalid pointer.");

    int32_t fd = memoryImpl->GetFd();

    size_t size = memoryImpl->GetSize();
    CHECK_AND_RETURN_RET_LOG((size > 0 && size < MAX_MMAP_BUFFER_SIZE), ERR_INVALID_PARAM,
        "invalid size: %{public}zu", size);
    uint64_t sizeTmp = static_cast<uint64_t>(size);

    std::string name = memoryImpl->GetName();

    parcel.WriteFileDescriptor(fd);
    parcel.WriteUint64(sizeTmp);
    parcel.WriteString(name);

    return SUCCESS;
}

std::shared_ptr<AudioSharedMemory> AudioSharedMemory::ReadFromParcel(MessageParcel &parcel)
{
    int fd = parcel.ReadFileDescriptor();

    uint64_t sizeTmp = parcel.ReadUint64();
    CHECK_AND_RETURN_RET_LOG((sizeTmp > 0 && sizeTmp < MAX_MMAP_BUFFER_SIZE), nullptr, "failed with invalid size");
    size_t size = static_cast<size_t>(sizeTmp);

    std::string name = parcel.ReadString();

    std::shared_ptr<AudioSharedMemory> memory = AudioSharedMemory::CreateFromRemote(fd, size, name);
    if (memory == nullptr || memory->GetBase() == nullptr) {
        AUDIO_ERR_LOG("ReadFromParcel failed");
        memory = nullptr;
    }
    close(fd);
    return memory;
}

// OHAudioBuffer
OHAudioBuffer::OHAudioBuffer(AudioBufferHolder bufferHolder, uint32_t totalSizeInFrame, uint32_t spanSizeInFrame,
    uint32_t byteSizePerFrame) : bufferHolder_(bufferHolder), totalSizeInFrame_(totalSizeInFrame),
    spanSizeInFrame_(spanSizeInFrame), byteSizePerFrame_(byteSizePerFrame), audioMode_(AUDIO_MODE_PLAYBACK),
    basicBufferInfo_(nullptr), spanInfoList_(nullptr)
{
    AUDIO_INFO_LOG("ctor with holder:%{public}d mode:%{public}d", bufferHolder_, audioMode_);
}

OHAudioBuffer::~OHAudioBuffer()
{
    AUDIO_INFO_LOG("enter ~OHAudioBuffer()");
    basicBufferInfo_ = nullptr;
    spanInfoList_ = nullptr;
    spanConut_ = 0;
}

int32_t OHAudioBuffer::SizeCheck()
{
    if (totalSizeInFrame_ < spanSizeInFrame_ || totalSizeInFrame_ % spanSizeInFrame_ != 0 ||
        totalSizeInFrame_ > UINT_MAX / byteSizePerFrame_) {
        AUDIO_ERR_LOG("failed: invalid size.");
        return ERR_INVALID_PARAM;
    }
    totalSizeInByte_ = totalSizeInFrame_ * byteSizePerFrame_;
    // data buffer size check
    CHECK_AND_RETURN_RET_LOG((totalSizeInByte_ < MAX_MMAP_BUFFER_SIZE), ERR_INVALID_PARAM, "too large totalSizeInByte "
        "%{public}zu", totalSizeInByte_);

    spanSizeInByte_ = spanSizeInFrame_ * byteSizePerFrame_;
    spanConut_ = totalSizeInFrame_ / spanSizeInFrame_;

    return SUCCESS;
}

int32_t OHAudioBuffer::Init(int dataFd, int infoFd)
{
    AUDIO_INFO_LOG("Init with dataFd %{public}d, infoFd %{public}d, bufferSize %{public}d, spanSize %{public}d,"
        " byteSizePerFrame %{public}d", dataFd, infoFd, totalSizeInFrame_, spanSizeInFrame_, byteSizePerFrame_);

    int32_t ret = SizeCheck();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "failed: invalid size.");

    // init for statusInfoBuffer
    size_t statusInfoSize = sizeof(BasicBufferInfo) + spanConut_ * sizeof(SpanInfo);
    if (infoFd != INVALID_FD && (bufferHolder_ == AUDIO_CLIENT || bufferHolder_ == AUDIO_SERVER_INDEPENDENT)) {
        statusInfoMem_ = AudioSharedMemory::CreateFromRemote(infoFd, statusInfoSize, STATUS_INFO_BUFFER);
    } else {
        statusInfoMem_ = AudioSharedMemory::CreateFormLocal(statusInfoSize, STATUS_INFO_BUFFER);
    }
    CHECK_AND_RETURN_RET_LOG(statusInfoMem_ != nullptr, ERR_OPERATION_FAILED, "BasicBufferInfo mmap failed.");

    // init for dataBuffer
    if (dataFd == INVALID_FD && bufferHolder_ == AUDIO_SERVER_SHARED) {
        dataMem_ = AudioSharedMemory::CreateFormLocal(totalSizeInByte_, "server_client_buffer");
    } else {
        std::string memoryDesc = (bufferHolder_ == AUDIO_SERVER_ONLY ? "server_hdi_buffer" : "server_client_buffer");
        dataMem_ = AudioSharedMemory::CreateFromRemote(dataFd, totalSizeInByte_, memoryDesc);
    }
    CHECK_AND_RETURN_RET_LOG(dataMem_ != nullptr, ERR_OPERATION_FAILED, "dataMem_ mmap failed.");

    dataBase_ = dataMem_->GetBase();

    basicBufferInfo_ = reinterpret_cast<BasicBufferInfo *>(statusInfoMem_->GetBase());
    spanInfoList_ = reinterpret_cast<SpanInfo *>(statusInfoMem_->GetBase() + sizeof(BasicBufferInfo));

    // As basicBufferInfo_ is created from memory, we need to set the value with 0.
    basicBufferInfo_->basePosInFrame.store(0);
    basicBufferInfo_->curReadFrame.store(0);
    basicBufferInfo_->curWriteFrame.store(0);

    basicBufferInfo_->underrunCount.store(0);

    basicBufferInfo_->streamVolume.store(MAX_FLOAT_VOLUME);
    basicBufferInfo_->duckFactor.store(MAX_FLOAT_VOLUME);

    if (bufferHolder_ == AUDIO_SERVER_SHARED || bufferHolder_ == AUDIO_SERVER_ONLY) {
        basicBufferInfo_->handlePos.store(0);
        basicBufferInfo_->handleTime.store(0);
        basicBufferInfo_->totalSizeInFrame = totalSizeInFrame_;
        basicBufferInfo_->spanSizeInFrame = spanSizeInFrame_;
        basicBufferInfo_->byteSizePerFrame = byteSizePerFrame_;
        basicBufferInfo_->streamStatus.store(STREAM_INVALID);

        for (uint32_t i = 0; i < spanConut_; i++) {
            spanInfoList_[i].spanStatus.store(SPAN_INVALID);
        }
    }

    AUDIO_INFO_LOG("Init done.");
    return SUCCESS;
}

std::shared_ptr<OHAudioBuffer> OHAudioBuffer::CreateFromLocal(uint32_t totalSizeInFrame, uint32_t spanSizeInFrame,
    uint32_t byteSizePerFrame)
{
    AUDIO_INFO_LOG("totalSizeInFrame %{public}d, spanSizeInFrame %{public}d, byteSizePerFrame"
        " %{public}d", totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    std::shared_ptr<OHAudioBuffer> buffer = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    CHECK_AND_RETURN_RET_LOG(buffer->Init(INVALID_FD, INVALID_FD) == SUCCESS,
        nullptr, "failed to init.");
    return buffer;
}

std::shared_ptr<OHAudioBuffer> OHAudioBuffer::CreateFromRemote(uint32_t totalSizeInFrame, uint32_t spanSizeInFrame,
    uint32_t byteSizePerFrame, AudioBufferHolder bufferHolder, int dataFd, int infoFd)
{
    AUDIO_INFO_LOG("dataFd %{public}d, infoFd %{public}d", dataFd, infoFd);

    int minfd = 2; // ignore stdout, stdin and stderr.
    CHECK_AND_RETURN_RET_LOG(dataFd > minfd, nullptr, "invalid dataFd: %{public}d", dataFd);

    if (infoFd != INVALID_FD) {
        CHECK_AND_RETURN_RET_LOG(infoFd > minfd, nullptr, "invalid infoFd: %{public}d", infoFd);
    }
    std::shared_ptr<OHAudioBuffer> buffer = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    if (buffer->Init(dataFd, infoFd) != SUCCESS) {
        AUDIO_ERR_LOG("failed to init.");
        return nullptr;
    }
    return buffer;
}

int32_t OHAudioBuffer::WriteToParcel(const std::shared_ptr<OHAudioBuffer> &buffer, MessageParcel &parcel)
{
    AUDIO_INFO_LOG("WriteToParcel start.");
    AudioBufferHolder bufferHolder = buffer->GetBufferHolder();
    CHECK_AND_RETURN_RET_LOG(bufferHolder == AudioBufferHolder::AUDIO_SERVER_SHARED ||
        bufferHolder == AudioBufferHolder::AUDIO_SERVER_INDEPENDENT,
        ERROR_INVALID_PARAM, "buffer holder error:%{public}d", bufferHolder);

    parcel.WriteUint32(bufferHolder);
    parcel.WriteUint32(buffer->totalSizeInFrame_);
    parcel.WriteUint32(buffer->spanSizeInFrame_);
    parcel.WriteUint32(buffer->byteSizePerFrame_);

    parcel.WriteFileDescriptor(buffer->dataMem_->GetFd());
    parcel.WriteFileDescriptor(buffer->statusInfoMem_->GetFd());

    AUDIO_INFO_LOG("WriteToParcel done.");
    return SUCCESS;
}

std::shared_ptr<OHAudioBuffer> OHAudioBuffer::ReadFromParcel(MessageParcel &parcel)
{
    AUDIO_INFO_LOG("ReadFromParcel start.");
    uint32_t holder = parcel.ReadUint32();
    AudioBufferHolder bufferHolder = static_cast<AudioBufferHolder>(holder);
    if (bufferHolder != AudioBufferHolder::AUDIO_SERVER_SHARED &&
        bufferHolder != AudioBufferHolder::AUDIO_SERVER_INDEPENDENT) {
        AUDIO_ERR_LOG("ReadFromParcel buffer holder error:%{public}d", bufferHolder);
        return nullptr;
    }
    bufferHolder = bufferHolder == AudioBufferHolder::AUDIO_SERVER_SHARED ?
         AudioBufferHolder::AUDIO_CLIENT : bufferHolder;
    uint32_t totalSizeInFrame = parcel.ReadUint32();
    uint32_t spanSizeInFrame = parcel.ReadUint32();
    uint32_t byteSizePerFrame = parcel.ReadUint32();

    int dataFd = parcel.ReadFileDescriptor();
    int infoFd = parcel.ReadFileDescriptor();

    std::shared_ptr<OHAudioBuffer> buffer = OHAudioBuffer::CreateFromRemote(totalSizeInFrame, spanSizeInFrame,
        byteSizePerFrame, bufferHolder, dataFd, infoFd);
    if (buffer == nullptr) {
        AUDIO_ERR_LOG("ReadFromParcel failed.");
    } else if (totalSizeInFrame != buffer->basicBufferInfo_->totalSizeInFrame ||
        spanSizeInFrame != buffer->basicBufferInfo_->spanSizeInFrame ||
        byteSizePerFrame != buffer->basicBufferInfo_->byteSizePerFrame) {
        AUDIO_WARNING_LOG("data in shared memory diff.");
    } else {
        AUDIO_INFO_LOG("Read some data done.");
    }
    close(dataFd);
    close(infoFd);
    AUDIO_INFO_LOG("ReadFromParcel done.");
    return buffer;
}

AudioBufferHolder OHAudioBuffer::GetBufferHolder()
{
    return bufferHolder_;
}

int32_t OHAudioBuffer::GetSizeParameter(uint32_t &totalSizeInFrame, uint32_t &spanSizeInFrame,
    uint32_t &byteSizePerFrame)
{
    totalSizeInFrame = totalSizeInFrame_;
    spanSizeInFrame = spanSizeInFrame_;
    byteSizePerFrame = byteSizePerFrame_;

    return SUCCESS;
}

std::atomic<StreamStatus> *OHAudioBuffer::GetStreamStatus()
{
    if (basicBufferInfo_ == nullptr) {
        return nullptr;
    }
    return &basicBufferInfo_->streamStatus;
}


float OHAudioBuffer::GetStreamVolume()
{
    CHECK_AND_RETURN_RET_LOG(basicBufferInfo_ != nullptr, MAX_FLOAT_VOLUME, "buffer is not inited!");
    float vol = basicBufferInfo_->streamVolume.load();
    if (vol < MIN_FLOAT_VOLUME) {
        AUDIO_WARNING_LOG("vol < 0.0, invalid volume! using 0.0 instead.");
        return MIN_FLOAT_VOLUME;
    } else if (vol > MAX_FLOAT_VOLUME) {
        AUDIO_WARNING_LOG("vol > 0.0, invalid volume! using 1.0 instead.");
        return MAX_FLOAT_VOLUME;
    }
    return vol;
}

bool OHAudioBuffer::SetStreamVolume(float streamVolume)
{
    CHECK_AND_RETURN_RET_LOG(basicBufferInfo_ != nullptr, false, "buffer is not inited!");
    if (streamVolume < MIN_FLOAT_VOLUME || streamVolume > MAX_FLOAT_VOLUME) {
        AUDIO_ERR_LOG("invlaid volume:%{public}f", streamVolume);
        return false;
    }
    basicBufferInfo_->streamVolume.store(streamVolume);
    return true;
}

float OHAudioBuffer::GetDuckFactor()
{
    CHECK_AND_RETURN_RET_LOG(basicBufferInfo_ != nullptr, MAX_FLOAT_VOLUME, "buffer is not inited!");
    float factor = basicBufferInfo_->duckFactor.load();
    if (factor < MIN_FLOAT_VOLUME) {
        AUDIO_WARNING_LOG("vol < 0.0, invalid duckFactor! using 0.0 instead.");
        return MIN_FLOAT_VOLUME;
    } else if (factor > MAX_FLOAT_VOLUME) {
        AUDIO_WARNING_LOG("vol > 0.0, invalid duckFactor! using 1.0 instead.");
        return MAX_FLOAT_VOLUME;
    }
    return factor;
}

bool OHAudioBuffer::SetDuckFactor(float duckFactor)
{
    CHECK_AND_RETURN_RET_LOG(basicBufferInfo_ != nullptr, false, "buffer is not inited!");
    if (duckFactor < MIN_FLOAT_VOLUME || duckFactor > MAX_FLOAT_VOLUME) {
        AUDIO_ERR_LOG("invlaid factor:%{public}f", duckFactor);
        return false;
    }
    basicBufferInfo_->duckFactor.store(duckFactor);
    return true;
}


uint32_t OHAudioBuffer::GetUnderrunCount()
{
    CHECK_AND_RETURN_RET_LOG(basicBufferInfo_ != nullptr, 0,
        "Get nullptr, buffer is not inited.");
    return basicBufferInfo_->underrunCount.load();
}

bool OHAudioBuffer::SetUnderrunCount(uint32_t count)
{
    CHECK_AND_RETURN_RET_LOG(basicBufferInfo_ != nullptr, false,
        "Get nullptr, buffer is not inited.");
    basicBufferInfo_->underrunCount.store(count);
    return true;
}

bool OHAudioBuffer::GetHandleInfo(uint64_t &frames, int64_t &nanoTime)
{
    CHECK_AND_RETURN_RET_LOG(basicBufferInfo_ != nullptr, false,
        "Get nullptr, failed to GetHandleInfo.");

    frames = basicBufferInfo_->handlePos.load();
    nanoTime = basicBufferInfo_->handleTime.load();
    return true;
}

void OHAudioBuffer::SetHandleInfo(uint64_t frames, int64_t nanoTime)
{
    basicBufferInfo_->handlePos.store(frames);
    basicBufferInfo_->handleTime.store(nanoTime);
}

int32_t OHAudioBuffer::GetAvailableDataFrames()
{
    int32_t result = -1; // failed
    uint64_t write = basicBufferInfo_->curWriteFrame.load();
    uint64_t read = basicBufferInfo_->curReadFrame.load();
    CHECK_AND_RETURN_RET_LOG(write >= read, result, "invalid write and read position.");
    uint32_t temp = write - read;
    CHECK_AND_RETURN_RET_LOG(temp <= INT32_MAX && temp <= totalSizeInFrame_,
        result, "failed to GetAvailableDataFrames.");
    result = static_cast<int32_t>(totalSizeInFrame_ - temp);
    return result;
}

int32_t OHAudioBuffer::ResetCurReadWritePos(uint64_t readFrame, uint64_t writeFrame)
{
    CHECK_AND_RETURN_RET_LOG(readFrame <= writeFrame && writeFrame - readFrame < totalSizeInFrame_,
        ERR_INVALID_PARAM, "Invalid read or write position:read%{public}" PRIu64" write%{public}" PRIu64".",
        readFrame, writeFrame);
    uint64_t tempBase = (readFrame / totalSizeInFrame_) * totalSizeInFrame_;
    basicBufferInfo_->basePosInFrame.store(tempBase);
    basicBufferInfo_->curWriteFrame.store(writeFrame);
    basicBufferInfo_->curReadFrame.store(readFrame);

    AUDIO_INFO_LOG("Reset position:read%{public}" PRIu64" write%{public}" PRIu64".", readFrame, writeFrame);
    return SUCCESS;
}

uint64_t OHAudioBuffer::GetCurWriteFrame()
{
    CHECK_AND_RETURN_RET_LOG(basicBufferInfo_ != nullptr, 0, "basicBufferInfo_ is null");
    return basicBufferInfo_->curWriteFrame.load();
}

uint64_t OHAudioBuffer::GetCurReadFrame()
{
    CHECK_AND_RETURN_RET_LOG(basicBufferInfo_ != nullptr, 0, "basicBufferInfo_ is null");
    return basicBufferInfo_->curReadFrame.load();
}

int32_t OHAudioBuffer::SetCurWriteFrame(uint64_t writeFrame)
{
    uint64_t basePos = basicBufferInfo_->basePosInFrame.load();
    uint64_t oldWritePos = basicBufferInfo_->curWriteFrame.load();
    if (writeFrame == oldWritePos) {
        return SUCCESS;
    }
    CHECK_AND_RETURN_RET_LOG(writeFrame > oldWritePos, ERR_INVALID_PARAM, "Too small writeFrame:%{public}" PRIu64".",
        writeFrame);

    uint64_t deltaToBase = writeFrame - basePos; // writeFrame % spanSizeInFrame_ --> 0
    CHECK_AND_RETURN_RET_LOG(deltaToBase / spanSizeInFrame_ * spanSizeInFrame_ == deltaToBase, ERR_INVALID_PARAM,
        "Invalid deltaToBase, writeFrame:%{public}" PRIu64".", writeFrame);

    // check new pos in range: base ~ base + 2*total
    CHECK_AND_RETURN_RET_LOG(deltaToBase < (totalSizeInFrame_ + totalSizeInFrame_),
        ERR_INVALID_PARAM, "Invalid writeFrame %{public}" PRIu64" out of base range.", writeFrame);

    // check new pos in (read + cache) range: read ~ read + totalSize - 1*spanSize
    uint64_t curRead = basicBufferInfo_->curReadFrame.load();
    CHECK_AND_RETURN_RET_LOG(writeFrame >= curRead && writeFrame - curRead <= totalSizeInFrame_,
        ERR_INVALID_PARAM, "Invalid writeFrame %{public}" PRIu64" out of cache range, curRead %{public}" PRIu64".",
        writeFrame, curRead);

    if (writeFrame - oldWritePos != spanSizeInFrame_) {
        AUDIO_WARNING_LOG("Not advanced in one step. newWritePos %{public}" PRIu64", oldWritePos %{public}" PRIu64".",
            writeFrame, oldWritePos);
    }

    basicBufferInfo_->curWriteFrame.store(writeFrame);
    return SUCCESS;
}

int32_t OHAudioBuffer::SetCurReadFrame(uint64_t readFrame)
{
    CHECK_AND_RETURN_RET_LOG(basicBufferInfo_ != nullptr, ERR_INVALID_PARAM, "basicBufferInfo_ is nullptr");
    uint64_t oldBasePos = basicBufferInfo_->basePosInFrame.load();
    uint64_t oldReadPos = basicBufferInfo_->curReadFrame.load();
    if (readFrame == oldReadPos) {
        return SUCCESS;
    }

    // new read position should not be bigger than write position or less than old read position
    CHECK_AND_RETURN_RET_LOG(readFrame >= oldReadPos && readFrame <= basicBufferInfo_->curWriteFrame.load(),
        ERR_INVALID_PARAM, "Invalid readFrame %{public}" PRIu64".", readFrame);

    uint64_t deltaToBase = readFrame - oldBasePos;
    CHECK_AND_RETURN_RET_LOG((deltaToBase / spanSizeInFrame_ * spanSizeInFrame_) == deltaToBase,
        ERR_INVALID_PARAM, "Invalid deltaToBase, readFrame %{public}" PRIu64", oldBasePos %{public}" PRIu64".",
            readFrame, oldBasePos);

    if (deltaToBase > totalSizeInFrame_) {
        AUDIO_ERR_LOG("Invalid readFrame:%{public}" PRIu64", out of range.", readFrame);
        return ERR_INVALID_PARAM;
    } else if (deltaToBase == totalSizeInFrame_) {
        basicBufferInfo_->basePosInFrame.store(oldBasePos + totalSizeInFrame_); // move base position
    }

    if (readFrame - oldReadPos != spanSizeInFrame_) {
        AUDIO_WARNING_LOG("Not advanced in one step. newReadPos %{public}" PRIu64", oldReadPos %{public}" PRIu64".",
            readFrame, oldReadPos);
    }

    basicBufferInfo_->curReadFrame.store(readFrame);
    return SUCCESS;
}

int32_t OHAudioBuffer::GetBufferByFrame(uint64_t posInFrame, BufferDesc &bufferDesc)
{
    uint64_t basePos = basicBufferInfo_->basePosInFrame.load();
    uint64_t maxDelta = 2 * totalSizeInFrame_; // 0 ~ 2*totalSizeInFrame_
    CHECK_AND_RETURN_RET_LOG(posInFrame >= basePos && posInFrame - basePos < maxDelta,
        ERR_INVALID_PARAM, "Invalid position:%{public}" PRIu64".", posInFrame);

    uint32_t deltaToBase = posInFrame - basePos;
    if (deltaToBase >= totalSizeInFrame_) {
        deltaToBase -= totalSizeInFrame_;
    }
    CHECK_AND_RETURN_RET_LOG(deltaToBase < UINT32_MAX && deltaToBase < totalSizeInFrame_, ERR_INVALID_PARAM,
        "invalid deltaToBase, posInFrame %{public}" PRIu64" basePos %{public}" PRIu64".", posInFrame, basePos);
    deltaToBase = (deltaToBase / spanSizeInFrame_) * spanSizeInFrame_;
    size_t offset = deltaToBase * byteSizePerFrame_;
    CHECK_AND_RETURN_RET_LOG(offset < totalSizeInByte_, ERR_INVALID_PARAM, "invalid deltaToBase:%{public}zu", offset);
    bufferDesc.buffer = dataBase_ + offset;
    bufferDesc.bufLength = spanSizeInByte_;
    bufferDesc.dataLength = spanSizeInByte_;

    return SUCCESS;
}

uint32_t OHAudioBuffer::GetSessionId()
{
    return sessionId_;
}

int32_t OHAudioBuffer::SetSessionId(uint32_t sessionId)
{
    sessionId_ = sessionId;

    return SUCCESS;
}

int32_t OHAudioBuffer::GetWriteBuffer(uint64_t writePosInFrame, BufferDesc &bufferDesc)
{
    uint64_t basePos = basicBufferInfo_->basePosInFrame.load();
    uint64_t readPos = basicBufferInfo_->curReadFrame.load();
    uint64_t maxWriteDelta = 2 * totalSizeInFrame_; // 0 ~ 2*totalSizeInFrame_
    CHECK_AND_RETURN_RET_LOG(writePosInFrame >= basePos && writePosInFrame - basePos < maxWriteDelta &&
        writePosInFrame >= readPos, ERR_INVALID_PARAM, "Invalid write position:%{public}" PRIu64".", writePosInFrame);
    return GetBufferByFrame(writePosInFrame, bufferDesc);
}

int32_t OHAudioBuffer::GetReadbuffer(uint64_t readPosInFrame, BufferDesc &bufferDesc)
{
    uint64_t basePos = basicBufferInfo_->basePosInFrame.load();
    CHECK_AND_RETURN_RET_LOG(readPosInFrame >= basePos && readPosInFrame - basePos < totalSizeInFrame_,
        ERR_INVALID_PARAM, "Invalid read position:%{public}" PRIu64".", readPosInFrame);
    return GetBufferByFrame(readPosInFrame, bufferDesc);
}

SpanInfo *OHAudioBuffer::GetSpanInfo(uint64_t posInFrame)
{
    uint64_t basePos = basicBufferInfo_->basePosInFrame.load();
    uint64_t maxPos = basePos + totalSizeInFrame_ + totalSizeInFrame_;
    CHECK_AND_RETURN_RET_LOG((basePos <= posInFrame && posInFrame < maxPos), nullptr, "posInFrame %{public}" PRIu64" "
        "out of range, basePos %{public}" PRIu64", maxPos %{public}" PRIu64".", posInFrame, basePos, maxPos);

    uint64_t deltaToBase = posInFrame - basePos;
    if (deltaToBase >= totalSizeInFrame_) {
        deltaToBase -= totalSizeInFrame_;
    }
    CHECK_AND_RETURN_RET_LOG(deltaToBase < UINT32_MAX && deltaToBase < totalSizeInFrame_, nullptr,"invalid "
        "deltaToBase, posInFrame %{public}"  PRIu64" basePos %{public}" PRIu64".", posInFrame, basePos);
         
    if (spanSizeInFrame_ > 0) {
        uint32_t spanIndex = deltaToBase / spanSizeInFrame_;
        CHECK_AND_RETURN_RET_LOG(spanIndex < spanConut_, nullptr, "invalid spanIndex:%{public}d", spanIndex);
        return &spanInfoList_[spanIndex];
    }
    return nullptr;
}

SpanInfo *OHAudioBuffer::GetSpanInfoByIndex(uint32_t spanIndex)
{
    CHECK_AND_RETURN_RET_LOG(spanIndex < spanConut_, nullptr, "invalid spanIndex:%{public}d", spanIndex);
    return &spanInfoList_[spanIndex];
}

uint32_t OHAudioBuffer::GetSpanCount()
{
    return spanConut_;
}

int64_t OHAudioBuffer::GetLastWrittenTime()
{
    return lastWrittenTime_;
}

void OHAudioBuffer::SetLastWrittenTime(int64_t time)
{
    lastWrittenTime_ = time;
}

std::atomic<uint32_t> *OHAudioBuffer::GetFutex()
{
    return &basicBufferInfo_->futexObj;
}

uint8_t *OHAudioBuffer::GetDataBase()
{
    return dataBase_;
}

size_t OHAudioBuffer::GetDataSize()
{
    return totalSizeInByte_;
}

} // namespace AudioStandard
} // namespace OHOS
