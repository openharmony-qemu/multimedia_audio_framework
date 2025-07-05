/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioPolicyDump"
#endif

#include "audio_policy_dump.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"

#include "audio_policy_log.h"

#include "iservice_registry.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "audio_converter_parser.h"
#include "audio_bundle_manager.h"

namespace {
const std::string CALLER_NAME = "audio_server";
};

namespace OHOS {
namespace AudioStandard {

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

bool AudioPolicyDump::IsStreamSupported(AudioStreamType streamType)
{
    switch (streamType) {
        case STREAM_MUSIC:
        case STREAM_RING:
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_COMMUNICATION:
        case STREAM_VOICE_ASSISTANT:
        case STREAM_WAKEUP:
        case STREAM_SYSTEM:
        case STREAM_CAMCORDER:
            return true;
        default:
            return false;
    }
}

void AudioPolicyDump::DevicesInfoDump(std::string &dumpString)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors;

    dumpString += "\nInput local Devices:\n";
    audioDeviceDescriptors = GetDumpDeviceInfo(dumpString, INPUT_DEVICES_FLAG);
    AppendFormat(dumpString, "- %zu Input Devices (s) available\n", audioDeviceDescriptors.size());

    dumpString += "\nOutput local Devices:\n";
    audioDeviceDescriptors = GetDumpDeviceInfo(dumpString, OUTPUT_DEVICES_FLAG);
    AppendFormat(dumpString, "- %zu output Devices (s) available\n", audioDeviceDescriptors.size());

    dumpString += "\nInput distributed Devices:\n";
    audioDeviceDescriptors = GetDumpDeviceInfo(dumpString, DISTRIBUTED_INPUT_DEVICES_FLAG);
    AppendFormat(dumpString, "- %zu output Devices (s) available\n", audioDeviceDescriptors.size());

    dumpString += "\nOutput distributed Devices:\n";
    audioDeviceDescriptors = GetDumpDeviceInfo(dumpString, DISTRIBUTED_OUTPUT_DEVICES_FLAG);
    AppendFormat(dumpString, "- %zu output Devices (s) available\n", audioDeviceDescriptors.size());

    priorityOutputDevice_ = audioActiveDevice_.GetCurrentOutputDeviceType();
    priorityInputDevice_ = audioActiveDevice_.GetCurrentInputDeviceType();
    AppendFormat(dumpString, "\nHighest priority output device: %s",
        AudioInfoDumpUtils::GetDeviceTypeName(priorityOutputDevice_).c_str());
    AppendFormat(dumpString, "\nHighest priority input device: %s \n",
        AudioInfoDumpUtils::GetDeviceTypeName(priorityInputDevice_).c_str());

