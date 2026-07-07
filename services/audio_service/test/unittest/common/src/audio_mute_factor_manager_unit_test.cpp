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

#include "audio_mute_factor_manager.h"
#include <gtest/gtest.h>

using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr bool MDM_MUTED = true;
constexpr bool MDM_NOT_MUTED = false;
}
class AudioMuteFactorManagerUnitTest : public testing::Test {
public:
    void SetUp() override
    {
        AudioMuteFactorManager::GetInstance().SetMdmMuteStatus(MDM_NOT_MUTED);
    }
    void TearDown() override
    {
        AudioMuteFactorManager::GetInstance().SetMdmMuteStatus(MDM_NOT_MUTED);
    }
};

HWTEST_F(AudioMuteFactorManagerUnitTest, GetInstance_001, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    // GetInstance() returns a reference, which can't be nullptr
    // Verify that instance is properly initialized by calling its method
    EXPECT_FALSE(instance.IsMdmMuted());  // Default state should be not muted
}

HWTEST_F(AudioMuteFactorManagerUnitTest, IsMdmMuted_001, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    instance.SetMdmMuteStatus(MDM_NOT_MUTED);
    EXPECT_FALSE(instance.IsMdmMuted());
}

HWTEST_F(AudioMuteFactorManagerUnitTest, IsMdmMuted_002, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    instance.SetMdmMuteStatus(MDM_MUTED);
    EXPECT_TRUE(instance.IsMdmMuted());
}

HWTEST_F(AudioMuteFactorManagerUnitTest, GetMdmMuteStatus_001, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    instance.SetMdmMuteStatus(MDM_NOT_MUTED);
    EXPECT_FLOAT_EQ(instance.GetMdmMuteStatus(), 1.0f);
}

HWTEST_F(AudioMuteFactorManagerUnitTest, GetMdmMuteStatus_002, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    instance.SetMdmMuteStatus(MDM_MUTED);
    EXPECT_FLOAT_EQ(instance.GetMdmMuteStatus(), 0.0f);
}

HWTEST_F(AudioMuteFactorManagerUnitTest, SetMdmMuteStatus_001, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    instance.SetMdmMuteStatus(MDM_MUTED);
    EXPECT_TRUE(instance.IsMdmMuted());
    EXPECT_FLOAT_EQ(instance.GetMdmMuteStatus(), 0.0f);

    instance.SetMdmMuteStatus(MDM_NOT_MUTED);
    EXPECT_FALSE(instance.IsMdmMuted());
    EXPECT_FLOAT_EQ(instance.GetMdmMuteStatus(), 1.0f);
}

HWTEST_F(AudioMuteFactorManagerUnitTest, SetMdmMuteStatus_002, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    // Test multiple set operations
    instance.SetMdmMuteStatus(MDM_MUTED);
    EXPECT_TRUE(instance.IsMdmMuted());
    EXPECT_FLOAT_EQ(instance.GetMdmMuteStatus(), 0.0f);

    instance.SetMdmMuteStatus(MDM_NOT_MUTED);
    EXPECT_FALSE(instance.IsMdmMuted());
    EXPECT_FLOAT_EQ(instance.GetMdmMuteStatus(), 1.0f);

    // Set again to same value
    instance.SetMdmMuteStatus(MDM_NOT_MUTED);
    EXPECT_FALSE(instance.IsMdmMuted());
    EXPECT_FLOAT_EQ(instance.GetMdmMuteStatus(), 1.0f);
}

HWTEST_F(AudioMuteFactorManagerUnitTest, InitialState_001, testing::ext::TestSize.Level1)
{
    // Note: We can't directly test the initial state because SetUp() sets it to MDM_NOT_MUTED
    // But we can verify that SetUp() and TearDown() work correctly
    auto &instance = AudioMuteFactorManager::GetInstance();
    
    // After SetUp(), should be not muted
    EXPECT_FALSE(instance.IsMdmMuted());
    EXPECT_FLOAT_EQ(instance.GetMdmMuteStatus(), 1.0f);
    
    // Modify state
    instance.SetMdmMuteStatus(MDM_MUTED);
    EXPECT_TRUE(instance.IsMdmMuted());
    
    // After TearDown() (called automatically), should be reset to not muted
    // But we can't call TearDown() manually in the test
    // This is verified by the next test case that runs
}
} // namespace AudioStandard
} // namespace OHOS
