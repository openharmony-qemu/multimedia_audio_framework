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
#include "get_server_util.h"
#include "audio_policy_service_third_unit_test.h"
#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
static const std::string PIPE_PRIMARY_OUTPUT_UNITTEST = "primary_output";
static const std::string PIPE_PRIMARY_INPUT_UNITTEST = "primary_input";
static const std::string PIPE_USB_ARM_OUTPUT_UNITTEST = "usb_arm_output";
static const std::string PIPE_DP_OUTPUT_UNITTEST = "dp_output";
static const std::string PIPE_USB_ARM_INPUT_UNITTEST = "usb_arm_input";

void AudioPolicyServiceThirdUnitTest::SetUpTestCase(void) {}
void AudioPolicyServiceThirdUnitTest::TearDownTestCase(void) {}
void AudioPolicyServiceThirdUnitTest::SetUp(void) {}
void AudioPolicyServiceThirdUnitTest::TearDown(void) {}

static AudioPolicyServer* GetServerPtr()
{
    return GetServerUtil::GetServerPtr();
}

/**
* @tc.name  : Test DeviceUpdateClearRecongnitionStatus.
* @tc.number: DeviceUpdateClearRecongnitionStatus_001
* @tc.desc  : Test DeviceUpdateClearRecongnitionStatus.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, DeviceUpdateClearRecongnitionStatus_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();
    // dummy data
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    desc->isEnable_ = true;

    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.DeviceUpdateClearRecongnitionStatus(*desc);
}

/**
* @tc.name  : Test DeviceUpdateClearRecongnitionStatus.
* @tc.number: DeviceUpdateClearRecongnitionStatus_002
* @tc.desc  : Test DeviceUpdateClearRecongnitionStatus.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, DeviceUpdateClearRecongnitionStatus_002, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();
    // dummy data
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;

    desc->connectState_ = DEACTIVE_CONNECTED;
    desc->deviceCategory_ = BT_HEARAID;
    desc->isEnable_ = true;

    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.DeviceUpdateClearRecongnitionStatus(*desc);
}

/**
* @tc.name  : Test DeviceUpdateClearRecongnitionStatus.
* @tc.number: DeviceUpdateClearRecongnitionStatus_003
* @tc.desc  : Test DeviceUpdateClearRecongnitionStatus.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, DeviceUpdateClearRecongnitionStatus_003, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();
    // dummy data
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;

    desc->connectState_ = VIRTUAL_CONNECTED;
    desc->deviceCategory_ = BT_HEARAID;
    desc->isEnable_ = false;

    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.DeviceUpdateClearRecongnitionStatus(*desc);
}

/**
* @tc.name  : Test DeviceUpdateClearRecongnitionStatus.
* @tc.number: DeviceUpdateClearRecongnitionStatus_004
* @tc.desc  : Test DeviceUpdateClearRecongnitionStatus.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, DeviceUpdateClearRecongnitionStatus_004, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();
    // dummy data
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    desc->connectState_ = VIRTUAL_CONNECTED;
    desc->deviceCategory_ = BT_HEARAID;
    desc->isEnable_ = false;

    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.DeviceUpdateClearRecongnitionStatus(*desc);
}

/**
* @tc.name  : Test DeviceUpdateClearRecongnitionStatus.
* @tc.number: DeviceUpdateClearRecongnitionStatus_005
* @tc.desc  : Test DeviceUpdateClearRecongnitionStatus.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, DeviceUpdateClearRecongnitionStatus_005, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();
    // dummy data
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    desc->connectState_ = VIRTUAL_CONNECTED;
    desc->deviceCategory_ = BT_HEARAID;
    desc->isEnable_ = true;

    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.DeviceUpdateClearRecongnitionStatus(*desc);
}

/**
 * @tc.name  : Test AudioPolicyService.
 * @tc.number: GetOffloadStatusDump_001
 * @tc.desc  : Test GetOffloadStatusDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetOffloadStatusDump_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::string dumpString = "666";
    server->audioPolicyDump_.GetOffloadStatusDump(dumpString);
    EXPECT_NE(dumpString, "666");
}

/**
 * @tc.name  : Test AudioPolicyService.
 * @tc.number: GetOffloadStatusDump_002
 * @tc.desc  : Test GetOffloadStatusDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetOffloadStatusDump_002, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::string dumpString = "666";
    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    server->audioPolicyDump_.GetOffloadStatusDump(dumpString);
    EXPECT_NE(dumpString, "666");
}

/**
 * @tc.name  : Test AudioPolicyService.
 * @tc.number: GetOffloadStatusDump_003
 * @tc.desc  : Test GetOffloadStatusDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetOffloadStatusDump_003, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::string dumpString = "666";
    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    server->audioPolicyDump_.GetOffloadStatusDump(dumpString);
    EXPECT_NE(dumpString, "666");
}

/**
 * @tc.name  : Test AudioPolicyService.
 * @tc.number: GetOffloadStatusDump_004
 * @tc.desc  : Test GetOffloadStatusDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetOffloadStatusDump_004, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);
    std::string dumpString = "666";

    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    server->audioPolicyDump_.GetOffloadStatusDump(dumpString);
    EXPECT_NE(dumpString, "666");
}
#ifdef AUDIO_POLICY_SERVICE_UNIT_TEST_DIFF
/**
* @tc.name  : Test HandleRemoteCastDevice.
* @tc.number: HandleRemoteCastDevice_001
* @tc.desc  : Test HandleRemoteCastDevice.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, HandleRemoteCastDevice_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();

    bool isConnected = false;
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate =  AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.HandleRemoteCastDevice(isConnected, audioStreamInfo);
    sleep(1);
}
#endif
/**
 * @tc.name  : Test DeviceVolumeInfosDump.
 * @tc.number: DeviceVolumeInfosDump_001
 * @tc.desc  : Test DeviceVolumeInfosDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, DeviceVolumeInfosDump_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::string dumpString = "666";
    DeviceVolumeInfoMap deviceVolumeInfos;
    server->audioPolicyDump_.DeviceVolumeInfosDump(dumpString, deviceVolumeInfos);
}

/**
 * @tc.name  : Test StreamVolumesDump.
 * @tc.number: StreamVolumesDump_001
 * @tc.desc  : Test StreamVolumesDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, StreamVolumesDump_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::string dumpString = "666";
    server->audioPolicyDump_.StreamVolumesDump(dumpString);
}

/**
 * @tc.name  : Test StreamVolumesDump.
 * @tc.number: StreamVolumesDump_002
 * @tc.desc  : Test StreamVolumesDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, StreamVolumesDump_002, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::string dumpString = "666";
    server->audioPolicyDump_.StreamVolumesDump(dumpString);
}

/**
 * @tc.name  : Test IsStreamSupported.
 * @tc.number: IsStreamSupported_001
 * @tc.desc  : Test IsStreamSupported interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, IsStreamSupported_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioStreamType streamType = STREAM_MUSIC;
    EXPECT_TRUE(server->audioPolicyDump_.IsStreamSupported(streamType));
    streamType = STREAM_VOICE_CALL;
    EXPECT_TRUE(server->audioPolicyDump_.IsStreamSupported(streamType));
    streamType = STREAM_VOICE_COMMUNICATION;
    EXPECT_TRUE(server->audioPolicyDump_.IsStreamSupported(streamType));
    streamType = STREAM_VOICE_ASSISTANT;
    EXPECT_TRUE(server->audioPolicyDump_.IsStreamSupported(streamType));
    streamType = STREAM_WAKEUP;
    EXPECT_TRUE(server->audioPolicyDump_.IsStreamSupported(streamType));
    streamType = STREAM_CAMCORDER;
    EXPECT_TRUE(server->audioPolicyDump_.IsStreamSupported(streamType));
}

/**
 * @tc.name  : Test IsStreamSupported.
 * @tc.number: IsStreamSupported_002
 * @tc.desc  : Test IsStreamSupported interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, IsStreamSupported_002, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioStreamType streamType = STREAM_MEDIA;
    EXPECT_FALSE(server->audioPolicyDump_.IsStreamSupported(streamType));
}

/**
 * @tc.name  : Test GetCallStatusDump.
 * @tc.number: GetCallStatusDump_001
 * @tc.desc  : Test GetCallStatusDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetCallStatusDump_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::string dumpString = "666";
    server->audioPolicyDump_.GetCallStatusDump(dumpString);
}

/**
 * @tc.name  : Test GetCallStatusDump.
 * @tc.number: GetCallStatusDump_002
 * @tc.desc  : Test GetCallStatusDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetCallStatusDump_002, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::string dumpString = "666";
    server->audioPolicyDump_.GetCallStatusDump(dumpString);
}

/**
 * @tc.name  : Test GetRingerModeDump.
 * @tc.number: GetRingerModeDump_001
 * @tc.desc  : Test GetRingerModeDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetRingerModeDump_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::string dumpString = "666";
    server->audioPolicyDump_.GetRingerModeDump(dumpString);
}

/**
 * @tc.name  : Test GetDumpDevices.
 * @tc.number: GetDumpDevices_001
 * @tc.desc  : Test GetDumpDevices interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetDumpDevices_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    server->audioPolicyDump_.GetDumpDevices(DeviceFlag::NONE_DEVICES_FLAG);
    server->audioPolicyDump_.GetDumpDevices(DeviceFlag::DISTRIBUTED_OUTPUT_DEVICES_FLAG);
    server->audioPolicyDump_.GetDumpDevices(DeviceFlag::DISTRIBUTED_INPUT_DEVICES_FLAG);
    server->audioPolicyDump_.GetDumpDevices(DeviceFlag::ALL_DISTRIBUTED_DEVICES_FLAG);
}

/**
 * @tc.name  : Test GetDumpDevices.
 * @tc.number: GetDumpDevices_002
 * @tc.desc  : Test GetDumpDevices interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetDumpDevices_002, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    server->audioPolicyDump_.GetDumpDevices(DeviceFlag::ALL_L_D_DEVICES_FLAG);
}

/**
 * @tc.name  : Test GetDumpDevices.
 * @tc.number: GetDumpDevices_003
 * @tc.desc  : Test GetDumpDevices interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetDumpDevices_003, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    server->audioPolicyDump_.GetDumpDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
}
/**
 * @tc.name  : Test SetDeviceSafeVolumeStatus.
 * @tc.number: SetDeviceSafeVolumeStatus_001
 * @tc.desc  : Test SetDeviceSafeVolumeStatus interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetDeviceSafeVolumeStatus_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    server->audioPolicyService_.audioVolumeManager_.SetDeviceSafeVolumeStatus();
}

/**
 * @tc.name  : Test SetDeviceSafeVolumeStatus.
 * @tc.number: SetDeviceSafeVolumeStatus_002
 * @tc.desc  : Test SetDeviceSafeVolumeStatus interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetDeviceSafeVolumeStatus_002, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    server->audioPolicyService_.audioVolumeManager_.userSelect_ = true;
    server->audioPolicyService_.audioVolumeManager_.SetDeviceSafeVolumeStatus();
}

/**
 * @tc.name  : Test SetDeviceSafeVolumeStatus.
 * @tc.number: SetDeviceSafeVolumeStatus_003
 * @tc.desc  : Test SetDeviceSafeVolumeStatus interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetDeviceSafeVolumeStatus_003, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    server->audioPolicyService_.audioVolumeManager_.userSelect_ = true;
    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_DP;
    server->audioPolicyService_.audioVolumeManager_.SetDeviceSafeVolumeStatus();
}

/**
 * @tc.name  : Test SetDeviceSafeVolumeStatus.
 * @tc.number: SetDeviceSafeVolumeStatus_004
 * @tc.desc  : Test SetDeviceSafeVolumeStatus interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetDeviceSafeVolumeStatus_004, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    server->audioPolicyService_.audioVolumeManager_.userSelect_ = true;
    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_WAKEUP;
    server->audioPolicyService_.audioVolumeManager_.SetDeviceSafeVolumeStatus();
}

/**
* @tc.name  : Test CheckForA2dpSuspend.
* @tc.number: CheckForA2dpSuspend_001
* @tc.desc  : Test CheckForA2dpSuspend.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, CheckForA2dpSuspend_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;

    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.CheckForA2dpSuspend(*desc);
}

/**
* @tc.name  : Test CheckForA2dpSuspend.
* @tc.number: CheckForA2dpSuspend_002
* @tc.desc  : Test CheckForA2dpSuspend.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, CheckForA2dpSuspend_002, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, desc) << "audioDeviceDescriptor is nullptr.";
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.CheckForA2dpSuspend(*desc);
}

/**
* @tc.name  : Test GetActiveBluetoothDevice.
* @tc.number: GetActiveBluetoothDevice_001
* @tc.desc  : Test GetActiveBluetoothDevice.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetActiveBluetoothDevice_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();
    GetServerPtr()->audioPolicyService_.GetActiveBluetoothDevice();
}

/**
* @tc.name  : Test SetCallDeviceActive.
* @tc.number: SetCallDeviceActive_001
* @tc.desc  : Test SetCallDeviceActive.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetCallDeviceActive_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();
    InternalDeviceType deviceType = DEVICE_TYPE_INVALID;
    bool active =true;
    std::string address= "123456";

    GetServerPtr()->audioPolicyService_.SetCallDeviceActive(deviceType, active, address);
}

/**
* @tc.name  : Test GetMaxAmplitude.
* @tc.number: GetMaxAmplitude_001
* @tc.desc  : Test GetMaxAmplitude.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetMaxAmplitude_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, GetServerPtr());
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();

    const int32_t deviceId = 0;
    AudioInterrupt audioInterrupt;
    float amplitude = GetServerPtr()->audioPolicyService_.GetMaxAmplitude(deviceId, audioInterrupt);
    EXPECT_EQ((std::abs(amplitude - 0.0f) <= std::abs(FLOAT_EPS)), true);
}

/**
* @tc.name  : Test AudioToneParser.
* @tc.number: AudioToneParser_001
* @tc.desc  : Test AudioToneParser.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, AudioToneParser_001, TestSize.Level1)
{
    std::unique_ptr<AudioToneParser> audioToneParser = std::make_unique<AudioToneParser>();
    ASSERT_NE(nullptr, audioToneParser);

    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> toneDescriptorMap;
    int32_t res = -1;
    res = audioToneParser->LoadConfig(toneDescriptorMap);
    EXPECT_EQ(res, SUCCESS);
}

/**
* @tc.name  : Test AudioToneParser.
* @tc.number: AudioToneParser_002
* @tc.desc  : Test AudioToneParser.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, AudioToneParser_002, TestSize.Level1)
{
    std::unique_ptr<AudioToneParser> audioToneParser = std::make_unique<AudioToneParser>();
    ASSERT_NE(nullptr, audioToneParser);

    const std::string configPath = "/system/etc/audio/audio_tone_dtmf_config.xml";
    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> toneDescriptorMap;
    std::unordered_map<std::string, std::unordered_map<int32_t, std::shared_ptr<ToneInfo>>> customToneDescriptorMap;

    int32_t res = -1;
    res = audioToneParser->LoadNewConfig("", toneDescriptorMap, customToneDescriptorMap);
    EXPECT_NE(res, SUCCESS);

    res = audioToneParser->LoadNewConfig(configPath, toneDescriptorMap, customToneDescriptorMap);
    EXPECT_EQ(res, SUCCESS);
}

/**
* @tc.name  : Test AudioToneParser.
* @tc.number: AudioToneParser_003
* @tc.desc  : Test AudioToneParser.
*/
HWTEST_F(AudioPolicyServiceThirdUnitTest, AudioToneParser_003, TestSize.Level1)
{
    std::unique_ptr<AudioToneParser> audioToneParser = std::make_unique<AudioToneParser>();
    ASSERT_NE(nullptr, audioToneParser);

    std::shared_ptr<ToneInfo> ltoneDesc = std::make_shared<ToneInfo>();
    std::shared_ptr<AudioXmlNode> node = AudioXmlNode::Create();
    audioToneParser->ParseToneInfoAttribute(node, ltoneDesc);
    EXPECT_NE(nullptr, ltoneDesc);

    std::vector<ToneInfoMap*> toneDescriptorMaps;
    std::unordered_map<int32_t, std::shared_ptr<ToneInfo>> toneInfoMap;
    toneDescriptorMaps.push_back(&toneInfoMap);
    toneDescriptorMaps.push_back(nullptr);
    audioToneParser->ParseToneInfo(node, toneDescriptorMaps);
    EXPECT_EQ(0, toneInfoMap.size());

    std::unordered_map<std::string, std::unordered_map<int32_t, std::shared_ptr<ToneInfo>>> customToneDescriptorMap;
    audioToneParser->ParseCustom(node, customToneDescriptorMap);
    EXPECT_EQ(0, customToneDescriptorMap.size());
}

