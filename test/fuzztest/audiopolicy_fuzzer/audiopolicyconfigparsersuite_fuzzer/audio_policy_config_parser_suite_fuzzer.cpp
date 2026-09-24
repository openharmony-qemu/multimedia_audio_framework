/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

/* Combined fuzz test suite for all parsers under:
 * services/audio_policy/server/infra/config/parser/src/
 *
 * Covers 10 source files:
 *   audio_concurrency_parser.cpp, audio_converter_parser.cpp,
 *   audio_device_parser.cpp, audio_focus_parser.cpp,
 *   audio_policy_config_parser.cpp, audio_policy_global_parser.cpp,
 *   audio_source_strategy_parser.cpp, audio_tone_parser.cpp,
 *   audio_usage_strategy_parser.cpp, audio_volume_parser.cpp
 */

#include <cstdio>
#include <cstdint>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include "securec.h"
#include "audio_log.h"
#include "audio_xml_parser.h"

#include "audio_concurrency_parser.h"
#include "audio_converter_parser.h"
#include "audio_device_parser.h"
#include "audio_device_manager.h"
#include "audio_focus_parser.h"
#include "audio_policy_global_parser.h"
#include "audio_source_strategy_parser.h"
#include "audio_tone_parser.h"
#include "audio_usage_strategy_parser.h"
#include "audio_volume_parser.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {

bool StrToInt(const std::string &str, int32_t &value);
std::string Trim(const std::string &str);
std::vector<std::string> SplitAndTrim(const std::string &str);

static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)();

class FuzzAudioXmlNode : public AudioXmlNode {
public:
    int32_t configResult = SUCCESS;
    int32_t maxValidIterations = 1;
    bool isElementNodeVal = true;
    bool compareNameResult = true;
    int32_t getPropResult = SUCCESS;
    std::string getPropValue;
    std::string nodeName = "mock";
    std::map<std::string, std::string> propValues;
    std::map<std::string, int32_t> propResults;
    std::shared_ptr<AudioXmlNode> childNode;

    std::shared_ptr<AudioXmlNode> GetChildrenNode() override { return childNode; }
    std::shared_ptr<AudioXmlNode> GetCopyNode() override
    {
        auto copy = std::make_shared<FuzzAudioXmlNode>(*this);
        copy->maxValidIterations = 0;
        return copy;
    }
    int32_t Config(const char *fileName, const char *encoding, int32_t options) override { return configResult; }
    void MoveToNext() override {}
    void MoveToChildren() override {}
    bool IsNodeValid() override { return maxValidIterations-- > 0; }
    bool HasProp(const char *propName) override { return false; }
    int32_t GetProp(const char *propName, std::string &result) override
    {
        auto it = propValues.find(propName);
        result = (it != propValues.end()) ? it->second : getPropValue;
        auto rit = propResults.find(propName);
        return (rit != propResults.end()) ? rit->second : getPropResult;
    }
    int32_t GetContent(std::string &result) override { return SUCCESS; }
    std::string GetName() override { return nodeName; }
    bool CompareName(const char *propName) override { return compareNameResult; }
    bool IsElementNode() override { return isElementNodeVal; }
    void FreeDoc() override {}
    void FreeXmlString(char *str) override {}
};

template <class T>
uint32_t GetArrLength(T &arr)
{
    if (arr == nullptr) { return 0; }
    return sizeof(arr) / sizeof(arr[0]);
}

template <class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (g_dataSize < g_pos) { return object; }
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) { return object; }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) { return {}; }
    g_pos += objectSize;
    return object;
}

std::string GetFuzzString()
{
    std::string str;
    uint8_t len = GetData<uint8_t>();
    for (uint8_t i = 0; i < len; ++i) {
        char c = static_cast<char>(GetData<uint8_t>());
        str.push_back(c);
    }
    return str;
}

bool WriteTempXmlFile(const char *path, const char *content)
{
    FILE *f = fopen(path, "w");
    if (f == nullptr) { return false; }
    (void)fputs(content, f);
    (void)fclose(f);
    return true;
}

// ==================== audio_concurrency_parser.cpp ====================

