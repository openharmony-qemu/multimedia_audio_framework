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
#define LOG_TAG "AudioEffectConfigParser"
#endif

#include "audio_effect_config_parser.h"
#include <libxml/tree.h>
#ifdef USE_CONFIG_POLICY
#include "config_policy_utils.h"
#endif
#include "media_monitor_manager.h"

namespace OHOS {
namespace AudioStandard {
#ifdef USE_CONFIG_POLICY
static constexpr char AUDIO_EFFECT_CONFIG_FILE[] = "etc/audio/audio_effect_config.xml";
#else
static constexpr char AUDIO_EFFECT_CONFIG_FILE[] = "system/etc/audio/audio_effect_config.xml";
#endif
static const std::string EFFECT_CONFIG_NAME[5] = {"libraries", "effects", "effectChains", "preProcess", "postProcess"};
static constexpr int32_t FILE_CONTENT_ERROR = -2;
static constexpr int32_t FILE_PARSE_ERROR = -3;
static constexpr int32_t INDEX_LIBRARIES = 0;
static constexpr int32_t INDEX_EFFECS = 1;
static constexpr int32_t INDEX_EFFECTCHAINE = 2;
static constexpr int32_t INDEX_PREPROCESS = 3;
static constexpr int32_t INDEX_POSTPROCESS = 4;
static constexpr int32_t INDEX_EXCEPTION = 5;
static constexpr int32_t NODE_SIZE = 6;
static constexpr int32_t MODULE_SIZE = 5;
static constexpr uint32_t XML_PARSE_NOERROR = 1 << 5;
static constexpr uint32_t XML_PARSE_NOWARNING = 1 << 6;
static constexpr int32_t INDEX_POST_STREAMS = 0;
static constexpr int32_t INDEX_POST_MAPPING = 1;
static constexpr int32_t INDEX_POST_EXCEPTION = 2;
static constexpr int32_t NODE_SIZE_POST = 3;

AudioEffectConfigParser::AudioEffectConfigParser()
{
    AUDIO_INFO_LOG("AudioEffectConfigParser created");
}

AudioEffectConfigParser::~AudioEffectConfigParser()
{
}

static int32_t ParseEffectConfigFile(xmlDoc* &doc)
{
#ifdef USE_CONFIG_POLICY
    CfgFiles *cfgFiles = GetCfgFiles(AUDIO_EFFECT_CONFIG_FILE);
    if (cfgFiles == nullptr) {
        AUDIO_ERR_LOG("Not found audio_effect_config.xml!");
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::AUDIO, Media::MediaMonitor::LOAD_CONFIG_ERROR,
            Media::MediaMonitor::FAULT_EVENT);
        bean->Add("CATEGORY", Media::MediaMonitor::AUDIO_EFFECT_CONFIG);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
        return FILE_PARSE_ERROR;
    }

    for (int32_t i = MAX_CFG_POLICY_DIRS_CNT - 1; i >= 0; i--) {
        if (cfgFiles->paths[i] && *(cfgFiles->paths[i]) != '\0') {
            AUDIO_INFO_LOG("effect config file path:%{public}s", cfgFiles->paths[i]);
            doc = xmlReadFile(cfgFiles->paths[i], nullptr, XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
            break;
        }
    }
    FreeCfgFiles(cfgFiles);
#else
    AUDIO_INFO_LOG("use default audio effect config file path: %{public}s", AUDIO_EFFECT_CONFIG_FILE);
    doc = xmlReadFile(AUDIO_EFFECT_CONFIG_FILE, nullptr, XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
#endif
    if (doc == nullptr) {
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::AUDIO, Media::MediaMonitor::LOAD_CONFIG_ERROR,
            Media::MediaMonitor::FAULT_EVENT);
        bean->Add("CATEGORY", Media::MediaMonitor::AUDIO_EFFECT_CONFIG);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
    }
    CHECK_AND_RETURN_RET_LOG(doc != nullptr, FILE_PARSE_ERROR, "load audio effect config fail");
    return 0;
}

static int32_t LoadConfigCheck(xmlDoc *doc, xmlNode *currNode)
{
    CHECK_AND_RETURN_RET_LOG(currNode != nullptr, FILE_PARSE_ERROR, "error: could not parse file");
    if (xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("audio_effects_conf"))) {
        AUDIO_ERR_LOG("Missing tag - audio_effects_conf");
        xmlFreeDoc(doc);
        return FILE_CONTENT_ERROR;
    }

