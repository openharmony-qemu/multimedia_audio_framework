/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_USAGE_STRATEGY_PARSER_H
#define AUDIO_USAGE_STRATEGY_PARSER_H

#include <list>
#include <unordered_map>
#include <string>
#include <sstream>

#include "audio_policy_log.h"
#include "iport_observer.h"
#include "parser.h"
#include "audio_xml_parser.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

class AudioUsageStrategyParser : public Parser {
public:
    static constexpr char DEVICE_CONFIG_FILE[] = "/system/etc/audio/audio_usage_strategy.xml";

    bool LoadConfiguration() final;
    void Destroy() final;

    AudioUsageStrategyParser()
    {
        curNode_ = AudioXmlNode::Create();
        AUDIO_DEBUG_LOG("AudioUsageStrategyParser ctor");
    }

    virtual ~AudioUsageStrategyParser()
    {
        AUDIO_DEBUG_LOG("AudioUsageStrategyParser dtor");
        Destroy();
        curNode_ = nullptr;
    }

    std::unordered_map<StreamUsage, std::string> renderConfigMap_;
    std::unordered_map<SourceType, std::string> capturerConfigMap_;

private:
    bool ParseInternal(std::shared_ptr<AudioXmlNode> curNode);
    void ParserStreamUsageList(std::shared_ptr<AudioXmlNode> curNode);
    void ParserSourceTypeList(std::shared_ptr<AudioXmlNode> curNode);
    void ParserStreamUsageInfo(const std::string &strategyName, const std::string &streamUsage);
    void ParserStreamUsage(const std::vector<std::string> &buf, const std::string &routerName);
    void ParserSourceTypeInfo(const std::string &sourceType, const std::string &nameSourceType);
    void ParserSourceTypes(const std::vector<std::string> &buf, const std::string &sourceTypes);

    std::vector<std::string> split(const std::string &line, const std::string &sep);

    const unordered_map<string, StreamUsage> streamUsageMap = {
        {"STREAM_USAGE_UNKNOWN", STREAM_USAGE_UNKNOWN},
        {"STREAM_USAGE_MEDIA", STREAM_USAGE_MEDIA},
        {"STREAM_USAGE_MUSIC", STREAM_USAGE_MUSIC},
        {"STREAM_USAGE_VOICE_COMMUNICATION", STREAM_USAGE_VOICE_COMMUNICATION},
        {"STREAM_USAGE_VOICE_ASSISTANT", STREAM_USAGE_VOICE_ASSISTANT},
        {"STREAM_USAGE_VOICE_CALL_ASSISTANT", STREAM_USAGE_VOICE_CALL_ASSISTANT},
        {"STREAM_USAGE_ALARM", STREAM_USAGE_ALARM},
        {"STREAM_USAGE_VOICE_MESSAGE", STREAM_USAGE_VOICE_MESSAGE},
        {"STREAM_USAGE_NOTIFICATION_RINGTONE", STREAM_USAGE_NOTIFICATION_RINGTONE},
        {"STREAM_USAGE_RINGTONE", STREAM_USAGE_RINGTONE},
        {"STREAM_USAGE_NOTIFICATION", STREAM_USAGE_NOTIFICATION},
        {"STREAM_USAGE_ACCESSIBILITY", STREAM_USAGE_ACCESSIBILITY},
        {"STREAM_USAGE_SYSTEM", STREAM_USAGE_SYSTEM},
        {"STREAM_USAGE_MOVIE", STREAM_USAGE_MOVIE},
        {"STREAM_USAGE_GAME", STREAM_USAGE_GAME},
        {"STREAM_USAGE_AUDIOBOOK", STREAM_USAGE_AUDIOBOOK},
        {"STREAM_USAGE_NAVIGATION", STREAM_USAGE_NAVIGATION},
        {"STREAM_USAGE_DTMF", STREAM_USAGE_DTMF},
        {"STREAM_USAGE_ENFORCED_TONE", STREAM_USAGE_ENFORCED_TONE},
        {"STREAM_USAGE_ULTRASONIC", STREAM_USAGE_ULTRASONIC},
        {"STREAM_USAGE_VIDEO_COMMUNICATION", STREAM_USAGE_VIDEO_COMMUNICATION},
        {"STREAM_USAGE_RANGING", STREAM_USAGE_RANGING},
        {"STREAM_USAGE_VOICE_MODEM_COMMUNICATION", STREAM_USAGE_VOICE_MODEM_COMMUNICATION},
        {"STREAM_USAGE_VOICE_RINGTONE", STREAM_USAGE_VOICE_RINGTONE},
    };

    const unordered_map<string, SourceType> sourceTypeMap = {
        {"SOURCE_TYPE_MIC", SOURCE_TYPE_MIC},
        {"SOURCE_TYPE_CAMCORDER", SOURCE_TYPE_CAMCORDER},
        {"SOURCE_TYPE_VOICE_RECOGNITION", SOURCE_TYPE_VOICE_RECOGNITION},
        {"SOURCE_TYPE_PLAYBACK_CAPTURE", SOURCE_TYPE_PLAYBACK_CAPTURE},
        {"SOURCE_TYPE_WAKEUP", SOURCE_TYPE_WAKEUP},
        {"SOURCE_TYPE_VOICE_COMMUNICATION", SOURCE_TYPE_VOICE_COMMUNICATION},
        {"SOURCE_TYPE_VOICE_CALL", SOURCE_TYPE_VOICE_CALL},
        {"SOURCE_TYPE_ULTRASONIC", SOURCE_TYPE_ULTRASONIC},
        {"SOURCE_TYPE_VIRTUAL_CAPTURE", SOURCE_TYPE_VIRTUAL_CAPTURE},
        {"SOURCE_TYPE_VOICE_MESSAGE", SOURCE_TYPE_VOICE_MESSAGE},
        {"SOURCE_TYPE_VOICE_TRANSCRIPTION", SOURCE_TYPE_VOICE_TRANSCRIPTION},
        {"SOURCE_TYPE_UNPROCESSED", SOURCE_TYPE_UNPROCESSED},
    };

    std::shared_ptr<AudioXmlNode> curNode_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_USAGE_STRATEGY_PARSER_H
