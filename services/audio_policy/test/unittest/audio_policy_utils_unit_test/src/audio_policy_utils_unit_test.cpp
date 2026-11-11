/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_policy_utils_unit_test.h"
#include "audio_errors.h"
#include "audio_router_infra.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioPolicyUtilsUnitTest::SetUp(void) {}

void AudioPolicyUtilsUnitTest::TearDown(void) {}

/**
 * @tc.name  : IsHigherSpecStreamInfo_001
 * @tc.type  : FUNC
 * @tc.desc  : Test sample rate, channel and format priority, including equal specifications.
 */
HWTEST(AudioPolicyUtilsUnitTest, IsHigherSpecStreamInfo_001, TestSize.Level1)
{
    AudioStreamInfo selectedInfo = {};
    selectedInfo.samplingRate = SAMPLE_RATE_48000;
    selectedInfo.channels = STEREO;
    selectedInfo.format = SAMPLE_S16LE;
    AudioStreamInfo info = selectedInfo;
    EXPECT_FALSE(AudioPolicyUtils::IsHigherSpecStreamInfo(info, selectedInfo));

    info.samplingRate = SAMPLE_RATE_96000;
    info.channels = MONO;
    info.format = SAMPLE_U8;
    EXPECT_TRUE(AudioPolicyUtils::IsHigherSpecStreamInfo(info, selectedInfo));
    EXPECT_FALSE(AudioPolicyUtils::IsHigherSpecStreamInfo(selectedInfo, info));

    info.samplingRate = selectedInfo.samplingRate;
    info.channels = CHANNEL_8;
    EXPECT_TRUE(AudioPolicyUtils::IsHigherSpecStreamInfo(info, selectedInfo));
    EXPECT_FALSE(AudioPolicyUtils::IsHigherSpecStreamInfo(selectedInfo, info));

    info.channels = selectedInfo.channels;
    info.format = SAMPLE_S32LE;
    EXPECT_TRUE(AudioPolicyUtils::IsHigherSpecStreamInfo(info, selectedInfo));
    EXPECT_FALSE(AudioPolicyUtils::IsHigherSpecStreamInfo(selectedInfo, info));
    selectedInfo.format = SAMPLE_F32LE;
    EXPECT_FALSE(AudioPolicyUtils::IsHigherSpecStreamInfo(info, selectedInfo));
    EXPECT_TRUE(AudioPolicyUtils::IsHigherSpecStreamInfo(selectedInfo, info));
}

