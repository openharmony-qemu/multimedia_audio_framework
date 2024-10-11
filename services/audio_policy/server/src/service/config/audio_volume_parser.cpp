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
    xmlDoc *doc = nullptr;
    xmlNode *rootElement = nullptr;
    doc = xmlReadFile(path, nullptr, 0);
    if (doc == nullptr) {
        WriteVolumeConfigErrorEvent();
        return ERROR;
    }
    rootElement = xmlDocGetRootElement(doc);
    xmlNode *currNode = rootElement;
    CHECK_AND_RETURN_RET_LOG(currNode != nullptr, ERROR, "root element is null");
    if (xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("audio_volume_config"))) {
        AUDIO_ERR_LOG("Missing tag - audio_volume_config in : %s", path);
        WriteVolumeConfigErrorEvent();
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return ERROR;
    }
    if (currNode->children) {
        currNode = currNode->children;
    } else {
        AUDIO_ERR_LOG("empty volume config in : %s", path);
        WriteVolumeConfigErrorEvent();
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return ERROR;
    }

    while (currNode != nullptr) {
        if ((currNode->type == XML_ELEMENT_NODE) &&
            (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("volume_type")))) {
            ParseStreamInfos(currNode, streamVolumeInfoMap);
            break;
        } else {
            currNode = currNode->next;
        }
    }

    xmlFreeDoc(doc);
    xmlCleanupParser();
    return SUCCESS;
}

void AudioVolumeParser::WriteVolumeConfigErrorEvent()
{
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

void AudioVolumeParser::ParseStreamInfos(xmlNode *node, StreamVolumeInfoMap &streamVolumeInfoMap)
{
    xmlNode *currNode = node;
    AUDIO_DEBUG_LOG("AudioVolumeParser::ParseStreamInfos");
    while (currNode) {
        if (currNode->type == XML_ELEMENT_NODE
            && (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("volume_type")))) {
            std::shared_ptr<StreamVolumeInfo> streamVolInfo = std::make_shared<StreamVolumeInfo>();
            ParseStreamVolumeInfoAttr(currNode, streamVolInfo);
            ParseDeviceVolumeInfos(currNode->children, streamVolInfo);
            AUDIO_DEBUG_LOG("Parse streamType:%{public}d ", streamVolInfo->streamType);
            streamVolumeInfoMap[streamVolInfo->streamType] = streamVolInfo;
        }
        currNode = currNode->next;
    }
}

void AudioVolumeParser::ParseStreamVolumeInfoAttr(xmlNode *node, std::shared_ptr<StreamVolumeInfo> &streamVolInfo)
{
    xmlNode *currNode = node;
    AUDIO_DEBUG_LOG("AudioVolumeParser::ParseStreamVolumeInfoAttr");
    char *pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
        reinterpret_cast<xmlChar*>(const_cast<char*>("type"))));
    streamVolInfo->streamType = audioStreamMap_[pValue];
    AUDIO_DEBUG_LOG("stream type: %{public}s; currNode->name %{public}s;", pValue, currNode->name);
    xmlFree(pValue);

    pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
        reinterpret_cast<xmlChar*>(const_cast<char*>("minidx"))));
    streamVolInfo->minLevel = atoi(pValue);
    AUDIO_DEBUG_LOG("minidx: %{public}d", atoi(pValue));
    xmlFree(pValue);

    pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
        reinterpret_cast<xmlChar*>(const_cast<char*>("maxidx"))));
    streamVolInfo->maxLevel = atoi(pValue);
    AUDIO_DEBUG_LOG("minidx: %{public}d", atoi(pValue));
    xmlFree(pValue);

    pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
        reinterpret_cast<xmlChar*>(const_cast<char*>("defaultidx"))));
    streamVolInfo->defaultLevel = atoi(pValue);
    AUDIO_DEBUG_LOG("defaultidx: %{public}d", atoi(pValue));
    xmlFree(pValue);
}

void AudioVolumeParser::ParseDeviceVolumeInfos(xmlNode *node, std::shared_ptr<StreamVolumeInfo> &streamVolInfo)
{
    xmlNode *currNode = node;
    AUDIO_DEBUG_LOG("AudioVolumeParser::ParseDeviceVolumeInfos");
    while (currNode) {
        if (currNode->type == XML_ELEMENT_NODE
            && (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("volumecurve")))) {
            char *pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("deviceClass"))));
            std::shared_ptr<DeviceVolumeInfo> deviceVolInfo = std::make_shared<DeviceVolumeInfo>();
            AUDIO_DEBUG_LOG("deviceClass: %{public}s; currNode->name %{public}s;", pValue, currNode->name);
            deviceVolInfo->deviceType = audioDeviceMap_[pValue];
            AUDIO_DEBUG_LOG("deviceVolInfo->deviceType %{public}d;", deviceVolInfo->deviceType);
            xmlFree(pValue);
            ParseVolumePoints(currNode->children, deviceVolInfo);
            streamVolInfo->deviceVolumeInfos[deviceVolInfo->deviceType] = deviceVolInfo;
        }
        currNode = currNode->next;
    }
}

void AudioVolumeParser::ParseVolumePoints(xmlNode *node, std::shared_ptr<DeviceVolumeInfo> &deviceVolInfo)
{
    xmlNode *currNode = node;
    AUDIO_DEBUG_LOG("AudioVolumeParser::ParseVolumePoints");
    while (currNode) {
        if (currNode->type == XML_ELEMENT_NODE
            && (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("point")))) {
            struct VolumePoint volumePoint;
            char *pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("idx"))));
            volumePoint.index = static_cast<uint32_t>(atoi(pValue));
            AUDIO_DEBUG_LOG("idx: %{public}d", atoi(pValue));
            xmlFree(pValue);
            pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("decibel"))));
            volumePoint.dbValue = atoi(pValue);
            AUDIO_DEBUG_LOG("decibel: %{public}d", atoi(pValue));
            xmlFree(pValue);
            deviceVolInfo->volumePoints.push_back(volumePoint);
        }
        currNode = currNode->next;
    }
}
} // namespace AudioStandard
} // namespace OHOS
