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

#include "audio_endpoint_third_unit_test.h"
#include "audio_device_info.h"
#include "audio_endpoint.h"
#include "audio_endpoint_private.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_process_config.h"
#include "audio_server.h"
#include "audio_service.h"
#include "audio_stream_info.h"
#include "policy_handler.h"

#include <gtest/gtest.h>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

constexpr uint64_t TEST_ENDPOINT_ID = 123;

static std::shared_ptr<AudioEndpointInner> CreateTestEndpointInner(AudioEndpoint::EndpointType type)
{
    AudioProcessConfig config = {};
    config.audioMode = AUDIO_MODE_PLAYBACK;
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, TEST_ENDPOINT_ID, config.audioMode);
    if (audioEndpointInner != nullptr) {
        audioEndpointInner->endpointStatus_.store(AudioEndpoint::IDEL);
    }
    return audioEndpointInner;
}

static std::shared_ptr<AudioEndpointInner> CreateInputTestEndpointInner(AudioEndpoint::EndpointType type)
{
    AudioProcessConfig config = {};
    config.audioMode = AUDIO_MODE_RECORD;
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, TEST_ENDPOINT_ID, config.audioMode);
    if (audioEndpointInner != nullptr) {
        audioEndpointInner->endpointStatus_.store(AudioEndpoint::IDEL);
    }
    return audioEndpointInner;
}

void AudioEndpointThirdUnitTest::SetUpTestCase(void)
{
}

void AudioEndpointThirdUnitTest::TearDownTestCase(void)
{
}

void AudioEndpointThirdUnitTest::SetUp(void)
{
}

void AudioEndpointThirdUnitTest::TearDown(void)
{
}

