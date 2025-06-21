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

#include <cstdint>

#include "audio_effect.h"
#include "audio_effect_common.h"

namespace OHOS {
namespace AudioStandard {
class AudioEnhanceChainManager {
public:
    static AudioEnhanceChainManager *GetInstance();

    virtual ~AudioEnhanceChainManager() = default;

    virtual void InitAudioEnhanceChainManager(const std::vector<EffectChain> &enhanceChains,
        const EffectChainManagerParam &managerParam,
        const std::vector<std::shared_ptr<AudioEffectLibEntry>> &libList) = 0;

    virtual int32_t CreateAudioEnhanceChainDynamic(uint64_t sceneKeyCode, const AudioEnhanceDeviceAttr &deviceAttr) = 0;

    virtual int32_t ReleaseAudioEnhanceChainDynamic(uint64_t sceneKeyCode) = 0;

    virtual int32_t AudioEnhanceChainGetAlgoConfig(uint64_t sceneKeyCode, AudioBufferConfig &micConfig,
        AudioBufferConfig &ecConfig, AudioBufferConfig &micRefConfig) = 0;

    virtual int32_t ApplyEnhanceChainById(uint64_t sceneKeyCode, const EnhanceTransBuffer &transBuf) = 0;

    virtual int32_t GetChainOutputDataById(uint64_t sceneKeyCode, void *buf, size_t bufSize) = 0;

    virtual int32_t SetInputDevice(uint32_t captureId, DeviceType inputDevice, const std::string &deviceName = "") = 0;

    virtual int32_t SetOutputDevice(uint32_t renderId, DeviceType outputDevice) = 0;

    virtual int32_t SetVolumeInfo(AudioVolumeType volumeType, float systemVol) = 0;

    virtual int32_t SetMicrophoneMuteInfo(bool isMute) = 0;

    virtual int32_t SetStreamVolumeInfo(uint32_t sessionId, float streamVol) = 0;

    virtual int32_t SetAudioEnhanceProperty(const AudioEffectPropertyArrayV3 &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) = 0;

    virtual int32_t GetAudioEnhanceProperty(AudioEffectPropertyArrayV3 &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) = 0;

    virtual int32_t SetAudioEnhanceProperty(const AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) = 0;

    virtual int32_t GetAudioEnhanceProperty(AudioEnhancePropertyArray &propertyArray,
        DeviceType deviceType = DEVICE_TYPE_NONE) = 0;

    virtual void ResetInfo() = 0;

    virtual void UpdateExtraSceneType(const std::string &mainkey, const std::string &subkey,
        const std::string &extraSceneType) = 0;

    virtual int32_t SendInitCommand() = 0;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_ENHANCE_CHAIN_MANAGER_H