/**
 * @tc.name  : Test NotifyRecreateRendererStream.
 * @tc.number: NotifyRecreateRendererStream_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, NotifyRecreateRendererStream_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);
    bool ret = true;

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    ret = server->audioPolicyService_.audioDeviceCommon_.NotifyRecreateRendererStream(audioDeviceDescriptor,
        rendererChangeInfo, AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    EXPECT_EQ(ret, false);

    rendererChangeInfo->outputDeviceInfo.networkId_ == LOCAL_NETWORK_ID;
    ret = server->audioPolicyService_.audioDeviceCommon_.NotifyRecreateRendererStream(audioDeviceDescriptor,
        rendererChangeInfo, AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test NotifyRecreateCapturerStream.
 * @tc.number: NotifyRecreateCapturerStream_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, NotifyRecreateCapturerStream_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);
    bool ret = true;

    shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = make_shared<AudioCapturerChangeInfo>();
    ret = server->audioPolicyService_.audioDeviceCommon_.NotifyRecreateCapturerStream(true, capturerChangeInfo,
        AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    EXPECT_EQ(ret, false);

    capturerChangeInfo->inputDeviceInfo.networkId_ == LOCAL_NETWORK_ID;
    capturerChangeInfo->capturerInfo.originalFlag = AUDIO_FLAG_MMAP;
    ret = server->audioPolicyService_.audioDeviceCommon_.NotifyRecreateCapturerStream(true, capturerChangeInfo,
        AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    EXPECT_EQ(ret, true);
}
} // namespace AudioStandard
} // namespace OHOS
