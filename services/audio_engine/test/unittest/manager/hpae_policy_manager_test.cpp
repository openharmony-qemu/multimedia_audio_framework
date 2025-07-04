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

    AudioEffectPropertyArrayV3 propertyArray;
    ret = HpaePolicyManager::GetInstance().SetAudioEffectProperty(propertyArray);
    EXPECT_EQ(ret, 0);

    AudioEffectPropertyArray propertyArray1;
    ret = HpaePolicyManager::GetInstance().GetAudioEffectProperty(propertyArray1);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(propertyArray1.property.size(), 0);

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
    AudioEffectPropertyArrayV3 propertyArray2;
    propertyArray2.property.push_back({"invalidEffect", "property1"});
    int32_t ret = HpaePolicyManager::GetInstance().SetAudioEnhanceProperty(propertyArray2, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, 0);

    AudioEffectPropertyArrayV3 propertyArray3;
    ret = HpaePolicyManager::GetInstance().GetAudioEnhanceProperty(propertyArray3, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, 0);

    AudioEnhancePropertyArray propertyArray4;
    ret = HpaePolicyManager::GetInstance().SetAudioEnhanceProperty(propertyArray4, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, 0);

    AudioEnhancePropertyArray propertyArray5;
    ret = HpaePolicyManager::GetInstance().GetAudioEnhanceProperty(propertyArray5, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, 0);

    std::string mainkey = "other_mainkey";
    std::string subkey = "other_subkey";
    std::string extraSceneType = "extra_scene";
    HpaePolicyManager::GetInstance().UpdateExtraSceneType(mainkey, subkey, extraSceneType);
    EXPECT_EQ(mainkey, "other_mainkey");
    EXPECT_EQ(subkey, "other_subkey");
    EXPECT_EQ(extraSceneType, "extra_scene");
}