    audioDeviceManager_.Dump(dumpString);
    GetMicrophoneDescriptorsDump(dumpString);
    GetOffloadStatusDump(dumpString);
    AllDeviceVolumeInfoDump(dumpString);
}

void AudioPolicyDump::AllDeviceVolumeInfoDump(std::string &dumpString)
{
    dumpString += "\nVolume Info for all devices:\n";
    std::vector<std::shared_ptr<AllDeviceVolumeInfo>> deviceVolumeInfo =
        audioVolumeManager_.GetAllDeviceVolumeInfo();
    if (deviceVolumeInfo.size() > 0) {
        for (auto it = deviceVolumeInfo.begin(); it != deviceVolumeInfo.end(); ++it) {
            AppendFormat(dumpString, " - DeviceType: %s\t",
                AudioInfoDumpUtils::GetDeviceTypeName((*it)->deviceType).c_str());
            AppendFormat(dumpString, "AudioStreamType: %s\t",
                AudioInfoDumpUtils::GetStreamName((*it)->streamType).c_str());
            AppendFormat(dumpString, "VolumeValue: %d\n", (*it)->volumeValue);
        }
    } else {
        AppendFormat(dumpString, "nothing Info to hidumper\n");
    }
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyDump::GetDumpDeviceInfo(std::string &dumpString,
    DeviceFlag deviceFlag)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs = GetDumpDevices(deviceFlag);

    for (const auto &desc : deviceDescs) {
        std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*desc);
        CHECK_AND_BREAK_LOG(devDesc != nullptr, "devDesc is nullptr");
        dumpString += "\n";
        AppendFormat(dumpString, "  - device name:%s\n",
            AudioInfoDumpUtils::GetDeviceTypeName(devDesc->deviceType_).c_str());
        AppendFormat(dumpString, "  - device type:%d\n", devDesc->deviceType_);
        AppendFormat(dumpString, "  - device id:%d\n", devDesc->deviceId_);
        AppendFormat(dumpString, "  - device role:%d\n", devDesc->deviceRole_);
        AppendFormat(dumpString, "  - device name:%s\n", devDesc->deviceName_.c_str());
        AppendFormat(dumpString, "  - device mac:%s\n", GetEncryptAddr(devDesc->macAddress_).c_str());
        AppendFormat(dumpString, "  - device network:%s\n", devDesc->networkId_.c_str());
        if (deviceFlag == DeviceFlag::INPUT_DEVICES_FLAG || deviceFlag == DeviceFlag::OUTPUT_DEVICES_FLAG) {
            conneceType_  = CONNECT_TYPE_LOCAL;
        } else if (deviceFlag == DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG ||
                deviceFlag == DeviceFlag::DISTRIBUTED_OUTPUT_DEVICES_FLAG) {
            conneceType_  = CONNECT_TYPE_DISTRIBUTED;
        }
        AppendFormat(dumpString, "  - connect type:%s\n", AudioInfoDumpUtils::GetConnectTypeName(conneceType_).c_str());
        for (auto &streamInfo : devDesc->audioStreamInfo_) {
            AppendFormat(dumpString, "  - device sampleRates:");
            for (auto &samplingRate : streamInfo.samplingRate) {
                AppendFormat(dumpString, "%d ", samplingRate);
            }
            AppendFormat(dumpString, "\n");
            AppendFormat(dumpString, "  - device channelLayouts:");
            for (auto &layout : streamInfo.channelLayout) {
                AppendFormat(dumpString, "%d ", layout);
            }
            AppendFormat(dumpString, "\n");
            AppendFormat(dumpString, "  - device format:%d\n", streamInfo.format);
        }
    }
    return deviceDescs;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyDump::GetDumpDevices(DeviceFlag deviceFlag)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    switch (deviceFlag) {
        case NONE_DEVICES_FLAG:
        case DISTRIBUTED_OUTPUT_DEVICES_FLAG:
        case DISTRIBUTED_INPUT_DEVICES_FLAG:
        case ALL_DISTRIBUTED_DEVICES_FLAG:
        case ALL_L_D_DEVICES_FLAG:
            if (!hasSystemPermission) {
                AUDIO_ERR_LOG("GetDevices: No system permission");
                std::vector<std::shared_ptr<AudioDeviceDescriptor>> info = {};
                return info;
            }
            break;
        default:
            break;
    }

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs =
        audioConnectedDevice_.GetDevicesInner(deviceFlag);
    if (!hasSystemPermission) {
        for (std::shared_ptr<AudioDeviceDescriptor> desc : deviceDescs) {
            desc->networkId_ = "";
            desc->interruptGroupId_ = GROUP_ID_NONE;
            desc->volumeGroupId_ = GROUP_ID_NONE;
        }
    }
    return deviceDescs;
}

void AudioPolicyDump::GetMicrophoneDescriptorsDump(std::string &dumpString)
{
    dumpString += "\nAvailable MicrophoneDescriptors:\n";

    std::vector<sptr<MicrophoneDescriptor>> micDescs = audioMicrophoneDescriptor_.GetAvailableMicrophones();
    for (auto it = micDescs.begin();
        it != micDescs.end(); ++it) {
        AppendFormat(dumpString, " - id:%d \n", (*it)->micId_);
        AppendFormat(dumpString, " - device type:%d  \n", (*it)->deviceType_);
        AppendFormat(dumpString, " - group id:%d  \n", (*it)->groupId_);
        AppendFormat(dumpString, " - sensitivity:%d  \n", (*it)->sensitivity_);
        AppendFormat(dumpString, " - position:%f %f %f (x, y, z)\n",
            (*it)->position_.x, (*it)->position_.y, (*it)->position_.z);
        AppendFormat(dumpString, " - orientation:%f %f %f (x, y, z)\n",
            (*it)->orientation_.x, (*it)->orientation_.y, (*it)->orientation_.z);
    }
}

