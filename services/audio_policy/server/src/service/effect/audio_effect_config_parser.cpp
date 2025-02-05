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
#ifndef LOG_TAG
#define LOG_TAG "AudioEffectConfigParser"
#endif

#include "audio_effect_config_parser.h"
#ifdef USE_CONFIG_POLICY
#include "config_policy_utils.h"
#endif
#include "media_monitor_manager.h"
#include "audio_xml_parser.h"
#include "audio_utils.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {
#ifdef USE_CONFIG_POLICY
static constexpr char AUDIO_EFFECT_CONFIG_FILE[] = "etc/audio/audio_effect_config.xml";
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
static constexpr int32_t AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT = 1;
static constexpr int32_t AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT = 1;
static constexpr int32_t AUDIO_EFFECT_COUNT_PRE_SECOND_NODE_UPPER_LIMIT = 1;
constexpr int32_t AUDIO_EFFECT_COUNT_STREAM_USAGE_UPPER_LIMIT = 200;
#ifdef USE_CONFIG_POLICY
static constexpr uint32_t XML_PARSE_NOERROR = 1 << 5;
static constexpr uint32_t XML_PARSE_NOWARNING = 1 << 6;
#endif

AudioEffectConfigParser::AudioEffectConfigParser()
{
    AUDIO_INFO_LOG("AudioEffectConfigParser created");
}

AudioEffectConfigParser::~AudioEffectConfigParser()
{
}

static int32_t ParseEffectConfigFile(std::shared_ptr<AudioXmlNode> curNode)
{
    int32_t ret = 0;
#ifdef USE_CONFIG_POLICY
    char buf[MAX_PATH_LEN];
    char *path = GetOneCfgFile(AUDIO_EFFECT_CONFIG_FILE, buf, MAX_PATH_LEN);
    if (path != nullptr && *path != '\0') {
        AUDIO_INFO_LOG("effect config file path: %{public}s", path);
        ret = curNode->Config(path, nullptr, XML_PARSE_NOERROR | XML_PARSE_NOWARNING);
    }
#endif
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("error: could not parse audio_effect_config.xml!");
        std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
            Media::MediaMonitor::AUDIO, Media::MediaMonitor::LOAD_CONFIG_ERROR,
            Media::MediaMonitor::FAULT_EVENT);
        bean->Add("CATEGORY", Media::MediaMonitor::AUDIO_EFFECT_CONFIG);
        Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
        return FILE_PARSE_ERROR;
    }
    return 0;
}

static int32_t LoadConfigCheck(std::shared_ptr<AudioXmlNode> curNode)
{
    CHECK_AND_RETURN_RET_LOG(curNode->IsNodeValid(), FILE_PARSE_ERROR, "error: could not parse file");
    if (!curNode->CompareName("audio_effects_conf")) {
        AUDIO_ERR_LOG("Missing tag - audio_effects_conf");
        return FILE_CONTENT_ERROR;
    }

    curNode->MoveToChildren();
    if (curNode->IsNodeValid()) {
        return 0;
    } else {
        AUDIO_ERR_LOG("Missing node - audio_effects_conf");
        return FILE_CONTENT_ERROR;
    }
}

