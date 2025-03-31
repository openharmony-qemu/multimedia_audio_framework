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

#include "audio_policy_service_first_unit_test.h"
#include "get_server_util.h"

#include <thread>
#include <memory>
#include <vector>
#include "audio_info.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

bool g_hasPermission = false;
static const std::string INNER_CAPTURER_SINK_LEGACY = "InnerCapturer";
const int32_t CONNECTING_NUMBER = 10;
const int32_t TEST_SESSIONID = MIN_STREAMID + 1010;
const int32_t ROUTER_MAP_ID0 = 1000;
const int32_t ROUTER_MAP_ID1 = 1001;
const int32_t ROUTER_MAP_ID2 = 1002;
const int32_t ROUTER_MAP_ID3 = 1003;
const int32_t ROUTER_MAP_ID4 = 1004;
const int32_t ROUTER_MAP_ID5 = 1005;
const int32_t ROUTER_MAP_ID6 = 1006;
const int32_t VALUE_ZERO = 0;
const int32_t DEFAULT_VOLUME_LEVEL = 7;
const int32_t G_UNKNOWN_PID = -1;
const uint32_t CHANNELS = 2;
const uint32_t RATE = 4;

void AudioPolicyServiceUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest::SetUpTestCase start-end");
}
void AudioPolicyServiceUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest::TearDownTestCase start-end");
}
void AudioPolicyServiceUnitTest::SetUp(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest::SetUp start-end");
}
void AudioPolicyServiceUnitTest::TearDown(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest::TearDown start-end");
}

static AudioPolicyServer* GetServerPtr()
{
    return GetServerUtil::GetServerPtr();
}

