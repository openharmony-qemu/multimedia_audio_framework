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
static const std::string PIPE_ACCESSORY_INPUT_UNITTEST = "accessory_input";
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
 * @tc.name  : Test SetDeviceSafeVolumeStatus.
 * @tc.number: SetDeviceSafeVolumeStatus_005
 * @tc.desc  : Test SetDeviceSafeVolumeStatus interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetDeviceSafeVolumeStatus_005, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    server->audioPolicyService_.audioVolumeManager_.userSelect_ = true;
    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
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

/**
 * @tc.name  : Test SetSystemVolumeLevelWithDevice.
 * @tc.number: SetSystemVolumeLevelWithDevice_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetSystemVolumeLevelWithDevice_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 0;
    DeviceType deviceType = DEVICE_TYPE_INVALID;

    int32_t ret = server->audioPolicyService_.SetSystemVolumeLevelWithDevice(streamType, volumeLevel, deviceType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetAppVolumeLevel.
 * @tc.number: SetAppVolumeLevel_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetAppVolumeLevel_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    int32_t appUid = -1;
    int32_t volumeLevel = 0;

    int32_t ret = server->audioPolicyService_.SetAppVolumeLevel(appUid, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetAppVolumeMuted.
 * @tc.number: SetAppVolumeMuted_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetAppVolumeMuted_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    int32_t appUid = -1;
    bool muted = false;

    int32_t ret = server->audioPolicyService_.SetAppVolumeMuted(appUid, muted);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test IsAppVolumeMute.
 * @tc.number: IsAppVolumeMute_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, IsAppVolumeMute_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    int32_t appUid = -1;
    bool owned = false;
    bool isMute = false;

    int32_t ret = server->audioPolicyService_.IsAppVolumeMute(appUid, owned, isMute);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetVoiceRingtoneMute.
 * @tc.number: SetVoiceRingtoneMute_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetVoiceRingtoneMute_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    bool isMute = false;
    int32_t ret = server->audioPolicyService_.SetVoiceRingtoneMute(isMute);

    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetAppVolumeLevel.
 * @tc.number: GetAppVolumeLevel_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetAppVolumeLevel_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    int32_t appUid = -1;
    int32_t volumeLevel = 0;

    int32_t ret = server->audioPolicyService_.GetAppVolumeLevel(appUid, volumeLevel);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetLowPowerVolume.
 * @tc.number: GetLowPowerVolume_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetLowPowerVolume_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    int32_t streamId = -1;
    float ret = server->audioPolicyService_.GetLowPowerVolume(streamId);
    EXPECT_EQ(ret, 1.0);
}

/**
 * @tc.name  : Test GetSingleStreamVolume.
 * @tc.number: GetSingleStreamVolume_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetSingleStreamVolume_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    int32_t streamId = -1;
    float ret = server->audioPolicyService_.GetSingleStreamVolume(streamId);
    EXPECT_EQ(ret, 1.0);
}

/**
 * @tc.name  : Test SelectOutputDevice.
 * @tc.number: SelectOutputDevice_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SelectOutputDevice_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;

    audioRendererFilter->uid = 20010041;
    audioRendererFilter->rendererInfo.contentType   = ContentType::CONTENT_TYPE_MUSIC;
    audioRendererFilter->rendererInfo.streamUsage   = StreamUsage::STREAM_USAGE_MEDIA;
    audioRendererFilter->rendererInfo.rendererFlags = 0;
    audioRendererFilter->streamId = 0;

    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    auto outputDevice =  audioDeviceDescriptors[0];
    outputDevice->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    outputDevice->networkId_ = LOCAL_NETWORK_ID;
    selectedDesc.push_back(outputDevice);

    int32_t ret = server->audioPolicyService_.SelectOutputDevice(audioRendererFilter, selectedDesc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SelectInputDevice.
 * @tc.number: SelectInputDevice_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SelectInputDevice_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;

    audioCapturerFilter->uid = 20010041;

    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    auto outputDevice =  audioDeviceDescriptors[0];
    outputDevice->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    outputDevice->networkId_ = LOCAL_NETWORK_ID;
    selectedDesc.push_back(outputDevice);

    int32_t ret = server->audioPolicyService_.SelectInputDevice(audioCapturerFilter, selectedDesc);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test IsStreamActive.
 * @tc.number: IsStreamActive_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, IsStreamActive_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioStreamType streamType = STREAM_DEFAULT;
    bool ret = server->audioPolicyService_.IsStreamActive(streamType);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test ConfigDistributedRoutingRole.
 * @tc.number: ConfigDistributedRoutingRole_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, ConfigDistributedRoutingRole_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::shared_ptr<AudioDeviceDescriptor> descriptor;
    CastType type = CAST_TYPE_NULL;

    server->audioPolicyService_.ConfigDistributedRoutingRole(descriptor, type);
    EXPECT_EQ(type, CAST_TYPE_NULL);
}

/**
 * @tc.name  : Test SetWakeUpAudioCapturerFromAudioServer.
 * @tc.number: SetWakeUpAudioCapturerFromAudioServer_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetWakeUpAudioCapturerFromAudioServer_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioProcessConfig config;
    int32_t ret = server->audioPolicyService_.SetWakeUpAudioCapturerFromAudioServer(config);

    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test NotifyCapturerAdded.
 * @tc.number: NotifyCapturerAdded_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, NotifyCapturerAdded_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioCapturerInfo capturerInfo;
    AudioStreamInfo streamInfo;
    uint32_t sessionId = 0;

    int32_t ret = server->audioPolicyService_.NotifyCapturerAdded(capturerInfo, streamInfo, sessionId);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test NotifyWakeUpCapturerRemoved.
 * @tc.number: NotifyWakeUpCapturerRemoved_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, NotifyWakeUpCapturerRemoved_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    int32_t ret = server->audioPolicyService_.NotifyWakeUpCapturerRemoved();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test IsAbsVolumeSupported.
 * @tc.number: IsAbsVolumeSupported_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, IsAbsVolumeSupported_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    bool ret = server->audioPolicyService_.IsAbsVolumeSupported();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test GetDevicesInner.
 * @tc.number: GetDevicesInner_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetDevicesInner_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    DeviceFlag deviceFlag = NONE_DEVICES_FLAG;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;

    ret = server->audioPolicyService_.GetDevicesInner(deviceFlag);
    EXPECT_EQ(ret.size(), 0);
}

/**
 * @tc.name  : Test GetPreferredInputDeviceDescriptors.
 * @tc.number: GetPreferredInputDeviceDescriptors_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetPreferredInputDeviceDescriptors_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioCapturerInfo captureInfo;
    std::string networkId = "1";
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;

    ret = server->audioPolicyService_.GetPreferredInputDeviceDescriptors(captureInfo, networkId);
    EXPECT_NE(ret.size(), 0);
}

/**
 * @tc.name  : Test GetPreferredOutputDeviceDescInner.
 * @tc.number: GetPreferredOutputDeviceDescInner_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetPreferredOutputDeviceDescInner_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioRendererInfo rendererInfo;
    std::string networkId = "1";
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;

    ret = server->audioPolicyService_.GetPreferredOutputDeviceDescInner(rendererInfo, networkId);
    EXPECT_NE(ret.size(), 0);
}

/**
 * @tc.name  : Test GetPreferredInputDeviceDescInner.
 * @tc.number: GetPreferredInputDeviceDescInner_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetPreferredInputDeviceDescInner_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioCapturerInfo captureInfo;
    std::string networkId = "1";
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ret;

    ret = server->audioPolicyService_.GetPreferredInputDeviceDescInner(captureInfo, networkId);
    EXPECT_NE(ret.size(), 0);
}

/**
 * @tc.name  : Test SetMicrophoneMute.
 * @tc.number: SetMicrophoneMute_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetMicrophoneMute_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    bool isMute =false;
    int32_t ret = server->audioPolicyService_.SetMicrophoneMute(isMute);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetMicrophoneMutePersistent.
 * @tc.number: SetMicrophoneMutePersistent_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetMicrophoneMutePersistent_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    bool isMute =false;
    int32_t ret = server->audioPolicyService_.SetMicrophoneMutePersistent(isMute);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetPersistentMicMuteState.
 * @tc.number: GetPersistentMicMuteState_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetPersistentMicMuteState_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    bool ret = server->audioPolicyService_.GetPersistentMicMuteState();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test SetSystemSoundUri.
 * @tc.number: SetSystemSoundUri_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetSystemSoundUri_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::string key = "key";
    std::string uri = "uri";
    int32_t ret = server->audioPolicyService_.SetSystemSoundUri(key, uri);

    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetSystemSoundUri.
 * @tc.number: GetSystemSoundUri_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetSystemSoundUri_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::string key = "key";
    std::string ret = server->audioPolicyService_.GetSystemSoundUri(key);

    EXPECT_EQ(ret, "");
}

/**
 * @tc.name  : Test SetDeviceActive.
 * @tc.number: SetDeviceActive_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetDeviceActive_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    InternalDeviceType deviceType = DEVICE_TYPE_INVALID;
    bool active = false;
    int32_t pid = -1;
    int32_t ret = server->audioPolicyService_.SetDeviceActive(deviceType, active, pid);

    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test IsDeviceActive.
 * @tc.number: IsDeviceActive_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, IsDeviceActive_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    InternalDeviceType deviceType = DEVICE_TYPE_INVALID;
    bool ret = server->audioPolicyService_.IsDeviceActive(deviceType);

    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test GetActiveInputDevice.
 * @tc.number: GetActiveInputDevice_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetActiveInputDevice_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    DeviceType ret = server->audioPolicyService_.GetActiveInputDevice();

    EXPECT_NE(ret, DEVICE_TYPE_INVALID);
}

/**
 * @tc.name  : Test GetDmDeviceType.
 * @tc.number: GetDmDeviceType_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetDmDeviceType_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    uint16_t ret = server->audioPolicyService_.GetDmDeviceType();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test SetRingerMode.
 * @tc.number: SetRingerMode_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetRingerMode_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioRingerMode ringMode = RINGER_MODE_NORMAL;
    int32_t ret = server->audioPolicyService_.SetRingerMode(ringMode);

    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetAudioScene.
 * @tc.number: SetAudioScene_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetAudioScene_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioScene audioScene = AUDIO_SCENE_DEFAULT;
    int32_t ret = server->audioPolicyService_.SetAudioScene(audioScene);

    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetLastAudioScene.
 * @tc.number: GetLastAudioScene_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetLastAudioScene_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioScene ret = server->audioPolicyService_.GetLastAudioScene();
    EXPECT_EQ(ret, AUDIO_SCENE_DEFAULT);
}

/**
 * @tc.name  : Test OnUpdateAnahsSupport.
 * @tc.number: OnUpdateAnahsSupport_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, OnUpdateAnahsSupport_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::string anahsShowType = "anahsShowType";

    server->audioPolicyService_.OnUpdateAnahsSupport(anahsShowType);
    EXPECT_NE(server->audioPolicyService_.deviceStatusListener_, nullptr);
}

/**
 * @tc.name  : Test OnDeviceStatusUpdated.
 * @tc.number: OnDeviceStatusUpdated_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, OnDeviceStatusUpdated_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    DeviceType devType = DEVICE_TYPE_INVALID;
    bool isConnected = false;
    std::string macAddress = "00:11:22:33:44:55";
    std::string deviceName = "deviceName";
    AudioStreamInfo streamInfo;
    DeviceRole role = DEVICE_ROLE_NONE;
    bool hasPair = false;

    server->audioPolicyService_.OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo,
        role, hasPair);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test OnDeviceStatusUpdated.
 * @tc.number: OnDeviceStatusUpdated_002
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, OnDeviceStatusUpdated_002, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioDeviceDescriptor updatedDesc;
    bool isConnected = false;

    server->audioPolicyService_.OnDeviceStatusUpdated(updatedDesc, isConnected);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test UpdateA2dpOffloadFlagBySpatialService.
 * @tc.number: UpdateA2dpOffloadFlagBySpatialService_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, UpdateA2dpOffloadFlagBySpatialService_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::string macAddress = "00:11:22:33:44:55";
    std::unordered_map<uint32_t, bool> sessionIDToSpatializationEnableMap;

    server->audioPolicyService_.UpdateA2dpOffloadFlagBySpatialService(macAddress, sessionIDToSpatializationEnableMap);
    EXPECT_NE(server->audioPolicyService_.audioA2dpOffloadManager_, nullptr);
}

/**
 * @tc.name  : Test SetVirtualCall.
 * @tc.number: SetVirtualCall_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetVirtualCall_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    bool isVirtual = false;
    int32_t ret = server->audioPolicyService_.SetVirtualCall(isVirtual);

    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test OnDeviceStatusUpdated.
 * @tc.number: OnDeviceStatusUpdated_003
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, OnDeviceStatusUpdated_003, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    DStatusInfo statusInfo;
    bool isStop = true;

    server->audioPolicyService_.OnDeviceStatusUpdated(statusInfo, isStop);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test OnServiceDisconnected.
 * @tc.number: OnServiceDisconnected_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, OnServiceDisconnected_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioServiceIndex serviceIndex = HDI_SERVICE_INDEX;

    server->audioPolicyService_.OnServiceDisconnected(serviceIndex);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test OnMonoAudioConfigChanged.
 * @tc.number: OnMonoAudioConfigChanged_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, OnMonoAudioConfigChanged_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    bool audioMono = false;

    server->audioPolicyService_.OnMonoAudioConfigChanged(audioMono);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test OnAudioBalanceChanged.
 * @tc.number: OnAudioBalanceChanged_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, OnAudioBalanceChanged_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    float audioBalance = 0.0;

    server->audioPolicyService_.OnAudioBalanceChanged(audioBalance);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test SetAudioClientInfoMgrCallback.
 * @tc.number: SetAudioClientInfoMgrCallback_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetAudioClientInfoMgrCallback_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    sptr<IRemoteObject> object = nullptr;
    int32_t ret = server->audioPolicyService_.SetAudioClientInfoMgrCallback(object);

    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RegisterTracker.
 * @tc.number: RegisterTracker_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, RegisterTracker_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioMode mode;
    AudioStreamChangeInfo streamChangeInfo;
    sptr<IRemoteObject> object = nullptr;
    int32_t apiVersion = 1;
    int32_t ret = server->audioPolicyService_.RegisterTracker(mode, streamChangeInfo, object, apiVersion);

    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test UpdateTracker.
 * @tc.number: UpdateTracker_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, UpdateTracker_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioMode mode;
    AudioStreamChangeInfo streamChangeInfo;
    int32_t ret = server->audioPolicyService_.UpdateTracker(mode, streamChangeInfo);

    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test FetchOutputDeviceForTrack.
 * @tc.number: FetchOutputDeviceForTrack_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, FetchOutputDeviceForTrack_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioStreamChangeInfo streamChangeInfo;

    server->audioPolicyService_.FetchOutputDeviceForTrack(streamChangeInfo,
        AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test FetchInputDeviceForTrack.
 * @tc.number: FetchInputDeviceForTrack_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, FetchInputDeviceForTrack_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioStreamChangeInfo streamChangeInfo;

    server->audioPolicyService_.FetchInputDeviceForTrack(streamChangeInfo);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test GetCurrentRendererChangeInfos.
 * @tc.number: GetCurrentRendererChangeInfos_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetCurrentRendererChangeInfos_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    bool hasBTPermission = false;
    bool hasSystemPermission = false;

    server->audioPolicyService_.GetCurrentRendererChangeInfos(audioRendererChangeInfos, hasBTPermission,
        hasSystemPermission);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test GetCurrentCapturerChangeInfos.
 * @tc.number: GetCurrentCapturerChangeInfos_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetCurrentCapturerChangeInfos_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    bool hasBTPermission = false;
    bool hasSystemPermission = false;

    int32_t ret = server->audioPolicyService_.GetCurrentCapturerChangeInfos(audioCapturerChangeInfos, hasBTPermission,
        hasSystemPermission);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RegisteredTrackerClientDied.
 * @tc.number: RegisteredTrackerClientDied_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, RegisteredTrackerClientDied_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    pid_t uid = 0;

    server->audioPolicyService_.RegisteredTrackerClientDied(uid);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test ReconfigureAudioChannel.
 * @tc.number: ReconfigureAudioChannel_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, ReconfigureAudioChannel_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    uint32_t channelCount = 2;
    DeviceType deviceType = DeviceType::DEVICE_TYPE_FILE_SINK;

    server->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_FILE_SINK;
    int32_t ret = server->audioPolicyService_.ReconfigureAudioChannel(channelCount, deviceType);
    EXPECT_EQ(ret, SUCCESS);

    deviceType = DeviceType::DEVICE_TYPE_FILE_SOURCE;
    ret = server->audioPolicyService_.ReconfigureAudioChannel(channelCount, deviceType);
    EXPECT_EQ(ret, SUCCESS);

    deviceType = DeviceType::DEVICE_TYPE_INVALID;
    ret = server->audioPolicyService_.ReconfigureAudioChannel(channelCount, deviceType);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test IsAbsVolumeScene.
 * @tc.number: IsAbsVolumeScene_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, IsAbsVolumeScene_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    bool ret = server->audioPolicyService_.IsAbsVolumeScene();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test GetAudioLatencyFromXml.
 * @tc.number: GetAudioLatencyFromXml_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetAudioLatencyFromXml_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    int32_t ret = server->audioPolicyService_.GetAudioLatencyFromXml();
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetSinkLatencyFromXml.
 * @tc.number: GetSinkLatencyFromXml_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetSinkLatencyFromXml_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    uint32_t ret = server->audioPolicyService_.GetSinkLatencyFromXml();
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test GetPreferredOutputStreamType.
 * @tc.number: GetPreferredOutputStreamType_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetPreferredOutputStreamType_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioRendererInfo rendererInfo;
    std::string bundleName;

    int32_t ret = server->audioPolicyService_.GetPreferredOutputStreamType(rendererInfo, bundleName);
    EXPECT_EQ(ret, AUDIO_FLAG_NORMAL);
}

/**
 * @tc.name  : Test SetNormalVoipFlag.
 * @tc.number: SetNormalVoipFlag_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetNormalVoipFlag_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    bool normalVoipFlag = false;

    server->audioPolicyService_.SetNormalVoipFlag(normalVoipFlag);
    EXPECT_NE(server, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS
