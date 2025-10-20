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
#ifndef AUDIO_INJECTOR_SERVICE_H
#define AUDIO_INJECTOR_SERVICE_H

#include "audio_module_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioInjectorService {
public:
    static AudioInjectorService& GetInstance()
    {
        static AudioInjectorService instance;
        return instance;
    }
    int32_t PeekAudioData(const uint32_t sinkPortIndex, uint8_t *buffer, const size_t bufferSize,
        AudioStreamInfo &streamInfo);
    void SetSinkPortIdx(uint32_t sinkPortIdx);
    uint32_t GetSinkPortIdx();
    AudioModuleInfo &GetModuleInfo();
private:
    AudioInjectorService();
    ~AudioInjectorService() = default;
    AudioInjectorService(const AudioInjectorService&) = delete;
    AudioInjectorService& operator=(const AudioInjectorService&) = delete;
private:
    AudioModuleInfo moduleInfo_;
    uint32_t sinkPortIndex_;
};
} //  namespace AudioStandard
} //  namespace OHOS
#endif  // AUDIO_INJECTOR_SERVICE_H