void ConcLoadConfigBothConfigFailFuzzTest()
{
    auto parser = std::make_shared<AudioConcurrencyParser>();
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->configResult = ERROR;
    parser->curNode_ = mockNode;
    std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> concurrencyMap;
    parser->LoadConfig(concurrencyMap);
}

void ConcLoadConfigWrongTagFuzzTest()
{
    auto parser = std::make_shared<AudioConcurrencyParser>();
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->configResult = SUCCESS;
    mockNode->compareNameResult = false;
    parser->curNode_ = mockNode;
    std::map<std::pair<AudioPipeType, AudioPipeType>, ConcurrencyAction> concurrencyMap;
    parser->LoadConfig(concurrencyMap);
}

// ==================== audio_converter_parser.cpp ====================

void ConvLoadConfigParseFailFuzzTest()
{
    AudioConverterParser &parser = AudioConverterParser::GetInstance();
    parser.cfg_ = nullptr;
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 0;
    parser.curNode_ = mockNode;
    ConverterConfig result = parser.LoadConfig();
    (void)result;
}

void ConvChannelLayoutGetPropFailFuzzTest()
{
    AudioConverterParser &parser = AudioConverterParser::GetInstance();
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 1;
    mockNode->compareNameResult = true;
    mockNode->getPropResult = ERROR;
    ConverterConfig result;
    parser.LoadConfigChannelLayout(result, mockNode);
}

void ConvChannelLayoutUnsupportedLayoutFuzzTest()
{
    AudioConverterParser &parser = AudioConverterParser::GetInstance();
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 1;
    mockNode->compareNameResult = true;
    mockNode->getPropResult = SUCCESS;
    mockNode->getPropValue = "UNSUPPORTED_LAYOUT";
    ConverterConfig result;
    parser.LoadConfigChannelLayout(result, mockNode);
}

void ConvChannelLayoutWrongNameFuzzTest()
{
    AudioConverterParser &parser = AudioConverterParser::GetInstance();
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 1;
    mockNode->compareNameResult = false;
    mockNode->nodeName = "wrong_tag";
    ConverterConfig result;
    parser.LoadConfigChannelLayout(result, mockNode);
}

void ConvChannelLayoutNoChildFuzzTest()
{
    AudioConverterParser &parser = AudioConverterParser::GetInstance();
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 0;
    ConverterConfig result;
    parser.LoadConfigChannelLayout(result, mockNode);
}

// ==================== audio_device_parser.cpp ====================

void DevLoadConfigurationFuzzTest()
{
    AudioDeviceParser parser(&AudioDeviceManager::GetAudioDeviceManager());
    parser.LoadConfiguration();
    parser.Destroy();
}

void DevLoadConfigurationWithMockFuzzTest()
{
    AudioDeviceParser parser(&AudioDeviceManager::GetAudioDeviceManager());
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 1;
    mockNode->compareNameResult = true;
    mockNode->isElementNodeVal = true;
    parser.curNode_ = mockNode;
    std::shared_ptr<AudioXmlNode> copyNode = parser.curNode_->GetCopyNode();
    if (copyNode != nullptr) {
        parser.ParseInternal(copyNode);
    }
    parser.curNode_ = nullptr;
}

// ==================== audio_focus_parser.cpp ====================

void FocusLoadConfigConfigFailFuzzTest()
{
    AudioFocusParser parser;
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->configResult = ERROR;
    parser.curNode_ = mockNode;
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> focusMap;
    parser.LoadConfig(focusMap);
}

void FocusLoadConfigWrongTagFuzzTest()
{
    AudioFocusParser parser;
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->configResult = SUCCESS;
    mockNode->maxValidIterations = 1;
    mockNode->compareNameResult = false;
    parser.curNode_ = mockNode;
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> focusMap;
    parser.LoadConfig(focusMap);
}

void FocusLoadDefaultConfigDirectFuzzTest()
{
    AudioFocusParser parser;
    std::map<std::pair<AudioFocusType, AudioFocusType>, AudioFocusEntry> focusMap;
    parser.LoadDefaultConfig(focusMap);
}

void FocusWriteConfigErrorEventDirectFuzzTest()
{
    AudioFocusParser parser;
    parser.WriteConfigErrorEvent();
}

// ==================== audio_policy_config_parser.cpp (StrToInt) ====================

