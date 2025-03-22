/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_effect_config_parser_unit_test.h"
#include "audio_policy_log.h"
#include "audio_policy_log.h"
#include "audio_errors.h"
#include "audio_effect.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <thread>
#include <string>
#include <memory>
#include <vector>
#include <sys/socket.h>
#include <cerrno>
#include <fstream>
#include <algorithm>
using namespace std;
using namespace std::chrono;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
static constexpr int32_t AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT = 1;
static constexpr int32_t AUDIO_EFFECT_COUNT_POST_SECOND_NODE_UPPER_LIMIT = 1;
static constexpr int32_t AUDIO_EFFECT_COUNT_PRE_SECOND_NODE_UPPER_LIMIT = 1;

void AudioEffectConfigParserUnitTest::SetUpTestCase(void) {}
void AudioEffectConfigParserUnitTest::TearDownTestCase(void) {}
void AudioEffectConfigParserUnitTest::SetUp(void) {}
void AudioEffectConfigParserUnitTest::TearDown(void) {}

#define PRINT_LINE printf("debug __LINE__:%d\n", __LINE__)

std::string createTempFile(const char* content)
{
    std::string tempFile = "/tmp/test_effect_config.xml";
    std::ofstream out(tempFile);
    out << content;
    out.close();
    return tempFile;
}

void removeTempFile(const std::string& tempFile)
{
    std::remove(tempFile.c_str());
}

xmlDocPtr createXmlDocument()
{
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    xmlNodePtr root = xmlNewNode(nullptr, BAD_CAST "root");
    xmlDocSetRootElement(doc, root);
    return doc;
}

void cleanupXmlDocument(xmlDocPtr doc)
{
    if (doc) {
        xmlFreeDoc(doc);
    }
}

/**
* @tc.name : Test LoadEffectConfigLibraries
* @tc.number: AudioEffectConfigParserTest_001
* @tc.desc : Test libraries node parsing in effect config xml
*/
HWTEST(AudioEffectConfigParserUnitTest, AudioEffectConfigParserTest_001, TestSize.Level1)
{
    OriginalEffectConfig result;
    xmlNode *currNode = xmlNewNode(nullptr, BAD_CAST "libraries");
    int32_t countFirstNode[NODE_SIZE_PRE] = {0};

    // Scenario 1: The number of libraries nodes exceeds the limit
    countFirstNode[INDEX_PRE_DEFAULT_SCENE] = AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT;
    AudioEffectConfigParser parser;
    parser.LoadEffectConfig(result);
    EXPECT_NE(countFirstNode[INDEX_PRE_DEFAULT_SCENE], AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT + 1);

    // Scenario 2: Valid libraries node (with child nodes)
    countFirstNode[INDEX_PRE_DEFAULT_SCENE] = 0;
    currNode->xmlChildrenNode = xmlNewNode(nullptr, BAD_CAST "library");
    parser.LoadEffectConfig(result);
    EXPECT_NE(countFirstNode[INDEX_PRE_DEFAULT_SCENE], 1);
    EXPECT_TRUE(result.libraries.empty());

    // Scenario 3: Invalid libraries node (no child nodes)
    countFirstNode[INDEX_PRE_DEFAULT_SCENE] = 0;
    currNode->xmlChildrenNode = nullptr;
    parser.LoadEffectConfig(result);
    EXPECT_NE(countFirstNode[INDEX_PRE_DEFAULT_SCENE], 1);

    // Clear
    if (currNode->xmlChildrenNode) {
        xmlFreeNode(currNode->xmlChildrenNode);
    }
    xmlFreeNode(currNode);
}

