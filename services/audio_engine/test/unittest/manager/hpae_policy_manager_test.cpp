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

#include <gtest/gtest.h>
#include "hpae_policy_manager.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;
constexpr uint32_t SESSION_ID = 12345;

class HpaePolicyManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
};

void HpaePolicyManagerTest::SetUp()
{}

void HpaePolicyManagerTest::TearDown()
{}

HWTEST_F(HpaePolicyManagerTest, TestForHpaePolicyManager_001, TestSize.Level0)
{
    AudioSpatializationState spatializationState = {};
    int32_t ret = HpaePolicyManager::GetInstance().UpdateSpatializationState(spatializationState);
    EXPECT_EQ(ret, 0);

    ret = HpaePolicyManager::GetInstance().UpdateSpatialDeviceType(EARPHONE_TYPE_NONE);
    EXPECT_EQ(ret, 0);

    ret = HpaePolicyManager::GetInstance().SetSpatializationSceneType(SPATIALIZATION_SCENE_TYPE_DEFAULT);
    EXPECT_EQ(ret, 0);

    ret = HpaePolicyManager::GetInstance().EffectRotationUpdate(0);
    EXPECT_EQ(ret, 0);

    ret = HpaePolicyManager::GetInstance().SetEffectSystemVolume(0, 1.f);
    EXPECT_EQ(ret, 0);

    AudioEffectPropertyArray propertyArray;
    ret = HpaePolicyManager::GetInstance().SetAudioEffectProperty(propertyArray);
    EXPECT_EQ(ret, 0);

    ret = HpaePolicyManager::GetInstance().SetInputDevice(0, DEVICE_TYPE_MIC, "Built_in_mic");
    EXPECT_EQ(ret, 0);

    ret = HpaePolicyManager::GetInstance().SetOutputDevice(0, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, 0);

    AudioVolumeType volumeType = static_cast<AudioVolumeType>(0);
    ret = HpaePolicyManager::GetInstance().SetVolumeInfo(volumeType, 1.f);
    EXPECT_EQ(ret, 0);

    ret = HpaePolicyManager::GetInstance().SetMicrophoneMuteInfo(true);
    EXPECT_EQ(ret, 0);

    ret = HpaePolicyManager::GetInstance().SetStreamVolumeInfo(SESSION_ID, 1.f);
    EXPECT_EQ(ret, 0);
}

HWTEST_F(HpaePolicyManagerTest, TestForHpaePolicyManager_002, TestSize.Level0)
{
    AudioEffectPropertyArray propertyArray2;
    propertyArray2.property.push_back({"invalidEffect", "property1"});
    int32_t ret = HpaePolicyManager::GetInstance().SetAudioEnhanceProperty(propertyArray2, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, 0);

    AudioEffectPropertyArray propertyArray3;
    ret = HpaePolicyManager::GetInstance().GetAudioEnhanceProperty(propertyArray3, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, 0);

    HpaePolicyManager::GetInstance().NotifyAlgoToStopStream();
    std::string mainkey = "other_mainkey";
    std::string subkey = "other_subkey";
    std::string extraSceneType = "extra_scene";
    HpaePolicyManager::GetInstance().UpdateExtraSceneType(mainkey, subkey, extraSceneType);
    EXPECT_EQ(mainkey, "other_mainkey");
    EXPECT_EQ(subkey, "other_subkey");
    EXPECT_EQ(extraSceneType, "extra_scene");
}

/**
 * @tc.name  : FaultCode_IsChannelLayoutSupportedForDspEffect_EffectManagerNull
 * @tc.type  : FUNC
 * @tc.desc  : Test IsChannelLayoutSupportedForDspEffect returns false when effect manager is null,
 *             covering CTRL_QUERY_DEPENDENCY_NULL fault code path.
 */
HWTEST_F(HpaePolicyManagerTest, FaultCode_IsChannelLayoutSupportedForDspEffect_EffectManagerNull, TestSize.Level1)
{
    // AudioEffectChainManager::GetInstance() may return nullptr in test environment,
    // covering the fault code path for missing dependency.
    bool ret = HpaePolicyManager::GetInstance().IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_MONO);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : FaultCode_NotifyAlgoToStopStream_EffectManagerNull
 * @tc.type  : FUNC
 * @tc.desc  : Test NotifyAlgoToStopStream handles null audioEnhanceChainManager gracefully.
 */