    if (currNode->xmlChildrenNode) {
        return 0;
    } else {
        AUDIO_ERR_LOG("Missing node - audio_effects_conf");
        xmlFreeDoc(doc);
        return FILE_CONTENT_ERROR;
    }
}

static void LoadConfigVersion(OriginalEffectConfig &result, xmlNode *currNode)
{
    bool ret = xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("version"));
    CHECK_AND_RETURN_LOG(ret, "missing information: audio_effects_conf node has no version attribute");

    result.version = reinterpret_cast<char *>(xmlGetProp(currNode, reinterpret_cast<const xmlChar *>("version")));
}

static void LoadLibrary(OriginalEffectConfig &result, xmlNode *secondNode)
{
    xmlNode *currNode = secondNode;
    int32_t countLibrary = 0;
    while (currNode != nullptr) {
        CHECK_AND_RETURN_LOG(countLibrary < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of library nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("library"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("name"))) {
                AUDIO_ERR_LOG("missing information: library has no name attribute");
            } else if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("path"))) {
                AUDIO_ERR_LOG("missing information: library has no path attribute");
            } else {
                std::string pLibName = reinterpret_cast<char*>
                                      (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("name")));
                std::string pLibPath = reinterpret_cast<char*>
                                      (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("path")));
                Library tmp = {pLibName, pLibPath};
                result.libraries.push_back(tmp);
            }
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be library", currNode->name);
        }
        countLibrary++;
        currNode = currNode->next;
    }
    if (countLibrary == 0) {
        AUDIO_WARNING_LOG("missing information: libraries have no child library");
    }
}

static void LoadEffectConfigLibraries(OriginalEffectConfig &result, const xmlNode *currNode,
                                      int32_t (&countFirstNode)[NODE_SIZE])
{
    if (countFirstNode[INDEX_LIBRARIES] >= AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
        if (countFirstNode[INDEX_LIBRARIES] == AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
            countFirstNode[INDEX_LIBRARIES]++;
            AUDIO_WARNING_LOG("the number of libraries nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (currNode->xmlChildrenNode) {
        LoadLibrary(result, currNode->xmlChildrenNode);
        countFirstNode[INDEX_LIBRARIES]++;
    } else {
        AUDIO_WARNING_LOG("missing information: libraries have no child library");
        countFirstNode[INDEX_LIBRARIES]++;
    }
}

static void LoadEffect(OriginalEffectConfig &result, xmlNode *secondNode)
{
    xmlNode *currNode = secondNode;
    int32_t countEffect = 0;
    while (currNode != nullptr) {
        CHECK_AND_RETURN_LOG(countEffect < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of effect nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("effect"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("name"))) {
                AUDIO_ERR_LOG("missing information: effect has no name attribute");
            } else if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("library"))) {
                AUDIO_ERR_LOG("missing information: effect has no library attribute");
            } else {
                std::string pEffectName = reinterpret_cast<char*>
                              (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("name")));
                std::string pEffectLib = reinterpret_cast<char*>
                             (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("library")));
                Effect tmp = {pEffectName, pEffectLib};
                result.effects.push_back(tmp);
            }
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be effect", currNode->name);
        }
        countEffect++;
        currNode = currNode->next;
    }
    if (countEffect == 0) {
        AUDIO_WARNING_LOG("missing information: effects have no child effect");
    }
}

