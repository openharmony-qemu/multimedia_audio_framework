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
#ifndef LOG_TAG
#define LOG_TAG "TaiheAudioDebuggingMgr"
#endif

#include "taihe_audio_debugging_manager.h"
#include "taihe_audio_error.h"
#include "taihe_audio_renderer.h"
#include "taihe_audio_capturer.h"
#include "taihe_audio_loopback.h"
#include "taihe_audio_session_manager.h"
#include "audio_errors.h"

namespace ANI::Audio {
using namespace OHOS::HiviewDFX;

AudioDebuggingManagerImpl::AudioDebuggingManagerImpl() : audioDebugMngr_(nullptr) {}

AudioDebuggingManagerImpl::AudioDebuggingManagerImpl(OHOS::AudioStandard::AudioDebugManager *audioDebugMngr)
    : audioDebugMngr_(nullptr)
{
    if (audioDebugMngr != nullptr) {
        audioDebugMngr_ = audioDebugMngr;
    }
}

AudioDebuggingManagerImpl::~AudioDebuggingManagerImpl() = default;

AudioDebuggingManager AudioDebuggingManagerImpl::CreateDebuggingManagerWrapper()
{
    auto *audioDebugMngr = &OHOS::AudioStandard::AudioDebugManager::GetInstance();
    if (audioDebugMngr == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Failed to get AudioDebugManager instance");
        return make_holder<AudioDebuggingManagerImpl, AudioDebuggingManager>(nullptr);
    }
    return make_holder<AudioDebuggingManagerImpl, AudioDebuggingManager>(audioDebugMngr);
}

void AudioDebuggingManagerImpl::PrintAppInfo(int32_t fd)
{
    if (audioDebugMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioDebugMngr_ is nullptr");
        return;
    }
    int32_t result = audioDebugMngr_->PrintAppAudioDebugInfo(fd);
    if (result != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "PrintAppInfo failed");
    }
}

void AudioDebuggingManagerImpl::PrintRendererInfo(weak::AudioRenderer renderer, int32_t fd)
{
    AudioRendererImpl *taiheRenderer = reinterpret_cast<AudioRendererImpl *>(renderer->GetImplPtr());
    if (taiheRenderer == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "renderer impl is nullptr");
        return;
    }
    if (taiheRenderer->audioRenderer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "renderer native object is nullptr");
        return;
    }
    if (audioDebugMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioDebugMngr_ is nullptr");
        return;
    }
    uintptr_t rendererKey = reinterpret_cast<uintptr_t>(taiheRenderer->audioRenderer_.get());
    int32_t result = audioDebugMngr_->PrintAudioRendererDebugInfo(rendererKey, fd);
    if (result != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "PrintRendererInfo failed");
    }
}

void AudioDebuggingManagerImpl::PrintCapturerInfo(weak::AudioCapturer capturer, int32_t fd)
{
    AudioCapturerImpl *taiheCapturer = reinterpret_cast<AudioCapturerImpl *>(capturer->GetImplPtr());
    if (taiheCapturer == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "capturer impl is nullptr");
        return;
    }
    if (taiheCapturer->audioCapturer_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "capturer native object is nullptr");
        return;
    }
    if (audioDebugMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioDebugMngr_ is nullptr");
        return;
    }
    uintptr_t capturerKey = reinterpret_cast<uintptr_t>(taiheCapturer->audioCapturer_.get());
    int32_t result = audioDebugMngr_->PrintAudioCapturerDebugInfo(capturerKey, fd);
    if (result != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "PrintCapturerInfo failed");
    }
}

void AudioDebuggingManagerImpl::PrintLoopbackInfo(weak::AudioLpback loopback, int32_t fd)
{
    AudioLoopbackImpl *taiheLoopback = reinterpret_cast<AudioLoopbackImpl *>(loopback->GetImplPtr());
    if (taiheLoopback == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "loopback impl is nullptr");
        return;
    }
    if (taiheLoopback->loopback_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_INVALID_PARAM, "loopback native object is nullptr");
        return;
    }
    if (audioDebugMngr_ == nullptr) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "audioDebugMngr_ is nullptr");
        return;
    }
    uint32_t loopbackKey = reinterpret_cast<uint32_t>(taiheLoopback->loopback_.get());
    int32_t result = audioDebugMngr_->PrintAudioLoopbackDebugInfo(loopbackKey, fd);
    if (result != AUDIO_OK) {
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "PrintLoopbackInfo failed");
    }
}

void AudioDebuggingManagerImpl::PrintSessionInfo(weak::AudioSessionManager session, int32_t fd)
{
    TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_UNSUPPORTED, "PrintSessionInfo is unsupported now");
}
} // namespace ANI::Audio