/**
 * @tc.name  : Test GetNewSinkPortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_002
 * @tc.desc  : Test GetNewSinkPortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_002, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_ACCESSORY;

    std::string ret = audioPolicyUtilsTest_->GetNewSinkPortName(deviceType);
    EXPECT_EQ(ret, ACCESSORY_SOURCE);
}

/**
 * @tc.name  : Test GetNewSinkPortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_003
 * @tc.desc  : Test GetNewSinkPortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_003, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_HEARING_AID;

    std::string ret = audioPolicyUtilsTest_->GetNewSinkPortName(deviceType);
    EXPECT_EQ(ret, HEARING_AID_SPEAKER);
}

/**
 * @tc.name  : Test GetSinkName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_004
 * @tc.desc  : Test GetSinkName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_004, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    int32_t descriptorType = 0;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(descriptorType);;
    desc->networkId_ = LOCAL_NETWORK_ID;
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    int32_t sessionId = 0;

    std::string ret = audioPolicyUtilsTest_->GetSinkName(desc, sessionId);
    EXPECT_EQ(ret, PRIMARY_SPEAKER);
}

/**
 * @tc.name  : Test GetSinkName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_005
 * @tc.desc  : Test GetSinkName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_005, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    int32_t descriptorType = 0;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(descriptorType);;
    desc->networkId_ = "";
    desc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    int32_t sessionId = 0;
    std::string test_result = "_out";

    std::string ret = audioPolicyUtilsTest_->GetSinkName(desc, sessionId);
    EXPECT_EQ(ret, test_result);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_006
 * @tc.desc  : Test GetSourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_006, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN;

    std::string ret = audioPolicyUtilsTest_->GetSourcePortName(deviceType);
    EXPECT_EQ(ret, BLUETOOTH_MIC);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_007
 * @tc.desc  : Test GetSourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_007, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_ACCESSORY;

    std::string ret = audioPolicyUtilsTest_->GetSourcePortName(deviceType);
    EXPECT_EQ(ret, ACCESSORY_SOURCE);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_007_Ext02
 * @tc.desc  : Test GetSourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_007_Ext02, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_MIC;
    uint32_t routeFlag = AUDIO_INPUT_FLAG_UNPROCESS;

    std::string ret = audioPolicyUtilsTest_->GetSourcePortName(deviceType, routeFlag);
    EXPECT_EQ(ret, PRIMARY_UNPROCESS_MIC);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_007_Ext03
 * @tc.desc  : Test GetSourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_007_Ext03, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_MIC;
    uint32_t routeFlag = AUDIO_INPUT_FLAG_NORMAL;

    std::string ret = audioPolicyUtilsTest_->GetSourcePortName(deviceType, routeFlag);
    EXPECT_EQ(ret, PRIMARY_MIC);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_007_Ext04
 * @tc.desc  : Test GetSourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_007_Ext04, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_MIC;
    uint32_t routeFlag = AUDIO_INPUT_FLAG_ULTRASONIC;

    std::string ret = audioPolicyUtilsTest_->GetSourcePortName(deviceType, routeFlag);
    EXPECT_EQ(ret, PRIMARY_ULTRASONIC_MIC);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_007_Ext05
 * @tc.desc  : Test GetSourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_007_Ext05, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_MIC;
    uint32_t routeFlag = AUDIO_INPUT_FLAG_LIVE;

    std::string ret = audioPolicyUtilsTest_->GetSourcePortName(deviceType, routeFlag);
    EXPECT_EQ(ret, PRIMARY_LIVE_MIC);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_007_Ext06
 * @tc.desc  : Test GetSourcePortName with camcorder route flag.
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_007_Ext06, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_MIC;
    uint32_t routeFlag = AUDIO_INPUT_FLAG_CAMCORDER;

    std::string ret = audioPolicyUtilsTest_->GetSourcePortName(deviceType, routeFlag);
    EXPECT_EQ(ret, PRIMARY_CAMCORDER);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_007_Ext07
 * @tc.desc  : Test GetSourcePortName with voice message route flag.
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_007_Ext07, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_MIC;
    uint32_t routeFlag = AUDIO_INPUT_FLAG_VOICE_MESSAGE;

    std::string ret = audioPolicyUtilsTest_->GetSourcePortName(deviceType, routeFlag);
    EXPECT_EQ(ret, PRIMARY_VOICE_MESSAGE_MIC);
}

/**
 * @tc.name  : Test GetOutputDeviceClassBySinkPortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_008
 * @tc.desc  : Test GetOutputDeviceClassBySinkPortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_008, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string sinkPortName = BLUETOOTH_SPEAKER;

    std::string ret = audioPolicyUtilsTest_->GetOutputDeviceClassBySinkPortName(sinkPortName);
    EXPECT_EQ(ret, A2DP_CLASS);
}

/**
 * @tc.name  : Test GetInputDeviceClassBySourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_009
 * @tc.desc  : Test GetInputDeviceClassBySourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_009, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string sourcePortName = BLUETOOTH_MIC;

    std::string ret = audioPolicyUtilsTest_->GetInputDeviceClassBySourcePortName(sourcePortName);
    EXPECT_EQ(ret, A2DP_CLASS);
}

/**
 * @tc.name  : Test GetInputDeviceClassBySourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_010
 * @tc.desc  : Test GetInputDeviceClassBySourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_010, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string sourcePortName = "test";

    std::string ret = audioPolicyUtilsTest_->GetInputDeviceClassBySourcePortName(sourcePortName);
    EXPECT_EQ(ret, INVALID_CLASS);
}

/**
 * @tc.name  : Test GetInputDeviceClassBySourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_010A
 * @tc.desc  : Test GetInputDeviceClassBySourcePortName for primary extra input ports
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_010A, TestSize.Level1)
{
    AudioPolicyUtils *audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    EXPECT_EQ(audioPolicyUtilsTest_->GetInputDeviceClassBySourcePortName(PRIMARY_TYPEC_CLASS), PRIMARY_CLASS);
    EXPECT_EQ(audioPolicyUtilsTest_->GetInputDeviceClassBySourcePortName(PRIMARY_WIRED_CLASS), PRIMARY_CLASS);
    EXPECT_EQ(audioPolicyUtilsTest_->GetInputDeviceClassBySourcePortName(PRIMARY_BT_CLASS), PRIMARY_CLASS);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_010B
 * @tc.desc  : Test enhanced routing input source port names use deviceMsmd mapping
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_010B, TestSize.Level1)
{
    AudioPolicyUtils *audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    EXPECT_EQ(audioPolicyUtilsTest_->deviceMsmd[DEVICE_TYPE_USB_HEADSET], PRIMARY_TYPEC_CLASS);
    EXPECT_EQ(audioPolicyUtilsTest_->deviceMsmd[DEVICE_TYPE_NEARLINK_IN], PRIMARY_BT_CLASS);
    EXPECT_EQ(audioPolicyUtilsTest_->deviceMsmd[DEVICE_TYPE_BLUETOOTH_SCO], PRIMARY_BT_CLASS);
}

/**
 * @tc.name  : Test GetInputDeviceClassBySourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_010_Ext01
 * @tc.desc  : Test GetInputDeviceClassBySourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_010_Ext01, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string sourcePortName = PRIMARY_LIVE_MIC;

    std::string ret = audioPolicyUtilsTest_->GetInputDeviceClassBySourcePortName(sourcePortName);
    EXPECT_EQ(ret, PRIMARY_CLASS);
}

/**
 * @tc.name  : Test GetInputDeviceClassBySourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_010_Ext02
 * @tc.desc  : Test GetInputDeviceClassBySourcePortName with camcorder source port.
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_010_Ext02, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string sourcePortName = PRIMARY_CAMCORDER;

    std::string ret = audioPolicyUtilsTest_->GetInputDeviceClassBySourcePortName(sourcePortName);
    EXPECT_EQ(ret, PRIMARY_CLASS);
}

/**
 * @tc.name  : Test GetInputDeviceClassBySourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_010_Ext03
 * @tc.desc  : Test GetInputDeviceClassBySourcePortName with voice message source port.
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_010_Ext03, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string sourcePortName = PRIMARY_VOICE_MESSAGE_MIC;

    std::string ret = audioPolicyUtilsTest_->GetInputDeviceClassBySourcePortName(sourcePortName);
    EXPECT_EQ(ret, PRIMARY_CLASS);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_011
 * @tc.desc  : Test GetDeviceType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_011, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = "Built_in_wakeup";

    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_WAKEUP);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_012
 * @tc.desc  : Test GetDeviceType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_012, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = "fifo_output";

    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_BLUETOOTH_SCO);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_013
 * @tc.desc  : Test GetDeviceType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_013, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = "fifo_input";

    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_BLUETOOTH_SCO);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_014
 * @tc.desc  : Test GetDeviceType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_014, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = "file_sink";

    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_FILE_SINK);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_015
 * @tc.desc  : Test GetDeviceType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_015, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = "test";

    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_NONE);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_015_ext02
 * @tc.desc  : Test GetDeviceType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_015_ext02, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = PRIMARY_UNPROCESS_MIC;
    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_MIC);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_015_ext03
 * @tc.desc  : Test GetDeviceType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_015_ext03, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = PRIMARY_ULTRASONIC_MIC;
    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_MIC);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_016_ext03
 * @tc.desc  : Test GetDeviceType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_016_ext03, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = PRIMARY_VOICE_RECOGNITION_MIC;
    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_MIC);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_016_ext06
 * @tc.desc  : Test GetDeviceType with voice message mic.
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_016_ext06, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = PRIMARY_VOICE_MESSAGE_MIC;
    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_MIC);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_016_ext04
 * @tc.desc  : Test GetDeviceType
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_016_ext04, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = PRIMARY_LIVE_MIC;
    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_MIC);
}

/**
 * @tc.name  : Test GetDeviceType API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_016_ext05
 * @tc.desc  : Test GetDeviceType with camcorder source port.
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_016_ext05, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string deviceName = PRIMARY_CAMCORDER;
    DeviceType ret = audioPolicyUtilsTest_->GetDeviceType(deviceName);
    EXPECT_EQ(ret, DEVICE_TYPE_MIC);
}

/**
 * @tc.name  : Test IsSupportedDevice API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_017
 * @tc.desc  : Test IsSupportedDevice with system permission
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_017, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string bundleName = "com.ohos.test";
    bool hasSystemPermission = true;

    int32_t ret = audioPolicyUtilsTest_->IsSupportedDevice(bundleName, DEVICE_TYPE_NEARLINK, hasSystemPermission);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsSupportedDevice API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_018
 * @tc.desc  : Test IsSupportedDevice without system permission.
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_018, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string bundleName = "com.ohos.test";
    bool hasSystemPermission = false;

    int32_t ret = audioPolicyUtilsTest_->IsSupportedDevice(bundleName, DEVICE_TYPE_NEARLINK, hasSystemPermission);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test IsSupportedDevice API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_019
 * @tc.desc  : Test IsSupportedDevice without system permission with being declared by app.
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_019, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    std::string bundleName = "com.ohos.test";
    bool hasSystemPermission = false;
    audioPolicyUtilsTest_->SetDeclaredDeviceTypes(bundleName, {DEVICE_TYPE_NEARLINK, DEVICE_TYPE_HEARING_AID});
    int32_t ret = audioPolicyUtilsTest_->IsSupportedDevice(bundleName, DEVICE_TYPE_NEARLINK, hasSystemPermission);
    EXPECT_EQ(ret, true);
    ret = audioPolicyUtilsTest_->IsSupportedDevice(bundleName, DEVICE_TYPE_NEARLINK_IN, hasSystemPermission);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test IsWirelessDevice API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_020
 * @tc.desc  : Test IsWirelessDevice.
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_020, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType type = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    int32_t ret = audioPolicyUtilsTest_->IsWirelessDevice(type);
    EXPECT_EQ(ret, true);

    type = DeviceType::DEVICE_TYPE_BLUETOOTH_SCO;
    ret = audioPolicyUtilsTest_->IsWirelessDevice(type);
    EXPECT_EQ(ret, true);

    type = DeviceType::DEVICE_TYPE_NEARLINK;
    ret = audioPolicyUtilsTest_->IsWirelessDevice(type);
    EXPECT_EQ(ret, true);

    type = DeviceType::DEVICE_TYPE_NEARLINK_IN;
    ret = audioPolicyUtilsTest_->IsWirelessDevice(type);
    EXPECT_EQ(ret, true);

    type = DeviceType::DEVICE_TYPE_SPEAKER;
    ret = audioPolicyUtilsTest_->IsWirelessDevice(type);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_021
 * @tc.desc  : Test GetSourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_021, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    uint32_t routeFlag = AUDIO_INPUT_FLAG_NORMAL;

    std::string ret = audioPolicyUtilsTest_->GetSourcePortName(deviceType, routeFlag);
    EXPECT_EQ(ret, PRIMARY_MIC);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_024
 * @tc.desc  : Test GetSourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_024, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_WIRED_MICIN;
    uint32_t routeFlag = AUDIO_INPUT_FLAG_NORMAL;

    std::string ret = audioPolicyUtilsTest_->GetSourcePortName(deviceType, routeFlag);
    EXPECT_EQ(ret, PRIMARY_MIC);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_025
 * @tc.desc  : Test GetSourcePortName
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_025, TestSize.Level1)
{
    AudioPolicyUtils* audioPolicyUtilsTest_ = nullptr;
    audioPolicyUtilsTest_ = &AudioPolicyUtils::GetInstance();
    ASSERT_TRUE(audioPolicyUtilsTest_ != nullptr);

    DeviceType deviceType = DeviceType::DEVICE_TYPE_LINE_DIGITAL;
    uint32_t routeFlag = AUDIO_INPUT_FLAG_NORMAL;

    std::string ret = audioPolicyUtilsTest_->GetSourcePortName(deviceType, routeFlag);
    EXPECT_EQ(ret, PRIMARY_MIC);
}

/**
 * @tc.name  : Test GetSourcePortName API
 * @tc.type  : FUNC
 * @tc.number: GetSourcePortName_VA_001
 * @tc.desc  : Test GetSourcePortName for VA spp / unidirectional / nearlink
 */