static void LoadEffectConfigEffects(OriginalEffectConfig &result, const xmlNode *currNode,
                                    int32_t (&countFirstNode)[NODE_SIZE])
{
    if (countFirstNode[INDEX_EFFECS] >= AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
        if (countFirstNode[INDEX_EFFECS] == AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
            countFirstNode[INDEX_EFFECS]++;
            AUDIO_WARNING_LOG("the number of effects nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (currNode->xmlChildrenNode) {
        LoadEffect(result, currNode->xmlChildrenNode);
        countFirstNode[INDEX_EFFECS]++;
    } else {
        AUDIO_WARNING_LOG("missing information: effects have no child effect");
        countFirstNode[INDEX_EFFECS]++;
    }
}

static void LoadApply(OriginalEffectConfig &result, const xmlNode *thirdNode, const int32_t segInx)
{
    CHECK_AND_RETURN_LOG(thirdNode->xmlChildrenNode, "missing information: effectChain has no child apply");
    int32_t countApply = 0;
    xmlNode *currNode = thirdNode->xmlChildrenNode;
    while (currNode != nullptr) {
        CHECK_AND_RETURN_LOG(countApply < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of apply nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("apply"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("effect"))) {
                AUDIO_WARNING_LOG("missing information: apply has no effect attribute");
            } else {
                std::string ppValue = reinterpret_cast<char*>
                                     (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("effect")));
                result.effectChains[segInx].apply.push_back(ppValue);
            }
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be apply", currNode->name);
        }
        countApply++;
        currNode = currNode->next;
    }
    if (countApply == 0) {
        AUDIO_WARNING_LOG("missing information: effectChain has no child apply");
    }
}

static void LoadEffectChain(OriginalEffectConfig &result, xmlNode *secondNode)
{
    xmlNode *currNode = secondNode;
    int32_t countEffectChain = 0;
    int32_t segInx = 0;
    std::vector<std::string> apply;
    while (currNode != nullptr) {
        CHECK_AND_RETURN_LOG(countEffectChain < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of effectChain nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("effectChain"))) {
            std::string label = "";
            if (xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("label"))) {
                label = reinterpret_cast<char*>(xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("label")));
            }
            if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("name"))) {
                AUDIO_WARNING_LOG("missing information: effectChain has no name attribute");
            } else {
                std::string peffectChainName = reinterpret_cast<char*>
                                   (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("name")));
                EffectChain tmp = {peffectChainName, apply, label};
                result.effectChains.push_back(tmp);
                LoadApply(result, currNode, segInx);
                segInx++;
            }
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be effectChain", currNode->name);
        }
        countEffectChain++;
        currNode = currNode->next;
    }
    if (countEffectChain == 0) {
        AUDIO_WARNING_LOG("missing information: effectChains have no child effectChain");
    }
}

static void LoadEffectConfigEffectChains(OriginalEffectConfig &result, const xmlNode *currNode,
                                         int32_t (&countFirstNode)[NODE_SIZE])
{
    if (countFirstNode[INDEX_EFFECTCHAINE] >= AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
        if (countFirstNode[INDEX_EFFECTCHAINE] == AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
            countFirstNode[INDEX_EFFECTCHAINE]++;
            AUDIO_WARNING_LOG("the number of effectChains nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (currNode->xmlChildrenNode) {
        LoadEffectChain(result, currNode->xmlChildrenNode);
        countFirstNode[INDEX_EFFECTCHAINE]++;
    } else {
        AUDIO_WARNING_LOG("missing information: effectChains have no child effectChain");
        countFirstNode[INDEX_EFFECTCHAINE]++;
    }
}

static void LoadPreDevice(OriginalEffectConfig &result, const xmlNode *fourthNode,
                          const int32_t modeNum, const int32_t streamNum)
{
    CHECK_AND_RETURN_LOG(fourthNode->xmlChildrenNode, "missing information: streamEffectMode has no child devicePort");
    int32_t countDevice = 0;
    xmlNode *currNode = fourthNode->xmlChildrenNode;
    while (currNode != nullptr) {
        CHECK_AND_RETURN_LOG(countDevice < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of devicePort nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("devicePort"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("type"))) {
                AUDIO_ERR_LOG("missing information: devicePort has no type attribute");
            } else if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("effectChain"))) {
                AUDIO_ERR_LOG("missing information: devicePort has no effectChain attribute");
            } else {
                std::string pDevType = reinterpret_cast<char*>
                           (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("type")));
                std::string pChain = reinterpret_cast<char*>
                         (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("effectChain")));
                Device tmpdev = {pDevType, pChain};
                result.preProcess[streamNum].device[modeNum].push_back(tmpdev);
            }
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be devicePort", currNode->name);
        }
        countDevice++;
        currNode = currNode->next;
    }
    if (countDevice == 0) {
        AUDIO_WARNING_LOG("missing information: streamEffectMode has no child devicePort");
    }
}

