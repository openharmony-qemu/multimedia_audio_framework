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
#define LOG_TAG "AudioConverterParser"
#endif

#include "audio_converter_parser.h"
#include <libxml/tree.h>
#ifdef USE_CONFIG_POLICY
#endif

#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {

#ifdef USE_CONFIG_POLICY
static constexpr char AUDIO_CONVERTER_CONFIG_FILE[] = "/etc/audio/audio_converter_config.xml";
#else
static constexpr char AUDIO_CONVERTER_CONFIG_FILE[] = "/system/etc/audio/audio_converter_config.xml";
#endif

static constexpr int32_t FILE_CONTENT_ERROR = -2;
static constexpr int32_t FILE_PARSE_ERROR = -3;

enum XML_ERROR {
    XML_PARSE_RECOVER = 1 << 0,   // recover on errors
    XML_PARSE_NOERROR = 1 << 5,   // suppress error reports
    XML_PARSE_NOWARNING = 1 << 6, // suppress warning reports
    XML_PARSE_PEDANTIC = 1 << 7   // pedantic error reporting
};

static std::map<std::string, AudioChannelLayout> str2layout = {
    {"CH_LAYOUT_UNKNOWN", CH_LAYOUT_UNKNOWN},
    {"CH_LAYOUT_MONO", CH_LAYOUT_MONO},
    {"CH_LAYOUT_STEREO", CH_LAYOUT_STEREO},
    {"CH_LAYOUT_STEREO_DOWNMIX", CH_LAYOUT_STEREO_DOWNMIX},
    {"CH_LAYOUT_2POINT1", CH_LAYOUT_2POINT1},
    {"CH_LAYOUT_3POINT0", CH_LAYOUT_3POINT0},
    {"CH_LAYOUT_SURROUND", CH_LAYOUT_SURROUND},
    {"CH_LAYOUT_3POINT1", CH_LAYOUT_3POINT1},
    {"CH_LAYOUT_4POINT0", CH_LAYOUT_4POINT0},
    {"CH_LAYOUT_QUAD_SIDE", CH_LAYOUT_QUAD_SIDE},
    {"CH_LAYOUT_QUAD", CH_LAYOUT_QUAD},
    {"CH_LAYOUT_2POINT0POINT2", CH_LAYOUT_2POINT0POINT2},
    {"CH_LAYOUT_4POINT1", CH_LAYOUT_4POINT1},
    {"CH_LAYOUT_5POINT0", CH_LAYOUT_5POINT0},
    {"CH_LAYOUT_5POINT0_BACK", CH_LAYOUT_5POINT0_BACK},
    {"CH_LAYOUT_2POINT1POINT2", CH_LAYOUT_2POINT1POINT2},
    {"CH_LAYOUT_3POINT0POINT2", CH_LAYOUT_3POINT0POINT2},
    {"CH_LAYOUT_5POINT1", CH_LAYOUT_5POINT1},
    {"CH_LAYOUT_5POINT1_BACK", CH_LAYOUT_5POINT1_BACK},
    {"CH_LAYOUT_6POINT0", CH_LAYOUT_6POINT0},
    {"CH_LAYOUT_HEXAGONAL", CH_LAYOUT_HEXAGONAL},
    {"CH_LAYOUT_3POINT1POINT2", CH_LAYOUT_3POINT1POINT2},
    {"CH_LAYOUT_6POINT0_FRONT", CH_LAYOUT_6POINT0_FRONT},
    {"CH_LAYOUT_6POINT1", CH_LAYOUT_6POINT1},
    {"CH_LAYOUT_6POINT1_BACK", CH_LAYOUT_6POINT1_BACK},
    {"CH_LAYOUT_6POINT1_FRONT", CH_LAYOUT_6POINT1_FRONT},
    {"CH_LAYOUT_7POINT0", CH_LAYOUT_7POINT0},
    {"CH_LAYOUT_7POINT0_FRONT", CH_LAYOUT_7POINT0_FRONT},
    {"CH_LAYOUT_7POINT1", CH_LAYOUT_7POINT1},
    {"CH_LAYOUT_OCTAGONAL", CH_LAYOUT_OCTAGONAL},
    {"CH_LAYOUT_5POINT1POINT2", CH_LAYOUT_5POINT1POINT2},
    {"CH_LAYOUT_7POINT1_WIDE", CH_LAYOUT_7POINT1_WIDE},
    {"CH_LAYOUT_7POINT1_WIDE_BACK", CH_LAYOUT_7POINT1_WIDE_BACK},
    {"CH_LAYOUT_5POINT1POINT4", CH_LAYOUT_5POINT1POINT4},
    {"CH_LAYOUT_7POINT1POINT2", CH_LAYOUT_7POINT1POINT2},
    {"CH_LAYOUT_7POINT1POINT4", CH_LAYOUT_7POINT1POINT4},
    {"CH_LAYOUT_10POINT2", CH_LAYOUT_10POINT2},
    {"CH_LAYOUT_9POINT1POINT4", CH_LAYOUT_9POINT1POINT4},
    {"CH_LAYOUT_9POINT1POINT6", CH_LAYOUT_9POINT1POINT6},
    {"CH_LAYOUT_HEXADECAGONAL", CH_LAYOUT_HEXADECAGONAL},
    {"CH_LAYOUT_AMB_ORDER1_ACN_N3D", CH_LAYOUT_HOA_ORDER1_ACN_N3D},
    {"CH_LAYOUT_AMB_ORDER1_ACN_SN3D", CH_LAYOUT_HOA_ORDER1_ACN_SN3D},
    {"CH_LAYOUT_AMB_ORDER1_FUMA", CH_LAYOUT_HOA_ORDER1_FUMA},
    {"CH_LAYOUT_AMB_ORDER2_ACN_N3D", CH_LAYOUT_HOA_ORDER2_ACN_N3D},
    {"CH_LAYOUT_AMB_ORDER2_ACN_SN3D", CH_LAYOUT_HOA_ORDER2_ACN_SN3D},
    {"CH_LAYOUT_AMB_ORDER2_FUMA", CH_LAYOUT_HOA_ORDER2_FUMA},
    {"CH_LAYOUT_AMB_ORDER3_ACN_N3D", CH_LAYOUT_HOA_ORDER3_ACN_N3D},
    {"CH_LAYOUT_AMB_ORDER3_ACN_SN3D", CH_LAYOUT_HOA_ORDER3_ACN_SN3D},
    {"CH_LAYOUT_AMB_ORDER3_FUMA", CH_LAYOUT_HOA_ORDER3_FUMA},
};

