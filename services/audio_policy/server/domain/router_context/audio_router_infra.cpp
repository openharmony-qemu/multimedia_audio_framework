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
#define LOG_TAG "AudioRouterInfra"
#endif

#include "audio_router_infra.h"
#include "audio_policy_log.h"
#include "audio_info.h"
#include "ipc_skeleton.h"
#include "media_monitor_manager.h"
#include "audio_active_device.h"
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
            WriteSelectOutputSysEvents(device, type);
            break;
        case SelectDeviceType::CALL_INPUT:
            systemSelectDevice_.callInput_.device_ = device;
            systemSelectDevice_.callInput_.selectTime_ = currentTime;
            break;
        case SelectDeviceType::CALL_OUTPUT:
            systemSelectDevice_.callOutput_.device_ = device;
            systemSelectDevice_.callOutput_.selectTime_ = currentTime;
            WriteSelectOutputSysEvents(device, type);
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

    auto &appContext = it->second;
    CHECK_AND_RETURN_LOG(appContext != nullptr, "appContext is null");
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
            WriteSelectOutputSysEvents(device, type);
            break;
        default:
            AUDIO_ERR_LOG("Invalid select device type: %{public}d", type);
            break;
    }
}

void AudioRouterInfra::ClearAllSelectSelectDevice(SelectDeviceType type,
    const std::shared_ptr<AudioDeviceSimpleDescriptor>& device)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    ClearSystemSelectDevice(type, device);
    ClearAllAppSelectDevice(type, device);
    ClearStreamSelectDevice(device);
}

void AudioRouterInfra::ClearSystemSelectDevice(SelectDeviceType type,
    const std::shared_ptr<AudioDeviceSimpleDescriptor>& device)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AUDIO_INFO_LOG("clear system select device for type: %{public}d", type);
    switch (type) {
        case SelectDeviceType::MEDIA_INPUT:
            if (device == nullptr ||(systemSelectDevice_.mediaInput_.device_ != nullptr &&
                *device == *systemSelectDevice_.mediaInput_.device_)) {
                systemSelectDevice_.mediaInput_.device_ = nullptr;
                systemSelectDevice_.mediaInput_.selectTime_ = 0;
            }
            break;
        case SelectDeviceType::MEDIA_OUTPUT:
            if (device == nullptr ||(systemSelectDevice_.mediaOutput_.device_ != nullptr &&
                *device == *systemSelectDevice_.mediaOutput_.device_)) {
                systemSelectDevice_.mediaOutput_.device_ = nullptr;
                systemSelectDevice_.mediaOutput_.selectTime_ = 0;
                WriteSelectOutputSysEvents(device, type);
            }
            break;
        case SelectDeviceType::CALL_INPUT:
            if (device == nullptr ||(systemSelectDevice_.callInput_.device_ != nullptr &&
                *device == *systemSelectDevice_.callInput_.device_)) {
                systemSelectDevice_.callInput_.device_ = nullptr;
                systemSelectDevice_.callInput_.selectTime_ = 0;
            }
            break;
        case SelectDeviceType::CALL_OUTPUT:
            if (device == nullptr ||(systemSelectDevice_.callOutput_.device_ != nullptr &&
                *device == *systemSelectDevice_.callOutput_.device_)) {
                systemSelectDevice_.callOutput_.device_ = nullptr;
                systemSelectDevice_.callOutput_.selectTime_ = 0;
                WriteSelectOutputSysEvents(device, type);
            }
            break;
        default:
            AUDIO_ERR_LOG("Invalid select device type: %{public}d", type);
            break;
    }
}

