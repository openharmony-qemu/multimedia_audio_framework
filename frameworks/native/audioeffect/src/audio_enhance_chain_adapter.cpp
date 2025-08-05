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
#ifdef SUPPORT_OLD_ENGINE
#undef LOG_TAG
#define LOG_TAG "AudioEnhanceChainAdapter"

#include "audio_enhance_chain_adapter.h"

#include <algorithm>
#include <map>

#include "audio_effect_log.h"
#include "audio_errors.h"
#include "audio_enhance_chain_manager.h"
#include "audio_effect_map.h"

using namespace OHOS::AudioStandard;

constexpr int32_t SAMPLE_FORMAT_U8 = 8;
constexpr int32_t SAMPLE_FORMAT_S16LE = 16;
constexpr int32_t SAMPLE_FORMAT_S24LE = 24;
constexpr int32_t SAMPLE_FORMAT_S32LE = 32;
constexpr int32_t SAMPLE_FORMAT_F32LE = 32;

namespace {
static const std::map<int32_t, pa_sample_format_t> FORMAT_CONVERT_MAP {
    {SAMPLE_FORMAT_U8, PA_SAMPLE_U8},
    {SAMPLE_FORMAT_S16LE, PA_SAMPLE_S16LE},
    {SAMPLE_FORMAT_S24LE, PA_SAMPLE_S24LE},
    {SAMPLE_FORMAT_S32LE, PA_SAMPLE_S32LE},
    {SAMPLE_FORMAT_F32LE, PA_SAMPLE_FLOAT32LE },
};
}

static pa_sample_format_t ConvertFormat(uint8_t format)
{
    auto item = FORMAT_CONVERT_MAP.find(format);
    if (item != FORMAT_CONVERT_MAP.end()) {
        return item->second;
    }
    return PA_SAMPLE_INVALID;
}

int32_t EnhanceChainManagerCreateCb(const uint64_t sceneKeyCode, const struct DeviceAttrAdapter *adapter)
{
    AudioEnhanceChainManager *audioEnhanceChainMananger = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainMananger != nullptr,
        ERR_INVALID_HANDLE, "null audioEnhanceChainManager");
    AudioEnhanceDeviceAttr deviceAttr = {};
    deviceAttr.micRate = adapter->micRate;
    deviceAttr.micChannels = adapter->micChannels;
    deviceAttr.micFormat = adapter->micFormat;
    if (adapter->needEc) {
        deviceAttr.needEc = adapter->needEc;
        deviceAttr.ecRate = adapter->ecRate;
        deviceAttr.ecChannels = adapter->ecChannels;
        deviceAttr.ecFormat = adapter->ecFormat;
    } else {
        deviceAttr.needEc = false;
    }
    if (adapter->needMicRef) {
        deviceAttr.needMicRef = adapter->needMicRef;
        deviceAttr.micRefRate = adapter->micRefRate;
        deviceAttr.micRefChannels = adapter->micRefChannels;
        deviceAttr.micRefFormat = adapter->micRefFormat;
    } else {
        deviceAttr.needMicRef = false;
    }
    return audioEnhanceChainMananger->CreateAudioEnhanceChainDynamic(sceneKeyCode, deviceAttr);
}

int32_t EnhanceChainManagerReleaseCb(const uint64_t sceneKeyCode)
{
    AudioEnhanceChainManager *audioEnhanceChainMananger = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainMananger != nullptr,
        ERR_INVALID_HANDLE, "null audioEnhanceChainManager");
    return audioEnhanceChainMananger->ReleaseAudioEnhanceChainDynamic(sceneKeyCode);
}

bool EnhanceChainManagerExist(const uint64_t sceneKeyCode)
{
    return true;
}

int32_t EnhanceChainManagerGetAlgoConfig(const uint64_t sceneKeyCode, pa_sample_spec *micSpec,
    pa_sample_spec *ecSpec, pa_sample_spec *micRefSpec)
{
    AudioEnhanceChainManager *audioEnhanceChainMananger = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainMananger != nullptr,
        ERROR, "null audioEnhanceChainManager");
    AudioBufferConfig micConfig = {};
    AudioBufferConfig ecConfig = {};
    AudioBufferConfig micRefConfig = {};

    int32_t ret = audioEnhanceChainMananger->AudioEnhanceChainGetAlgoConfig(sceneKeyCode, micConfig, ecConfig,
        micRefConfig);
    if (ret != 0 || micConfig.samplingRate == 0) {
        return ERROR;
    }

    micSpec->rate = micConfig.samplingRate;
    micSpec->channels = static_cast<uint8_t>(micConfig.channels);
    micSpec->format = ConvertFormat(micConfig.format);

    ecSpec->rate = ecConfig.samplingRate;
    ecSpec->channels = static_cast<uint8_t>(ecConfig.channels);
    ecSpec->format = ConvertFormat(ecConfig.format);

    micRefSpec->rate = micRefConfig.samplingRate;
    micRefSpec->channels = static_cast<uint8_t>(micRefConfig.channels);
    micRefSpec->format = ConvertFormat(micRefConfig.format);

    return SUCCESS;
}

bool EnhanceChainManagerIsEmptyEnhanceChain(void)
{
    return false;
}

int32_t EnhanceChainManagerInitEnhanceBuffer(void)
{
    return SUCCESS;
}

int32_t CopyToEnhanceBufferAdapter(void *data, uint32_t length)
{
    return SUCCESS;
}

int32_t CopyEcdataToEnhanceBufferAdapter(void *data, uint32_t length)
{
    return SUCCESS;
}

int32_t CopyMicRefdataToEnhanceBufferAdapter(void *data, uint32_t length)
{
    return SUCCESS;
}

int32_t CopyFromEnhanceBufferAdapter(void *data, uint32_t length)
{
    return SUCCESS;
}

int32_t EnhanceChainManagerProcess(const uint64_t sceneKeyCode, uint32_t length)
{
    return SUCCESS;
}

int32_t EnhanceChainManagerProcessDefault(const uint32_t captureId, uint32_t length)
{
    AudioEnhanceChainManager *audioEnhanceChainMananger = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainMananger != nullptr,
        ERR_INVALID_HANDLE, "null audioEnhanceChainManager");
    AUDIO_DEBUG_LOG("%{public}u default process success", captureId);
    return SUCCESS;
}

int32_t GetSceneTypeCode(const char *sceneType, uint64_t *sceneTypeCode)
{
    std::string sceneTypeString = "";
    const std::unordered_map<AudioEnhanceScene, std::string> &audioEnhanceSupportedSceneTypes =
        GetEnhanceSupportedSceneType();
    if (sceneType) {
        sceneTypeString = sceneType;
    } else {
        return ERROR;
    }
    auto item = std::find_if(audioEnhanceSupportedSceneTypes.begin(), audioEnhanceSupportedSceneTypes.end(),
        [&sceneTypeString](const std::pair<AudioEnhanceScene, std::string>& element) -> bool {
            return element.second == sceneTypeString;
        });
    if (item == audioEnhanceSupportedSceneTypes.end()) {
        return ERROR;
    }
    *sceneTypeCode = static_cast<uint64_t>(item->first);
    return SUCCESS;
}

int32_t EnhanceChainManagerSendInitCommand()
{
    AudioEnhanceChainManager *audioEnhanceChainMananger = AudioEnhanceChainManager::GetInstance();
    CHECK_AND_RETURN_RET_LOG(audioEnhanceChainMananger != nullptr,
        ERROR, "null audioEnhanceChainManager");
    return audioEnhanceChainMananger->SendInitCommand();
}
#endif // SUPPORT_OLD_ENGINE