static void GetPermission()
{
    if (!g_hasPermission) {
        uint64_t tokenId;
        constexpr int perNum = 10;
        const char *perms[perNum] = {
            "ohos.permission.MICROPHONE",
            "ohos.permission.MANAGE_INTELLIGENT_VOICE",
            "ohos.permission.MANAGE_AUDIO_CONFIG",
            "ohos.permission.MICROPHONE_CONTROL",
            "ohos.permission.MODIFY_AUDIO_SETTINGS",
            "ohos.permission.ACCESS_NOTIFICATION_POLICY",
            "ohos.permission.USE_BLUETOOTH",
            "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
            "ohos.permission.RECORD_VOICE_CALL",
            "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 10,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audio_policy_service_unit_test",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

static const std::vector<DeviceType> deviceTypes = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_HDMI
};

static const std::vector<StreamSetState> streamSetStates = {
    STREAM_PAUSE,
    STREAM_RESUME,
    STREAM_MUTE,
    STREAM_UNMUTE
};

static const std::vector<bool>isConnecteds = {
    true,
    false
};
static const std::vector<BluetoothOffloadState>flags = {
    NO_A2DP_DEVICE,
    A2DP_NOT_OFFLOAD,
    A2DP_OFFLOAD
};
static const std::vector<AudioPipeType>pipeTypes = {
    PIPE_TYPE_UNKNOWN,
    PIPE_TYPE_NORMAL_OUT,
    PIPE_TYPE_NORMAL_IN,
    PIPE_TYPE_OFFLOAD,
    PIPE_TYPE_MULTICHANNEL
};
static const std::vector<Media::MediaMonitor::PreferredType>preferredTypes = {
    Media::MediaMonitor::MEDIA_RENDER,
    Media::MediaMonitor::CALL_RENDER,
    Media::MediaMonitor::RING_RENDER,
    Media::MediaMonitor::TONE_RENDER,
    Media::MediaMonitor::CALL_CAPTURE,
    Media::MediaMonitor::RECORD_CAPTURE
};
static const std::vector<StreamUsage>streamUsages = {
    STREAM_USAGE_INVALID,
    STREAM_USAGE_UNKNOWN,
    STREAM_USAGE_MEDIA,
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_VOICE_ASSISTANT,
    STREAM_USAGE_ALARM,
    STREAM_USAGE_VOICE_MESSAGE,
    STREAM_USAGE_NOTIFICATION_RINGTONE,
    STREAM_USAGE_RINGTONE,
    STREAM_USAGE_NOTIFICATION,
    STREAM_USAGE_SYSTEM,
    STREAM_USAGE_MOVIE,
    STREAM_USAGE_GAME,
    STREAM_USAGE_AUDIOBOOK,
    STREAM_USAGE_NAVIGATION,
    STREAM_USAGE_DTMF,
    STREAM_USAGE_ENFORCED_TONE,
    STREAM_USAGE_ULTRASONIC,
    STREAM_USAGE_VIDEO_COMMUNICATION,
    STREAM_USAGE_RANGING,
    STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
    STREAM_USAGE_VOICE_RINGTONE,
    STREAM_USAGE_VOICE_CALL_ASSISTANT,
    STREAM_USAGE_MAX
};
static const std::vector<AudioStreamType>audioStreamTypes = {
    STREAM_DEFAULT,
    STREAM_VOICE_CALL,
    STREAM_MUSIC,
    STREAM_RING,
    STREAM_MEDIA,
    STREAM_VOICE_ASSISTANT,
    STREAM_SYSTEM,
    STREAM_ALARM,
    STREAM_NOTIFICATION,
    STREAM_BLUETOOTH_SCO,
    STREAM_ENFORCED_AUDIBLE,
    STREAM_DTMF,
    STREAM_TTS,
    STREAM_RECORDING,
    STREAM_MOVIE,
    STREAM_GAME,
    STREAM_SPEECH,
    STREAM_SYSTEM_ENFORCED,
    STREAM_ULTRASONIC,
    STREAM_WAKEUP,
    STREAM_VOICE_MESSAGE,
    STREAM_NAVIGATION,
    STREAM_INTERNAL_FORCE_STOP,
    STREAM_SOURCE_VOICE_CALL,
    STREAM_VOICE_COMMUNICATION,
    STREAM_VOICE_RING,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_TYPE_MAX,
    STREAM_ALL
};
static const std::vector<bool>isMutes = {
    true,
    false
};
static const std::vector<DeviceRole>deviceRoles = {
    DEVICE_ROLE_NONE,
    INPUT_DEVICE,
    OUTPUT_DEVICE,
    DEVICE_ROLE_MAX
};
static const std::vector<DeviceFlag>deviceFlags = {
    NONE_DEVICES_FLAG,
    OUTPUT_DEVICES_FLAG,
    INPUT_DEVICES_FLAG,
    ALL_DEVICES_FLAG,
    DISTRIBUTED_OUTPUT_DEVICES_FLAG,
    DISTRIBUTED_INPUT_DEVICES_FLAG,
    ALL_DISTRIBUTED_DEVICES_FLAG,
    ALL_L_D_DEVICES_FLAG,
    DEVICE_FLAG_MAX
};
static const std::vector<AudioSampleFormat>audioSampleFormats = {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
    INVALID_WIDTH
};
static const std::vector<AudioScene>audioScenes = {
    AUDIO_SCENE_INVALID,
    AUDIO_SCENE_DEFAULT,
    AUDIO_SCENE_RINGING,
    AUDIO_SCENE_PHONE_CALL,
    AUDIO_SCENE_PHONE_CHAT,
    AUDIO_SCENE_CALL_START,
    AUDIO_SCENE_CALL_END,
    AUDIO_SCENE_VOICE_RINGING,
    AUDIO_SCENE_MAX
};
static const std::vector<AudioRingerMode>audioRingerModes = {
    RINGER_MODE_SILENT,
    RINGER_MODE_VIBRATE,
    RINGER_MODE_NORMAL
};


/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, AudioPolicyServiceTest_Prepare001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest AudioPolicyServiceTest_Prepare001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    // Get permission for test
    GetPermission();

    // Call OnServiceConnected for HDI_SERVICE_INDEX
    GetServerPtr()->audioPolicyService_.OnServiceConnected(HDI_SERVICE_INDEX);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, AudioPolicyServiceTest_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest AudioPolicyServiceTest_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    // DeviceTest
    AudioDeviceDescriptor audioDeviceDescriptor;
    audioDeviceDescriptor.deviceName_ = "dummyName";
    audioDeviceDescriptor.macAddress_ = "11:22:33:44:55:66";
    for (const auto& deviceType : deviceTypes) {
        AUDIO_INFO_LOG("AudioPolicyServiceTest_001 deviceType:%{public}d, TEST_SESSIONID:%{public}d",
            static_cast<uint32_t>(deviceType), TEST_SESSIONID);
        audioDeviceDescriptor.deviceType_ = deviceType;
        for (const auto& isConnected : isConnecteds) {
            AUDIO_INFO_LOG("AudioPolicyServiceTest_001 isConnected:%{public}d", static_cast<uint32_t>(isConnected));
            GetServerPtr()->audioPolicyService_.audioDeviceStatus_.hasModulesLoaded = true;
            GetServerPtr()->audioPolicyService_.OnPnpDeviceStatusUpdated(audioDeviceDescriptor, isConnected);
            GetServerPtr()->audioPolicyService_.SetCallDeviceActive(deviceType, isConnected,
                audioDeviceDescriptor.macAddress_);
        }
        bool ret = GetServerPtr()->audioPolicyService_.IsA2dpOffloadConnected();
        EXPECT_EQ(false, ret);
        GetServerPtr()->audioPolicyService_.audioDeviceLock_.UpdateSessionConnectionState(TEST_SESSIONID,
            CONNECTING_NUMBER);
        GetServerPtr()->audioPolicyService_.audioDeviceLock_.UpdateSessionConnectionState(TEST_SESSIONID,
            (CONNECTING_NUMBER + 1));
        GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->UpdateOffloadWhenActiveDeviceSwitchFromA2dp();
        GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->GetA2dpOffloadCodecAndSendToDsp();
        GetServerPtr()->audioPolicyService_.audioMicrophoneDescriptor_.UpdateAudioCapturerMicrophoneDescriptor(
            deviceType);
        for (const auto& flag : flags) {
            AUDIO_INFO_LOG("AudioPolicyServiceTest_001 flag:%{public}d", static_cast<uint32_t>(flag));
            GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleA2dpDeviceInOffload(flag);
            GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleA2dpDeviceOutOffload(flag);
        }
        for (const auto& pipeType : pipeTypes) {
            AUDIO_INFO_LOG("AudioPolicyServiceTest_001 pipeType:%{public}d", static_cast<uint32_t>(pipeType));
            GetServerPtr()->audioPolicyService_.audioOffloadStream_.MoveToNewPipeInner(TEST_SESSIONID, pipeType);
        }
        // AccountTest
        GetServerPtr()->audioPolicyService_.GetCurActivateCount();
        GetServerPtr()->audioPolicyService_.NotifyAccountsChanged(TEST_SESSIONID);
        // SafeVolumeTest
        GetServerPtr()->audioPolicyService_.audioVolumeManager_.SetDeviceSafeVolumeStatus();
        GetServerPtr()->audioPolicyService_.DisableSafeMediaVolume();
    }
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, AudioPolicyServiceTest_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest AudioPolicyServiceTest_002 start");
    ASSERT_NE(nullptr, GetServerPtr());
    // clear data
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();
    // dummy data
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.push_back(audioDeviceDescriptor);
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.isOffloadAvailable_ = true;

    for (const auto& preferredType : preferredTypes) {
        AUDIO_INFO_LOG("AudioPolicyServiceTest_002 preferredType:%{public}d", static_cast<uint32_t>(preferredType));
        for (const auto& deviceType : deviceTypes) {
            AUDIO_INFO_LOG("AudioPolicyServiceTest_002 deviceType:%{public}d", static_cast<uint32_t>(deviceType));
            for (const auto& streamUsage : streamUsages) {
                AUDIO_INFO_LOG("AudioPolicyServiceTest_002 streamUsage:%{public}d", static_cast<uint32_t>(streamUsage));
                GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.HandleRecoveryPreferredDevices(
                    preferredType, deviceType, streamUsage);
            }
        }
    }
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_003
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, AudioPolicyServiceTest_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest AudioPolicyServiceTest_003 start");
    ASSERT_NE(nullptr, GetServerPtr());
    // set offload support on for covery
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.isOffloadAvailable_ = true;
    for (const auto& deviceType : deviceTypes) {
        AUDIO_ERR_LOG("AudioPolicyServiceTest_003 deviceType:%{public}d, TEST_SESSIONID:%{public}d",
            static_cast<uint32_t>(deviceType), TEST_SESSIONID);
        for (const auto& audioStreamType : audioStreamTypes) {
            AUDIO_ERR_LOG("AudioPolicyServiceTest_003 streamType:%{public}d", static_cast<uint32_t>(audioStreamType));
            GetServerPtr()->audioPolicyService_.GetSystemVolumeDb(audioStreamType);
        }
        GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->GetVolumeGroupType(deviceType);
        int32_t streamId = 1;
        float volume = 0.1f;
        GetServerPtr()->audioPolicyService_.SetLowPowerVolume(streamId, volume);
        GetServerPtr()->audioPolicyService_.audioOffloadStream_.SetOffloadMode();
        GetServerPtr()->audioPolicyService_.audioOffloadStream_.ResetOffloadMode(TEST_SESSIONID);
        GetServerPtr()->audioPolicyService_.OffloadStreamReleaseCheck(TEST_SESSIONID);
        GetServerPtr()->audioPolicyService_.audioOffloadStream_.RemoteOffloadStreamRelease(TEST_SESSIONID);
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.CheckActiveOutputDeviceSupportOffload();
        GetServerPtr()->audioPolicyService_.audioOffloadStream_.GetOffloadAvailableFromXml();
        int32_t uid = getuid();
        for (const auto& isMute : isMutes) {
            AUDIO_ERR_LOG("AudioPolicyServiceTest_003 isMute:%{public}d", static_cast<uint32_t>(isMute));
            GetServerPtr()->audioPolicyService_.SetSourceOutputStreamMute(uid, isMute);
        }
        std::string networkId = REMOTE_NETWORK_ID;
        AudioVolumeType volumeType =AudioVolumeType::STREAM_MEDIA;
        int32_t groupId = 0;
        std::string condition = "EVENT_TYPE=1;VOLUME_GROUP_ID=" + std::to_string(groupId) + ";AUDIO_VOLUME_TYPE="
            + std::to_string(volumeType) + ";";
        std::string value = std::to_string(volume);
        GetServerPtr()->audioPolicyService_.NotifyRemoteRenderState(networkId, condition, value);
    }
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_004
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, AudioPolicyServiceTest_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest AudioPolicyServiceTest_004 start");
    ASSERT_NE(nullptr, GetServerPtr());
    AudioDeviceDescriptor audioDeviceDescriptor;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptorSptr = std::make_shared<AudioDeviceDescriptor>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptorSptrVector;
    audioDeviceDescriptorSptrVector.push_back(audioDeviceDescriptorSptr);
    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    // set offload support on for covery
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.isOffloadAvailable_ = true;
    for (const auto& deviceType : deviceTypes) {
        AUDIO_ERR_LOG("AudioPolicyServiceTest_004 deviceType:%{public}d, TEST_SESSIONID:%{public}d",
            static_cast<uint32_t>(deviceType), TEST_SESSIONID);
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = deviceType;
        GetServerPtr()->audioPolicyService_.IsArmUsbDevice(audioDeviceDescriptor);
        GetServerPtr()->audioPolicyService_.audioDeviceCommon_.IsDeviceConnected(audioDeviceDescriptorSptr);
        for (const auto& deviceRole : deviceRoles) {
            GetServerPtr()->audioPolicyService_.audioDeviceCommon_.DeviceParamsCheck(deviceRole,
                audioDeviceDescriptorSptrVector);
        }
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.NotifyUserSelectionEventToBt(audioDeviceDescriptorSptr);
        for (const auto& streamUsage : streamUsages) {
            GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SetRenderDeviceForUsage(streamUsage,
                audioDeviceDescriptorSptr);
            GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.WriteSelectOutputSysEvents(
                audioDeviceDescriptorSptrVector, streamUsage);
        }
        GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SelectOutputDevice(audioRendererFilter,
            audioDeviceDescriptorSptrVector);
        GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SelectFastOutputDevice(audioRendererFilter,
            audioDeviceDescriptorSptr);
        GetServerPtr()->audioPolicyService_.audioDeviceCommon_.FilterSourceOutputs(TEST_SESSIONID);
        for (const auto& isConnected :isConnecteds) {
            GetServerPtr()->audioPolicyService_.OnPnpDeviceStatusUpdated(audioDeviceDescriptor, isConnected);
        }
    }
}

void debugPrintMemoryVariable()
{
    // currentActiveDevice_.deviceType_
    AUDIO_INFO_LOG("debugPrintMemoryVariable() currentActiveDevice_:%{public}d, addr:%{private}p",
        static_cast<std::uint32_t>(GetServerPtr()->
            audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_), &GetServerPtr()->
            audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_);
    // connectedA2dpDeviceMap_
    AUDIO_INFO_LOG("debugPrintMemoryVariable() connectedA2dpDeviceMap_ isEmpty:%{public}d, addr:%{private}p",
        GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.empty(), &GetServerPtr()->
        audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_);
    for (auto it = GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.begin();
                it != GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.end(); ++it) {
        AUDIO_INFO_LOG("debugPrintMemoryVariable() connectedA2dpDevice:%{public}s", it->first.c_str());
    }
    // activeBTDevice_
    AUDIO_INFO_LOG("debugPrintMemoryVariable() activeBTDevice_:%{public}s, addr:%{private}p",
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_.c_str(),
        &GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_);
}

/**
* @tc.name  : Test SetStreamMute.
* @tc.number: SetStreamMute_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SetStreamMute_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SetStreamMute_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    // clear connectedA2dpDeviceMap_
    GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.clear();

    // modify currentActiveDevice_.deviceType_ to DEVICE_TYPE_SPEAKER
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    GetServerPtr()->audioPolicyService_.GetStreamMute(STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.SetStreamMute(STREAM_MUSIC, true);
    GetServerPtr()->audioPolicyService_.GetStreamMute(STREAM_RING);
    GetServerPtr()->audioPolicyService_.SetStreamMute(STREAM_RING, true);

    // modify currentActiveDevice_.deviceType_ to DEVICE_TYPE_BLUETOOTH_A2DP
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_
        = DEVICE_TYPE_BLUETOOTH_A2DP;
    GetServerPtr()->audioPolicyService_.GetStreamMute(STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.SetStreamMute(STREAM_MUSIC, true);
    GetServerPtr()->audioPolicyService_.GetStreamMute(STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.SetStreamMute(STREAM_MUSIC, true);

    // modify activeBTDevice_ and connectedA2dpDeviceMap_
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_ = "activeBTDevice";
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate =  AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    A2dpDeviceConfigInfo configInfo = {audioStreamInfo, true};
    GetServerPtr()->
        audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"activeBTDevice", configInfo});
    GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"A2dpDeviceCommon", {}});
    GetServerPtr()->audioPolicyService_.GetStreamMute(STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.SetStreamMute(STREAM_MUSIC, true);
    GetServerPtr()->audioPolicyService_.SetStreamMute(STREAM_MUSIC, false);

    // modify configInfo.absVolumeSupport to false
    configInfo.absVolumeSupport = false;
    GetServerPtr()->audioPolicyService_.GetStreamMute(STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.SetStreamMute(STREAM_MUSIC, true);

    // use STREAM_WAKEUP to test GetVolumeTypeFromStreamType's else branch
    GetServerPtr()->audioPolicyService_.SetStreamMute(STREAM_WAKEUP, true);
}

/**
* @tc.name  : Test GetActiveA2dpDeviceStreamInfo.
* @tc.number: GetActiveA2dpDeviceStreamInfo_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetActiveA2dpDeviceStreamInfo_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetActiveA2dpDeviceStreamInfo_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    bool ret = false;

    // deviceType use DEVICE_TYPE_SPEAKER
    AudioStreamInfo streamInfoRet = {};
    ret = GetServerPtr()->audioPolicyService_.audioActiveDevice_.GetActiveA2dpDeviceStreamInfo(DEVICE_TYPE_SPEAKER,
        streamInfoRet);
    EXPECT_EQ(false, ret);

    // clear activeBTDevice_ and connectedA2dpDeviceMap_
    // deviceType use DEVICE_TYPE_BLUETOOTH_A2DP
    GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.clear();
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_ = "";
    ret = GetServerPtr()->
        audioPolicyService_.audioActiveDevice_.GetActiveA2dpDeviceStreamInfo(DEVICE_TYPE_BLUETOOTH_A2DP,
        streamInfoRet);
    EXPECT_EQ(false, ret);

    // modify activeBTDevice_ and connectedA2dpDeviceMap_
    // deviceType use DEVICE_TYPE_BLUETOOTH_A2DP
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_ = "activeBTDevice";
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate =  AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    A2dpDeviceConfigInfo configInfo = {audioStreamInfo, true};
    GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"activeBTDevice",
        configInfo});
    GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"A2dpDeviceCommon", {}});
    ret = GetServerPtr()->
        audioPolicyService_.audioActiveDevice_.GetActiveA2dpDeviceStreamInfo(DEVICE_TYPE_BLUETOOTH_A2DP,
        streamInfoRet);
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetActiveA2dpDeviceStreamInfo_001 sRate::%{public}d, format:%{public}d,"
        "channels:%{public}d", streamInfoRet.samplingRate, streamInfoRet.format, streamInfoRet.channels);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test GetSelectedDeviceInfo.
* @tc.number: GetActiveDeviceStreamInfo_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetSelectedDeviceInfo_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetSelectedDeviceInfo_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    // clear data
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();

    // dummy data
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDeviceDescriptor->networkId_ = REMOTE_NETWORK_ID;

    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.push_back(audioDeviceDescriptor);
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.isOffloadAvailable_ = true;

    GetServerPtr()->audioPolicyService_.audioRouteMap_.routerMap_[ROUTER_MAP_ID1]
        = std::pair(LOCAL_NETWORK_ID, (G_UNKNOWN_PID - 1));
    GetServerPtr()->audioPolicyService_.audioRouteMap_.routerMap_[ROUTER_MAP_ID2]
        = std::pair(LOCAL_NETWORK_ID, G_UNKNOWN_PID);
    GetServerPtr()->audioPolicyService_.audioRouteMap_.routerMap_[ROUTER_MAP_ID3]
        = std::pair(REMOTE_NETWORK_ID, (G_UNKNOWN_PID + 1));
    GetServerPtr()->audioPolicyService_.audioRouteMap_.routerMap_[ROUTER_MAP_ID4]
        = std::pair(REMOTE_NETWORK_ID, (G_UNKNOWN_PID + 2));
    GetServerPtr()->audioPolicyService_.audioRouteMap_.routerMap_[ROUTER_MAP_ID5]
        = std::pair(REMOTE_NETWORK_ID, G_UNKNOWN_PID);

    GetServerPtr()->audioPolicyService_.GetSelectedDeviceInfo(ROUTER_MAP_ID0, G_UNKNOWN_PID, STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.GetSelectedDeviceInfo(ROUTER_MAP_ID1, (G_UNKNOWN_PID - 1), STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.GetSelectedDeviceInfo(ROUTER_MAP_ID1, (G_UNKNOWN_PID + 1), STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.GetSelectedDeviceInfo(ROUTER_MAP_ID2, G_UNKNOWN_PID, STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.GetSelectedDeviceInfo(ROUTER_MAP_ID2, (G_UNKNOWN_PID + 1), STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.GetSelectedDeviceInfo(ROUTER_MAP_ID3, G_UNKNOWN_PID, STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.GetSelectedDeviceInfo(ROUTER_MAP_ID3, (G_UNKNOWN_PID + 1), STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.GetSelectedDeviceInfo(ROUTER_MAP_ID4, G_UNKNOWN_PID, STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.GetSelectedDeviceInfo(ROUTER_MAP_ID4, (G_UNKNOWN_PID + 1), STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.GetSelectedDeviceInfo(ROUTER_MAP_ID4, (G_UNKNOWN_PID + 2), STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.GetSelectedDeviceInfo(ROUTER_MAP_ID5, G_UNKNOWN_PID, STREAM_MUSIC);
    GetServerPtr()->audioPolicyService_.GetSelectedDeviceInfo(ROUTER_MAP_ID6, G_UNKNOWN_PID, STREAM_MUSIC);

    GetServerPtr()->audioPolicyService_.audioRouteMap_.routerMap_[ROUTER_MAP_ID6] =
        std::pair(std::string(REMOTE_NETWORK_ID) + "_out", G_UNKNOWN_PID);
    GetServerPtr()->audioPolicyService_.GetSelectedDeviceInfo(ROUTER_MAP_ID6, G_UNKNOWN_PID, STREAM_MUSIC);
}

/**
* @tc.name  : Test CheckActiveOutputDeviceSupportOffload.
* @tc.number: CheckActiveOutputDeviceSupportOffload_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, CheckActiveOutputDeviceSupportOffload_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest CheckActiveOutputDeviceSupportOffload_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    bool ret = false;
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.networkId_
        = std::string(LOCAL_NETWORK_ID) + "xyz";
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    ret = GetServerPtr()->audioPolicyService_.audioActiveDevice_.CheckActiveOutputDeviceSupportOffload();
    EXPECT_EQ(false, ret);

    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.networkId_
        = std::string(LOCAL_NETWORK_ID) + "xyz";
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    ret = GetServerPtr()->audioPolicyService_.audioActiveDevice_.CheckActiveOutputDeviceSupportOffload();
    EXPECT_EQ(false, ret);

    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.networkId_ = LOCAL_NETWORK_ID;
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    ret = GetServerPtr()->audioPolicyService_.audioActiveDevice_.CheckActiveOutputDeviceSupportOffload();
    EXPECT_EQ(false, ret);

    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.networkId_ = LOCAL_NETWORK_ID;
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    ret = GetServerPtr()->audioPolicyService_.audioActiveDevice_.CheckActiveOutputDeviceSupportOffload();
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test SetSourceOutputStreamMute.
* @tc.number: SetSourceOutputStreamMute_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SetSourceOutputStreamMute_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SetSourceOutputStreamMute_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    int32_t uid = getuid();
    bool setMute = false;
    int32_t result = GetServerPtr()->audioPolicyService_.SetSourceOutputStreamMute(uid, setMute);
    EXPECT_EQ(result, 0);

    setMute = true;
    result = GetServerPtr()->audioPolicyService_.SetSourceOutputStreamMute(uid, setMute);
    EXPECT_EQ(result, 0);
}

/**
* @tc.name  : Test SelectOutputDevice.
* @tc.number: SelectOutputDevice_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SelectOutputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SelectOutputDevice_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    ASSERT_NE(nullptr, audioRendererFilter) << "audioRendererFilter is nullptr.";
    audioRendererFilter->uid = getuid();
    audioRendererFilter->rendererInfo.rendererFlags = STREAM_FLAG_FAST;
    audioRendererFilter->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    deviceDescriptorVector.push_back(audioDeviceDescriptor);

    int32_t result = GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SelectOutputDevice(
        audioRendererFilter, deviceDescriptorVector);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test SelectOutputDevice.
* @tc.number: SelectOutputDevice_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SelectOutputDevice_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SelectOutputDevice_002 start");
    ASSERT_NE(nullptr, GetServerPtr());
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    ASSERT_NE(nullptr, audioRendererFilter) << "audioRendererFilter is nullptr.";
    audioRendererFilter->uid = -1;
    audioRendererFilter->rendererInfo.rendererFlags = AudioPolicyDump::STREAM_FLAG_NORMAL;
    audioRendererFilter->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    deviceDescriptorVector.push_back(audioDeviceDescriptor);

    int32_t result = GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SelectOutputDevice(
        audioRendererFilter, deviceDescriptorVector);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test SelectOutputDevice.
* @tc.number: SelectOutputDevice_003
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SelectOutputDevice_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SelectOutputDevice_003 start");
    ASSERT_NE(nullptr, GetServerPtr());
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    ASSERT_NE(nullptr, audioRendererFilter) << "audioRendererFilter is nullptr.";
    audioRendererFilter->uid = getuid();
    audioRendererFilter->rendererInfo.rendererFlags = AudioPolicyDump::STREAM_FLAG_NORMAL;
    audioRendererFilter->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDeviceDescriptor->connectState_ = VIRTUAL_CONNECTED;
    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    deviceDescriptorVector.push_back(audioDeviceDescriptor);

    int32_t result = GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SelectOutputDevice(
        audioRendererFilter, deviceDescriptorVector);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test MoveToRemoteOutputDevice.
* @tc.number: MoveToRemoteOutputDevice_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, MoveToRemoteOutputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest MoveToRemoteOutputDevice_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    SinkInput sinkInput = {};
    sinkInput.streamId = 123;
    sinkInput.streamType = STREAM_MUSIC;
    sinkInput.uid = getuid();
    sinkInput.pid = getpid();
    vector<SinkInput> sinkInputs;
    sinkInputs.push_back(sinkInput);

    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, remoteDeviceDescriptor) << "remoteDeviceDescriptor is nullptr.";
    remoteDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    remoteDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    remoteDeviceDescriptor->networkId_ = std::string(LOCAL_NETWORK_ID) + "xyz";

    int32_t result = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.MoveToRemoteOutputDevice(
        sinkInputs, remoteDeviceDescriptor);
    EXPECT_EQ(ERR_INVALID_PARAM, result);
}

/**
* @tc.name  : Test MoveToRemoteOutputDevice.
* @tc.number: MoveToRemoteOutputDevice_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, MoveToRemoteOutputDevice_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest MoveToRemoteOutputDevice_002 start");
    ASSERT_NE(nullptr, GetServerPtr());
    SinkInput sinkInput = {};
    sinkInput.streamId = 123;
    sinkInput.streamType = STREAM_MUSIC;
    sinkInput.uid = getuid();
    sinkInput.pid = getpid();
    vector<SinkInput> sinkInputs;
    sinkInputs.push_back(sinkInput);

    std::shared_ptr<AudioDeviceDescriptor> remoteDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, remoteDeviceDescriptor) << "remoteDeviceDescriptor is nullptr.";
    remoteDeviceDescriptor->deviceType_ = DEVICE_TYPE_MIC;
    remoteDeviceDescriptor->deviceRole_ = DeviceRole::INPUT_DEVICE;
    remoteDeviceDescriptor->networkId_ = std::string(LOCAL_NETWORK_ID) + "xyz";;

    int32_t result = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.MoveToRemoteOutputDevice(
        sinkInputs, remoteDeviceDescriptor);
    EXPECT_EQ(ERR_INVALID_PARAM, result);
}

/**
* @tc.name  : Test SetCaptureDeviceForUsage.
* @tc.number: SetCaptureDeviceForUsage_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SetCaptureDeviceForUsage_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SetCaptureDeviceForUsage_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    AudioScene scene = AUDIO_SCENE_PHONE_CALL;
    SourceType srcType = SOURCE_TYPE_VOICE_COMMUNICATION;
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, descriptor) << "descriptor is nullptr.";
    descriptor->deviceType_ = DEVICE_TYPE_MIC;
    descriptor->deviceRole_ = DeviceRole::INPUT_DEVICE;
    descriptor->networkId_ = LOCAL_NETWORK_ID;
    EXPECT_NO_THROW(
        GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SetCaptureDeviceForUsage(scene, srcType, descriptor);
    );
}

/**
* @tc.name  : Test SetCaptureDeviceForUsage.
* @tc.number: SetCaptureDeviceForUsage_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SetCaptureDeviceForUsage_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SetCaptureDeviceForUsage_002 start");
    ASSERT_NE(nullptr, GetServerPtr());
    AudioScene scene = AUDIO_SCENE_PHONE_CHAT;
    SourceType srcType = SOURCE_TYPE_VOICE_COMMUNICATION;
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, descriptor) << "descriptor is nullptr.";
    descriptor->deviceType_ = DEVICE_TYPE_MIC;
    descriptor->deviceRole_ = DeviceRole::INPUT_DEVICE;
    descriptor->networkId_ = LOCAL_NETWORK_ID;
    EXPECT_NO_THROW(
        GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SetCaptureDeviceForUsage(scene, srcType, descriptor);
    );
}

/**
* @tc.name  : Test SetCaptureDeviceForUsage.
* @tc.number: SetCaptureDeviceForUsage_003
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SetCaptureDeviceForUsage_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SetCaptureDeviceForUsage_003 start");
    ASSERT_NE(nullptr, GetServerPtr());
    AudioScene scene = AUDIO_SCENE_VOICE_RINGING;
    SourceType srcType = SOURCE_TYPE_VOICE_COMMUNICATION;
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, descriptor) << "descriptor is nullptr.";
    descriptor->deviceType_ = DEVICE_TYPE_MIC;
    descriptor->deviceRole_ = DeviceRole::INPUT_DEVICE;
    descriptor->networkId_ = LOCAL_NETWORK_ID;
    EXPECT_NO_THROW(
        GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SetCaptureDeviceForUsage(scene, srcType, descriptor);
    );
}

/**
* @tc.name  : Test SetCaptureDeviceForUsage.
* @tc.number: SetCaptureDeviceForUsage_004
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SetCaptureDeviceForUsage_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SetCaptureDeviceForUsage_004 start");
    ASSERT_NE(nullptr, GetServerPtr());
    AudioScene scene = AUDIO_SCENE_VOICE_RINGING;
    SourceType srcType = SOURCE_TYPE_VOICE_MESSAGE;
    std::shared_ptr<AudioDeviceDescriptor> descriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, descriptor) << "descriptor is nullptr.";
    descriptor->deviceType_ = DEVICE_TYPE_MIC;
    descriptor->deviceRole_ = DeviceRole::INPUT_DEVICE;
    descriptor->networkId_ = LOCAL_NETWORK_ID;
    EXPECT_NO_THROW(
        GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SetCaptureDeviceForUsage(scene, srcType, descriptor);
    );
}

/**
* @tc.name  : Test GetSinkPortName.
* @tc.number: GetSinkPortName_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetSinkPortName_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetSinkPortName_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    InternalDeviceType deviceType = DEVICE_TYPE_NONE;
    AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
    string retPortName = "";

    // case1 InternalDeviceType::DEVICE_TYPE_BLUETOOTH_A2DP
    deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    GetServerPtr()->audioPolicyService_.SetA2dpOffloadFlag(A2DP_OFFLOAD);
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(PRIMARY_SPEAKER, retPortName);
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetSinkPortName_001 aaa");
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetSinkPortName_001 bbb");
    pipeType = PIPE_TYPE_OFFLOAD;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(OFFLOAD_PRIMARY_SPEAKER, retPortName);

    pipeType = PIPE_TYPE_MULTICHANNEL;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(MCH_PRIMARY_SPEAKER, retPortName);

    pipeType = PIPE_TYPE_DIRECT_MUSIC;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(PRIMARY_SPEAKER, retPortName);

    GetServerPtr()->audioPolicyService_.SetA2dpOffloadFlag(A2DP_NOT_OFFLOAD);
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(BLUETOOTH_SPEAKER, retPortName);

    // case 2 InternalDeviceType::DEVICE_TYPE_EARPIECE
    deviceType = DEVICE_TYPE_EARPIECE;
    pipeType = PIPE_TYPE_OFFLOAD;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(OFFLOAD_PRIMARY_SPEAKER, retPortName);

    // case 3 InternalDeviceType::DEVICE_TYPE_SPEAKER
    deviceType = DEVICE_TYPE_SPEAKER;
    pipeType = PIPE_TYPE_MULTICHANNEL;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(MCH_PRIMARY_SPEAKER, retPortName);

    // case 4 InternalDeviceType::DEVICE_TYPE_WIRED_HEADSET
    deviceType = DEVICE_TYPE_WIRED_HEADSET;
    pipeType = PIPE_TYPE_NORMAL_IN;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(PRIMARY_SPEAKER, retPortName);
}

/**
* @tc.name  : Test GetSinkPortName.
* @tc.number: GetSinkPortName_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetSinkPortName_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetSinkPortName_002 start");
    ASSERT_NE(nullptr, GetServerPtr());
    InternalDeviceType deviceType = DEVICE_TYPE_NONE;
    AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
    string retPortName = "";

    // case 5 InternalDeviceType::DEVICE_TYPE_WIRED_HEADPHONES
    deviceType = DEVICE_TYPE_WIRED_HEADPHONES;
    pipeType = PIPE_TYPE_OFFLOAD;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(OFFLOAD_PRIMARY_SPEAKER, retPortName);

    // case 6 InternalDeviceType::DEVICE_TYPE_USB_HEADSET
    deviceType = DEVICE_TYPE_USB_HEADSET;
    pipeType = PIPE_TYPE_MULTICHANNEL;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(MCH_PRIMARY_SPEAKER, retPortName);

    // case 7 InternalDeviceType::DEVICE_TYPE_BLUETOOTH_SCO
    deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    pipeType = PIPE_TYPE_NORMAL_IN;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(PRIMARY_SPEAKER, retPortName);
    // case 8 InternalDeviceType::DEVICE_TYPE_USB_ARM_HEADSET

    deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(USB_SPEAKER, retPortName);

    // case 9 InternalDeviceType::DEVICE_TYPE_DP
    deviceType = DEVICE_TYPE_DP;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(DP_SINK, retPortName);

    // case 10 InternalDeviceType::DEVICE_TYPE_FILE_SINK
    deviceType = DEVICE_TYPE_FILE_SINK;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(FILE_SINK, retPortName);

    // case 11 InternalDeviceType::DEVICE_TYPE_REMOTE_CAST
    deviceType = DEVICE_TYPE_REMOTE_CAST;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(REMOTE_CAST_INNER_CAPTURER_SINK_NAME, retPortName);

    // case 12 InternalDeviceType::DEVICE_TYPE_NONE
    deviceType = DEVICE_TYPE_NONE;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(PORT_NONE, retPortName);
}

/**
* @tc.name  : Test GetSinkPortName.
* @tc.number: GetSinkPortName_003
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetSinkPortName_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetSinkPortName_003 start");
    ASSERT_NE(nullptr, GetServerPtr());
    InternalDeviceType deviceType = DEVICE_TYPE_NONE;
    AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
    string retPortName = "";
    bool isEnable = false;

    // case 13 InternalDeviceType::DEVICE_TYPE_HDMI
    deviceType = DEVICE_TYPE_HDMI;
    GetServerPtr()->audioPolicyService_.audioConfigManager_.OnUpdateDefaultAdapter(isEnable);
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(PRIMARY_SPEAKER, retPortName);

    isEnable = true;
    GetServerPtr()->audioPolicyService_.audioConfigManager_.OnUpdateDefaultAdapter(isEnable);
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(DP_SINK, retPortName);

    // case 14 InternalDeviceType::DEVICE_TYPE_LINE_DIGITAL
    deviceType = DEVICE_TYPE_LINE_DIGITAL;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(DP_SINK, retPortName);

    isEnable = false;
    GetServerPtr()->audioPolicyService_.audioConfigManager_.OnUpdateDefaultAdapter(isEnable);
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType, pipeType);
    EXPECT_EQ(PRIMARY_SPEAKER, retPortName);
}

/**
* @tc.name  : Test GetSourcePortName.
* @tc.number: GetSourcePortName_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetSourcePortName_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetSourcePortName_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    InternalDeviceType deviceType = DEVICE_TYPE_NONE;
    string retPortName = "";

    deviceType = DEVICE_TYPE_MIC;
    retPortName = AudioPolicyUtils::GetInstance().GetSourcePortName(deviceType);
    EXPECT_EQ(PRIMARY_MIC, retPortName);

    deviceType = DEVICE_TYPE_USB_HEADSET;
    retPortName = AudioPolicyUtils::GetInstance().GetSourcePortName(deviceType);
    EXPECT_EQ(PORT_NONE, retPortName);

    deviceType = DEVICE_TYPE_BLUETOOTH_SCO;
    retPortName = AudioPolicyUtils::GetInstance().GetSourcePortName(deviceType);
    EXPECT_EQ(PORT_NONE, retPortName);

    deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    retPortName = AudioPolicyUtils::GetInstance().GetSourcePortName(deviceType);
    EXPECT_EQ(USB_MIC, retPortName);

    deviceType = DEVICE_TYPE_WAKEUP;
    retPortName = AudioPolicyUtils::GetInstance().GetSourcePortName(deviceType);
    EXPECT_EQ(PRIMARY_WAKEUP, retPortName);

    deviceType = DEVICE_TYPE_FILE_SOURCE;
    retPortName = AudioPolicyUtils::GetInstance().GetSourcePortName(deviceType);
    EXPECT_EQ(FILE_SOURCE, retPortName);

    deviceType = DEVICE_TYPE_MAX;
    retPortName = AudioPolicyUtils::GetInstance().GetSourcePortName(deviceType);
    EXPECT_EQ(PORT_NONE, retPortName);

    deviceType = DEVICE_TYPE_HDMI;
    retPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceType);
    EXPECT_EQ(PRIMARY_SPEAKER, retPortName);
}

/**
* @tc.name  : Test ConstructRemoteAudioModuleInfo.
* @tc.number: ConstructRemoteAudioModuleInfo_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, ConstructRemoteAudioModuleInfo_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest ConstructRemoteAudioModuleInfo_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    string networkId = "";
    DeviceRole deviceRole = DEVICE_ROLE_NONE;
    DeviceType deviceType = DEVICE_TYPE_NONE;
    AudioModuleInfo audioModuleInforet = {};

    networkId = REMOTE_NETWORK_ID;
    deviceRole = OUTPUT_DEVICE;
    deviceType = DEVICE_TYPE_MIC;
    audioModuleInforet = AudioPolicyUtils::GetInstance().ConstructRemoteAudioModuleInfo(
        networkId, deviceRole, deviceType);
    EXPECT_EQ("48000", audioModuleInforet.rate);

    networkId = REMOTE_NETWORK_ID;
    deviceRole = INPUT_DEVICE;
    deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioModuleInforet = AudioPolicyUtils::GetInstance().ConstructRemoteAudioModuleInfo(
        networkId, deviceRole, deviceType);
    EXPECT_EQ("48000", audioModuleInforet.rate);

    networkId = REMOTE_NETWORK_ID;
    deviceRole = DEVICE_ROLE_MAX;
    deviceType = DEVICE_TYPE_SPEAKER;
    audioModuleInforet = AudioPolicyUtils::GetInstance().ConstructRemoteAudioModuleInfo(
        networkId, deviceRole, deviceType);
    EXPECT_EQ("48000", audioModuleInforet.rate);
}

/**
* @tc.name  : Test ActivateNewDevice.
* @tc.number: ActivateNewDevice_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, ActivateNewDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest ActivateNewDevice_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    string networkId = "";
    DeviceType deviceType = DEVICE_TYPE_NONE;
    bool isRemote = false;
    int32_t ret = SUCCESS;

    networkId = LOCAL_NETWORK_ID;
    deviceType = DEVICE_TYPE_MIC;
    isRemote = true;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.ActivateNewDevice(networkId, deviceType, isRemote);
    EXPECT_EQ(SUCCESS, ret);

    networkId = REMOTE_NETWORK_ID;
    deviceType = DEVICE_TYPE_USB_HEADSET;
    isRemote = false;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.ActivateNewDevice(networkId, deviceType, isRemote);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetWakeUpAudioCapturer and ConstructWakeupAudioModuleInfo.
* @tc.number: SetWakeUpAudioCapturer_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SetWakeUpAudioCapturer_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SetWakeUpAudioCapturer_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    AudioStreamInfo audioStreamInfo;
    audioStreamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::MONO;

    InternalAudioCapturerOptions capturerOptions;
    capturerOptions.streamInfo = audioStreamInfo;

    GetServerPtr()->audioPolicyService_.audioConfigManager_.isAdapterInfoMap_.store(false);
    GetServerPtr()->audioPolicyService_.audioConfigManager_.OnUpdateRouteSupport(false);
    int32_t ret = GetServerPtr()->audioPolicyService_.audioCapturerSession_.SetWakeUpAudioCapturer(capturerOptions);
    EXPECT_EQ(ERROR, ret);

    GetServerPtr()->audioPolicyService_.audioConfigManager_.isAdapterInfoMap_.store(true);
    GetServerPtr()->audioPolicyService_.audioConfigManager_.OnUpdateRouteSupport(true);
    ret = GetServerPtr()->audioPolicyService_.audioCapturerSession_.SetWakeUpAudioCapturer(capturerOptions);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test GetDevices and GetDevicesInner.
* @tc.number: GetDevices_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetDevices_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetDevices_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    // case nullptr
    DeviceFlag deviceFlag = OUTPUT_DEVICES_FLAG;
    std::shared_ptr<AudioDeviceDescriptor> ptr = nullptr;
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.push_back(ptr);
    GetServerPtr()->audioPolicyService_.GetDevices(deviceFlag);

    // case deviceType_ is DEVICE_TYPE_REMOTE_CAST
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    std::vector<DeviceFlag> deviceFlagsTmp = {ALL_DEVICES_FLAG, OUTPUT_DEVICES_FLAG, INPUT_DEVICES_FLAG,
        ALL_DISTRIBUTED_DEVICES_FLAG, DISTRIBUTED_OUTPUT_DEVICES_FLAG, DISTRIBUTED_INPUT_DEVICES_FLAG};
    for (const auto& deviceFlag : deviceFlagsTmp) {
        std::vector<DeviceRole> deviceRolesTmp = {OUTPUT_DEVICE, INPUT_DEVICE};
        for (const auto& deviceRole : deviceRolesTmp) {
            audioDeviceDescriptor->deviceRole_ = deviceRole;
            audioDeviceDescriptor->networkId_ = LOCAL_NETWORK_ID;
            GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.push_back(
                audioDeviceDescriptor);
            GetServerPtr()->audioPolicyService_.GetDevices(deviceFlag);
            audioDeviceDescriptor->networkId_ = REMOTE_NETWORK_ID;
            GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.push_back(
                audioDeviceDescriptor);
            GetServerPtr()->audioPolicyService_.GetDevices(deviceFlag);
        }
    }

    // case deviceType_ is not DEVICE_TYPE_REMOTE_CAST
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    for (const auto& deviceFlag : deviceFlagsTmp) {
        std::vector<DeviceRole> deviceRolesTmp = {OUTPUT_DEVICE, INPUT_DEVICE};
        for (const auto& deviceRole : deviceRolesTmp) {
            audioDeviceDescriptor->deviceRole_ = deviceRole;
            audioDeviceDescriptor->networkId_ = LOCAL_NETWORK_ID;
            GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.push_back(
                audioDeviceDescriptor);
            GetServerPtr()->audioPolicyService_.GetDevices(deviceFlag);
            audioDeviceDescriptor->networkId_ = REMOTE_NETWORK_ID;
            GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.push_back(
                audioDeviceDescriptor);
            GetServerPtr()->audioPolicyService_.GetDevices(deviceFlag);
        }
    }
}

/**
* @tc.name  : Test GetPreferredOutputDeviceDescriptors.
* @tc.number: GetPreferredOutputDeviceDescriptors_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetPreferredOutputDeviceDescriptors_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetPreferredOutputDeviceDescriptors_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    EXPECT_NO_THROW(
        AudioRendererInfo rendererInfo;
        rendererInfo.streamUsage = STREAM_USAGE_INVALID;
        string networkId = REMOTE_NETWORK_ID;
        GetServerPtr()->audioPolicyService_.GetPreferredOutputDeviceDescriptors(rendererInfo, networkId);

        rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
        GetServerPtr()->audioPolicyService_.GetPreferredOutputDeviceDescriptors(rendererInfo, networkId);
    );
}

/**
* @tc.name  : Test GetDeviceRole.
* @tc.number: GetDeviceRole_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetDeviceRole_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetDeviceRole_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    DeviceRole ret = AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_WIRED_HEADSET);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_WIRED_HEADPHONES);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_USB_HEADSET);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_DP);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_USB_ARM_HEADSET);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_REMOTE_CAST);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_MIC);
    EXPECT_EQ(INPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_WAKEUP);
    EXPECT_EQ(INPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(DEVICE_TYPE_NONE);
    EXPECT_EQ(DEVICE_ROLE_NONE, ret);
}

/**
* @tc.name  : Test GetDeviceRole.
* @tc.number: GetDeviceRole_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetDeviceRole_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetDeviceRole_002 start");
    ASSERT_NE(nullptr, GetServerPtr());
    DeviceRole ret = AudioPolicyUtils::GetInstance().GetDeviceRole(ROLE_SINK);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(ROLE_SOURCE);
    EXPECT_EQ(INPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole("none");
    EXPECT_EQ(DEVICE_ROLE_NONE, ret);
}

/**
* @tc.name  : Test GetDeviceRole.
* @tc.number: GetDeviceRole_003
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetDeviceRole_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetDeviceRole_003 start");
    ASSERT_NE(nullptr, GetServerPtr());
    DeviceRole ret = AudioPolicyUtils::GetInstance().GetDeviceRole(AUDIO_PIN_NONE);
    EXPECT_EQ(DEVICE_ROLE_NONE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(AUDIO_PIN_OUT_SPEAKER);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(AUDIO_PIN_OUT_HEADSET);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(AUDIO_PIN_OUT_LINEOUT);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(AUDIO_PIN_OUT_HDMI);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(AUDIO_PIN_OUT_USB);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(AUDIO_PIN_OUT_USB_EXT);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(AUDIO_PIN_OUT_DAUDIO_DEFAULT);
    EXPECT_EQ(OUTPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(AUDIO_PIN_IN_MIC);
    EXPECT_EQ(INPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(AUDIO_PIN_IN_HS_MIC);
    EXPECT_EQ(INPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(AUDIO_PIN_IN_LINEIN);
    EXPECT_EQ(INPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(AUDIO_PIN_IN_USB_EXT);
    EXPECT_EQ(INPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(AUDIO_PIN_IN_DAUDIO_DEFAULT);
    EXPECT_EQ(INPUT_DEVICE, ret);

    ret = AudioPolicyUtils::GetInstance().GetDeviceRole(static_cast<AudioPin>(AUDIO_PIN_IN_DAUDIO_DEFAULT + 1));
    EXPECT_EQ(DEVICE_ROLE_NONE, ret);
}

/**
* @tc.name  : Test GetAudioScene.
* @tc.number: GetAudioScene_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetAudioScene_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetAudioScene_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    bool hasSystemPermission = true;
    AudioScene ret = AUDIO_SCENE_INVALID;
    GetServerPtr()->audioPolicyService_.audioSceneManager_.audioScene_ = AUDIO_SCENE_RINGING;
    ret = GetServerPtr()->audioPolicyService_.GetAudioScene(hasSystemPermission);
    EXPECT_EQ(AUDIO_SCENE_RINGING, ret);

    hasSystemPermission = false;
    GetServerPtr()->audioPolicyService_.audioSceneManager_.audioScene_ = AUDIO_SCENE_CALL_START;
    ret = GetServerPtr()->audioPolicyService_.GetAudioScene(hasSystemPermission);
    EXPECT_EQ(AUDIO_SCENE_DEFAULT, ret);

    GetServerPtr()->audioPolicyService_.audioSceneManager_.audioScene_ = AUDIO_SCENE_CALL_END;
    ret = GetServerPtr()->audioPolicyService_.GetAudioScene(hasSystemPermission);
    EXPECT_EQ(AUDIO_SCENE_DEFAULT, ret);

    GetServerPtr()->audioPolicyService_.audioSceneManager_.audioScene_ = AUDIO_SCENE_PHONE_CHAT;
    ret = GetServerPtr()->audioPolicyService_.GetAudioScene(hasSystemPermission);
    EXPECT_EQ(AUDIO_SCENE_PHONE_CHAT, ret);
}

/**
* @tc.name  : Test UpdateEffectDefaultSink.
* @tc.number: UpdateEffectDefaultSink_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, UpdateEffectDefaultSink_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest UpdateEffectDefaultSink_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    EXPECT_NO_THROW(
        AudioPolicyUtils::GetInstance().UpdateEffectDefaultSink(DEVICE_TYPE_EARPIECE);
        AudioPolicyUtils::GetInstance().UpdateEffectDefaultSink(DEVICE_TYPE_NONE);
    );
}

/**
* @tc.name  : Test HasLowLatencyCapability.
* @tc.number: HasLowLatencyCapability_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, HasLowLatencyCapability_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest HasLowLatencyCapability_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    bool ret = false;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.HasLowLatencyCapability(DEVICE_TYPE_NONE, true);
    EXPECT_EQ(true, ret);

    ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.HasLowLatencyCapability(DEVICE_TYPE_EARPIECE, false);
    EXPECT_EQ(true, ret);

    ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.HasLowLatencyCapability(DEVICE_TYPE_SPEAKER, false);
    EXPECT_EQ(true, ret);

    ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.HasLowLatencyCapability(DEVICE_TYPE_WIRED_HEADSET,
        false);
    EXPECT_EQ(true, ret);

    ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.HasLowLatencyCapability(DEVICE_TYPE_WIRED_HEADPHONES,
        false);
    EXPECT_EQ(true, ret);

    ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.HasLowLatencyCapability(DEVICE_TYPE_USB_HEADSET,
        false);
    EXPECT_EQ(true, ret);

    ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.HasLowLatencyCapability(DEVICE_TYPE_DP, false);
    EXPECT_EQ(true, ret);

    ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.HasLowLatencyCapability(DEVICE_TYPE_BLUETOOTH_SCO,
        false);
    EXPECT_EQ(false, ret);

    ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.HasLowLatencyCapability(DEVICE_TYPE_BLUETOOTH_A2DP,
        false);
    EXPECT_EQ(false, ret);

    ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.HasLowLatencyCapability(DEVICE_TYPE_INVALID, false);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test HandleActiveDevice.
* @tc.number: HandleActiveDevice_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, HandleActiveDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest HandleActiveDevice start");
    ASSERT_NE(nullptr, GetServerPtr());
    int32_t ret = SUCCESS;
    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_NONE);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_INVALID);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_WIRED_HEADSET);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(
        DEVICE_TYPE_WIRED_HEADPHONES);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_MIC);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_WAKEUP);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_USB_HEADSET);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_DP);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_REMOTE_CAST);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_FILE_SINK);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_FILE_SOURCE);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_EXTERN_CABLE);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_DEFAULT);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_USB_ARM_HEADSET);
    EXPECT_EQ(SUCCESS, ret);

    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_MAX);
    EXPECT_EQ(ERR_OPERATION_FAILED, ret);

    GetServerPtr()->audioPolicyService_.audioConfigManager_.OnUpdateRouteSupport(false);
    ret = GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_->HandleActiveDevice(DEVICE_TYPE_MIC);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test HandleLocalDeviceConnected.
* @tc.number: HandleLocalDeviceConnected_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, HandleLocalDeviceConnected_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest HandleLocalDeviceConnected_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    int32_t ret = SUCCESS;
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_EARPIECE;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_WIRED_HEADPHONES;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_MIC;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_WAKEUP;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_DP;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test HandleLocalDeviceConnected.
* @tc.number: HandleLocalDeviceConnected_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, HandleLocalDeviceConnected_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest HandleLocalDeviceConnected_002 start");
    ASSERT_NE(nullptr, GetServerPtr());
    int32_t ret = SUCCESS;
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_FILE_SINK;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_FILE_SOURCE;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_EXTERN_CABLE;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_DEFAULT;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_NE(ERROR, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_MAX;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_NONE;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_INVALID;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceConnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test HandleLocalDeviceDisconnected.
* @tc.number: HandleLocalDeviceDisconnected_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, HandleLocalDeviceDisconnected_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest HandleLocalDeviceDisconnected_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    int32_t ret = SUCCESS;
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_EARPIECE;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_WIRED_HEADSET;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_WIRED_HEADPHONES;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_MIC;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_WAKEUP;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_DP;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test HandleLocalDeviceDisconnected.
* @tc.number: HandleLocalDeviceDisconnected_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, HandleLocalDeviceDisconnected_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest HandleLocalDeviceDisconnected_002 start");
    ASSERT_NE(nullptr, GetServerPtr());
    int32_t ret = SUCCESS;
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_FILE_SINK;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_FILE_SOURCE;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_EXTERN_CABLE;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_DEFAULT;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_MAX;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_NONE;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);

    updatedDesc.deviceType_ = DEVICE_TYPE_INVALID;
    ret = GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleLocalDeviceDisconnected(updatedDesc);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AddAudioCapturerMicrophoneDescriptor and GetAudioCapturerMicrophoneDescriptors.
* @tc.number: AddAudioCapturerMicrophoneDescriptor_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, AddAudioCapturerMicrophoneDescriptor_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest AddAudioCapturerMicrophoneDescriptor_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    GetServerPtr()->audioPolicyService_.GetAudioCapturerMicrophoneDescriptors(TEST_SESSIONID);
    // clear data
    GetServerPtr()->audioPolicyService_.audioMicrophoneDescriptor_.connectedMicrophones_.clear();

    // call when devType is DEVICE_TYPE_NONE
    GetServerPtr()->audioPolicyService_.audioMicrophoneDescriptor_.AddAudioCapturerMicrophoneDescriptor(
        TEST_SESSIONID, DEVICE_TYPE_NONE);
    GetServerPtr()->audioPolicyService_.GetAudioCapturerMicrophoneDescriptors(TEST_SESSIONID);

    // call when devType is DEVICE_TYPE_MIC and connectedMicrophones_ is empty
    GetServerPtr()->audioPolicyService_.audioMicrophoneDescriptor_.AddAudioCapturerMicrophoneDescriptor(
        TEST_SESSIONID, DEVICE_TYPE_MIC);
    GetServerPtr()->audioPolicyService_.GetAudioCapturerMicrophoneDescriptors(TEST_SESSIONID);

    // dummy data
    sptr<MicrophoneDescriptor> microphoneDescriptor = new(std::nothrow) MicrophoneDescriptor();
    ASSERT_NE(nullptr, microphoneDescriptor) << "microphoneDescriptor is nullptr.";
    microphoneDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    GetServerPtr()->audioPolicyService_.audioMicrophoneDescriptor_.connectedMicrophones_.push_back(
        microphoneDescriptor);

    // call when devType is DEVICE_TYPE_MIC but connectedMicrophones_ is DEVICE_TYPE_BLUETOOTH_A2DP
    GetServerPtr()->audioPolicyService_.audioMicrophoneDescriptor_.AddAudioCapturerMicrophoneDescriptor(
        TEST_SESSIONID, DEVICE_TYPE_MIC);
    GetServerPtr()->audioPolicyService_.GetAudioCapturerMicrophoneDescriptors(TEST_SESSIONID);

    // call when devType is DEVICE_TYPE_BLUETOOTH_A2DP and connectedMicrophones_ is also DEVICE_TYPE_BLUETOOTH_A2DP
    GetServerPtr()->audioPolicyService_.audioMicrophoneDescriptor_.AddAudioCapturerMicrophoneDescriptor(
        TEST_SESSIONID, DEVICE_TYPE_BLUETOOTH_A2DP);
    GetServerPtr()->audioPolicyService_.GetAudioCapturerMicrophoneDescriptors(TEST_SESSIONID);
}

/**
* @tc.name  : Test UpdateAudioCapturerMicrophoneDescriptor.
* @tc.number: UpdateAudioCapturerMicrophoneDescriptor_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, UpdateAudioCapturerMicrophoneDescriptor_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest UpdateAudioCapturerMicrophoneDescriptor_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    // clear data
    GetServerPtr()->audioPolicyService_.audioMicrophoneDescriptor_.connectedMicrophones_.clear();

    // dummy data
    GetServerPtr()->audioPolicyService_.audioMicrophoneDescriptor_.audioCaptureMicrophoneDescriptor_[TEST_SESSIONID] =
        new MicrophoneDescriptor(0, DEVICE_TYPE_MIC);

    sptr<MicrophoneDescriptor> microphoneDescriptor = new(std::nothrow) MicrophoneDescriptor();
    ASSERT_NE(nullptr, microphoneDescriptor) << "microphoneDescriptor is nullptr.";
    microphoneDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    GetServerPtr()->audioPolicyService_.audioMicrophoneDescriptor_.connectedMicrophones_.push_back(
        microphoneDescriptor);

    for (const auto& deviceType : deviceTypes) {
        GetServerPtr()->audioPolicyService_.audioMicrophoneDescriptor_.UpdateAudioCapturerMicrophoneDescriptor(
            deviceType);
    }

    // modify data
    GetServerPtr()->audioPolicyService_.audioMicrophoneDescriptor_.audioCaptureMicrophoneDescriptor_[TEST_SESSIONID] =
    new MicrophoneDescriptor(0, DEVICE_TYPE_BLUETOOTH_A2DP);

    for (const auto& deviceType : deviceTypes) {
        GetServerPtr()->audioPolicyService_.audioMicrophoneDescriptor_.UpdateAudioCapturerMicrophoneDescriptor(
            deviceType);
    }
}

/**
* @tc.name  : Test GetOutputDevice
* @tc.number: GetOutputDevice_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetOutputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetOutputDevice_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    audioRendererFilter->uid = 0;
    audioRendererFilter->rendererInfo.rendererFlags = STREAM_FLAG_FAST;
    GetServerPtr()->audioPolicyService_.GetOutputDevice(audioRendererFilter);

    audioRendererFilter->uid = 1;
    GetServerPtr()->audioPolicyService_.GetOutputDevice(audioRendererFilter);

    audioRendererFilter->uid = -1;
    GetServerPtr()->audioPolicyService_.GetOutputDevice(audioRendererFilter);
}

/**
* @tc.name  : Test GetInputDevice
* @tc.number: GetInputDevice_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetInputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetInputDevice_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = 0;
    GetServerPtr()->audioPolicyService_.GetInputDevice(audioCapturerFilter);

    audioCapturerFilter->uid = 1;
    GetServerPtr()->audioPolicyService_.GetInputDevice(audioCapturerFilter);

    audioCapturerFilter->uid = -1;
    GetServerPtr()->audioPolicyService_.GetInputDevice(audioCapturerFilter);
}

/**
* @tc.name  : Test GetDumpDeviceInfo
* @tc.number: GetDumpDeviceInfo_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetDumpDeviceInfo_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetDumpDeviceInfo_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    std::string dumpString = "";

    std::vector<DeviceFlag> deviceFlagsTmp = {NONE_DEVICES_FLAG, INPUT_DEVICES_FLAG, OUTPUT_DEVICES_FLAG,
    DISTRIBUTED_INPUT_DEVICES_FLAG, DISTRIBUTED_OUTPUT_DEVICES_FLAG, ALL_DEVICES_FLAG};
    for (const auto& deviceFlag : deviceFlagsTmp) {
        GetServerPtr()->audioPolicyDump_.GetDumpDeviceInfo(dumpString, deviceFlag);
    }
}

/**
* @tc.name  : Test OnCapturerSessionAdded.
* @tc.number: OnCapturerSessionAdded_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, OnCapturerSessionAdded_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest OnCapturerSessionAdded_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamInfo.channels = AudioChannel::STEREO;
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_VOICE_CALL;
    sessionInfo.rate = RATE;
    sessionInfo.channels = CHANNELS;

    GetServerPtr()->audioPolicyService_.audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_INVALID;
    GetServerPtr()->audioPolicyService_.OnCapturerSessionAdded(TEST_SESSIONID, sessionInfo, streamInfo);

    // dummy data
    AudioAdapterInfo adapterInfo = {};
    adapterInfo.adapterName_ = "wakeup_input";
    adapterInfo.adaptersupportScene_ = "supportScene";
    std::list<PipeInfo> pipeInfos_;
    PipeInfo pipeInfo = {};
    pipeInfo.name_ = "wakeup_input";
    StreamPropInfo streamPropInfo;
    pipeInfo.streamPropInfos_.push_back(streamPropInfo);
    pipeInfos_.push_back(pipeInfo);
    adapterInfo.pipeInfos_ = pipeInfos_;
    GetServerPtr()->audioPolicyService_.audioConfigManager_.adapterInfoMap_ = {};
    GetServerPtr()->audioPolicyService_.audioConfigManager_.adapterInfoMap_.insert({AdaptersType::TYPE_PRIMARY,
        adapterInfo});

    int32_t ret = SUCCESS;
    GetServerPtr()->audioPolicyService_.audioEcManager_.normalSourceOpened_ = SOURCE_TYPE_VOICE_CALL;
    ret = GetServerPtr()->audioPolicyService_.audioCapturerSession_.OnCapturerSessionAdded(TEST_SESSIONID,
        sessionInfo, streamInfo);
    EXPECT_NE(SUCCESS, ret);

    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();
    sessionInfo.sourceType = SOURCE_TYPE_REMOTE_CAST;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.OnCapturerSessionAdded(TEST_SESSIONID,
        sessionInfo, streamInfo);
    EXPECT_NE(SUCCESS, ret);

    sessionInfo.sourceType = SOURCE_TYPE_WAKEUP;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.OnCapturerSessionAdded(TEST_SESSIONID,
        sessionInfo, streamInfo);
    EXPECT_NE(SUCCESS, ret);

    GetServerPtr()->audioPolicyService_.audioCapturerSession_.sessionIdisRemovedSet_.insert(TEST_SESSIONID);
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.OnCapturerSessionAdded(TEST_SESSIONID,
        sessionInfo, streamInfo);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test OnCapturerSessionRemoved.
* @tc.number: OnCapturerSessionRemoved_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, OnCapturerSessionRemoved_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest OnCapturerSessionRemoved_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    // clear sessionWithSpecialSourceType_ and sessionWithNormalSourceType_
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.sessionWithSpecialSourceType_.clear();
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.sessionWithNormalSourceType_.clear();
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.OnCapturerSessionRemoved(TEST_SESSIONID);

    // dummy sessionWithSpecialSourceType_
    SessionInfo sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_REMOTE_CAST;
    sessionInfo.rate = RATE;
    sessionInfo.channels = CHANNELS;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.sessionWithSpecialSourceType_[TEST_SESSIONID] =
        sessionInfo;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.OnCapturerSessionRemoved(TEST_SESSIONID);

    sessionInfo.sourceType = SOURCE_TYPE_WAKEUP;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.sessionWithSpecialSourceType_[TEST_SESSIONID] =
        sessionInfo;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.OnCapturerSessionRemoved(TEST_SESSIONID);

    // dummy sessionWithNormalSourceType_
    sessionInfo.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.sessionWithNormalSourceType_[TEST_SESSIONID] =
        sessionInfo;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.OnCapturerSessionRemoved(TEST_SESSIONID);

    sessionInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.sessionWithNormalSourceType_[TEST_SESSIONID] =
        sessionInfo;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.OnCapturerSessionRemoved(TEST_SESSIONID);

    sessionInfo.sourceType = SOURCE_TYPE_WAKEUP;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.sessionWithNormalSourceType_[TEST_SESSIONID] =
        sessionInfo;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.OnCapturerSessionRemoved(TEST_SESSIONID);

    sessionInfo.sourceType = SOURCE_TYPE_WAKEUP;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.sessionWithNormalSourceType_[TEST_SESSIONID] =
        sessionInfo;
    sessionInfo.sourceType = SOURCE_TYPE_MIC;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.sessionWithNormalSourceType_[TEST_SESSIONID + 1] =
        sessionInfo;
    GetServerPtr()->audioPolicyService_.audioCapturerSession_.OnCapturerSessionRemoved(TEST_SESSIONID);
}

/**
* @tc.name  : Test LoadInnerCapturerSink and GetSampleFormatValue.
* @tc.number: LoadInnerCapturerSink_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, LoadInnerCapturerSink_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest LoadInnerCapturerSink_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    AudioStreamInfo streamInfo;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamInfo.channels = AudioChannel::STEREO;
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;

    std::string moduleName = INNER_CAPTURER_SINK_LEGACY;
    for (const auto& audioSampleFormat : audioSampleFormats) {
        streamInfo.format = audioSampleFormat;
        GetServerPtr()->audioPolicyService_.audioCapturerSession_.LoadInnerCapturerSink(moduleName, streamInfo);
        GetServerPtr()->audioPolicyService_.audioCapturerSession_.UnloadInnerCapturerSink(moduleName);
    }
}

/**
* @tc.name  : Test GetOffloadStatusDump.
* @tc.number: GetOffloadStatusDump_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetOffloadStatusDump_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetOffloadStatusDump_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    std::string dumpString = "";
    for (const auto& deviceType : deviceTypes) {
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = deviceType;
        GetServerPtr()->audioPolicyDump_.GetOffloadStatusDump(dumpString);
    }
}

/**
* @tc.name  : Test GetCapturerStreamDump.
* @tc.number: GetCapturerStreamDump_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetCapturerStreamDump_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetCapturerStreamDump_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo = make_shared<AudioCapturerChangeInfo>();
    shared_ptr<AudioCapturerChangeInfo> capturerChangeInfo2 = make_shared<AudioCapturerChangeInfo>();

    AudioCapturerInfo capturerInfo;
    capturerInfo.capturerFlags = AudioPolicyDump::STREAM_FLAG_NORMAL;
    capturerChangeInfo->createrUID = 0;
    capturerChangeInfo->capturerInfo = capturerInfo;
    GetServerPtr()->audioPolicyService_.streamCollector_.audioCapturerChangeInfos_.push_back(
        move(capturerChangeInfo));
    std::string dumpString = "";
    GetServerPtr()->audioPolicyDump_.GetCapturerStreamDump(dumpString);
    GetServerPtr()->audioPolicyService_.streamCollector_.audioCapturerChangeInfos_.clear();

    AudioCapturerInfo capturerInfo2;
    capturerInfo2.capturerFlags = STREAM_FLAG_FAST;
    capturerChangeInfo2->createrUID = 0;
    capturerChangeInfo2->capturerInfo = capturerInfo2;
    GetServerPtr()->audioPolicyService_.streamCollector_.audioCapturerChangeInfos_.push_back(
        move(capturerChangeInfo2));
    std::string dumpString2 = "";
    GetServerPtr()->audioPolicyDump_.GetCapturerStreamDump(dumpString2);
    GetServerPtr()->audioPolicyService_.streamCollector_.audioCapturerChangeInfos_.clear();
}

/**
* @tc.name  : Test GetCallStatusDump.
* @tc.number: GetCallStatusDump_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetCallStatusDump_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetCallStatusDump_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    std::string dumpString = "";
    for (const auto& audioScene : audioScenes) {
        GetServerPtr()->audioPolicyService_.audioSceneManager_.audioScene_ = audioScene;
        GetServerPtr()->audioPolicyDump_.GetCallStatusDump(dumpString);
    }
}

/**
* @tc.name  : Test GetRingerModeDump.
* @tc.number: GetRingerModeDump_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetRingerModeDump_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetRingerModeDump_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    std::string dumpString = "";
    GetServerPtr()->audioPolicyDump_.GetRingerModeDump(dumpString);
}

/**
* @tc.name  : Test SetVoiceCallVolume.
* @tc.number: SetVoiceCallVolume_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SetVoiceCallVolume_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SetVoiceCallVolume_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    std::string dumpString = "";
    int32_t volumeLevel = 1;

#ifdef BLUE_YELLOW_DIFF
    GetServerPtr()->audioPolicyService_.audioPolicyManager_.SetVgsVolumeSupported(true);
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.SetVoiceCallVolume(volumeLevel);
    GetServerPtr()->audioPolicyService_.audioPolicyManager_.SetVgsVolumeSupported(false);
#endif
    GetServerPtr()->audioPolicyService_.audioVolumeManager_.SetVoiceCallVolume(volumeLevel);
}

/**
* @tc.name  : Test NotifyUserSelectionEventToBt.
* @tc.number: NotifyUserSelectionEventToBt_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, NotifyUserSelectionEventToBt_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest NotifyUserSelectionEventToBt_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    // call api when descriptor is nullptr
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.NotifyUserSelectionEventToBt(nullptr);

    // dummy data
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    std::vector<DeviceType> deviceTypesTmp = {DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_TYPE_BLUETOOTH_A2DP,
        DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_EARPIECE};

    // call api when descriptor is not nullptr
    for (const auto& deviceType : deviceTypesTmp) {
        audioDeviceDescriptor->deviceType_ = deviceType;
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = deviceType;
        GetServerPtr()->audioPolicyService_.audioActiveDevice_.NotifyUserSelectionEventToBt(audioDeviceDescriptor);
    }
}

/**
* @tc.name  : Test SelectOutputDeviceByFilterInner.
* @tc.number: SelectOutputDeviceByFilterInner_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SelectOutputDeviceByFilterInner_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SelectOutputDeviceByFilterInner_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    ASSERT_NE(nullptr, audioRendererFilter) << "audioRendererFilter is nullptr.";
    audioRendererFilter->uid = getuid();
    audioRendererFilter->rendererInfo.rendererFlags = AudioPolicyDump::STREAM_FLAG_NORMAL;
    audioRendererFilter->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;

    std::vector<DeviceType> deviceTypesTmp = {DEVICE_TYPE_BLUETOOTH_A2DP, DEVICE_TYPE_BLUETOOTH_SCO,
        DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_SPEAKER};
    for (const auto& deviceType : deviceTypesTmp) {
        std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
        ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
        audioDeviceDescriptor->deviceType_ = deviceType;
        audioDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
        audioDeviceDescriptor->connectState_ = VIRTUAL_CONNECTED;
        vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
        deviceDescriptorVector.push_back(audioDeviceDescriptor);
        int32_t result = GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SelectOutputDeviceByFilterInner(
            audioRendererFilter, deviceDescriptorVector);
        EXPECT_EQ(SUCCESS, result);

        // insert audioDeviceDescriptor into audioDeviceManager_.connectedDevices_
        // so isVirtualDevice will be true
        shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(audioDeviceDescriptor);
        GetServerPtr()->audioPolicyService_.audioDeviceManager_.AddConnectedDevices(devDesc);
        result = GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SelectOutputDeviceByFilterInner(
            audioRendererFilter, deviceDescriptorVector);
        EXPECT_EQ(SUCCESS, result);
    }
}

/**
* @tc.name  : Test SelectOutputDeviceByFilterInner.
* @tc.number: SelectOutputDeviceByFilterInner_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SelectOutputDeviceByFilterInner_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SelectOutputDeviceByFilterInner_002 start");
    ASSERT_NE(nullptr, GetServerPtr());

    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    ASSERT_NE(nullptr, audioRendererFilter) << "audioRendererFilter is nullptr.";
    audioRendererFilter->uid = getuid();
    audioRendererFilter->rendererInfo.rendererFlags = AudioPolicyDump::STREAM_FLAG_NORMAL;
    audioRendererFilter->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    deviceDescriptorVector.push_back(audioDeviceDescriptor);

    // dummy audioRendererChangeInfos_
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    ASSERT_NE(nullptr, rendererChangeInfo) << "audioDeviceDescriptor is nullptr.";
    rendererChangeInfo->clientUID = getuid();
    rendererChangeInfo->sessionId = TEST_SESSIONID;
    GetServerPtr()->audioPolicyService_.streamCollector_.audioRendererChangeInfos_.push_back(
        move(rendererChangeInfo));
    int32_t result = GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SelectOutputDeviceByFilterInner(
        audioRendererFilter, deviceDescriptorVector);
    EXPECT_EQ(SUCCESS, result);

    GetServerPtr()->audioPolicyService_.streamCollector_.audioRendererChangeInfos_.clear();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo2 = make_shared<AudioRendererChangeInfo>();
    ASSERT_NE(nullptr, rendererChangeInfo2) << "audioDeviceDescriptor is nullptr.";
    rendererChangeInfo2->clientUID = getuid();
    rendererChangeInfo2->sessionId = 0;
    GetServerPtr()->audioPolicyService_.streamCollector_.audioRendererChangeInfos_.push_back(
        move(rendererChangeInfo2));
    result = GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SelectOutputDeviceByFilterInner(
        audioRendererFilter, deviceDescriptorVector);
    EXPECT_EQ(SUCCESS, result);

    GetServerPtr()->audioPolicyService_.streamCollector_.audioRendererChangeInfos_.clear();
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo3 = make_shared<AudioRendererChangeInfo>();
    ASSERT_NE(nullptr, rendererChangeInfo3) << "audioDeviceDescriptor is nullptr.";
    rendererChangeInfo3->clientUID = getuid() + 1;
    rendererChangeInfo3->sessionId = 0;
    GetServerPtr()->audioPolicyService_.streamCollector_.audioRendererChangeInfos_.push_back(
        move(rendererChangeInfo3));
    result = GetServerPtr()->audioPolicyService_.audioRecoveryDevice_.SelectOutputDeviceByFilterInner(
        audioRendererFilter, deviceDescriptorVector);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name  : Test OnDeviceConfigurationChanged.
* @tc.number: OnDeviceConfigurationChanged_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, OnDeviceConfigurationChanged_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest OnDeviceConfigurationChanged_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    ASSERT_NE(nullptr, audioRendererFilter) << "audioRendererFilter is nullptr.";
    audioRendererFilter->uid = getuid();
    audioRendererFilter->rendererInfo.rendererFlags = AudioPolicyDump::STREAM_FLAG_NORMAL;
    audioRendererFilter->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;

    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.Init(
        GetServerPtr()->audioPolicyService_.audioA2dpOffloadManager_,
        GetServerPtr()->audioPolicyService_.audioPolicyServerHandler_);
    // clear activeBTDevice_ and connectedA2dpDeviceMap_
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_ = "";
    GetServerPtr()->audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.clear();

    // modify currentActiveDevice_.deviceType_
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_
        = DEVICE_TYPE_BLUETOOTH_A2DP;

    // dummy test data
    AudioStreamInfo audioStreamInfo = {};
    audioStreamInfo.samplingRate =  AudioSamplingRate::SAMPLE_RATE_48000;
    audioStreamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamInfo.channels = AudioChannel::STEREO;
    A2dpDeviceConfigInfo configInfo = {audioStreamInfo, true};
    std::string macAddress = "11-22-33-44-55-66";
    std::string deviceName = "deviceName";
    std::vector<DeviceType> deviceTypesTmp = {DEVICE_TYPE_BLUETOOTH_A2DP, DEVICE_TYPE_BLUETOOTH_SCO,
        DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_SPEAKER};

    for (const auto& deviceType : deviceTypesTmp) {
        GetServerPtr()->audioPolicyService_.OnDeviceConfigurationChanged(
            deviceType, macAddress, deviceName, audioStreamInfo);
    }

    // modify activeBTDevice_ and connectedA2dpDeviceMap_
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.activeBTDevice_ = "AA-BB-CC-DD-EE-FF";
    GetServerPtr()->
        audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"AA-BB-CC-DD-EE-FF", configInfo});
    GetServerPtr()->
        audioPolicyService_.audioA2dpDevice_.connectedA2dpDeviceMap_.insert({"A2dpDeviceCommon", {}});
    for (const auto& deviceType : deviceTypesTmp) {
        GetServerPtr()->audioPolicyService_.OnDeviceConfigurationChanged(
            deviceType, macAddress, deviceName, audioStreamInfo);
    }

    // modify macAddress
    macAddress = "AA-BB-CC-DD-EE-FF";
    for (const auto& deviceType : deviceTypesTmp) {
        GetServerPtr()->audioPolicyService_.OnDeviceConfigurationChanged(
            deviceType, macAddress, deviceName, audioStreamInfo);
    }
}

/**
* @tc.name  : Test RemoveDeviceInRouterMap and RemoveDeviceInFastRouterMap.
* @tc.number: GetActiveDeviceStreamInfo_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, RemoveDeviceInRouterMap_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest RemoveDeviceInRouterMap_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    // clear routeMap_
    GetServerPtr()->audioPolicyService_.audioRouteMap_.routerMap_.clear();
    GetServerPtr()->audioPolicyService_.audioRouteMap_.fastRouterMap_.clear();

    // call RemoveDeviceInRouterMap() when map is empty
    std::string networkId = LOCAL_NETWORK_ID;
    GetServerPtr()->audioPolicyService_.audioRouteMap_.RemoveDeviceInRouterMap(networkId);
    GetServerPtr()->audioPolicyService_.audioRouteMap_.RemoveDeviceInFastRouterMap(networkId);

    // dummy data
    GetServerPtr()->audioPolicyService_.audioRouteMap_.routerMap_[ROUTER_MAP_ID1]
        = std::pair(LOCAL_NETWORK_ID, G_UNKNOWN_PID);
    GetServerPtr()->audioPolicyService_.audioRouteMap_.routerMap_[ROUTER_MAP_ID2]
        = std::pair(REMOTE_NETWORK_ID, G_UNKNOWN_PID);
    GetServerPtr()->audioPolicyService_.audioRouteMap_.fastRouterMap_[ROUTER_MAP_ID1]
        = std::pair(LOCAL_NETWORK_ID, INPUT_DEVICE);
    GetServerPtr()->audioPolicyService_.audioRouteMap_.fastRouterMap_[ROUTER_MAP_ID1]
        = std::pair(REMOTE_NETWORK_ID, OUTPUT_DEVICE);

    // call RemoveDeviceInRouterMap() twice using LOCAL_NETWORK_ID
    GetServerPtr()->audioPolicyService_.audioRouteMap_.RemoveDeviceInRouterMap(networkId);
    GetServerPtr()->audioPolicyService_.audioRouteMap_.RemoveDeviceInRouterMap(networkId);
    GetServerPtr()->audioPolicyService_.audioRouteMap_.RemoveDeviceInFastRouterMap(networkId);
    GetServerPtr()->audioPolicyService_.audioRouteMap_.RemoveDeviceInFastRouterMap(networkId);
}

/**
* @tc.name  : Test SetDisplayName.
* @tc.number: SetDisplayName_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SetDisplayName_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SetDisplayName_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    // clear data
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();

    // dummy data
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDeviceDescriptor->displayName_ = "deviceA";
    audioDeviceDescriptor->networkId_ = LOCAL_NETWORK_ID;
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.push_back(audioDeviceDescriptor);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor2 = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor2) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor2->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptor2->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDeviceDescriptor2->displayName_ = "deviceB";
    audioDeviceDescriptor2->networkId_ = REMOTE_NETWORK_ID;
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.push_back(audioDeviceDescriptor2);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor3 = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor3) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor3->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptor3->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDeviceDescriptor3->displayName_ = "deviceC";
    audioDeviceDescriptor3->networkId_ = std::string(REMOTE_NETWORK_ID) + "xx";
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.push_back(audioDeviceDescriptor3);

    bool isLocalDevice = true;
    GetServerPtr()->audioPolicyService_.SetDisplayName("deviceX", isLocalDevice);
    isLocalDevice = false;
    GetServerPtr()->audioPolicyService_.SetDisplayName("deviceY", isLocalDevice);
    GetServerPtr()->audioPolicyService_.SetDisplayName("deviceZ", isLocalDevice);
    GetServerPtr()->audioPolicyService_.SetDmDeviceType(1);
}

/**
* @tc.name  : Test UpdateDisplayName.
* @tc.number: UpdateDisplayName_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, UpdateDisplayName_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest UpdateDisplayName_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDeviceDescriptor->networkId_ = REMOTE_NETWORK_ID;
    AudioPolicyUtils::GetInstance().UpdateDisplayName(audioDeviceDescriptor);

    audioDeviceDescriptor->networkId_ = LOCAL_NETWORK_ID;
    AudioPolicyUtils::GetInstance().UpdateDisplayName(audioDeviceDescriptor);
}

/**
* @tc.name  : Test HandleOfflineDistributedDevice.
* @tc.number: HandleOfflineDistributedDevice_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, HandleOfflineDistributedDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest HandleOfflineDistributedDevice_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    // clear data
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.clear();

    // dummy data
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDeviceDescriptor->networkId_ = REMOTE_NETWORK_ID;
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.push_back(audioDeviceDescriptor);
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleOfflineDistributedDevice();

    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDeviceDescriptor->networkId_ = REMOTE_NETWORK_ID;
    audioDeviceDescriptor->networkId_ = LOCAL_NETWORK_ID;
    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.push_back(audioDeviceDescriptor);
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleOfflineDistributedDevice();

    GetServerPtr()->audioPolicyService_.audioConnectedDevice_.connectedDevices_.push_back(nullptr);
    GetServerPtr()->audioPolicyService_.audioDeviceStatus_.HandleOfflineDistributedDevice();
}

/**
* @tc.name  : Test ResetToSpeaker.
* @tc.number: ResetToSpeaker_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, ResetToSpeaker_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest ResetToSpeaker_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    for (const auto& deviceType : deviceTypes) {
        GetServerPtr()->audioPolicyService_.ResetToSpeaker(deviceType);
    }
}

/**
* @tc.name  : Test OnForcedDeviceSelected.
* @tc.number: OnForcedDeviceSelected_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, OnForcedDeviceSelected_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest OnForcedDeviceSelected_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    std::string macAddress = "";
    GetServerPtr()->audioPolicyService_.OnForcedDeviceSelected(DEVICE_TYPE_SPEAKER, macAddress);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    audioDeviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDeviceDescriptor->macAddress_ = "11-11-11-11-11-11";
    shared_ptr<AudioDeviceDescriptor> devDesc = make_shared<AudioDeviceDescriptor>(audioDeviceDescriptor);
    GetServerPtr()->audioPolicyService_.audioDeviceManager_.AddConnectedDevices(devDesc);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor2 = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor2) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor2->deviceType_ = DEVICE_TYPE_SPEAKER;
    audioDeviceDescriptor2->deviceRole_ = DeviceRole::INPUT_DEVICE;
    audioDeviceDescriptor->macAddress_ = "66-66-66-66-66-66";
    shared_ptr<AudioDeviceDescriptor> devDesc2 = make_shared<AudioDeviceDescriptor>(audioDeviceDescriptor2);
    GetServerPtr()->audioPolicyService_.audioDeviceManager_.AddConnectedDevices(devDesc2);

    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor3 = std::make_shared<AudioDeviceDescriptor>();
    ASSERT_NE(nullptr, audioDeviceDescriptor3) << "audioDeviceDescriptor is nullptr.";
    audioDeviceDescriptor3->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    audioDeviceDescriptor3->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioDeviceDescriptor3->macAddress_ = "22-22-22-22-22-22";
    shared_ptr<AudioDeviceDescriptor> devDesc3 = make_shared<AudioDeviceDescriptor>(audioDeviceDescriptor3);
    GetServerPtr()->audioPolicyService_.audioDeviceManager_.AddConnectedDevices(devDesc3);

    macAddress = "11-11-11-11-11-11";
    for (const auto& deviceType : deviceTypes) {
        GetServerPtr()->audioPolicyService_.OnForcedDeviceSelected(deviceType, macAddress);
    }

    macAddress = "22-22-22-22-22-22";
    for (const auto& deviceType : deviceTypes) {
        GetServerPtr()->audioPolicyService_.OnForcedDeviceSelected(deviceType, macAddress);
    }
}

/**
* @tc.name  : Test SetSystemVolumeLevel.
* @tc.number: SetSystemVolumeLevel_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SetSystemVolumeLevel_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SetSystemVolumeLevel_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    int32_t volumeLevel = 1;
    for (const auto& audioStreamType : audioStreamTypes) {
        GetServerPtr()->audioPolicyService_.SetSystemVolumeLevel(audioStreamType, volumeLevel);
        GetServerPtr()->audioPolicyService_.audioVolumeManager_.SetVoiceCallVolume(volumeLevel);
    }
}

/**
* @tc.name  : Test HandlePowerStateChanged.
* @tc.number: HandlePowerStateChanged_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, HandlePowerStateChanged_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest HandlePowerStateChanged_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    GetServerPtr()->audioPolicyService_.audioOffloadStream_.offloadSessionID_.reset();

    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.networkId_ = REMOTE_NETWORK_ID;
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_REMOTE_CAST;

    PowerMgr::PowerState state = PowerMgr::PowerState::STAND_BY;
    GetServerPtr()->audioPolicyService_.HandlePowerStateChanged(state);

    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.networkId_ = LOCAL_NETWORK_ID;
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_SPEAKER;
    GetServerPtr()->audioPolicyService_.audioOffloadStream_.offloadSessionID_ = TEST_SESSIONID;
    GetServerPtr()->audioPolicyService_.HandlePowerStateChanged(state);
}

/**
* @tc.name  : Test GetSystemVolumeInDb.
* @tc.number: GetSystemVolumeInDb_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetSystemVolumeInDb_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetSystemVolumeInDb_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    int32_t volumeLevel = DEFAULT_VOLUME_LEVEL;
    DeviceType deviceType = DEVICE_TYPE_USB_ARM_HEADSET;
    for (const auto& volumeType : audioStreamTypes) {
        AUDIO_INFO_LOG("GetSystemVolumeInDb_001 volumeType : %{public}d", static_cast<uint32_t>(volumeType));
        GetServerPtr()->GetSystemVolumeInDb(volumeType, volumeLevel, deviceType);
    }

    for (const auto& streamType : audioStreamTypes) {
        int32_t res = GetServerPtr()->SetSystemVolumeLevelLegacy(streamType, volumeLevel);
    }
    GetServerPtr()->volumeApplyToAll_ = true;
    for (const auto& streamType : audioStreamTypes) {
        int32_t res = GetServerPtr()->SetSystemVolumeLevelLegacy(streamType, volumeLevel);
    }
    for (const auto& volumeType : audioStreamTypes) {
        int32_t res = GetServerPtr()->AdjustSystemVolumeByStep(volumeType, VOLUME_UP);
    }
    for (const auto& volumeType : audioStreamTypes) {
        int32_t res = GetServerPtr()->AdjustSystemVolumeByStep(volumeType, VOLUME_DOWN);
    }
    int32_t uid = getuid();
    for (const auto& streamSetState : streamSetStates) {
        int32_t res = GetServerPtr()->UpdateStreamState(uid, streamSetState, STREAM_USAGE_MUSIC);
    }

    AudioVolumeType volumeType = STREAM_MUSIC;
    int32_t maxVolumeLevel = GetServerPtr()->audioPolicyService_.GetMaxVolumeLevel(volumeType);
    int32_t minVolumeLevel = GetServerPtr()->audioPolicyService_.GetMinVolumeLevel(volumeType);
    EXPECT_GE(minVolumeLevel, VALUE_ZERO);
}

/**
* @tc.name  : Test SetRingerMode.
* @tc.number: SetRingerMode_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SetRingerMode_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SetRingerMode_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    int32_t volumeLevel = DEFAULT_VOLUME_LEVEL;
    for (const auto& ringMode : audioRingerModes) {
        int32_t res = GetServerPtr()->SetRingerMode(ringMode);
        EXPECT_EQ(SUCCESS, res);
    }
}

/**
* @tc.name  : Test SetStreamMuteLegacy.
* @tc.number: SetStreamMuteLegacy_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, SetStreamMuteLegacy_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest SetStreamMuteLegacy_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    for (const auto& streamType : audioStreamTypes) {
        int32_t res = GetServerPtr()->SetStreamMuteLegacy(streamType, true);
        EXPECT_EQ(SUCCESS, res);
    }
    for (const auto& streamType : audioStreamTypes) {
        int32_t res = GetServerPtr()->SetStreamMuteLegacy(streamType, false);
        EXPECT_EQ(SUCCESS, res);
    }
}

/**
* @tc.name  : Test GetNetworkIdByGroupId.
* @tc.number: GetNetworkIdByGroupId_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, GetNetworkIdByGroupId_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest GetNetworkIdByGroupId_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    int32_t groupId = 0;
    string networkId = "";
    networkId = LOCAL_NETWORK_ID;
    int32_t res = GetServerPtr()->GetNetworkIdByGroupId(groupId, networkId);
    EXPECT_EQ(ERROR, res);
}

/**
* @tc.name  : Test MoveToNewOutputDevice.
* @tc.number: MoveToNewOutputDevice_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, MoveToNewOutputDevice_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest MoveToNewOutputDevice_001 start");
    ASSERT_NE(nullptr, GetServerPtr());
    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();

    std::vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescs;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceType_ = DEVICE_TYPE_SPEAKER;
    deviceDescs.push_back(deviceDesc);
    std::vector<SinkInput> sinkInputs;
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);

    GetServerPtr()->audioPolicyService_.audioDeviceCommon_.MoveToNewOutputDevice(rendererChangeInfo, deviceDescs,
        sinkInputs, reason);
    EXPECT_EQ(DEVICE_TYPE_SPEAKER, rendererChangeInfo->outputDeviceInfo.deviceType_);
}

/**
* @tc.name  : Test IsFastFromA2dpToA2dp.
* @tc.number: IsFastFromA2dpToA2dp_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, IsFastFromA2dpToA2dp_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest IsFastFromA2dpToA2dp_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);

    bool ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.IsFastFromA2dpToA2dp(audioDeviceDescriptor,
        rendererChangeInfo, reason);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test IsFastFromA2dpToA2dp.
* @tc.number: IsFastFromA2dpToA2dp_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, IsFastFromA2dpToA2dp_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest IsFastFromA2dpToA2dp_002 start");
    ASSERT_NE(nullptr, GetServerPtr());

    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->outputDeviceInfo.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    rendererChangeInfo->rendererInfo.originalFlag = AUDIO_FLAG_MMAP;
    rendererChangeInfo->outputDeviceInfo.deviceId_ = 1;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);

    bool ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.IsFastFromA2dpToA2dp(audioDeviceDescriptor,
        rendererChangeInfo, reason);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test NotifyRecreateDirectStream.
* @tc.number: NotifyRecreateDirectStream_001
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, NotifyRecreateDirectStream_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest NotifyRecreateDirectStream_001 start");
    ASSERT_NE(nullptr, GetServerPtr());

    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);

    bool ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.NotifyRecreateDirectStream(rendererChangeInfo,
        reason);
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test NotifyRecreateDirectStream.
* @tc.number: NotifyRecreateDirectStream_002
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, NotifyRecreateDirectStream_002, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest NotifyRecreateDirectStream_002 start");
    ASSERT_NE(nullptr, GetServerPtr());

    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    rendererChangeInfo->outputDeviceInfo.deviceId_ = DEVICE_TYPE_USB_ARM_HEADSET;
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_WAKEUP;

    bool ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.NotifyRecreateDirectStream(rendererChangeInfo,
        reason);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test NotifyRecreateDirectStream.
* @tc.number: NotifyRecreateDirectStream_003
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, NotifyRecreateDirectStream_003, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest NotifyRecreateDirectStream_003 start");
    ASSERT_NE(nullptr, GetServerPtr());

    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_DIRECT_MUSIC;
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_WAKEUP;

    bool ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.NotifyRecreateDirectStream(rendererChangeInfo,
        reason);
    EXPECT_TRUE(ret);
}

/**
* @tc.name  : Test NotifyRecreateDirectStream.
* @tc.number: NotifyRecreateDirectStream_004
* @tc.desc  : Test AudioPolicyService interfaces.
*/
HWTEST_F(AudioPolicyServiceUnitTest, NotifyRecreateDirectStream_004, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioPolicyServiceUnitTest NotifyRecreateDirectStream_004 start");
    ASSERT_NE(nullptr, GetServerPtr());

    shared_ptr<AudioRendererChangeInfo> rendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    rendererChangeInfo->rendererInfo.pipeType = PIPE_TYPE_UNKNOWN;
    rendererChangeInfo->rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    rendererChangeInfo->rendererInfo.rendererFlags = AUDIO_FLAG_NORMAL;
    rendererChangeInfo->rendererInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererChangeInfo->rendererInfo.format = SAMPLE_S24LE;
    AudioStreamDeviceChangeReasonExt reason(AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE);
    GetServerPtr()->audioPolicyService_.audioActiveDevice_.currentActiveDevice_.deviceType_ = DEVICE_TYPE_USB_HEADSET;

    bool ret = GetServerPtr()->audioPolicyService_.audioDeviceCommon_.NotifyRecreateDirectStream(rendererChangeInfo,
        reason);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test GetDefaultAdapterEnable.
 * @tc.number: GetDefaultAdapterEnable_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceUnitTest, GetDefaultAdapterEnable_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    bool isEnable = true;
    server->audioPolicyService_.audioConfigManager_.OnUpdateDefaultAdapter(isEnable);
    bool ret = server->audioPolicyService_.audioConfigManager_.GetDefaultAdapterEnable();
    EXPECT_EQ(ret, true);

    isEnable = false;
    server->audioPolicyService_.audioConfigManager_.OnUpdateDefaultAdapter(isEnable);
    ret = server->audioPolicyService_.audioConfigManager_.GetDefaultAdapterEnable();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test GetDeviceClassInfo.
 * @tc.number: GetDeviceClassInfo_001
 * @tc.desc  : Test AudioPolicyService interfaces.
 */
HWTEST_F(AudioPolicyServiceUnitTest, GetDeviceClassInfo_001, TestSize.Level1)
{
    auto server = GetServerUtil::GetServerPtr();
    bool ret = server->audioPolicyService_.audioConfigManager_.Init();
    EXPECT_EQ(ret, true);

    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo = {};
    server->audioPolicyService_.audioConfigManager_.GetDeviceClassInfo(deviceClassInfo);
    bool isEnable = server->audioPolicyService_.audioConfigManager_.GetDefaultAdapterEnable();
    for (auto [classType, moduleInfo] : deviceClassInfo) {
        for (auto module : moduleInfo) {
            std::string defaultAdapterEnable = module.defaultAdapterEnable;
            if (isEnable) {
                EXPECT_EQ(defaultAdapterEnable, "1");
            } else {
                EXPECT_EQ(defaultAdapterEnable, "0");
            }
        }
    }
}
} // namespace AudioStandard
} // namespace OHOS
