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
#define LOG_TAG "AudioRouterInfra"
#endif

#include "audio_router_infra.h"
#include "audio_policy_log.h"
#include "audio_info.h"
#include <chrono>
#include <algorithm>

namespace OHOS {
namespace AudioStandard {

void AudioRouterInfra::RefreshSelectDevice(SelectDeviceType type, int32_t uid,
    const std::shared_ptr<AudioDeviceSimpleDescriptor>& device)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    bool isSystem = IsSystemUid(uid);

    if (isSystem) {
        RefreshSystemSelectDevice(type, device);
    } else {
        RefreshAppSelectDevice(type, uid, device);
    }
}

void AudioRouterInfra::RefreshSystemSelectDevice(SelectDeviceType type,
    const std::shared_ptr<AudioDeviceSimpleDescriptor>& device)
{
    int64_t currentTime = GetCurrentTimeMs();

    switch (type) {
        case SelectDeviceType::MEDIA_INPUT:
            systemSelectDevice_.mediaInput_.device_ = device;
            systemSelectDevice_.mediaInput_.selectTime_ = currentTime;
            break;
        case SelectDeviceType::MEDIA_OUTPUT:
            systemSelectDevice_.mediaOutput_.device_ = device;
            systemSelectDevice_.mediaOutput_.selectTime_ = currentTime;
            break;
        case SelectDeviceType::CALL_INPUT:
            systemSelectDevice_.callInput_.device_ = device;
            systemSelectDevice_.callInput_.selectTime_ = currentTime;
            break;
        case SelectDeviceType::CALL_OUTPUT:
            systemSelectDevice_.callOutput_.device_ = device;
            systemSelectDevice_.callOutput_.selectTime_ = currentTime;
            break;
        default:
            AUDIO_ERR_LOG("Invalid select device type: %{public}d", type);
            break;
    }
}

void AudioRouterInfra::RefreshAppSelectDevice(SelectDeviceType type, int32_t uid, 
    const std::shared_ptr<AudioDeviceSimpleDescriptor>& device)
{
    int64_t currentTime = GetCurrentTimeMs();

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        it = appContexts_.emplace(uid, std::make_shared<AudioAppRouterContext>(0, uid, true)).first;
    }

    auto& appContext = it->second;
    switch (type) {
        case SelectDeviceType::MEDIA_INPUT:
            appContext->appSelectDevice_.mediaInput_.device_ = device;
            appContext->appSelectDevice_.mediaInput_.selectTime_ = currentTime;
            AdjustInputPriority(uid, HasRunningInputStream(uid), appContext->isForeground_, true);
            break;
        case SelectDeviceType::MEDIA_OUTPUT:
            appContext->appSelectDevice_.mediaOutput_.device_ = device;
            appContext->appSelectDevice_.mediaOutput_.selectTime_ = currentTime;
            AdjustOutputPriority(uid, HasRunningOutputStream(uid), appContext->isForeground_, true);
            break;
        case SelectDeviceType::CALL_INPUT:
            appContext->appSelectDevice_.callInput_.device_ = device;
            appContext->appSelectDevice_.callInput_.selectTime_ = currentTime;
            break;
        case SelectDeviceType::CALL_OUTPUT:
            appContext->appSelectDevice_.callOutput_.device_ = device;
            appContext->appSelectDevice_.callOutput_.selectTime_ = currentTime;
            break;
        default:
            AUDIO_ERR_LOG("Invalid select device type: %{public}d", type);
            break;
    }
}

void AudioRouterInfra::ClearAllSelectSelectDevice(SelectDeviceType type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    ClearSystemSelectDevice(type);
    ClearAllAppSelectDevice(type);
}

void AudioRouterInfra::ClearSystemSelectDevice(SelectDeviceType type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    switch (type) {
        case SelectDeviceType::MEDIA_INPUT:
            systemSelectDevice_.mediaInput_.device_ = nullptr;
            systemSelectDevice_.mediaInput_.selectTime_ = 0;
            break;
        case SelectDeviceType::MEDIA_OUTPUT:
            systemSelectDevice_.mediaOutput_.device_ = nullptr;
            systemSelectDevice_.mediaOutput_.selectTime_ = 0;
            break;
        case SelectDeviceType::CALL_INPUT:
            systemSelectDevice_.callInput_.device_ = nullptr;
            systemSelectDevice_.callInput_.selectTime_ = 0;
            break;
        case SelectDeviceType::CALL_OUTPUT:
            systemSelectDevice_.callOutput_.device_ = nullptr;
            systemSelectDevice_.callOutput_.selectTime_ = 0;
            break;
        default:
            AUDIO_ERR_LOG("Invalid select device type: %{public}d", type);
            break;
    }
}

