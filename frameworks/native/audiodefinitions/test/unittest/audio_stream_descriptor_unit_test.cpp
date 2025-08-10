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

#include "audio_stream_descriptor.h"
#include "audio_pipe_info.h"

#include <cinttypes>
#include "audio_common_log.h"
#include "audio_utils.h"
#include <gtest/gtest.h>

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace AudioStandard {

static const int32_t MAX_STREAM_DESCRIPTORS_SIZE = 1003;

class AudioStreamDescriptorUnitTest : public ::testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    virtual void SetUp(){};
    virtual void TearDown(){};
};

class AudioPipeInfoUnitTest : public ::testing::Test {
public:
    static void SetUpTestCase(){};
    static void TearDownTestCase(){};
    virtual void SetUp(){};
    virtual void TearDown(){};
};

/**
 * @tc.name   : Test InitAudioStreamInfo
 * @tc.number : InitAudioStreamInfo_001
 * @tc.desc   : Test InitAudioStreamInfo
 */
HWTEST_F(AudioPipeInfoUnitTest, InitAudioStreamInfo_001, TestSize.Level1)
{
    AudioPipeInfo info;
    info.InitAudioStreamInfo();
    EXPECT_EQ(info.audioStreamInfo_.format, AudioSampleFormat::INVALID_WIDTH);

    info.moduleInfo_.rate = "48000";
    info.moduleInfo_.channels = "2";
    info.moduleInfo_.format = "s16";
    info.InitAudioStreamInfo();
    EXPECT_NE(info.audioStreamInfo_.format, AudioSampleFormat::INVALID_WIDTH);
}

/**
 * @tc.name   : Test WriteDeviceDescVectorToParcel
 * @tc.number : WriteDeviceDescVectorToParcel_001
 * @tc.desc   : Test WriteDeviceDescVectorToParcel
 */
HWTEST_F(AudioStreamDescriptorUnitTest, WriteDeviceDescVectorToParcel_001, TestSize.Level1)
{
    AudioStreamDescriptor audioStreamDescriptor;
    Parcel parcel;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs(MAX_STREAM_DESCRIPTORS_SIZE,
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE));
    EXPECT_TRUE(audioStreamDescriptor.WriteDeviceDescVectorToParcel(parcel, descs));
}

/**
 * @tc.name   : Test Dump
 * @tc.number : Dump_001
 * @tc.desc   : Test Dump
 */
HWTEST_F(AudioStreamDescriptorUnitTest, Dump_001, TestSize.Level1)
{
    AudioDeviceDescriptor audioDeviceDescriptor(DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    audioDeviceDescriptor.deviceId_ = 1;
    audioDeviceDescriptor.deviceName_ = "BuiltinSpeaker";
    std::string dumpString;
    std::string expected = "      - device 1: role Output type 2 (SPEAKER) name: BuiltinSpeaker\n";
    audioDeviceDescriptor.Dump(dumpString);
    EXPECT_EQ(dumpString, expected);
}

/**
 * @tc.name   : Test Dump
 * @tc.number : Dump_002
 * @tc.desc   : Test Dump
 */
HWTEST_F(AudioStreamDescriptorUnitTest, Dump_002, TestSize.Level1)
{
    AudioDeviceDescriptor audioDeviceDescriptor(DeviceType::DEVICE_TYPE_MIC, DeviceRole::INPUT_DEVICE);
    audioDeviceDescriptor.deviceId_ = 2;
    audioDeviceDescriptor.deviceName_ = "BuiltinMic";
    std::string dumpString;
    std::string expected = "      - device 2: role Input type 15 (MIC) name: BuiltinMic\n";
    audioDeviceDescriptor.Dump(dumpString);
    EXPECT_EQ(dumpString, expected);
}
} // namespace AudioStandard
} // namespace OHOS