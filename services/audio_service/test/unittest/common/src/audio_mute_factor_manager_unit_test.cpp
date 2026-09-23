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
        AudioMuteFactorManager::GetInstance().ClearUnmuteWhitelist();
    }
};

/**
* @tc.name   : GetInstance_001
* @tc.number : GetInstance_001
* @tc.desc   : Test GetInstance returns a valid singleton reference and default state is not muted.
*/
HWTEST_F(AudioMuteFactorManagerUnitTest, GetInstance_001, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    EXPECT_FALSE(instance.IsMdmMuted());
}

/**
* @tc.name   : IsMdmMuted_001
* @tc.number : IsMdmMuted_001
* @tc.desc   : Test IsMdmMuted returns false when MDM mute status is set to not muted.
*/
HWTEST_F(AudioMuteFactorManagerUnitTest, IsMdmMuted_001, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    instance.SetMdmMuteStatus(MDM_NOT_MUTED);
    EXPECT_FALSE(instance.IsMdmMuted());
}

/**
* @tc.name   : IsMdmMuted_002
* @tc.number : IsMdmMuted_002
* @tc.desc   : Test IsMdmMuted returns true when MDM mute status is set to muted.
*/
HWTEST_F(AudioMuteFactorManagerUnitTest, IsMdmMuted_002, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    instance.SetMdmMuteStatus(MDM_MUTED);
    EXPECT_TRUE(instance.IsMdmMuted());
}

/**
* @tc.name   : GetMdmMuteStatus_001
* @tc.number : GetMdmMuteStatus_001
* @tc.desc   : Test GetMdmMuteStatus returns 1.0f when MDM is not muted.
*/
HWTEST_F(AudioMuteFactorManagerUnitTest, GetMdmMuteStatus_001, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    instance.SetMdmMuteStatus(MDM_NOT_MUTED);
    EXPECT_FLOAT_EQ(instance.GetMdmMuteStatus(), 1.0f);
}

/**
* @tc.name   : GetMdmMuteStatus_002
* @tc.number : GetMdmMuteStatus_002
* @tc.desc   : Test GetMdmMuteStatus returns 0.0f when MDM is muted.
*/
HWTEST_F(AudioMuteFactorManagerUnitTest, GetMdmMuteStatus_002, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    instance.SetMdmMuteStatus(MDM_MUTED);
    EXPECT_FLOAT_EQ(instance.GetMdmMuteStatus(), 0.0f);
}

/**
* @tc.name   : SetMdmMuteStatus_001
* @tc.number : SetMdmMuteStatus_001
* @tc.desc   : Test SetMdmMuteStatus toggles between muted and not muted correctly.
*/
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

/**
* @tc.name   : SetMdmMuteStatus_002
* @tc.number : SetMdmMuteStatus_002
* @tc.desc   : Test SetMdmMuteStatus with repeated calls including same-value idempotent set.
*/
HWTEST_F(AudioMuteFactorManagerUnitTest, SetMdmMuteStatus_002, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    instance.SetMdmMuteStatus(MDM_MUTED);
    EXPECT_TRUE(instance.IsMdmMuted());
    EXPECT_FLOAT_EQ(instance.GetMdmMuteStatus(), 0.0f);

    instance.SetMdmMuteStatus(MDM_NOT_MUTED);
    EXPECT_FALSE(instance.IsMdmMuted());
    EXPECT_FLOAT_EQ(instance.GetMdmMuteStatus(), 1.0f);

    instance.SetMdmMuteStatus(MDM_NOT_MUTED);
    EXPECT_FALSE(instance.IsMdmMuted());
    EXPECT_FLOAT_EQ(instance.GetMdmMuteStatus(), 1.0f);
}

/**
* @tc.name   : InitialState_001
* @tc.number : InitialState_001
* @tc.desc   : Test that SetUp initializes to not muted and state changes are observable.
*/
HWTEST_F(AudioMuteFactorManagerUnitTest, InitialState_001, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    EXPECT_FALSE(instance.IsMdmMuted());
    EXPECT_FLOAT_EQ(instance.GetMdmMuteStatus(), 1.0f);

    instance.SetMdmMuteStatus(MDM_MUTED);
    EXPECT_TRUE(instance.IsMdmMuted());
}