void AudioRouterInfra::ClearAllAppSelectDevice(SelectDeviceType type,
    const std::shared_ptr<AudioDeviceSimpleDescriptor>& device)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    AUDIO_INFO_LOG("clear app select device for type: %{public}d", type);
    for (auto& [uid, context] : appContexts_) {
        CHECK_AND_CONTINUE_LOG(context != nullptr, "null context found for uid: %{public}d", uid);
        auto &appSelect = context->appSelectDevice_;
        switch (type) {
            case SelectDeviceType::MEDIA_INPUT:
                if (device == nullptr ||(appSelect.mediaInput_.device_ != nullptr &&
                    *device == *appSelect.mediaInput_.device_)) {
                    appSelect.mediaInput_.device_ = nullptr;
                    appSelect.mediaInput_.selectTime_ = 0;
                }
                break;
            case SelectDeviceType::MEDIA_OUTPUT:
                if (device == nullptr ||(appSelect.mediaOutput_.device_ != nullptr &&
                    *device == *appSelect.mediaOutput_.device_)) {
                    appSelect.mediaOutput_.device_ = nullptr;
                    appSelect.mediaOutput_.selectTime_ = 0;
                }
                break;
            case SelectDeviceType::CALL_INPUT:
                if (device == nullptr ||(appSelect.callInput_.device_ != nullptr &&
                    *device == *appSelect.callInput_.device_)) {
                    appSelect.callInput_.device_ = nullptr;
                    appSelect.callInput_.selectTime_ = 0;
                }
                break;
            case SelectDeviceType::CALL_OUTPUT:
                if (device == nullptr ||(appSelect.callOutput_.device_ != nullptr &&
                    *device == *appSelect.callOutput_.device_)) {
                    appSelect.callOutput_.device_ = nullptr;
                    appSelect.callOutput_.selectTime_ = 0;
                    WriteSelectOutputSysEvents(device, type);
                }
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

    auto &appContext = it->second;
    CHECK_AND_RETURN_RET_LOG(appContext != nullptr, AudioSelectDeviceInfo(), "appContext is nullptr");
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

AudioSelectDeviceInfo AudioRouterInfra::GetLatestAppSelectDevice(SelectDeviceType type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    AudioSelectDeviceInfo latestSelectInfo;
    for (const auto& [uid, context] : appContexts_) {
        CHECK_AND_CONTINUE_LOG(context != nullptr, "null context found for uid: %{public}d", uid);
        AudioSelectDeviceInfo currentSelectInfo;
        switch (type) {
            case SelectDeviceType::MEDIA_INPUT:
                currentSelectInfo = context->appSelectDevice_.mediaInput_;
                break;
            case SelectDeviceType::MEDIA_OUTPUT:
                currentSelectInfo = context->appSelectDevice_.mediaOutput_;
                break;
            case SelectDeviceType::CALL_INPUT:
                currentSelectInfo = context->appSelectDevice_.callInput_;
                break;
            case SelectDeviceType::CALL_OUTPUT:
                currentSelectInfo = context->appSelectDevice_.callOutput_;
                break;
            default:
                AUDIO_ERR_LOG("Invalid select device type: %{public}d", type);
                return AudioSelectDeviceInfo();
        }

        if (currentSelectInfo.device_ != nullptr &&
            currentSelectInfo.selectTime_ >= latestSelectInfo.selectTime_) {
            latestSelectInfo = currentSelectInfo;
        }
    }
    return latestSelectInfo;
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
        it = appContexts_.emplace(uid, std::make_shared<AudioAppRouterContext>(0, uid, false)).first;
    }

    auto &appContext = it->second;
    appContext->currentOutputDevices_ = devices;
}

std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> AudioRouterInfra::GetCurrentOutputDevice(int32_t uid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return {};
    }

    auto &appContext = it->second;
    return appContext->currentOutputDevices_;
}

void AudioRouterInfra::UpdateCurrentInputDevice(int32_t uid,
    const std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>>& devices)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        it = appContexts_.emplace(uid, std::make_shared<AudioAppRouterContext>(0, uid, false)).first;
    }

    auto &appContext = it->second;
    appContext->currentInputDevices_ = devices;
}

std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> AudioRouterInfra::GetCurrentInputDevice(int32_t uid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return {};
    }

    auto &appContext = it->second;
    return appContext->currentInputDevices_;
}