void AudioPolicyDump::GetOffloadStatusDump(std::string &dumpString)
{
    dumpString += "\nOffload status:";
    DeviceType dev = audioActiveDevice_.GetCurrentOutputDeviceType();
    if (dev != DEVICE_TYPE_SPEAKER && dev != DEVICE_TYPE_USB_HEADSET && dev != DEVICE_TYPE_BLUETOOTH_A2DP) {
        AppendFormat(dumpString, " - current device do not supportted offload: %d\n", dev);
    }
    dumpString += "\nPrimary Offload\n";
    if (dev == DEVICE_TYPE_SPEAKER || dev == DEVICE_TYPE_USB_HEADSET) {
        AppendFormat(dumpString, " - primary deviceType : %d\n", dev);
        AppendFormat(dumpString, " - primary offloadEnable : %d\n", audioOffloadStream_.GetOffloadAvailableFromXml());
    } else {
        AppendFormat(dumpString, " - current device is not primary\n");
    }
    dumpString += "\nA2DP offload\n";
    if (dev == DEVICE_TYPE_BLUETOOTH_A2DP) {
        AppendFormat(dumpString, " - A2DP deviceType: %d\n", dev);
        AppendFormat(dumpString, " - A2DP offloadstatus : %d\n", audioA2dpOffloadFlag_.GetA2dpOffloadFlag());
    } else {
        AppendFormat(dumpString, " - current device is not A2DP\n");
    }
    AppendFormat(dumpString, "\n");
}


void AudioPolicyDump::AudioModeDump(std::string &dumpString)
{
    GetCallStatusDump(dumpString);
    GetRingerModeDump(dumpString);
    GetRingerModeInfoDump(dumpString);
}

// LCOV_EXCL_START
void AudioPolicyDump::GetCallStatusDump(std::string &dumpString)
{
    dumpString += "\nAudio Scene:";
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    AudioScene callStatus = audioSceneManager_.GetAudioScene(hasSystemPermission);
    switch (callStatus) {
        case AUDIO_SCENE_DEFAULT:
            dumpString += "DEFAULT";
            break;
        case AUDIO_SCENE_RINGING:
            dumpString += "RINGING";
            break;
        case AUDIO_SCENE_PHONE_CALL:
            dumpString += "PHONE_CALL";
            break;
        case AUDIO_SCENE_PHONE_CHAT:
            dumpString += "PHONE_CHAT";
            break;
        default:
            dumpString += "UNKNOWN";
    }
    dumpString += "\n";
}
// LCOV_EXCL_STOP

void AudioPolicyDump::GetRingerModeDump(std::string &dumpString)
{
    dumpString += "Ringer Mode:";
    AudioRingerMode ringerMode = audioPolicyManager_.GetRingerMode();
    switch (ringerMode) {
        case RINGER_MODE_NORMAL:
            dumpString += "NORMAL";
            break;
        case RINGER_MODE_SILENT:
            dumpString += "SILENT";
            break;
        case RINGER_MODE_VIBRATE:
            dumpString += "VIBRATE";
            break;
        default:
            dumpString += "UNKNOWN";
    }
    dumpString += "\n\n";
}

void AudioPolicyDump::GetRingerModeInfoDump(std::string &dumpString)
{
    dumpString += "\nSetRingerMode Info:\n";
    std::vector<RingerModeAdjustInfo> ringerModeInfo;
    audioPolicyManager_.GetRingerModeInfo(ringerModeInfo);
    if (ringerModeInfo.size() > 0) {
        for (const auto &item : ringerModeInfo) {
            AppendFormat(dumpString, " - RingerMode: %s  ", GetRingerModeType(item.ringMode).c_str());
            AppendFormat(dumpString, "CallerName: %s  ", item.callerName.c_str());
            AppendFormat(dumpString, "InvocationTime: %s\n", item.invocationTime.c_str());
        }
    } else {
        AppendFormat(dumpString, "nothing Info to hidumper\n");
    }
}

void AudioPolicyDump::StreamVolumesDump(std::string &dumpString)
{
    dumpString += "\nStream Volumes:\n";
    // Get stream volumes
    std::map<AudioStreamType, int32_t> streamVolumes_;
    for (int stream = AudioStreamType::STREAM_VOICE_CALL; stream <= AudioStreamType::STREAM_TYPE_MAX; stream++) {
        AudioStreamType streamType = (AudioStreamType)stream;

        if (IsStreamSupported(streamType)) {
            if (streamType == STREAM_ALL) {
                streamType = STREAM_MUSIC;
                AUDIO_DEBUG_LOG("GetVolume of STREAM_ALL for streamType = %{public}d ", streamType);
            }
            int32_t volume = audioVolumeManager_.GetSystemVolumeLevel(streamType);
            streamVolumes_.insert({ streamType, volume });
        }
    }
    AppendFormat(dumpString, "   [StreamName]: [Volume]\n");
    for (auto it = streamVolumes_.cbegin(); it != streamVolumes_.cend();
        ++it) {
        AppendFormat(dumpString, " - %s: %d\n", AudioInfoDumpUtils::GetStreamName(it->first).c_str(), it->second);
    }
    GetVolumeConfigDump(dumpString);
    GetGroupInfoDump(dumpString);
    audioPolicyManager_.SafeVolumeDump(dumpString);
    GetAdjustVolumeDump(dumpString);

    std::vector<VolumeKeyEventRegistration> volumeKeyRegistrations;
    audioVolumeManager_.GetVolumeKeyRegistrationInfo(volumeKeyRegistrations);
    AppendFormat(dumpString, "\nVolume Key Event Registrations:\n");
    if (volumeKeyRegistrations.size() > 0) {
        for (const auto &registration : volumeKeyRegistrations) {
            AppendFormat(dumpString, " - KeyType: %s\t\t", registration.keyType.c_str());
            AppendFormat(dumpString, "SubscriptionId: %d\t ", registration.subscriptionId);
            AppendFormat(dumpString, "InvocationTime: %s\t", registration.registrationTime.c_str());
            AppendFormat(dumpString, "Regist Success: %s\n", registration.registrationResult ? "Yes" : "No");
        }
    } else {
        AppendFormat(dumpString, "\nnothing Info to hidumper\n");
    }
}