static void LoadPreMode(OriginalEffectConfig &result, const xmlNode *thirdNode, const int32_t streamNum)
{
    CHECK_AND_RETURN_LOG(thirdNode->xmlChildrenNode,
        "missing information: stream has no child streamEffectMode");
    int32_t countMode = 0;
    int32_t modeNum = 0;
    xmlNode *currNode = thirdNode->xmlChildrenNode;
    while (currNode != nullptr) {
        CHECK_AND_RETURN_LOG(countMode < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of streamEffectMode nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("streamEffectMode"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("mode"))) {
                AUDIO_WARNING_LOG("missing information: streamEffectMode has no mode attribute");
            } else {
                std::string pStreamAEMode = reinterpret_cast<char*>
                                (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("mode")));
                result.preProcess[streamNum].mode.push_back(pStreamAEMode);
                result.preProcess[streamNum].device.push_back({});
                LoadPreDevice(result, currNode, modeNum, streamNum);
                modeNum++;
            }
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be streamEffectMode", currNode->name);
        }
        countMode++;
        currNode = currNode->next;
    }
    if (countMode == 0) {
        AUDIO_WARNING_LOG("missing information: stream has no child streamEffectMode");
    }
}

static void LoadPreProcess(OriginalEffectConfig &result, xmlNode *secondNode)
{
    std::string stream;
    std::vector<std::string> mode;
    std::vector<std::vector<Device>> device;
    Preprocess tmp = {stream, mode, device};
    xmlNode *currNode = secondNode;
    int32_t countPreprocess = 0;
    int32_t streamNum = 0;
    while (currNode != nullptr) {
        CHECK_AND_RETURN_LOG(countPreprocess < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of stream nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("stream"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("scene"))) {
                AUDIO_WARNING_LOG("missing information: stream has no scene attribute");
            } else {
                std::string pStreamType = reinterpret_cast<char*>
                                         (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("scene")));
                tmp.stream = pStreamType;
                result.preProcess.push_back(tmp);
                LoadPreMode(result, currNode, streamNum);
                streamNum++;
            }
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be stream", currNode->name);
        }
        countPreprocess++;
        currNode = currNode->next;
    }
    if (countPreprocess == 0) {
        AUDIO_WARNING_LOG("missing information: preProcess has no child stream");
    }
}

