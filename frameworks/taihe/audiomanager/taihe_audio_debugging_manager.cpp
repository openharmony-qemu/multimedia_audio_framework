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
#include "audio_log.h"

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
    AUDIO_DEBUG_LOG("in");
    auto *audioDebugMngr = &OHOS::AudioStandard::AudioDebugManager::GetInstance();
    if (audioDebugMngr == nullptr) {
        AUDIO_ERR_LOG("Failed to get AudioDebugManager instance");
        TaiheAudioError::ThrowErrorAndReturn(TAIHE_ERR_SYSTEM, "Failed to get AudioDebugManager instance");
        return make_holder<AudioDebuggingManagerImpl, AudioDebuggingManager>(nullptr);
    }
    return make_holder<AudioDebuggingManagerImpl, AudioDebuggingManager>(audioDebugMngr);
}

void AudioDebuggingManagerImpl::PrintAppInfo(int32_t fd)
{
    AUDIO_DEBUG_LOG("in, fd:%{public}d", fd);
    CHECK_AND_RETURN_LOG(audioDebugMngr_ != nullptr, "audioDebugMngr_ is nullptr");
    int32_t result = audioDebugMngr_->PrintAppAudioDebugInfo(fd);
    CHECK_AND_RETURN_LOG(result == AUDIO_OK,
        "PrintAppAudioDebugInfo failed, result:%{public}d", result);
}

void AudioDebuggingManagerImpl::PrintRendererInfo(weak::AudioRenderer renderer, int32_t fd)
{
    AUDIO_DEBUG_LOG("in, fd:%{public}d", fd);
    AudioRendererImpl *taiheRenderer = reinterpret_cast<AudioRendererImpl *>(renderer->GetImplPtr());
    CHECK_AND_RETURN_LOG(taiheRenderer != nullptr, "renderer impl is nullptr");
    CHECK_AND_RETURN_LOG(taiheRenderer->audioRenderer_ != nullptr, "renderer native object is nullptr");
    CHECK_AND_RETURN_LOG(audioDebugMngr_ != nullptr, "audioDebugMngr_ is nullptr");
    uint32_t rendererKey = taiheRenderer->audioRenderer_->GetRendererDebugKey();
    AUDIO_DEBUG_LOG("rendererKey:%{public}u", rendererKey);
    int32_t result = audioDebugMngr_->PrintAudioRendererDebugInfo(rendererKey, fd);
    CHECK_AND_RETURN_LOG(result == AUDIO_OK,
        "PrintAudioRendererDebugInfo failed, rendererKey:%{public}u, result:%{public}d",
        rendererKey, result);
}

void AudioDebuggingManagerImpl::PrintCapturerInfo(weak::AudioCapturer capturer, int32_t fd)
{
    AUDIO_DEBUG_LOG("in, fd:%{public}d", fd);
    AudioCapturerImpl *taiheCapturer = reinterpret_cast<AudioCapturerImpl *>(capturer->GetImplPtr());
    CHECK_AND_RETURN_LOG(taiheCapturer != nullptr, "capturer impl is nullptr");
    CHECK_AND_RETURN_LOG(taiheCapturer->audioCapturer_ != nullptr, "capturer native object is nullptr");
    CHECK_AND_RETURN_LOG(audioDebugMngr_ != nullptr, "audioDebugMngr_ is nullptr");
    uint32_t capturerKey = taiheCapturer->audioCapturer_->GetCapturerDebugKey();
    AUDIO_DEBUG_LOG("capturerKey:%{public}u", capturerKey);
    int32_t result = audioDebugMngr_->PrintAudioCapturerDebugInfo(capturerKey, fd);
    CHECK_AND_RETURN_LOG(result == AUDIO_OK,
        "PrintAudioCapturerDebugInfo failed, capturerKey:%{public}u, result:%{public}d",
        capturerKey, result);
}

void AudioDebuggingManagerImpl::PrintLoopbackInfo(AudioLpback loopback, int32_t fd)
{
    AUDIO_DEBUG_LOG("in, fd:%{public}d", fd);
    AudioLoopback audioLoopback = loopback.GetTypeAudioLpback();
    AudioLoopbackImpl *taiheLoopback = reinterpret_cast<AudioLoopbackImpl *>(audioLoopback.GetImplPtr());
    CHECK_AND_RETURN_LOG(taiheLoopback != nullptr, "loopback impl is nullptr");
    CHECK_AND_RETURN_LOG(taiheLoopback->loopback_ != nullptr, "loopback native object is nullptr");
    CHECK_AND_RETURN_LOG(audioDebugMngr_ != nullptr, "audioDebugMngr_ is nullptr");
    uint32_t loopbackKey = taiheLoopback->loopback_->GetDebugKey();
    AUDIO_DEBUG_LOG("loopbackKey:%{public}u", loopbackKey);
    int32_t result = audioDebugMngr_->PrintAudioLoopbackDebugInfo(loopbackKey, fd);
    CHECK_AND_RETURN_LOG(result == AUDIO_OK,
        "PrintAudioLoopbackDebugInfo failed, loopbackKey:%{public}u, result:%{public}d",
        loopbackKey, result);
}

void AudioDebuggingManagerImpl::PrintSessionInfo(weak::AudioSessionManager session, int32_t fd)
{
    AUDIO_DEBUG_LOG("in, fd:%{public}d", fd);
    CHECK_AND_RETURN_LOG(audioDebugMngr_ != nullptr, "audioDebugMngr_ is nullptr");
    int32_t result = audioDebugMngr_->PrintAudioSessionDebugInfo(fd);
    CHECK_AND_RETURN_LOG(result == AUDIO_OK,
        "PrintAudioSessionDebugInfo failed, result:%{public}d", result);
}
} // namespace ANI::Audio