HWTEST(AudioPolicyUtilsUnitTest, GetSourcePortName_VA_001, TestSize.Level1)
{
    auto &utils = AudioPolicyUtils::GetInstance();
    EXPECT_EQ(utils.GetSourcePortName(DEVICE_TYPE_BT_SPP, AUDIO_FLAG_NONE, false), VA_INPUT_SPP);
    EXPECT_EQ(utils.GetSourcePortName(DEVICE_TYPE_BT_SPP, AUDIO_FLAG_NONE, true),
        VA_INPUT_UNIDIRECTIONAL_SPP);
    EXPECT_EQ(utils.GetSourcePortName(DEVICE_TYPE_NEARLINK_PORT, AUDIO_FLAG_NONE, false),
        VA_INPUT_NEARLINK);
}

/**
 * @tc.name  : Test AudioPolicyUtils
 * @tc.number: ClearScoDeviceSuspendState_001
 * @tc.desc  : Test ClearScoDeviceSuspendState
 */
HWTEST(AudioPolicyUtilsUnitTest, ClearScoDeviceSuspendState_001, TestSize.Level1)
{
    auto &devMan = AudioDeviceManager::GetAudioDeviceManager();
    string macAddress1 = "sdfs1";
    string macAddress2 = "sdfs2";
    AudioDeviceDescriptor desc;
    desc.deviceId_ = 114514;
    desc.deviceType_ = DEVICE_TYPE_NEARLINK;
    desc.macAddress_ = macAddress1;
    desc.networkId_ = LOCAL_NETWORK_ID;
    devMan.AddNewDevice(make_shared<AudioDeviceDescriptor>(desc));
    desc.deviceId_ = 114515;
    desc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc.macAddress_ = macAddress2;
    devMan.AddNewDevice(make_shared<AudioDeviceDescriptor>(desc));
    auto &utils = AudioPolicyUtils::GetInstance();
    utils.ClearScoDeviceSuspendState(macAddress1);
    utils.ClearScoDeviceSuspendState(macAddress2);
    EXPECT_EQ(devMan.ExistsByType(DEVICE_TYPE_NEARLINK), true);
}