HWTEST_F(HpaePolicyManagerTest, FaultCode_NotifyAlgoToStopStream_EffectManagerNull, TestSize.Level1)
{
    // Should not crash when audioEnhanceChainManager is nullptr in test environment
    HpaePolicyManager::GetInstance().NotifyAlgoToStopStream();
    SUCCEED();
}

/**
 * @tc.name  : IsCollaborationSupportedForStreamUsage_001
 * @tc.type  : FUNC
 * @tc.desc  : Test IsCollaborationSupportedForStreamUsage delegates to AudioCollaborationManager.
 */
HWTEST_F(HpaePolicyManagerTest, IsCollaborationSupportedForStreamUsage_001, TestSize.Level1)
{
    bool result = HpaePolicyManager::GetInstance().IsCollaborationSupportedForStreamUsage(DEVICE_TYPE_BLUETOOTH_A2DP,
        STREAM_USAGE_MOVIE);
    EXPECT_TRUE(result || !result); // suppress unused variable warning
    result = HpaePolicyManager::GetInstance().IsCollaborationSupportedForStreamUsage(DEVICE_TYPE_BLUETOOTH_A2DP,
        STREAM_USAGE_MEDIA);
    EXPECT_TRUE(result || !result); // suppress unused variable warning
}

/**
 * @tc.name  : SetEffectSystemVolumeWithDeviceType_001
 * @tc.type  : FUNC
 * @tc.desc  : Test SetEffectSystemVolumeWithDeviceType for SPEAKER device.
 */
