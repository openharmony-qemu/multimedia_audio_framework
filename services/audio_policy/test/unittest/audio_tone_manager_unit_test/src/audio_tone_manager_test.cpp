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

#include "audio_tone_manager_test.h"
#include <thread>
#include <string>
#include <memory>
#include <vector>
#include <sys/socket.h>
#include <cerrno>
#include <fstream>
#include <algorithm>
#include <unistd.h>
#include "audio_policy_log.h"
#include "audio_errors.h"
using namespace std;
using namespace std::chrono;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioToneManagerUnitTest::SetUpTestCase(void) {}
void AudioToneManagerUnitTest::TearDownTestCase(void) {}
void AudioToneManagerUnitTest::SetUp(void) {}
void AudioToneManagerUnitTest::TearDown(void) {}


#define PRINT_LINE printf("debug __LINE__:%d\n", __LINE__)

#ifdef FEATURE_DTMF_TONE

/**
 * @tc.name  : Test LoadToneDtmfConfig
 * @tc.number: LoadToneDtmfConfig_Fail_001
 * @tc.desc  : Test LoadToneDtmfConfig return false when the profile content is illegal.
 */
HWTEST_F(AudioToneManagerUnitTest, LoadToneDtmfConfig_LoadFail_001, TestSize.Level4)
{
    AudioToneManager audioToneManager;

    std::ofstream file("/system/etc/audio/audio_tone_dtmf_config.xml");
    file << "<INVALID></INVALID>";
    file.close();

    bool ret = audioToneManager.LoadToneDtmfConfig();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test LoadToneDtmfConfig
 * @tc.number: LoadToneDtmfConfig_Success_002
 * @tc.desc  : Test LoadToneDtmfConfig return true when the profile content is legal.
 */
HWTEST_F(AudioToneManagerUnitTest, LoadToneDtmfConfig_Success_002, TestSize.Level4)
{
    std::string configPath = "/system/etc/audio/audio_tone_dtmf_config.xml";
    std::ofstream file(configPath);
    ASSERT_TRUE(file.is_open());

    file << "<DTMF>"
         << "<Default>"
         << "<Tone name=\"tone1\" freq=\"440\" duration=\"100\" />"
         << "</Default>"
         << "</DTMF>";
    file.close();

    AudioToneManager manager;
    bool result = manager.LoadToneDtmfConfig();
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test GetSupportedTones
 * @tc.number: GetSupportedTones_Empty_001
 * @tc.desc  : Test return empty list when both the default and custom tone are empty.
 */
HWTEST_F(AudioToneManagerUnitTest, GetSupportedTones_Empty_001, TestSize.Level4)
{
    AudioToneManager manager;

    auto result = manager.GetSupportedTones("CN");
    EXPECT_TRUE(result.empty());
}

/**
 * @tc.name  : Test GetSupportedTones
 * @tc.number: GetSupportedTones_CustomFound_002
 * @tc.desc  : Test customToneDescriptorMap_ branch, return custom and default collection.
 */
HWTEST_F(AudioToneManagerUnitTest, GetSupportedTones_CustomFound_002, TestSize.Level4)
{
    AudioToneManager manager;

    manager.toneDescriptorMap_[100] = std::make_shared<ToneInfo>();
    manager.toneDescriptorMap_[200] = std::make_shared<ToneInfo>();

    std::string country = "CN";
    manager.customToneDescriptorMap_[country][200] = std::make_shared<ToneInfo>();
    manager.customToneDescriptorMap_[country][300] = std::make_shared<ToneInfo>();

    auto result = manager.GetSupportedTones(country);

    std::set<int32_t> resultSet(result.begin(), result.end());
    EXPECT_EQ(resultSet.size(), 3);
    EXPECT_TRUE(resultSet.count(100));
    EXPECT_TRUE(resultSet.count(200));
    EXPECT_TRUE(resultSet.count(300));
}

/**
 * @tc.name  : Test GetSupportedTones
 * @tc.number: GetSupportedTones_CustomNotFound_003
 * @tc.desc  : Test Input country code not in custom list, return default tone.
 */
HWTEST_F(AudioToneManagerUnitTest, GetSupportedTones_CustomNotFound_002, TestSize.Level4)
{
    AudioToneManager manager;

    manager.toneDescriptorMap_[100] = std::make_shared<ToneInfo>();
    manager.toneDescriptorMap_[200] = std::make_shared<ToneInfo>();

    manager.customToneDescriptorMap_["US"][300] = std::make_shared<ToneInfo>();

    auto result = manager.GetSupportedTones("FR");

    std::set<int32_t> resultSet(result.begin(), result.end());
    EXPECT_EQ(resultSet.size(), 2);
    EXPECT_TRUE(resultSet.count(100));
    EXPECT_TRUE(resultSet.count(200));
    EXPECT_FALSE(resultSet.count(300));
}

/**
 * @tc.name   : Test GetToneConfig
 * @tc.number : GetToneConfig_NotFound_001
 * @tc.desc   : Test return nullptr when toneType not found in either customToneDescriptorMap_ or toneDescriptorMap_.
 */
HWTEST_F(AudioToneManagerUnitTest, GetToneConfig_NotFound_001, TestSize.Level4)
{
    AudioToneManager manager;
    std::string country = "FR";
    int32_t toneType = 999;

    auto result = manager.GetToneConfig(toneType, country);
    EXPECT_EQ(result, nullptr);
}

/**
 * @tc.name   : Test GetToneConfig
 * @tc.number : GetToneConfig_CustomHit_002
 * @tc.desc   : Test return custom tone config when both country code and toneType exist in customToneDescriptorMap_.
 */
HWTEST_F(AudioToneManagerUnitTest, GetToneConfig_CustomHit_002, TestSize.Level4)
{
    AudioToneManager manager;
    std::string country = "CN";
    int32_t toneType = 100;

    auto toneInfo = std::make_shared<ToneInfo>();
    manager.customToneDescriptorMap_[country][toneType] = toneInfo;

    auto result = manager.GetToneConfig(toneType, country);
    EXPECT_EQ(result, toneInfo);
}

/**
 * @tc.name   : Test GetToneConfig
 * @tc.number : GetToneConfig_CustomMissDefaultHit_003
 * @tc.desc   : Test return default when country code exists in customToneDescriptorMap_ but toneType does not.
 */
HWTEST_F(AudioToneManagerUnitTest, GetToneConfig_CustomMissDefaultHit_003, TestSize.Level4)
{
    AudioToneManager manager;
    std::string country = "CN";
    int32_t toneType = 200;

    manager.customToneDescriptorMap_[country][999] = std::make_shared<ToneInfo>();
    auto defaultTone = std::make_shared<ToneInfo>();
    manager.toneDescriptorMap_[toneType] = defaultTone;

    auto result = manager.GetToneConfig(toneType, country);
    EXPECT_EQ(result, defaultTone);
}

/**
 * @tc.name   : Test GetToneConfig
 * @tc.number : GetToneConfig_DefaultOnly_004
 * @tc.desc   : Test return default tone config when customToneDescriptorMap_ does not contain the country code.
 */
HWTEST_F(AudioToneManagerUnitTest, GetToneConfig_DefaultOnly_004, TestSize.Level4)
{
    AudioToneManager manager;
    std::string country = "US";
    int32_t toneType = 300;

    auto defaultTone = std::make_shared<ToneInfo>();
    manager.toneDescriptorMap_[toneType] = defaultTone;

    auto result = manager.GetToneConfig(toneType, country);
    EXPECT_EQ(result, defaultTone);
}

#endif
}
}
