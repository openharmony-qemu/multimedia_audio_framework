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

#include <gtest/gtest.h>

#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_system_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

const int32_t TEST_RET_NUM = 0;

class AudioSystemManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

/**
 * @tc.name  : Test GetMaxVolume API
 * @tc.type  : FUNC
 * @tc.number: GetMaxVolume_001
 * @tc.desc  : Test GetMaxVolume interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetMaxVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMaxVolume_001 start");
    int32_t result = AudioSystemManager::GetInstance()->GetMaxVolume(STREAM_ALL);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMaxVolume_001 result1:%{public}d", result);
    EXPECT_GT(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->GetMaxVolume(STREAM_ULTRASONIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMaxVolume_001 result2:%{public}d", result);
    EXPECT_GT(result, TEST_RET_NUM);
}

/**
 * @tc.name  : Test GetMinVolume API
 * @tc.type  : FUNC
 * @tc.number: GetMinVolume_001
 * @tc.desc  : Test GetMinVolume interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetMinVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMinVolume_001 start");
    int32_t result = AudioSystemManager::GetInstance()->GetMinVolume(STREAM_ALL);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMinVolume_001 result1:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
    result = AudioSystemManager::GetInstance()->GetMinVolume(STREAM_ULTRASONIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetMinVolume_001 result2:%{public}d", result);
    EXPECT_EQ(result, TEST_RET_NUM);
}

/**
 * @tc.name  : Test IsStreamMute API
 * @tc.type  : FUNC
 * @tc.number: IsStreamMute_001
 * @tc.desc  : Test IsStreamMute interface.
 */
HWTEST(AudioSystemManagerUnitTest, IsStreamMute_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMute_001 start");
    bool result = AudioSystemManager::GetInstance()->IsStreamMute(STREAM_MUSIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMute_001 result1:%{public}d", result);
    EXPECT_EQ(result, false);

    result = AudioSystemManager::GetInstance()->IsStreamMute(STREAM_RING);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMute_001 result2:%{public}d", result);
    EXPECT_EQ(result, false);

    result = AudioSystemManager::GetInstance()->IsStreamMute(STREAM_NOTIFICATION);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamMute_001 result3:%{public}d", result);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test IsStreamActive API
 * @tc.type  : FUNC
 * @tc.number: IsStreamActive_002
 * @tc.desc  : Test IsStreamActive interface.
 */
HWTEST(AudioSystemManagerUnitTest, IsStreamActive_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActive_002 start");
    bool result = AudioSystemManager::GetInstance()->IsStreamActive(STREAM_MUSIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActive_002 result1:%{public}d", result);
    EXPECT_EQ(result, false);

    result = AudioSystemManager::GetInstance()->IsStreamActive(STREAM_ULTRASONIC);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActive_002 result2:%{public}d", result);
    EXPECT_EQ(result, false);

    result = AudioSystemManager::GetInstance()->IsStreamActive(STREAM_ALL);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest IsStreamActive_002 result3:%{public}d", result);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test GetSelfBundleName API
 * @tc.type  : FUNC
 * @tc.number: GetSelfBundleName_001
 * @tc.desc  : Test GetSelfBundleName interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetSelfBundleName_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSelfBundleName_001 start");
    std::string bundleName = AudioSystemManager::GetInstance()->GetSelfBundleName();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetSelfBundleName_001 bundleName:%{public}s", bundleName.c_str());
    EXPECT_EQ(bundleName, "");
}

/**
 * @tc.name  : Test GetPinValueFromType API
 * @tc.type  : FUNC
 * @tc.number: GetPinValueFromType_001
 * @tc.desc  : Test GetPinValueFromType interface.
 */
HWTEST(AudioSystemManagerUnitTest, GetPinValueFromType_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest GetPinValueFromType_001 start");
    AudioPin pinValue = AudioSystemManager::GetInstance()->GetPinValueFromType(DEVICE_TYPE_DP, INPUT_DEVICE);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ->GetPinValueFromType_001() pinValue:%{public}d", pinValue);
    EXPECT_NE(pinValue, AUDIO_PIN_NONE);
}

/**
 * @tc.name  : Test RegisterWakeupSourceCallback API
 * @tc.type  : FUNC
 * @tc.number: RegisterWakeupSourceCallback_001
 * @tc.desc  : Test RegisterWakeupSourceCallback interface.
 */
HWTEST(AudioSystemManagerUnitTest, RegisterWakeupSourceCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest RegisterWakeupSourceCallback_001 start");
    int32_t result = AudioSystemManager::GetInstance()->RegisterWakeupSourceCallback();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ->RegisterWakeupSourceCallback_001() result:%{public}d", result);
    EXPECT_NE(result, ERROR);
}

/**
* @tc.name   : Test ConfigDistributedRoutingRole API
* @tc.number : ConfigDistributedRoutingRoleTest_001
* @tc.desc   : Test ConfigDistributedRoutingRole interface, when descriptor is nullptr.
*/
HWTEST(AudioSystemManagerUnitTest, ConfigDistributedRoutingRoleTest_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ConfigDistributedRoutingRoleTest_001 start");
    CastType castType = CAST_TYPE_ALL;
    int32_t result = AudioSystemManager::GetInstance()->ConfigDistributedRoutingRole(nullptr, castType);
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ConfigDistributedRoutingRoleTest_001() result:%{public}d", result);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}
} // namespace AudioStandard
} // namespace OHOS