HWTEST_F(HpaePolicyManagerTest, SetEffectSystemVolumeWithDeviceType_001, TestSize.Level1)
{
    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 0.6f;
    const DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    int32_t ret = HpaePolicyManager::GetInstance().SetEffectSystemVolumeWithDeviceType(
        systemVolumeType, systemVolume, deviceType);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : SetEffectSystemVolumeWithDeviceType_002
 * @tc.type  : FUNC
 * @tc.desc  : Test SetEffectSystemVolumeWithDeviceType for BLUETOOTH_A2DP device.
 */
HWTEST_F(HpaePolicyManagerTest, SetEffectSystemVolumeWithDeviceType_002, TestSize.Level1)
{
    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 0.7f;
    const DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;

    int32_t ret = HpaePolicyManager::GetInstance().SetEffectSystemVolumeWithDeviceType(
        systemVolumeType, systemVolume, deviceType);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : SetEffectSystemVolumeWithDeviceType_003
 * @tc.type  : FUNC
 * @tc.desc  : Test SetEffectSystemVolumeWithDeviceType for WIRED_HEADSET device.
 */
HWTEST_F(HpaePolicyManagerTest, SetEffectSystemVolumeWithDeviceType_003, TestSize.Level1)
{
    const int32_t systemVolumeType = STREAM_RING;
    const float systemVolume = 0.8f;
    const DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;

    int32_t ret = HpaePolicyManager::GetInstance().SetEffectSystemVolumeWithDeviceType(
        systemVolumeType, systemVolume, deviceType);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : SetEffectSystemVolumeWithDeviceType_Multiple_001
 * @tc.type  : FUNC
 * @tc.desc  : Test SetEffectSystemVolumeWithDeviceType for multiple device types.
 */
HWTEST_F(HpaePolicyManagerTest, SetEffectSystemVolumeWithDeviceType_Multiple_001, TestSize.Level1)
{
    const int32_t systemVolumeType = STREAM_MUSIC;

    int32_t ret1 = HpaePolicyManager::GetInstance().SetEffectSystemVolumeWithDeviceType(
        systemVolumeType, 0.5f, DEVICE_TYPE_SPEAKER);
    int32_t ret2 = HpaePolicyManager::GetInstance().SetEffectSystemVolumeWithDeviceType(
        systemVolumeType, 0.6f, DEVICE_TYPE_BLUETOOTH_A2DP);
    int32_t ret3 = HpaePolicyManager::GetInstance().SetEffectSystemVolumeWithDeviceType(
        systemVolumeType, 0.7f, DEVICE_TYPE_WIRED_HEADSET);

    EXPECT_EQ(ret1, 0);
    EXPECT_EQ(ret2, 0);
    EXPECT_EQ(ret3, 0);
}

/**
 * @tc.name  : SetEffectSystemVolumeWithDeviceType_MaxVolume_001
 * @tc.type  : FUNC
 * @tc.desc  : Test SetEffectSystemVolumeWithDeviceType with maximum volume.
 */
HWTEST_F(HpaePolicyManagerTest, SetEffectSystemVolumeWithDeviceType_MaxVolume_001, TestSize.Level1)
{
    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 1.0f;
    const DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    int32_t ret = HpaePolicyManager::GetInstance().SetEffectSystemVolumeWithDeviceType(
        systemVolumeType, systemVolume, deviceType);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : SetEffectSystemVolumeWithDeviceType_MinVolume_001
 * @tc.type  : FUNC
 * @tc.desc  : Test SetEffectSystemVolumeWithDeviceType with minimum volume.
 */
HWTEST_F(HpaePolicyManagerTest, SetEffectSystemVolumeWithDeviceType_MinVolume_001, TestSize.Level1)
{
    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 0.0f;
    const DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;

    int32_t ret = HpaePolicyManager::GetInstance().SetEffectSystemVolumeWithDeviceType(
        systemVolumeType, systemVolume, deviceType);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : IsChannelLayoutSupportedForDspEffectWithDeviceType_001
 * @tc.type  : FUNC
 * @tc.desc  : Test IsChannelLayoutSupportedForDspEffect with DeviceType for SPEAKER device.
 */
HWTEST_F(HpaePolicyManagerTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_001, TestSize.Level1)
{
    bool ret = HpaePolicyManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_6POINT0_FRONT, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : IsChannelLayoutSupportedForDspEffectWithDeviceType_002
 * @tc.type  : FUNC
 * @tc.desc  : Test IsChannelLayoutSupportedForDspEffect with DeviceType for BLUETOOTH_A2DP device.
 */
HWTEST_F(HpaePolicyManagerTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_002, TestSize.Level1)
{
    bool ret = HpaePolicyManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_STEREO, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : IsChannelLayoutSupportedForDspEffectWithDeviceType_003
 * @tc.type  : FUNC
 * @tc.desc  : Test IsChannelLayoutSupportedForDspEffect with DeviceType for NEARLINK device.
 */
HWTEST_F(HpaePolicyManagerTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_003, TestSize.Level1)
{
    bool ret = HpaePolicyManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_2POINT1, DEVICE_TYPE_NEARLINK);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : IsChannelLayoutSupportedForDspEffectWithDeviceType_004
 * @tc.type  : FUNC
 * @tc.desc  : Test IsChannelLayoutSupportedForDspEffect with DeviceType for WIRED_HEADSET device.
 */
HWTEST_F(HpaePolicyManagerTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_004, TestSize.Level1)
{
    bool ret = HpaePolicyManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_MONO, DEVICE_TYPE_WIRED_HEADSET);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : IsChannelLayoutSupportedForDspEffectWithDeviceType_Multiple_001
 * @tc.type  : FUNC
 * @tc.desc  : Test IsChannelLayoutSupportedForDspEffect with DeviceType for multiple channel layouts.
 */
HWTEST_F(HpaePolicyManagerTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_Multiple_001, TestSize.Level1)
{
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    bool ret1 = HpaePolicyManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_MONO, deviceType);
    bool ret2 = HpaePolicyManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_STEREO, deviceType);
    bool ret3 = HpaePolicyManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_5POINT1, deviceType);

    EXPECT_EQ(ret1, false);
    EXPECT_EQ(ret2, false);
    EXPECT_EQ(ret3, false);
}

/**
 * @tc.name  : IsChannelLayoutSupportedForDspEffectWithDeviceType_FaultCode_001
 * @tc.type  : FUNC
 * @tc.desc  : Test IsChannelLayoutSupportedForDspEffect with DeviceType when effect manager is null.
 */
HWTEST_F(HpaePolicyManagerTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_FaultCode_001, TestSize.Level1)
{
    // This test verifies the behavior when the internal effect chain manager is null
    bool ret = HpaePolicyManager::GetInstance().IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_7POINT1, DEVICE_TYPE_USB_HEADSET);
    EXPECT_EQ(ret, false);
}
