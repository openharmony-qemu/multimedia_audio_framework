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
#ifndef AUDIO_PCM_DUMP_H
#define AUDIO_PCM_DUMP_H

#include <string.h>
#include <memory>

namespace OHOS {
namespace AudioStandard {

const std::string setOpenKey = "OPEN";
const std::string setCloseKey = "CLOSE";
const std::string setUploadKey = "UPLOAD";

const std::string getStatusKey = "STATUS";
const std::string getTimeKey = "TIME";
const std::string getMemoryKey = "MEMORY";

class AudioCacheMgr {
public:
    static AudioCacheMgr& GetInstance();
    AudioCacheMgr() = default;
    virtual ~AudioCacheMgr() = default;

    virtual bool Init();
    virtual bool DeInit();

    virtual void CacheData(std::string& dumpFileName, void* dataPointer, size_t dataLength) = 0;
    virtual int32_t DumpAllMemBlock() = 0;
    virtual void GetCachedDuration(int64_t& startTime, int64_t& endTime) = 0;
    virtual void GetCurMemoryCondition(size_t& dataLength, size_t& bufferLength, size_t& structLength) = 0;
};

} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_PCM_DUMP_H