void CfgStrToIntFuzzTest()
{
    std::string str = GetFuzzString();
    int32_t value = 0;
    StrToInt(str, value);
}

void CfgStrToIntEmptyFuzzTest()
{
    int32_t value = 0;
    StrToInt("", value);
}

void CfgStrToIntValidFuzzTest()
{
    int32_t value = 0;
    StrToInt("12345", value);
}

void CfgStrToIntOutOfRangeFuzzTest()
{
    int32_t value = 0;
    StrToInt("99999999999", value);
}

void CfgStrToIntNonNumericFuzzTest()
{
    int32_t value = 0;
    StrToInt("abc", value);
}

// ==================== audio_policy_global_parser.cpp ====================

void GlobParseMissingChildrenFuzzTest()
{
    AudioPolicyGlobalParser parser;
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    CHECK_AND_RETURN(doc != nullptr);
    xmlNodePtr root = xmlNewNode(nullptr, BAD_CAST "global");
    if (root != nullptr) {
        xmlDocSetRootElement(doc, root);
        parser.mDoc_ = doc;
        parser.Parse();
    }
    parser.mDoc_ = nullptr;
    xmlFreeDoc(doc);
}

void GlobParserAttributeMissingNameFuzzTest()
{
    AudioPolicyGlobalParser parser;
    xmlNodePtr node = xmlNewNode(nullptr, BAD_CAST "attribute");
    CHECK_AND_RETURN(node != nullptr);
    node->type = XML_ELEMENT_NODE;
    xmlNewProp(node, BAD_CAST "value", BAD_CAST "123");
    parser.ParserAttribute(node);
    xmlFreeNode(node);
}

void GlobParserAttributeMissingValueFuzzTest()
{
    AudioPolicyGlobalParser parser;
    xmlNodePtr node = xmlNewNode(nullptr, BAD_CAST "attribute");
    CHECK_AND_RETURN(node != nullptr);
    node->type = XML_ELEMENT_NODE;
    xmlNewProp(node, BAD_CAST "name", BAD_CAST "testName");
    parser.ParserAttribute(node);
    xmlFreeNode(node);
}

void GlobGetConfigByKeyNameNotFoundFuzzTest()
{
    AudioPolicyGlobalParser parser;
    uint32_t value = 0;
    parser.GetConfigByKeyName("nonexistent_key", value);
}

// ==================== audio_source_strategy_parser.cpp ====================

void SrcStratDestructorFuzzTest()
{
    auto parser = std::make_shared<AudioSourceStrategyParser>();
}

void SrcStratLoadConfigConfigFailFuzzTest()
{
    auto parser = std::make_shared<AudioSourceStrategyParser>();
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->configResult = ERROR;
    parser->curNode_ = mockNode;
    parser->LoadConfig();
}

void SrcStratLoadConfigWrongTagFuzzTest()
{
    auto parser = std::make_shared<AudioSourceStrategyParser>();
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->configResult = SUCCESS;
    mockNode->maxValidIterations = 1;
    mockNode->compareNameResult = false;
    parser->curNode_ = mockNode;
    parser->LoadConfig();
}

void SrcStratParseConfigNullMapFuzzTest()
{
    auto parser = std::make_shared<AudioSourceStrategyParser>();
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    std::shared_ptr<std::map<SourceType, AudioSourceStrategyType>> nullMap = nullptr;
    parser->ParseConfig(mockNode, nullMap);
}

void SrcStratAddSourceStrategyMapEmptyParamFuzzTest()
{
    auto parser = std::make_shared<AudioSourceStrategyParser>();
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    auto strategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    parser->AddSourceStrategyMap(mockNode, "", "hdiSource", strategyMap);
}

void SrcStratAddSourceStrategyMapUnknownSourceFuzzTest()
{
    auto parser = std::make_shared<AudioSourceStrategyParser>();
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    auto strategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    parser->AddSourceStrategyMap(mockNode, "UNKNOWN_SOURCE", "hdiSource", strategyMap);
}

