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
#define LOG_TAG "AudioVolumeParser"
#endif

#include "audio_volume_parser.h"
#ifdef USE_CONFIG_POLICY
#include "config_policy_utils.h"
#endif
#include "audio_utils.h"
#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {
AudioVolumeParser::AudioVolumeParser()
{
    AUDIO_INFO_LOG("AudioVolumeParser ctor");
    audioStreamMap_ = {
        {"VOICE_CALL", STREAM_VOICE_CALL},
        {"MUSIC", STREAM_MUSIC},
        {"RING", STREAM_RING},
        {"VOICE_ASSISTANT", STREAM_VOICE_ASSISTANT},
        {"ALARM", STREAM_ALARM},
        {"ACCESSIBILITY", STREAM_ACCESSIBILITY},
        {"ULTRASONIC", STREAM_ULTRASONIC},
        {"SYSTEM", STREAM_SYSTEM},
        {"APP", STREAM_APP}
    };

    audioDeviceMap_ = {
        {"earpiece", EARPIECE_VOLUME_TYPE},
        {"speaker", SPEAKER_VOLUME_TYPE},
        {"headset", HEADSET_VOLUME_TYPE},
    };
}

AudioVolumeParser::~AudioVolumeParser()
{
    AUDIO_INFO_LOG("AudioVolumeParser dtor");
}

int32_t AudioVolumeParser::ParseVolumeConfig(const char *path, StreamVolumeInfoMap &streamVolumeInfoMap)
{
    std::shared_ptr<AudioXmlNode> curNode = AudioXmlNode::Create();
    int32_t ret = curNode->Config(path, nullptr, 0);
    if (ret != SUCCESS) {
        WriteVolumeConfigErrorEvent();
        return ERROR;
    }

    if (!curNode->CompareName("audio_volume_config")) {
        AUDIO_ERR_LOG("Missing tag - audio_volume_config in : %s", path);
        WriteVolumeConfigErrorEvent();
        curNode = nullptr;
        return ERROR;
    }
    curNode->MoveToChildren();
    if (!curNode->IsNodeValid()) {
        AUDIO_ERR_LOG("empty volume config in : %s", path);
        WriteVolumeConfigErrorEvent();
        curNode = nullptr;
        return ERROR;
    }

    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("volume_type")) {
            ParseStreamInfos(curNode->GetCopyNode(), streamVolumeInfoMap);
            break;
        } else {
            curNode->MoveToNext();
        }
    }
    curNode = nullptr;
    int32_t result = UseVoiceAssistantFixedVolumeConfig(streamVolumeInfoMap);
    AUDIO_INFO_LOG("The voice assistant uses a fixed volume configuration. Result: %{public}d", result);
    return SUCCESS;
}

int32_t AudioVolumeParser::UseVoiceAssistantFixedVolumeConfig(StreamVolumeInfoMap &streamVolumeInfoMap)
{
    if (streamVolumeInfoMap.find(STREAM_VOICE_ASSISTANT) == streamVolumeInfoMap.end() ||
        streamVolumeInfoMap[STREAM_VOICE_ASSISTANT] == nullptr) {
        AUDIO_ERR_LOG("Failed to find the volume config of STREAM_VOICE_ASSISTANT!");
        return ERROR;
    }

    // Allow to set voice assistant volume to 0.
    streamVolumeInfoMap[STREAM_VOICE_ASSISTANT]->minLevel = 0;

    // Modify the volume point index for volume level 0.
    const std::vector<DeviceVolumeType> DEVICE_VOLUME_TYPE_LIST = {
        EARPIECE_VOLUME_TYPE,
        SPEAKER_VOLUME_TYPE,
        HEADSET_VOLUME_TYPE,
    };
    DeviceVolumeInfoMap &deviceVolumeInfos = streamVolumeInfoMap[STREAM_VOICE_ASSISTANT]->deviceVolumeInfos;
    for (auto device : DEVICE_VOLUME_TYPE_LIST) {
        if (deviceVolumeInfos.find(device) == deviceVolumeInfos.end() ||
            deviceVolumeInfos[device] == nullptr) {
            AUDIO_ERR_LOG("Failed to find the device %{public}d in deviceVolumeInfos!", device);
            continue;
        }
        deviceVolumeInfos[device]->minLevel = -1; // Ensure that the minLevel of this device is an invalid value.
        std::vector<VolumePoint> &volumePoints = deviceVolumeInfos[device]->volumePoints;
        if (volumePoints.empty()) {
            AUDIO_ERR_LOG("The vector fo volumePoints is empty!");
            continue;
        }
        if (volumePoints[0].index == 0) {
            volumePoints[0].index = 1;
        }
    }
    return SUCCESS;
}