void AudioPolicyDump::GetVolumeConfigDump(std::string &dumpString)
{
    dumpString += "\nVolume config of streams:\n";

    StreamVolumeInfoMap streamVolumeInfos;
    audioPolicyManager_.GetStreamVolumeInfoMap(streamVolumeInfos);
    for (auto it = streamVolumeInfos.cbegin();
        it != streamVolumeInfos.cend(); ++it) {
        auto streamType = it->first;
        AppendFormat(dumpString, " %s: ", AudioInfoDumpUtils::GetStreamName(streamType).c_str());
        if (streamType == STREAM_ALL) {
            streamType = STREAM_MUSIC;
            AUDIO_INFO_LOG("GetStreamMute of STREAM_ALL for streamType = %{public}d ", streamType);
        }
        AppendFormat(dumpString, "mute = %d  ", audioVolumeManager_.GetStreamMute(streamType));
        auto streamVolumeInfo = it->second;
        AppendFormat(dumpString, "minLevel = %d  ", streamVolumeInfo->minLevel);
        AppendFormat(dumpString, "maxLevel = %d  ", streamVolumeInfo->maxLevel);
        AppendFormat(dumpString, "defaultLevel = %d\n", streamVolumeInfo->defaultLevel);
        DeviceVolumeInfosDump(dumpString, streamVolumeInfo->deviceVolumeInfos);
    }
}

void AudioPolicyDump::DeviceVolumeInfosDump(std::string &dumpString, DeviceVolumeInfoMap &deviceVolumeInfos)
{
    for (auto iter = deviceVolumeInfos.cbegin(); iter != deviceVolumeInfos.cend(); ++iter) {
        AppendFormat(dumpString, "    %s : {", AudioInfoDumpUtils::GetDeviceVolumeTypeName(iter->first).c_str());
        auto volumePoints = iter->second->volumePoints;
        for (auto volPoint = volumePoints.cbegin(); volPoint != volumePoints.cend(); ++volPoint) {
            AppendFormat(dumpString, "[%u, %d]", volPoint->index, volPoint->dbValue);
            if (volPoint + 1 != volumePoints.cend()) {
                dumpString += ", ";
            }
        }
        dumpString += "}\n";
    }
}

void AudioPolicyDump::GetGroupInfoDump(std::string &dumpString)
{
    dumpString += "\nVolume GroupInfo:\n";
    // Get group info
    std::vector<sptr<VolumeGroupInfo>> groupInfos;
    audioVolumeManager_.GetVolumeGroupInfo(groupInfos);
    AppendFormat(dumpString, "- %zu Group Infos (s) available :\n", groupInfos.size());

    for (auto it = groupInfos.begin(); it != groupInfos.end(); it++) {
        AppendFormat(dumpString, "  Group Infos %d\n", it - groupInfos.begin() + 1);
        AppendFormat(dumpString, "  - ConnectType(0 for Local, 1 for Remote): %d\n", (*it)->connectType_);
        AppendFormat(dumpString, "  - Name: %s\n", (*it)->groupName_.c_str());
        AppendFormat(dumpString, "  - Id: %d\n", (*it)->volumeGroupId_);
    }
    dumpString += "\n";
}

