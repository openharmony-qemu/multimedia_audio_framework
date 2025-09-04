/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "audio_policy_global_parser_unit_test.h"
#include "audio_errors.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPolicyGlobalParserUnitTest::SetUpTestCase(void) {}
void AudioPolicyGlobalParserUnitTest::TearDownTestCase(void) {}
void AudioPolicyGlobalParserUnitTest::SetUp(void) {}
void AudioPolicyGlobalParserUnitTest::TearDown(void)
{
    if (parser_.mDoc_ != nullptr) {
        xmlFreeDoc(parser_.mDoc_);
        parser_.mDoc_ = nullptr;
    }
}

/**
 * @tc.name  : Test AudioPolicyGlobalParser.
 * @tc.number: AudioPolicyGlobalParserUnitTest_001
 * @tc.desc  : Test AudioPolicyGlobalParser Parse interface.
 */
HWTEST_F(AudioPolicyGlobalParserUnitTest, AudioPolicyGlobalParserUnitTest_001, TestSize.Level4)
{
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    ASSERT_NE(doc, nullptr);
    xmlNodePtr root = xmlNewNode(nullptr, BAD_CAST "global");
    ASSERT_NE(root, nullptr);
    xmlDocSetRootElement(doc, root);
    parser_.mDoc_ = doc;
    bool res = parser_.Parse();
    EXPECT_FALSE(res);
}

/**
 * @tc.name  : Test AudioPolicyGlobalParser.
 * @tc.number: AudioPolicyGlobalParserUnitTest_002
 * @tc.desc  : Test AudioPolicyGlobalParser Parse interface.
 */
HWTEST_F(AudioPolicyGlobalParserUnitTest, AudioPolicyGlobalParserUnitTest_002, TestSize.Level4)
{
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    ASSERT_NE(doc, nullptr);
    xmlNodePtr root = xmlNewNode(nullptr, BAD_CAST "global");
    ASSERT_NE(root, nullptr);
    xmlDocSetRootElement(doc, root);

    xmlNodePtr child = xmlNewChild(root, nullptr, BAD_CAST "attribute", nullptr);
    ASSERT_NE(child, nullptr);
    xmlNewProp(child, BAD_CAST "name", BAD_CAST "foo");
    xmlNewProp(child, BAD_CAST "value", BAD_CAST "1");

    parser_.mDoc_ = doc;
    bool res = parser_.Parse();
    EXPECT_TRUE(res);
}

/**
 * @tc.name  : Test AudioPolicyGlobalParser.
 * @tc.number: AudioPolicyGlobalParserUnitTest_003
 * @tc.desc  : Test AudioPolicyGlobalParser Destroy interface.
 */
HWTEST_F(AudioPolicyGlobalParserUnitTest, AudioPolicyGlobalParserUnitTest_003, TestSize.Level4)
{
    ASSERT_EQ(parser_.mDoc_, nullptr);
    parser_.Destroy();
    EXPECT_EQ(parser_.mDoc_, nullptr);
}

/**
 * @tc.name  : Test AudioPolicyGlobalParser.
 * @tc.number: AudioPolicyGlobalParserUnitTest_004
 * @tc.desc  : Test AudioPolicyGlobalParser Destroy interface.
 */
HWTEST_F(AudioPolicyGlobalParserUnitTest, AudioPolicyGlobalParserUnitTest_004, TestSize.Level4)
{
    xmlDocPtr doc = xmlNewDoc(BAD_CAST "1.0");
    ASSERT_NE(doc, nullptr);
    parser_.mDoc_ = doc;
    parser_.Destroy();
    parser_.mDoc_ = nullptr;
    EXPECT_EQ(parser_.mDoc_, nullptr);
}

/**
 * @tc.name  : Test AudioPolicyGlobalParser.
 * @tc.number: AudioPolicyGlobalParserUnitTest_005
 * @tc.desc  : Test AudioPolicyGlobalParser GetConfigByKeyName interface.
 */
