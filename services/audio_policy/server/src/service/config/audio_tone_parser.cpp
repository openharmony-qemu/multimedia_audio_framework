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
    xmlDoc *doc = xmlReadFile(configPath.c_str(), nullptr, 0);
    CHECK_AND_RETURN_RET_LOG(doc != nullptr, ERROR, "error: could not parse file %{public}s", configPath.c_str());
    xmlNode *rootElement = xmlDocGetRootElement(doc);
    xmlNode *currNode = rootElement;
    CHECK_AND_RETURN_RET_LOG(currNode != nullptr, ERROR, "root element is null");

    if (xmlStrcmp(currNode->name, BAD_CAST"DTMF")) {
        AUDIO_ERR_LOG("Missing tag - DTMF: %{public}s", configPath.c_str());
        xmlFreeDoc(doc);
        return ERROR;
    }
    if (currNode->xmlChildrenNode == nullptr) {
        AUDIO_ERR_LOG("Missing child - DTMF: %{public}s", configPath.c_str());
        xmlFreeDoc(doc);
        return ERROR;
    }
    currNode = currNode->xmlChildrenNode;

    while (currNode != nullptr) {
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, BAD_CAST"Default") ||
            !xmlStrcmp(currNode->name, BAD_CAST"Tones")) {
            std::vector<ToneInfoMap*> toneDescriptorMaps;
            toneDescriptorMaps.push_back(&toneDescriptorMap);
            ParseToneInfo(currNode->xmlChildrenNode, toneDescriptorMaps);
        } else if (!xmlStrcmp(currNode->name, BAD_CAST"Custom")) {
            ParseCustom(currNode->xmlChildrenNode, customToneDescriptorMap);
        }
        currNode = currNode->next;
    }

    xmlFreeDoc(doc);
    AUDIO_INFO_LOG("Done");
    return SUCCESS;
}

void AudioToneParser::ParseCustom(xmlNode *node, std::unordered_map<std::string, ToneInfoMap> &customToneDescriptorMap)
{
    xmlNode *currNode = node;
    AUDIO_DEBUG_LOG("Enter");
    while (currNode != nullptr) {
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, BAD_CAST"CountryInfo")) {
            char *pCountryName = reinterpret_cast<char*>(xmlGetProp(currNode, BAD_CAST"names"));
            if (pCountryName == nullptr) {
                currNode = currNode->next;
                continue;
            }
            AUDIO_DEBUG_LOG("ParseCustom names %{public}s", pCountryName);
            std::vector<ToneInfoMap*> toneDescriptorMaps;
            std::vector<std::string> cuntryNames = SplitAndTrim(pCountryName);
            for (auto &countryName : cuntryNames) {
                toneDescriptorMaps.push_back(&customToneDescriptorMap[countryName]);
            }
            ParseToneInfo(currNode->xmlChildrenNode, toneDescriptorMaps);
            xmlFree(pCountryName);
        }
        currNode = currNode->next;
    }
}

int32_t AudioToneParser::LoadConfig(std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> &toneDescriptorMap)
{
    AUDIO_INFO_LOG("Enter");
    xmlDoc *doc = nullptr;
    xmlNode *rootElement = nullptr;
    AUDIO_ERR_LOG("AudioToneParser::LoadConfig");
    doc = xmlReadFile(AUDIO_TONE_CONFIG_FILE, nullptr, 0);
    CHECK_AND_RETURN_RET_LOG(doc != nullptr, ERROR, "error: could not parse file %s", AUDIO_TONE_CONFIG_FILE);
    rootElement = xmlDocGetRootElement(doc);
    xmlNode *currNode = rootElement;
    CHECK_AND_RETURN_RET_LOG(currNode != nullptr, ERROR, "root element is null");
    if (xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("DTMF"))) {
        AUDIO_ERR_LOG("Missing tag - DTMF: %s", AUDIO_TONE_CONFIG_FILE);
        xmlFreeDoc(doc);
        return ERROR;
    }
    if (currNode->xmlChildrenNode) {
        currNode = currNode->xmlChildrenNode;
    } else {
        AUDIO_ERR_LOG("Missing child - DTMF: %s", AUDIO_TONE_CONFIG_FILE);
        xmlFreeDoc(doc);
        return ERROR;
    }

    while (currNode != nullptr) {
        if ((currNode->type == XML_ELEMENT_NODE) &&
            (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("Tones")))) {
            currNode = currNode->xmlChildrenNode;
        } else if ((currNode->type == XML_ELEMENT_NODE) &&
            (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("ToneInfo")))) {
            std::vector<ToneInfoMap*> toneDescriptorMaps;
            toneDescriptorMaps.push_back(&toneDescriptorMap);
            ParseToneInfo(currNode, toneDescriptorMaps);
            break;
        } else {
            currNode = currNode->next;
        }
    }
    if (currNode == nullptr) {
        AUDIO_WARNING_LOG("Missing tag - Tones, ToneInfo: %s", AUDIO_TONE_CONFIG_FILE);
    }
    xmlFreeDoc(doc);
    AUDIO_INFO_LOG("Done");
    return SUCCESS;
}

