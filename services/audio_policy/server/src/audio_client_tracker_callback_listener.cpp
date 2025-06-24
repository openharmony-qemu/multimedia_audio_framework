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
#define LOG_TAG "AudioClientTrackerCallbackListener"
#endif

#include "audio_policy_log.h"
#include "audio_client_tracker_callback_listener.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

ClientTrackerCallbackListener::ClientTrackerCallbackListener(const sptr<IStandardClientTracker> &listener)
    : listener_(listener)
{
}

ClientTrackerCallbackListener::~ClientTrackerCallbackListener()
{
}

void ClientTrackerCallbackListener::MuteStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    if (listener_ != nullptr) {
        listener_->MuteStreamImpl(streamSetStateEventInternal);
    }
}

void ClientTrackerCallbackListener::UnmuteStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    if (listener_ != nullptr) {
        listener_->UnmuteStreamImpl(streamSetStateEventInternal);
    }
}

void ClientTrackerCallbackListener::PausedStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    if (listener_ != nullptr) {
        listener_->PausedStreamImpl(streamSetStateEventInternal);
    }
}

void ClientTrackerCallbackListener::ResumeStreamImpl(
    const StreamSetStateEventInternal &streamSetStateEventInternal)
{
    if (listener_ != nullptr) {
        listener_->ResumeStreamImpl(streamSetStateEventInternal);
    }
}

void ClientTrackerCallbackListener::SetLowPowerVolumeImpl(float volume)
{
    if (listener_ != nullptr) {
        listener_->SetLowPowerVolumeImpl(volume);
    }
}

void ClientTrackerCallbackListener::GetLowPowerVolumeImpl(float &volume)
{
    if (listener_ != nullptr) {
        listener_->GetLowPowerVolumeImpl(volume);
    }
}

void ClientTrackerCallbackListener::GetSingleStreamVolumeImpl(float &volume)
{
    if (listener_ != nullptr) {
        listener_->GetSingleStreamVolumeImpl(volume);
    }
}

void ClientTrackerCallbackListener::SetOffloadModeImpl(int32_t state, bool isAppBack)
{
    if (listener_ != nullptr) {
        listener_->SetOffloadModeImpl(state, isAppBack);
    }
}

void ClientTrackerCallbackListener::UnsetOffloadModeImpl()
{
    if (listener_ != nullptr) {
        listener_->UnsetOffloadModeImpl();
    }
}

} // namespace AudioStandard
} // namespace OHOS