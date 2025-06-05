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
#define LOG_TAG "AudioUsageStrategyParser"
#endif

#include "audio_usage_strategy_parser.h"
#include "media_monitor_manager.h"
#include "audio_errors.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
bool AudioUsageStrategyParser::LoadConfiguration()
{
    curNode_ = AudioXmlNode::Create();
    int32_t ret = curNode_->Config(DEVICE_CONFIG_FILE, nullptr, 0);
    if (ret != SUCCESS) {
        Trace trace("SYSEVENT FAULT EVENT LOAD_CONFIG_ERROR, CATEGORY: "
            + std::to_string(Media::MediaMonitor::AUDIO_USAGE_STRATEGY));
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::AUDIO, Media::MediaMonitor::LOAD_CONFIG_ERROR,
            Media::MediaMonitor::FAULT_EVENT);
        bean->Add("CATEGORY", Media::MediaMonitor::AUDIO_USAGE_STRATEGY);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
        return false;
    }
    if (!ParseInternal(curNode_->GetCopyNode())) {
        return false;
    }
    return true;
}

void AudioUsageStrategyParser::Destroy()
{
    curNode_->FreeDoc();
}

bool AudioUsageStrategyParser::ParseInternal(std::shared_ptr<AudioXmlNode> curNode)
{
    for (; curNode->IsNodeValid(); curNode->MoveToNext()) {
        if (curNode->CompareName("adapter")) {
            std::string pValueStr;
            curNode->GetProp("name", pValueStr);
            if (pValueStr == "streamUsage") {
                ParserStreamUsageList(curNode->GetChildrenNode());
            } else if (pValueStr == "sourceType") {
                ParserSourceTypeList(curNode->GetChildrenNode());
            }
        } else {
            ParseInternal(curNode->GetChildrenNode());
        }
    }
    return true;
}

void AudioUsageStrategyParser::ParserStreamUsageList(std::shared_ptr<AudioXmlNode> curNode)
{
    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("strategy")) {
            std::string strategyName;
            std::string streamUsages;
            curNode->GetProp("name", strategyName);
            curNode->GetProp("streamUsage", streamUsages);
            ParserStreamUsageInfo(strategyName, streamUsages);
        }
        curNode->MoveToNext();
    }
}

void AudioUsageStrategyParser::ParserSourceTypeList(std::shared_ptr<AudioXmlNode> curNode)
{
    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("strategy")) {
            std::string strategyName;
            std::string sourceTypes;
            curNode->GetProp("name", strategyName);
            curNode->GetProp("sourceType", sourceTypes);
            ParserSourceTypeInfo(strategyName, sourceTypes);
        }
        curNode->MoveToNext();
    }
}

std::vector<std::string> AudioUsageStrategyParser::split(const std::string &line, const std::string &sep)
{
    std::vector<std::string> buf;
    size_t temp = 0;
    std::string::size_type pos = 0;
    while (true) {
        pos = line.find(sep, temp);
        if (pos == std::string::npos) {
            break;
        }
        buf.push_back(line.substr(temp, pos-temp));
        temp = pos + sep.length();
    }
    buf.push_back(line.substr(temp, line.length()));
    return buf;
}

void AudioUsageStrategyParser::ParserStreamUsage(const std::vector<std::string> &buf,
    const std::string &routerName)
{
    StreamUsage usage;
    for (auto &name : buf) {
        auto pos = streamUsageMap.find(name);
        if (pos != streamUsageMap.end()) {
            usage = pos->second;
        }
        renderConfigMap_[usage] = routerName;
    }
}

void AudioUsageStrategyParser::ParserStreamUsageInfo(const std::string &strategyName,
    const std::string &streamUsage)
{
    std::vector<std::string> buf = split(streamUsage, ",");
    if (strategyName == "MEDIA_RENDER") {
        ParserStreamUsage(buf, "MediaRenderRouters");
    } else if (strategyName == "CALL_RENDER") {
        ParserStreamUsage(buf, "CallRenderRouters");
    } else if (strategyName == "RING_RENDER") {
        ParserStreamUsage(buf, "RingRenderRouters");
    } else if (strategyName == "TONE_RENDER") {
        ParserStreamUsage(buf, "ToneRenderRouters");
    }
}

void AudioUsageStrategyParser::ParserSourceTypes(const std::vector<std::string> &buf,
    const std::string &sourceTypes)
{
    SourceType sourceType;
    for (auto &name : buf) {
        auto pos = sourceTypeMap.find(name);
        if (pos != sourceTypeMap.end()) {
            sourceType = pos->second;
        }
        capturerConfigMap_[sourceType] = sourceTypes;
    }
}

void AudioUsageStrategyParser::ParserSourceTypeInfo(const std::string &strategyName, const std::string &sourceTypes)
{
    std::vector<std::string> buf = split(sourceTypes, ",");
    if (strategyName == "RECORD_CAPTURE") {
        ParserSourceTypes(buf, "RecordCaptureRouters");
    } else if (strategyName == "CALL_CAPTURE") {
        ParserSourceTypes(buf, "CallCaptureRouters");
    } else if (strategyName == "VOICE_MESSAGE_CAPTURE") {
        ParserSourceTypes(buf, "VoiceMessages");
    }
}
} // namespace AudioStandard
} // namespace OHOS