void AudioVolumeParser::WriteVolumeConfigErrorEvent()
{
    Trace trace("SYSEVENT FAULT EVENT LOAD_CONFIG_ERROR, CATEGORY: "
        + std::to_string(Media::MediaMonitor::AUDIO_VOLUME_CONFIG));
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::LOAD_CONFIG_ERROR,
        Media::MediaMonitor::FAULT_EVENT);
    bean->Add("CATEGORY", Media::MediaMonitor::AUDIO_VOLUME_CONFIG);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

int32_t AudioVolumeParser::LoadConfig(StreamVolumeInfoMap &streamVolumeInfoMap)
{
    AUDIO_INFO_LOG("Load Volume Config xml");
    int ret = ERROR;
#ifdef USE_CONFIG_POLICY
    CfgFiles *cfgFiles = GetCfgFiles(AUDIO_VOLUME_CONFIG_FILE);
    if (cfgFiles == nullptr) {
        Trace trace("SYSEVENT FAULT EVENT LOAD_CONFIG_ERROR, CATEGORY: "
            + std::to_string(Media::MediaMonitor::AUDIO_VOLUME_CONFIG));
        AUDIO_ERR_LOG("Not found audio_volume_config.xml!");
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::AUDIO, Media::MediaMonitor::LOAD_CONFIG_ERROR,
            Media::MediaMonitor::FAULT_EVENT);
        bean->Add("CATEGORY", Media::MediaMonitor::AUDIO_VOLUME_CONFIG);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
        return ERROR;
    }

    for (int32_t i = MAX_CFG_POLICY_DIRS_CNT - 1; i >= 0; i--) {
        if (cfgFiles->paths[i] && *(cfgFiles->paths[i]) != '\0') {
            AUDIO_INFO_LOG("volume config file path:%{public}s", cfgFiles->paths[i]);
            ret = ParseVolumeConfig(cfgFiles->paths[i], streamVolumeInfoMap);
            break;
        }
    }
    FreeCfgFiles(cfgFiles);
#else
    ret = ParseVolumeConfig(AUDIO_VOLUME_CONFIG_FILE, streamVolumeInfoMap);
    AUDIO_INFO_LOG("use default volume config file path:%{public}s", AUDIO_VOLUME_CONFIG_FILE);
#endif
    return ret;
}

void AudioVolumeParser::ParseStreamInfos(std::shared_ptr<AudioXmlNode> curNode,
    StreamVolumeInfoMap &streamVolumeInfoMap)
{
    AUDIO_DEBUG_LOG("AudioVolumeParser::ParseStreamInfos");
    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("volume_type")) {
            std::shared_ptr<StreamVolumeInfo> streamVolInfo = std::make_shared<StreamVolumeInfo>();
            if (ParseStreamVolumeInfoAttr(curNode->GetCopyNode(), streamVolInfo) == AUDIO_OK) {
                ParseDeviceVolumeInfos(curNode->GetChildrenNode(), streamVolInfo);
                AUDIO_DEBUG_LOG("Parse streamType:%{public}d ", streamVolInfo->streamType);
                streamVolumeInfoMap[streamVolInfo->streamType] = streamVolInfo;
            }
        }
        curNode->MoveToNext();
    }
}

int32_t AudioVolumeParser::ParseStreamVolumeInfoAttr(std::shared_ptr<AudioXmlNode> curNode,
    std::shared_ptr<StreamVolumeInfo> &streamVolInfo)
{
    AUDIO_DEBUG_LOG("AudioVolumeParser::ParseStreamVolumeInfoAttr");
    std::string pValueStr;
    CHECK_AND_RETURN_RET_LOG(curNode->GetProp("type", pValueStr) == SUCCESS,
        ERR_INVALID_PARAM, "invalid type parameter");

    if (pValueStr == "VOICE_PC") {
        VolumeUtils::SetPCVolumeEnable(true);
        AUDIO_INFO_LOG("PC Volume is Enable");
        // only read PC volume flag
        return ERR_NOT_SUPPORTED;
    }
    streamVolInfo->streamType = audioStreamMap_[pValueStr];

    CHECK_AND_RETURN_RET_LOG(curNode->GetProp("minidx", pValueStr) == SUCCESS,
        ERR_INVALID_PARAM, "invalid minidx parameter");
    CHECK_AND_RETURN_RET_LOG(StringConverter<int32_t>(pValueStr, streamVolInfo->minLevel), ERROR,
        "convert streamVolInfo->minLevel fail!");
    AUDIO_DEBUG_LOG("minidx: %{public}d", streamVolInfo->minLevel);

    CHECK_AND_RETURN_RET_LOG(curNode->GetProp("maxidx", pValueStr) == SUCCESS,
        ERR_INVALID_PARAM, "invalid maxidx parameter");
    CHECK_AND_RETURN_RET_LOG(StringConverter<int32_t>(pValueStr, streamVolInfo->maxLevel), ERROR,
        "convert streamVolInfo->maxLevel fail!");
    AUDIO_DEBUG_LOG("maxidx: %{public}d", streamVolInfo->maxLevel);

    CHECK_AND_RETURN_RET_LOG(curNode->GetProp("defaultidx", pValueStr) == SUCCESS,
        ERR_INVALID_PARAM, "invalid defaultidx parameter");
    CHECK_AND_RETURN_RET_LOG(StringConverter<int32_t>(pValueStr, streamVolInfo->defaultLevel), ERROR,
        "convert streamVolInfo->defaultLevel fail!");
    AUDIO_DEBUG_LOG("defaultidx: %{public}d", streamVolInfo->defaultLevel);

    return AUDIO_OK;
}

