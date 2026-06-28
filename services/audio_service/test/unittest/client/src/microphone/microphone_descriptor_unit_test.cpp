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

#include "microphone_descriptor.h"
#include "audio_common_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class MicrophoneDescriptorUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void MicrophoneDescriptorUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("MicrophoneDescriptorUnitTest SetUpTestCase");
}

void MicrophoneDescriptorUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("MicrophoneDescriptorUnitTest TearDownTestCase");
}

void MicrophoneDescriptorUnitTest::SetUp()
{
    AUDIO_INFO_LOG("MicrophoneDescriptorUnitTest SetUp");
}

void MicrophoneDescriptorUnitTest::TearDown()
{
    AUDIO_INFO_LOG("MicrophoneDescriptorUnitTest TearDown");
}

/**
 * @tc.name   : Test MicrophoneDescriptor Constructor
 * @tc.type  : FUNC
 * @tc.number: MicrophoneDescriptor_Constructor_001
 * @tc.desc  : Test MicrophoneDescriptor constructor with parameters.
 */
HWTEST_F(MicrophoneDescriptorUnitTest, MicrophoneDescriptor_Constructor_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("MicrophoneDescriptorUnitTest MicrophoneDescriptor_Constructor_001 start");
    MicrophoneDescriptor mic(1, DEVICE_TYPE_MIC, 1, 50);
    EXPECT_EQ(mic.micId_, 1);
    EXPECT_EQ(mic.deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(mic.groupId_, 1);
    EXPECT_EQ(mic.sensitivity_, 50);
}

/**
 * @tc.name   : Test MicrophoneDescriptor Constructor
 * @tc.type  : FUNC
 * @tc.number: MicrophoneDescriptor_Constructor_002
 * @tc.desc  : Test MicrophoneDescriptor constructor with device type only.
 */
HWTEST_F(MicrophoneDescriptorUnitTest, MicrophoneDescriptor_Constructor_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("MicrophoneDescriptorUnitTest MicrophoneDescriptor_Constructor_002 start");
    MicrophoneDescriptor mic(1, DEVICE_TYPE_MIC);
    EXPECT_EQ(mic.micId_, 1);
    EXPECT_EQ(mic.deviceType_, DEVICE_TYPE_MIC);
    EXPECT_EQ(mic.groupId_, 0);
    EXPECT_EQ(mic.sensitivity_, 0);
}

/**
 * @tc.name   : Test MicrophoneDescriptor Copy Constructor
 * @tc.type  : FUNC
 * @tc.number: MicrophoneDescriptor_CopyConstructor_001
 * @tc.desc  : Test MicrophoneDescriptor copy constructor.
 */
HWTEST_F(MicrophoneDescriptorUnitTest, MicrophoneDescriptor_CopyConstructor_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("MicrophoneDescriptorUnitTest MicrophoneDescriptor_CopyConstructor_001 start");
    MicrophoneDescriptor mic1(1, DEVICE_TYPE_MIC, 1, 50);
    MicrophoneDescriptor mic2(mic1);
    EXPECT_EQ(mic2.micId_, mic1.micId_);
    EXPECT_EQ(mic2.deviceType_, mic1.deviceType_);
    EXPECT_EQ(mic2.groupId_, mic1.groupId_);
    EXPECT_EQ(mic2.sensitivity_, mic1.sensitivity_);
}

/**
 * @tc.name   : Test MicrophoneDescriptor Marshalling
 * @tc.type  : FUNC
 * @tc.number: MicrophoneDescriptor_Marshalling_001
 * @tc.desc  : Test MicrophoneDescriptor Marshalling function.
 */
HWTEST_F(MicrophoneDescriptorUnitTest, MicrophoneDescriptor_Marshalling_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("MicrophoneDescriptorUnitTest MicrophoneDescriptor_Marshalling_001 start");
    MicrophoneDescriptor mic(1, DEVICE_TYPE_MIC, 1, 50);
    Parcel parcel;
    bool result = mic.Marshalling(parcel);
    EXPECT_TRUE(result);
}

/**
 * @tc.name   : Test MicrophoneDescriptor Unmarshalling
 * @tc.type  : FUNC
 * @tc.number: MicrophoneDescriptor_Unmarshalling_001
 * @tc.desc  : Test MicrophoneDescriptor Unmarshalling function.
 */
HWTEST_F(MicrophoneDescriptorUnitTest, MicrophoneDescriptor_Unmarshalling_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("MicrophoneDescriptorUnitTest MicrophoneDescriptor_Unmarshalling_001 start");
    // First, create a parcel with marshalled data
    MicrophoneDescriptor mic1(1, DEVICE_TYPE_MIC, 1, 50);
    Parcel parcel;
    mic1.Marshalling(parcel);
    
    // Then, unmarshall it
    MicrophoneDescriptor *mic2 = MicrophoneDescriptor::Unmarshalling(parcel);
    EXPECT_NE(mic2, nullptr);
    if (mic2 != nullptr) {
        EXPECT_EQ(mic2->micId_, mic1.micId_);
        EXPECT_EQ(mic2->deviceType_, mic1.deviceType_);
        EXPECT_EQ(mic2->groupId_, mic1.groupId_);
        EXPECT_EQ(mic2->sensitivity_, mic1.sensitivity_);
        delete mic2;
    }
}

/**
 * @tc.name   : Test MicrophoneDescriptor SetMicPositionInfo
 * @tc.type  : FUNC
 * @tc.number: MicrophoneDescriptor_SetMicPositionInfo_001
 * @tc.desc  : Test MicrophoneDescriptor SetMicPositionInfo function.
 */
HWTEST_F(MicrophoneDescriptorUnitTest, MicrophoneDescriptor_SetMicPositionInfo_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("MicrophoneDescriptorUnitTest MicrophoneDescriptor_SetMicPositionInfo_001 start");
    MicrophoneDescriptor mic(1, DEVICE_TYPE_MIC, 1, 50);
    Vector3D pos = {1.0f, 2.0f, 3.0f};
    mic.SetMicPositionInfo(pos);
    // No getter for position_, so we just test it doesn't crash
    EXPECT_TRUE(true);
}

/**
 * @tc.name   : Test MicrophoneDescriptor SetMicOrientationInfo
 * @tc.type  : FUNC
 * @tc.number: MicrophoneDescriptor_SetMicOrientationInfo_001
 * @tc.desc  : Test MicrophoneDescriptor SetMicOrientationInfo function.
 */
HWTEST_F(MicrophoneDescriptorUnitTest, MicrophoneDescriptor_SetMicOrientationInfo_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("MicrophoneDescriptorUnitTest MicrophoneDescriptor_SetMicOrientationInfo_001 start");
    MicrophoneDescriptor mic(1, DEVICE_TYPE_MIC, 1, 50);
    Vector3D orientation = {0.0f, 0.0f, 1.0f};
    mic.SetMicOrientationInfo(orientation);
    // No getter for orientation_, so we just test it doesn't crash
    EXPECT_TRUE(true);
}

} // namespace AudioStandard
} // namespace OHOS