static void LoadEffectConfigPreProcess(OriginalEffectConfig &result, const xmlNode *currNode,
                                       int32_t (&countFirstNode)[NODE_SIZE])
{
    if (countFirstNode[INDEX_PREPROCESS] >= AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
        if (countFirstNode[INDEX_PREPROCESS] == AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
            countFirstNode[INDEX_PREPROCESS]++;
            AUDIO_WARNING_LOG("the number of preProcess nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (currNode->xmlChildrenNode) {
        LoadPreProcess(result, currNode->xmlChildrenNode);
        countFirstNode[INDEX_PREPROCESS]++;
    } else {
        AUDIO_WARNING_LOG("missing information: preProcess has no child stream");
        countFirstNode[INDEX_PREPROCESS]++;
    }
}

static void LoadStreamUsageMapping(OriginalEffectConfig &result, xmlNode *thirdNode)
{
    SceneMappingItem tmp;
    xmlNode *currNode = thirdNode;
    int32_t countUsage = 0;
    while (currNode != nullptr) {
        CHECK_AND_RETURN_LOG(countUsage < AUDIO_EFFECT_COUNT_STREAM_USAGE_UPPER_LIMIT,
            "streamUsage map item exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_STREAM_USAGE_UPPER_LIMIT);
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("streamUsage"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("name")) ||
                !xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("scene"))) {
                AUDIO_WARNING_LOG("missing information: streamUsage misses attribute");
            } else {
                tmp.name = reinterpret_cast<char*>(
                    xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("name")));
                tmp.sceneType = reinterpret_cast<char*>(
                    xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("scene")));
                result.postProcess.sceneMap.push_back(tmp);
            }
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be streamUsage", currNode->name);
        }
        countUsage++;
        currNode = currNode->next;
    }
    if (countUsage == 0) {
        AUDIO_WARNING_LOG("missing information: sceneMap has no child streamUsage");
    }
}

static void LoadPostDevice(OriginalEffectConfig &result, const xmlNode *fifthNode,
                           const int32_t modeNum, const int32_t streamNum)
{
    CHECK_AND_RETURN_LOG(fifthNode->xmlChildrenNode, "missing information: streamEffectMode has no child devicePort");
    int32_t countDevice = 0;
    xmlNode *currNode = fifthNode->xmlChildrenNode;
    while (currNode != nullptr) {
        CHECK_AND_RETURN_LOG(countDevice < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of devicePort nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("devicePort"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("type"))) {
                AUDIO_WARNING_LOG("missing information: devicePort has no type attribute");
            } else if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("effectChain"))) {
                AUDIO_WARNING_LOG("missing information: devicePort has no effectChain attribute");
            } else {
                std::string pDevType = reinterpret_cast<char*>
                           (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("type")));
                std::string pChain = reinterpret_cast<char*>
                         (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("effectChain")));
                Device tmpdev = {pDevType, pChain};
                result.postProcess.effectSceneStreams[streamNum].device[modeNum].push_back(tmpdev);
            }
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be devicePort", currNode->name);
        }
        countDevice++;
        currNode = currNode->next;
    }
    if (countDevice == 0) {
        AUDIO_WARNING_LOG("missing information: streamEffectMode has no child devicePort");
    }
}

static void LoadPostMode(OriginalEffectConfig &result, const xmlNode *fourthNode, const int32_t streamNum)
{
    CHECK_AND_RETURN_LOG(fourthNode->xmlChildrenNode,
        "missing information: stream has no child streamEffectMode");
    int32_t countMode = 0;
    int32_t modeNum = 0;
    xmlNode *currNode = fourthNode->xmlChildrenNode;
    while (currNode != nullptr) {
        CHECK_AND_RETURN_LOG(countMode < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of streamEffectMode nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("streamEffectMode"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("mode"))) {
                AUDIO_ERR_LOG("missing information: streamEffectMode has no mode attribute");
            } else {
                std::string pStreamAEMode = reinterpret_cast<char*>
                                (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("mode")));
                result.postProcess.effectSceneStreams[streamNum].mode.push_back(pStreamAEMode);
                result.postProcess.effectSceneStreams[streamNum].device.push_back({});
                LoadPostDevice(result, currNode, modeNum, streamNum);
                modeNum++;
            }
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be streamEffectMode", currNode->name);
        }
        countMode++;
        currNode = currNode->next;
    }
    if (countMode == 0) {
        AUDIO_WARNING_LOG("missing information: stream has no child streamEffectMode");
    }
}

