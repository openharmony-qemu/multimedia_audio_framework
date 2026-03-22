/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#ifndef ST_AUDIO_CONCURRENCY_MANAGER_H
#define ST_AUDIO_CONCURRENCY_MANAGER_H
#include <mutex>

#include "iremote_object.h"

#include "audio_policy_log.h"
#include "audio_concurrency_parser.h"

namespace OHOS {
namespace AudioStandard {

class AudioConcurrencyManager {
public:
    AudioConcurrencyManager()
    {
        AUDIO_INFO_LOG("ctor");
    }
    virtual ~AudioConcurrencyManager()
    {
        AUDIO_ERR_LOG("dtor");
    }

    void Init();

    static AudioConcurrencyManager& GetInstance()
    {
        static AudioConcurrencyManager instance;
        return instance;
    }

    ConcurrencyAction GetConcurrencyAction(const AudioPipeType existingPipe, const AudioPipeType commingPipe);
    AudioPipeType GetPipeTypeByRouteFlag(uint32_t flag, AudioMode audioMode);

private:
    std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> concurrencyConfigMap_ = {};
};
} // namespace AudioStandard
} // namespace OHOS
#endif