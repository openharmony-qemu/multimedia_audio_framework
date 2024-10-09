/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#ifndef AUDIO_TONE_PARSER_H
#define AUDIO_TONE_PARSER_H

#include <map>
#include <string>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
    static constexpr char AUDIO_TONE_CONFIG_FILE[] = "system/etc/audio/audio_tone_dtmf_config.xml";

class AudioToneParser {
public:
    AudioToneParser();
    virtual ~AudioToneParser();
    int32_t LoadConfig(std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> &toneDescriptorMap);
private:
    void ParseSegment(xmlNode *node, int32_t segInx, std::shared_ptr<ToneInfo> ltoneDesc);
    void ParseToneInfoAttribute(xmlNode *sNode, std::shared_ptr<ToneInfo> ltoneDesc);
    void ParseToneInfo(xmlNode *node, std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> &toneDescriptorMap);
    void ParseFrequency(std::string freqList, ToneSegment &ltoneSeg);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_TONE_PARSER_H
