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

#ifndef AUDIO_ENHANCE_CHAIN_MANAGER_IMPL_H
#define AUDIO_ENHANCE_CHAIN_MANAGER_IMPL_H

#include "audio_enhance_chain_manager.h"

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <vector>

#include "audio_enhance_chain.h"
#include "thread_handler.h"

namespace OHOS {
namespace AudioStandard {
struct EnhanceConfigInfo {
    bool relateWithDevice { false };
    std::string defaultProp;
    std::shared_ptr<AudioEffectLibEntry> enhanceLib { nullptr };
};

struct EnhanceChainConfigInfo {
    std::string chainName;
    std::string chainLabel;
    std::vector<std::string> enhanceNames;
};

class AudioEnhanceChainManagerImpl : public AudioEnhanceChainManager, public NoCopyable {
public:
    void InitAudioEnhanceChainManager(const std::vector<EffectChain> &enhanceChains,
        const EffectChainManagerParam &managerParam,
        const std::vector<std::shared_ptr<AudioEffectLibEntry>> &libList) override;

    int32_t CreateAudioEnhanceChainDynamic(uint64_t sceneKeyCode, const AudioEnhanceDeviceAttr &deviceAttr) override;

    int32_t ReleaseAudioEnhanceChainDynamic(uint64_t sceneKeyCode) override;

    int32_t AudioEnhanceChainGetAlgoConfig(uint64_t sceneKeyCode, AudioBufferConfig &micConfig,
        AudioBufferConfig &ecConfig, AudioBufferConfig &micRefConfig) override;

    int32_t ApplyEnhanceChainById(uint64_t sceneKeyCode, const EnhanceTransBuffer &transBuf) override;

    int32_t GetChainOutputDataById(uint64_t sceneKeyCode, void *buf, size_t bufSize) override;

    int32_t SetInputDevice(uint32_t captureId, DeviceType inputDevice, const std::string &deviceName = "") override;

    int32_t SetOutputDevice(uint32_t renderId, DeviceType outputDevice) override;

    int32_t SetVolumeInfo(AudioVolumeType volumeType, float systemVol) override;

    int32_t SetMicrophoneMuteInfo(bool isMute) override;

    int32_t SetStreamVolumeInfo(uint32_t sessionId, float streamVol) override;

    int32_t SetAudioEnhanceProperty(const AudioEffectPropertyArrayV3 &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) override;

    int32_t GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) override;

    int32_t SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) override;

    int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) override;

    void ResetInfo() override;

    void UpdateExtraSceneType(const std::string &mainkey, const std::string &subkey,
        const std::string &extraSceneType) override;

    int32_t SendInitCommand() override;

private:
    friend class AudioEnhanceChainManager;
    AudioEnhanceChainManagerImpl() = default;
    ~AudioEnhanceChainManagerImpl() override = default;

    void SendFoldStateToChain(uint32_t foldState);
    void SendPowerStateToChain(uint32_t powerState);
    int32_t AddAudioEnhanceChainHandles(std::shared_ptr<AudioEnhanceChain> &audioEnhanceChain,
        const std::vector<std::string> &enhanceNames);
    int32_t ParseSceneKeyCode(uint64_t sceneKeyCode, std::string &sceneType, std::string &capturerDeviceStr,
        std::string &rendererDeviceStr);
    std::shared_ptr<AudioEnhanceChain> CreateEnhanceChainInner(uint64_t sceneKeyCode,
        const AudioEnhanceDeviceAttr &deviceAttr);
    std::vector<std::string> GetEnhanceNamesBySceneCode(uint64_t sceneKeyCode, bool defaultFlag);
    void ReleaseThreadHandlerByScene(AudioEnhanceScene scene);
    int32_t UpdatePropertyAndSendToAlgo(DeviceType inputDevice);
    void UpdateEnhancePropertyMapFromDb(DeviceType deviceType);
    int32_t WriteEnhancePropertyToDb(const std::string &key, const std::string &property);
    int32_t SetAudioEnhancePropertyToChains(const AudioEffectPropertyV3 &property);
    void GetDeviceTypeName(DeviceType deviceType, std::string &deviceName);
    void SetRelateWithDevicePropForEnhance();
    std::shared_ptr<ThreadHandler> GetThreadHandlerByScene(AudioEnhanceScene scene);

    std::map<std::string, EnhanceConfigInfo> enhanceConfigInfoMap_;
    std::map<std::string, EnhanceChainConfigInfo> chainConfigInfoMap_;
    std::map<std::string, std::string> enhancePropertyMap_;
    std::map<uint32_t, DeviceType> captureIdToDeviceMap_;
    std::map<uint32_t, std::pair<std::shared_ptr<ThreadHandler>, uint32_t>> threadHandlerMap_;
    uint32_t maxNormalInstanceNum_ { 0 };
    std::string defaultScene_;
    std::set<std::string> priorSceneSet_;
    AudioEnhanceParamAdapter enhancePara_ {};
    std::mutex chainManagerMutex_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_ENHANCE_CHAIN_MANAGER_IMPL_H