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
    int32_t CreateAudioEnhanceChainDynamic(const uint64_t sceneKeyCode, const AudioEnhanceDeviceAttr &deviceAttr);
    int32_t ReleaseAudioEnhanceChainDynamic(const uint64_t sceneKeyCode);
    bool ExistAudioEnhanceChain(const uint64_t sceneKeyCode);
    int32_t AudioEnhanceChainGetAlgoConfig(const uint64_t sceneKeyCode, AudioBufferConfig &micConfig,
        AudioBufferConfig &ecConfig, AudioBufferConfig &micRefConfig);
    bool IsEmptyEnhanceChain();
    int32_t InitEnhanceBuffer();
    int32_t CopyToEnhanceBuffer(void *data, uint32_t length);
    int32_t CopyEcToEnhanceBuffer(void *data, uint32_t length);
    int32_t CopyMicRefToEnhanceBuffer(void *data, uint32_t length);
    int32_t CopyFromEnhanceBuffer(void *data, uint32_t length);
    int32_t ApplyAudioEnhanceChain(const uint64_t sceneKeyCode, uint32_t length);
    int32_t SetInputDevice(const uint32_t &captureId, const DeviceType &inputDevice,
        const std::string &deviceName = "");
    int32_t SetOutputDevice(const uint32_t &renderId, const DeviceType &outputDevice);
    int32_t SetVolumeInfo(const AudioVolumeType &volumeType, const float &systemVol);
    int32_t SetMicrophoneMuteInfo(const bool &isMute);
    int32_t SetStreamVolumeInfo(const uint32_t &sessionId, const float &streamVol);
    // for effect V3
    int32_t SetAudioEnhanceProperty(const AudioEffectPropertyArrayV3 &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE);
    int32_t GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE);
    // for enhance
    int32_t SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE);
    int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE);
    void ResetInfo();  // use for unit test
    int32_t ApplyAudioEnhanceChainDefault(const uint32_t captureId, uint32_t length);
    void UpdateExtraSceneType(const std::string &mainkey, const std::string &subkey, const std::string &extraSceneType);
    int32_t SendInitCommand();

private:
    int32_t AddAudioEnhanceChainHandles(std::shared_ptr<AudioEnhanceChain> &audioEnhanceChain,
        const std::string &enhanceChain);
    int32_t FreeEnhanceBuffer();
    int32_t ParseSceneKeyCode(const uint64_t sceneKeyCode, std::string &sceneType, std::string &capturerDeviceStr,
        std::string &rendererDeivceStr);
    int32_t CreateEnhanceChainInner(std::shared_ptr<AudioEnhanceChain> &audioEnhanceChain,
        const uint64_t sceneKeyCode, const AudioEnhanceDeviceAttr &deviceAttr, bool &createFlag, bool &defaultFlag);
    int32_t DeleteEnhanceChainInner(std::shared_ptr<AudioEnhanceChain> &audioEnhanceChain,
        const uint64_t sceneKeyCode);
    std::string GetEnhanceChainNameBySceneCode(const uint64_t sceneKeyCode, const bool defaultFlag);
    int32_t UpdatePropertyAndSendToAlgo(const DeviceType &inputDevice);
    void UpdateEnhancePropertyMapFromDb(DeviceType deviceType);
    int32_t WriteEnhancePropertyToDb(const std::string &key, const std::string &property);
    int32_t SetAudioEnhancePropertyToChains(AudioEnhanceProperty property);
    int32_t SetAudioEnhancePropertyToChains(AudioEffectPropertyV3 property);
    void GetDeviceTypeName(DeviceType deviceType, std::string &deviceName);
    void GetDeviceNameByCaptureId(const uint32_t captureId, std::string &deviceName);
    // construct when init
    void ConstructEnhanceChainMgrMaps(std::vector<EffectChain> &enhanceChains,
        const EffectChainManagerParam &managerParam,
        std::vector<std::shared_ptr<AudioEffectLibEntry>> &enhanceLibraryList);
    void ConstructDeviceEnhances();

    std::map<uint64_t, std::shared_ptr<AudioEnhanceChain>> sceneTypeToEnhanceChainMap_;
    std::map<uint64_t, int32_t> sceneTypeToEnhanceChainCountMap_;
    std::unordered_map<std::string, std::string> sceneTypeAndModeToEnhanceChainNameMap_;
    std::map<std::string, std::vector<std::string>> enhanceChainToEnhancesMap_;
    std::map<std::string, std::shared_ptr<AudioEffectLibEntry>> enhanceToLibraryEntryMap_;
    std::map<std::string, std::string> enhanceToLibraryNameMap_;
    std::unordered_map<std::string, std::string> enhancePropertyMap_;
    std::unordered_map<std::string, std::string> defaultPropertyMap_;
    std::map<uint32_t, DeviceType> captureIdToDeviceMap_;
    std::map<uint32_t, std::string> captureIdToDeviceNameMap_;
    std::map<uint32_t, DeviceType> renderIdToDeviceMap_;

    std::map<uint32_t, uint32_t> captureId2SceneCount_;
    std::map<uint32_t, uint32_t> captureId2DefaultChainCount_;
    std::map<uint32_t, std::shared_ptr<AudioEnhanceChain>> captureId2DefaultChain_;
    std::set<std::string> withDeviceEnhances_;

    // for effect instances limit
    std::string defaultScene_;
    std::unordered_set<std::string> priorSceneSet_;
    uint32_t normalSceneLimit_ = 0;
    uint32_t chainNum_ = 0;

    std::unique_ptr<EnhanceBuffer> enhanceBuffer_ = nullptr;
    std::mutex chainManagerMutex_;
    bool isInitialized_;
    uint32_t sessionId_ = 0;
    AudioVolumeType volumeType_ = STREAM_MUSIC;
    float systemVol_ = 0.0f;
    float streamVol_ = 0.0f;
    bool isMute_ = false;
    std::string deviceName_ = "";
    uint32_t foldState_ = FOLD_STATE_MIDDLE;
};

}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_ENHANCE_CHAIN_MANAGER_H
