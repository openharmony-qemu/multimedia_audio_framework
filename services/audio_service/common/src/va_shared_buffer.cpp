/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "VASharedBuffer"
#endif

#include "va_shared_buffer.h"

#include <cinttypes>
#include <climits>
#include <memory>
#include <sys/mman.h>
#include "ashmem.h"

#include "audio_errors.h"
#include "audio_service_log.h"
#include "futex_tool.h"
#include "audio_utils.h"
#include "audio_parcel_helper.h"

namespace OHOS {
namespace AudioStandard {

namespace {
    static const size_t MAX_MMAP_BUFFER_SIZE = 10 * 1024 * 1024;
    static const std::string STATUS_INFO_BUFFER = "status_info_buffer";
    static const std::string DATA_BUFFER = "data_buffer";
    static constexpr int MINFD = 2;
}


VAAudioSharedMemory::VAAudioSharedMemory(size_t size, const std::string &name)
    : base_(nullptr), fd_(INVALID_FD), size_(size), name_(name)
{
    AUDIO_INFO_LOG("AudioSharedMemory construct with size: %{public}zu name: %{public}s", size_, name_.c_str());
}

VAAudioSharedMemory::VAAudioSharedMemory(int fd, size_t size, const std::string &name)
    : base_(nullptr), fd_(fd), size_(size), name_(name)
{
    AUDIO_INFO_LOG("AudioSharedMemory construct with fd %{public}d size %{public}zu name %{public}s", fd_, size_,
        name_.c_str());
}

VAAudioSharedMemory::~VAAudioSharedMemory()
{
    AUDIO_INFO_LOG(" %{public}s enter ~AudioSharedMemoryImpl()", name_.c_str());
    Close();
}

std::shared_ptr<VAAudioSharedMemory> VAAudioSharedMemory::CreateFromLocal(size_t size, const std::string &name)
{
    std::shared_ptr<VAAudioSharedMemory> sharedMemory = std::make_shared<VAAudioSharedMemory>(size, name);
    CHECK_AND_RETURN_RET_LOG(sharedMemory->Init() == SUCCESS, nullptr, "CreateFormLocal failed");
    return sharedMemory;
}

std::shared_ptr<VAAudioSharedMemory> VAAudioSharedMemory::CreateFromRemote(int fd, size_t size, const std::string &name)
{
    CHECK_AND_RETURN_RET_LOG(fd > MINFD, nullptr, "CreateFromRemote failed: invalid fd: %{public}d", fd);
    std::shared_ptr<VAAudioSharedMemory> sharedMemory = std::make_shared<VAAudioSharedMemory>(fd, size, name);
    CHECK_AND_RETURN_RET_LOG(sharedMemory->Init() == SUCCESS, nullptr, "CreateFromRemote failed");
    return sharedMemory;
}

int32_t VAAudioSharedMemory::Init()
{
    CHECK_AND_RETURN_RET_LOG((size_ > 0 && size_ < MAX_MMAP_BUFFER_SIZE), ERR_INVALID_PARAM,
        "Init failed: size out of range: %{public}zu", size_);
    bool isFromRemote = false;
    if (fd_ >= 0) {
        if (fd_ == STDIN_FILENO || fd_ == STDOUT_FILENO || fd_ == STDERR_FILENO) {
            AUDIO_WARNING_LOG("fd is special fd: %{public}d", fd_);
        }
        isFromRemote = true;
        int size = AshmemGetSize(fd_);
        if (size < 0 || static_cast<size_t>(size) != size_) {
            AUDIO_WARNING_LOG("AshmemGetSize failed, get size: %{public}d size_: %{public}zu", size, size_);
            return ERR_OPERATION_FAILED;
        }
        ashmem_ = sptr<Ashmem>(new Ashmem(fd_, size));
        CHECK_AND_RETURN_RET_LOG((ashmem_ != nullptr), ERR_OPERATION_FAILED, "CreateAshmem failed.");
    } else {
        ashmem_ = Ashmem::CreateAshmem(name_.c_str(), size_);
        CHECK_AND_RETURN_RET_LOG((ashmem_ != nullptr), ERR_OPERATION_FAILED, "CreateAshmem failed.");
        fd_ = ashmem_->GetAshmemFd();
        CHECK_AND_RETURN_RET_LOG((fd_ >= 0), ERR_OPERATION_FAILED, "Init failed: fd %{public}d", fd_);
    }
    if (!ashmem_->MapReadAndWriteAshmem()) {
        AUDIO_INFO_LOG("ashmem Map shared memory fail");
    } else {
        AUDIO_INFO_LOG("ashmem Map shared memory success");
    }

    void *addr = mmap(nullptr, size_, PROT_READ | PROT_WRITE, MAP_SHARED, fd_, 0);
    CHECK_AND_RETURN_RET_LOG(addr != MAP_FAILED, ERR_OPERATION_FAILED, "Init failed: fd %{public}d size %{public}zu",
        fd_, size_);
    base_ = static_cast<uint8_t *>(addr);
    AUDIO_INFO_LOG("Init %{public}s <%{public}s> done.", (isFromRemote ? "remote" : "local"),
        name_.c_str());
    return SUCCESS;
}


void VAAudioSharedMemory::Close()
{
    if (base_ != nullptr) {
        (void)munmap(base_, size_);
        base_ = nullptr;
        size_ = 0;
        AUDIO_INFO_LOG("%{public}s munmap done", name_.c_str());
    }

    if (fd_ >= 0) {
        (void)CloseFd(fd_);
        fd_ = INVALID_FD;
        AUDIO_INFO_LOG("%{public}s close fd done", name_.c_str());
    }
}

uint8_t *VAAudioSharedMemory::GetBase()
{
    return base_;
}

size_t VAAudioSharedMemory::GetSize()
{
    return size_;
}

std::string VAAudioSharedMemory::GetName()
{
    return name_;
}

int VAAudioSharedMemory::GetFd()
{
    return fd_;
}

sptr<Ashmem> VAAudioSharedMemory::GetAshmem()
{
    return ashmem_;
}

int32_t VASharedBuffer::SizeCheck()
{
    return SUCCESS;
}

VASharedBuffer::VASharedBuffer()
{}

int32_t VASharedBuffer::Init(const VASharedMemInfo& memInfo)
{
    AUDIO_INFO_LOG("VASharedBuffer::VASharedBuffer Init START.");
    int32_t ret = SizeCheck();
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERR_INVALID_PARAM, "failed: invalid size.");