void AudioRouterInfra::ClearAllAppSelectDevice(SelectDeviceType type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    for (auto& [uid, context] : appContexts_) {
        switch (type) {
            case SelectDeviceType::MEDIA_INPUT:
                context->appSelectDevice_.mediaInput_.device_ = nullptr;
                context->appSelectDevice_.mediaInput_.selectTime_ = 0;
                break;
            case SelectDeviceType::MEDIA_OUTPUT:
                context->appSelectDevice_.mediaOutput_.device_ = nullptr;
                context->appSelectDevice_.mediaOutput_.selectTime_ = 0;
                break;
            case SelectDeviceType::CALL_INPUT:
                context->appSelectDevice_.callInput_.device_ = nullptr;
                context->appSelectDevice_.callInput_.selectTime_ = 0;
                break;
            case SelectDeviceType::CALL_OUTPUT:
                context->appSelectDevice_.callOutput_.device_ = nullptr;
                context->appSelectDevice_.callOutput_.selectTime_ = 0;
                break;
            default:
                AUDIO_ERR_LOG("Invalid select device type: %{public}d", type);
                break;
        }
    }
}

void AudioRouterInfra::SetRecognitionCaptureDevice(
    const std::shared_ptr<AudioDeviceSimpleDescriptor>& device)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    recognitionSelectDevice_.device_ = device;
    recognitionSelectDevice_.selectTime_ = GetCurrentTimeMs();
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterInfra::GetRecognitionCaptureDevice()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return recognitionSelectDevice_.device_;
}

AudioSelectDeviceInfo AudioRouterInfra::GetSystemSelectDevice(SelectDeviceType type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    switch (type) {
        case SelectDeviceType::MEDIA_INPUT:
            return systemSelectDevice_.mediaInput_;
        case SelectDeviceType::MEDIA_OUTPUT:
            return systemSelectDevice_.mediaOutput_;
        case SelectDeviceType::CALL_INPUT:
            return systemSelectDevice_.callInput_;
        case SelectDeviceType::CALL_OUTPUT:
            return systemSelectDevice_.callOutput_;
        default:
            AUDIO_ERR_LOG("Invalid select device type: %{public}d", type);
            return AudioSelectDeviceInfo();
    }
}

AudioSelectDeviceInfo AudioRouterInfra::GetAppSelectDevice(SelectDeviceType type, int32_t uid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return AudioSelectDeviceInfo();
    }

    auto& appContext = it->second;
    switch (type) {
        case SelectDeviceType::MEDIA_INPUT:
            return appContext->appSelectDevice_.mediaInput_;
        case SelectDeviceType::MEDIA_OUTPUT:
            return appContext->appSelectDevice_.mediaOutput_;
        case SelectDeviceType::CALL_INPUT:
            return appContext->appSelectDevice_.callInput_;
        case SelectDeviceType::CALL_OUTPUT:
            return appContext->appSelectDevice_.callOutput_;
        default:
            AUDIO_ERR_LOG("Invalid select device type: %{public}d", type);
            return AudioSelectDeviceInfo();
    }
}

AudioSelectDeviceInfo AudioRouterInfra::GetHighestPrioritySelectDevice(SelectDeviceType type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (type == SelectDeviceType::MEDIA_INPUT || type == SelectDeviceType::CALL_INPUT) {
        if (inputPriorityList_.empty()) {
            return AudioSelectDeviceInfo();
        }
        int32_t highestUid = inputPriorityList_.front();
        return GetAppSelectDevice(type, highestUid);
    } else {
        if (outputPriorityList_.empty()) {
            return AudioSelectDeviceInfo();
        }
        int32_t highestUid = outputPriorityList_.front();
        return GetAppSelectDevice(type, highestUid);
    }
}

void AudioRouterInfra::UpdatePreferredInputCategory(int32_t uid, 
    BluetoothAndNearlinkPreferredRecordCategory category)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        it = appContexts_.emplace(uid, std::make_shared<AudioAppRouterContext>(0, uid, true)).first;
    }

    it->second->appSelectDevice_.preferredInputCategory_ = category;
}

BluetoothAndNearlinkPreferredRecordCategory AudioRouterInfra::GetPreferredInputCategory(int32_t uid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE;
    }

    return it->second->appSelectDevice_.preferredInputCategory_;
}

BluetoothAndNearlinkPreferredRecordCategory AudioRouterInfra::GetHighestPriorityPreferredInputCategory()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (inputPriorityList_.empty()) {
        return BluetoothAndNearlinkPreferredRecordCategory::PREFERRED_NONE;
    }

    int32_t highestUid = inputPriorityList_.front();
    return GetPreferredInputCategory(highestUid);
}

void AudioRouterInfra::UpdateCurrentOutputDevice(int32_t uid,
    const std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>>& devices)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return;
    }

    auto& appContext = it->second;
    appContext->currentOutputDevices_ = devices;
}

