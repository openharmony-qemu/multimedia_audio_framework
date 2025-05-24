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
#include "audio_utils.h"
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
    {"STREAM_CAMCORDER",
        {AudioStreamType::STREAM_CAMCORDER, SourceType::SOURCE_TYPE_INVALID, true}},
    // source type for audio interrupt
    {"SOURCE_TYPE_MIC",
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_MIC, false}},
    {"SOURCE_TYPE_CAMCORDER",
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_CAMCORDER, false}},
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
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_VOICE_TRANSCRIPTION, false}},
    {"SOURCE_TYPE_UNPROCESSED",
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_UNPROCESSED, false}},
    {"SOURCE_TYPE_LIVE",
        {AudioStreamType::STREAM_DEFAULT, SourceType::SOURCE_TYPE_LIVE, false}},
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
    curNode_ = AudioXmlNode::Create();
    AUDIO_DEBUG_LOG("AudioFocusParser ctor");
}

AudioFocusParser::~AudioFocusParser()
{
    curNode_ = nullptr;
    AUDIO_DEBUG_LOG("AudioFocusParser dtor");
}

void AudioFocusParser::LoadDefaultConfig(std::map<std::pair<AudioFocusType, AudioFocusType>,
    AudioFocusEntry> &focusMap)
{
}

int32_t AudioFocusParser::LoadConfig(std::map<std::pair<AudioFocusType, AudioFocusType>,
    AudioFocusEntry> &focusMap)
{
#ifdef USE_CONFIG_POLICY
    char buf[MAX_PATH_LEN];
    char *path = GetOneCfgFile(AUDIO_FOCUS_CONFIG_FILE, buf, MAX_PATH_LEN);
#else
    const char *path = AUDIO_FOCUS_CONFIG_FILE;
#endif
    CHECK_AND_RETURN_RET_LOG(path != nullptr && *path != '\0', ERROR, "invalid path!");
    if (curNode_->Config(path, nullptr, 0) != SUCCESS) {
        AUDIO_ERR_LOG("load path: %{public}s fail!", path);
        LoadDefaultConfig(focusMap);
        WriteConfigErrorEvent();
        return ERROR;
    }
    CHECK_AND_RETURN_RET_LOG(curNode_->IsNodeValid(), ERROR, "root element is null");

    if (!curNode_->CompareName("audio_focus_policy")) {
        AUDIO_ERR_LOG("Missing tag - focus_policy in : %s", AUDIO_FOCUS_CONFIG_FILE);
        WriteConfigErrorEvent();
        curNode_ = nullptr;
        return ERROR;
    }

    curNode_->MoveToChildren();
    CHECK_AND_RETURN_RET_LOG(curNode_->IsNodeValid(), ERROR, "Missing child: %s", AUDIO_FOCUS_CONFIG_FILE);

    while (curNode_->IsNodeValid()) {
        if (curNode_->CompareName("focus_type")) {
            ParseStreams(curNode_->GetCopyNode(), focusMap);
            break;
        } else {
            curNode_->MoveToNext();
        }
    }
    curNode_ = nullptr;
    return SUCCESS;
}

void AudioFocusParser::WriteConfigErrorEvent()
{
    Trace trace("SYSEVENT FAULT EVENT LOAD_CONFIG_ERROR, CATEGORY: "
        + std::to_string(Media::MediaMonitor::AUDIO_INTERRUPT_POLICY_CONFIG));
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::LOAD_CONFIG_ERROR, Media::MediaMonitor::FAULT_EVENT);
    bean->Add("CATEGORY", Media::MediaMonitor::AUDIO_INTERRUPT_POLICY_CONFIG);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

void AudioFocusParser::ParseFocusChildrenMap(std::shared_ptr<AudioXmlNode> curNode, const std::string &curStream,
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap)
{
    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            if (curNode->CompareName("deny")) {
                ParseRejectedStreams(curNode->GetChildrenNode(), curStream, focusMap);
            } else {
                ParseAllowedStreams(curNode->GetChildrenNode(), curStream, focusMap);
            }
        }
        curNode->MoveToNext();
    }
}

void AudioFocusParser::ParseFocusMap(std::shared_ptr<AudioXmlNode> curNode, const std::string &curStream,
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap)
{
    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("focus_table")) {
            AUDIO_DEBUG_LOG("node type: Element, name: %s", curNode->GetName().c_str());
            ParseFocusChildrenMap(curNode->GetChildrenNode(), curStream, focusMap);
        }
        curNode->MoveToNext();
    }
}

void AudioFocusParser::ParseStreams(std::shared_ptr<AudioXmlNode> curNode,
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap)
{
    while (curNode->IsNodeValid()) {
        if (curNode->IsElementNode()) {
            std::string typeStr;
            curNode->GetProp("value", typeStr);
            std::map<std::string, AudioFocusType>::iterator it = audioFocusMap.find(typeStr);
            if (it != audioFocusMap.end()) {
                AUDIO_DEBUG_LOG("stream type: %{public}s",  typeStr.c_str());
                ParseFocusMap(curNode->GetChildrenNode(), typeStr, focusMap);
            }
        }
        curNode->MoveToNext();
    }
}

void AudioFocusParser::AddRejectedFocusEntry(std::shared_ptr<AudioXmlNode> curNode, const std::string &curStream,
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap)
{
    std::string newStreamStr;
    curNode->GetProp("value", newStreamStr);
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
}

void AudioFocusParser::ParseRejectedStreams(std::shared_ptr<AudioXmlNode> curNode, const std::string &curStream,
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap)
{
    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("focus_type")) {
            AddRejectedFocusEntry(curNode->GetCopyNode(), curStream, focusMap);
        }
        curNode->MoveToNext();
    }
}

void AudioFocusParser::AddAllowedFocusEntry(std::shared_ptr<AudioXmlNode> curNode, const std::string &curStream,
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap)
{
    std::string newStreamStr;
    std::string aTargetStr;
    std::string aTypeStr;
    std::string isForcedStr;
    curNode->GetProp("value", newStreamStr);
    curNode->GetProp("action_on", aTargetStr);
    curNode->GetProp("action_type", aTypeStr);
    curNode->GetProp("is_forced", isForcedStr);

    std::map<std::string, AudioFocusType>::iterator it1 = audioFocusMap.find(newStreamStr);
    std::map<std::string, ActionTarget>::iterator it2 = targetMap.find(aTargetStr);
    std::map<std::string, InterruptHint>::iterator it3 = actionMap.find(aTypeStr);
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
}

void AudioFocusParser::ParseAllowedStreams(std::shared_ptr<AudioXmlNode> curNode, const std::string &curStream,
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> &focusMap)
{
    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("focus_type")) {
            AddAllowedFocusEntry(curNode->GetCopyNode(), curStream, focusMap);
        }
        curNode->MoveToNext();
    }
}
} // namespace AudioStandard
} // namespace OHOS