/**
* @tc.name   : GetAacMuteFactor_001
* @tc.number : GetAacMuteFactor_001
* @tc.desc   : Test GetAacMuteFactor returns 1.0f when whitelist is empty (no muting).
*/
HWTEST_F(AudioMuteFactorManagerUnitTest, GetAacMuteFactor_001, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    instance.ClearUnmuteWhitelist();
    int32_t appUid = 1234;
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(appUid), 1.0f);
}

/**
* @tc.name   : GetAacMuteFactor_002
* @tc.number : GetAacMuteFactor_002
* @tc.desc   : Test GetAacMuteFactor returns 1.0f when appUid is in the whitelist.
*/
HWTEST_F(AudioMuteFactorManagerUnitTest, GetAacMuteFactor_002, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    int32_t whitelistedUid = 2000;
    std::vector<int32_t> uids = {whitelistedUid};
    instance.SetUnmuteWhitelist(uids);
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(whitelistedUid), 1.0f);
}

/**
* @tc.name   : GetAacMuteFactor_003
* @tc.number : GetAacMuteFactor_003
* @tc.desc   : Test GetAacMuteFactor returns 0.0f when appUid is not in the whitelist.
*/
HWTEST_F(AudioMuteFactorManagerUnitTest, GetAacMuteFactor_003, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    int32_t whitelistedUid = 3000;
    int32_t otherUid = 3001;
    std::vector<int32_t> uids = {whitelistedUid};
    instance.SetUnmuteWhitelist(uids);
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(otherUid), 0.0f);
}

/**
* @tc.name   : SetUnmuteWhitelist_001
* @tc.number : SetUnmuteWhitelist_001
* @tc.desc   : Test SetUnmuteWhitelist with multiple uids: whitelisted uids unmuted, others muted.
*/
HWTEST_F(AudioMuteFactorManagerUnitTest, SetUnmuteWhitelist_001, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    int32_t uid1 = 4000;
    int32_t uid2 = 4001;
    int32_t uidNotInList = 4999;
    std::vector<int32_t> uids = {uid1, uid2};
    instance.SetUnmuteWhitelist(uids);
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(uid1), 1.0f);
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(uid2), 1.0f);
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(uidNotInList), 0.0f);
}

/**
* @tc.name   : SetUnmuteWhitelist_002
* @tc.number : SetUnmuteWhitelist_002
* @tc.desc   : Test SetUnmuteWhitelist with empty vector resets whitelist to empty (all unmuted).
*/
HWTEST_F(AudioMuteFactorManagerUnitTest, SetUnmuteWhitelist_002, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    int32_t uid1 = 5000;
    std::vector<int32_t> uids = {uid1};
    instance.SetUnmuteWhitelist(uids);
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(uid1), 1.0f);
    std::vector<int32_t> emptyUids;
    instance.SetUnmuteWhitelist(emptyUids);
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(uid1), 1.0f);
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(5999), 1.0f);
}

/**
* @tc.name   : SetUnmuteWhitelist_003
* @tc.number : SetUnmuteWhitelist_003
* @tc.desc   : Test SetUnmuteWhitelist replaces previous whitelist entirely.
*/
HWTEST_F(AudioMuteFactorManagerUnitTest, SetUnmuteWhitelist_003, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    std::vector<int32_t> uids1 = {6000, 6001};
    instance.SetUnmuteWhitelist(uids1);
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(6999), 0.0f);
    std::vector<int32_t> uids2 = {7000};
    instance.SetUnmuteWhitelist(uids2);
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(7000), 1.0f);
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(6000), 0.0f);
}

/**
* @tc.name   : ClearUnmuteWhitelist_001
* @tc.number : ClearUnmuteWhitelist_001
* @tc.desc   : Test ClearUnmuteWhitelist resets whitelist so all uids return 1.0f.
*/
HWTEST_F(AudioMuteFactorManagerUnitTest, ClearUnmuteWhitelist_001, testing::ext::TestSize.Level1)
{
    auto &instance = AudioMuteFactorManager::GetInstance();
    int32_t whitelistedUid = 8000;
    int32_t otherUid = 8001;
    std::vector<int32_t> uids = {whitelistedUid};
    instance.SetUnmuteWhitelist(uids);
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(otherUid), 0.0f);
    instance.ClearUnmuteWhitelist();
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(whitelistedUid), 1.0f);
    EXPECT_FLOAT_EQ(instance.GetAacMuteFactor(otherUid), 1.0f);
}
} // namespace AudioStandard
} // namespace OHOS