set<int32_t> AudioRouterInfra::FindCurrentInputDevice(set<DeviceType> &&types)
{
    set<int32_t> result;
    lock_guard lock(mutex_);
    for (auto& entry : appContexts_) {
        CHECK_AND_CONTINUE(entry.second);
        for (auto& item: entry.second->currentInputDevices_) {
            CHECK_AND_CONTINUE(item && types.contains(item->deviceType_));
            result.insert(item->deviceId_);
        }
    }
    return result;
}

set<int32_t> AudioRouterInfra::FindCurrentOutputDevice(set<DeviceType> &&types)
{
    set<int32_t> result;
    lock_guard lock(mutex_);
    for (auto& entry : appContexts_) {
        CHECK_AND_CONTINUE(entry.second);
        for (auto& item: entry.second->currentOutputDevices_) {
            CHECK_AND_CONTINUE(item && types.contains(item->deviceType_));
            result.insert(item->deviceId_);
        }
    }
    return result;
}

void AudioRouterInfra::UpdateInputStreamState(int32_t uid, uint32_t streamId,
    SourceType sourceType, CapturerState state)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        it = appContexts_.emplace(uid, std::make_shared<AudioAppRouterContext>(0, uid, true)).first;
    }

    auto &appContext = it->second;
    if (state == CapturerState::CAPTURER_RUNNING) {
        appContext->inputStreams_[streamId] = sourceType;
        AdjustInputPriority(uid, true, appContext->isForeground_, false);
    } else if (state == CapturerState::CAPTURER_STOPPED || state == CapturerState::CAPTURER_PAUSED) {
        appContext->inputStreams_.erase(streamId);
    } else if (state == CapturerState::CAPTURER_RELEASED) {
        appContext->inputStreams_.erase(streamId);
        appContext->streamSelectDevices_.erase(streamId);
        appContext->mediaDefaultDevices_.erase(streamId);
        appContext->callDefaultDevices_.erase(streamId);
    }

    if (!HasRunningInputStream(uid)) {
        AdjustInputPriority(uid, false, appContext->isForeground_, false);
    }
}

void AudioRouterInfra::UpdateOutputStreamState(int32_t uid, uint32_t streamId,
    StreamUsage streamUsage, RendererState state, bool updatePriority)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        it = appContexts_.emplace(uid, std::make_shared<AudioAppRouterContext>(0, uid, true)).first;
    }

    auto &appContext = it->second;
    CHECK_AND_RETURN_LOG(appContext != nullptr, "appContext is nullptr");
    if (state == RendererState::RENDERER_RUNNING) {
        appContext->outputStreams_[streamId] = streamUsage;
        if (updatePriority) {
            AdjustOutputPriority(uid, true, appContext->isForeground_, false);
        }
    } else if (state == RendererState::RENDERER_STOPPED || state == RendererState::RENDERER_PAUSED) {
        appContext->outputStreams_.erase(streamId);
    } else if (state == RendererState::RENDERER_RELEASED) {
        appContext->outputStreams_.erase(streamId);
        appContext->streamSelectDevices_.erase(streamId);
        appContext->mediaDefaultDevices_.erase(streamId);
        appContext->callDefaultDevices_.erase(streamId);
    }

    if (!HasRunningOutputStream(uid)) {
        AdjustOutputPriority(uid, false, appContext->isForeground_, false);
    }
}

int32_t AudioRouterInfra::GetHighestInputPriorityApp()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (inputPriorityList_.empty()) {
        return INVALID_UID;
    }
    return inputPriorityList_.front();
}

int32_t AudioRouterInfra::GetHighestOutputPriorityApp()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (outputPriorityList_.empty()) {
        return INVALID_UID;
    }
    return outputPriorityList_.front();
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

void AudioRouterInfra::UpdateAppForegroundState(int32_t uid, bool isForeground)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        it = appContexts_.emplace(uid, std::make_shared<AudioAppRouterContext>(0, uid, isForeground)).first;
        return;
    }
    CHECK_AND_RETURN_LOG(it->second != nullptr, "audioAppRouterContext is nullptr");
    it->second->isForeground_ = isForeground;
    CHECK_AND_RETURN(isForeground);
    AdjustOutputPriority(uid, HasRunningOutputStream(uid), isForeground, false);
}

