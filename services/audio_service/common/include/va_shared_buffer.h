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
#ifndef VA_SHARED_BUFFER_H
#define VA_SHARED_BUFFER_H

#include <atomic>
#include <string>
#include <type_traits>
#include <optional>

#include "message_parcel.h"

#include "audio_info.h"

#include "audio_shared_memory.h"
#include "futex_tool.h"

#include "va_device_info.h"
#include "oh_audio_buffer.h"


namespace OHOS {
namespace AudioStandard {

struct VASharedStatusInfo {
    size_t basePos;
    size_t readPos;
    size_t writePos;
    std::atomic<uint32_t> futex;
};

class VAAudioSharedMemory {
public:
    uint8_t *GetBase();
    size_t GetSize();
    int GetFd();
    std::string GetName();

    VAAudioSharedMemory(size_t size, const std::string &name);
    
    VAAudioSharedMemory(int fd, size_t size, const std::string &name);

    ~VAAudioSharedMemory();

    static std::shared_ptr<VAAudioSharedMemory> CreateFromLocal(size_t size, const std::string &name);

    static std::shared_ptr<VAAudioSharedMemory> CreateFromRemote(int fd, size_t size, const std::string &name);

    int32_t Init();

    sptr<Ashmem> GetAshmem();

private:
    void Close();

    uint8_t *base_;
    int fd_;
    size_t size_;
    std::string name_;

    sptr<Ashmem> ashmem_;
};

class VASharedBuffer {
public:
    VASharedBuffer();

    ~VASharedBuffer();

    static std::shared_ptr<VASharedBuffer> CreateFromLocal(uint32_t dataSize);
    static std::shared_ptr<VASharedBuffer> CreateFromRemote(const VASharedMemInfo &memInfo);

    int32_t Init(const VASharedMemInfo &memInfo);

    uint8_t *GetDataBase();
    size_t GetDataSize();
    sptr<Ashmem> GetDataAshmem();
    uint8_t *GetStatusInfoBase();

    void GetVASharedMemInfo(VASharedMemInfo &memInfo);
    int64_t GetLastWrittenTime();

    void SetLastWrittenTime(int64_t time);

private:
    std::shared_ptr<VAAudioSharedMemory> dataMem_;
    std::shared_ptr<VAAudioSharedMemory> statusInfoMem_;
    uint8_t *dataBase_ = nullptr;

    int64_t lastWrittenTime_ = 0;

    size_t totalSizeInByte_ = 0;

    int32_t SizeCheck();
};
}   //namespace AudioStandard
}   //namespace OHOS
#endif  //VA_SHARED_BUFFER_H