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
#include "data_share_observer_callback.h"

#include "audio_policy_log.h"

#include "iservice_registry.h"
#include "audio_manager_listener_stub.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "audio_converter_parser.h"


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
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioPolicyDump::GetDumpDeviceInfo(std::string &dumpString,
    DeviceFlag deviceFlag)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs = GetDumpDevices(deviceFlag);

    for (const auto &desc : deviceDescs) {
        std::shared_ptr<AudioDeviceDescriptor> devDesc = std::make_shared<AudioDeviceDescriptor>(*desc);
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
        for (auto &samplingRate : devDesc->audioStreamInfo_.samplingRate) {
            AppendFormat(dumpString, "  - device sampleRates:%d\n", samplingRate);
        }
        for (auto &channel : devDesc->audioStreamInfo_.channels) {
            AppendFormat(dumpString, "  - device channels:%d\n", channel);
        }
        AppendFormat(dumpString, "  - device format:%d\n", devDesc->audioStreamInfo_.format);
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
}

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

void AudioPolicyDump::AudioPolicyParserDumpInner(std::string &dumpString,
    const std::unordered_map<AdaptersType, AudioAdapterInfo>& adapterInfoMap,
    const std::unordered_map<std::string, std::string>& volumeGroupData,
    std::unordered_map<std::string, std::string>& interruptGroupData,
    GlobalConfigs globalConfigs)
{
    for (auto &[adapterType, adapterInfo] : adapterInfoMap) {
        AppendFormat(dumpString, " - adapter : %s -- adapterType:%u\n", adapterInfo.adapterName_.c_str(), adapterType);
        for (auto &deviceInfo : adapterInfo.deviceInfos_) {
            AppendFormat(dumpString, "     - device --  name:%s, pin:%s, type:%s, role:%s\n", deviceInfo.name_.c_str(),
                deviceInfo.pin_.c_str(), deviceInfo.type_.c_str(), deviceInfo.role_.c_str());
        }
        for (auto &pipeInfo : adapterInfo.pipeInfos_) {
            AppendFormat(dumpString, "     - module : -- name:%s, pipeRole:%s, pipeFlags:%s, lib:%s, paPropRole:%s, "
                "fixedLatency:%s, renderInIdleState:%s\n", pipeInfo.name_.c_str(),
                pipeInfo.pipeRole_.c_str(), pipeInfo.pipeFlags_.c_str(), pipeInfo.lib_.c_str(),
                pipeInfo.paPropRole_.c_str(), pipeInfo.fixedLatency_.c_str(), pipeInfo.renderInIdleState_.c_str());

            for (auto &configInfo : pipeInfo.configInfos_) {
                AppendFormat(dumpString, "         - config : -- name:%s, value:%s\n", configInfo.name_.c_str(),
                    configInfo.value_.c_str());
            }
        }
    }
    for (auto& volume : volumeGroupData) {
        AppendFormat(dumpString, " - volumeGroupMap_ first:%s, second:%s\n\n", volume.first.c_str(),
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
    for (auto &outputConfig : globalConfigs.outputConfigInfos_) {
        AppendFormat(dumpString, " - output config name:%s, type:%s, value:%s\n", outputConfig.name_.c_str(),
            outputConfig.type_.c_str(), outputConfig.value_.c_str());
    }
    for (auto &inputConfig : globalConfigs.inputConfigInfos_) {
        AppendFormat(dumpString, " - input config name:%s, type_%s, value:%s\n\n", inputConfig.name_.c_str(),
            inputConfig.type_.c_str(), inputConfig.value_.c_str());
    }
    AppendFormat(dumpString, " - module curActiveCount:%d\n\n", audioPolicyManager_.GetCurActivateCount());
}

void AudioPolicyDump::AudioPolicyParserDump(std::string &dumpString)
{
    dumpString += "\nAudioPolicyParser:\n";
    std::unordered_map<AdaptersType, AudioAdapterInfo> adapterInfoMap;
    std::unordered_map<std::string, std::string> volumeGroupData;
    std::unordered_map<std::string, std::string> interruptGroupData;
    GlobalConfigs globalConfigs;

    audioConfigManager_.GetAudioAdapterInfos(adapterInfoMap);
    audioConfigManager_.GetVolumeGroupData(volumeGroupData);
    audioConfigManager_.GetInterruptGroupData(interruptGroupData);
    audioConfigManager_.GetGlobalConfigs(globalConfigs);

    AudioPolicyParserDumpInner(dumpString, adapterInfoMap, volumeGroupData, interruptGroupData, globalConfigs);
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

            for (auto supportedChannel : deviceClassInfoIter.supportedChannels_) {
                AppendFormat(dumpString, "     - supportedChannel:%u\n", supportedChannel);
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

    std::unordered_map<AdaptersType, AudioAdapterInfo> adapterInfoMap;
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

}
}