/**
 * @tc.name  : Test CheckInterphone API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_CheckInterphone_001
 * @tc.desc  : Test CheckInterphone with USB ARM headset (should return false)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_CheckInterphone_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_USB_ARM_HEADSET, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = LOCAL_NETWORK_ID;

    bool result = AudioPolicyUtils::CheckInterphone(STREAM_USAGE_INTERPHONE, desc);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test CheckInterphone API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_CheckInterphone_002
 * @tc.desc  : Test CheckInterphone with earpiece (should return false)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_CheckInterphone_002, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_EARPIECE, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = LOCAL_NETWORK_ID;

    bool result = AudioPolicyUtils::CheckInterphone(STREAM_USAGE_INTERPHONE, desc);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test CheckInterphone API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_CheckInterphone_003
 * @tc.desc  : Test CheckInterphone with hearing aid (should return false)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_CheckInterphone_003, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_HEARING_AID, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = LOCAL_NETWORK_ID;

    bool result = AudioPolicyUtils::CheckInterphone(STREAM_USAGE_INTERPHONE, desc);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test CheckInterphone API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_CheckInterphone_004
 * @tc.desc  : Test CheckInterphone with Bluetooth SCO watch (should return false)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_CheckInterphone_004, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = LOCAL_NETWORK_ID;
    desc->deviceCategory_ = BT_WATCH;

    bool result = AudioPolicyUtils::CheckInterphone(STREAM_USAGE_INTERPHONE, desc);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test CheckInterphone API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_CheckInterphone_005
 * @tc.desc  : Test CheckInterphone with Bluetooth SCO hearing aid (should return false)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_CheckInterphone_005, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = LOCAL_NETWORK_ID;
    desc->deviceCategory_ = BT_HEARAID;

    bool result = AudioPolicyUtils::CheckInterphone(STREAM_USAGE_INTERPHONE, desc);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test CheckInterphone API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_CheckInterphone_006
 * @tc.desc  : Test CheckInterphone with Bluetooth SCO (should return true)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_CheckInterphone_006, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = LOCAL_NETWORK_ID;
    desc->deviceCategory_ = CATEGORY_DEFAULT;

    bool result = AudioPolicyUtils::CheckInterphone(STREAM_USAGE_INTERPHONE, desc);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test CheckInterphone API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_CheckInterphone_007
 * @tc.desc  : Test CheckInterphone with Nearlink (should return true)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_CheckInterphone_007, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_NEARLINK, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = LOCAL_NETWORK_ID;
    desc->deviceCategory_ = CATEGORY_DEFAULT;

    bool result = AudioPolicyUtils::CheckInterphone(STREAM_USAGE_INTERPHONE, desc);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test CheckInterphone API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_CheckInterphone_008
 * @tc.desc  : Test CheckInterphone with local speaker (should return true)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_CheckInterphone_008, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = LOCAL_NETWORK_ID;

    bool result = AudioPolicyUtils::CheckInterphone(STREAM_USAGE_INTERPHONE, desc);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test CheckInterphone API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_CheckInterphone_009
 * @tc.desc  : Test CheckInterphone with remote speaker (should return false)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_CheckInterphone_009, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = "remote_device_id";

    bool result = AudioPolicyUtils::CheckInterphone(STREAM_USAGE_INTERPHONE, desc);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test CheckInterphone API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_CheckInterphone_010
 * @tc.desc  : Test CheckInterphone with non-INTERPHONE usage (should return true)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_CheckInterphone_010, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_USB_ARM_HEADSET, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = LOCAL_NETWORK_ID;

    bool result = AudioPolicyUtils::CheckInterphone(STREAM_USAGE_VOICE_COMMUNICATION, desc);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test CheckInterphone API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_CheckInterphone_011
 * @tc.desc  : Test CheckInterphone with nullptr descriptor (should return true)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_CheckInterphone_011, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = nullptr;

    bool result = AudioPolicyUtils::CheckInterphone(STREAM_USAGE_INTERPHONE, desc);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test CheckInterphone API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_CheckInterphone_012
 * @tc.desc  : Test CheckInterphone with paired device and local network (should return true)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_CheckInterphone_012, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = LOCAL_NETWORK_ID;
    desc->pairDeviceDescriptor_ = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN, DeviceRole::INPUT_DEVICE);

    bool result = AudioPolicyUtils::CheckInterphone(STREAM_USAGE_INTERPHONE, desc);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test CheckInterphone API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_CheckInterphone_013
 * @tc.desc  : Test CheckInterphone with paired device and remote network (should return false)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_CheckInterphone_013, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = "remote_device_id";
    desc->pairDeviceDescriptor_ = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN, DeviceRole::INPUT_DEVICE);

    bool result = AudioPolicyUtils::CheckInterphone(STREAM_USAGE_INTERPHONE, desc);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test CheckInterphone API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_CheckInterphone_014
 * @tc.desc  : Test CheckInterphone without paired device (should return false)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_CheckInterphone_014, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>(
        DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    desc->networkId_ = LOCAL_NETWORK_ID;
    desc->pairDeviceDescriptor_ = nullptr;

    bool result = AudioPolicyUtils::CheckInterphone(STREAM_USAGE_INTERPHONE, desc);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test FilterDevicesByTokenId API (NEW BRANCH coverage)
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_FilterDevicesByTokenId_001
 * @tc.desc  : Test FilterDevicesByTokenId filters devices with different tokenId
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_FilterDevicesByTokenId_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;

    // Device with tokenId = 0 - NOT filtered
    auto device1 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    device1->deviceTokenId_ = 0;
    devices.push_back(device1);

    // Device with different non-zero tokenId - FILTERED
    auto device2 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    device2->deviceTokenId_ = 12345;
    devices.push_back(device2);

    // nullptr device - NOT filtered
    devices.push_back(nullptr);

    AudioPolicyUtils::GetInstance().FilterDevicesByTokenId(devices);

    EXPECT_EQ(devices.size(), 2);
}

/**
 * @tc.name  : Test FilterDevicesByTokenId API (NEW BRANCH coverage)
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_FilterDevicesByTokenId_002
 * @tc.desc  : Test FilterDevicesByTokenId keeps all devices when all have tokenId = 0
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_FilterDevicesByTokenId_002, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;

    auto device1 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    device1->deviceTokenId_ = 0;
    devices.push_back(device1);

    auto device2 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    device2->deviceTokenId_ = 0;
    devices.push_back(device2);

    AudioPolicyUtils::GetInstance().FilterDevicesByTokenId(devices);

    EXPECT_EQ(devices.size(), 2);
}

/**
 * @tc.name  : Test FilterDevicesByTokenId API (NEW BRANCH coverage)
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_FilterDevicesByTokenId_003
 * @tc.desc  : Test FilterDevicesByTokenId filters all devices with non-zero tokenId (all branches filter out).
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_FilterDevicesByTokenId_003, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;

    auto device1 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    device1->deviceTokenId_ = 1001;
    devices.push_back(device1);

    auto device2 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_SCO, OUTPUT_DEVICE);
    device2->deviceTokenId_ = 1002;
    devices.push_back(device2);

    auto device3 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP_IN, INPUT_DEVICE);
    device3->deviceTokenId_ = 1003;
    devices.push_back(device3);

    AudioPolicyUtils::GetInstance().FilterDevicesByTokenId(devices);

    EXPECT_EQ(devices.size(), 0);
}

/**
 * @tc.name  : Test UpdateTimestampWhenBecomeEnable API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_UpdateTimestampWhenBecomeEnable_001
 * @tc.desc  : Test UpdateTimestampWhenBecomeEnable
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_UpdateTimestampWhenBecomeEnable_001, TestSize.Level1)
{
    auto backup = AudioRouterInfra::GetInstance().GetCurrentOutputDevice(SYSTEM_UID);
    AudioPolicyUtils::GetInstance().audioDeviceManager_.ParseDeviceXml();
    auto current = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    current->deviceCategory_ = BT_SOUNDBOX;
    current->deviceUsage_ = ALL_USAGE;
    current->macAddress_ = "11:11:11:11:11:11";
    current->deviceId_ = 2;

    // current is public device
    AudioRouterInfra::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID,
        {std::make_shared<AudioDeviceSimpleDescriptor>(current->deviceType_,
            current->deviceRole_,
            current->deviceId_,
            current->networkId_,
            current->macAddress_)});

    auto updated = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    updated->deviceUsage_ = ALL_USAGE;
    updated->macAddress_ = "22:22:22:22:22:22";
    updated->deviceId_ = 3;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices {current, updated};

    // glasses become enable
    updated->deviceCategory_ = BT_GLASSES;
    EXPECT_FALSE(AudioPolicyUtils::GetInstance().UpdateTimestampWhenBecomeEnable(devices, updated));

    // soundbox become enable
    updated->deviceCategory_ = BT_SOUNDBOX;
    EXPECT_FALSE(AudioPolicyUtils::GetInstance().UpdateTimestampWhenBecomeEnable(devices, updated));

    // headphone become enable
    updated->deviceCategory_ = BT_HEADPHONE;
    EXPECT_FALSE(AudioPolicyUtils::GetInstance().UpdateTimestampWhenBecomeEnable(devices, updated));

    // current is privacy device
    current->deviceCategory_ = BT_HEADPHONE;
    AudioRouterInfra::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID,
        {std::make_shared<AudioDeviceSimpleDescriptor>(current->deviceType_,
            current->deviceRole_,
            current->deviceId_,
            current->networkId_,
            current->macAddress_)});
    EXPECT_TRUE(AudioPolicyUtils::GetInstance().UpdateTimestampWhenBecomeEnable(devices, updated));

    AudioRouterInfra::GetInstance().UpdateCurrentOutputDevice(SYSTEM_UID, backup);
}

/**
 * @tc.name  : Test UpdateTimestampAfterUnwearGlasses API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_UpdateTimestampAfterUnwearGlasses_001
 * @tc.desc  : Test UpdateTimestampAfterUnwearGlasses
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_UpdateTimestampAfterUnwearGlasses_001, TestSize.Level1)
{
    AudioPolicyUtils::GetInstance().audioDeviceManager_.ParseDeviceXml();
    auto updated = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    updated->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    updated->deviceUsage_ = ALL_USAGE;
    updated->macAddress_ = "11:11:11:11:11:11";
    updated->isPriorityReduced_ = false;
    updated->connectTimeStamp_ = 500;
    updated->deviceId_ = 3;

    auto desc1 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    desc1->deviceCategory_ = BT_HEADPHONE;
    desc1->deviceUsage_ = ALL_USAGE;
    desc1->macAddress_ = "22:22:22:22:22:22";
    desc1->isPriorityReduced_ = true;
    desc1->connectTimeStamp_ = 0;
    desc1->deviceId_ = 4;

    auto desc2 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_NEARLINK, OUTPUT_DEVICE);
    desc2->deviceCategory_ = BT_HEADPHONE;
    desc2->deviceUsage_ = ALL_USAGE;
    desc2->macAddress_ = "33:33:33:33:33:33";
    desc2->isPriorityReduced_ = false;
    desc2->connectTimeStamp_ = 1000;
    desc2->deviceId_ = 5;

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices {updated, desc1, desc2};
    EXPECT_TRUE(desc1->isPriorityReduced_);
    EXPECT_TRUE(desc1->connectTimeStamp_ < updated->connectTimeStamp_);
    AudioPolicyUtils::GetInstance().UpdateTimestampAfterUnwearGlasses(devices, updated);
    EXPECT_FALSE(desc1->isPriorityReduced_);
    EXPECT_FALSE(desc1->connectTimeStamp_ < updated->connectTimeStamp_);
}

/**
 * @tc.name  : Test FilterSystemDevices API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_FilterSystemDevices_001
 * @tc.desc  : Test FilterSystemDevices with empty input (no crash, stays empty)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_FilterSystemDevices_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    AudioPolicyUtils::GetInstance().FilterSystemDevices(devices);
    EXPECT_EQ(devices.size(), 0);
}

/**
 * @tc.name  : Test FilterSystemDevices API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_FilterSystemDevices_002
 * @tc.desc  : Test FilterSystemDevices keeps all non-system devices
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_FilterSystemDevices_002, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto device1 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    auto device2 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    devices.push_back(device1);
    devices.push_back(device2);

    AudioPolicyUtils::GetInstance().FilterSystemDevices(devices);

    EXPECT_EQ(devices.size(), 2);
    EXPECT_FALSE(devices[0]->IsSystemVisibility());
    EXPECT_FALSE(devices[1]->IsSystemVisibility());
}

/**
 * @tc.name  : Test FilterSystemDevices API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_FilterSystemDevices_003
 * @tc.desc  : Test FilterSystemDevices removes all system devices (result empty)
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_FilterSystemDevices_003, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto device1 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    device1->deviceVisibility_ = SYSTEM_VISIBILITY;
    auto device2 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    device2->deviceVisibility_ = SYSTEM_VISIBILITY;
    devices.push_back(device1);
    devices.push_back(device2);

    AudioPolicyUtils::GetInstance().FilterSystemDevices(devices);

    EXPECT_EQ(devices.size(), 0);
}

/**
 * @tc.name  : Test FilterSystemDevices API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_FilterSystemDevices_004
 * @tc.desc  : Test FilterSystemDevices keeps only non-system devices from mixed list, order preserved
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_FilterSystemDevices_004, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto systemDev1 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_USB_HEADSET, OUTPUT_DEVICE);
    systemDev1->deviceVisibility_ = SYSTEM_VISIBILITY;
    auto normalDev1 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    auto systemDev2 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_WIRED_HEADSET, OUTPUT_DEVICE);
    systemDev2->deviceVisibility_ = SYSTEM_VISIBILITY;
    auto normalDev2 = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_BLUETOOTH_A2DP, OUTPUT_DEVICE);
    devices.push_back(systemDev1);
    devices.push_back(normalDev1);
    devices.push_back(systemDev2);
    devices.push_back(normalDev2);

    AudioPolicyUtils::GetInstance().FilterSystemDevices(devices);

    EXPECT_EQ(devices.size(), 2);
    EXPECT_EQ(devices[0]->deviceType_, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(devices[1]->deviceType_, DEVICE_TYPE_BLUETOOTH_A2DP);
}

/**
 * @tc.name  : Test FilterSystemDevices API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_FilterSystemDevices_005
 * @tc.desc  : Test FilterSystemDevices keeps nullptr entries and non-system, removes only system devices
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_FilterSystemDevices_005, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto systemDev = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_USB_HEADSET, OUTPUT_DEVICE);
    systemDev->deviceVisibility_ = SYSTEM_VISIBILITY;
    auto normalDev = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    devices.push_back(systemDev);
    devices.push_back(nullptr);
    devices.push_back(normalDev);

    AudioPolicyUtils::GetInstance().FilterSystemDevices(devices);

    EXPECT_EQ(devices.size(), 2);
    EXPECT_EQ(devices[0], nullptr);
    EXPECT_NE(devices[1], nullptr);
    EXPECT_EQ(devices[1]->deviceType_, DEVICE_TYPE_SPEAKER);
}

/**
 * @tc.name  : Test FilterSystemDevices API
 * @tc.type  : FUNC
 * @tc.number: AudioPolicyUtilsUnitTest_FilterSystemDevices_006
 * @tc.desc  : Test FilterSystemDevices does not modify original non-system device flags
 */
HWTEST(AudioPolicyUtilsUnitTest, AudioPolicyUtilsUnitTest_FilterSystemDevices_006, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto normalDev = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    auto systemDev = std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_MIC, INPUT_DEVICE);
    systemDev->deviceVisibility_ = SYSTEM_VISIBILITY;
    devices.push_back(normalDev);
    devices.push_back(systemDev);

    AudioPolicyUtils::GetInstance().FilterSystemDevices(devices);

    EXPECT_EQ(devices.size(), 1);
    EXPECT_FALSE(devices[0]->IsSystemVisibility());
    EXPECT_EQ(devices[0]->deviceType_, DEVICE_TYPE_SPEAKER);
}
} // namespace AudioStandard
} // namespace OHOS