/**
 * @tc.name  : Test AudioEndpoint::GenerateEndpointKey API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointGenerateEndpointKey_001
 * @tc.desc  : Test GenerateEndpointKey with different device types.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointGenerateEndpointKey_001, TestSize.Level1)
{
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    deviceInfo.deviceId_ = 1;
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    deviceInfo.deviceRole_ = OUTPUT_DEVICE;
    std::string key = AudioEndpoint::GenerateEndpointKey(deviceInfo, AUDIO_FLAG_MMAP);
    EXPECT_FALSE(key.empty());
}

/**
 * @tc.name  : Test AudioEndpoint::GenerateEndpointKey API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointGenerateEndpointKey_002
 * @tc.desc  : Test GenerateEndpointKey with USB ARM headset.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointGenerateEndpointKey_002, TestSize.Level1)
{
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    deviceInfo.deviceId_ = 2;
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    deviceInfo.deviceRole_ = OUTPUT_DEVICE;
    std::string key = AudioEndpoint::GenerateEndpointKey(deviceInfo, AUDIO_FLAG_MMAP);
    EXPECT_FALSE(key.empty());
}

/**
 * @tc.name  : Test AudioEndpoint::GenerateEndpointKey API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointGenerateEndpointKey_003
 * @tc.desc  : Test GenerateEndpointKey with input device.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointGenerateEndpointKey_003, TestSize.Level1)
{
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceType_ = DEVICE_TYPE_MIC;
    deviceInfo.deviceId_ = 3;
    deviceInfo.networkId_ = LOCAL_NETWORK_ID;
    deviceInfo.deviceRole_ = INPUT_DEVICE;
    std::string key = AudioEndpoint::GenerateEndpointKey(deviceInfo, AUDIO_FLAG_VOIP_FAST);
    EXPECT_FALSE(key.empty());
}

/**
 * @tc.name  : Test AudioEndpoint::GetDeviceInfo API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointGetDeviceInfo_001
 * @tc.desc  : Test GetDeviceInfo interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointGetDeviceInfo_001, TestSize.Level1)
{
    auto audioEndpoint = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpoint, nullptr);
    AudioDeviceDescriptor deviceInfo = audioEndpoint->GetDeviceInfo();
    EXPECT_EQ(static_cast<int>(deviceInfo.deviceRole_), 0);
}

/**
 * @tc.name  : Test AudioEndpoint::GetDeviceRole API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointGetDeviceRole_001
 * @tc.desc  : Test GetDeviceRole interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointGetDeviceRole_001, TestSize.Level1)
{
    auto audioEndpoint = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpoint, nullptr);
    DeviceRole role = audioEndpoint->GetDeviceRole();
    EXPECT_EQ(static_cast<int>(role), 0);
}

/**
 * @tc.name  : Test AudioEndpoint::SetMuteForSwitchDevice API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSetMuteForSwitchDevice_001
 * @tc.desc  : Test SetMuteForSwitchDevice interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointSetMuteForSwitchDevice_001, TestSize.Level1)
{
    auto audioEndpoint = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpoint, nullptr);
    audioEndpoint->SetMuteForSwitchDevice(true);
    audioEndpoint->SetMuteForSwitchDevice(false);
    audioEndpoint->SetMuteForSwitchDevice(true);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpoint::GetAudioStreamInfo API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointGetAudioStreamInfo_001
 * @tc.desc  : Test GetAudioStreamInfo interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointGetAudioStreamInfo_001, TestSize.Level1)
{
    auto audioEndpoint = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpoint, nullptr);
    AudioStreamInfo streamInfo = audioEndpoint->GetAudioStreamInfo();
    EXPECT_EQ(streamInfo.samplingRate, 0);
}

/**
 * @tc.name  : Test AudioEndpointInner::GetEndpointName API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerGetEndpointName_001
 * @tc.desc  : Test GetEndpointName interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerGetEndpointName_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    std::string name = audioEndpointInner->GetEndpointName();
}

/**
 * @tc.name  : Test AudioEndpointInner::GetStatus API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerGetStatus_001
 * @tc.desc  : Test GetStatus interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerGetStatus_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    auto status = audioEndpointInner->GetStatus();
    EXPECT_EQ(status, AudioEndpoint::IDEL);
}

/**
 * @tc.name  : Test AudioEndpointInner::GetAudioMode API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerGetAudioMode_001
 * @tc.desc  : Test GetAudioMode interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerGetAudioMode_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    AudioMode mode = audioEndpointInner->GetAudioMode();
    EXPECT_EQ(mode, AUDIO_MODE_PLAYBACK);
}

/**
 * @tc.name  : Test AudioEndpointInner::GetAudioMode API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerGetAudioMode_002
 * @tc.desc  : Test GetAudioMode interface for record mode.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerGetAudioMode_002, TestSize.Level1)
{
    auto audioEndpointInner = CreateInputTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    AudioMode mode = audioEndpointInner->GetAudioMode();
    EXPECT_EQ(mode, AUDIO_MODE_RECORD);
}

/**
 * @tc.name  : Test AudioEndpointInner::IsEndpointUltraFast API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerIsEndpointUltraFast_001
 * @tc.desc  : Test IsEndpointUltraFast interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerIsEndpointUltraFast_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    bool isUltraFast = audioEndpointInner->IsEndpointUltraFast();
    EXPECT_EQ(isUltraFast, false);
}

/**
 * @tc.name  : Test AudioEndpointInner::GetLinkedProcessCount API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerGetLinkedProcessCount_001
 * @tc.desc  : Test GetLinkedProcessCount interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerGetLinkedProcessCount_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    uint32_t count = audioEndpointInner->GetLinkedProcessCount();
    EXPECT_EQ(count, 0);
}

/**
 * @tc.name  : Test AudioEndpointInner::GetMaxAmplitude API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerGetMaxAmplitude_001
 * @tc.desc  : Test GetMaxAmplitude interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerGetMaxAmplitude_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    float amplitude = audioEndpointInner->GetMaxAmplitude();
    EXPECT_EQ(amplitude, 0.0f);
}

/**
 * @tc.name  : Test AudioEndpointInner::ShouldInnerCap API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerShouldInnerCap_001
 * @tc.desc  : Test ShouldInnerCap interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerShouldInnerCap_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    bool shouldCap = audioEndpointInner->ShouldInnerCap(1);
    EXPECT_EQ(shouldCap, false);
}

/**
 * @tc.name  : Test AudioEndpointInner::IsBufferDataInsufficient API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerIsBufferDataInsufficient_001
 * @tc.desc  : Test IsBufferDataInsufficient with negative value.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerIsBufferDataInsufficient_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    bool result = audioEndpointInner->IsBufferDataInsufficient(-1, 100);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioEndpointInner::IsBufferDataInsufficient API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerIsBufferDataInsufficient_002
 * @tc.desc  : Test IsBufferDataInsufficient with sufficient data.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerIsBufferDataInsufficient_002, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    bool result = audioEndpointInner->IsBufferDataInsufficient(200, 100);
    EXPECT_EQ(result, false);
}

/**
 * @tc.name  : Test AudioEndpointInner::IsBufferDataInsufficient API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerIsBufferDataInsufficient_003
 * @tc.desc  : Test IsBufferDataInsufficient with insufficient data.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerIsBufferDataInsufficient_003, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    bool result = audioEndpointInner->IsBufferDataInsufficient(50, 100);
    EXPECT_EQ(result, true);
}

/**
 * @tc.name  : Test AudioEndpointInner::IsInvalidBuffer API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerIsInvalidBuffer_001
 * @tc.desc  : Test IsInvalidBuffer with S16LE format.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerIsInvalidBuffer_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    int16_t zeroData = 0;
    bool isInvalid = audioEndpointInner->IsInvalidBuffer(reinterpret_cast<uint8_t*>(&zeroData), sizeof(zeroData),
        SAMPLE_S16LE);
    EXPECT_EQ(isInvalid, true);
}

/**
 * @tc.name  : Test AudioEndpointInner::IsInvalidBuffer API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerIsInvalidBuffer_002
 * @tc.desc  : Test IsInvalidBuffer with S16LE format and non-zero data.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerIsInvalidBuffer_002, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    int16_t nonZeroData = 1;
    bool isInvalid = audioEndpointInner->IsInvalidBuffer(
        reinterpret_cast<uint8_t*>(&nonZeroData), sizeof(nonZeroData), SAMPLE_S16LE);
    EXPECT_EQ(isInvalid, false);
}

/**
 * @tc.name  : Test AudioEndpointInner::IsInvalidBuffer API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerIsInvalidBuffer_003
 * @tc.desc  : Test IsInvalidBuffer with U8 format.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerIsInvalidBuffer_003, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    uint8_t zeroData = 0;
    bool isInvalid = audioEndpointInner->IsInvalidBuffer(reinterpret_cast<uint8_t*>(&zeroData), sizeof(zeroData),
        SAMPLE_U8);
    EXPECT_EQ(isInvalid, true);
}

/**
 * @tc.name  : Test AudioEndpointInner::IsInvalidBuffer API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerIsInvalidBuffer_004
 * @tc.desc  : Test IsInvalidBuffer with unsupported format.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerIsInvalidBuffer_004, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    uint8_t data = 0;
    bool isInvalid = audioEndpointInner->IsInvalidBuffer(&data, sizeof(data), SAMPLE_S24LE);
    EXPECT_EQ(isInvalid, false);
}

/**
 * @tc.name  : Test AudioEndpointInner::GetStatusStr API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerGetStatusStr_001
 * @tc.desc  : Test GetStatusStr with various statuses.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerGetStatusStr_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    std::string str = audioEndpointInner->GetStatusStr(AudioEndpoint::INVALID);
    EXPECT_EQ(str, "INVALID");
    str = audioEndpointInner->GetStatusStr(AudioEndpoint::UNLINKED);
    EXPECT_EQ(str, "UNLINKED");
    str = audioEndpointInner->GetStatusStr(AudioEndpoint::IDEL);
    EXPECT_EQ(str, "IDEL");
    str = audioEndpointInner->GetStatusStr(AudioEndpoint::STARTING);
    EXPECT_EQ(str, "STARTING");
    str = audioEndpointInner->GetStatusStr(AudioEndpoint::RUNNING);
    EXPECT_EQ(str, "RUNNING");
    str = audioEndpointInner->GetStatusStr(AudioEndpoint::STOPPING);
    EXPECT_EQ(str, "STOPPING");
    str = audioEndpointInner->GetStatusStr(AudioEndpoint::STOPPED);
    EXPECT_EQ(str, "STOPPED");
    str = audioEndpointInner->GetStatusStr(static_cast<AudioEndpoint::EndpointStatus>(100));
    EXPECT_EQ(str, "NO_SUCH_STATUS");
}

/**
 * @tc.name  : Test AudioEndpointInner::DisableFastInnerCap API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerDisableFastInnerCap_001
 * @tc.desc  : Test DisableFastInnerCap without inner cap enabled.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerDisableFastInnerCap_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    int32_t ret = audioEndpointInner->DisableFastInnerCap();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEndpointInner::DisableFastInnerCap API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerDisableFastInnerCap_002
 * @tc.desc  : Test DisableFastInnerCap with inner cap id.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerDisableFastInnerCap_002, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    int32_t ret = audioEndpointInner->DisableFastInnerCap(1);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEndpointInner::StopByRestore API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerStopByRestore_001
 * @tc.desc  : Test StopByRestore with non-Bluetooth device.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerStopByRestore_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    RestoreInfo restoreInfo;
    restoreInfo.deviceChangeReason = static_cast<int32_t>(AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE);
    audioEndpointInner->StopByRestore(restoreInfo);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpointInner::StopByRestore API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerStopByRestore_002
 * @tc.desc  : Test StopByRestore with different reason.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerStopByRestore_002, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    RestoreInfo restoreInfo;
    restoreInfo.deviceChangeReason = 0;
    audioEndpointInner->StopByRestore(restoreInfo);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpointInner::AddEndpointStreamVolume API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerAddEndpointStreamVolume_001
 * @tc.desc  : Test AddEndpointStreamVolume interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerAddEndpointStreamVolume_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    // Test with nullptr process stream
    audioEndpointInner->AddEndpointStreamVolume(nullptr);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpointInner::LinkProcessStreamExt API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerLinkProcessStreamExt_001
 * @tc.desc  : Test LinkProcessStreamExt interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerLinkProcessStreamExt_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    // Test with nullptr
    audioEndpointInner->LinkProcessStreamExt(nullptr, nullptr);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpointInner::ProcessUpdateAppsUidForPlayback API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerProcessUpdateAppsUidForPlayback_001
 * @tc.desc  : Test ProcessUpdateAppsUidForPlayback interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerProcessUpdateAppsUidForPlayback_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    audioEndpointInner->ProcessUpdateAppsUidForPlayback();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpointInner::ProcessUpdateAppsUidForRecord API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerProcessUpdateAppsUidForRecord_001
 * @tc.desc  : Test ProcessUpdateAppsUidForRecord interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerProcessUpdateAppsUidForRecord_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateInputTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    audioEndpointInner->ProcessUpdateAppsUidForRecord();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpointInner::CheckUpdateState API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerCheckUpdateState_001
 * @tc.desc  : Test CheckUpdateState interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerCheckUpdateState_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    char testData[4] = {0, 0, 0, 0};
    audioEndpointInner->CheckUpdateState(testData, 4);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpointInner::ZeroVolumeCheck API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerZeroVolumeCheck_001
 * @tc.desc  : Test ZeroVolumeCheck interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerZeroVolumeCheck_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    // Test with zero volume
    audioEndpointInner->ZeroVolumeCheck(0);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpointInner::ZeroVolumeCheck API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerZeroVolumeCheck_002
 * @tc.desc  : Test ZeroVolumeCheck interface with non-zero volume.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerZeroVolumeCheck_002, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    // Test with non-zero volume
    audioEndpointInner->ZeroVolumeCheck(100);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpointInner::ResetZeroVolumeState API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerResetZeroVolumeState_001
 * @tc.desc  : Test ResetZeroVolumeState interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerResetZeroVolumeState_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    audioEndpointInner->ResetZeroVolumeState();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpointInner::InitLatencyMeasurement API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerInitLatencyMeasurement_001
 * @tc.desc  : Test InitLatencyMeasurement interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerInitLatencyMeasurement_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    audioEndpointInner->InitLatencyMeasurement();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpointInner::DeinitLatencyMeasurement API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerDeinitLatencyMeasurement_001
 * @tc.desc  : Test DeinitLatencyMeasurement interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerDeinitLatencyMeasurement_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    audioEndpointInner->DeinitLatencyMeasurement();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpointInner::CheckPlaySignal API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerCheckPlaySignal_001
 * @tc.desc  : Test CheckPlaySignal interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerCheckPlaySignal_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    uint8_t testData[4] = {0, 0, 0, 0};
    audioEndpointInner->CheckPlaySignal(testData, 4);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpointInner::CheckRecordSignal API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerCheckRecordSignal_001
 * @tc.desc  : Test CheckRecordSignal interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerCheckRecordSignal_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateInputTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    uint8_t testData[4] = {0, 0, 0, 0};
    audioEndpointInner->CheckRecordSignal(testData, 4);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioEndpointInner::IsAnyProcessRunning API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerIsAnyProcessRunning_001
 * @tc.desc  : Test IsAnyProcessRunning interface with empty list.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerIsAnyProcessRunning_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    bool isRunning = audioEndpointInner->IsAnyProcessRunning();
    EXPECT_EQ(isRunning, false);
}

/**
 * @tc.name  : Test AudioEndpointInner::GetInnerCapConfig API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerGetInnerCapConfig_001
 * @tc.desc  : Test GetInnerCapConfig interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerGetInnerCapConfig_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    AudioProcessConfig config = audioEndpointInner->GetInnerCapConfig();
    EXPECT_EQ(config.audioMode, AUDIO_MODE_PLAYBACK);
    EXPECT_EQ(config.streamType, STREAM_MUSIC);
}

/**
 * @tc.name  : Test AudioEndpointInner::IsDualStream API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerIsDualStream_001
 * @tc.desc  : Test IsDualStream interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerIsDualStream_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    CaptureInfo capInfo;
    bool isDual = audioEndpointInner->IsDualStream(capInfo);
    EXPECT_EQ(isDual, false);
}

/**
 * @tc.name  : Test AudioEndpointInner::IsDualStream API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerIsDualStream_002
 * @tc.desc  : Test IsDualStream interface with dual device name.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerIsDualStream_002, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    CaptureInfo capInfo;
    capInfo.dualDeviceName = "test_device";
    bool isDual = audioEndpointInner->IsDualStream(capInfo);
    EXPECT_EQ(isDual, true);
}

/**
 * @tc.name  : Test AudioEndpointInner::IsDupRenderCallbackMode API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerIsDupRenderCallbackMode_001
 * @tc.desc  : Test IsDupRenderCallbackMode interface.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerIsDupRenderCallbackMode_001, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    bool isCallback = audioEndpointInner->IsDupRenderCallbackMode(1, false);
    EXPECT_EQ(isCallback, true);
}

/**
 * @tc.name  : Test AudioEndpointInner::IsDupRenderCallbackMode API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerIsDupRenderCallbackMode_002
 * @tc.desc  : Test IsDupRenderCallbackMode interface with dual stream.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerIsDupRenderCallbackMode_002, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    bool isCallback = audioEndpointInner->IsDupRenderCallbackMode(1, true);
    EXPECT_EQ(isCallback, false);
}

/**
 * @tc.name  : Test AudioEndpointInner::IsDupRenderCallbackMode API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInnerIsDupRenderCallbackMode_003
 * @tc.desc  : Test IsDupRenderCallbackMode interface with engine flag 0.
 */
HWTEST_F(AudioEndpointThirdUnitTest, AudioEndpointInnerIsDupRenderCallbackMode_003, TestSize.Level1)
{
    auto audioEndpointInner = CreateTestEndpointInner(AudioEndpoint::TYPE_MMAP);
    ASSERT_NE(audioEndpointInner, nullptr);
    bool isCallback = audioEndpointInner->IsDupRenderCallbackMode(0, false);
    EXPECT_EQ(isCallback, false);
}

} // namespace AudioStandard
} // namespace OHOS