void AudioPolicyDump::AudioPolicyParserDumpAdapterInfo(std::string &dumpString,
    std::unordered_map<AudioAdapterType, std::shared_ptr<PolicyAdapterInfo>>& adapterInfoMap)
{
    for (auto &[adapterType, adapterInfo] : adapterInfoMap) {
        AppendFormat(dumpString, " - adapter : %s -- adapterType=%u, supportSelectScene=%s\n",
            adapterInfo->adapterName.c_str(), adapterType, adapterInfo->adapterSupportScene.c_str());
        AudioPolicyParserDumpPipeInfo(dumpString, adapterInfo);

        for (auto &deviceInfo : adapterInfo->deviceInfos) {
            AppendFormat(dumpString, "     - device : %s -- type=%u, pin=%u, role=%u\n",
                deviceInfo->name_.c_str(), deviceInfo->type_, deviceInfo->pin_, deviceInfo->role_);
            AppendFormat(dumpString, "         - support pipe | ");
            for (auto pipeIt : deviceInfo->supportPipeMap_) {
                AppendFormat(dumpString, "%s,", pipeIt.second->name_.c_str());
            }
            AppendFormat(dumpString, "\n");
        }
    }
}

void AudioPolicyDump::AudioPolicyParserDumpPipeInfo(std::string &dumpString,
    std::shared_ptr<PolicyAdapterInfo> &adapterInfo)
{
    for (auto &pipeInfo : adapterInfo->pipeInfos) {
        AppendFormat(dumpString, "     -pipeInfo : %s -- role=%u, supportFlags=0x%x, lib=%s, "
            "paPropRole=%s, fixedLatency=%s, renderInIdleState=%s\n", pipeInfo->name_.c_str(),
            pipeInfo->role_, pipeInfo->supportFlags_, pipeInfo->paProp_.lib_.c_str(),
            pipeInfo->paProp_.role_.c_str(), pipeInfo->paProp_.fixedLatency_.c_str(),
            pipeInfo->paProp_.renderInIdleState_.c_str());

        for (auto &streamProp : pipeInfo->streamPropInfos_) {
            AppendFormat(dumpString, "         - streamProp : -- format=%zu, sampleRates=%zu, channelLayout=%zu,"
                " channels=%zu, bufferSize=%zu\n", streamProp->format_, streamProp->sampleRate_,
                streamProp->channelLayout_, streamProp->channels_, streamProp->bufferSize_);
            AppendFormat(dumpString, "             - support device | ");
            for (auto deviceIt : streamProp->supportDeviceMap_) {
                AppendFormat(dumpString, "%s,", deviceIt.second->name_.c_str());
            }
            AppendFormat(dumpString, "\n");
        }

        for (auto &attributeInfo : pipeInfo->attributeInfos_) {
            AppendFormat(dumpString, "         - attribute : -- name=%s, value=%s\n", attributeInfo->name_.c_str(),
                attributeInfo->value_.c_str());
        }
    }
}

void AudioPolicyDump::GetAdjustVolumeDump(std::string &dumpString)
{
    dumpString += "\nSystemVolumeLevel Info:\n";
    std::vector<AdjustVolumeInfo> systemVolumeLevelInfo;
    audioVolumeManager_.GetSystemVolumeLevelInfo(systemVolumeLevelInfo);
    if (systemVolumeLevelInfo.size() > 0) {
        for (const auto &item : systemVolumeLevelInfo) {
            AppendFormat(dumpString, " - DeviceType: %s",
                AudioInfoDumpUtils::GetDeviceTypeName(item.deviceType).c_str());
            AppendFormat(dumpString, "\tStreamType: %s     ",
                AudioInfoDumpUtils::GetStreamName(item.streamType).c_str());
            AppendFormat(dumpString, "\tVolumeLevel: %d\n", item.volumeLevel);
            AppExecFwk::BundleInfo bundleInfo = AudioBundleManager::GetBundleInfoFromUid(item.appUid);
            std::string callerName = bundleInfo.name == "" ? CALLER_NAME : bundleInfo.name;
            AppendFormat(dumpString, "\tCallerName: %s", callerName.c_str());
            AppendFormat(dumpString, "\tInvocationTime: %s\n", item.invocationTime.c_str());
        }
    } else {
        AppendFormat(dumpString, "nothing Info to hidumper\n");
    }

    std::vector<AdjustStreamVolumeInfo> adjustStreamVolumeInfo;
    dumpString += "\nSetStreamVolume Info:\n";
    adjustStreamVolumeInfo = audioPolicyManager_.GetStreamVolumeInfo(AdjustStreamVolume::STREAM_VOLUME_INFO);
    AdjustVolumeAppend(adjustStreamVolumeInfo, dumpString);

    dumpString += "\nSetLowPowerVolume Info:\n";
    adjustStreamVolumeInfo = audioPolicyManager_.GetStreamVolumeInfo(AdjustStreamVolume::LOW_POWER_VOLUME_INFO);
    AdjustVolumeAppend(adjustStreamVolumeInfo, dumpString);

    dumpString += "\nSetDuckVolume Info:\n";
    adjustStreamVolumeInfo = audioPolicyManager_.GetStreamVolumeInfo(AdjustStreamVolume::DUCK_VOLUME_INFO);
    AdjustVolumeAppend(adjustStreamVolumeInfo, dumpString);
}

