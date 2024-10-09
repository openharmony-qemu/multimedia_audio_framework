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

#ifndef AUDIO_SHARED_MEMORY_H
#define AUDIO_SHARED_MEMORY_H

#include <string>
#include "stdint.h"

#include "message_parcel.h"

namespace OHOS {
namespace AudioStandard {
class AudioSharedMemory {
public:
    virtual ~AudioSharedMemory() = default;

    virtual uint8_t *GetBase() = 0;
    virtual size_t GetSize() = 0;
    virtual int GetFd() = 0;
    virtual std::string GetName() = 0;

    static std::shared_ptr<AudioSharedMemory> CreateFormLocal(size_t size, const std::string &name);
    static std::shared_ptr<AudioSharedMemory> CreateFromRemote(int fd, size_t size, const std::string &name);

    static int32_t WriteToParcel(const std::shared_ptr<AudioSharedMemory> &memory, MessageParcel &parcel);
    static std::shared_ptr<AudioSharedMemory> ReadFromParcel(MessageParcel &parcel);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SHARED_MEMORY_H
