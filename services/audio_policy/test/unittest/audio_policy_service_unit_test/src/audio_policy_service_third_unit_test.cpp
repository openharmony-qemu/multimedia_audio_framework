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
#include "audio_server_proxy.h"
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

static AudioPolicyServer* GetServerPtr()
{
    return GetServerUtil::GetServerPtr();
}

void AudioPolicyServiceThirdUnitTest::SetUpTestCase(void) {}
void AudioPolicyServiceThirdUnitTest::TearDownTestCase(void)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);
    server->audioPolicyService_.Deinit();
}
void AudioPolicyServiceThirdUnitTest::SetUp(void) {}
void AudioPolicyServiceThirdUnitTest::TearDown(void) {}

class AudioSharedMemoryTest : public AudioSharedMemory {
public:
    uint8_t *GetBase() override { return nullptr; };
    size_t GetSize() override { return 0; };
    int GetFd() override { return 0; };
    std::string GetName() override { return "abc"; };
    bool Marshalling(Parcel &parcel) const override { return true; };
};

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
    float amplitude = GetServerPtr()->audioActiveDevice_.GetMaxAmplitude(deviceId, audioInterrupt);
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
 * @tc.name  : Test SaveSpecifiedDeviceVolume.
 * @tc.number: SaveSpecifiedDeviceVolume_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SaveSpecifiedDeviceVolume_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioStreamType streamType = STREAM_MUSIC;
    int32_t volumeLevel = 0;
    DeviceType deviceType = DEVICE_TYPE_INVALID;

    int32_t ret = server->audioVolumeManager_.SaveSpecifiedDeviceVolume(streamType, volumeLevel, deviceType);
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

    int32_t ret = server->audioVolumeManager_.SetAppVolumeMuted(appUid, muted);
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

    int32_t ret = server->audioVolumeManager_.IsAppVolumeMute(appUid, owned, isMute);
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
    int32_t ret = server->audioVolumeManager_.SetVoiceRingtoneMute(isMute);

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

    int32_t ret = server->audioVolumeManager_.GetAppVolumeLevel(appUid, volumeLevel);
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
    float ret = server->streamCollector_.GetLowPowerVolume(streamId);
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
    float ret = server->streamCollector_.GetSingleStreamVolume(streamId);
    EXPECT_EQ(ret, 1.0);
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
    bool ret = server->audioSceneManager_.IsStreamActive(streamType);

    EXPECT_EQ(ret, false);
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

    ret = server->audioConnectedDevice_.GetDevicesInner(deviceFlag);
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

#ifdef TEMP_DISABLE
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
    int32_t ret = server->audioMicrophoneDescriptor_.SetMicrophoneMute(isMute);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

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
    int32_t ret = server->audioMicrophoneDescriptor_.SetMicrophoneMutePersistent(isMute);
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

    bool ret = server->audioMicrophoneDescriptor_.GetPersistentMicMuteState();
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
    int32_t ret = server->audioPolicyManager_.SetSystemSoundUri(key, uri);

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
    std::string ret = server->audioPolicyManager_.GetSystemSoundUri(key);

    EXPECT_EQ(ret, "");
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
    bool ret = server->audioActiveDevice_.IsDeviceActive(deviceType);

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

    DeviceType ret = server->audioActiveDevice_.GetCurrentInputDeviceType();

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

    uint16_t ret = server->audioDeviceStatus_.GetDmDeviceType();
    EXPECT_EQ(ret, 0);
}

#ifdef TEMP_DISABLE
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
#endif

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

    AudioDeviceDescriptor updatedDesc = {};
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
    int32_t ret = server->audioDeviceCommon_.SetVirtualCall(0, isVirtual);
    EXPECT_EQ(ret, SUCCESS);
    ret = server->audioDeviceCommon_.SetVirtualCall(5523, isVirtual); /* 5523: call manager */
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

