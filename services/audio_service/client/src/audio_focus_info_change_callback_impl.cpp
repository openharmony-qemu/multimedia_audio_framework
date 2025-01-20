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
#define LOG_TAG "AudioFocusInfoChangeCallbackImpl"
#endif

#include "audio_focus_info_change_callback_impl.h"

#include <mutex>
#include "audio_common_log.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
AudioFocusInfoChangeCallbackImpl::AudioFocusInfoChangeCallbackImpl()
{
    AUDIO_INFO_LOG("AudioFocusInfoChangeCallbackImpl constructor");
}

AudioFocusInfoChangeCallbackImpl::~AudioFocusInfoChangeCallbackImpl()
{
    AUDIO_INFO_LOG("AudioFocusInfoChangeCallbackImpl: destroy");
}

void AudioFocusInfoChangeCallbackImpl::SaveCallback(const std::weak_ptr<AudioFocusInfoChangeCallback> &callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    bool hasCallback = false;
    std::lock_guard<std::mutex> cbListLock(cbListMutex_);
    for (auto it = callbackList_.begin(); it != callbackList_.end(); ++it) {
        if ((*it).lock() == callback.lock()) {
            hasCallback = true;
        }
    }
    if (!hasCallback) {
        callbackList_.push_back(callback);
    }
}

void AudioFocusInfoChangeCallbackImpl::RemoveCallback(const std::weak_ptr<AudioFocusInfoChangeCallback> &callback)
{
    AUDIO_INFO_LOG("Entered %{public}s", __func__);
    std::lock_guard<std::mutex> cbListLock(cbListMutex_);
    callbackList_.remove_if([&callback](std::weak_ptr<AudioFocusInfoChangeCallback> &callback_) {
        return callback_.lock() == callback.lock();
    });
}

void AudioFocusInfoChangeCallbackImpl::OnAudioFocusInfoChange(
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList)
{
    AUDIO_DEBUG_LOG("on callback Entered AudioFocusInfoChangeCallbackImpl %{public}s", __func__);
    std::vector<std::shared_ptr<AudioFocusInfoChangeCallback>> temp_;
    std::unique_lock<std::mutex> cbListLock(cbListMutex_);
    for (auto callback = callbackList_.begin(); callback != callbackList_.end(); ++callback) {
        cb_ = (*callback).lock();
        if (cb_ != nullptr) {
            AUDIO_DEBUG_LOG("OnAudioFocusInfoChange : Notify event to app complete");
            temp_.push_back(cb_);
        } else {
            AUDIO_ERR_LOG("OnAudioFocusInfoChange: callback is null");
        }
    }
    cbListLock.unlock();
    for (uint32_t i = 0; i < temp_.size(); i++) {
        temp_[i]->OnAudioFocusInfoChange(focusInfoList);
    }
    return;
}

void AudioFocusInfoChangeCallbackImpl::OnAudioFocusRequested(const AudioInterrupt &requestFocus)
{
    AUDIO_DEBUG_LOG("on callback Entered OnAudioFocusRequested %{public}s", __func__);

    std::vector<std::shared_ptr<AudioFocusInfoChangeCallback>> temp_;
    std::unique_lock<std::mutex> cbListLock(cbListMutex_);
    for (auto callback = callbackList_.begin(); callback != callbackList_.end(); ++callback) {
        cb_ = (*callback).lock();
        if (cb_ != nullptr) {
            AUDIO_DEBUG_LOG("OnAudioFocusRequested : Notify event to app complete");
            temp_.push_back(cb_);
        } else {
            AUDIO_ERR_LOG("OnAudioFocusRequested: callback is null");
        }
    }
    cbListLock.unlock();
    for (uint32_t i = 0; i < temp_.size(); i++) {
        temp_[i]->OnAudioFocusRequested(requestFocus);
    }
    return;
}

void AudioFocusInfoChangeCallbackImpl::OnAudioFocusAbandoned(const AudioInterrupt &abandonFocus)
{
    AUDIO_DEBUG_LOG("on callback Entered OnAudioFocusAbandoned %{public}s", __func__);
    std::vector<std::shared_ptr<AudioFocusInfoChangeCallback>> temp_;
    std::unique_lock<std::mutex> cbListLock(cbListMutex_);
    for (auto callback = callbackList_.begin(); callback != callbackList_.end(); ++callback) {
        cb_ = (*callback).lock();
        if (cb_ != nullptr) {
            AUDIO_DEBUG_LOG("OnAudioFocusAbandoned : Notify event to app complete");
            temp_.push_back(cb_);
        } else {
            AUDIO_ERR_LOG("OnAudioFocusAbandoned: callback is null");
        }
    }
    cbListLock.unlock();
    for (uint32_t i = 0; i < temp_.size(); i++) {
        temp_[i]->OnAudioFocusAbandoned(abandonFocus);
    }
    return;
}
}
}