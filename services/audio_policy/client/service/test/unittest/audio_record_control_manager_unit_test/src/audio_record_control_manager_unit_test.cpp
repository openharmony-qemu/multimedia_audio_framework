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

#include "audio_record_control_manager_unit_test.h"
#include "audio_errors.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioRecordControlManagerUnitTest::SetUpTestCase(void) {}
void AudioRecordControlManagerUnitTest::TearDownTestCase(void) {}
void AudioRecordControlManagerUnitTest::SetUp(void) {}
void AudioRecordControlManagerUnitTest::TearDown(void) {}

/**
 * @tc.name  : EnableSystemRecordController_001
 * @tc.desc  : Test EnableSystemRecordController with invalid sourceType
 * @tc.type  : FUNC
 */
HWTEST_F(AudioRecordControlManagerUnitTest, EnableSystemRecordController_001, TestSize.Level1)
{
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_INVALID;
    int32_t result = AudioRecordControlManager::GetInstance().EnableSystemRecordController(true, config);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : EnableSystemRecordController_002
 * @tc.desc  : Test EnableSystemRecordController with invalid sourceType (VOICE_RECOGNITION)
 * @tc.type  : FUNC
 */
HWTEST_F(AudioRecordControlManagerUnitTest, EnableSystemRecordController_002, TestSize.Level1)
{
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    int32_t result = AudioRecordControlManager::GetInstance().EnableSystemRecordController(true, config);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : EnableSystemRecordController_003
 * @tc.desc  : Test EnableSystemRecordController with valid sourceType MIC
 * @tc.type  : FUNC
 */
HWTEST_F(AudioRecordControlManagerUnitTest, EnableSystemRecordController_003, TestSize.Level1)
{
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t result = AudioRecordControlManager::GetInstance().EnableSystemRecordController(true, config);
    EXPECT_NE(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : EnableSystemRecordController_004
 * @tc.desc  : Test EnableSystemRecordController with valid sourceType CAMCORDER
 * @tc.type  : FUNC
 */
HWTEST_F(AudioRecordControlManagerUnitTest, EnableSystemRecordController_004, TestSize.Level1)
{
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_CAMCORDER;
    int32_t result = AudioRecordControlManager::GetInstance().EnableSystemRecordController(false, config);
    EXPECT_NE(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : EnableSystemRecordController_005
 * @tc.desc  : Test EnableSystemRecordController with valid sourceType LIVE
 * @tc.type  : FUNC
 */
HWTEST_F(AudioRecordControlManagerUnitTest, EnableSystemRecordController_005, TestSize.Level1)
{
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_LIVE;
    int32_t result = AudioRecordControlManager::GetInstance().EnableSystemRecordController(true, config);
    EXPECT_NE(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : SetRecordControllerEnabledChangeCallback_001
 * @tc.desc  : Test SetRecordControllerEnabledChangeCallback with nullptr callback
 * @tc.type  : FUNC
 */
HWTEST_F(AudioRecordControlManagerUnitTest, SetRecordControllerEnabledChangeCallback_001, TestSize.Level1)
{
    int32_t result = AudioRecordControlManager::GetInstance().SetRecordControllerEnabledChangeCallback(nullptr);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : UnsetRecordControllerEnabledChangeCallback_001
 * @tc.desc  : Test UnsetRecordControllerEnabledChangeCallback with nullptr callback
 * @tc.type  : FUNC
 */
HWTEST_F(AudioRecordControlManagerUnitTest, UnsetRecordControllerEnabledChangeCallback_001, TestSize.Level1)
{
    int32_t result = AudioRecordControlManager::GetInstance().UnsetRecordControllerEnabledChangeCallback(nullptr);
    EXPECT_NE(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : GetInstance_001
 * @tc.desc  : Test GetInstance returns same instance
 * @tc.type  : FUNC
 */
HWTEST_F(AudioRecordControlManagerUnitTest, GetInstance_001, TestSize.Level1)
{
    auto &instance1 = AudioRecordControlManager::GetInstance();
    auto &instance2 = AudioRecordControlManager::GetInstance();
    EXPECT_EQ(&instance1, &instance2);
}

/**
 * @tc.name  : EnableSystemRecordController_006
 * @tc.desc  : Test EnableSystemRecordController with invalid sourceType (VOICE_COMMUNICATION)
 * @tc.type  : FUNC
 */
HWTEST_F(AudioRecordControlManagerUnitTest, EnableSystemRecordController_006, TestSize.Level1)
{
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    int32_t result = AudioRecordControlManager::GetInstance().EnableSystemRecordController(true, config);
    EXPECT_EQ(result, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : EnableSystemRecordController_007
 * @tc.desc  : Test EnableSystemRecordController with show=false and valid sourceType
 * @tc.type  : FUNC
 */
HWTEST_F(AudioRecordControlManagerUnitTest, EnableSystemRecordController_007, TestSize.Level1)
{
    SystemRecordControllerConfig config;
    config.sourceType = SOURCE_TYPE_MIC;
    int32_t result = AudioRecordControlManager::GetInstance().EnableSystemRecordController(false, config);
    EXPECT_NE(result, ERR_INVALID_PARAM);
}

} // namespace AudioStandard
} // namespace OHOS