static void LoadEffectSceneStreams(OriginalEffectConfig &result, xmlNode *thirdNode)
{
    std::string stream;
    std::vector<std::string> mode;
    std::vector<std::vector<Device>> device;
    EffectSceneStream tmp = {stream, mode, device};
    xmlNode *currNode = thirdNode;
    int32_t countPostProcess = 0;
    int32_t streamNum = 0;
    while (currNode != nullptr) {
        CHECK_AND_RETURN_LOG(countPostProcess < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of stream nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }
        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("stream"))) {
            if (!xmlHasProp(currNode, reinterpret_cast<const xmlChar*>("scene"))) {
                AUDIO_WARNING_LOG("missing information: stream has no scene attribute");
            } else {
                std::string pStreamType = reinterpret_cast<char*>
                                         (xmlGetProp(currNode, reinterpret_cast<const xmlChar*>("scene")));
                tmp.stream = pStreamType;
                result.postProcess.effectSceneStreams.push_back(tmp);
                LoadPostMode(result, currNode, streamNum);
                streamNum++;
            }
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be stream", currNode->name);
        }
        countPostProcess++;
        currNode = currNode->next;
    }
    if (countPostProcess == 0) {
        AUDIO_WARNING_LOG("missing information: postProcess has no child stream");
    }
}

static void LoadEffectSceneStreamsCheck(OriginalEffectConfig &result, const xmlNode *currNode,
                                        int32_t (&countPostSecondNode)[NODE_SIZE_POST])
{
    if (countPostSecondNode[INDEX_POST_STREAMS] >= AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT) {
        if (countPostSecondNode[INDEX_POST_STREAMS] == AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT) {
            countPostSecondNode[INDEX_POST_STREAMS]++;
            AUDIO_WARNING_LOG("the number of postprocessStreams nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT);
        }
    } else if (currNode->xmlChildrenNode) {
        LoadEffectSceneStreams(result, currNode->xmlChildrenNode);
        countPostSecondNode[INDEX_POST_STREAMS]++;
    } else {
        AUDIO_WARNING_LOG("missing information: postprocessStreams has no child stream");
        countPostSecondNode[INDEX_POST_STREAMS]++;
    }
}

static void LoadStreamUsageMappingCheck(OriginalEffectConfig &result, const xmlNode *currNode,
                                        int32_t (&countPostSecondNode)[NODE_SIZE_POST])
{
    if (countPostSecondNode[INDEX_POST_MAPPING] >= AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT) {
        if (countPostSecondNode[INDEX_POST_MAPPING] == AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT) {
            countPostSecondNode[INDEX_POST_MAPPING]++;
            AUDIO_WARNING_LOG("the number of sceneMap nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT);
        }
    } else if (currNode->xmlChildrenNode) {
        LoadStreamUsageMapping(result, currNode->xmlChildrenNode);
        countPostSecondNode[INDEX_POST_MAPPING]++;
    } else {
        AUDIO_WARNING_LOG("missing information: sceneMap has no child stream");
        countPostSecondNode[INDEX_POST_MAPPING]++;
    }
}

static void LoadPostprocessExceptionCheck(OriginalEffectConfig &result, const xmlNode *currNode,
                                          int32_t (&countPostSecondNode)[NODE_SIZE_POST])
{
    if (countPostSecondNode[INDEX_POST_EXCEPTION] >= AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT) {
        if (countPostSecondNode[INDEX_POST_EXCEPTION] == AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT) {
            countPostSecondNode[INDEX_POST_EXCEPTION]++;
            AUDIO_ERR_LOG("the number of postprocess nodes with wrong name exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT);
        }
    } else {
        AUDIO_WARNING_LOG("wrong name: %{public}s", currNode->name);
        countPostSecondNode[INDEX_POST_EXCEPTION]++;
    }
}

static void LoadPostProcessCfg(OriginalEffectConfig &result, xmlNode *secondNode)
{
    int32_t countPostSecondNode[NODE_SIZE_POST] = {0};
    xmlNode *currNode = secondNode;
    while (currNode != nullptr) {
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }

        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("effectSceneStreams"))) {
            LoadEffectSceneStreamsCheck(result, currNode, countPostSecondNode);
        } else if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("sceneMap"))) {
            LoadStreamUsageMappingCheck(result, currNode, countPostSecondNode);
        } else {
            LoadPostprocessExceptionCheck(result, currNode, countPostSecondNode);
        }
        currNode = currNode->next;
    }
}