HWTEST_F(AudioPolicyGlobalParserUnitTest, AudioPolicyGlobalParserUnitTest_005, TestSize.Level4)
{
    uint32_t value = 0;
    int32_t ret = parser_.GetConfigByKeyName("NotExistKey", value);

    EXPECT_EQ(ret, ERR_CONFIG_NAME_ERROR);
    EXPECT_EQ(value, 0);
}

/**
 * @tc.name  : Test AudioPolicyGlobalParser.
 * @tc.number: AudioPolicyGlobalParserUnitTest_006
 * @tc.desc  : Test AudioPolicyGlobalParser GetConfigByKeyName interface.
 */
HWTEST_F(AudioPolicyGlobalParserUnitTest, AudioPolicyGlobalParserUnitTest_006, TestSize.Level4)
{
    parser_.globalConfigs_["volume"] = 123;

    uint32_t value = 0;
    int32_t ret = parser_.GetConfigByKeyName("volume", value);

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(value, 123);
}

/**
 * @tc.name  : Test AudioPolicyGlobalParser.
 * @tc.number: AudioPolicyGlobalParserUnitTest_007
 * @tc.desc  : Test AudioPolicyGlobalParser ParseInternal interface.
 */
HWTEST_F(AudioPolicyGlobalParserUnitTest, AudioPolicyGlobalParserUnitTest_007, TestSize.Level4)
{
    xmlNodePtr node = xmlNewNode(nullptr, BAD_CAST "not_attribute");
    ASSERT_NE(node, nullptr);
    node->type = XML_TEXT_NODE;
    bool ret = parser_.ParseInternal(node);

    EXPECT_TRUE(ret);
    EXPECT_TRUE(parser_.globalConfigs_.empty());

    xmlFreeNode(node);
}

/**
 * @tc.name  : Test AudioPolicyGlobalParser.
 * @tc.number: AudioPolicyGlobalParserUnitTest_008
 * @tc.desc  : Test AudioPolicyGlobalParser ParseInternal interface.
 */
HWTEST_F(AudioPolicyGlobalParserUnitTest, AudioPolicyGlobalParserUnitTest_008, TestSize.Level4)
{
    xmlNodePtr node = xmlNewNode(nullptr, BAD_CAST "attribute");
    ASSERT_NE(node, nullptr);
    node->type = XML_TEXT_NODE;
    bool ret = parser_.ParseInternal(node);

    EXPECT_TRUE(ret);
    EXPECT_TRUE(parser_.globalConfigs_.empty());

    xmlFreeNode(node);
}

/**
 * @tc.name  : Test AudioPolicyGlobalParser.
 * @tc.number: AudioPolicyGlobalParserUnitTest_009
 * @tc.desc  : Test AudioPolicyGlobalParser ParseInternal interface.
 */
HWTEST_F(AudioPolicyGlobalParserUnitTest, AudioPolicyGlobalParserUnitTest_009, TestSize.Level4)
{
    xmlNodePtr node = xmlNewNode(nullptr, BAD_CAST "not_attribute");
    ASSERT_NE(node, nullptr);
    node->type = XML_ELEMENT_NODE;
    bool ret = parser_.ParseInternal(node);

    EXPECT_TRUE(ret);
    EXPECT_TRUE(parser_.globalConfigs_.empty());

    xmlFreeNode(node);
}

/**
 * @tc.name  : Test AudioPolicyGlobalParser.
 * @tc.number: AudioPolicyGlobalParserUnitTest_010
 * @tc.desc  : Test AudioPolicyGlobalParser ParseInternal interface.
 */
HWTEST_F(AudioPolicyGlobalParserUnitTest, AudioPolicyGlobalParserUnitTest_010, TestSize.Level4)
{
    xmlNodePtr node = xmlNewNode(nullptr, BAD_CAST "attribute");
    ASSERT_NE(node, nullptr);
    node->type = XML_ELEMENT_NODE;
    xmlNewProp(node, BAD_CAST "name", BAD_CAST "foo");
    xmlNewProp(node, BAD_CAST "value", BAD_CAST "100");

    bool ret = parser_.ParseInternal(node);

    EXPECT_TRUE(ret);
    ASSERT_FALSE(parser_.globalConfigs_.empty());
    EXPECT_EQ(parser_.globalConfigs_["foo"], 100);

    xmlFreeNode(node);
}