std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> AudioRouterInfra::GetCurrentOutputDevice(int32_t uid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return {};
    }

    auto& appContext = it->second;
    return appContext->currentOutputDevices_;
}

void AudioRouterInfra::UpdateCurrentInputDevice(int32_t uid,
    const std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>>& devices)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return;
    }

    auto& appContext = it->second;
    appContext->currentInputDevices_ = devices;
}

std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> AudioRouterInfra::GetCurrentInputDevice(int32_t uid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return {};
    }

    auto& appContext = it->second;
    return appContext->currentInputDevices_;
}

void AudioRouterInfra::UpdateInputStreamState(int32_t uid, uint32_t streamId, 
    SourceType sourceType, StreamState state)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        it = appContexts_.emplace(uid, std::make_shared<AudioAppRouterContext>(0, uid, true)).first;
    }

    auto& appContext = it->second;
    if (state == StreamState::STARTED) {
        appContext->inputStreams_[streamId] = sourceType;
    } else if (state == StreamState::STOPPED || state == StreamState::PAUSED) {
        appContext->inputStreams_.erase(streamId);
    }

    bool hasRunningStream = HasRunningInputStream(uid);
    AdjustInputPriority(uid, hasRunningStream, appContext->isForeground_, false);
}

void AudioRouterInfra::UpdateOutputStreamState(int32_t uid, uint32_t streamId, 
    StreamUsage streamUsage, StreamState state)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        it = appContexts_.emplace(uid, std::make_shared<AudioAppRouterContext>(0, uid, true)).first;
    }

    auto& appContext = it->second;
    if (state == StreamState::STARTED) {
        appContext->outputStreams_[streamId] = streamUsage;
    } else if (state == StreamState::STOPPED || state == StreamState::PAUSED) {
        appContext->outputStreams_.erase(streamId);
    }

    bool hasRunningStream = HasRunningOutputStream(uid);
    AdjustOutputPriority(uid, hasRunningStream, appContext->isForeground_, false);
}

bool AudioRouterInfra::HasRunningSourceType(const std::vector<SourceType>& sourceTypes)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    for (const auto& [uid, context] : appContexts_) {
        for (const auto& [streamId, sourceType] : context->inputStreams_) {
            if (std::find(sourceTypes.begin(), sourceTypes.end(), sourceType) != sourceTypes.end()) {
                return true;
            }
        }
    }
    return false;
}

bool AudioRouterInfra::HasHighestPriorityRunningSourceType(const std::vector<SourceType>& sourceTypes)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (inputPriorityList_.empty()) {
        return false;
    }

    int32_t highestUid = inputPriorityList_.front();
    auto it = appContexts_.find(highestUid);
    if (it == appContexts_.end()) {
        return false;
    }

    for (const auto& [streamId, sourceType] : it->second->inputStreams_) {
        if (std::find(sourceTypes.begin(), sourceTypes.end(), sourceType) != sourceTypes.end()) {
            return true;
        }
    }
    return false;
}

bool AudioRouterInfra::HasRunningStreamUsage(const std::vector<StreamUsage>& streamUsages)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    for (const auto& [uid, context] : appContexts_) {
        for (const auto& [streamId, streamUsage] : context->outputStreams_) {
            if (std::find(streamUsages.begin(), streamUsages.end(), streamUsage) != streamUsages.end()) {
                return true;
            }
        }
    }
    return false;
}

void AudioRouterInfra::UpdateAppForegroundState(int32_t uid, bool isForeground)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        it = appContexts_.emplace(uid, std::make_shared<AudioAppRouterContext>(0, uid, isForeground)).first;
        return;
    }

    it->second->isForeground_ = isForeground;
}

void AudioRouterInfra::Lock()
{
    mutex_.lock();
}

void AudioRouterInfra::Unlock()
{
    mutex_.unlock();
}

std::vector<SourceType> AudioRouterInfra::GetHighestPriorityRunningSourceTypes()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (inputPriorityList_.empty()) {
        return {};
    }

    int32_t highestUid = inputPriorityList_.front();
    auto it = appContexts_.find(highestUid);
    if (it == appContexts_.end()) {
        return {};
    }

    std::vector<SourceType> sourceTypes;
    for (const auto& [streamId, sourceType] : it->second->inputStreams_) {
        sourceTypes.push_back(sourceType);
    }
    return sourceTypes;
}

std::vector<StreamUsage> AudioRouterInfra::GetHighestPriorityRunningStreamUsages()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (outputPriorityList_.empty()) {
        return {};
    }

    int32_t highestUid = outputPriorityList_.front();
    auto it = appContexts_.find(highestUid);
    if (it == appContexts_.end()) {
        return {};
    }

    std::vector<StreamUsage> streamUsages;
    for (const auto& [streamId, streamUsage] : it->second->outputStreams_) {
        streamUsages.push_back(streamUsage);
    }
    return streamUsages;
}

