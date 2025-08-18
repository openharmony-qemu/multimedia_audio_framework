/*
 * Copyright (C) 2025 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AsrProcessingControllerImpl"
#endif

#include "taihe_asr_processing_controller.h"

#include <map>
#include <string>
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "taihe_audio_enum.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"
#include "taihe_audio_capturer.h"

namespace ANI::Audio {
using namespace std;
using namespace OHOS::HiviewDFX;

static const std::map<int32_t, int32_t> ERR_MAP = {
    {OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED, TAIHE_ERR_PERMISSION_DENIED},
    {OHOS::AudioStandard::ERR_INVALID_PARAM, TAIHE_ERR_INVALID_PARAM },
    {OHOS::AudioStandard::ERROR, TAIHE_ERR_UNSUPPORTED},
    {-1, TAIHE_ERR_UNSUPPORTED},
};

static const std::map<int32_t, std::string> ERR_INFO_MAP = {
    {OHOS::AudioStandard::ERR_SYSTEM_PERMISSION_DENIED, "Caller is not a system application."},
    {OHOS::AudioStandard::ERR_INVALID_PARAM, "Parameter verification failed. : The param of mode must be mode enum"},
    {OHOS::AudioStandard::ERROR, "Operation not allowed."},
    {-1, "Operation not allowed."},
};

static int32_t GetResInt(int32_t errNum)
{
    auto it = ERR_MAP.find(errNum);
    int32_t errInt = TAIHE_ERR_UNSUPPORTED;
    if (it != ERR_MAP.end()) {
        errInt = it->second;
    } else {
        AUDIO_ERR_LOG("err not found.");
    }
    return errInt;
}

static std::string GetResStr(int32_t errNum)
{
    auto it = ERR_INFO_MAP.find(errNum);
    std::string errStr = "Operation not allowed.";
    if (it != ERR_INFO_MAP.end()) {
        errStr = it->second;
    } else {
        AUDIO_ERR_LOG("err not found.");
    }
    return errStr;
}

static bool CheckCapturerValid(weak::AudioCapturer capturer)
{
    AudioCapturerImpl *taiheCapturer = reinterpret_cast<AudioCapturerImpl*>(capturer->GetImplPtr());

    if (taiheCapturer == nullptr) {
        AUDIO_ERR_LOG("taihe capturer is nullptr");
        return false;
    }

    taiheCapturer->audioCapturer_ = taiheCapturer->GetNativePtr();
    if (taiheCapturer->audioCapturer_ == nullptr) {
        AUDIO_ERR_LOG("taihe capturer is nullptr");
        return false;
    }

    OHOS::AudioStandard::AudioCapturerInfo capturerInfo;
    taiheCapturer->audioCapturer_->GetCapturerInfo(capturerInfo);
    if ((capturerInfo.sourceType != OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_RECOGNITION) &&
        (capturerInfo.sourceType != OHOS::AudioStandard::SourceType::SOURCE_TYPE_WAKEUP) &&
        (capturerInfo.sourceType != OHOS::AudioStandard::SourceType::SOURCE_TYPE_VOICE_CALL)) {
        AUDIO_ERR_LOG("sourceType not valid. type : %{public}d", capturerInfo.sourceType);
        return false;
    }
    return true;
}

AsrProcessingControllerImpl::AsrProcessingControllerImpl()
    : audioMngr_(nullptr) {}

AsrProcessingControllerImpl::AsrProcessingControllerImpl(std::shared_ptr<AsrProcessingControllerImpl> obj)
    : audioMngr_(nullptr)
{
    if (obj != nullptr) {
        audioMngr_ = obj->audioMngr_;
    }
}

bool AsrProcessingControllerImpl::SetAsrAecMode(::ohos::multimedia::audio::AsrAecMode mode)
{
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return false;
    }
    int32_t asrAecMode = mode.get_value();
    int32_t asrAecModeMax = static_cast<int32_t>(::AsrAecMode::FOLDED);
    if (!(asrAecMode >= 0 && asrAecMode <= asrAecModeMax)) {
        AUDIO_ERR_LOG("Input parameter value error. ");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of mode must be enum AsrAecMode");
        return false;
    }
    int32_t res = audioMngr_->SetAsrAecMode(static_cast<::AsrAecMode>(asrAecMode));
    if (res != 0) {
        AUDIO_ERR_LOG("SetAsrAecMode fail");
        TaiheAudioError::ThrowErrorAndReturn(GetResInt(res), GetResStr(res));
        return false;
    }
    return (res == 0);
}

::ohos::multimedia::audio::AsrAecMode AsrProcessingControllerImpl::GetAsrAecMode()
{
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return ::ohos::multimedia::audio::AsrAecMode::key_t::BYPASS;
    }
    ::AsrAecMode asrAecMode;
    int32_t res = audioMngr_->GetAsrAecMode(asrAecMode);
    if (res != 0) {
        AUDIO_ERR_LOG("SetAsrAecMode fail");
        TaiheAudioError::ThrowErrorAndReturn(GetResInt(res), GetResStr(res));
        return ::ohos::multimedia::audio::AsrAecMode::key_t::BYPASS;
    }
    return TaiheAudioEnum::ToTaiheAsrAecMode(asrAecMode);
}

bool AsrProcessingControllerImpl::SetAsrNoiseSuppressionMode(::ohos::multimedia::audio::AsrNoiseSuppressionMode mode)
{
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return false;
    }

    int32_t asrNoiseSuppressionMode = mode.get_value();
    int32_t asrVoiceControlModeMax = static_cast<int32_t>(::AsrNoiseSuppressionMode::ASR_WHISPER_MODE);
    if (!(asrNoiseSuppressionMode >= 0 && asrNoiseSuppressionMode <= asrVoiceControlModeMax)) {
        AUDIO_ERR_LOG("Input parameter value error. ");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of mode must be enum AsrNoiseSuppressionMode");
        return false;
    }
    int32_t res = audioMngr_->SetAsrNoiseSuppressionMode(
        static_cast<::AsrNoiseSuppressionMode>(asrNoiseSuppressionMode));
    if (res != 0) {
        AUDIO_ERR_LOG("SetNSMode fail");
        TaiheAudioError::ThrowErrorAndReturn(GetResInt(res), GetResStr(res));
        return false;
    }
    bool setSuc = ((res == 0) ? true : false);
    return setSuc;
}

::ohos::multimedia::audio::AsrNoiseSuppressionMode AsrProcessingControllerImpl::GetAsrNoiseSuppressionMode()
{
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return ::ohos::multimedia::audio::AsrNoiseSuppressionMode::key_t::BYPASS;
    }
    ::AsrNoiseSuppressionMode asrNoiseSuppressionMode;
    int32_t res = audioMngr_->GetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    if (res != 0) {
        AUDIO_ERR_LOG("GetNSMode fail");
        TaiheAudioError::ThrowErrorAndReturn(GetResInt(res), GetResStr(res));
        return ::ohos::multimedia::audio::AsrNoiseSuppressionMode::key_t::BYPASS;
    }
    return TaiheAudioEnum::ToTaiheAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
}

bool AsrProcessingControllerImpl::SetAsrWhisperDetectionMode(::ohos::multimedia::audio::AsrWhisperDetectionMode mode)
{
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return false;
    }

    int32_t asrWhisperDetectionMode = mode.get_value();
    if (!(asrWhisperDetectionMode == 0 || asrWhisperDetectionMode == 1)) {
        AUDIO_ERR_LOG("Input parameter value error. ");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM);
        return false;
    }
    int32_t res = audioMngr_->SetAsrWhisperDetectionMode(
        static_cast<::AsrWhisperDetectionMode>(asrWhisperDetectionMode));
    if (res != 0) {
        AUDIO_ERR_LOG("SetAsrWhisperDetectionMode fail");
        TaiheAudioError::ThrowErrorAndReturn(GetResInt(res), GetResStr(res));
        return false;
    }
    bool setSuc = ((res == 0) ? true : false);
    return setSuc;
}

::ohos::multimedia::audio::AsrWhisperDetectionMode AsrProcessingControllerImpl::GetAsrWhisperDetectionMode()
{
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return ::ohos::multimedia::audio::AsrWhisperDetectionMode::key_t::BYPASS;
    }
    ::AsrWhisperDetectionMode asrWhisperDetectionMode;
    int32_t res = audioMngr_->GetAsrWhisperDetectionMode(asrWhisperDetectionMode);
    if (res != 0) {
        AUDIO_ERR_LOG("GetAsrWhisperDetectionMode fail");
        TaiheAudioError::ThrowErrorAndReturn(GetResInt(res), GetResStr(res));
        return ::ohos::multimedia::audio::AsrWhisperDetectionMode::key_t::BYPASS;
    }
    return TaiheAudioEnum::ToTaiheAsrWhisperDetectionMode(asrWhisperDetectionMode);
}

bool AsrProcessingControllerImpl::SetAsrVoiceControlMode(::ohos::multimedia::audio::AsrVoiceControlMode mode,
    bool enable)
{
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return false;
    }

    int32_t asrVoiceControlMode = mode.get_value();
    int32_t asrVoiceControlModeMax = static_cast<int32_t>(::AsrVoiceControlMode::VOICE_TXRX_DECREASE);
    if (!(asrVoiceControlMode >= 0 && asrVoiceControlMode <= asrVoiceControlModeMax)) {
        AUDIO_ERR_LOG("Input parameter value error. ");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of mode must be enum AsrVoiceControlMode");
        return false;
    }
    int32_t res = audioMngr_->SetAsrVoiceControlMode(static_cast<::AsrVoiceControlMode>(asrVoiceControlMode), enable);
    if (res != 0) {
        AUDIO_ERR_LOG("SetVCMode fail");
        TaiheAudioError::ThrowErrorAndReturn(GetResInt(res), GetResStr(res));
        return false;
    }
    bool setSuc = ((res == 0) ? true : false);
    return setSuc;
}

bool AsrProcessingControllerImpl::SetAsrVoiceMuteMode(::ohos::multimedia::audio::AsrVoiceMuteMode mode, bool enable)
{
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return false;
    }

    int32_t asrVoiceMuteMode = mode.get_value();
    int32_t asrVoiceMuteModeMax = static_cast<int32_t>(::AsrVoiceMuteMode::OUTPUT_MUTE_EX);
    if (!(asrVoiceMuteMode >= 0 && asrVoiceMuteMode <= asrVoiceMuteModeMax)) {
        AUDIO_ERR_LOG("Input parameter value error. ");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM,
            "parameter verification failed: The param of mode must be enum AsrVoiceMuteMode");
        return false;
    }
    int32_t res = audioMngr_->SetAsrVoiceMuteMode(static_cast<::AsrVoiceMuteMode>(asrVoiceMuteMode), enable);
    if (res != 0) {
        AUDIO_ERR_LOG("SetVMMode fail");
        TaiheAudioError::ThrowErrorAndReturn(GetResInt(res), GetResStr(res));
        return false;
    }
    bool setSuc = ((res == 0) ? true : false);
    return setSuc;
}

bool AsrProcessingControllerImpl::IsWhispering()
{
    if (audioMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioMngr_ is nullptr");
        return false;
    }
    int32_t res = audioMngr_->IsWhispering();
    if (!(res == 0 || res == 1)) {
        AUDIO_ERR_LOG("IsWhispering fail");
        TaiheAudioError::ThrowErrorAndReturn(GetResInt(res), GetResStr(res));
        return false;
    }
    bool setSuc = ((res == 0) ? true : false);
    return setSuc;
}

AsrProcessingController CreateAsrProcessingController(weak::AudioCapturer audioCapturer)
{
    if (!OHOS::AudioStandard::PermissionUtil::VerifySelfPermission()) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_PERMISSION_DENIED, "No system permission");
        return make_holder<AsrProcessingControllerImpl, AsrProcessingController>(nullptr);
    }
    bool isCapturerValid = CheckCapturerValid(audioCapturer);
    if (!isCapturerValid) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_UNSUPPORTED, "Operation not allowed. ");
        return make_holder<AsrProcessingControllerImpl, AsrProcessingController>(nullptr);
    }
    shared_ptr<AsrProcessingControllerImpl> asrControllerImpl = make_shared<AsrProcessingControllerImpl>();
    if (asrControllerImpl == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "asrControllerImpl is nullptr");
        return make_holder<AsrProcessingControllerImpl, AsrProcessingController>(nullptr);
    }
    asrControllerImpl->audioMngr_ = OHOS::AudioStandard::AudioSystemManager::GetInstance();
    return make_holder<AsrProcessingControllerImpl, AsrProcessingController>(asrControllerImpl);
}
} // namespace ANI::Audio

TH_EXPORT_CPP_API_CreateAsrProcessingController(ANI::Audio::CreateAsrProcessingController);