    if (memInfo.dataFd_ > MINFD && memInfo.dataMemCapacity_ > 0) {
        AUDIO_INFO_LOG("dataMem_ CreateFromRemote");
        dataMem_ = VAAudioSharedMemory::CreateFromRemote(memInfo.dataFd_, memInfo.dataMemCapacity_, DATA_BUFFER);
    } else {
        AUDIO_INFO_LOG("dataMem_ CreateFromLocal");
        dataMem_ = VAAudioSharedMemory::CreateFromLocal(memInfo.dataMemCapacity_, DATA_BUFFER);
    }

    if (memInfo.statusFd_ > MINFD && memInfo.statusMemCapacity_ > 0) {
        AUDIO_INFO_LOG("statusInfoMem_ CreateFromRemote");
        statusInfoMem_ = VAAudioSharedMemory::CreateFromRemote(memInfo.statusFd_,
            memInfo.statusMemCapacity_, STATUS_INFO_BUFFER);
    } else {
        AUDIO_INFO_LOG("statusInfoMem_ CreateFromLocal");
        statusInfoMem_ = VAAudioSharedMemory::CreateFromLocal(sizeof(VASharedStatusInfo), STATUS_INFO_BUFFER);
    }

    CHECK_AND_RETURN_RET_LOG(statusInfoMem_ != nullptr, ERR_OPERATION_FAILED, "statusInfoMem_ mmap failed.");

    CHECK_AND_RETURN_RET_LOG(dataMem_ != nullptr, ERR_OPERATION_FAILED, "dataMem_ mmap failed.");

    dataBase_ = dataMem_->GetBase();

    AUDIO_INFO_LOG("VASharedBuffer::VASharedBuffer Init DONE.");
    return SUCCESS;
}

VASharedBuffer::~VASharedBuffer()
{}

std::shared_ptr<VASharedBuffer> VASharedBuffer::CreateFromLocal(uint32_t dataSize)
{
    std::shared_ptr<VASharedBuffer> buffer = std::make_shared<VASharedBuffer>();
    CHECK_AND_RETURN_RET_LOG(buffer != nullptr, nullptr, "buffer is null");
    VASharedMemInfo memInfo;
    memInfo.dataMemCapacity_ = dataSize;
    memInfo.dataFd_ = INVALID_FD;
    memInfo.statusMemCapacity_ = 0;
    memInfo.statusFd_ = INVALID_FD;

    if (buffer->Init(memInfo) == SUCCESS) {
        return buffer;
    } else {
        AUDIO_ERR_LOG("init va shared buffer failed");
        return nullptr;
    }
}

std::shared_ptr<VASharedBuffer> VASharedBuffer::CreateFromRemote(const VASharedMemInfo &memInfo)
{
    if (memInfo.dataFd_ <= MINFD || memInfo.statusFd_ <= MINFD ||
        memInfo.dataMemCapacity_ <= 0 || memInfo.statusMemCapacity_ <= 0) {
        return nullptr;
    }

    std::shared_ptr<VASharedBuffer> buffer = std::make_shared<VASharedBuffer>();
    if (buffer->Init(memInfo) == SUCCESS) {
        return buffer;
    } else {
        AUDIO_ERR_LOG("init va shared buffer failed");
        return nullptr;
    }
}

uint8_t *VASharedBuffer::GetDataBase()
{
    return dataBase_;
}

size_t VASharedBuffer::GetDataSize()
{
    CHECK_AND_RETURN_RET_LOG(dataMem_ != nullptr, 0, "dataMem is nullptr");
    return dataMem_->GetSize();
}

sptr<Ashmem> VASharedBuffer::GetDataAshmem()
{
    CHECK_AND_RETURN_RET_LOG(dataMem_ != nullptr, nullptr, "dataMem is nullptr");
    return dataMem_->GetAshmem();
}

uint8_t *VASharedBuffer::GetStatusInfoBase()
{
    return statusInfoMem_ != nullptr ? statusInfoMem_->GetBase() : nullptr;
}


void VASharedBuffer::GetVASharedMemInfo(VASharedMemInfo &memInfo)
{
    if (dataMem_ != nullptr) {
        memInfo.dataFd_ = dataMem_->GetFd();
        memInfo.dataMemCapacity_ = dataMem_->GetSize();
    }
    if (statusInfoMem_ != nullptr) {
        memInfo.statusFd_ = statusInfoMem_->GetFd();
        memInfo.statusMemCapacity_ = statusInfoMem_->GetSize();
    }
}

int64_t VASharedBuffer::GetLastWrittenTime()
{
    return lastWrittenTime_;
}

void VASharedBuffer::SetLastWrittenTime(int64_t time)
{
    lastWrittenTime_ = time;
}

}   // namespace AudioStandard
}   // namespace OHOS