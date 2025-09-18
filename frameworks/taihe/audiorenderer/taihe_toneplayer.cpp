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
#ifndef LOG_TAG
#define LOG_TAG "TonePlayerImpl"
#endif

#include "taihe_toneplayer.h"
#include "audio_utils.h"
#include "taihe_audio_error.h"
#include "taihe_param_utils.h"

namespace ANI::Audio {
std::mutex TonePlayerImpl::createMutex_;
int32_t TonePlayerImpl::isConstructSuccess_ = OHOS::AudioStandard::SUCCESS;

TonePlayerImpl::TonePlayerImpl(std::shared_ptr<OHOS::AudioStandard::TonePlayer> obj)
{
    if (obj != nullptr) {
        tonePlayer_ = obj;
    }
}

TonePlayer TonePlayerImpl::CreateTonePlayerWrapper(
    std::unique_ptr<OHOS::AudioStandard::AudioRendererInfo> rendererInfo)
{
    std::lock_guard<std::mutex> lock(TonePlayerImpl::createMutex_);
    if (rendererInfo == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Failed in CreateTonePlayerWrapper");
        return make_holder<TonePlayerImpl, TonePlayer>();
    }

    std::string cacheDir = "/data/storage/el2/base/cache";
    /* TonePlayerImpl not support other rendererFlags, only support flag 0 */
    if (rendererInfo->rendererFlags != 0) {
        rendererInfo->rendererFlags = 0;
    }

    auto tonePlayer = OHOS::AudioStandard::TonePlayer::Create(cacheDir, *(rendererInfo.get()));
    if (tonePlayer  == nullptr) {
        TonePlayerImpl::isConstructSuccess_ = TAIHE_ERR_PERMISSION_DENIED;
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Toneplayer Create failed");
        return make_holder<TonePlayerImpl, TonePlayer>();
    }
    return make_holder<TonePlayerImpl, TonePlayer>(tonePlayer);
}

void TonePlayerImpl::LoadSync(ToneType type)
{
    int32_t toneType = static_cast<int32_t>(type);
    if (tonePlayer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "tonePlayer_ is nullptr");
        return;
    }
    bool isTrue = tonePlayer_->LoadTone(static_cast<OHOS::AudioStandard::ToneType>(toneType));
    if (!isTrue) {
        TaiheAudioError::ThrowError(TAIHE_ERR_SYSTEM);
    }
}

void TonePlayerImpl::ReleaseSync()
{
    if (tonePlayer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "tonePlayer_ is nullptr");
        return;
    }
    bool isTrue = tonePlayer_->Release();
    if (!isTrue) {
        TaiheAudioError::ThrowError(TAIHE_ERR_SYSTEM);
    }
}

void TonePlayerImpl::StopSync()
{
    if (tonePlayer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "tonePlayer_ is nullptr");
        return;
    }
    bool isTrue = tonePlayer_->StopTone();
    if (!isTrue) {
        TaiheAudioError::ThrowError(TAIHE_ERR_SYSTEM);
    }
}

void TonePlayerImpl::StartSync()
{
    if (tonePlayer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "tonePlayer_ is nullptr");
        return;
    }
    bool isTrue = tonePlayer_->StartTone();
    if (!isTrue) {
        TaiheAudioError::ThrowError(TAIHE_ERR_SYSTEM);
    }
}

TonePlayer CreateTonePlayerSync(AudioRendererInfo const &options)
{
    OHOS::AudioStandard::AudioRendererInfo rendererInfo;
    if (TaiheParamUtils::GetRendererInfo(rendererInfo, options) != AUDIO_OK) {
        AUDIO_ERR_LOG("GetRendererInfo failed");
        TaiheAudioError::ThrowError(TAIHE_ERR_INVALID_PARAM);
        return make_holder<TonePlayerImpl, TonePlayer>();
    }

    std::unique_ptr<OHOS::AudioStandard::AudioRendererInfo> audioRendererInfo =
        std::make_unique<OHOS::AudioStandard::AudioRendererInfo>(rendererInfo);
    if (audioRendererInfo == nullptr) {
        AUDIO_ERR_LOG("audioRendererInfo create failed,no memory.");
        TaiheAudioError::ThrowError(TAIHE_ERR_SYSTEM);
        return make_holder<TonePlayerImpl, TonePlayer>();
    }
    return TonePlayerImpl::CreateTonePlayerWrapper(std::move(audioRendererInfo));
}
} // namespace ANI::Audio

TH_EXPORT_CPP_API_CreateTonePlayerSync(ANI::Audio::CreateTonePlayerSync);