/**
* @tc.name : Test LoadEffectConfig
* @tc.number: AudioEffectConfigParser_002
* @tc.desc : Test libraries node parsing in effect config xml
*/
HWTEST(AudioEffectConfigParserUnitTest, AudioEffectConfigParser_002, TestSize.Level1)
{
    OriginalEffectConfig result;
    xmlNode *currNode = xmlNewNode(nullptr, BAD_CAST "libraries");
    int32_t countFirstNode[NODE_SIZE_PRE] = {0};

    // Scenario 1: The number of libraries nodes exceeds the limit
    countFirstNode[INDEX_PRE_DEFAULT_SCENE] = AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT;
    AudioEffectConfigParser parser;
    parser.LoadEffectConfig(result);
    EXPECT_NE(countFirstNode[INDEX_PRE_DEFAULT_SCENE], AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT + 1);

    // Scenario 2: Valid libraries node (with child nodes)
    countFirstNode[INDEX_PRE_DEFAULT_SCENE] = 0;
    currNode->xmlChildrenNode = xmlNewNode(nullptr, BAD_CAST "library");
    parser.LoadEffectConfig(result);
    EXPECT_NE(countFirstNode[INDEX_PRE_DEFAULT_SCENE], 1);
    EXPECT_TRUE(result.libraries.empty());

    // Scenario 3: Invalid libraries node (no child nodes)
    countFirstNode[INDEX_PRE_DEFAULT_SCENE] = 0;
    currNode->xmlChildrenNode = nullptr;
    parser.LoadEffectConfig(result);
    EXPECT_NE(countFirstNode[INDEX_PRE_DEFAULT_SCENE], 1);

    // Clear
    if (currNode->xmlChildrenNode) {
        xmlFreeNode(currNode->xmlChildrenNode);
    }
    xmlFreeNode(currNode);
}


/**
* @tc.name : Test LoadEffectConfig
* @tc.number: AudioEffectConfigParser_003
* @tc.desc : Test The number of effects nodes exceeds the limit
*/
HWTEST(AudioEffectConfigParserUnitTest, AudioEffectConfigParser_003, TestSize.Level1)
{
    OriginalEffectConfig result;

    // Create XML doc
    xmlDocPtr doc = createXmlDocument();
    xmlNodePtr root = xmlDocGetRootElement(doc);

    int32_t countFirstNode[NODE_SIZE_POST] = {0};
    countFirstNode[INDEX_POST_NORMAL_SCENE] = AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT;

    for (int i = 0; i <= AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT; ++i) {
        xmlNewChild(root, nullptr, BAD_CAST "effects", nullptr);
    }

    AudioEffectConfigParser parser;
    parser.LoadEffectConfig(result);

    EXPECT_NE(countFirstNode[INDEX_POST_NORMAL_SCENE], AUDIO_EFFECT_COUNT_FIRST_NODE_UPPER_LIMIT + 1);
    cleanupXmlDocument(doc);
}

/**
* @tc.name : Test LoadEffectConfig
* @tc.number: AudioEffectConfigParser_004
* @tc.desc : Test Valid effects node (with child nodes)
*/
HWTEST(AudioEffectConfigParserUnitTest, AudioEffectConfigParser_004, TestSize.Level1)
{
    OriginalEffectConfig result;

    xmlDocPtr doc = createXmlDocument();
    xmlNodePtr root = xmlDocGetRootElement(doc);

    int32_t countFirstNode[NODE_SIZE_POST] = {0};
    countFirstNode[INDEX_POST_NORMAL_SCENE] = 0;

    xmlNodePtr effectsNode = xmlNewChild(root, nullptr, BAD_CAST "effects", nullptr);
    xmlNewChild(effectsNode, nullptr, BAD_CAST "effect", nullptr);

    AudioEffectConfigParser parser;
    parser.LoadEffectConfig(result);

    EXPECT_NE(countFirstNode[INDEX_POST_NORMAL_SCENE], 1);
    cleanupXmlDocument(doc);
}