/**
 * @tc.name  : Test AudioPolicyGlobalParser.
 * @tc.number: AudioPolicyGlobalParserUnitTest_011
 * @tc.desc  : Test AudioPolicyGlobalParser ParserAttribute interface.
 */
HWTEST_F(AudioPolicyGlobalParserUnitTest, AudioPolicyGlobalParserUnitTest_011, TestSize.Level4)
{
    xmlNodePtr node1 = xmlNewNode(nullptr, BAD_CAST "attribute");
    ASSERT_NE(node1, nullptr);
    node1->type = XML_ELEMENT_NODE;

    xmlNodePtr node2 = xmlNewNode(nullptr, BAD_CAST "attribute");
    ASSERT_NE(node2, nullptr);
    node2->type = XML_ELEMENT_NODE;
    xmlNewProp(node2, BAD_CAST "value", BAD_CAST "123");

    xmlNodePtr node3 = xmlNewNode(nullptr, BAD_CAST "attribute");
    ASSERT_NE(node3, nullptr);
    node3->type = XML_ELEMENT_NODE;
    xmlNewProp(node3, BAD_CAST "name", BAD_CAST "only_name");

    xmlNodePtr node4 = xmlNewNode(nullptr, BAD_CAST "attribute");
    ASSERT_NE(node4, nullptr);
    node4->type = XML_ELEMENT_NODE;
    xmlNewProp(node4, BAD_CAST "name", BAD_CAST "key");
    xmlNewProp(node4, BAD_CAST "value", BAD_CAST "456");

    node1->next = node2;
    node2->next = node3;
    node3->next = node4;
    node4->next = nullptr;

    parser_.ParserAttribute(node1);
    ASSERT_EQ(parser_.globalConfigs_.size(), 1);
    auto it = parser_.globalConfigs_.find("key");
    ASSERT_NE(it, parser_.globalConfigs_.end());
    EXPECT_EQ(it->second, 456);

    xmlFreeNode(node1);
    xmlFreeNode(node2);
    xmlFreeNode(node3);
    xmlFreeNode(node4);
}

/**
 * @tc.name  : Test AudioPolicyGlobalParser.
 * @tc.number: AudioPolicyGlobalParserUnitTest_012
 * @tc.desc  : Test AudioPolicyGlobalParser ParserAttribute interface.
 */
HWTEST_F(AudioPolicyGlobalParserUnitTest, AudioPolicyGlobalParserUnitTest_012, TestSize.Level4)
{
    xmlNodePtr node1 = xmlNewNode(nullptr, BAD_CAST "attribute");
    ASSERT_NE(node1, nullptr);
    node1->type = XML_ELEMENT_NODE;
    xmlNewProp(node1, BAD_CAST "name", BAD_CAST "key1");
    xmlNewProp(node1, BAD_CAST "value", BAD_CAST "1");

    xmlNodePtr node2 = xmlNewNode(nullptr, BAD_CAST "attribute");
    ASSERT_NE(node2, nullptr);
    node2->type = XML_ELEMENT_NODE;
    xmlNewProp(node2, BAD_CAST "name", BAD_CAST "key2");
    xmlNewProp(node2, BAD_CAST "value", BAD_CAST "2");

    node1->next = node2;
    node2->next = nullptr;

    parser_.ParserAttribute(node1);

    ASSERT_EQ(parser_.globalConfigs_.size(), 2);
    EXPECT_EQ(parser_.globalConfigs_["key1"], 1);
    EXPECT_EQ(parser_.globalConfigs_["key2"], 2);

    xmlFreeNode(node1);
    xmlFreeNode(node2);
}
} // namespace AudioStandard
} // namespace OHOS