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
#ifndef LOG_TAG
#define LOG_TAG "AudioToneParser"
#endif

#include "audio_tone_parser.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
AudioToneParser::AudioToneParser()
{
    AUDIO_INFO_LOG("AudioToneParser ctor");
}

AudioToneParser::~AudioToneParser()
{
}

std::string Trim(const std::string& str)
{
    size_t start = str.find_first_not_of(" \t\n\r");
    size_t end = str.find_last_not_of(" \t\n\r");
    if (start == std::string::npos || end == std::string::npos || start > end) {
        AUDIO_ERR_LOG("Failed to trim the string: %{public}s", str.c_str());
        return "";
    }

    std::string trimStr = str.substr(start, end - start + 1);
    for (char &c : trimStr) {
        c = std::tolower(c);
    }
    return trimStr;
}

std::vector<std::string> SplitAndTrim(const std::string& str)
{
    std::stringstream ss(str);
    std::string token;
    std::vector<std::string> result;

    while (std::getline(ss, token, ',')) {
        std::string trimmedToken = Trim(token);
        if (!trimmedToken.empty()) {
            AUDIO_DEBUG_LOG("Trim string : %{public}s", trimmedToken.c_str());
            result.push_back(trimmedToken);
        }
    }

    if (result.empty()) {
        AUDIO_ERR_LOG("Failed to split and trim the string: %{public}s", str.c_str());
    }

    return result;
}

int32_t AudioToneParser::LoadNewConfig(const std::string &configPath, ToneInfoMap &toneDescriptorMap,
    std::unordered_map<std::string, ToneInfoMap> &customToneDescriptorMap)
{
    AUDIO_INFO_LOG("Enter");
    std::shared_ptr<AudioXmlNode> curNode = AudioXmlNode::Create();
    CHECK_AND_RETURN_RET_LOG(curNode->Config(configPath.c_str(), nullptr, 0) == SUCCESS, ERROR,
        "error: could not parse file %{public}s", configPath.c_str());

    if (!curNode->CompareName("DTMF")) {
        AUDIO_ERR_LOG("Missing tag - DTMF: %{public}s", configPath.c_str());
        curNode = nullptr;
        return ERROR;
    }
    curNode->MoveToChildren();
    if (!curNode->IsNodeValid()) {
        AUDIO_ERR_LOG("Missing child - DTMF: %{public}s", configPath.c_str());
        curNode = nullptr;
        return ERROR;
    }

    while (curNode->IsNodeValid()) {
        if (!curNode->IsNodeValid()) {
            curNode->MoveToNext();
            continue;
        }
        if (curNode->CompareName("Default") || curNode->CompareName("Tones")) {
            std::vector<ToneInfoMap*> toneDescriptorMaps;
            toneDescriptorMaps.push_back(&toneDescriptorMap);
            ParseToneInfo(curNode->GetChildrenNode(), toneDescriptorMaps);
        } else if (curNode->CompareName("Custom")) {
            ParseCustom(curNode->GetChildrenNode(), customToneDescriptorMap);
        }
        curNode->MoveToNext();
    }
    curNode = nullptr;
    AUDIO_INFO_LOG("Done");
    return SUCCESS;
}

void AudioToneParser::ParseCustom(std::shared_ptr<AudioXmlNode> curNode,
    std::unordered_map<std::string, ToneInfoMap> &customToneDescriptorMap)
{
    AUDIO_DEBUG_LOG("Enter");
    while (curNode->IsNodeValid()) {
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        if (curNode->CompareName("CountryInfo")) {
            std::string pCountryName;
            int32_t ret = curNode->GetProp("names", pCountryName);
            if (ret != SUCCESS) {
                curNode->MoveToNext();
                continue;
            }
            AUDIO_DEBUG_LOG("ParseCustom names %{public}s", pCountryName.c_str());
            std::vector<ToneInfoMap*> toneDescriptorMaps;
            std::vector<std::string> cuntryNames = SplitAndTrim(pCountryName);
            for (auto &countryName : cuntryNames) {
                toneDescriptorMaps.push_back(&customToneDescriptorMap[countryName]);
            }
            ParseToneInfo(curNode->GetChildrenNode(), toneDescriptorMaps);
        }
        curNode->MoveToNext();
    }
}