void SrcStratAddSourceStrategyMapUnknownFlagFuzzTest()
{
    auto parser = std::make_shared<AudioSourceStrategyParser>();
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->propValues["audioFlag"] = "UNKNOWN_FLAG";
    auto strategyMap = std::make_shared<std::map<SourceType, AudioSourceStrategyType>>();
    parser->AddSourceStrategyMap(mockNode, "SOURCE_TYPE_MIC", "hdiSource", strategyMap);
}

// ==================== audio_tone_parser.cpp ====================

void ToneTrimFuzzTest()
{
    Trim("   ");
    Trim("");
}

void ToneSplitAndTrimFuzzTest()
{
    SplitAndTrim("   ");
    SplitAndTrim("");
}

void ToneLoadNewConfigWrongTagFuzzTest()
{
    const char *path = "/data/local/tmp/fuzz_tone_wrong_tag.xml";
    if (!WriteTempXmlFile(path, "<?xml version=\"1.0\"?><NotDTMF/>")) { return; }
    AudioToneParser parser;
    ToneInfoMap toneMap;
    unordered_map<string, ToneInfoMap> customMap;
    parser.LoadNewConfig(path, toneMap, customMap);
    (void)remove(path);
}

void ToneLoadNewConfigNoChildFuzzTest()
{
    const char *path = "/data/local/tmp/fuzz_tone_no_child.xml";
    if (!WriteTempXmlFile(path, "<?xml version=\"1.0\"?><DTMF/>")) { return; }
    AudioToneParser parser;
    ToneInfoMap toneMap;
    unordered_map<string, ToneInfoMap> customMap;
    parser.LoadNewConfig(path, toneMap, customMap);
    (void)remove(path);
}

void ToneLoadConfigFuzzTest()
{
    AudioToneParser parser;
    ToneInfoMap toneMap;
    parser.LoadConfig(toneMap);
}

void ToneParseCustomGetPropFailFuzzTest()
{
    AudioToneParser parser;
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 1;
    mockNode->isElementNodeVal = true;
    mockNode->compareNameResult = true;
    mockNode->getPropResult = ERROR;
    unordered_map<string, ToneInfoMap> customMap;
    parser.ParseCustom(mockNode, customMap);
}

void ToneParseToneInfoNotToneInfoFuzzTest()
{
    AudioToneParser parser;
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 1;
    mockNode->isElementNodeVal = true;
    mockNode->compareNameResult = false;
    vector<ToneInfoMap *> maps;
    parser.ParseToneInfo(mockNode, maps);
}

void ToneParseToneInfoGetPropFailFuzzTest()
{
    AudioToneParser parser;
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 1;
    mockNode->isElementNodeVal = true;
    mockNode->compareNameResult = true;
    mockNode->getPropResult = ERROR;
    vector<ToneInfoMap *> maps;
    parser.ParseToneInfo(mockNode, maps);
}

// ==================== audio_usage_strategy_parser.cpp ====================

void UsageLoadConfigurationFuzzTest()
{
    AudioUsageStrategyParser parser;
    parser.LoadConfiguration();
    parser.Destroy();
}

void UsageParseInternalWithMockFuzzTest()
{
    AudioUsageStrategyParser parser;
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 1;
    mockNode->compareNameResult = false;
    parser.ParseInternal(mockNode);
    parser.curNode_ = nullptr;
}

// ==================== audio_volume_parser.cpp ====================

void VolParseVolumeConfigFailFuzzTest()
{
    AudioVolumeParser parser;
    StreamVolumeInfoMap map;
    parser.ParseVolumeConfig("/nonexistent/path/volume.xml", map);
}

void VolParseVolumeConfigWrongTagFuzzTest()
{
    const char *path = "/data/local/tmp/fuzz_vol_wrong_tag.xml";
    if (!WriteTempXmlFile(path, "<?xml version=\"1.0\"?><NotVolumeConfig/>")) { return; }
    AudioVolumeParser parser;
    StreamVolumeInfoMap map;
    parser.ParseVolumeConfig(path, map);
    (void)remove(path);
}

void VolParseVolumeConfigNoChildFuzzTest()
{
    const char *path = "/data/local/tmp/fuzz_vol_no_child.xml";
    if (!WriteTempXmlFile(path, "<?xml version=\"1.0\"?><audio_volume_config/>")) { return; }
    AudioVolumeParser parser;
    StreamVolumeInfoMap map;
    parser.ParseVolumeConfig(path, map);
    (void)remove(path);
}