void AudioRouterInfra::Lock()
{
    mutex_.lock();
}

void AudioRouterInfra::Unlock()
{
    mutex_.unlock();
}

std::vector<SourceType> AudioRouterInfra::GetAppRunningSourceTypes(int32_t uid)
{
    if (uid == INVALID_UID) {
        return {};
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return GetAppRunningSourceTypesNoLock(uid);
}

std::vector<SourceType> AudioRouterInfra::GetAppRunningSourceTypesNoLock(int32_t uid)
{
    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return {};
    }

    std::vector<SourceType> sourceTypes;
    for (const auto& [streamId, sourceType] : it->second->inputStreams_) {
        sourceTypes.push_back(sourceType);
    }
    return sourceTypes;
}

std::vector<StreamUsage> AudioRouterInfra::GetAppRunningStreamUsages(int32_t uid)
{
    if (uid == INVALID_UID) {
        return {};
    }
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto it = appContexts_.find(uid);
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
            CHECK_AND_RETURN(HasValidSelectDevice(GetAppSelectDevice(SelectDeviceType::MEDIA_OUTPUT, uid)));
            outputPriorityList_.remove(uid);
            outputPriorityList_.push_front(uid);
        }
    } else {
        if (isForeground && hasRunningStream) {
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
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return false;
    }
    return !it->second->inputStreams_.empty();
}

bool AudioRouterInfra::HasRunningOutputStream(int32_t uid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return false;
    }
    CHECK_AND_RETURN_RET(it->second != nullptr, false);
    return !it->second->outputStreams_.empty();
}

bool AudioRouterInfra::HasValidSelectDevice(AudioSelectDeviceInfo selectInfo)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return selectInfo.device_ != nullptr && selectInfo.device_->deviceType_ != DEVICE_TYPE_NONE;
}

void AudioRouterInfra::UpdateStreamSelectDevice(int32_t uid, uint32_t streamId,
    const std::shared_ptr<AudioDeviceSimpleDescriptor>& device)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        it = appContexts_.emplace(uid, std::make_shared<AudioAppRouterContext>(0, uid, true)).first;
    }

    auto &appContext = it->second;
    CHECK_AND_RETURN_LOG(appContext != nullptr, "appContext is null");
    appContext->streamSelectDevices_[streamId] = device;
}