/**
* @tc.name : Test LoadEffectConfig
* @tc.number: AudioEffectConfigParser_005
* @tc.desc : Test Scenario 3: Invalid effects node (no child nodes)
*/
HWTEST(AudioEffectConfigParserUnitTest, AudioEffectConfigParser_005, TestSize.Level1)
{
    OriginalEffectConfig result;

    xmlDocPtr doc = createXmlDocument();
    xmlNodePtr root = xmlDocGetRootElement(doc);

    int32_t countFirstNode[NODE_SIZE_POST] = {0};
    countFirstNode[INDEX_POST_NORMAL_SCENE] = 0;
    xmlNewChild(root, nullptr, BAD_CAST "effects", nullptr);
    AudioEffectConfigParser parser;
    parser.LoadEffectConfig(result);
    EXPECT_NE(countFirstNode[INDEX_POST_NORMAL_SCENE], 1);
    cleanupXmlDocument(doc);
}

/**
* @tc.name: Test LoadEffectConfig with different XML files
* @tc.number: AudioEffectConfigParser_006
* @tc.desc: Test LoadEffectConfig function with various XML configurations for effects
*/
HWTEST(AudioEffectConfigParserUnitTest, AudioEffectConfigParser_006, TestSize.Level1)
{
    OriginalEffectConfig result;
    AudioEffectConfigParser parser;

    int32_t ret = parser.LoadEffectConfig(result);
    EXPECT_NE(ret, 0);
    EXPECT_NE(result.effects.size(), 1);

    result = OriginalEffectConfig();

    ret = parser.LoadEffectConfig(result);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(result.effects.empty());

    result = OriginalEffectConfig();

    ret = parser.LoadEffectConfig(result);
    EXPECT_NE(ret, 0);
    EXPECT_EQ(result.effects.size(), 0);
}

/**
* @tc.name: Test LoadEffectConfig for EffectChains Processing
* @tc.number: AudioEffectConfigParser_007
* @tc.desc: Test LoadEffectConfig function focusing on effectChains node processing branch
*/
HWTEST(AudioEffectConfigParserUnitTest, AudioEffectConfigParser_007, TestSize.Level1)
{
    OriginalEffectConfig result;
    AudioEffectConfigParser parser;

    // test1: test normal effectChains
    const std::string normalXml = R"(<?xml version="1.0" encoding="UTF-8"?>
        <audio_effects_config version="1.0">
            <effectChains>
                <effectChain name="chain1" label="label1">
                    <apply>effect1</apply>
                    <apply>effect2</apply>
                </effectChain>
            </effectChains>
        </audio_effects_config>)";

    std::ofstream file("/etc/audio_effect.xml");
    file << normalXml;
    file.close();

    int32_t ret = parser.LoadEffectConfig(result);
    EXPECT_NE(ret, 0);
    EXPECT_NE(result.effectChains.size(), 1);
}


/**
* @tc.name: Test LoadEffectConfig for EffectChains Processing
* @tc.number: AudioEffectConfigParser_008
* @tc.desc: Test LoadEffectConfig function focusing on effectChains node processing branch
*/
HWTEST(AudioEffectConfigParserUnitTest, AudioEffectConfigParser_008, TestSize.Level1)
{
    OriginalEffectConfig result;
    AudioEffectConfigParser parser;

    const std::string emptyXml = R"(<?xml version="1.0" encoding="UTF-8"?>
        <audio_effects_config version="1.0">
            <effectChains>
            </effectChains>
        </audio_effects_config>)";

    std::ofstream file("/etc/audio_effect.xml");
    file << emptyXml;
    file.close();

    int32_t ret = parser.LoadEffectConfig(result);
    EXPECT_NE(ret, 0);
    EXPECT_TRUE(result.effectChains.empty());
}

/**
* @tc.name: Test LoadEffectConfig for EffectChains Processing
* @tc.number: AudioEffectConfigParser_009
* @tc.desc: Test LoadEffectConfig function focusing on effectChains node processing branch
*/
HWTEST(AudioEffectConfigParserUnitTest, AudioEffectConfigParser_009, TestSize.Level1)
{
    OriginalEffectConfig result;
    AudioEffectConfigParser parser;

    //test2: Exceed limit effectChains
    const std::string multipleXml = R"(<?xml version="1.0" encoding="UTF-8"?>
        <audio_effects_config version="1.0">
            <effectChains>
                <effectChain name="chain1">
                    <apply>effect1</apply>
                </effectChain>
                <effectChain name="chain2">
                    <apply>effect2</apply>
                </effectChain>
            </effectChains>
        </audio_effects_config>)";

    std::ofstream file("/etc/audio_effect.xml");
    file << multipleXml;
    file.close();

    int32_t ret = parser.LoadEffectConfig(result);
    EXPECT_NE(ret, 0);
    EXPECT_NE(result.effectChains.size(), 1);
}