void AudioVolumeParser::ParseDeviceVolumeInfos(std::shared_ptr<AudioXmlNode> curNode,
    std::shared_ptr<StreamVolumeInfo> &streamVolInfo)
{
    AUDIO_DEBUG_LOG("AudioVolumeParser::ParseDeviceVolumeInfos");
    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("volumecurve")) {
            std::string pValueStr;
            curNode->GetProp("deviceClass", pValueStr);
            std::shared_ptr<DeviceVolumeInfo> deviceVolInfo = std::make_shared<DeviceVolumeInfo>();
            deviceVolInfo->deviceType = audioDeviceMap_[pValueStr];
            AUDIO_DEBUG_LOG("deviceVolInfo->deviceType %{public}d;", deviceVolInfo->deviceType);
            int32_t result = curNode->GetProp("minidx", pValueStr);
            if (result == SUCCESS) {
                StringConverter<int32_t>(pValueStr, deviceVolInfo->minLevel);
                AUDIO_DEBUG_LOG("minidx: %{public}d", deviceVolInfo->minLevel);
            } else {
                AUDIO_DEBUG_LOG("The minidx attribute is not configured or minidx parameter is invalid");
            }
            result = curNode->GetProp("maxidx", pValueStr);
            if (result == SUCCESS) {
                StringConverter<int32_t>(pValueStr, deviceVolInfo->maxLevel);
                AUDIO_DEBUG_LOG("maxidx: %{public}d", deviceVolInfo->maxLevel);
            } else {
                AUDIO_DEBUG_LOG("The maxidx attribute is not configured or maxidx parameter is invalid");
            }
            result = curNode->GetProp("defaultidx", pValueStr);
            if (result == SUCCESS) {
                StringConverter<int32_t>(pValueStr, deviceVolInfo->defaultLevel);
                AUDIO_DEBUG_LOG("defaultidx: %{public}d", deviceVolInfo->defaultLevel);
            } else {
                AUDIO_DEBUG_LOG("The defaultidx attribute is not configured or defaultidx parameter is invalid");
            }
            ParseVolumePoints(curNode->GetChildrenNode(), deviceVolInfo);
            streamVolInfo->deviceVolumeInfos[deviceVolInfo->deviceType] = deviceVolInfo;
        }
        curNode->MoveToNext();
    }
}

void AudioVolumeParser::ParseVolumePoints(std::shared_ptr<AudioXmlNode> curNode,
    std::shared_ptr<DeviceVolumeInfo> &deviceVolInfo)
{
    AUDIO_DEBUG_LOG("AudioVolumeParser::ParseVolumePoints");
    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("point")) {
            struct VolumePoint volumePoint;
            std::string pValueStr;
            curNode->GetProp("idx", pValueStr);
            CHECK_AND_RETURN_LOG(StringConverter(pValueStr, volumePoint.index),
                "convert volumePoint.index fail!");
            AUDIO_DEBUG_LOG("idx: %{public}d", volumePoint.index);

            curNode->GetProp("decibel", pValueStr);
            CHECK_AND_RETURN_LOG(StringConverter(pValueStr, volumePoint.dbValue),
                "convert volumePoint.dbValue fail!");
            AUDIO_DEBUG_LOG("decibel: %{public}d", volumePoint.dbValue);

            deviceVolInfo->volumePoints.push_back(volumePoint);
        }
        curNode->MoveToNext();
    }
}
} // namespace AudioStandard
} // namespace OHOS