void AudioRouterInfra::UpdateStreamDefaultDevice(int32_t uid, uint32_t streamId, SelectDeviceType type,
    const std::shared_ptr<AudioDeviceSimpleDescriptor>& device)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        it = appContexts_.emplace(uid, std::make_shared<AudioAppRouterContext>(0, uid, true)).first;
    }

    auto &appContext = it->second;
    if (type == SelectDeviceType::MEDIA_OUTPUT) {
        appContext->mediaDefaultDevices_[streamId] = device;
    } else if (type == SelectDeviceType::CALL_OUTPUT) {
        appContext->callDefaultDevices_[streamId] = device;
    }
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
    uint32_t streamId, SelectDeviceType type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return nullptr;
    }

    auto& appContext = it->second;
    if (type == SelectDeviceType::MEDIA_OUTPUT) {
        auto streamIt = appContext->mediaDefaultDevices_.find(streamId);
        if (streamIt == appContext->mediaDefaultDevices_.end()) {
            return nullptr;
        }
        return streamIt->second;
    } else if (type == SelectDeviceType::CALL_OUTPUT) {
        auto streamIt = appContext->callDefaultDevices_.find(streamId);
        if (streamIt == appContext->callDefaultDevices_.end()) {
            return nullptr;
        }
        return streamIt->second;
    }
    return nullptr;
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterInfra::GetFirstRunningStreamDefaultDevice(
    int32_t uid, SelectDeviceType type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return nullptr;
    }

    auto& appContext = it->second;
    if (type == SelectDeviceType::MEDIA_OUTPUT) {
        for (const auto& [streamId, device] : appContext->mediaDefaultDevices_) {
            if (appContext->outputStreams_.find(streamId) != appContext->outputStreams_.end()) {
                return device;
            }
        }
    } else if (type == SelectDeviceType::CALL_OUTPUT) {
        for (const auto& [streamId, device] : appContext->callDefaultDevices_) {
            if (appContext->outputStreams_.find(streamId) != appContext->outputStreams_.end()) {
                return device;
            }
        }
    }
    return GetFirstRunningStreamDefaultDeviceNoLock(uid, type);
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterInfra::GetFirstRunningStreamDefaultDeviceNoLock(
    int32_t uid, SelectDeviceType type)
{
    auto it = appContexts_.find(uid);
    if (it == appContexts_.end()) {
        return nullptr;
    }

    auto& appContext = it->second;
    if (type == SelectDeviceType::MEDIA_OUTPUT) {
        for (const auto& [streamId, device] : appContext->mediaDefaultDevices_) {
            if (appContext->outputStreams_.find(streamId) != appContext->outputStreams_.end()) {
                return device;
            }
        }
    } else if (type == SelectDeviceType::CALL_OUTPUT) {
        for (const auto& [streamId, device] : appContext->callDefaultDevices_) {
            if (appContext->outputStreams_.find(streamId) != appContext->outputStreams_.end()) {
                return device;
            }
        }
    }
    return nullptr;
}

std::shared_ptr<AudioDeviceSimpleDescriptor> AudioRouterInfra::GetLatestStreamSelectDevice(SelectDeviceType type)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    for (const auto &uid : outputPriorityList_) {
        auto simpleDesc = GetFirstRunningStreamDefaultDeviceNoLock(uid, type);
        if (simpleDesc != nullptr) {
            return simpleDesc;
        }
    }
    return nullptr;
}

void AudioRouterInfra::ClearStreamSelectDevice(const std::shared_ptr<AudioDeviceSimpleDescriptor> &device)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    if (device == nullptr) {
        for (auto &[uid, context] : appContexts_) {
            CHECK_AND_CONTINUE_LOG(context != nullptr, "null context found for uid: %{public}d", uid);
            context->streamSelectDevices_.clear();
        }
        return;
    }

    for (auto &[uid, context] : appContexts_) {
        CHECK_AND_CONTINUE_LOG(context != nullptr, "null context found for uid: %{public}d", uid);
        auto &streamSelectDevices = context->streamSelectDevices_;
        auto it = streamSelectDevices.begin();
        while (it != streamSelectDevices.end()) {
            if (it->second != nullptr &&
                it->second->deviceId_ == device->deviceId_) {
                it = streamSelectDevices.erase(it);
            } else {
                ++it;
            }
        }
    }
}

int64_t AudioRouterInfra::GetCurrentTimeMs() const
{
    constexpr int32_t msPerS = 1000;
    constexpr int32_t nsPerMs = 1000000;

    timespec tm {};
    clock_gettime(CLOCK_MONOTONIC, &tm);
    return tm.tv_sec * msPerS + (tm.tv_nsec / nsPerMs);
}

bool AudioRouterInfra::IsSystemUid(int32_t uid) const
{
    return uid == SYSTEM_UID;
}

void AudioRouterInfra::ExcludeDevice(const std::shared_ptr<AudioDeviceSimpleDescriptor> &device,
    AudioDeviceUsage usage)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (device == nullptr) {
        return;
    }

    auto it = excludedDevices_.find(device);
    if (it == excludedDevices_.end()) {
        excludedDevices_[device] = usage;
    } else {
        it->second = static_cast<AudioDeviceUsage>(it->second | usage);
    }
    WriteExcludeOutputSysEvents(usage, device, EXCLUDED);
}