/**
* @tc.name: Test LoadEffectConfig for Exception Node Processing
* @tc.number: AAudioEffectConfigParser_010
* @tc.desc: Test LoadEffectConfig function focusing on handling unknown XML nodes
*/
HWTEST(AudioEffectConfigParserUnitTest, AudioEffectConfigParser_010, TestSize.Level1)
{
    OriginalEffectConfig result;
    AudioEffectConfigParser parser;

    // singal unknown node
    const std::string singleUnknownXml = R"(<?xml version="1.0" encoding="UTF-8"?>
        <audio_effects_config version="1.0">
            <libraries>
                <library name="bundle" path="lib/bundle.so"/>
            </libraries>
            <unknownNode>
                <someChild>value</someChild>
            </unknownNode>
        </audio_effects_config>)";

    std::ofstream file("/etc/audio_effect.xml");
    file << singleUnknownXml;
    file.close();

    int32_t ret = parser.LoadEffectConfig(result);
    EXPECT_NE(ret, 0);
}

/**
* @tc.name: Test LoadEffectConfig for Exception Node Processing
* @tc.number: AAudioEffectConfigParser_009
* @tc.desc: Test LoadEffectConfig function focusing on handling unknown XML nodes
*/
HWTEST(AudioEffectConfigParserUnitTest, AudioEffectConfigParser_011, TestSize.Level1)
{
    OriginalEffectConfig result;
    AudioEffectConfigParser parser;

    // test3: Multiple unknown nodes, but the upper limit is not exceeded
    const std::string multipleUnknownXml = R"(<?xml version="1.0" encoding="UTF-8"?>
        <audio_effects_config version="1.0">
            <libraries>
                <library name="bundle" path="lib/bundle.so"/>
            </libraries>
            <unknownNode1>value1</unknownNode1>
            <unknownNode2>value2</unknownNode2>
            <unknownNode3>value3</unknownNode3>
        </audio_effects_config>)";

    std::ofstream file("/etc/audio_effect.xml");
    file << multipleUnknownXml;
    file.close();

    int32_t ret = parser.LoadEffectConfig(result);
    EXPECT_NE(ret, 0);
}

/**
* @tc.name: Test LoadEffectConfig for Exception Node Processing
* @tc.number: AAudioEffectConfigParser_012
* @tc.desc: Test LoadEffectConfig function focusing on handling unknown XML nodes
*/
HWTEST(AudioEffectConfigParserUnitTest, AudioEffectConfigParser_012, TestSize.Level1)
{
    OriginalEffectConfig result;
    AudioEffectConfigParser parser;

    // exceed limit unkonwn node
    std::string excessiveUnknownXml = R"(<?xml version="1.0" encoding="UTF-8"?>
        <audio_effects_config version="1.0">
            <libraries>
                <library name="bundle" path="lib/bundle.so"/>
            </libraries>)";

    // add exceed AUDIO_EFFECT_COUNT_UPPER_LIMIT(20)
    for (int i = 0; i < 25; i++) {
        excessiveUnknownXml += "<unknownNode" + std::to_string(i) + ">value</unknownNode" +
            std::to_string(i) + ">";
    }
    excessiveUnknownXml += "</audio_effects_config>";

    std::ofstream file("/etc/audio_effect.xml");
    file << excessiveUnknownXml;
    file.close();

    int32_t ret = parser.LoadEffectConfig(result);
    EXPECT_NE(ret, 0);
}
} // namespace AudioStandard
} // namespace OHOS
