/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "PolicyHandler"
#endif

#include "policy_handler.h"

#include <iomanip>
#include <thread>

#include "audio_errors.h"
#include "audio_common_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const uint32_t FIRST_SESSIONID = 100000;
constexpr uint32_t MAX_VALID_SESSIONID = UINT32_MAX - FIRST_SESSIONID;
}

PolicyHandler& PolicyHandler::GetInstance()
{
    static PolicyHandler PolicyHandler;

    return PolicyHandler;
}

PolicyHandler::PolicyHandler()
{
    AUDIO_INFO_LOG("PolicyHandler()");
}

PolicyHandler::~PolicyHandler()
{
    volumeVector_ = nullptr;
    sharedAbsVolumeScene_ = nullptr;
    policyVolumeMap_ = nullptr;
    iPolicyProvider_ = nullptr;
    AUDIO_INFO_LOG("~PolicyHandler()");
}

void PolicyHandler::Dump(std::string &dumpString)
{
    AUDIO_INFO_LOG("PolicyHandler dump begin");
    if (iPolicyProvider_ == nullptr || policyVolumeMap_ == nullptr || volumeVector_ == nullptr) {
        dumpString += "PolicyHandler is null...\n";
        AUDIO_INFO_LOG("nothing to dump");
        return;
    }
    // dump active output device
    AppendFormat(dumpString, "  - active output device: %d\n", deviceType_);
    // dump volume
    for (size_t i = 0; i < IPolicyProvider::GetVolumeVectorSize(); i++) {
        AppendFormat(dumpString, "  streamtype: %d ", g_volumeIndexVector[i].first);
        AppendFormat(dumpString, "  device: %d ", g_volumeIndexVector[i].second);
        AppendFormat(dumpString, "  isMute: %s ", (volumeVector_[i].isMute ? "true" : "false"));
        AppendFormat(dumpString, "  volFloat: %f ", volumeVector_[i].volumeFloat);
        AppendFormat(dumpString, "  volint: %u \n", volumeVector_[i].volumeInt);
    }
    if (sharedAbsVolumeScene_ == nullptr) {
        dumpString += "sharedAbsVolumeScene_ is null...\n";
        AUDIO_INFO_LOG("sharedAbsVolumeScene_ is null");
        return;
    }
    AppendFormat(dumpString, "  sharedAbsVolumeScene: %s \n", (*sharedAbsVolumeScene_ ? "true" : "false"));
}

bool PolicyHandler::ConfigPolicyProvider(const sptr<IPolicyProviderIpc> policyProvider)
{
    CHECK_AND_RETURN_RET_LOG(policyProvider != nullptr, false, "ConfigPolicyProvider failed with null provider.");
    if (iPolicyProvider_ == nullptr) {
        iPolicyProvider_ = policyProvider;
    } else {
        AUDIO_ERR_LOG("Provider is already configed!");
        return false;
    }
    bool ret = InitVolumeMap();
    AUDIO_INFO_LOG("ConfigPolicyProvider end and InitVolumeMap %{public}s", (ret ? "SUCCESS" : "FAILED"));
    return ret;
}

bool PolicyHandler::GetProcessDeviceInfo(const AudioProcessConfig &config, bool lockFlag,
    AudioDeviceDescriptor &deviceInfo)
{
    // send the config to AudioPolicyServer and get the device info.
    CHECK_AND_RETURN_RET_LOG(iPolicyProvider_ != nullptr, false, "GetProcessDeviceInfo failed with null provider.");
    int32_t ret = iPolicyProvider_->GetProcessDeviceInfo(config, lockFlag, deviceInfo);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, false, "GetProcessDeviceInfo failed:%{public}d", ret);
    return true;
}

bool PolicyHandler::InitVolumeMap()
{
    CHECK_AND_RETURN_RET_LOG(iPolicyProvider_ != nullptr, false, "InitVolumeMap failed with null provider.");
    iPolicyProvider_->InitSharedVolume(policyVolumeMap_);
    CHECK_AND_RETURN_RET_LOG((policyVolumeMap_ != nullptr && policyVolumeMap_->GetBase() != nullptr), false,
        "InitSharedVolume failed.");
    size_t mapSize = IPolicyProvider::GetVolumeVectorSize() * sizeof(Volume) + sizeof(bool);
    CHECK_AND_RETURN_RET_LOG(policyVolumeMap_->GetSize() == mapSize, false,
        "InitSharedVolume get error size:%{public}zu, target:%{public}zu", policyVolumeMap_->GetSize(), mapSize);
    volumeVector_ = reinterpret_cast<Volume *>(policyVolumeMap_->GetBase());
    sharedAbsVolumeScene_ = reinterpret_cast<bool *>(policyVolumeMap_->GetBase()) +
        IPolicyProvider::GetVolumeVectorSize() * sizeof(Volume);
    AUDIO_INFO_LOG("InitSharedVolume success.");
    return true;
}

bool PolicyHandler::GetSharedVolume(AudioVolumeType streamType, DeviceType deviceType, Volume &vol)
{
    CHECK_AND_RETURN_RET_LOG((iPolicyProvider_ != nullptr && volumeVector_ != nullptr), false,
        "GetSharedVolume failed not configed");
    size_t index = 0;
    if (!IPolicyProvider::GetVolumeIndex(streamType, GetVolumeGroupForDevice(deviceType), index) ||
        index >= IPolicyProvider::GetVolumeVectorSize()) {
        return false;
    }
    vol.isMute = volumeVector_[index].isMute;
    vol.volumeFloat = volumeVector_[index].volumeFloat;
    vol.volumeInt = volumeVector_[index].volumeInt;
    return true;
}