static void LoadEffectConfigPostProcessCfg(OriginalEffectConfig &result, const xmlNode *currNode,
                                           int32_t (&countFirstNode)[NODE_SIZE])
{
    if (countFirstNode[INDEX_POSTPROCESS] >= AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
        if (countFirstNode[INDEX_POSTPROCESS] == AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
            countFirstNode[INDEX_POSTPROCESS]++;
            AUDIO_WARNING_LOG("the number of postProcess nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (currNode->xmlChildrenNode) {
        LoadPostProcessCfg(result, currNode->xmlChildrenNode);
        countFirstNode[INDEX_POSTPROCESS]++;
    } else {
        AUDIO_WARNING_LOG("missing information: postProcess has no child stream");
        countFirstNode[INDEX_POSTPROCESS]++;
    }
}

static void LoadEffectConfigException(OriginalEffectConfig &result, const xmlNode *currNode,
                                      int32_t (&countFirstNode)[NODE_SIZE])
{
    if (countFirstNode[INDEX_EXCEPTION] >= AUDIO_EFFECT_COUNT_UPPER_LIMIT) {
        if (countFirstNode[INDEX_EXCEPTION] == AUDIO_EFFECT_COUNT_UPPER_LIMIT) {
            countFirstNode[INDEX_EXCEPTION]++;
            AUDIO_ERR_LOG("the number of nodes with wrong name exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        }
    } else {
        AUDIO_WARNING_LOG("wrong name: %{public}s", currNode->name);
        countFirstNode[INDEX_EXCEPTION]++;
    }
}

int32_t AudioEffectConfigParser::LoadEffectConfig(OriginalEffectConfig &result)
{
    int32_t countFirstNode[NODE_SIZE] = {0};
    xmlDoc *doc = nullptr;
    xmlNode *rootElement = nullptr;

    int32_t ret = ParseEffectConfigFile(doc);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "error: could not parse audio effect config file");

    rootElement = xmlDocGetRootElement(doc);
    xmlNode *currNode = rootElement;

    if (LoadConfigCheck(doc, currNode) == 0) {
        LoadConfigVersion(result, currNode);
        currNode = currNode->xmlChildrenNode;
    } else {
        return FILE_CONTENT_ERROR;
    }

    while (currNode != nullptr) {
        if (currNode->type != XML_ELEMENT_NODE) {
            currNode = currNode->next;
            continue;
        }

        if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("libraries"))) {
            LoadEffectConfigLibraries(result, currNode, countFirstNode);
        } else if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("effects"))) {
            LoadEffectConfigEffects(result, currNode, countFirstNode);
        } else if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("effectChains"))) {
            LoadEffectConfigEffectChains(result, currNode, countFirstNode);
        } else if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("preProcess"))) {
            LoadEffectConfigPreProcess(result, currNode, countFirstNode);
        } else if (!xmlStrcmp(currNode->name, reinterpret_cast<const xmlChar*>("postProcess"))) {
            LoadEffectConfigPostProcessCfg(result, currNode, countFirstNode);
        } else {
            LoadEffectConfigException(result, currNode, countFirstNode);
        }

        currNode = currNode->next;
    }

    for (int32_t i = 0; i < MODULE_SIZE; i++) {
        if (countFirstNode[i] == 0) {
            AUDIO_WARNING_LOG("missing information: %{public}s", EFFECT_CONFIG_NAME[i].c_str());
        }
    }

    if (doc) {
        xmlFreeDoc(doc);
    }
    return 0;
}
} // namespace AudioStandard
} // namespace OHOS