#ifdef TEMP_DISABLE
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
    int32_t ret = server->SetAudioClientInfoMgrCallback(object);

    EXPECT_EQ(ret, SUCCESS);
}
#endif

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
 * @tc.name  : Test IsAbsVolumeScene.
 * @tc.number: IsAbsVolumeScene_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, IsAbsVolumeScene_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    bool ret = server->audioPolicyManager_.IsAbsVolumeScene();
    EXPECT_EQ(ret, false);
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

    server->audioPolicyConfigManager_.SetNormalVoipFlag(normalVoipFlag);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test AudioPolicyDump.
 * @tc.number: AudioPolicyDump_001
 * @tc.desc  : Test AudioPolicyDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, AudioPolicyDump_001, TestSize.Level1)
{
    auto audioPolicyDump = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDump, nullptr);

    std::string dumpString = "abc";
    audioPolicyDump->audioPolicyManager_.SetRingerMode(RINGER_MODE_SILENT);
    audioPolicyDump->GetRingerModeDump(dumpString);
    EXPECT_EQ(dumpString, "abcRinger Mode:SILENT\n\n");
}

/**
 * @tc.name  : Test AudioPolicyDump.
 * @tc.number: AudioPolicyDump_002
 * @tc.desc  : Test AudioPolicyDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, AudioPolicyDump_002, TestSize.Level1)
{
    auto audioPolicyDump = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDump, nullptr);

    std::string dumpString = "abc";
    audioPolicyDump->audioPolicyManager_.SetRingerMode(RINGER_MODE_VIBRATE);
    audioPolicyDump->GetRingerModeDump(dumpString);
    EXPECT_EQ(dumpString, "abcRinger Mode:VIBRATE\n\n");
}

/**
 * @tc.name  : Test AudioPolicyDump.
 * @tc.number: AudioPolicyDump_003
 * @tc.desc  : Test AudioPolicyDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, AudioPolicyDump_003, TestSize.Level1)
{
    auto audioPolicyDump = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDump, nullptr);

    std::string dumpString = "abc";
    audioPolicyDump->audioPolicyManager_.SetRingerMode(static_cast<AudioRingerMode>(3));
    audioPolicyDump->GetRingerModeDump(dumpString);
    EXPECT_EQ(dumpString, "abcRinger Mode:UNKNOWN\n\n");
}

/**
 * @tc.name  : Test AudioPolicyDump.
 * @tc.number: AudioPolicyDump_004
 * @tc.desc  : Test AudioPolicyDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, AudioPolicyDump_004, TestSize.Level1)
{
    auto audioPolicyDump = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDump, nullptr);

    AdjustStreamVolumeInfo adjustStreamVolumeInfo;
    std::vector<AdjustStreamVolumeInfo> adjustInfo;
    adjustInfo.push_back(adjustStreamVolumeInfo);

    std::string dumpString = "abc";

    audioPolicyDump->AdjustVolumeAppend(adjustInfo, dumpString);
    EXPECT_NE(adjustInfo.size(), 0);
}

/**
 * @tc.name  : Test AudioPolicyDump.
 * @tc.number: AudioPolicyDump_005
 * @tc.desc  : Test AudioPolicyDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, AudioPolicyDump_005, TestSize.Level1)
{
    auto audioPolicyDump = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDump, nullptr);

    std::string dumpString = "abc";
    auto audioRendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    audioRendererChangeInfo->rendererInfo.rendererFlags = 0;
    audioPolicyDump->streamCollector_.audioRendererChangeInfos_.push_back(audioRendererChangeInfo);

    audioPolicyDump->AudioStreamDump(dumpString);
    EXPECT_NE(dumpString, "abc");
}

/**
 * @tc.name  : Test AudioPolicyDump.
 * @tc.number: AudioPolicyDump_006
 * @tc.desc  : Test AudioPolicyDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, AudioPolicyDump_006, TestSize.Level1)
{
    auto audioPolicyDump = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDump, nullptr);

    std::string dumpString = "abc";
    auto audioRendererChangeInfo = std::make_shared<AudioRendererChangeInfo>();
    audioRendererChangeInfo->rendererInfo.rendererFlags = 1;
    audioPolicyDump->streamCollector_.audioRendererChangeInfos_.push_back(audioRendererChangeInfo);

    audioPolicyDump->AudioStreamDump(dumpString);
    EXPECT_NE(dumpString, "abc");
}

/**
 * @tc.name  : Test AudioPolicyDump.
 * @tc.number: AudioPolicyDump_007
 * @tc.desc  : Test AudioPolicyDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, AudioPolicyDump_007, TestSize.Level1)
{
    auto audioPolicyDump = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDump, nullptr);

    AudioRingerMode ringerMode = RINGER_MODE_SILENT;
    auto audioRingerMode = audioPolicyDump->GetRingerModeType(ringerMode);
    EXPECT_EQ(audioRingerMode, "RINGER_MODE_SILENT");
}

/**
 * @tc.name  : Test AudioPolicyDump.
 * @tc.number: AudioPolicyDump_008
 * @tc.desc  : Test AudioPolicyDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, AudioPolicyDump_008, TestSize.Level1)
{
    auto audioPolicyDump = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDump, nullptr);

    AudioRingerMode ringerMode = RINGER_MODE_VIBRATE;
    auto audioRingerMode = audioPolicyDump->GetRingerModeType(ringerMode);
    EXPECT_EQ(audioRingerMode, "RINGER_MODE_VIBRATE");
}

/**
 * @tc.name  : Test AudioPolicyDump.
 * @tc.number: AudioPolicyDump_009
 * @tc.desc  : Test AudioPolicyDump interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, AudioPolicyDump_009, TestSize.Level1)
{
    auto audioPolicyDump = std::make_shared<AudioPolicyDump>();
    EXPECT_NE(audioPolicyDump, nullptr);

    AudioRingerMode ringerMode = static_cast<AudioRingerMode>(3);
    auto audioRingerMode = audioPolicyDump->GetRingerModeType(ringerMode);
    EXPECT_EQ(audioRingerMode, "UNKNOWMTYPE");
}

/**
 * @tc.name  : Test AudioPolicyUtils.
 * @tc.number: AudioPolicyUtils_001
 * @tc.desc  : Test AudioPolicyUtils interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, AudioPolicyUtils_001, TestSize.Level1)
{
    auto audioPolicyUtils = std::make_shared<AudioPolicyUtils>();
    EXPECT_NE(audioPolicyUtils, nullptr);

    AudioDeviceDescriptor desc;
    desc.networkId_ == REMOTE_NETWORK_ID;
    int32_t sessionId = 0;

    auto ret = audioPolicyUtils->GetSinkName(desc, sessionId);
    EXPECT_EQ(ret, "none");
}

/**
 * @tc.name  : Test AudioPolicyUtils.
 * @tc.number: AudioPolicyUtils_002
 * @tc.desc  : Test AudioPolicyUtils interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, AudioPolicyUtils_002, TestSize.Level1)
{
    auto audioPolicyUtils = std::make_shared<AudioPolicyUtils>();
    EXPECT_NE(audioPolicyUtils, nullptr);

    DeviceType deviceType = InternalDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP_IN;

    auto ret = audioPolicyUtils->GetSourcePortName(deviceType);
    EXPECT_EQ(ret, "Bt_Mic");
}

/**
 * @tc.name  : Test UpdateStreamState.
 * @tc.number: UpdateStreamState_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, UpdateStreamState_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    int32_t clientUid = 1;
    StreamSetStateEventInternal streamSetStateEventInternal;

    int32_t ret = server->streamCollector_.UpdateStreamState(clientUid, streamSetStateEventInternal);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test RemoveDeviceForUid.
 * @tc.number: RemoveDeviceForUid_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, RemoveDeviceForUid_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    int32_t uid = 1;
    server->audioAffinityManager_.DelSelectCapturerDevice(uid);
    server->audioAffinityManager_.DelSelectRendererDevice(uid);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test InitSharedVolume.
 * @tc.number: InitSharedVolume_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, InitSharedVolume_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::shared_ptr<AudioSharedMemory> buffer = std::make_shared<AudioSharedMemoryTest>();
    int32_t ret = server->audioPolicyService_.InitSharedVolume(buffer);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test MoveToNewPipe.
 * @tc.number: MoveToNewPipe_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, MoveToNewPipe_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    uint32_t sessionId = 0;
    AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
    int32_t ret = server->audioOffloadStream_.MoveToNewPipe(sessionId, pipeType);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test DynamicUnloadModule.
 * @tc.number: DynamicUnloadModule_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, DynamicUnloadModule_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioPipeType pipeType = PIPE_TYPE_OFFLOAD;
    int32_t ret = server->audioPolicyService_.DynamicUnloadModule(pipeType);
    EXPECT_EQ(ret, SUCCESS);

    pipeType = PIPE_TYPE_MULTICHANNEL;
    ret = server->audioPolicyService_.DynamicUnloadModule(pipeType);
    EXPECT_NE(ret, SUCCESS);

    pipeType = PIPE_TYPE_DIRECT_VOIP;
    ret = server->audioPolicyService_.DynamicUnloadModule(pipeType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test GetMaxRendererInstances.
 * @tc.number: GetMaxRendererInstances_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetMaxRendererInstances_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    int32_t ret = server->audioPolicyService_.GetMaxRendererInstances();
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test GetMinStreamVolume.
 * @tc.number: GetMinStreamVolume_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetMinStreamVolume_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    float ret = server->audioPolicyManager_.GetMinStreamVolume();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test GetMaxStreamVolume.
 * @tc.number: GetMaxStreamVolume_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetMaxStreamVolume_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    float ret = server->audioPolicyManager_.GetMaxStreamVolume();
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test QueryEffectManagerSceneMode.
 * @tc.number: QueryEffectManagerSceneMode_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, QueryEffectManagerSceneMode_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    SupportedEffectConfig supportedEffectConfig;
    int32_t ret = server->audioPolicyService_.QueryEffectManagerSceneMode(supportedEffectConfig);
    EXPECT_NE(ret, 0);
}

/**
 * @tc.name  : Test GetHardwareOutputSamplingRate.
 * @tc.number: GetHardwareOutputSamplingRate_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetHardwareOutputSamplingRate_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    std::shared_ptr<AudioDeviceDescriptor> desc = nullptr;
    int32_t ret = server->audioPolicyService_.GetHardwareOutputSamplingRate(desc);
    EXPECT_EQ(ret, -1);

    desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ == DeviceRole::DEVICE_ROLE_NONE;
    ret = server->audioPolicyService_.GetHardwareOutputSamplingRate(desc);
    EXPECT_EQ(ret, -1);

    desc->deviceRole_ == DeviceRole::OUTPUT_DEVICE;
    ret = server->audioPolicyService_.GetHardwareOutputSamplingRate(desc);
    EXPECT_NE(ret, 48000);
}

/**
 * @tc.name  : Test DeviceFilterByUsageInner.
 * @tc.number: DeviceFilterByUsageInner_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, DeviceFilterByUsageInner_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioDeviceUsage usage = MEDIA_OUTPUT_DEVICES;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    descs.push_back(desc);
    server->audioPolicyService_.DeviceFilterByUsageInner(usage, descs);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test GetConverterConfig.
 * @tc.number: GetConverterConfig_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetConverterConfig_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    ConverterConfig ret = AudioConverterParser::GetInstance().LoadConfig();
    EXPECT_NE(ret.outChannelLayout, 0);
}

/**
 * @tc.name  : Test GetSupportedAudioEffectProperty.
 * @tc.number: GetSupportedAudioEffectProperty_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetSupportedAudioEffectProperty_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioEffectPropertyArrayV3 propertyArray;
    int32_t ret = server->audioPolicyService_.GetSupportedAudioEffectProperty(propertyArray);
    EXPECT_EQ(ret, AUDIO_OK);
}

/**
 * @tc.name  : Test GetSupportedEffectProperty.
 * @tc.number: GetSupportedEffectProperty_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetSupportedEffectProperty_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioEffectPropertyArrayV3 propertyArray;
    server->audioPolicyService_.GetSupportedEffectProperty(propertyArray);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test GetSupportedEnhanceProperty.
 * @tc.number: GetSupportedEnhanceProperty_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetSupportedEnhanceProperty_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioEffectPropertyArrayV3 propertyArray;
    server->audioPolicyService_.GetSupportedEnhanceProperty(propertyArray);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test CheckSupportedAudioEffectProperty.
 * @tc.number: CheckSupportedAudioEffectProperty_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, CheckSupportedAudioEffectProperty_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioEffectPropertyArrayV3 propertyArray;
    EffectFlag flag = CAPTURE_EFFECT_FLAG;
    int32_t ret = server->audioPolicyService_.CheckSupportedAudioEffectProperty(propertyArray, flag);
    EXPECT_EQ(ret, AUDIO_OK);

    flag = RENDER_EFFECT_FLAG;
    ret = server->audioPolicyService_.CheckSupportedAudioEffectProperty(propertyArray, flag);
    EXPECT_EQ(ret, AUDIO_OK);
}

/**
 * @tc.name  : Test SetAudioEffectProperty.
 * @tc.number: SetAudioEffectProperty_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetAudioEffectProperty_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioEffectPropertyArrayV3 propertyArray;
    AudioEffectPropertyV3 effectProperty;
    effectProperty.flag = CAPTURE_EFFECT_FLAG;
    propertyArray.property.push_back(effectProperty);
    int32_t ret = server->audioPolicyService_.SetAudioEffectProperty(propertyArray);
    EXPECT_NE(ret, AUDIO_OK);

    propertyArray.property.clear();
    effectProperty.flag = RENDER_EFFECT_FLAG;
    propertyArray.property.push_back(effectProperty);
    ret = server->audioPolicyService_.SetAudioEffectProperty(propertyArray);
    EXPECT_NE(ret, AUDIO_OK);
}

/**
 * @tc.name  : Test GetAudioEnhanceProperty.
 * @tc.number: GetAudioEnhanceProperty_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetAudioEnhanceProperty_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioEffectPropertyArrayV3 propertyArray;
    AudioEffectPropertyV3 effectProperty;
    effectProperty.flag = CAPTURE_EFFECT_FLAG;
    propertyArray.property.push_back(effectProperty);
    int32_t ret = server->audioPolicyService_.SetAudioEffectProperty(propertyArray);
    EXPECT_NE(ret, AUDIO_OK);
}

/**
 * @tc.name  : Test GetAudioEffectProperty.
 * @tc.number: GetAudioEffectProperty_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, GetAudioEffectProperty_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    AudioEffectPropertyArrayV3 propertyArray;
    AudioEffectPropertyV3 effectProperty;
    effectProperty.flag = CAPTURE_EFFECT_FLAG;
    propertyArray.property.push_back(effectProperty);
    int32_t ret = server->audioPolicyService_.GetAudioEffectProperty(propertyArray);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test SetRotationToEffect.
 * @tc.number: SetRotationToEffect_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetRotationToEffect_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    uint32_t rotate = 0;
    AudioServerProxy::GetInstance().SetRotationToEffectProxy(rotate);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test IsCurrentActiveDeviceA2dp.
 * @tc.number: IsCurrentActiveDeviceA2dp_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, IsCurrentActiveDeviceA2dp_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    auto ret = server->audioPolicyManager_.GetActiveDevice() == DEVICE_TYPE_BLUETOOTH_A2DP;
    EXPECT_EQ(ret, false);
}

#ifdef TEMP_DISABLE
/**
 * @tc.name  : Test SetInputDevice.
 * @tc.number: SetInputDevice_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetInputDevice_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    DeviceType deviceType = DEVICE_TYPE_MAX;
    uint32_t sessionID = 0;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    bool isRunning = false;

    int32_t ret = server->SetInputDevice(deviceType, sessionID, sourceType, isRunning);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
 * @tc.name  : Test CheckConnectedDevice.
 * @tc.number: CheckConnectedDevice_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, CheckConnectedDevice_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    server->CheckConnectedDevice();
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test SetDeviceConnectedFlagFalseAfterDuration.
 * @tc.number: SetDeviceConnectedFlagFalseAfterDuration_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetDeviceConnectedFlagFalseAfterDuration_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    server->SetDeviceConnectedFlagFalseAfterDuration();
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test CheckHibernateState.
 * @tc.number: CheckHibernateState_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, CheckHibernateState_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    bool hibernate = false;
    server->CheckHibernateState(hibernate);
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test UpdateSafeVolumeByS4.
 * @tc.number: UpdateSafeVolumeByS4_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, UpdateSafeVolumeByS4_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    server->UpdateSafeVolumeByS4();
    EXPECT_NE(server, nullptr);
}

/**
 * @tc.name  : Test SetQueryAllowedPlaybackCallback.
 * @tc.number: SetQueryAllowedPlaybackCallback_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetQueryAllowedPlaybackCallback_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    sptr<IRemoteObject> object = nullptr;
    int32_t ret = server->audioBackgroundManager_.SetQueryAllowedPlaybackCallback(object);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test SetBackgroundMuteCallback.
 * @tc.number: SetBackgroundMuteCallback_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceThirdUnitTest, SetBackgroundMuteCallback_001, TestSize.Level1)
{
    auto server = GetServerPtr();
    ASSERT_NE(nullptr, server);

    sptr<IRemoteObject> object = nullptr;
    int32_t ret = server->audioBackgroundManager_.SetBackgroundMuteCallback(object);
    EXPECT_EQ(ret, SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
