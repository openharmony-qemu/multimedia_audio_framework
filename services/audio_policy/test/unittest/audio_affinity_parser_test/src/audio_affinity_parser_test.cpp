/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <iostream>
#include "audio_affinity_parser_test.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioAffinityParserTest::SetUpTestCase(void) {}
void AudioAffinityParserTest::TearDownTestCase(void) {}
void AudioAffinityParserTest::SetUp(void) {}
void AudioAffinityParserTest::TearDown(void) {}

/**
* @tc.name  : Test audioAffinityParser.
* @tc.number: audioAffinityParser_001
* @tc.desc  : Test LoadConfiguration.
*/
HWTEST(AudioAffinityParserTest, audioAffinityParser_001, TestSize.Level1)
{
    auto affinityManager = std::make_shared<AudioAffinityManager>();
    auto audioAffinity_ = std::make_shared<audioAffinityParser>(affinityManager.get());
    audioAffinity_->LoadConfiguration();
    EXPECT_NE(audioAffinity_, nullptr);

    audioAffinity_->mDoc_ = new xmlDoc();
    audioAffinity_->LoadConfiguration();
    EXPECT_NE(audioAffinity_, nullptr);
    delete audioAffinity_->mDoc_;
}

/**
* @tc.name  : Test audioAffinityParser.
* @tc.number: audioAffinityParser_002
* @tc.desc  : Test Parse.
*/
HWTEST(AudioAffinityParserTest, audioAffinityParser_002, TestSize.Level1)
{
    auto affinityManager = std::make_shared<AudioAffinityManager>();
    auto audioAffinity_ = std::make_shared<audioAffinityParser>(affinityManager.get());
    audioAffinity_->mDoc_ = new xmlDoc();
    audioAffinity_->Parse();
    EXPECT_NE(audioAffinity_, nullptr);

    audioAffinity_->LoadConfiguration();
    audioAffinity_->Parse();
    EXPECT_NE(audioAffinity_, nullptr);
    delete audioAffinity_->mDoc_;
}

/**
* @tc.name  : Test audioAffinityParser.
* @tc.number: audioAffinityParser_003
* @tc.desc  : Test Destroy.
*/
HWTEST(AudioAffinityParserTest, audioAffinityParser_003, TestSize.Level1)
{
    auto affinityManager = std::make_shared<AudioAffinityManager>();
    auto audioAffinity_ = std::make_shared<audioAffinityParser>(affinityManager.get());
    audioAffinity_->mDoc_ = new xmlDoc();
    audioAffinity_->Destroy();
    EXPECT_NE(audioAffinity_, nullptr);

    audioAffinity_->LoadConfiguration();
    audioAffinity_->Destroy();
    EXPECT_NE(audioAffinity_, nullptr);
    delete audioAffinity_->mDoc_;
}

/**
* @tc.name  : Test audioAffinityParser.
* @tc.number: audioAffinityParser_004
* @tc.desc  : Test Destory.
*/
HWTEST(AudioAffinityParserTest, audioAffinityParser_004, TestSize.Level1)
{
    auto affinityManager = std::make_shared<AudioAffinityManager>();
    auto audioAffinity_ = std::make_shared<audioAffinityParser>(affinityManager.get());
    std::unique_ptr<xmlNode> node = std::make_unique<xmlNode>();
    audioAffinity_->ParseInternal(node.get());
    EXPECT_NE(audioAffinity_, nullptr);

    node.get()->type = XML_ELEMENT_NODE;
    audioAffinity_->ParseInternal(node.get());
    EXPECT_NE(audioAffinity_, nullptr);
}