SourceType AudioRouterInfra::GetStreamSourceType(int32_t uid, uint32_t streamId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return SourceType::SOURCE_TYPE_INVALID;
    }

    auto streamIt = it->second->inputStreams_.find(streamId);
    if (streamIt == it->second->inputStreams_.end()) {
        return SourceType::SOURCE_TYPE_INVALID;
    }

    return streamIt->second;
}

StreamUsage AudioRouterInfra::GetStreamStreamUsage(int32_t uid, uint32_t streamId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return (StreamUsage::STREAM_USAGE_INVALID);
    }

    auto streamIt = it->second->outputStreams_.find(streamId);
    if (streamIt == it->second->outputStreams_.end()) {
        return (StreamUsage::STREAM_USAGE_INVALID);
    }

    return streamIt->second;
}

void AudioRouterInfra::OnAppDied(int32_t uid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    appContexts_.erase(uid);
    inputPriorityList_.remove(uid);
    outputPriorityList_.remove(uid);
}

void AudioRouterInfra::AdjustInputPriority(int32_t uid, bool hasRunningStream, 
    bool isForeground, bool isSelfSelecting)
{
    if (isSelfSelecting) {
        if (isForeground && hasRunningStream) {
            inputPriorityList_.remove(uid);
            inputPriorityList_.push_front(uid);
        }
    } else {
        if (hasRunningStream) {
            inputPriorityList_.remove(uid);
            inputPriorityList_.push_front(uid);
        } else {
            inputPriorityList_.remove(uid);
            inputPriorityList_.push_back(uid);
        }
    }
}

void AudioRouterInfra::AdjustOutputPriority(int32_t uid, bool hasRunningStream, 
    bool isForeground, bool isSelfSelecting)
{
    if (isSelfSelecting) {
        if (isForeground && hasRunningStream) {
            outputPriorityList_.remove(uid);
            outputPriorityList_.push_front(uid);
        }
    } else {
        if (hasRunningStream) {
            outputPriorityList_.remove(uid);
            outputPriorityList_.push_front(uid);
        } else {
            outputPriorityList_.remove(uid);
            outputPriorityList_.push_back(uid);
        }
    }
}

bool AudioRouterInfra::HasRunningInputStream(int32_t uid)
{
    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return false;
    }
    return !it->second->inputStreams_.empty();
}

bool AudioRouterInfra::HasRunningOutputStream(int32_t uid)
{
    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return false;
    }
    return !it->second->outputStreams_.empty();
}


SourceType AudioRouterInfra::GetFirstAppRunningSourceType(int32_t uid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return SourceType::SOURCE_TYPE_INVALID;
    }

    if (it->second->inputStreams_.empty()) {
        return SourceType::SOURCE_TYPE_INVALID;
    }

    return it->second->inputStreams_.begin()->second;
}

void AudioRouterInfra::UpdateStreamSelectDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceSimpleDescriptor>& device)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        it = appContexts_.emplace(uid, std::make_shared<AudioAppRouterContext>(0, uid, true)).first;
    }

    auto& appContext = it->second;
    appContext->streamSelectDevices_[streamId] = device;
}

void AudioRouterInfra::UpdateStreamDefaultDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceSimpleDescriptor>& device)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        it = appContexts_.emplace(uid, std::make_shared<AudioAppRouterContext>(0, uid, true)).first;
    }

    auto& appContext = it->second;
    appContext->streamDefaultDevices_[streamId] = device;
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterInfra::GetStreamSelectDevice(int32_t uid,
    uint32_t streamId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return nullptr;
    }

    auto& appContext = it->second;
    auto streamIt = appContext->streamSelectDevices_.find(streamId);
    if (streamIt == appContext->streamSelectDevices_.end()) {
        return nullptr;
    }
    return streamIt->second;
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterInfra::GetStreamDefaultDevice(int32_t uid,
    uint32_t streamId)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return nullptr;
    }

    auto& appContext = it->second;
    auto streamIt = appContext->streamDefaultDevices_.find(streamId);
    if (streamIt == appContext->streamDefaultDevices_.end()) {
        return nullptr;
    }
    return streamIt->second;
}

int64_t AudioRouterInfra::GetCurrentTimeMs() const
{
    constexpr int32_t MS_PER_S = 1000;
    constexpr int32_t NS_PER_MS = 1000000;

    timespec tm {};
    clock_gettime(CLOCK_MONOTONIC, &tm);
    return tm.tv_sec * MS_PER_S + (tm.tv_nsec / NS_PER_MS);
}

bool AudioRouterInfra::IsSystemUid(int32_t uid) const
{
    return uid == SYSTEM_PID;
}

} 
} 