void VolUseVoiceAssistantNotFoundFuzzTest()
{
    AudioVolumeParser parser;
    StreamVolumeInfoMap map;
    parser.UseVoiceAssistantFixedVolumeConfig(map);
}

void VolUseVoiceAssistantDeviceNotFoundFuzzTest()
{
    AudioVolumeParser parser;
    StreamVolumeInfoMap map;
    auto info = std::make_shared<StreamVolumeInfo>();
    info->streamType = STREAM_VOICE_ASSISTANT;
    map[STREAM_VOICE_ASSISTANT] = info;
    parser.UseVoiceAssistantFixedVolumeConfig(map);
}

void VolUseVoiceAssistantEmptyVolumePointsFuzzTest()
{
    AudioVolumeParser parser;
    StreamVolumeInfoMap map;
    auto info = std::make_shared<StreamVolumeInfo>();
    info->streamType = STREAM_VOICE_ASSISTANT;
    auto devInfo = std::make_shared<DeviceVolumeInfo>();
    devInfo->deviceType = SPEAKER_VOLUME_TYPE;
    info->deviceVolumeInfos[SPEAKER_VOLUME_TYPE] = devInfo;
    map[STREAM_VOICE_ASSISTANT] = info;
    parser.UseVoiceAssistantFixedVolumeConfig(map);
}

void VolWriteVolumeConfigErrorEventFuzzTest()
{
    AudioVolumeParser parser;
    parser.WriteVolumeConfigErrorEvent();
}

void VolLoadConfigFuzzTest()
{
    AudioVolumeParser parser;
    StreamVolumeInfoMap map;
    parser.LoadConfig(map);
}

void VolLoadDefaultAppVolumeConfigFuzzTest()
{
    AudioVolumeParser parser;
    StreamVolumeInfoMap map;
    parser.LoadDefaultAppVolumeConfig(map);
}

void VolParseAppVolumeConfigFuzzTest()
{
    AudioVolumeParser parser;
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 1;
    mockNode->compareNameResult = true;
    mockNode->propValues["type"] = "APP";
    mockNode->propValues["minidx"] = "0";
    mockNode->propValues["maxidx"] = "15";
    mockNode->propValues["defaultidx"] = "5";
    StreamVolumeInfoMap map;
    parser.ParseAppVolumeConfig(mockNode, map);
}

void VolParseStreamVolumeInfoAttrVoicePCFuzzTest()
{
    AudioVolumeParser parser;
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 1;
    mockNode->propValues["type"] = "VOICE_PC";
    auto streamVolInfo = std::make_shared<StreamVolumeInfo>();
    parser.ParseStreamVolumeInfoAttr(mockNode, streamVolInfo);
}

void VolParseVolumeFixInfoEnableFuzzTest()
{
    AudioVolumeParser parser;
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 1;
    mockNode->propValues["type"] = "VOLUME_FIX_ENABLE";
    mockNode->propValues["enable"] = "1";
    parser.ParseVolumeFixInfo(mockNode);
}

void VolParseVolumeFixInfoLegacyFuzzTest()
{
    AudioVolumeParser parser;
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 1;
    mockNode->propValues["type"] = "LEGACY_HIGH_VOLUME_LEVEL_TO_NOTIFY";
    mockNode->propValues["enable"] = "5";
    parser.ParseVolumeFixInfo(mockNode);
}

void VolParseLowerVolumeInfoInvalidTypeFuzzTest()
{
    AudioVolumeParser parser;
    auto childMock = std::make_shared<FuzzAudioXmlNode>();
    childMock->maxValidIterations = 1;
    childMock->compareNameResult = true;
    childMock->propValues["type"] = "INVALID_TYPE";
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 1;
    mockNode->childNode = childMock;
    parser.ParseLowerVolumeInfo(mockNode);
}

void VolParseDeviceVolumeInfosMinMaxFuzzTest()
{
    AudioVolumeParser parser;
    auto mockNode = std::make_shared<FuzzAudioXmlNode>();
    mockNode->maxValidIterations = 1;
    mockNode->compareNameResult = true;
    mockNode->propValues["deviceClass"] = "speaker";
    mockNode->propValues["minidx"] = "1";
    mockNode->propValues["maxidx"] = "15";
    auto streamVolInfo = std::make_shared<StreamVolumeInfo>();
    parser.ParseDeviceVolumeInfos(mockNode, streamVolInfo);
}