void PolicyHandler::SetActiveOutputDevice(DeviceType deviceType)
{
    AUDIO_INFO_LOG("SetActiveOutputDevice to device[%{public}d].", deviceType);
    deviceType_ = deviceType;
}

std::atomic<uint32_t> g_sessionId = {FIRST_SESSIONID}; // begin at 100000

uint32_t PolicyHandler::GenerateSessionId(int32_t uid)
{
    uint32_t sessionId = g_sessionId++;
    AUDIO_INFO_LOG("uid:%{public}d sessionId:%{public}d", uid, sessionId);
    if (g_sessionId > MAX_VALID_SESSIONID) {
        AUDIO_WARNING_LOG("sessionId is too large, reset it!");
        g_sessionId = FIRST_SESSIONID;
    }
    return sessionId;
}

DeviceType PolicyHandler::GetActiveOutPutDevice()
{
    return deviceType_;
}

int32_t PolicyHandler::NotifyCapturerAdded(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
    uint32_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(iPolicyProvider_ != nullptr, ERROR, "iPolicyProvider_ is nullptr");
    return iPolicyProvider_->NotifyCapturerAdded(capturerInfo, streamInfo, sessionId);
}

int32_t PolicyHandler::NotifyWakeUpCapturerRemoved()
{
    CHECK_AND_RETURN_RET_LOG(iPolicyProvider_ != nullptr, ERROR, "iPolicyProvider_ is nullptr");
    return iPolicyProvider_->NotifyWakeUpCapturerRemoved();
}

bool PolicyHandler::IsAbsVolumeSupported()
{
    CHECK_AND_RETURN_RET_LOG((iPolicyProvider_ != nullptr && sharedAbsVolumeScene_ != nullptr), false,
        "abs volume scene failed not configed");

    return *sharedAbsVolumeScene_;
}

int32_t PolicyHandler::OffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize, uint32_t &timeStamp)
{
    CHECK_AND_RETURN_RET_LOG(iPolicyProvider_ != nullptr, ERROR, "iPolicyProvider_ is nullptr");
    return iPolicyProvider_->OffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
}

int32_t PolicyHandler::NearlinkGetRenderPosition(uint32_t &delayValue)
{
    CHECK_AND_RETURN_RET_LOG(iPolicyProvider_ != nullptr, ERROR, "iPolicyProvider_ is nullptr");
    return iPolicyProvider_->NearlinkGetRenderPosition(delayValue);
}

bool PolicyHandler::GetHighResolutionExist()
{
    return isHighResolutionExist_;
}

void PolicyHandler::SetHighResolutionExist(bool isHighResExist)
{
    isHighResolutionExist_ = isHighResExist;
}

int32_t PolicyHandler::GetAndSaveClientType(uint32_t uid, const std::string &bundleName)
{
    CHECK_AND_RETURN_RET_LOG(iPolicyProvider_ != nullptr, ERROR, "iPolicyProvider_ is nullptr");
    return iPolicyProvider_->GetAndSaveClientType(uid, bundleName);
}

int32_t PolicyHandler::GetMaxRendererInstances()
{
    CHECK_AND_RETURN_RET_LOG(iPolicyProvider_ != nullptr, ERROR, "iPolicyProvider_ is nullptr");
    int32_t ret = ERROR;
    iPolicyProvider_->GetMaxRendererInstances(ret);
    return ret;
}

int32_t PolicyHandler::NotifyCapturerRemoved(uint64_t sessionId)
{
    CHECK_AND_RETURN_RET_LOG(iPolicyProvider_ != nullptr, ERROR, "iPolicyProvider_ is nullptr");
    return iPolicyProvider_->NotifyCapturerRemoved(sessionId);
}

#ifdef HAS_FEATURE_INNERCAPTURER
int32_t PolicyHandler::LoadModernInnerCapSink(int32_t innerCapId)
{
    CHECK_AND_RETURN_RET_LOG(iPolicyProvider_ != nullptr, ERROR, "iPolicyProvider_ is nullptr");
    return iPolicyProvider_->LoadModernInnerCapSink(innerCapId);
}

int32_t PolicyHandler::UnloadModernInnerCapSink(int32_t innerCapId)
{
    CHECK_AND_RETURN_RET_LOG(iPolicyProvider_ != nullptr, ERROR, "iPolicyProvider_ is nullptr");
    return iPolicyProvider_->UnloadModernInnerCapSink(innerCapId);
}
#endif

int32_t PolicyHandler::ClearAudioFocusBySessionID(const int32_t &sessionID)
{
    CHECK_AND_RETURN_RET_LOG(iPolicyProvider_ != nullptr, ERROR, "iPolicyProvider_ is nullptr");
    return iPolicyProvider_->ClearAudioFocusBySessionID(sessionID);
}

int32_t PolicyHandler::CaptureConcurrentCheck(const uint32_t &sessionID)
{
    CHECK_AND_RETURN_RET_LOG(iPolicyProvider_ != nullptr, ERROR, "iPolicyProvider_ is nullptr");
    return iPolicyProvider_->CaptureConcurrentCheck(sessionID);
}
} // namespace AudioStandard
} // namespace OHOS