void AudioRouterInfra::UnexcludeDevice(const std::shared_ptr<AudioDeviceSimpleDescriptor> &device,
    AudioDeviceUsage usage)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (device == nullptr) {
        return;
    }

    auto it = excludedDevices_.find(device);
    if (it == excludedDevices_.end()) {
        return;
    }

    it->second = static_cast<AudioDeviceUsage>(it->second & ~usage);
    if (it->second == 0) {
        excludedDevices_.erase(it);
        WriteExcludeOutputSysEvents(usage, device, UNEXCLUDED);
    }
}

std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> AudioRouterInfra::GetExcludedDevices(
    AudioDeviceUsage usage)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> result;
    for (auto &[device, devUsage] : excludedDevices_) {
        if ((devUsage & usage) != 0) {
            result.push_back(device);
        }
    }
    return result;
}

void AudioRouterInfra::UnexcludeAllDevice()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    for (auto &[device, usage] : excludedDevices_) {
        WriteExcludeOutputSysEvents(usage, device, UNEXCLUDED);
    }
    excludedDevices_.clear();
}

bool AudioRouterInfra::IsDeviceExcluded(const std::shared_ptr<AudioDeviceSimpleDescriptor> &device,
    AudioDeviceUsage usage)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    if (device == nullptr) {
        return false;
    }

    auto it = excludedDevices_.find(device);
    if (it == excludedDevices_.end()) {
        return false;
    }

    return (it->second & usage) != 0;
}

void AudioRouterInfra::SetScoExcluded(bool scoExcluded)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    isScoExcluded_.store(scoExcluded);
}

bool AudioRouterInfra::GetScoExcluded()
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    return isScoExcluded_.load();
}

void AudioRouterInfra::WriteExcludeOutputSysEvents(const AudioDeviceUsage audioDevUsage,
    const std::shared_ptr<AudioDeviceSimpleDescriptor> &deviceDesc, ExcludeDeviceType excludeType)
{
    int64_t timeStamp = GetCurrentTimeMs();
    auto uid = IPCSkeleton::GetCallingUid();
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::EXCLUDE_OUTPUT_DEVICE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("CLIENT_UID", static_cast<int32_t>(uid));
    bean->Add("TIME_STAMP", static_cast<uint64_t>(timeStamp));
    if (excludeType == EXCLUDED) {
        bean->Add("EXCLUSION_STATUS", EXCLUDED);
    } else {
        bean->Add("EXCLUSION_STATUS", UNEXCLUDED);
    }
    bean->Add("AUDIO_DEVICE_USAGE", static_cast<int32_t>(audioDevUsage));
    bean->Add("DEVICE_TYPE", 0);
    bean->Add("NETWORKID", deviceDesc->networkId_);
    bean->Add("ADDRESS", deviceDesc->macAddress_);
    bean->Add("DEVICE_NAME", "");
    bean->Add("BT_TYPE", 0);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioRouterInfra::WriteSelectOutputSysEvents(
    const std::shared_ptr<AudioDeviceSimpleDescriptor>& device, SelectDeviceType type)
{
    CHECK_AND_RETURN(device != nullptr);
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::SET_FORCE_USE_AUDIO_DEVICE,
        Media::MediaMonitor::BEHAVIOR_EVENT);
    bean->Add("CLIENT_UID", static_cast<int32_t>(IPCSkeleton::GetCallingUid()));
    bean->Add("DEVICE_TYPE", device->deviceType_);
    if (type == SelectDeviceType::MEDIA_OUTPUT) {
        bean->Add("STREAM_TYPE", STREAM_USAGE_MEDIA);
    } else if (type == SelectDeviceType::CALL_OUTPUT) {
        bean->Add("STREAM_TYPE", STREAM_USAGE_VOICE_COMMUNICATION);
    }
    bean->Add("BT_TYPE", 0);
    bean->Add("DEVICE_NAME", "");
    bean->Add("ADDRESS", device->macAddress_);
    bean->Add("IS_PLAYBACK", 1);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

bool AudioRouterInfra::IsPreferredDevice(const std::shared_ptr<AudioDeviceSimpleDescriptor> &desc)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    CHECK_AND_RETURN_RET(desc != nullptr, false);
    std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> preferredDescs = {
        systemSelectDevice_.mediaInput_.device_,
        systemSelectDevice_.mediaOutput_.device_,
        systemSelectDevice_.callInput_.device_,
        systemSelectDevice_.callOutput_.device_,
        recognitionSelectDevice_.device_
    };
    bool isPreferred = std::any_of(preferredDescs.begin(), preferredDescs.end(), [&desc](const auto &preferred) {
        return preferred && preferred->deviceId_ == desc->deviceId_;
    });
    CHECK_AND_RETURN_RET(!isPreferred, isPreferred);
    for (const auto& [uid, context] : appContexts_) {
        CHECK_AND_CONTINUE_LOG(context != nullptr, "null context found for uid: %{public}d", uid);
        std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> appPreferredDescs = {
            context->appSelectDevice_.mediaInput_.device_,
            context->appSelectDevice_.mediaOutput_.device_,
            context->appSelectDevice_.callInput_.device_,
            context->appSelectDevice_.callOutput_.device_
        };
        isPreferred = std::any_of(appPreferredDescs.begin(), appPreferredDescs.end(), [&desc](const auto &preferred) {
            return preferred && preferred->deviceType_ == desc->deviceType_ &&
                preferred->macAddress_ == desc->macAddress_ && preferred->networkId_ == desc->networkId_;
        });
        CHECK_AND_RETURN_RET(!isPreferred, isPreferred);
        for (const auto& [sessionId, streamDevice] : context->streamSelectDevices_) {
            CHECK_AND_CONTINUE_LOG(
                streamDevice != nullptr, "Null streamDevice found for sessionId: %{public}d", sessionId);
            isPreferred = streamDevice->deviceType_ == desc->deviceType_ &&
                streamDevice->macAddress_ == desc->macAddress_ && streamDevice->networkId_ == desc->networkId_;
            CHECK_AND_RETURN_RET(!isPreferred, isPreferred);
        }
    }
    return false;
}

