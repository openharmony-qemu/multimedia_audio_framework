/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioFocusParser"
#endif

#include "audio_focus_parser.h"
#ifdef USE_CONFIG_POLICY
#include "config_policy_utils.h"
#endif

#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {

// Initialize stream map with string vs AudioStreamType
std::map<std::string, AudioFocusType> AudioFocusParser::audioFocusMap = {
    // stream type for audio interrupt
    {"STREAM_VOICE_CALL",
        {AudioStreamType::STREAM_VOICE_CALL, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_VOICE_CALL_ASSISTANT",
        {AudioStreamType::STREAM_VOICE_CALL_ASSISTANT, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_VOICE_MESSAGE",
        {AudioStreamType::STREAM_VOICE_MESSAGE, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_SYSTEM",
        {AudioStreamType::STREAM_SYSTEM, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_RING",
        {AudioStreamType::STREAM_RING, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_MUSIC",
        {AudioStreamType::STREAM_MUSIC, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_MOVIE",
        {AudioStreamType::STREAM_MOVIE, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_GAME",
        {AudioStreamType::STREAM_GAME, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_SPEECH",
        {AudioStreamType::STREAM_SPEECH, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_NAVIGATION",
        {AudioStreamType::STREAM_NAVIGATION, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_ALARM",
        {AudioStreamType::STREAM_ALARM, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_NOTIFICATION",
        {AudioStreamType::STREAM_NOTIFICATION, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_SYSTEM_ENFORCED",
        {AudioStreamType::STREAM_SYSTEM_ENFORCED, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_DTMF",
        {AudioStreamType::STREAM_DTMF, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_VOICE_ASSISTANT",
        {AudioStreamType::STREAM_VOICE_ASSISTANT, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_ACCESSIBILITY",
        {AudioStreamType::STREAM_ACCESSIBILITY, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_ULTRASONIC",
        {AudioStreamType::STREAM_ULTRASONIC, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_INTERNAL_FORCE_STOP",
        {AudioStreamType::STREAM_INTERNAL_FORCE_STOP, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_VOICE_COMMUNICATION",
        {AudioStreamType::STREAM_VOICE_COMMUNICATION, SourceType::SOURCE_TYPE_INVALID, true}},
    {"STREAM_VOICE_RING",
        {AudioStreamType::STREAM_VOICE_RING, SourceType::SOURCE_TYPE_INVALID, true}},
    // source type for audio interrupt
    {"SOURCE_TYPE_MIC",
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_MIC, false}},
    {"SOURCE_TYPE_VOICE_RECOGNITION",
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_VOICE_RECOGNITION, false}},
    {"SOURCE_TYPE_WAKEUP",
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_WAKEUP, false}},
    {"SOURCE_TYPE_VOICE_COMMUNICATION",
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_VOICE_COMMUNICATION, false}},
    {"SOURCE_TYPE_ULTRASONIC",
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_ULTRASONIC, false}},
    {"SOURCE_TYPE_PLAYBACK_CAPTURE",
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_PLAYBACK_CAPTURE, false}},
    {"SOURCE_TYPE_VOICE_CALL",
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_VOICE_CALL, false}},
    {"SOURCE_TYPE_VOICE_MESSAGE",
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_VOICE_MESSAGE, false}},
    {"SOURCE_TYPE_REMOTE_CAST",
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_REMOTE_CAST, false}},
    {"SOURCE_TYPE_VOICE_TRANSCRIPTION",
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_VOICE_TRANSCRIPTION, false}}
};

// Initialize action map with string vs InterruptActionType
std::map<std::string, InterruptHint> AudioFocusParser::actionMap = {
    {"DUCK", INTERRUPT_HINT_DUCK},
    {"PAUSE", INTERRUPT_HINT_PAUSE},
    {"REJECT", INTERRUPT_HINT_STOP},
    {"STOP", INTERRUPT_HINT_STOP},
    {"PLAY", INTERRUPT_HINT_NONE}
};

// Initialize target map with string vs InterruptActionTarget
std::map<std::string, ActionTarget> AudioFocusParser::targetMap = {
    {"incoming", INCOMING},
    {"existing", CURRENT},
    {"both", BOTH},
};

std::map<std::string, InterruptForceType> AudioFocusParser::forceMap = {
    {"true", INTERRUPT_FORCE},
    {"false", INTERRUPT_SHARE},
};

AudioFocusParser::AudioFocusParser()
{
    AUDIO_DEBUG_LOG("AudioFocusParser ctor");
}

AudioFocusParser::~AudioFocusParser()
{
    AUDIO_DEBUG_LOG("AudioFocusParser dtor");
}

void AudioFocusParser::LoadDefaultConfig(std::map<std::pair<AudioFocusType, AudioFocusType>,
    AudioFocusEntry> &focusMap)
{
}

int32_t AudioFocusParser::LoadConfig(std::map<std::pair<AudioFocusType, AudioFocusType>,
    AudioFocusEntry> &focusMap)
{
    xmlDoc *doc = nullptr;
    xmlNode *rootElement = nullptr;
#ifdef USE_CONFIG_POLICY
    char buf[MAX_PATH_LEN];
    char *path = GetOneCfgFile(AUDIO_FOCUS_CONFIG_FILE, buf, MAX_PATH_LEN);
#else
    const char *path = AUDIO_FOCUS_CONFIG_FILE;
#endif
    if (path != nullptr && *path != '\0') {
        doc = xmlReadFile(path, nullptr, 0);
    }
    if (doc == nullptr) {
        AUDIO_ERR_LOG("error: could not parse audio_interrupt_policy_config.xml");
        LoadDefaultConfig(focusMap);
        WriteConfigErrorEvent();
        return ERROR;
    }
    rootElement = xmlDocGetRootElement(doc);
    xmlNode *currNode = rootElement;
    CHECK_AND_RETURN_RET_LOG(currNode != nullptr, ERROR, "root element is null");
    if (xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("audio_focus_policy"))) {
        AUDIO_ERR_LOG("Missing tag - focus_policy in : %s", AUDIO_FOCUS_CONFIG_FILE);
        WriteConfigErrorEvent();
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return ERROR;
    }
    if (currNode->children) {
        currNode = currNode->children;
    } else {
        AUDIO_ERR_LOG("Missing child: %s", AUDIO_FOCUS_CONFIG_FILE);
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return ERROR;
    }
    while (currNode != nullptr) {
        if ((currNode->type == XML_ELEMENT_NODE) &&
            (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("focus_type")))) {
            ParseStreams(currNode, focusMap);
            break;
        } else {
            currNode = currNode->next;
        }
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return SUCCESS;
}

void AudioFocusParser::WriteConfigErrorEvent()
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::LOAD_CONFIG_ERROR, Media::MediaMonitor::FAULT_EVENT);
    bean->Add("CATEGORY", Media::MediaMonitor::AUDIO_INTERRUPT_POLICY_CONFIG);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioFocusParser::ParseFocusChildrenMap(xmlNode *node, const std::string &curStream,
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap)
{
    xmlNode *sNode = node;
    while (sNode) {
        if (sNode->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(sNode->name, reinterpret_cast<const xmlChar*>("deny"))) {
                ParseRejectedStreams(sNode->children, curStream, focusMap);
            } else {
                ParseAllowedStreams(sNode->children, curStream, focusMap);
            }
        }
        sNode = sNode->next;
    }
}

void AudioFocusParser::ParseFocusMap(xmlNode *node, const std::string &curStream,
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap)
{
    xmlNode *currNode = node;
    while (currNode != nullptr) {
        if (currNode->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("focus_table"))) {
                AUDIO_DEBUG_LOG("node type: Element, name: %s", currNode->name);
                ParseFocusChildrenMap(currNode->children, curStream, focusMap);
            }
        }
        currNode = currNode->next;
    }
}

void AudioFocusParser::ParseStreams(xmlNode *node,
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap)
{
    xmlNode *currNode = node;
    while (currNode) {
        if (currNode->type == XML_ELEMENT_NODE) {
            char *sType = reinterpret_cast<char*>(xmlGetProp(currNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("value"))));
            std::string typeStr(sType);
            std::map<std::string, AudioFocusType>::iterator it = audioFocusMap.find(typeStr);
            if (it != audioFocusMap.end()) {
                AUDIO_DEBUG_LOG("stream type: %{public}s",  sType);
                ParseFocusMap(currNode->children, typeStr, focusMap);
            }
            xmlFree(sType);
        }
        currNode = currNode->next;
    }
}

void AudioFocusParser::AddRejectedFocusEntry(xmlNode *currNode, const std::string &curStream,
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap)
{
    char *newStream = reinterpret_cast<char*>(xmlGetProp(currNode,
        reinterpret_cast<xmlChar*>(const_cast<char*>("value"))));

    std::string newStreamStr(newStream);
    std::map<std::string, AudioFocusType>::iterator it1 = audioFocusMap.find(newStreamStr);
    if (it1 != audioFocusMap.end()) {
        std::pair<AudioFocusType, AudioFocusType> rejectedStreamsPair =
            std::make_pair(audioFocusMap[curStream], audioFocusMap[newStreamStr]);
        AudioFocusEntry rejectedFocusEntry;
        rejectedFocusEntry.actionOn = INCOMING;
        rejectedFocusEntry.hintType = INTERRUPT_HINT_STOP;
        rejectedFocusEntry.forceType = INTERRUPT_FORCE;
        rejectedFocusEntry.isReject = true;
        focusMap.emplace(rejectedStreamsPair, rejectedFocusEntry);

        AUDIO_DEBUG_LOG("current stream: %s, incoming stream: %s", curStream.c_str(), newStreamStr.c_str());
        AUDIO_DEBUG_LOG("actionOn: %d, hintType: %d, forceType: %d isReject: %d",
            rejectedFocusEntry.actionOn, rejectedFocusEntry.hintType,
            rejectedFocusEntry.forceType, rejectedFocusEntry.isReject);
    }
    xmlFree(newStream);
}

void AudioFocusParser::ParseRejectedStreams(xmlNode *node, const std::string &curStream,
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap)
{
    xmlNode *currNode = node;

    while (currNode) {
        if (currNode->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("focus_type"))) {
                AddRejectedFocusEntry(currNode, curStream, focusMap);
            }
        }
        currNode = currNode->next;
    }
}

