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

#ifndef AUDIO_ENHANCE_CHAIN_H
#define AUDIO_ENHANCE_CHAIN_H

#include <vector>
#include <mutex>
#include <map>

#include "audio_effect.h"
#include "audio_enhance_chain_adapter.h"

namespace OHOS {
namespace AudioStandard {

const std::map<std::string, uint32_t> REF_CHANNEL_NUM_MAP {
    {"SCENE_VOIP_3A", 2},
    {"SCENE_RECORD", 0},
};

struct DataDescription {
    uint32_t frameLength_;
    uint32_t sampleRate_;
    uint32_t dataFormat_;
    uint32_t micNum_;
    uint32_t refNum_;
    uint32_t outChannelNum_;

    DataDescription(uint32_t frameLength, uint32_t sampleRate, uint32_t dataFormat,
        uint32_t micNum, uint32_t refNum, uint32_t outChannelNum)
        : frameLength_(frameLength), sampleRate_(sampleRate), dataFormat_(dataFormat),
        micNum_(micNum), refNum_(refNum), outChannelNum_(outChannelNum) {}
} __attribute__((packed));

class AudioEnhanceChain {
public:
    AudioEnhanceChain(const std::string &scene);
    ~AudioEnhanceChain();
    void SetEnhanceMode(const std::string &mode);
    void AddEnhanceHandle(AudioEffectHandle handle, AudioEffectLibrary *libHandle);
    void SetHandleConfig(AudioEffectHandle handle, DataDescription desc);
    int32_t ApplyEnhanceChain(EnhanceBufferAttr *enhanceBufferAttr);
    bool IsEmptyEnhanceHandles();

private:
    void ReleaseEnhanceChain();

    bool setConfigFlag_;
    std::mutex chainMutex_;
    std::string sceneType_;
    std::string enhanceMode_;
    std::vector<AudioEffectHandle> standByEnhanceHandles_;
    std::vector<AudioEffectLibrary*> enhanceLibHandles_;
};

}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_ENHANCE_CHAIN_H