void AudioPolicyDump::AdjustVolumeAppend(std::vector<AdjustStreamVolumeInfo> adjustInfo, std::string &dumpString)
{
    if (adjustInfo.size() > 0) {
        for (const auto &item : adjustInfo) {
            AppendFormat(dumpString, " - VolumeValue: %f  ", item.volume);
            AppendFormat(dumpString, "SessionId: %u  ", item.sessionId);
            AppendFormat(dumpString, "InvocationTime: %s\n", item.invocationTime.c_str());
        }
    } else {
        AppendFormat(dumpString, "nothing Info to hidumper\n");
    }
}

void AudioPolicyDump::AudioPolicyParserDumpInner(std::string &dumpString,
    const std::unordered_map<std::string, std::string>& volumeGroupData,
    std::unordered_map<std::string, std::string>& interruptGroupData, PolicyGlobalConfigs globalConfigs)
{
    for (auto& volume : volumeGroupData) {
        AppendFormat(dumpString, " - volumeGroupMap_ first:%s, second:%s\n", volume.first.c_str(),
            volume.second.c_str());
    }
    for (auto& interrupt : interruptGroupData) {
        AppendFormat(dumpString, " - interruptGroupMap_ first:%s, second:%s\n", interrupt.first.c_str(),
            interrupt.second.c_str());
    }
    AppendFormat(dumpString, " - globalConfig  adapter:%s, pipe:%s, device:%s, updateRouteSupport:%d, "
        "audioLatency:%s, sinkLatency:%s\n", globalConfigs.adapter_.c_str(),
        globalConfigs.pipe_.c_str(), globalConfigs.device_.c_str(),
        globalConfigs.updateRouteSupport_,
        globalConfigs.globalPaConfigs_.audioLatency_.c_str(),
        globalConfigs.globalPaConfigs_.sinkLatency_.c_str());
    for (auto &commonConfig : globalConfigs.commonConfigs_) {
        AppendFormat(dumpString, "     - common config name:%s, type:%s, value:%s\n", commonConfig.name_.c_str(),
            commonConfig.type_.c_str(), commonConfig.value_.c_str());
    }
    AppendFormat(dumpString, " - module curActiveCount:%d\n\n", audioPolicyManager_.GetCurActivateCount());
}

void AudioPolicyDump::AudioPolicyParserDump(std::string &dumpString)
{
    dumpString += "\nAudioPolicyParser:\n";
    std::unordered_map<AudioAdapterType, std::shared_ptr<PolicyAdapterInfo>> adapterInfoMap;
    std::unordered_map<std::string, std::string> volumeGroupData;
    std::unordered_map<std::string, std::string> interruptGroupData;
    PolicyGlobalConfigs globalConfigs;

    audioConfigManager_.GetAudioAdapterInfos(adapterInfoMap);
    audioConfigManager_.GetVolumeGroupData(volumeGroupData);
    audioConfigManager_.GetInterruptGroupData(interruptGroupData);
    audioConfigManager_.GetGlobalConfigs(globalConfigs);

    AudioPolicyParserDumpAdapterInfo(dumpString, adapterInfoMap);
    AudioPolicyParserDumpInner(dumpString, volumeGroupData, interruptGroupData, globalConfigs);
}

void AudioPolicyDump::AudioStreamDump(std::string &dumpString)
{
    dumpString += "\nAudioRenderer stream:\n";
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    streamCollector_.GetCurrentRendererChangeInfos(audioRendererChangeInfos);

    AppendFormat(dumpString, " - audiorenderer stream size : %zu\n", audioRendererChangeInfos.size());
    for (auto it = audioRendererChangeInfos.begin(); it != audioRendererChangeInfos.end(); it++) {
        if ((*it)->rendererInfo.rendererFlags == STREAM_FLAG_NORMAL) {
            AppendFormat(dumpString, " - normal AudioCapturer stream:\n");
        } else if ((*it)->rendererInfo.rendererFlags == STREAM_FLAG_FAST) {
            AppendFormat(dumpString, " - fast AudioCapturer stream:\n");
        }
        AppendFormat(dumpString, " - rendererStatus : %d\n", (*it)->rendererState);
        AppendFormat(dumpString, " - clientUID : %d\n", (*it)->clientUID);
        AppendFormat(dumpString, " - streamId : %d\n", (*it)->sessionId);
        AppendFormat(dumpString, " - deviceType : %d\n", (*it)->outputDeviceInfo.deviceType_);
        AppendFormat(dumpString, " - contentType : %d\n", (*it)->rendererInfo.contentType);
        AppendFormat(dumpString, " - streamUsage : %d\n", (*it)->rendererInfo.streamUsage);
        AppendFormat(dumpString, " - samplingRate : %d\n", (*it)->rendererInfo.samplingRate);
        AppendFormat(dumpString, " - pipeType : %d\n", (*it)->rendererInfo.pipeType);
        AppendFormat(dumpString, " - rendererState : %d\n", (*it)->rendererState);
        AppendFormat(dumpString, " - appVolume : %d\n", (*it)->appVolume);
    }
    GetCapturerStreamDump(dumpString);
}