static void WriteConverterConfigError()
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::AUDIO, Media::MediaMonitor::LOAD_CONFIG_ERROR,
        Media::MediaMonitor::FAULT_EVENT);
    bean->Add("CATEGORY", Media::MediaMonitor::AUDIO_CONVERTER_CONFIG);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

static void ParseEffectConfigFile(xmlDoc* &doc)
{
    AUDIO_INFO_LOG("use default audio effect config file path: %{public}s", AUDIO_CONVERTER_CONFIG_FILE);
    doc = xmlReadFile(AUDIO_CONVERTER_CONFIG_FILE, nullptr, XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
}

AudioConverterParser::AudioConverterParser()
{
    AUDIO_INFO_LOG("AudioConverterParser created");
}

static int32_t LoadConfigCheck(xmlDoc *doc, xmlNode *currNode)
{
    CHECK_AND_RETURN_RET_LOG(currNode != nullptr, FILE_PARSE_ERROR, "error: could not parse file %{public}s",
        AUDIO_CONVERTER_CONFIG_FILE);
    bool ret = xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar *>("audio_converter_conf"));
    CHECK_AND_RETURN_RET_LOG(!ret, FILE_CONTENT_ERROR, "Missing tag - audio_converter_conf: %{public}s",
        AUDIO_CONVERTER_CONFIG_FILE);
    CHECK_AND_RETURN_RET_LOG(currNode->xmlChildrenNode != nullptr, FILE_CONTENT_ERROR,
        "Missing node - audio_converter_conf: %s", AUDIO_CONVERTER_CONFIG_FILE);

    return 0;
}

