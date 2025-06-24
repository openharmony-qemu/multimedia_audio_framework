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
#define LOG_TAG "AudioRoutingManagerListener"
#endif

#include "audio_routing_manager_listener.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {

AudioRoutingManagerListener::AudioRoutingManagerListener()
{
}

AudioRoutingManagerListener::~AudioRoutingManagerListener()
{
}

int32_t AudioRoutingManagerListener::OnDistributedRoutingRoleChange(
    const std::shared_ptr<AudioDeviceDescriptor> &descriptor, int32_t type)
{
    std::shared_ptr<AudioDistributedRoutingRoleCallback> audioDistributedRoutingRoleCallback =
        audioDistributedRoutingRoleCallback_.lock();

    CHECK_AND_RETURN_RET_LOG(audioDistributedRoutingRoleCallback != nullptr,
        ERR_CALLBACK_NOT_REGISTERED,
        "OnDistributedRoutingRoleChange: audioDistributedRoutingRoleCallback_ is nullptr");

    audioDistributedRoutingRoleCallback->OnDistributedRoutingRoleChange(descriptor, static_cast<CastType>(type));
    return SUCCESS;
}

void AudioRoutingManagerListener::SetDistributedRoutingRoleCallback(
    const std::weak_ptr<AudioDistributedRoutingRoleCallback> &callback)
{
    audioDistributedRoutingRoleCallback_ = callback;
}

void AudioRoutingManagerListener::SetAudioDeviceRefinerCallback(const std::weak_ptr<AudioDeviceRefiner> &callback)
{
    std::lock_guard<std::mutex> lock(deviceRefinerCallbackMutex_);
    audioDeviceRefinerCallback_ = callback;
    std::shared_ptr<AudioDeviceRefiner> audioDeviceRefinerCallback = audioDeviceRefinerCallback_.lock();
    CHECK_AND_RETURN_LOG(audioDeviceRefinerCallback != nullptr, "audioDeviceRefinerCallback_ is nullptr");
}

int32_t AudioRoutingManagerListener::OnAudioOutputDeviceRefined(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs, int32_t routerType, int32_t streamUsage,
    int32_t clientUid, int32_t audioPipeType)
{
    std::unique_lock<std::mutex> lock(deviceRefinerCallbackMutex_);
    std::shared_ptr<AudioDeviceRefiner> audioDeviceRefinerCallback = audioDeviceRefinerCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(audioDeviceRefinerCallback != nullptr,
        ERR_CALLBACK_NOT_REGISTERED, "audioDeviceRefinerCallback_ is nullptr");
    lock.unlock();

    return audioDeviceRefinerCallback->OnAudioOutputDeviceRefined(descs,
        static_cast<RouterType>(routerType), static_cast<StreamUsage>(streamUsage), clientUid,
        static_cast<AudioPipeType>(audioPipeType));
}

int32_t AudioRoutingManagerListener::OnDistributedOutputChange(bool isRemote)
{
    std::unique_lock<std::mutex> lock(deviceRefinerCallbackMutex_);
    std::shared_ptr<AudioDeviceRefiner> audioDeviceRefinerCallback = audioDeviceRefinerCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(audioDeviceRefinerCallback != nullptr,
        ERR_CALLBACK_NOT_REGISTERED, "audioDeviceRefinerCallback_ is nullptr");
    lock.unlock();

    return audioDeviceRefinerCallback->OnDistributedOutputChange(isRemote);
}

int32_t AudioRoutingManagerListener::OnAudioInputDeviceRefined(
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs, int32_t routerType, int32_t sourceType,
    int32_t clientUid, int32_t audioPipeType)
{
    std::unique_lock<std::mutex> lock(deviceRefinerCallbackMutex_);
    std::shared_ptr<AudioDeviceRefiner> audioDeviceRefinerCallback = audioDeviceRefinerCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(audioDeviceRefinerCallback != nullptr, ERR_CALLBACK_NOT_REGISTERED,
        "audioDeviceRefinerCallback_ is nullptr");
    lock.unlock();

    return audioDeviceRefinerCallback->OnAudioInputDeviceRefined(descs, static_cast<RouterType>(routerType),
        static_cast<SourceType>(sourceType), clientUid, static_cast<AudioPipeType>(audioPipeType));
}

int32_t AudioRoutingManagerListener::GetSplitInfoRefined(std::string &splitInfo)
{
    std::unique_lock lock(deviceRefinerCallbackMutex_);
    std::shared_ptr<AudioDeviceRefiner> audioDeviceRefinerCallback = audioDeviceRefinerCallback_.lock();
    CHECK_AND_RETURN_RET_LOG(audioDeviceRefinerCallback != nullptr,
        ERR_CALLBACK_NOT_REGISTERED, "audioDeviceRefinerCallback_ is nullptr");
    lock.unlock();

    return audioDeviceRefinerCallback->GetSplitInfoRefined(splitInfo);
}
} // namespace AudioStandard
} // namespace OHOS
