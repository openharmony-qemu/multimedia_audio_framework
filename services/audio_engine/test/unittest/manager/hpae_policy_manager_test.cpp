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

constexpr uint32_t SESSION_ID = 12345;

class HpaePolicyManagerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    std::shared_ptr<HpaePolicyManager> hpaePolicyManager_ = nullptr;
};

void HpaePolicyManagerTest::SetUp()
{
    hpaePolicyManager_ = std::make_shared<HpaePolicyManager>();
}

void HpaePolicyManagerTest::TearDown()
{}

TEST_F(HpaePolicyManagerTest, allTestForHpaePolicyManager)
{
    AudioSpatializationState spatializationState = {};
    int32_t ret = hpaePolicyManager_->UpdateSpatializationState(spatializationState);
    EXPECT_EQ(ret, 0);

    ret = hpaePolicyManager_->UpdateSpatialDeviceType(EARPHONE_TYPE_NONE);
    EXPECT_EQ(ret, 0);

    ret = hpaePolicyManager_->SetSpatializationSceneType(SPATIALIZATION_SCENE_TYPE_DEFAULT);
    EXPECT_EQ(ret, 0);

    ret = hpaePolicyManager_->EffectRotationUpdate(0);
    EXPECT_EQ(ret, 0);

    ret = hpaePolicyManager_->SetEffectSystemVolume(0, 1.f);
    EXPECT_EQ(ret, 0);

    AudioEffectPropertyArrayV3 propertyArray;
    ret = hpaePolicyManager_->SetAudioEffectProperty(propertyArray);
    EXPECT_EQ(ret, 0);

    AudioEffectPropertyArray propertyArray1;
    ret = hpaePolicyManager_->GetAudioEffectProperty(propertyArray1);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(propertyArray1.property.size(), 0);

    ret = hpaePolicyManager_->SetInputDevice(0, DEVICE_TYPE_MIC, "Built_in_mic");
    EXPECT_EQ(ret, 0);

    ret = hpaePolicyManager_->SetOutputDevice(0, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, 0);

    AudioVolumeType volumeType = static_cast<AudioVolumeType>(0);
    ret = hpaePolicyManager_->SetVolumeInfo(volumeType, 1.f);
    EXPECT_EQ(ret, 0);

    ret = hpaePolicyManager_->SetMicrophoneMuteInfo(true);
    EXPECT_EQ(ret, 0);

    ret = hpaePolicyManager_->SetStreamVolumeInfo(SESSION_ID, 1.f);
    EXPECT_EQ(ret, 0);

    AudioEffectPropertyArrayV3 propertyArray2;
    propertyArray.property.push_back({"invalidEffect", "property1"});
    ret = hpaePolicyManager_->SetAudioEnhanceProperty(propertyArray2, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, 0);

    AudioEffectPropertyArrayV3 propertyArray3;
    ret = hpaePolicyManager_->GetAudioEnhanceProperty(propertyArray3, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, 0);

    AudioEnhancePropertyArray propertyArray4;
    ret = hpaePolicyManager_->SetAudioEnhanceProperty(propertyArray4, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, 0);

    AudioEnhancePropertyArray propertyArray5;
    ret = hpaePolicyManager_->GetAudioEnhanceProperty(propertyArray5, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, 0);

    std::string mainkey = "other_mainkey";
    std::string subkey = "other_subkey";
    std::string extraSceneType = "extra_scene";
    hpaePolicyManager_->UpdateExtraSceneType(mainkey, subkey, extraSceneType);
    EXPECT_EQ(mainkey, "other_mainkey");
    EXPECT_EQ(subkey, "other_subkey");
    EXPECT_EQ(extraSceneType, "extra_scene");
}