// ==================== Dispatch Table ====================

TestPtr g_testPtrs[] = {
    // audio_concurrency_parser.cpp
    ConcLoadConfigBothConfigFailFuzzTest,
    ConcLoadConfigWrongTagFuzzTest,
    // audio_converter_parser.cpp
    ConvLoadConfigParseFailFuzzTest,
    ConvChannelLayoutGetPropFailFuzzTest,
    ConvChannelLayoutUnsupportedLayoutFuzzTest,
    ConvChannelLayoutWrongNameFuzzTest,
    ConvChannelLayoutNoChildFuzzTest,
    // audio_device_parser.cpp
    DevLoadConfigurationFuzzTest,
    DevLoadConfigurationWithMockFuzzTest,
    // audio_focus_parser.cpp
    FocusLoadConfigConfigFailFuzzTest,
    FocusLoadConfigWrongTagFuzzTest,
    FocusLoadDefaultConfigDirectFuzzTest,
    FocusWriteConfigErrorEventDirectFuzzTest,
    // audio_policy_config_parser.cpp (StrToInt)
    CfgStrToIntFuzzTest,
    CfgStrToIntEmptyFuzzTest,
    CfgStrToIntValidFuzzTest,
    CfgStrToIntOutOfRangeFuzzTest,
    CfgStrToIntNonNumericFuzzTest,
    // audio_policy_global_parser.cpp
    GlobParseMissingChildrenFuzzTest,
    GlobParserAttributeMissingNameFuzzTest,
    GlobParserAttributeMissingValueFuzzTest,
    GlobGetConfigByKeyNameNotFoundFuzzTest,
    // audio_source_strategy_parser.cpp
    SrcStratDestructorFuzzTest,
    SrcStratLoadConfigConfigFailFuzzTest,
    SrcStratLoadConfigWrongTagFuzzTest,
    SrcStratParseConfigNullMapFuzzTest,
    SrcStratAddSourceStrategyMapEmptyParamFuzzTest,
    SrcStratAddSourceStrategyMapUnknownSourceFuzzTest,
    SrcStratAddSourceStrategyMapUnknownFlagFuzzTest,
    // audio_tone_parser.cpp
    ToneTrimFuzzTest,
    ToneSplitAndTrimFuzzTest,
    ToneLoadNewConfigWrongTagFuzzTest,
    ToneLoadNewConfigNoChildFuzzTest,
    ToneLoadConfigFuzzTest,
    ToneParseCustomGetPropFailFuzzTest,
    ToneParseToneInfoNotToneInfoFuzzTest,
    ToneParseToneInfoGetPropFailFuzzTest,
    // audio_usage_strategy_parser.cpp
    UsageLoadConfigurationFuzzTest,
    UsageParseInternalWithMockFuzzTest,
    // audio_volume_parser.cpp
    VolParseVolumeConfigFailFuzzTest,
    VolParseVolumeConfigWrongTagFuzzTest,
    VolParseVolumeConfigNoChildFuzzTest,
    VolUseVoiceAssistantNotFoundFuzzTest,
    VolUseVoiceAssistantDeviceNotFoundFuzzTest,
    VolUseVoiceAssistantEmptyVolumePointsFuzzTest,
    VolWriteVolumeConfigErrorEventFuzzTest,
    VolLoadConfigFuzzTest,
    VolLoadDefaultAppVolumeConfigFuzzTest,
    VolParseAppVolumeConfigFuzzTest,
    VolParseStreamVolumeInfoAttrVoicePCFuzzTest,
    VolParseVolumeFixInfoEnableFuzzTest,
    VolParseVolumeFixInfoLegacyFuzzTest,
    VolParseLowerVolumeInfoInvalidTypeFuzzTest,
    VolParseDeviceVolumeInfosMinMaxFuzzTest,
};

void FuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr) { return; }
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;
    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testPtrs);
    if (len > 0) {
        g_testPtrs[code % len]();
    }
}

} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) { return 0; }
    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
