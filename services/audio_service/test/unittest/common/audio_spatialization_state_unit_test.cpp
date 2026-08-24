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
#include <cstring>
#include <memory>
#include <string>
#include "audio_spatialization_types.h"
#include "parcel.h"

using namespace testing::ext;
using namespace OHOS::AudioStandard;

class AudioSpatializationStateTest : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}
};

/**
 * @tc.name: AudioSpatializationState_Marshalling_001
 * @tc.type: FUNC
 * @tc.number: AudioSpatializationState_Marshalling_001
 * @tc.desc: Marshalling succeeds and all six fields (incl. deviceType/macAddress) survive a round-trip.
 */
HWTEST_F(AudioSpatializationStateTest, AudioSpatializationState_Marshalling_001, TestSize.Level0)
{
    AudioSpatializationState original(true, true, true, false, 7, "AA:BB:CC:DD:EE:FF");

    OHOS::Parcel parcel;
    EXPECT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    std::unique_ptr<AudioSpatializationState> restored(AudioSpatializationState::Unmarshalling(parcel));
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->spatializationEnabled, true);
    EXPECT_EQ(restored->headTrackingEnabled, true);
    EXPECT_EQ(restored->adaptiveSpatialRenderingEnabled, true);
    EXPECT_EQ(restored->personalizedSpatializationEnabled, false);
    EXPECT_EQ(restored->deviceType, 7);
    EXPECT_EQ(restored->macAddress, "AA:BB:CC:DD:EE:FF");
}

/**
 * @tc.name: AudioSpatializationState_Marshalling_002
 * @tc.type: FUNC
 * @tc.number: AudioSpatializationState_Marshalling_002
 * @tc.desc: Default-constructed state (deviceType = 0, macAddress empty) round-trips correctly.
 */
HWTEST_F(AudioSpatializationStateTest, AudioSpatializationState_Marshalling_002, TestSize.Level0)
{
    AudioSpatializationState original;

    OHOS::Parcel parcel;
    EXPECT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    std::unique_ptr<AudioSpatializationState> restored(AudioSpatializationState::Unmarshalling(parcel));
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->spatializationEnabled, false);
    EXPECT_EQ(restored->headTrackingEnabled, false);
    EXPECT_EQ(restored->adaptiveSpatialRenderingEnabled, false);
    EXPECT_EQ(restored->personalizedSpatializationEnabled, false);
    EXPECT_EQ(restored->deviceType, 0);
    EXPECT_EQ(restored->macAddress, "");
}

/**
 * @tc.name: AudioSpatializationState_Marshalling_003
 * @tc.type: FUNC
 * @tc.number: AudioSpatializationState_Marshalling_003
 * @tc.desc: A populated deviceType with an empty macAddress still round-trips (mixed field coverage).
 */
HWTEST_F(AudioSpatializationStateTest, AudioSpatializationState_Marshalling_003, TestSize.Level0)
{
    AudioSpatializationState original;
    original.spatializationEnabled = true;
    original.headTrackingEnabled = false;
    original.adaptiveSpatialRenderingEnabled = true;
    original.personalizedSpatializationEnabled = true;
    original.deviceType = 3; // DEVICE_TYPE_BLUETOOTH_A2DP-like value
    original.macAddress = "";

    OHOS::Parcel parcel;
    EXPECT_TRUE(original.Marshalling(parcel));

    parcel.RewindRead(0);
    std::unique_ptr<AudioSpatializationState> restored(AudioSpatializationState::Unmarshalling(parcel));
    ASSERT_NE(restored, nullptr);
    EXPECT_EQ(restored->deviceType, 3);
    EXPECT_EQ(restored->macAddress, "");
    EXPECT_EQ(restored->spatializationEnabled, true);
    EXPECT_EQ(restored->personalizedSpatializationEnabled, true);
}
