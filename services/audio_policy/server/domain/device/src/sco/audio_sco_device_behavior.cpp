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
#define LOG_TAG "AudioScoDeviceBehavior"
#endif

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_core_service.h"
#include "audio_device_manager.h"
#include "audio_pipe_manager.h"
#include "audio_policy_utils.h"
#include "audio_scene_manager.h"

#include "audio_sco_device_behavior.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const int32_t REFETCH_DEVICE = 4;
static const std::string EMPTY_ADDRESS = "00:00:00:00:00:00";
static const std::string NULL_ADDRESS = "";
} // namespace
int32_t AudioScoDeviceBehavior::ActivateDevice(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET_LOG(streamDesc != nullptr, ERR_NULL_POINTER, "streamDesc is nullptr");
    CHECK_AND_RETURN_RET_LOG(!streamDesc->newDeviceDescs_.empty(), ERR_INVALID_PARAM, "newDeviceDescs_ is empty");
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = streamDesc->newDeviceDescs_.front();
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");

    int32_t ret = SUCCESS;
    if (streamDesc->audioMode_ == AUDIO_MODE_PLAYBACK) {
        ret = HandleScoOutputDeviceFetched(streamDesc, reason);
    } else {
        ret = BluetoothScoInputFetch(streamDesc);
    }
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("sco [%{public}s] is not connected yet",
            AudioPolicyUtils::GetInstance().GetEncryptAddr(deviceDesc->macAddress_).c_str());
        return REFETCH_DEVICE;
    }
    return ret;
}

int32_t AudioScoDeviceBehavior::HandleScoOutputDeviceFetched(const std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("In");
    Trace trace("AudioScoDeviceBehavior::HandleScoOutputDeviceFetched");
#ifdef BLUETOOTH_ENABLE
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = streamDesc->newDeviceDescs_.front();
    CHECK_AND_RETURN_RET_LOG(deviceDesc != nullptr, ERR_NULL_POINTER, "deviceDesc is nullptr");
    int32_t ret = Bluetooth::AudioHfpManager::SetActiveHfpDevice(deviceDesc->macAddress_);
    if (ret != SUCCESS) {
        auto pipeManager = AudioPipeManager::GetPipeManager();
        CHECK_AND_RETURN_RET_LOG(pipeManager != nullptr, ERR_NULL_POINTER, "pipeManager is nullptr");
        RecoverFetchedDescs(pipeManager->GetAllOutputStreamDescs());
        AUDIO_ERR_LOG("Active hfp device failed, retrigger fetch output device.");
        deviceDesc->exceptionFlag_ = true;
        AudioStreamDeviceChangeReasonExt inreason = AudioStreamDeviceChangeReason::UNKNOWN;
        AudioDeviceManager::GetAudioDeviceManager().UpdateDevicesListInfo(
            std::make_shared<AudioDeviceDescriptor>(*deviceDesc), EXCEPTION_FLAG_UPDATE, inreason);
        return ERROR;
    }
    if (streamDesc->streamStatus_ == STREAM_STATUS_STARTED) {
        Bluetooth::AudioHfpManager::UpdateAudioScene(AudioSceneManager::GetInstance().GetAudioScene(true), true);
    }
#endif
    AUDIO_INFO_LOG("out");
    return SUCCESS;
}

int32_t AudioScoDeviceBehavior::BluetoothScoInputFetch(const std::shared_ptr<AudioStreamDescriptor> &streamDesc)
{
    Trace trace("AudioScoDeviceBehavior::BluetoothScoFetch");
    shared_ptr<AudioDeviceDescriptor> desc = streamDesc->newDeviceDescs_[0];
    int32_t ret = Bluetooth::AudioHfpManager::SetActiveHfpDevice(desc->macAddress_);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Active hfp device failed, retrigger fetch input device");
        desc->exceptionFlag_ = true;
        AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
        AudioDeviceManager::GetAudioDeviceManager().UpdateDevicesListInfo(
            std::make_shared<AudioDeviceDescriptor>(*desc), EXCEPTION_FLAG_UPDATE, reason);
        return ERROR;
    }

    if (streamDesc->streamStatus_ != STREAM_STATUS_STARTED) {
        return SUCCESS;
    }
    auto pipeManager = AudioPipeManager::GetPipeManager();
    CHECK_AND_RETURN_RET_LOG(pipeManager != nullptr, ERR_NULL_POINTER, "pipeManager is nullptr");
    bool hasRunningRecognitionCapturerStream = pipeManager->HasRunningRecognitionCapturerStream();
    if (desc->isVrSupported_ &&
        (Util::IsScoSupportSource(streamDesc->capturerInfo_.sourceType) || hasRunningRecognitionCapturerStream)) {
        ret = ScoInputDeviceFetchedForRecongnition(true, desc->macAddress_, desc->connectState_, desc->isVrSupported_);
    } else {
        ret = Bluetooth::AudioHfpManager::UpdateAudioScene(AudioSceneManager::GetInstance().GetAudioScene(true), true);
    }
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("sco [%{public}s] is not connected yet",
            AudioPolicyUtils::GetInstance().GetEncryptAddr(desc->macAddress_).c_str());
    }
    return SUCCESS;
}

int32_t AudioScoDeviceBehavior::ScoInputDeviceFetchedForRecongnition(bool handleFlag, const std::string &address,
    ConnectState connectState, bool isVrSupported)
{
    HILOG_COMM_INFO("[ScoInputDeviceFetchedForRecongnition]handleflag %{public}d, address %{public}s, "
        "connectState %{public}d", handleFlag, AudioPolicyUtils::GetInstance().GetEncryptAddr(address).c_str(),
        connectState);
    if (handleFlag && (connectState != DEACTIVE_CONNECTED || !isVrSupported)) {
        return SUCCESS;
    }
    return Bluetooth::AudioHfpManager::HandleScoWithRecongnition(handleFlag);
}

bool AudioScoDeviceBehavior::RecoverFetchedDescs(const std::vector<std::shared_ptr<AudioStreamDescriptor>> &streamDescs)
{
    auto pipeManager = AudioPipeManager::GetPipeManager();
    CHECK_AND_RETURN_RET_LOG(pipeManager != nullptr, false, "pipeManager is nullptr");
    for (auto &streamDesc : streamDescs) {
        CHECK_AND_CONTINUE_LOG(streamDesc != nullptr, "Stream desc is nullptr");
        pipeManager->UpdateNewDeviceDesc(streamDesc, streamDesc->oldDeviceDescs_);
    }

    return true;
}

int32_t AudioScoDeviceBehavior::DeactivateDevice()
{
    if (Bluetooth::AudioHfpManager::GetActiveHfpDeviceLocal() != EMPTY_ADDRESS) {
        Bluetooth::AudioHfpManager::SetActiveHfpDevice(NULL_ADDRESS);
    }
    return SUCCESS;
}

int32_t AudioScoDeviceBehavior::DeactivateDevice(const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    return Bluetooth::AudioHfpManager::ClearActiveHfpDevice(deviceDesc->macAddress_);
}

} // namespace AudioStandard
} // namespace OHOS
