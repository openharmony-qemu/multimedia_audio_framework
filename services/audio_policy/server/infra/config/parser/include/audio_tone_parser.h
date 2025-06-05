/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include <vector>
#include <string>
#include <sstream>
#include <unordered_map>
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"
#include "audio_xml_parser.h"

namespace OHOS {
namespace AudioStandard {
using ToneInfoMap = std::unordered_map<int32_t, std::shared_ptr<ToneInfo>>;
class AudioToneParser {
public:
    static constexpr char AUDIO_TONE_CONFIG_FILE[] = "/system/etc/audio/audio_tone_dtmf_config.xml";
    AudioToneParser();
    virtual ~AudioToneParser();
    int32_t LoadConfig(ToneInfoMap &toneDescriptorMap);
    int32_t LoadNewConfig(const std::string &configPath, ToneInfoMap &toneDescriptorMap,
        std::unordered_map<std::string, ToneInfoMap> &customToneDescriptorMap);

private:
    void ParseSegment(std::shared_ptr<AudioXmlNode> curNode, int32_t segInx, std::shared_ptr<ToneInfo> ltoneDesc);
    void ParseToneInfoAttribute(std::shared_ptr<AudioXmlNode>, std::shared_ptr<ToneInfo> ltoneDesc);
    void ParseToneInfo(std::shared_ptr<AudioXmlNode>, std::vector<ToneInfoMap*> &toneDescriptorMaps);
    void ParseFrequency(std::string freqList, ToneSegment &ltoneSeg);
    void ParseCustom(std::shared_ptr<AudioXmlNode>,
        std::unordered_map<std::string, ToneInfoMap> &customToneDescriptorMap);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_TONE_PARSER_H