void AudioRouterInfra::ClearWirelessSelectDevice(const std::shared_ptr<AudioDeviceSimpleDescriptor> &device)
{
    std::lock_guard<std::recursive_mutex> lock(mutex_);
    CHECK_AND_RETURN(IsWirelessDevice(device));

    if (IsWirelessDevice(systemSelectDevice_.mediaOutput_.device_)) {
        systemSelectDevice_.mediaOutput_.device_ = nullptr;
        systemSelectDevice_.mediaOutput_.selectTime_ = 0;
    }
    for (auto& [uid, context] : appContexts_) {
        CHECK_AND_CONTINUE_LOG(context != nullptr, "null context found for uid: %{public}d", uid);
        if (IsWirelessDevice(context->appSelectDevice_.mediaOutput_.device_)) {
            context->appSelectDevice_.mediaOutput_.device_ = nullptr;
            context->appSelectDevice_.mediaOutput_.selectTime_ = 0;
        }
        auto &streamSelectDevices = context->streamSelectDevices_;
        auto it = streamSelectDevices.begin();
        while (it != streamSelectDevices.end()) {
            if (IsWirelessDevice(it->second) && it->second->deviceRole_ == OUTPUT_DEVICE) {
                it = streamSelectDevices.erase(it);
            } else {
                ++it;
            }
        }
    }
}

bool AudioRouterInfra::IsWirelessDevice(const std::shared_ptr<AudioDeviceSimpleDescriptor> &device)
{
    CHECK_AND_RETURN_RET(device != nullptr, false);
    DeviceType type = device->deviceType_;
    return type == DEVICE_TYPE_BLUETOOTH_SCO ||
           type == DEVICE_TYPE_BLUETOOTH_A2DP ||
           type == DEVICE_TYPE_BLUETOOTH_A2DP_IN ||
           type == DEVICE_TYPE_NEARLINK ||
           type == DEVICE_TYPE_NEARLINK_IN;
}
}
}