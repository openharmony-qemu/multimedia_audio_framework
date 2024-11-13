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
#define LOG_TAG "AudioPolicyUtils"
#endif

#include "audio_policy_utils.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_utils.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "audio_manager_listener_stub.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "data_share_observer_callback.h"
#include "audio_policy_manager_factory.h"
#include "device_init_callback.h"

namespace OHOS {
namespace AudioStandard {

constexpr int32_t NS_PER_MS = 1000000;
constexpr int32_t MS_PER_S = 1000;

static std::string GetEncryptAddr(const std::string &addr)
{
    const int32_t START_POS = 6;
    const int32_t END_POS = 13;
    const int32_t ADDRESS_STR_LEN = 17;
    if (addr.empty() || addr.length() != ADDRESS_STR_LEN) {
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    for (int i = START_POS; i <= END_POS; i++) {
        out[i] = tmp[i];
    }
    return out;
}

std::map<std::string, AudioSampleFormat> AudioPolicyUtils::formatStrToEnum = {
    {"s8", SAMPLE_U8},
    {"s16", SAMPLE_S16LE},
    {"s24", SAMPLE_S24LE},
    {"s32", SAMPLE_S32LE},
};

void AudioPolicyUtils::WriteServiceStartupError(std::string reason)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::AUDIO_SERVICE_STARTUP_ERROR,
        Media::MediaMonitor::EventType::FAULT_EVENT);
    bean->Add("SERVICE_ID", static_cast<int32_t>(Media::MediaMonitor::AUDIO_POLICY_SERVICE_ID));
    bean->Add("ERROR_CODE", static_cast<int32_t>(Media::MediaMonitor::AUDIO_POLICY_SERVER));
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

std::string AudioPolicyUtils::GetRemoteModuleName(std::string networkId, DeviceRole role)
{
    return networkId + (role == DeviceRole::OUTPUT_DEVICE ? "_out" : "_in");
}

std::vector<std::unique_ptr<AudioDeviceDescriptor>> AudioPolicyUtils::GetAvailableDevicesInner(AudioDeviceUsage usage)
{
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;

    audioDeviceDescriptors = audioDeviceManager_.GetAvailableDevicesByUsage(usage);
    return audioDeviceDescriptors;
}

int32_t AudioPolicyUtils::SetPreferredDevice(const PreferredType preferredType,
    const sptr<AudioDeviceDescriptor> &desc)
{
    int32_t ret = SUCCESS;
    switch (preferredType) {
        case AUDIO_MEDIA_RENDER:
            audioStateManager_.SetPreferredMediaRenderDevice(desc);
            break;
        case AUDIO_CALL_RENDER:
            audioStateManager_.SetPreferredCallRenderDevice(desc);
            break;
        case AUDIO_CALL_CAPTURE:
            audioStateManager_.SetPreferredCallCaptureDevice(desc);
            break;
        case AUDIO_RECORD_CAPTURE:
            audioStateManager_.SetPreferredRecordCaptureDevice(desc);
            break;
        case AUDIO_RING_RENDER:
        case AUDIO_TONE_RENDER:
            AUDIO_WARNING_LOG("preferredType:%{public}d, not supported", preferredType);
            ret = ERR_INVALID_PARAM;
            break;
        default:
            AUDIO_ERR_LOG("invalid preferredType: %{public}d", preferredType);
            ret = ERR_INVALID_PARAM;
            break;
    }
    if (desc == nullptr || desc->deviceType_ == DEVICE_TYPE_NONE) {
        ErasePreferredDeviceByType(preferredType);
    }
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Set preferredType %{public}d failed, ret: %{public}d", preferredType, ret);
    }
    return ret;
}

int32_t AudioPolicyUtils::ErasePreferredDeviceByType(const PreferredType preferredType)
{
    if (isBTReconnecting_) {
        return SUCCESS;
    }
    auto type = static_cast<Media::MediaMonitor::PerferredType>(preferredType);
    int32_t ret = Media::MediaMonitor::MediaMonitorManager::GetInstance().ErasePreferredDeviceByType(type);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("Erase preferredType %{public}d failed, ret: %{public}d", preferredType, ret);
        return ERROR;
    }
    return SUCCESS;
}

void AudioPolicyUtils::SetBtConnecting(bool flag)
{
    isBTReconnecting_ = flag;
}