static void LoadLibrary(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode)
{
    int32_t countLibrary = 0;
    while (curNode->IsNodeValid()) {
        CHECK_AND_RETURN_LOG(countLibrary < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of library nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        if (curNode->CompareName("library")) {
            std::string pLibName;
            std::string pLibPath;
            if (curNode->GetProp("name", pLibName) != SUCCESS) {
                AUDIO_ERR_LOG("missing information: library has no name attribute");
            }
            if (curNode->GetProp("path", pLibPath) != SUCCESS) {
                AUDIO_ERR_LOG("missing information: library has no path attribute");
            }
            Library tmp = {pLibName, pLibPath};
            result.libraries.push_back(tmp);
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be library", curNode->GetName().c_str());
        }
        countLibrary++;
        curNode->MoveToNext();
    }
    if (countLibrary == 0) {
        AUDIO_WARNING_LOG("missing information: libraries have no child library");
    }
}

static void LoadEffectConfigLibraries(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode,
                                      int32_t (&countFirstNode)[NODE_SIZE])
{
    if (countFirstNode[INDEX_LIBRARIES] >= AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
        if (countFirstNode[INDEX_LIBRARIES] == AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
            countFirstNode[INDEX_LIBRARIES]++;
            AUDIO_WARNING_LOG("the number of libraries nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (curNode->GetChildrenNode()->IsNodeValid()) {
        LoadLibrary(result, curNode->GetChildrenNode());
        countFirstNode[INDEX_LIBRARIES]++;
    } else {
        AUDIO_WARNING_LOG("missing information: libraries have no child library");
        countFirstNode[INDEX_LIBRARIES]++;
    }
}

static void LoadEffectProperty(OriginalEffectConfig &result,
    std::shared_ptr<AudioXmlNode> curNode, const int32_t effectIdx)
{
    curNode->MoveToChildren();
    CHECK_AND_RETURN_LOG(curNode->IsNodeValid(), "effect '%{public}s' does not support effectProperty settings.",
        result.effects[effectIdx].name.c_str());
    int32_t countProperty = 0;
    while (curNode->IsNodeValid()) {
        CHECK_AND_RETURN_LOG(countProperty < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of effectProperty nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        if (curNode->CompareName("effectProperty")) {
            std::string pModeStr;
            if (curNode->GetProp("mode", pModeStr) != SUCCESS) {
                AUDIO_WARNING_LOG("missing information: EFFECTPROPERTY has no MODE attribute");
            }
            result.effects[effectIdx].effectProperty.push_back(pModeStr);
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be effectProperty", curNode->GetName().c_str());
        }
        countProperty++;
        curNode->MoveToNext();
    }
    if (countProperty == 0) {
        AUDIO_WARNING_LOG("effect '%{public}s' does not support effectProperty settings.",
            result.effects[effectIdx].name.c_str());
    }
}

static void LoadEffect(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode)
{
    int32_t countEffect = 0;
    std::vector<std::string> effectProperty = {};
    int32_t effectIdx = 0;
    while (curNode->IsNodeValid()) {
        CHECK_AND_RETURN_LOG(countEffect < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of effect nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        if (curNode->CompareName("effect")) {
            std::string pEffectName;
            std::string pEffectLib;
            if (curNode->GetProp("name", pEffectName) != SUCCESS) {
                AUDIO_ERR_LOG("missing information: effect has no name attribute");
            }
            if (curNode->GetProp("library", pEffectLib) != SUCCESS) {
                AUDIO_ERR_LOG("missing information: effect has no library attribute");
            }
            Effect tmp = {pEffectName, pEffectLib, effectProperty};
            result.effects.push_back(tmp);
            LoadEffectProperty(result, curNode->GetCopyNode(), effectIdx);
            effectIdx++;
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be effect", curNode->GetName().c_str());
        }
        countEffect++;
        curNode->MoveToNext();
    }
    if (countEffect == 0) {
        AUDIO_WARNING_LOG("missing information: effects have no child effect");
    }
}

static void LoadEffectConfigEffects(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode,
                                    int32_t (&countFirstNode)[NODE_SIZE])
{
    if (countFirstNode[INDEX_EFFECS] >= AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
        if (countFirstNode[INDEX_EFFECS] == AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
            countFirstNode[INDEX_EFFECS]++;
            AUDIO_WARNING_LOG("the number of effects nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (curNode->GetChildrenNode()->IsNodeValid()) {
        LoadEffect(result, curNode->GetChildrenNode());
        countFirstNode[INDEX_EFFECS]++;
    } else {
        AUDIO_WARNING_LOG("missing information: effects have no child effect");
        countFirstNode[INDEX_EFFECS]++;
    }
}

static void LoadApply(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode, const int32_t segInx)
{
    curNode->MoveToChildren();
    CHECK_AND_RETURN_LOG(curNode->IsNodeValid(), "missing information: effectChain has no child apply");
    int32_t countApply = 0;
    while (curNode->IsNodeValid()) {
        CHECK_AND_RETURN_LOG(countApply < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of apply nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        if (curNode->CompareName("apply")) {
            std::string ppValue;
            if (curNode->GetProp("effect", ppValue) != SUCCESS) {
                AUDIO_WARNING_LOG("missing information: apply has no effect attribute");
            }
            result.effectChains[segInx].apply.push_back(ppValue);
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be apply", curNode->GetName().c_str());
        }
        countApply++;
        curNode->MoveToNext();
    }
    if (countApply == 0) {
        AUDIO_WARNING_LOG("missing information: effectChain has no child apply");
    }
}

static void LoadEffectChain(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode)
{
    int32_t countEffectChain = 0;
    int32_t segInx = 0;
    std::vector<std::string> apply;
    while (curNode->IsNodeValid()) {
        CHECK_AND_RETURN_LOG(countEffectChain < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of effectChain nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        if (curNode->CompareName("effectChain")) {
            std::string label = "";
            if (curNode->GetProp("label", label) != SUCCESS) {
                AUDIO_WARNING_LOG("missing information: effectChain has no label attribute");
            }
            std::string peffectChainName;
            if (curNode->GetProp("name", peffectChainName) != SUCCESS) {
                AUDIO_WARNING_LOG("missing information: effectChain has no name attribute");
            }
            EffectChain tmp = {peffectChainName, apply, label};
            result.effectChains.push_back(tmp);
            LoadApply(result, curNode->GetCopyNode(), segInx);
            segInx++;
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be effectChain", curNode->GetName().c_str());
        }
        countEffectChain++;
        curNode->MoveToNext();
    }
    if (countEffectChain == 0) {
        AUDIO_WARNING_LOG("missing information: effectChains have no child effectChain");
    }
}

static void LoadEffectConfigEffectChains(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode,
                                         int32_t (&countFirstNode)[NODE_SIZE])
{
    if (countFirstNode[INDEX_EFFECTCHAINE] >= AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
        if (countFirstNode[INDEX_EFFECTCHAINE] == AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
            countFirstNode[INDEX_EFFECTCHAINE]++;
            AUDIO_WARNING_LOG("the number of effectChains nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (curNode->GetChildrenNode()->IsNodeValid()) {
        LoadEffectChain(result, curNode->GetChildrenNode());
        countFirstNode[INDEX_EFFECTCHAINE]++;
    } else {
        AUDIO_WARNING_LOG("missing information: effectChains have no child effectChain");
        countFirstNode[INDEX_EFFECTCHAINE]++;
    }
}

static void LoadPreDevice(std::vector<Device> &devices, std::shared_ptr<AudioXmlNode> curNode)
{
    curNode->MoveToChildren();
    int32_t countDevice = 0;
    while (curNode->IsNodeValid()) {
        CHECK_AND_RETURN_LOG(countDevice < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of devicePort nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        if (curNode->CompareName("devicePort")) {
            std::string pDevType;
            std::string pChain;
            if (curNode->GetProp("type", pDevType) != SUCCESS) {
                AUDIO_ERR_LOG("missing information: devicePort has no type attribute");
            }
            if (curNode->GetProp("effectChain", pChain) != SUCCESS) {
                AUDIO_ERR_LOG("missing information: devicePort has no effectChain attribute");
            }
            Device tmpdev = {pDevType, pChain};
            devices.push_back(tmpdev);
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be devicePort", curNode->GetName().c_str());
        }
        countDevice++;
        curNode->MoveToNext();
    }
    if (countDevice == 0) {
        AUDIO_WARNING_LOG("missing information: streamEffectMode has no child devicePort");
    }
}

static void LoadPreMode(PreStreamScene &scene, std::shared_ptr<AudioXmlNode> curNode)
{
    curNode->MoveToChildren();
    int32_t countMode = 0;
    int32_t modeNum = 0;
    while (curNode->IsNodeValid()) {
        CHECK_AND_RETURN_LOG(countMode < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of streamEffectMode nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        if (curNode->CompareName("streamEffectMode")) {
            std::string pStreamAEMode;
            if (curNode->GetProp("mode", pStreamAEMode) != SUCCESS) {
                AUDIO_WARNING_LOG("missing information: streamEffectMode has no mode attribute");
            }
            scene.mode.push_back(pStreamAEMode);
            scene.device.push_back({});
            LoadPreDevice(scene.device[modeNum], curNode->GetCopyNode());
            modeNum++;
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be streamEffectMode", curNode->GetName().c_str());
        }
        countMode++;
        curNode->MoveToNext();
    }
    if (countMode == 0) {
        AUDIO_WARNING_LOG("missing information: stream has no child streamEffectMode");
    }
}

static void LoadPreStreamScenes(std::vector<PreStreamScene> &scenes, std::shared_ptr<AudioXmlNode> curNode)
{
    std::string stream;
    std::vector<std::string> mode;
    std::vector<std::vector<Device>> device;
    PreStreamScene tmp = {stream, mode, device};
    int32_t countPreprocess = 0;
    int32_t streamNum = 0;
    while (curNode->IsNodeValid()) {
        CHECK_AND_RETURN_LOG(countPreprocess < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of stream nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        if (curNode->CompareName("stream")) {
            std::string pStreamType;
            if (curNode->GetProp("scene", pStreamType) != SUCCESS) {
                AUDIO_WARNING_LOG("missing information: stream has no scene attribute");
            }
            tmp.stream = pStreamType;
            scenes.push_back(tmp);
            LoadPreMode(scenes[streamNum], curNode->GetCopyNode());
            streamNum++;
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be stream", curNode->GetName().c_str());
        }
        countPreprocess++;
        curNode->MoveToNext();
    }
    if (countPreprocess == 0) {
        AUDIO_WARNING_LOG("missing information: preProcess has no child stream");
    }
}

static void LoadPreStreamScenesCheck(std::vector<PreStreamScene> &scenes, std::shared_ptr<AudioXmlNode> curNode,
                                     int32_t &nodeCounter)
{
    if (nodeCounter >= AUDIO_EFFECT_COUNT_PRE_SECOND_NODE_UPPER_LIMIT) {
        if (nodeCounter == AUDIO_EFFECT_COUNT_PRE_SECOND_NODE_UPPER_LIMIT) {
            nodeCounter++;
            AUDIO_WARNING_LOG("the number of preprocessStreams nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_PRE_SECOND_NODE_UPPER_LIMIT);
        }
    } else if (curNode->GetChildrenNode()->IsNodeValid()) {
        LoadPreStreamScenes(scenes, curNode->GetChildrenNode());
        nodeCounter++;
    } else {
        AUDIO_WARNING_LOG("missing information: preprocessStreams has no child stream");
        nodeCounter++;
    }
}

static void LoadPreprocessExceptionCheck(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode,
                                         int32_t (&countPreSecondNode)[NODE_SIZE_PRE])
{
    if (countPreSecondNode[INDEX_PRE_EXCEPTION] >= AUDIO_EFFECT_COUNT_PRE_SECOND_NODE_UPPER_LIMIT) {
        if (countPreSecondNode[INDEX_PRE_EXCEPTION] == AUDIO_EFFECT_COUNT_PRE_SECOND_NODE_UPPER_LIMIT) {
            countPreSecondNode[INDEX_PRE_EXCEPTION]++;
            AUDIO_ERR_LOG("the number of postprocess nodes with wrong name exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_PRE_SECOND_NODE_UPPER_LIMIT);
        }
    } else {
        AUDIO_WARNING_LOG("wrong name: %{public}s", curNode->GetName().c_str());
        countPreSecondNode[INDEX_PRE_EXCEPTION]++;
    }
}

static void LoadPreProcessCfg(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode)
{
    int32_t countPreSecondNode[NODE_SIZE_PRE] = {0};
    while (curNode->IsNodeValid()) {
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }

        if (curNode->CompareName("defaultScene")) {
            LoadPreStreamScenesCheck(result.preProcess.defaultScenes, curNode->GetCopyNode(),
                countPreSecondNode[INDEX_PRE_DEFAULT_SCENE]);
        } else if (curNode->CompareName("priorScene")) {
            LoadPreStreamScenesCheck(result.preProcess.priorScenes, curNode->GetCopyNode(),
                countPreSecondNode[INDEX_PRE_PRIOR_SCENE]);
        } else if (curNode->CompareName("normalScene")) {
            std::string maxExtraNumStr;
            curNode->GetProp("maxExtSceneNumber", maxExtraNumStr);
            CHECK_AND_RETURN_LOG(StringConverter(maxExtraNumStr, result.preProcess.maxExtSceneNum),
                "convert maxExtraNumStr: %{public}s fail!", maxExtraNumStr.c_str());
            LoadPreStreamScenesCheck(result.preProcess.normalScenes, curNode->GetCopyNode(),
                countPreSecondNode[INDEX_PRE_NORMAL_SCENE]);
        } else {
            LoadPreprocessExceptionCheck(result, curNode->GetCopyNode(), countPreSecondNode);
        }
        curNode->MoveToNext();
    }
}

static void LoadEffectConfigPreProcessCfg(OriginalEffectConfig &result,
    std::shared_ptr<AudioXmlNode> curNode, int32_t (&countFirstNode)[NODE_SIZE])
{
    if (countFirstNode[INDEX_PREPROCESS] >= AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
        if (countFirstNode[INDEX_PREPROCESS] == AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
            countFirstNode[INDEX_PREPROCESS]++;
            AUDIO_WARNING_LOG("the number of preProcess nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (curNode->GetChildrenNode()->IsNodeValid()) {
        LoadPreProcessCfg(result, curNode->GetChildrenNode());
        countFirstNode[INDEX_PREPROCESS]++;
    } else {
        AUDIO_WARNING_LOG("missing information: preProcess has no child stream");
        countFirstNode[INDEX_PREPROCESS]++;
    }
}

static void LoadStreamUsageMapping(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode)
{
    SceneMappingItem tmp;
    int32_t countUsage = 0;
    while (curNode->IsNodeValid()) {
        CHECK_AND_RETURN_LOG(countUsage < AUDIO_EFFECT_COUNT_STREAM_USAGE_UPPER_LIMIT,
            "streamUsage map item exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_STREAM_USAGE_UPPER_LIMIT);
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        if (curNode->CompareName("streamUsage")) {
            if (curNode->GetProp("name", tmp.name) != SUCCESS) {
                AUDIO_WARNING_LOG("missing information: streamUsage misses name");
            }
            if (curNode->GetProp("scene", tmp.sceneType) != SUCCESS) {
                AUDIO_WARNING_LOG("missing information: streamUsage misses scene");
            }
            result.postProcess.sceneMap.push_back(tmp);
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be streamUsage", curNode->GetName().c_str());
        }
        countUsage++;
        curNode->MoveToNext();
    }
    if (countUsage == 0) {
        AUDIO_WARNING_LOG("missing information: sceneMap has no child streamUsage");
    }
}

static void LoadPostDevice(std::vector<Device> &devices, std::shared_ptr<AudioXmlNode> curNode)
{
    curNode->MoveToChildren();
    int32_t countDevice = 0;
    while (curNode->IsNodeValid()) {
        CHECK_AND_RETURN_LOG(countDevice < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of devicePort nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        if (curNode->CompareName("devicePort")) {
            std::string pDevType;
            std::string pChain;
            if (curNode->GetProp("type", pDevType) != SUCCESS) {
                AUDIO_WARNING_LOG("missing information: devicePort has no type attribute");
            }
            if (curNode->GetProp("effectChain", pChain) != SUCCESS) {
                AUDIO_WARNING_LOG("missing information: devicePort has no effectChain attribute");
            }
            Device tmpdev = {pDevType, pChain};
            devices.push_back(tmpdev);
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be devicePort", curNode->GetName().c_str());
        }
        countDevice++;
        curNode->MoveToNext();
    }
    if (countDevice == 0) {
        AUDIO_WARNING_LOG("missing information: streamEffectMode has no child devicePort");
    }
}

static void LoadPostMode(PostStreamScene &scene, std::shared_ptr<AudioXmlNode> curNode)
{
    curNode->MoveToChildren();
    int32_t countMode = 0;
    int32_t modeNum = 0;
    while (curNode->IsNodeValid()) {
        CHECK_AND_RETURN_LOG(countMode < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of streamEffectMode nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        if (curNode->CompareName("streamEffectMode")) {
            std::string pStreamAEMode;
            if (curNode->GetProp("mode", pStreamAEMode) != SUCCESS) {
                AUDIO_ERR_LOG("missing information: streamEffectMode has no mode attribute");
            }
            scene.mode.push_back(pStreamAEMode);
            scene.device.push_back({});
            LoadPostDevice(scene.device[modeNum], curNode->GetCopyNode());
            modeNum++;
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be streamEffectMode", curNode->GetName().c_str());
        }
        countMode++;
        curNode->MoveToNext();
    }
    if (countMode == 0) {
        AUDIO_WARNING_LOG("missing information: stream has no child streamEffectMode");
    }
}

static void LoadPostStreamScenes(std::vector<PostStreamScene> &scenes, std::shared_ptr<AudioXmlNode> curNode)
{
    std::string stream;
    std::vector<std::string> mode;
    std::vector<std::vector<Device>> device;
    PostStreamScene tmp = {stream, mode, device};
    int32_t countPostProcess = 0;
    int32_t streamNum = 0;
    while (curNode->IsNodeValid()) {
        CHECK_AND_RETURN_LOG(countPostProcess < AUDIO_EFFECT_COUNT_UPPER_LIMIT,
            "the number of stream nodes exceeds limit: %{public}d", AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }
        if (curNode->CompareName("stream")) {
            std::string pStreamType;
            if (curNode->GetProp("scene", pStreamType) != SUCCESS) {
                AUDIO_WARNING_LOG("missing information: stream has no scene attribute");
            }
            tmp.stream = pStreamType;
            scenes.push_back(tmp);
            LoadPostMode(scenes[streamNum], curNode->GetCopyNode());
            streamNum++;
        } else {
            AUDIO_WARNING_LOG("wrong name: %{public}s, should be stream", curNode->GetName().c_str());
        }
        countPostProcess++;
        curNode->MoveToNext();
    }
    if (countPostProcess == 0) {
        AUDIO_WARNING_LOG("missing information: postProcess has no child stream");
    }
}

static void LoadPostStreamScenesCheck(std::vector<PostStreamScene> &scenes, std::shared_ptr<AudioXmlNode> curNode,
                                      int32_t &nodeCounter)
{
    if (nodeCounter >= AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT) {
        if (nodeCounter == AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT) {
            nodeCounter++;
            AUDIO_WARNING_LOG("the number of postprocessStreams nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT);
        }
    } else if (curNode->GetChildrenNode()->IsNodeValid()) {
        LoadPostStreamScenes(scenes, curNode->GetChildrenNode());
        nodeCounter++;
    } else {
        AUDIO_WARNING_LOG("missing information: postprocessStreams has no child stream");
        nodeCounter++;
    }
}

static void LoadStreamUsageMappingCheck(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode,
                                        int32_t (&countPostSecondNode)[NODE_SIZE_POST])
{
    if (countPostSecondNode[INDEX_POST_MAPPING] >= AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT) {
        if (countPostSecondNode[INDEX_POST_MAPPING] == AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT) {
            countPostSecondNode[INDEX_POST_MAPPING]++;
            AUDIO_WARNING_LOG("the number of sceneMap nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT);
        }
    } else if (curNode->GetChildrenNode()->IsNodeValid()) {
        LoadStreamUsageMapping(result, curNode->GetChildrenNode());
        countPostSecondNode[INDEX_POST_MAPPING]++;
    } else {
        AUDIO_WARNING_LOG("missing information: sceneMap has no child stream");
        countPostSecondNode[INDEX_POST_MAPPING]++;
    }
}

static void LoadPostprocessExceptionCheck(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode,
                                          int32_t (&countPostSecondNode)[NODE_SIZE_POST])
{
    if (countPostSecondNode[INDEX_POST_EXCEPTION] >= AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT) {
        if (countPostSecondNode[INDEX_POST_EXCEPTION] == AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT) {
            countPostSecondNode[INDEX_POST_EXCEPTION]++;
            AUDIO_ERR_LOG("the number of postprocess nodes with wrong name exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT);
        }
    } else {
        AUDIO_WARNING_LOG("wrong name: %{public}s", curNode->GetName().c_str());
        countPostSecondNode[INDEX_POST_EXCEPTION]++;
    }
}

static void LoadPostProcessCfg(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode)
{
    int32_t countPostSecondNode[NODE_SIZE_POST] = {0};
    while (curNode->IsNodeValid()) {
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }

        if (curNode->CompareName("defaultScene")) {
            LoadPostStreamScenesCheck(result.postProcess.defaultScenes, curNode->GetCopyNode(),
                countPostSecondNode[INDEX_POST_DEFAULT_SCENE]);
        } else if (curNode->CompareName("priorScene")) {
            LoadPostStreamScenesCheck(result.postProcess.priorScenes, curNode->GetCopyNode(),
                countPostSecondNode[INDEX_POST_PRIOR_SCENE]);
        } else if (curNode->CompareName("normalScene")) {
            std::string maxExtraNumStr;
            curNode->GetProp("maxExtSceneNumber", maxExtraNumStr);
            CHECK_AND_RETURN_LOG(StringConverter(maxExtraNumStr, result.postProcess.maxExtSceneNum),
                "convert maxExtraNumStr: %{public}s fail!", maxExtraNumStr.c_str());
            LoadPostStreamScenesCheck(result.postProcess.normalScenes, curNode->GetCopyNode(),
                countPostSecondNode[INDEX_POST_NORMAL_SCENE]);
        } else if (curNode->CompareName("effectSceneStreams")) {
            // TO BE COMPATIBLE WITH OLDER VERSION XML
            LoadPostStreamScenesCheck(result.postProcess.normalScenes, curNode->GetCopyNode(),
                countPostSecondNode[INDEX_POST_NORMAL_SCENE]);
        } else if (curNode->CompareName("sceneMap")) {
            LoadStreamUsageMappingCheck(result, curNode->GetCopyNode(), countPostSecondNode);
        } else {
            LoadPostprocessExceptionCheck(result, curNode->GetCopyNode(), countPostSecondNode);
        }
        curNode->MoveToNext();
    }
}

static void LoadEffectConfigPostProcessCfg(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode,
                                           int32_t (&countFirstNode)[NODE_SIZE])
{
    if (countFirstNode[INDEX_POSTPROCESS] >= AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
        if (countFirstNode[INDEX_POSTPROCESS] == AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT) {
            countFirstNode[INDEX_POSTPROCESS]++;
            AUDIO_WARNING_LOG("the number of postProcess nodes exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT);
        }
    } else if (curNode->GetChildrenNode()->IsNodeValid()) {
        LoadPostProcessCfg(result, curNode->GetChildrenNode());
        countFirstNode[INDEX_POSTPROCESS]++;
    } else {
        AUDIO_WARNING_LOG("missing information: postProcess has no child stream");
        countFirstNode[INDEX_POSTPROCESS]++;
    }
}

static void LoadEffectConfigException(OriginalEffectConfig &result, std::shared_ptr<AudioXmlNode> curNode,
                                      int32_t (&countFirstNode)[NODE_SIZE])
{
    if (countFirstNode[INDEX_EXCEPTION] >= AUDIO_EFFECT_COUNT_UPPER_LIMIT) {
        if (countFirstNode[INDEX_EXCEPTION] == AUDIO_EFFECT_COUNT_UPPER_LIMIT) {
            countFirstNode[INDEX_EXCEPTION]++;
            AUDIO_ERR_LOG("the number of nodes with wrong name exceeds limit: %{public}d",
                AUDIO_EFFECT_COUNT_UPPER_LIMIT);
        }
    } else {
        AUDIO_WARNING_LOG("wrong name: %{public}s", curNode->GetName().c_str());
        countFirstNode[INDEX_EXCEPTION]++;
    }
}

int32_t AudioEffectConfigParser::LoadEffectConfig(OriginalEffectConfig &result)
{
    int32_t countFirstNode[NODE_SIZE] = {0};
    std::shared_ptr<AudioXmlNode> curNode = AudioXmlNode::Create();

    int32_t ret = ParseEffectConfigFile(curNode);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ret, "error: could not parse audio effect config file");

    if (LoadConfigCheck(curNode->GetCopyNode()) == 0) {
        curNode->GetProp("version", result.version);
        curNode->MoveToChildren();
    } else {
        return FILE_CONTENT_ERROR;
    }

    while (curNode->IsNodeValid()) {
        if (!curNode->IsElementNode()) {
            curNode->MoveToNext();
            continue;
        }

        if (curNode->CompareName("libraries")) {
            LoadEffectConfigLibraries(result, curNode->GetCopyNode(), countFirstNode);
        } else if (curNode->CompareName("effects")) {
            LoadEffectConfigEffects(result, curNode->GetCopyNode(), countFirstNode);
        } else if (curNode->CompareName("effectChains")) {
            LoadEffectConfigEffectChains(result, curNode->GetCopyNode(), countFirstNode);
        } else if (curNode->CompareName("preProcess")) {
            LoadEffectConfigPreProcessCfg(result, curNode->GetCopyNode(), countFirstNode);
        } else if (curNode->CompareName("postProcess")) {
            LoadEffectConfigPostProcessCfg(result, curNode->GetCopyNode(), countFirstNode);
        } else {
            LoadEffectConfigException(result, curNode->GetCopyNode(), countFirstNode);
        }
        curNode->MoveToNext();
    }

    for (int32_t i = 0; i < MODULE_SIZE; i++) {
        if (countFirstNode[i] == 0) {
            AUDIO_WARNING_LOG("missing information: %{public}s", EFFECT_CONFIG_NAME[i].c_str());
        }
    }

    curNode = nullptr;
    return 0;
}
} // namespace AudioStandard
} // namespace OHOS