static void LoadConfigLibrary(ConverterConfig &result, xmlNode *currNode)
{
    if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar *>("name"))) {
        AUDIO_WARNING_LOG("missing information: library has no name attribute");
    } else if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar *>("path"))) {
        AUDIO_WARNING_LOG("missing information: library has no path attribute");
    } else {
        std::string libName = reinterpret_cast<char *>(xmlGetProp(currNode, reinterpret_cast<const xmlChar *>("name")));
        std::string libPath = reinterpret_cast<char *>(xmlGetProp(currNode, reinterpret_cast<const xmlChar *>("path")));
        result.library = {libName, libPath};
    }
}

static void LoadConfigChannelLayout(ConverterConfig &result, xmlNode *currNode)
{
    if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar *>("out_channel_layout"))) {
        AUDIO_ERR_LOG("missing information: config has no out_channel_layout attribute, set to default STEREO");
        result.outChannelLayout = CH_LAYOUT_STEREO;
    } else {
        std::string strChannelLayout =
            reinterpret_cast<char *>(xmlGetProp(currNode, reinterpret_cast<const xmlChar *>("out_channel_layout")));
        if (str2layout.count(strChannelLayout) == 0) {
            AUDIO_ERR_LOG("unsupported format: invalid channel layout, set to STEREO");
            result.outChannelLayout = CH_LAYOUT_STEREO;
        } else {
            result.outChannelLayout = str2layout[strChannelLayout];
            AUDIO_INFO_LOG("AudioVivid MCR output format is %{public}s", strChannelLayout.c_str());
        }
    }
}

static void LoadConfigVersion(ConverterConfig &result, xmlNode *currNode)
{
    bool ret = xmlHasProp(currNode, reinterpret_cast<const xmlChar *>("version"));
    CHECK_AND_RETURN_LOG(ret, "missing information: audio_converter_conf node has no version attribute");

    result.version = reinterpret_cast<char *>(xmlGetProp(currNode, reinterpret_cast<const xmlChar *>("version")));
}

AudioConverterParser &AudioConverterParser::GetInstance()
{
    static AudioConverterParser instance;
    return instance;
}

ConverterConfig AudioConverterParser::LoadConfig()
{
    std::lock_guard<std::mutex> lock(loadConfigMutex_);
    int32_t ret = 0;
    AUDIO_INFO_LOG("AudioConverterParser::LoadConfig");
    CHECK_AND_RETURN_RET(cfg_ == nullptr, *cfg_);
    xmlDoc *doc = nullptr;
    xmlNode *rootElement = nullptr;
    cfg_ = std::make_unique<ConverterConfig>();
    ConverterConfig &result = *cfg_;

    ParseEffectConfigFile(doc);
    if (doc == nullptr) {
        WriteConverterConfigError();
    }
    CHECK_AND_RETURN_RET_LOG(doc != nullptr, result, "error: could not parse file %{public}s",
        AUDIO_CONVERTER_CONFIG_FILE);

    rootElement = xmlDocGetRootElement(doc);
    xmlNode *currNode = rootElement;

    if ((ret = LoadConfigCheck(doc, currNode)) != 0) {
        xmlFreeDoc(doc);
        xmlCleanupParser();
        return result;
    }

    LoadConfigVersion(result, currNode);
    currNode = currNode->xmlChildrenNode;

    while (currNode != nullptr) {
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }

        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar *>("library"))) {
            LoadConfigLibrary(result, currNode);
        } else if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar *>("converter_conf"))) {
            LoadConfigChannelLayout(result, currNode);
        }

        currNode = currNode->next;
    }
    xmlFreeDoc(doc);
    xmlCleanupParser();
    return result;
}
} // namespace AudioStandard
} // namespace OHOS