void AudioPolicyDump::GetCapturerStreamDump(std::string &dumpString)
{
    dumpString += "\nAudioCapturer stream:\n";
    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    streamCollector_.GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    AppendFormat(dumpString, " - audiocapturer stream size : %zu\n", audioCapturerChangeInfos.size());
    for (auto it = audioCapturerChangeInfos.begin(); it != audioCapturerChangeInfos.end(); it++) {
        if ((*it)->capturerInfo.capturerFlags == STREAM_FLAG_NORMAL) {
            AppendFormat(dumpString, " - normal AudioCapturer stream:\n");
        } else if ((*it)->capturerInfo.capturerFlags == STREAM_FLAG_FAST) {
            AppendFormat(dumpString, " - fast AudioCapturer stream:\n");
        }
        AppendFormat(dumpString, " - clientUID : %d\n", (*it)->clientUID);
        AppendFormat(dumpString, " - streamId : %d\n", (*it)->sessionId);
        AppendFormat(dumpString, " - is muted : %s\n", (*it)->muted ? "true" : "false");
        AppendFormat(dumpString, " - deviceType : %d\n", (*it)->inputDeviceInfo.deviceType_);
        AppendFormat(dumpString, " - samplingRate : %d\n", (*it)->capturerInfo.samplingRate);
        AppendFormat(dumpString, " - pipeType : %d\n", (*it)->capturerInfo.pipeType);
        AppendFormat(dumpString, " - capturerState : %d\n", (*it)->capturerState);
    }
}

void AudioPolicyDump::XmlParsedDataMapDump(std::string &dumpString)
{
    dumpString += "\nXmlParsedDataParser:\n";

    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo = {};
    audioConfigManager_.GetDeviceClassInfo(deviceClassInfo);

    for (auto &[adapterType, deviceClassInfos] : deviceClassInfo) {
        AppendFormat(dumpString, " - DeviceClassInfo type %d\n", adapterType);
        for (const auto &deviceClassInfoIter : deviceClassInfos) {
            AppendFormat(dumpString, " - Data : className:%s, name:%s, adapter:%s, id:%s, lib:%s, role:%s, rate:%s\n",
                deviceClassInfoIter.className.c_str(), deviceClassInfoIter.name.c_str(),
                deviceClassInfoIter.adapterName.c_str(), deviceClassInfoIter.id.c_str(),
                deviceClassInfoIter.lib.c_str(), deviceClassInfoIter.role.c_str(), deviceClassInfoIter.rate.c_str());

            for (auto rate : deviceClassInfoIter.supportedRate_) {
                AppendFormat(dumpString, "     - rate:%u\n", rate);
            }

            for (auto supportedChannelLayout : deviceClassInfoIter.supportedChannelLayout_) {
                AppendFormat(dumpString, "     - supportedChannelLayout:%u\n", supportedChannelLayout);
            }

            AppendFormat(dumpString, " -DeviceClassInfo : format:%s, channels:%s, bufferSize:%s, fixedLatency:%s, "
                " sinkLatency:%s, renderInIdleState:%s, OpenMicSpeaker:%s, fileName:%s, networkId:%s, "
                "deviceType:%s, sceneName:%s, sourceType:%s, offloadEnable:%s\n",
                deviceClassInfoIter.format.c_str(), deviceClassInfoIter.channels.c_str(),
                deviceClassInfoIter.bufferSize.c_str(),
                deviceClassInfoIter.fixedLatency.c_str(), deviceClassInfoIter.sinkLatency.c_str(),
                deviceClassInfoIter.renderInIdleState.c_str(), deviceClassInfoIter.OpenMicSpeaker.c_str(),
                deviceClassInfoIter.fileName.c_str(), deviceClassInfoIter.networkId.c_str(),
                deviceClassInfoIter.deviceType.c_str(), deviceClassInfoIter.sceneName.c_str(),
                deviceClassInfoIter.sourceType.c_str(), deviceClassInfoIter.offloadEnable.c_str());
        }
        AppendFormat(dumpString, "-----EndOfXmlParsedDataMap-----\n");
    }
}