void AudioFocusParser::AddAllowedFocusEntry(xmlNode *currNode, const std::string &curStream,
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap)
{
    char *newStream = reinterpret_cast<char*>(xmlGetProp(currNode,
        reinterpret_cast<xmlChar*>(const_cast<char*>("value"))));
    char *aType = reinterpret_cast<char*>(xmlGetProp(currNode,
        reinterpret_cast<xmlChar*>(const_cast<char*>("action_type"))));
    char *aTarget = reinterpret_cast<char*>(xmlGetProp(currNode,
        reinterpret_cast<xmlChar*>(const_cast<char*>("action_on"))));
    char *isForced = reinterpret_cast<char*>(xmlGetProp(currNode,
        reinterpret_cast<xmlChar*>(const_cast<char*>("is_forced"))));

    std::string newStreamStr(newStream);
    std::map<std::string, AudioFocusType>::iterator it1 = audioFocusMap.find(newStreamStr);
    std::string aTargetStr(aTarget);
    std::map<std::string, ActionTarget>::iterator it2 = targetMap.find(aTargetStr);
    std::string aTypeStr(aType);
    std::map<std::string, InterruptHint>::iterator it3 = actionMap.find(aTypeStr);
    std::string isForcedStr(isForced);
    std::map<std::string, InterruptForceType>::iterator it4 = forceMap.find(isForcedStr);
    if ((it1 != audioFocusMap.end()) && (it2 != targetMap.end()) && (it3 != actionMap.end()) &&
        (it4 != forceMap.end())) {
        std::pair<AudioFocusType, AudioFocusType> allowedStreamsPair =
            std::make_pair(audioFocusMap[curStream], audioFocusMap[newStreamStr]);
        AudioFocusEntry allowedFocusEntry;
        allowedFocusEntry.actionOn = targetMap[aTargetStr];
        allowedFocusEntry.hintType = actionMap[aTypeStr];
        allowedFocusEntry.forceType = forceMap[isForcedStr];
        allowedFocusEntry.isReject = false;
        focusMap.emplace(allowedStreamsPair, allowedFocusEntry);

        AUDIO_DEBUG_LOG("current stream: %s, incoming stream: %s", curStream.c_str(), newStreamStr.c_str());
        AUDIO_DEBUG_LOG("actionOn: %d, hintType: %d, forceType: %d isReject: %d",
            allowedFocusEntry.actionOn, allowedFocusEntry.hintType,
            allowedFocusEntry.forceType, allowedFocusEntry.isReject);
    }
    xmlFree(newStream);
    xmlFree(aType);
    xmlFree(aTarget);
    xmlFree(isForced);
}

void AudioFocusParser::ParseAllowedStreams(xmlNode *node, const std::string &curStream,
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap)
{
    xmlNode *currNode = node;

    while (currNode) {
        if (currNode->type == XML_ELEMENT_NODE) {
            if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("focus_type"))) {
                AddAllowedFocusEntry(currNode, curStream, focusMap);
            }
        }
        currNode = currNode->next;
    }
}
} // namespace AudioStandard
} // namespace OHOS
