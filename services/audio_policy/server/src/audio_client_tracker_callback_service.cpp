/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioClientTrackerCallbackService"
#endif

#include "audio_policy_log.h"
#include "audio_client_tracker_callback_service.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

AudioClientTrackerCallbackService::AudioClientTrackerCallbackService()
{
}

AudioClientTrackerCallbackService::~AudioClientTrackerCallbackService()
{
}

void AudioClientTrackerCallbackService::SetClientTrackerCallback(
    const std::weak_ptr<AudioClientTracker> &callback)
{
    std::unique_lock<std::mutex> lock(clientTrackerMutex_);
    callback_ = callback;
}

void AudioClientTrackerCallbackService::UnsetClientTrackerCallback()
{
    std::unique_lock<std::mutex> lock(clientTrackerMutex_);
    callback_.reset();
}

int32_t AudioClientTrackerCallbackService::MuteStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    std::unique_lock<std::mutex> lock(clientTrackerMutex_);
    std::shared_ptr<AudioClientTracker> cb = callback_.lock();
    lock.unlock();
    if (cb != nullptr) {
        cb->MuteStreamImpl(streamSetStateEventInternal);
    } else {
        AUDIO_WARNING_LOG("AudioClientTrackerCallbackService: MuteStreamImpl callback_ is nullptr");
    }
    return SUCCESS;
}

int32_t AudioClientTrackerCallbackService::UnmuteStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    std::unique_lock<std::mutex> lock(clientTrackerMutex_);
    std::shared_ptr<AudioClientTracker> cb = callback_.lock();
    lock.unlock();
    if (cb != nullptr) {
        cb->UnmuteStreamImpl(streamSetStateEventInternal);
    } else {
        AUDIO_WARNING_LOG("AudioClientTrackerCallbackService: UnmuteStreamImpl callback_ is nullptr");
    }
    return SUCCESS;
}

int32_t AudioClientTrackerCallbackService::PausedStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    std::unique_lock<std::mutex> lock(clientTrackerMutex_);
    std::shared_ptr<AudioClientTracker> cb = callback_.lock();
    lock.unlock();
    if (cb != nullptr) {
        cb->PausedStreamImpl(streamSetStateEventInternal);
    } else {
        AUDIO_WARNING_LOG("AudioClientTrackerCallbackService: PausedStreamImpl callback_ is nullptr");
    }
    return SUCCESS;
}

int32_t AudioClientTrackerCallbackService::SetLowPowerVolumeImpl(float volume)
{
    std::unique_lock<std::mutex> lock(clientTrackerMutex_);
    std::shared_ptr<AudioClientTracker> cb = callback_.lock();
    lock.unlock();
    if (cb != nullptr) {
        cb->SetLowPowerVolumeImpl(volume);
    } else {
        AUDIO_WARNING_LOG("AudioClientTrackerCallbackService: SetLowPowerVolumeImpl callback_ is nullptr");
    }
    return SUCCESS;
}

int32_t AudioClientTrackerCallbackService::ResumeStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    std::unique_lock<std::mutex> lock(clientTrackerMutex_);
    std::shared_ptr<AudioClientTracker> cb = callback_.lock();
    lock.unlock();
    if (cb != nullptr) {
        cb->ResumeStreamImpl(streamSetStateEventInternal);
    } else {
        AUDIO_WARNING_LOG("AudioClientTrackerCallbackService: ResumeStreamImpl callback_ is nullptr");
    }
    return SUCCESS;
}

int32_t AudioClientTrackerCallbackService::SetOffloadModeImpl(int32_t state, bool isAppBack)
{
    std::unique_lock<std::mutex> lock(clientTrackerMutex_);
    std::shared_ptr<AudioClientTracker> cb = callback_.lock();
    lock.unlock();
    if (cb != nullptr) {
        cb->SetOffloadModeImpl(state, isAppBack);
    } else {
        AUDIO_WARNING_LOG("AudioClientTrackerCallbackService: SetOffloadModeImpl callback_ is nullptr");
    }
    return SUCCESS;
}

int32_t AudioClientTrackerCallbackService::UnsetOffloadModeImpl()
{
    std::unique_lock<std::mutex> lock(clientTrackerMutex_);
    std::shared_ptr<AudioClientTracker> cb = callback_.lock();
    lock.unlock();
    if (cb != nullptr) {
        cb->UnsetOffloadModeImpl();
    } else {
        AUDIO_WARNING_LOG("AudioClientTrackerCallbackService: UnsetOffloadModeImpl callback_ is nullptr");
    }
    return SUCCESS;
}

int32_t AudioClientTrackerCallbackService::GetLowPowerVolumeImpl(float &volume)
{
    std::unique_lock<std::mutex> lock(clientTrackerMutex_);
    std::shared_ptr<AudioClientTracker> cb = callback_.lock();
    lock.unlock();
    if (cb != nullptr) {
        cb->GetLowPowerVolumeImpl(volume);
    } else {
        AUDIO_WARNING_LOG("AudioClientTrackerCallbackService: GetLowPowerVolumeImpl callback_ is nullptr");
    }
    return SUCCESS;
}

int32_t AudioClientTrackerCallbackService::GetSingleStreamVolumeImpl(float &volume)
{
    std::unique_lock<std::mutex> lock(clientTrackerMutex_);
    std::shared_ptr<AudioClientTracker> cb = callback_.lock();
    lock.unlock();
    if (cb != nullptr) {
        cb->GetSingleStreamVolumeImpl(volume);
    } else {
        AUDIO_WARNING_LOG("AudioClientTrackerCallbackService: GetSingleStreamVolumeImpl callback_ is nullptr");
    }
    return SUCCESS;
}

} // namespace AudioStandard
} // namespace OHOS