void AudioPolicyUtils::ClearScoDeviceSuspendState(std::string macAddress)
{
    AUDIO_DEBUG_LOG("Clear sco suspend state %{public}s", GetEncryptAddr(macAddress).c_str());
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = audioDeviceManager_.GetDevicesByFilter(
        DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_ROLE_NONE, macAddress, "", SUSPEND_CONNECTED);
    for (auto &desc : descs) {
        desc->connectState_ = DEACTIVE_CONNECTED;
    }
}

int64_t AudioPolicyUtils::GetCurrentTimeMS()
{
    timespec tm {};
    clock_gettime(CLOCK_MONOTONIC, &tm);
    return tm.tv_sec * MS_PER_S + (tm.tv_nsec / NS_PER_MS);
}

uint32_t AudioPolicyUtils::PcmFormatToBytes(AudioSampleFormat format)
{
    // AudioSampleFormat / PCM_8_BIT
    switch (format) {
        case SAMPLE_U8:
            return 1; // 1 byte
        case SAMPLE_S16LE:
            return 2; // 2 byte
        case SAMPLE_S24LE:
            return 3; // 3 byte
        case SAMPLE_S32LE:
            return 4; // 4 byte
        case SAMPLE_F32LE:
            return 4; // 4 byte
        default:
            return 2; // 2 byte
    }
}

std::string AudioPolicyUtils::GetSinkPortName(DeviceType deviceType, AudioPipeType pipeType)
{
    std::string portName = PORT_NONE;
    switch (deviceType) {
        case DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP:
            // BTH tells us that a2dpoffload is OK
            if (audioA2dpOffloadFlag_.GetA2dpOffloadFlag() == A2DP_OFFLOAD) {
                if (pipeType == PIPE_TYPE_OFFLOAD) {
                    portName = OFFLOAD_PRIMARY_SPEAKER;
                } else if (pipeType == PIPE_TYPE_MULTICHANNEL) {
                    portName = MCH_PRIMARY_SPEAKER;
                } else {
                    portName = PRIMARY_SPEAKER;
                }
            } else {
                portName = BLUETOOTH_SPEAKER;
            }
            break;
        case DeviceType::DEVICE_TYPE_EARPIECE:
        case DeviceType::DEVICE_TYPE_SPEAKER:
        case DeviceType::DEVICE_TYPE_WIRED_HEADSET:
        case DeviceType::DEVICE_TYPE_WIRED_HEADPHONES:
        case DeviceType::DEVICE_TYPE_USB_HEADSET:
        case DeviceType::DEVICE_TYPE_BLUETOOTH_SCO:
            if (pipeType == PIPE_TYPE_OFFLOAD) {
                portName = OFFLOAD_PRIMARY_SPEAKER;
            } else if (pipeType == PIPE_TYPE_MULTICHANNEL) {
                portName = MCH_PRIMARY_SPEAKER;
            } else {
                portName = PRIMARY_SPEAKER;
            }
            break;
        case DeviceType::DEVICE_TYPE_USB_ARM_HEADSET:
            portName = USB_SPEAKER;
            break;
        case DeviceType::DEVICE_TYPE_DP:
            portName = DP_SINK;
            break;
        case DeviceType::DEVICE_TYPE_FILE_SINK:
            portName = FILE_SINK;
            break;
        case DeviceType::DEVICE_TYPE_REMOTE_CAST:
            portName = REMOTE_CAST_INNER_CAPTURER_SINK_NAME;
            break;
        default:
            portName = PORT_NONE;
            break;
    }

    return portName;
}

string AudioPolicyUtils::ConvertToHDIAudioFormat(AudioSampleFormat sampleFormat)
{
    switch (sampleFormat) {
        case SAMPLE_U8:
            return "u8";
        case SAMPLE_S16LE:
            return "s16le";
        case SAMPLE_S24LE:
            return "s24le";
        case SAMPLE_S32LE:
            return "s32le";
        default:
            return "";
    }
}

std::string AudioPolicyUtils::GetSinkName(const AudioDeviceDescriptor &desc, int32_t sessionId)
{
    if (desc.networkId_ == LOCAL_NETWORK_ID) {
        AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
        streamCollector_.GetPipeType(sessionId, pipeType);
        return GetSinkPortName(desc.deviceType_, pipeType);
    } else {
        return GetRemoteModuleName(desc.networkId_, desc.deviceRole_);
    }
}

}
}