int32_t AudioToneParser::LoadConfig(std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> &toneDescriptorMap)
{
    AUDIO_INFO_LOG("Enter");
    std::shared_ptr<AudioXmlNode> curNode = AudioXmlNode::Create();
    AUDIO_ERR_LOG("AudioToneParser::LoadConfig");

    CHECK_AND_RETURN_RET_LOG(curNode->Config(AUDIO_TONE_CONFIG_FILE, nullptr, 0) == SUCCESS, ERROR,
        "error: could not parse file %s", AUDIO_TONE_CONFIG_FILE);

    if (!curNode->CompareName("DTMF")) {
        AUDIO_ERR_LOG("Missing tag - DTMF: %s", AUDIO_TONE_CONFIG_FILE);
        curNode = nullptr;
        return ERROR;
    }
    curNode->MoveToChildren();
    if (!curNode->IsNodeValid()) {
        AUDIO_ERR_LOG("Missing child - DTMF: %s", AUDIO_TONE_CONFIG_FILE);
        curNode = nullptr;
        return ERROR;
    }
    while (curNode->IsNodeValid()) {
        if (curNode->CompareName("Tones")) {
            curNode->MoveToChildren();
        } else if (curNode->CompareName("ToneInfo")) {
            std::vector<ToneInfoMap*> toneDescriptorMaps;
            toneDescriptorMaps.push_back(&toneDescriptorMap);
            ParseToneInfo(curNode->GetCopyNode(), toneDescriptorMaps);
            break;
        } else {
            curNode->MoveToNext();
        }
    }
    curNode = nullptr;
    AUDIO_INFO_LOG("Done");
    return SUCCESS;
}

void AudioToneParser::ParseToneInfoAttribute(std::shared_ptr<AudioXmlNode> curNode,
    std::shared_ptr<ToneInfo> ltoneDesc)
{
    int segCnt = 0;
    int segInx = 0;
    while (curNode->IsNodeValid()) {
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        std::string pValueStr;
        if (curNode->CompareName("RepeatCount")) {
            AUDIO_DEBUG_LOG("RepeatCount node type: Element, name: RepeatCount");
            curNode->GetProp("value", pValueStr);

            if (pValueStr == "INF") {
                ltoneDesc->repeatCnt = TONEINFO_INF;
            } else {
                CHECK_AND_RETURN_LOG(StringConverter(pValueStr, ltoneDesc->repeatCnt),
                    "convert ltoneDesc->repeatCnt fail!");
            }
            AUDIO_DEBUG_LOG("ParseToneInfo repeatCnt %{public}d", ltoneDesc->repeatCnt);
        } else if (curNode->CompareName("RepeatSegment")) {
            AUDIO_DEBUG_LOG("RepeatSegment node type: Element, name: RepeatSegment");
            curNode->GetProp("value", pValueStr);
            CHECK_AND_RETURN_LOG(StringConverter(pValueStr, ltoneDesc->repeatSegment),
                "convert ltoneDesc->repeatSegment fail!");
            AUDIO_DEBUG_LOG("ParseToneInfo repeatSegment %{public}d", ltoneDesc->repeatSegment);
        } else if (curNode->CompareName("SegmentCount")) {
            AUDIO_DEBUG_LOG("SegmentCount node type: Element, name: SegmentCount");
            curNode->GetProp("value", pValueStr);
            CHECK_AND_RETURN_LOG(StringConverter(pValueStr, segCnt),
                "convert segCnt fail!");
            ltoneDesc->segmentCnt = static_cast<uint32_t>(segCnt);
            AUDIO_DEBUG_LOG("ParseToneInfo segmentCnt %{public}d", ltoneDesc->segmentCnt);
        } else if (curNode->CompareName("Segment")) {
            if (segInx < segCnt) {
                ParseSegment(curNode->GetCopyNode(), segInx, ltoneDesc);
                segInx++;
            }
        }
        curNode->MoveToNext();
    }
}