/**
* @tc.name  : Test audioAffinityParser.
* @tc.number: audioAffinityParser_005
* @tc.desc  : Test ParserAffinityGroups.
*/
HWTEST(AudioAffinityParserTest, audioAffinityParser_005, TestSize.Level1)
{
    auto affinityManager = std::make_shared<AudioAffinityManager>();
    auto audioAffinity_ = std::make_shared<audioAffinityParser>(affinityManager.get());
    std::unique_ptr<xmlNode> node = std::make_unique<xmlNode>();

    audioAffinity_->ParserAffinityGroups(node.get(), DeviceFlag::ALL_DEVICES_FLAG);
    EXPECT_NE(audioAffinity_, nullptr);

    node.get()->xmlChildrenNode = new xmlNode();
    audioAffinity_->ParserAffinityGroups(node.get(), DeviceFlag::ALL_DEVICES_FLAG);
    EXPECT_NE(audioAffinity_, nullptr);

    node.get()->xmlChildrenNode->type = XML_ELEMENT_NODE;
    audioAffinity_->ParserAffinityGroups(node.get(), DeviceFlag::ALL_DEVICES_FLAG);
    EXPECT_NE(audioAffinity_, nullptr);

    node.get()->xmlChildrenNode->name = reinterpret_cast<const xmlChar*>("affinityGroups");
    audioAffinity_->ParserAffinityGroups(node.get(), DeviceFlag::ALL_DEVICES_FLAG);
    EXPECT_NE(audioAffinity_, nullptr);
    delete node.get()->xmlChildrenNode;
}

/**
* @tc.name  : Test audioAffinityParser.
* @tc.number: audioAffinityParser_006
* @tc.desc  : Test ParserAffinityGroupAttribute.
*/
HWTEST(AudioAffinityParserTest, audioAffinityParser_006, TestSize.Level1)
{
    auto affinityManager = std::make_shared<AudioAffinityManager>();
    auto audioAffinity_ = std::make_shared<audioAffinityParser>(affinityManager.get());
    std::unique_ptr<xmlNode> node = std::make_unique<xmlNode>();

    audioAffinity_->ParserAffinityGroupAttribute(node.get(), DeviceFlag::ALL_DEVICES_FLAG);
    EXPECT_NE(audioAffinity_, nullptr);

    node.get()->xmlChildrenNode = new xmlNode();
    audioAffinity_->ParserAffinityGroupAttribute(node.get(), DeviceFlag::ALL_DEVICES_FLAG);
    EXPECT_NE(audioAffinity_, nullptr);

    node.get()->xmlChildrenNode->type = XML_ELEMENT_NODE;
    audioAffinity_->ParserAffinityGroupAttribute(node.get(), DeviceFlag::ALL_DEVICES_FLAG);
    EXPECT_NE(audioAffinity_, nullptr);

    node.get()->xmlChildrenNode->name = reinterpret_cast<const xmlChar*>("affinityGroup");
    audioAffinity_->ParserAffinityGroupAttribute(node.get(), DeviceFlag::ALL_DEVICES_FLAG);
    EXPECT_NE(audioAffinity_, nullptr);
    delete node.get()->xmlChildrenNode;
}

/**
* @tc.name  : Test audioAffinityParser.
* @tc.number: audioAffinityParser_007
* @tc.desc  : Test ParserAffinityGroupDeviceInfos.
*/
HWTEST(AudioAffinityParserTest, audioAffinityParser_007, TestSize.Level1)
{
    auto affinityManager = std::make_shared<AudioAffinityManager>();
    auto audioAffinity_ = std::make_shared<audioAffinityParser>(affinityManager.get());
    AffinityDeviceInfo deviceInfo;
    std::unique_ptr<xmlNode> node = std::make_unique<xmlNode>();

    audioAffinity_->ParserAffinityGroupDeviceInfos(node.get(), deviceInfo);
    EXPECT_NE(audioAffinity_, nullptr);

    node.get()->xmlChildrenNode = new xmlNode();
    audioAffinity_->ParserAffinityGroupDeviceInfos(node.get(), deviceInfo);
    EXPECT_NE(audioAffinity_, nullptr);

    node.get()->xmlChildrenNode->name = reinterpret_cast<const xmlChar*>("affinity");
    audioAffinity_->ParserAffinityGroupDeviceInfos(node.get(), deviceInfo);
    EXPECT_NE(audioAffinity_, nullptr);
    delete node.get()->xmlChildrenNode;
}

} // namespace AudioStandard
} // namespace OHOS