void AudioToneParser::ParseToneInfoAttribute(xmlNode *sNode, std::shared_ptr<ToneInfo> ltoneDesc)
{
    int segCnt = 0;
    int segInx = 0;
    while (sNode != nullptr) {
        if (sNode->type != XML_ELEMENT_NODE) {
            sNode = sNode->next;
            continue;
        }
        char *pValue = nullptr;
        if (!xmlStrcmp(sNode->name, reinterpret_cast<const xmlChar*>("RepeatCount"))) {
            AUDIO_DEBUG_LOG("RepeatCount node type: Element, name: %{public}s", sNode->name);
            pValue = reinterpret_cast<char*>(xmlGetProp(sNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("value"))));
            if (!xmlStrcmp(reinterpret_cast<const xmlChar*>(pValue), reinterpret_cast<const xmlChar*>("INF"))) {
                ltoneDesc->repeatCnt = TONEINFO_INF;
            } else {
                ltoneDesc->repeatCnt = static_cast<uint32_t>(atoi(pValue));
            }
            AUDIO_DEBUG_LOG("ParseToneInfo repeatCnt %{public}d", ltoneDesc->repeatCnt);
        } else if (!xmlStrcmp(sNode->name, reinterpret_cast<const xmlChar*>("RepeatSegment"))) {
            AUDIO_DEBUG_LOG("RepeatSegment node type: Element, name: %{public}s", sNode->name);
            pValue = reinterpret_cast<char*>(xmlGetProp(sNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("value"))));
            ltoneDesc->repeatSegment = static_cast<uint32_t>(atoi(pValue));
            AUDIO_DEBUG_LOG("ParseToneInfo repeatSegment %{public}d", ltoneDesc->repeatSegment);
        } else if (!xmlStrcmp(sNode->name, reinterpret_cast<const xmlChar*>("SegmentCount"))) {
            AUDIO_DEBUG_LOG("SegmentCount node type: Element, name: %{public}s", sNode->name);
            pValue = reinterpret_cast<char*>(xmlGetProp(sNode,
                reinterpret_cast<xmlChar*>(const_cast<char*>("value"))));
            segCnt = atoi(pValue);
            ltoneDesc->segmentCnt = static_cast<uint32_t>(segCnt);
            AUDIO_DEBUG_LOG("ParseToneInfo segmentCnt %{public}d", ltoneDesc->segmentCnt);
        } else if (!xmlStrcmp(sNode->name, reinterpret_cast<const xmlChar*>("Segment"))) {
            if (segInx < segCnt) {
                ParseSegment(sNode, segInx, ltoneDesc);
                segInx++;
            }
        }
        if (pValue != nullptr) {
            xmlFree(pValue);
        }
        sNode = sNode->next;
    }
}

void AudioToneParser::ParseToneInfo(xmlNode *node, std::vector<ToneInfoMap*> &toneDescriptorMaps)
{
    xmlNode *currNode = node;
    while (currNode != nullptr) {
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("ToneInfo"))) {
            currNode = currNode->next;
            continue;
        }
        std::shared_ptr<ToneInfo> ltoneDesc = std::make_shared<ToneInfo>();
        AUDIO_DEBUG_LOG("node type: Element, name: %s", currNode->name);
        char *pToneType = reinterpret_cast<char*>(xmlGetProp(currNode,
            reinterpret_cast<xmlChar*>(const_cast<char*>("toneType"))));
        if (pToneType == nullptr) {
            AUDIO_DEBUG_LOG("toneType is null");
            currNode = currNode->next;
            continue;
        }
        int32_t toneType = atoi(pToneType);
        AUDIO_DEBUG_LOG("toneType value: %{public}d", toneType);
        xmlFree(pToneType);
        if (currNode->xmlChildrenNode) {
            xmlNode *sNode = currNode->xmlChildrenNode;
            ParseToneInfoAttribute(sNode, ltoneDesc);
        }
        for (auto toneDescriptorMap : toneDescriptorMaps) {
            if (toneDescriptorMap) {
                (*toneDescriptorMap)[toneType] = ltoneDesc;
            }
        }
        currNode = currNode->next;
    }
}

void AudioToneParser::ParseSegment(xmlNode *node, int SegInx, std::shared_ptr<ToneInfo> ltoneDesc)
{
    xmlNode *currNode = node;
    for (uint32_t i = 0; i < TONEINFO_MAX_WAVES + 1; i++) {
        ltoneDesc->segments[SegInx].waveFreq[i]=0;
    }
    if ((currNode->type == XML_ELEMENT_NODE) && (!xmlStrcmp(currNode->name,
        reinterpret_cast<const xmlChar*>("Segment")))) {
        char *pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
            reinterpret_cast<xmlChar*>(const_cast<char*>("duration"))));
        if (!xmlStrcmp(reinterpret_cast<const xmlChar*>(pValue), reinterpret_cast<const xmlChar*>("INF"))) {
            ltoneDesc->segments[SegInx].duration = TONEINFO_INF;
        } else {
            ltoneDesc->segments[SegInx].duration = static_cast<uint32_t>(atoi(pValue));
        }
        AUDIO_DEBUG_LOG("duration: %{public}d", ltoneDesc->segments[SegInx].duration);
        xmlFree(pValue);
        pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
            reinterpret_cast<xmlChar*>(const_cast<char*>("loopCount"))));
        ltoneDesc->segments[SegInx].loopCnt = atoi(pValue);
        AUDIO_DEBUG_LOG("loopCnt: %{public}d", ltoneDesc->segments[SegInx].loopCnt);
        xmlFree(pValue);
        pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
            reinterpret_cast<xmlChar*>(const_cast<char*>("loopIndex"))));
        ltoneDesc->segments[SegInx].loopIndx = atoi(pValue);
        AUDIO_DEBUG_LOG("loopIndx: %{public}d", ltoneDesc->segments[SegInx].loopIndx);
        xmlFree(pValue);
        pValue = reinterpret_cast<char*>(xmlGetProp(currNode,
            reinterpret_cast<xmlChar*>(const_cast<char*>("freq"))));
        ParseFrequency(pValue, ltoneDesc->segments[SegInx]);
        xmlFree(pValue);
    }
}

void AudioToneParser::ParseFrequency (std::string freqList, ToneSegment &ltonesegment)
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
