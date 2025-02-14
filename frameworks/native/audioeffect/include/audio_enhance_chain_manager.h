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

#ifndef AUDIO_ENHANCE_CHAIN_MANAGER_H
#define AUDIO_ENHANCE_CHAIN_MANAGER_H

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <set>

#include "audio_effect.h"
#include "audio_enhance_chain.h"

namespace OHOS {
namespace AudioStandard {

class AudioEnhanceChainManager {
public:
    AudioEnhanceChainManager();
    ~AudioEnhanceChainManager();
    static AudioEnhanceChainManager* GetInstance();
    void InitAudioEnhanceChainManager(std::vector<EffectChain> &enhanceChains,
        const EffectChainManagerParam &managerParam,
        std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList);
    int32_t CreateAudioEnhanceChainDynamic(const std::string &sceneType,
        const std::string &enhanceMode, const std::string &upAndDownDevice);
    int32_t ReleaseAudioEnhanceChainDynamic(const std::string &sceneType, const std::string &upAndDownDevice);
    int32_t ApplyAudioEnhanceChain(const std::string &sceneType, EnhanceBufferAttr *enhanceBufferAttr);
    bool ExistAudioEnhanceChain(const std::string &sceneType);
    std::string GetUpAndDownDevice();
    // for effect V3
    int32_t SetAudioEnhanceProperty(const AudioEffectPropertyArrayV3 &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE);
    int32_t GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE);

private:
    int32_t SetAudioEnhanceChainDynamic(const std::string &sceneType, const std::string &enhanceMode,
        const std::string &upAndDownDevice);
    
    std::map<std::string, std::shared_ptr<AudioEnhanceChain>> sceneTypeToEnhanceChainMap_;
    std::map<std::string, int32_t> sceneTypeToEnhanceChainCountMap_;
    std::map<std::string, std::string> sceneTypeAndModeToEnhanceChainNameMap_;
    std::map<std::string, std::vector<std::string>> enhanceChainToEnhancesMap_;
    std::map<std::string, std::shared_ptr<AudioEffectLibEntry>> enhanceToLibraryEntryMap_;
    std::map<std::string, std::string> enhanceToLibraryNameMap_;
    std::mutex chainManagerMutex_;
    bool isInitialized_;
    std::string upAndDownDevice_;
};

}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_ENHANCE_CHAIN_MANAGER_H