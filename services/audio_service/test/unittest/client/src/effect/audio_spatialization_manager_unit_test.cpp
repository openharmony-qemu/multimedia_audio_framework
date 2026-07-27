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

#include <gtest/gtest.h>

#include "audio_spatialization_manager.h"
#include "audio_errors.h"
#include "audio_common_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class AudioSpatializationManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioSpatializationManagerUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest SetUpTestCase");
}

void AudioSpatializationManagerUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest TearDownTestCase");
}

void AudioSpatializationManagerUnitTest::SetUp()
{
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest SetUp");
}

void AudioSpatializationManagerUnitTest::TearDown()
{
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest TearDown");
}

/**
 * @tc.name   : Test IsSpatializationEnabled API
 * @tc.type  : FUNC
 * @tc.number: IsSpatializationEnabled_001
 * @tc.desc  : Test IsSpatializationEnabled interface (no device).
 */
HWTEST_F(AudioSpatializationManagerUnitTest, IsSpatializationEnabled_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest IsSpatializationEnabled_001 start");
    bool result = AudioSpatializationManager::GetInstance()->IsSpatializationEnabled();
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest IsSpatializationEnabled_001 result:%{public}d", result);
    // Result depends on AudioPolicyManager::IsSpatializationEnabled()
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test IsSpatializationEnabledForCurrentDevice API
 * @tc.type  : FUNC
 * @tc.number: IsSpatializationEnabledForCurrentDevice_001
 * @tc.desc  : Test IsSpatializationEnabledForCurrentDevice interface.
 */
HWTEST_F(AudioSpatializationManagerUnitTest, IsSpatializationEnabledForCurrentDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest IsSpatializationEnabledForCurrentDevice_001 start");
    bool result = AudioSpatializationManager::GetInstance()->IsSpatializationEnabledForCurrentDevice();
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest IsSpatializationEnabledForCurrentDevice_001 result:%{public}d",
        result);
    // Result depends on AudioPolicyManager::IsSpatializationEnabledForCurrentDevice()
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test SetSpatializationEnabled API
 * @tc.type  : FUNC
 * @tc.number: SetSpatializationEnabled_001
 * @tc.desc  : Test SetSpatializationEnabled interface.
 */
HWTEST_F(AudioSpatializationManagerUnitTest, SetSpatializationEnabled_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest SetSpatializationEnabled_001 start");
    int32_t result = AudioSpatializationManager::GetInstance()->SetSpatializationEnabled(true);
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest SetSpatializationEnabled_001 result:%{public}d", result);
    // Result depends on AudioPolicyManager::SetSpatializationEnabled()
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test IsHeadTrackingEnabled API
 * @tc.type  : FUNC
 * @tc.number: IsHeadTrackingEnabled_001
 * @tc.desc  : Test IsHeadTrackingEnabled interface (no device).
 */
HWTEST_F(AudioSpatializationManagerUnitTest, IsHeadTrackingEnabled_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest IsHeadTrackingEnabled_001 start");
    bool result = AudioSpatializationManager::GetInstance()->IsHeadTrackingEnabled();
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest IsHeadTrackingEnabled_001 result:%{public}d", result);
    // Result depends on AudioPolicyManager::IsHeadTrackingEnabled()
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test SetHeadTrackingEnabled API
 * @tc.type  : FUNC
 * @tc.number: SetHeadTrackingEnabled_001
 * @tc.desc  : Test SetHeadTrackingEnabled interface.
 */
HWTEST_F(AudioSpatializationManagerUnitTest, SetHeadTrackingEnabled_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest SetHeadTrackingEnabled_001 start");
    int32_t result = AudioSpatializationManager::GetInstance()->SetHeadTrackingEnabled(true);
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest SetHeadTrackingEnabled_001 result:%{public}d", result);
    // Result depends on AudioPolicyManager::SetHeadTrackingEnabled()
    // Just check function doesn't crash
    (void)result;
}

/**
 * @tc.name   : Test IsSpatializationSupported API
 * @tc.type  : FUNC
 * @tc.number: IsSpatializationSupported_001
 * @tc.desc  : Test IsSpatializationSupported interface.
 */
HWTEST_F(AudioSpatializationManagerUnitTest, IsSpatializationSupported_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest IsSpatializationSupported_001 start");
    bool result = AudioSpatializationManager::GetInstance()->IsSpatializationSupported();
    AUDIO_INFO_LOG("AudioSpatializationManagerUnitTest IsSpatializationSupported_001 result:%{public}d", result);
    // Result depends on AudioPolicyManager::IsSpatializationSupported()
    // Just check function doesn't crash
    (void)result;
}

} // namespace AudioStandard
} // namespace OHOS
