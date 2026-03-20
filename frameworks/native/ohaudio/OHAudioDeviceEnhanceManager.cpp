/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "OHAudioDeviceEnhanceManager.h"

#include "audio_common_log.h"
#include "audio_device_enhance_manager.h"
#include "audio_errors.h"
#include "OHAudioCommon.h"
#include "OHAudioRenderer.h"
#include "OHAudioCapturer.h"

namespace {
static OHOS::AudioStandard::OHAudioDeviceEnhanceManager *ConvertEnhanceManager(OH_AudioDeviceEnhanceManager *manager)
{
    return reinterpret_cast<OHOS::AudioStandard::OHAudioDeviceEnhanceManager *>(manager);
}

static OHOS::AudioStandard::OHAudioRenderer *ConvertRenderer(OH_AudioRenderer *renderer)
{
    return reinterpret_cast<OHOS::AudioStandard::OHAudioRenderer *>(renderer);
}

static OHOS::AudioStandard::OHAudioCapturer *ConvertCapturer(OH_AudioCapturer *capturer)
{
    return reinterpret_cast<OHOS::AudioStandard::OHAudioCapturer *>(capturer);
}
} // namespace

namespace OHOS {
namespace AudioStandard {
OHAudioDeviceEnhanceManager &OHAudioDeviceEnhanceManager::GetInstance()
{
    static OHAudioDeviceEnhanceManager manager;
    return manager;
}

int32_t OHAudioDeviceEnhanceManager::IsEnhancedRoutingSupported(bool &supported)
{
    return AudioDeviceEnhanceManager::GetInstance().IsEnhancedRoutingSupported(supported);
}

int32_t OHAudioDeviceEnhanceManager::SelectOutputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    return AudioDeviceEnhanceManager::GetInstance().SelectOutputDevice(desc);
}

int32_t OHAudioDeviceEnhanceManager::SelectInputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc)
{
    return AudioDeviceEnhanceManager::GetInstance().SelectInputDevice(desc);
}
} // namespace AudioStandard
} // namespace OHOS

OH_AudioCommon_Result OH_AudioManager_GetAudioDeviceEnhanceManager(
    OH_AudioDeviceEnhanceManager **audioDeviceEnhanceManager)
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceEnhanceManager != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "audioDeviceEnhanceManager is nullptr");
    auto &ins = OHOS::AudioStandard::OHAudioDeviceEnhanceManager::GetInstance();
    *audioDeviceEnhanceManager = reinterpret_cast<OH_AudioDeviceEnhanceManager *>(&ins);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioDeviceEnhanceManager_IsEnhancedRoutingSupported(
    OH_AudioDeviceEnhanceManager *audioDeviceEnhanceManager, bool *supported)
{
    auto *manager = ConvertEnhanceManager(audioDeviceEnhanceManager);
    CHECK_AND_RETURN_RET_LOG(manager != nullptr && supported != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "invalid params");
    int32_t ret = manager->IsEnhancedRoutingSupported(*supported);
    return OHOS::AudioStandard::OHAudioCommon::ConvertResult(ret);
}

OH_AudioCommon_Result OH_AudioDeviceEnhanceManager_SelectOutputDevice(
    OH_AudioDeviceEnhanceManager *audioDeviceEnhanceManager, OH_AudioDeviceDescriptor *deviceDescriptor)
{
    auto *manager = ConvertEnhanceManager(audioDeviceEnhanceManager);
    CHECK_AND_RETURN_RET_LOG(manager != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "manager is nullptr");
    int32_t ret = manager->SelectOutputDevice(OHOS::AudioStandard::OHAudioCommon::ConvertDevice(deviceDescriptor));
    return OHOS::AudioStandard::OHAudioCommon::ConvertResult(ret);
}

OH_AudioCommon_Result OH_AudioDeviceEnhanceManager_SelectInputDevice(
    OH_AudioDeviceEnhanceManager *audioDeviceEnhanceManager, OH_AudioDeviceDescriptor *deviceDescriptor)
{
    auto *manager = ConvertEnhanceManager(audioDeviceEnhanceManager);
    CHECK_AND_RETURN_RET_LOG(manager != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "manager is nullptr");
    int32_t ret = manager->SelectInputDevice(OHOS::AudioStandard::OHAudioCommon::ConvertDevice(deviceDescriptor));
    return OHOS::AudioStandard::OHAudioCommon::ConvertResult(ret);
}

OH_AudioCommon_Result OH_AudioDeviceEnhanceManager_SelectOutputDeviceForAudioRenderer(
    OH_AudioDeviceEnhanceManager *audioDeviceEnhanceManager, OH_AudioRenderer *renderer,
    OH_AudioDeviceDescriptor *deviceDescriptor)
{
    auto *manager = ConvertEnhanceManager(audioDeviceEnhanceManager);
    auto *audioRender = ConvertRenderer(renderer);
    CHECK_AND_RETURN_RET_LOG(manager != nullptr && audioRender != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "invalid params");
    int32_t ret = audioRender->SelectOutputDevice(deviceDescriptor);
    return OHOS::AudioStandard::OHAudioCommon::ConvertResult(ret);
}

OH_AudioCommon_Result OH_AudioDeviceEnhanceManager_SelectInputDeviceForAudioCapturer(
    OH_AudioDeviceEnhanceManager *audioDeviceEnhanceManager, OH_AudioCapturer *capturer,
    OH_AudioDeviceDescriptor *deviceDescriptor)
{
    auto *manager = ConvertEnhanceManager(audioDeviceEnhanceManager);
    auto *audioCapturer = ConvertCapturer(capturer);
    CHECK_AND_RETURN_RET_LOG(manager != nullptr && audioCapturer != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
        "invalid params");
    int32_t ret = audioCapturer->SelectInputDevice(deviceDescriptor);
    return OHOS::AudioStandard::OHAudioCommon::ConvertResult(ret);
}