void AudioToneParser::ParseToneInfo(std::shared_ptr<AudioXmlNode> curNode,
    std::vector<ToneInfoMap*> &toneDescriptorMaps)
{
    while (curNode->IsNodeValid()) {
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        if (!curNode->CompareName("ToneInfo")) {
            curNode->MoveToNext();
            continue;
        }
        std::shared_ptr<ToneInfo> ltoneDesc = std::make_shared<ToneInfo>();
        std::string pToneType;
        if (curNode->GetProp("toneType", pToneType) != SUCCESS) {
            AUDIO_DEBUG_LOG("getprop toneType fail");
            curNode->MoveToNext();
            continue;
        }
        int32_t toneType = 0;
        CHECK_AND_RETURN_LOG(StringConverter(pToneType, toneType),
            "convert pToneType: %{public}s Fail!", pToneType.c_str());
        AUDIO_DEBUG_LOG("toneType value: %{public}d", toneType);
        
        ParseToneInfoAttribute(curNode->GetChildrenNode(), ltoneDesc);

        for (auto toneDescriptorMap : toneDescriptorMaps) {
            if (toneDescriptorMap) {
                (*toneDescriptorMap)[toneType] = ltoneDesc;
            }
        }
        curNode->MoveToNext();
    }
}

void AudioToneParser::ParseSegment(std::shared_ptr<AudioXmlNode> curNode,
    int SegInx, std::shared_ptr<ToneInfo> ltoneDesc)
{
    for (uint32_t i = 0; i < TONEINFO_MAX_WAVES + 1; i++) {
        ltoneDesc->segments[SegInx].waveFreq[i]=0;
    }
    if (curNode->CompareName("Segment")) {
        std::string pValueStr;
        curNode->GetProp("duration", pValueStr);
        if (pValueStr == "INF") {
            ltoneDesc->segments[SegInx].duration = TONEINFO_INF;
        } else {
            CHECK_AND_RETURN_LOG(StringConverter(pValueStr, ltoneDesc->segments[SegInx].duration),
                "convert ltoneDesc->segments[SegInx].duration fail!");
        }
        AUDIO_DEBUG_LOG("duration: %{public}d", ltoneDesc->segments[SegInx].duration);

        curNode->GetProp("loopCount", pValueStr);
        CHECK_AND_RETURN_LOG(StringConverter(pValueStr, ltoneDesc->segments[SegInx].loopCnt),
            "convert ltoneDesc->segments[SegInx].loopCnt fail!");
        AUDIO_DEBUG_LOG("loopCnt: %{public}d", ltoneDesc->segments[SegInx].loopCnt);

        curNode->GetProp("loopIndex", pValueStr);
        CHECK_AND_RETURN_LOG(StringConverter(pValueStr, ltoneDesc->segments[SegInx].loopIndx),
            "convert ltoneDesc->segments[SegInx].loopIndx fail!");
        AUDIO_DEBUG_LOG("loopIndx: %{public}d", ltoneDesc->segments[SegInx].loopIndx);

        curNode->GetProp("freq", pValueStr);
        ParseFrequency(pValueStr, ltoneDesc->segments[SegInx]);
    }
}

void AudioToneParser::ParseFrequency(std::string freqList, ToneSegment &ltonesegment)
{
    std::vector<int> vect;
    std::stringstream ss(freqList);

    for (int i; ss >> i;) {
        vect.push_back(i);
        if (ss.peek() == ',') {
            ss.ignore();
        }
    }

    for (std::size_t i = 0; i < vect.size(); i++) {
        AUDIO_DEBUG_LOG("Freq: %{public}d", vect[i]);
        ltonesegment.waveFreq[i] = vect[i];
    }
}
} // namespace AudioStandard
} // namespace OHOS