static void StreamEffectSceneInfoDump(string &dumpString, const ProcessNew &processNew, const string processType)
{
    AppendFormat(dumpString, "- %zu %s supported :\n", processNew.stream.size(), processType.c_str());

    for (Stream x : processNew.stream) {
        AppendFormat(dumpString, "  %s stream scene = %s \n", processType.c_str(), x.scene.c_str());
        int32_t count = 0;
        for (StreamEffectMode mode : x.streamEffectMode) {
            count++;
            AppendFormat(dumpString, "  - modeName%d = %s \n", count, mode.mode.c_str());
            int32_t n = 0;
            for (Device deviceInfo : mode.devicePort) {
                n++;
                AppendFormat(dumpString, "    - device%d type = %s \n", n, deviceInfo.type.c_str());
                AppendFormat(dumpString, "    - device%d chain = %s \n", n, deviceInfo.chain.c_str());
            }
        }
        dumpString += "\n";
    }
}

void AudioPolicyDump::GetEffectManagerInfo()
{
    AudioConverterParser &converterParser = AudioConverterParser::GetInstance();
    converterConfig_ = converterParser.LoadConfig();
    audioEffectService_.GetSupportedEffectConfig(supportedEffectConfig_);
}

void AudioPolicyDump::EffectManagerInfoDump(string &dumpString)
{
    int32_t count = 0;
    GetEffectManagerInfo();

    std::unordered_map<AudioAdapterType, std::shared_ptr<PolicyAdapterInfo>> adapterInfoMap;
    audioConfigManager_.GetAudioAdapterInfos(adapterInfoMap);

    dumpString += "==== Audio Effect Manager INFO ====\n";

    // effectChain info
    count = 0;
    AppendFormat(dumpString, "- system support %d effectChain(s):\n",
        supportedEffectConfig_.effectChains.size());
    for (EffectChain x : supportedEffectConfig_.effectChains) {
        count++;
        AppendFormat(dumpString, "  effectChain%d :\n", count);
        AppendFormat(dumpString, "  - effectChain name = %s \n", x.name.c_str());
        int32_t countEffect = 0;
        for (string effectUnit : x.apply) {
            countEffect++;
            AppendFormat(dumpString, "    - effectUnit%d = %s \n", countEffect, effectUnit.c_str());
        }
        dumpString += "\n";
    }

    // converter info
    AppendFormat(dumpString, "- system support audio converter for special streams:\n");
    AppendFormat(dumpString, "  - converter name: %s\n", converterConfig_.library.name.c_str());
    AppendFormat(dumpString, "  - converter out channel layout: %" PRId64 "\n",
        converterConfig_.outChannelLayout);
    dumpString += "\n";

    // preProcess info
    StreamEffectSceneInfoDump(dumpString, supportedEffectConfig_.preProcessNew, "preProcess");
    dumpString += "\n";
    // postProcess info
    StreamEffectSceneInfoDump(dumpString, supportedEffectConfig_.postProcessNew, "postProcess");

    // postProcess scene maping
    AppendFormat(dumpString, "- postProcess scene maping config:\n");
    for (SceneMappingItem it: supportedEffectConfig_.postProcessSceneMap) {
        AppendFormat(dumpString, "  - streamUsage: %s = %s \n", it.name.c_str(), it.sceneType.c_str());
    }
    dumpString += "\n";
}

void AudioPolicyDump::MicrophoneMuteInfoDump(string &dumpString)
{
    dumpString += "==== Microphone Mute INFO ====\n";
    // non-persistent microphone mute info
    AppendFormat(dumpString, "  - non-persistent microphone isMuted: %d \n",
        audioMicrophoneDescriptor_.GetMicrophoneMuteTemporary());
    // persistent microphone mute info
    AppendFormat(dumpString, "  - persistent microphone isMuted: %d \n",
        audioMicrophoneDescriptor_.GetMicrophoneMutePersistent());
    dumpString += "\n";
}

std::string AudioPolicyDump::GetRingerModeType(AudioRingerMode ringerMode)
{
    std::string audioRingerMode = "";
    switch (ringerMode) {
        case RINGER_MODE_SILENT:
            audioRingerMode = "RINGER_MODE_SILENT";
            break;
        case RINGER_MODE_VIBRATE:
            audioRingerMode = "RINGER_MODE_VIBRATE";
            break;
        case RINGER_MODE_NORMAL:
            audioRingerMode = "RINGER_MODE_NORMAL";
            break;
        default:
            audioRingerMode = "UNKNOWMTYPE";
            break;
    }
    return audioRingerMode;
}
}
}
