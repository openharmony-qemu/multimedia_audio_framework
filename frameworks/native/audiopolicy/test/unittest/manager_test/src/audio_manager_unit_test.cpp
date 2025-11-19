/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "audio_manager_unit_test.h"

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_renderer.h"
#include "audio_capturer.h"
#include "audio_stream_manager.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace {
    constexpr uint32_t MIN_DEVICE_COUNT = 2;
    constexpr uint32_t MIN_DEVICE_ID = 1;
    constexpr uint32_t MIN_DEVICE_NUM = 1;
    constexpr int32_t MAX_VOL = 15;
    constexpr int32_t AUDIO_ERR = -3;
    constexpr float DISCOUNT_VOLUME = 0.5;
    constexpr float INVALID_VOLUME = -1.0;
    constexpr float VOLUME_MIN = 0;
    constexpr float VOLUME_MAX = 1.0;
    constexpr uid_t UID_CAR_DISTRIBUTED_ENGINE_SA = 65872;
    static constexpr char CONFIG_FILE[] = "/vendor/etc/audio/audio_policy_config.xml";
    static constexpr char CONFIG_FILE_NEW[] = "/chip_prod/etc/audio/audio_policy_config.xml";
    // "hello world" sha256
    constexpr const char *TEST_NETWORK_ID = "b94d27b9934d3e08a52e52d7da7dabfac484efe37a5380ee9088f7ace2efcde9";
    constexpr const char *TEST_SPLIT_ARGS = "8:4096:1";
    bool g_hasPermission = false;
}

void GetPermission()
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
            .processName = "audio_manager_unit_test",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

void AudioManagerUnitTest::SetUpTestCase(void) {}
void AudioManagerUnitTest::TearDownTestCase(void) {}

void AudioManagerUnitTest::SetUp(void)
{
    GetPermission();
}

void AudioManagerUnitTest::TearDown(void) {}

/**
* @tc.name   : Test GetDevices API
* @tc.number : GetConnectedDevicesList_001
* @tc.desc   : Test GetDevices interface. Returns list of all input and output devices
*/
HWTEST(AudioManagerUnitTest, GetConnectedDevicesList_001, TestSize.Level1)
{
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::ALL_DEVICES_FLAG);
    auto deviceCount = audioDeviceDescriptors.size();
    EXPECT_GE(deviceCount, MIN_DEVICE_COUNT);
}

/**
* @tc.name   : Test GetDevices API
* @tc.number : GetConnectedDevicesList_002
* @tc.desc   : Test GetDevices interface. Returns list of input devices
*/
HWTEST(AudioManagerUnitTest, GetConnectedDevicesList_002, TestSize.Level1)
{
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    auto inputDevice = audioDeviceDescriptors[0];

    EXPECT_EQ(inputDevice->deviceRole_, DeviceRole::INPUT_DEVICE);
    EXPECT_EQ(inputDevice->deviceType_, DeviceType::DEVICE_TYPE_MIC);
    EXPECT_GE(inputDevice->deviceId_, MIN_DEVICE_ID);
    auto audioStreamInfo = inputDevice->GetDeviceStreamInfo();
    EXPECT_THAT(audioStreamInfo.samplingRate, Each(AllOf(Le(SAMPLE_RATE_96000), Ge(SAMPLE_RATE_8000))));
    EXPECT_EQ(audioStreamInfo.encoding, AudioEncodingType::ENCODING_PCM);
    std::set<AudioChannel> channels = audioStreamInfo.GetChannels();
    EXPECT_THAT(channels, Each(AllOf(Le(CHANNEL_8), Ge(MONO))));
}

/**
* @tc.name   : Test GetAudioParameter API
* @tc.number : GetAudioParameter_001
* @tc.desc   : Test GetAudioParameter interface. Returns if app in fastlist
*/
HWTEST(AudioManagerUnitTest, GetAudioParameter_001, TestSize.Level1)
{
    std::string mockBundleName = "Is_Fast_Blocked_For_AppName#com.samples.audio";
    std::string result = AudioSystemManager::GetInstance()->GetAudioParameter(mockBundleName);
    EXPECT_EQ(result, "");
}

/**
* @tc.name   : Test GetDevices API
* @tc.number : GetConnectedDevicesList_003
* @tc.desc   : Test GetDevices interface. Returns list of output devices
*/
HWTEST(AudioManagerUnitTest, GetConnectedDevicesList_003, TestSize.Level1)
{
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);

    for (auto outputDevice : audioDeviceDescriptors) {
        EXPECT_EQ(outputDevice->deviceRole_, DeviceRole::OUTPUT_DEVICE);
        if (outputDevice->deviceType_ != DeviceType::DEVICE_TYPE_SPEAKER) {
            continue;
        }
        EXPECT_GE(outputDevice->deviceId_, MIN_DEVICE_ID);
        auto audioStreamInfo = outputDevice->GetDeviceStreamInfo();
        EXPECT_THAT(audioStreamInfo.samplingRate, Each(AllOf(Le(SAMPLE_RATE_96000), Ge(SAMPLE_RATE_8000))));
        EXPECT_EQ(audioStreamInfo.encoding, AudioEncodingType::ENCODING_PCM);
        std::set<AudioChannel> channels = audioStreamInfo.GetChannels();
        EXPECT_THAT(channels, Each(AllOf(Le(CHANNEL_8), Ge(MONO))));
    }
}

#ifdef TEMP_DISABLE
/**
* @tc.name    : Test SelectOutputDevice API
* @tc.number  : SelectOutputDevice_001
* @tc.desc    : Test SelectOutputDevice interface.
* @tc.require : issueI5NZAQ
*/
HWTEST(AudioManagerUnitTest, SelectOutputDevice_001, TestSize.Level1)
{
    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;

    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    auto outputDevice =  audioDeviceDescriptors[0];
    outputDevice->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    outputDevice->networkId_ = LOCAL_NETWORK_ID;
    deviceDescriptorVector.push_back(outputDevice);
    auto ret = AudioSystemManager::GetInstance()->SelectOutputDevice(deviceDescriptorVector);
    EXPECT_EQ(SUCCESS, ret);
}
#endif

/**
* @tc.name    : Test SelectOutputDevice API
* @tc.number  : SelectOutputDevice_002
* @tc.desc    : Test SelectOutputDevice interface.
* @tc.require : issueI5NZAQ
*/
HWTEST(AudioManagerUnitTest, SelectOutputDevice_002, TestSize.Level1)
{
    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;

    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    auto outputDevice =  audioDeviceDescriptors[0];
    outputDevice->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    outputDevice->networkId_ = std::string("");
    deviceDescriptorVector.push_back(outputDevice);
    auto ret = AudioSystemManager::GetInstance()->SelectOutputDevice(deviceDescriptorVector);
    EXPECT_TRUE(ret < 0);
}

#ifdef TEMP_DISABLE
/**
* @tc.name    : Test SelectOutputDevice API
* @tc.number  : SelectOutputDevice_003
* @tc.desc    : Test SelectOutputDevice interface.
* @tc.require : issueI5NZAQ
*/
HWTEST(AudioManagerUnitTest, SelectOutputDevice_003, TestSize.Level1)
{
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    audioRendererFilter->uid = 20010041;
    audioRendererFilter->rendererInfo.contentType   = ContentType::CONTENT_TYPE_MUSIC;
    audioRendererFilter->rendererInfo.streamUsage   = StreamUsage::STREAM_USAGE_MEDIA;
    audioRendererFilter->rendererInfo.rendererFlags = 0;
    audioRendererFilter->streamId = 0;

    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;

    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    auto outputDevice =  audioDeviceDescriptors[0];
    outputDevice->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    outputDevice->networkId_ = LOCAL_NETWORK_ID;
    deviceDescriptorVector.push_back(outputDevice);
    auto ret = AudioSystemManager::GetInstance()->SelectOutputDevice(audioRendererFilter, deviceDescriptorVector);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name    : Test SelectOutputDevice API
 * @tc.number  : SelectOutputDevice_004
 * @tc.desc    : Test SelectOutputDevice interface.
 * @tc.require : issueI5NZAQ
 */
HWTEST(AudioManagerUnitTest, SelectOutputDevice_004, TestSize.Level1)
{
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    audioRendererFilter->uid = -1;
    audioRendererFilter->rendererInfo.contentType   = ContentType::CONTENT_TYPE_MUSIC;
    audioRendererFilter->rendererInfo.streamUsage   = StreamUsage::STREAM_USAGE_MEDIA;
    audioRendererFilter->rendererInfo.rendererFlags = 0;
    audioRendererFilter->streamId = 0;

    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;

    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    auto outputDevice =  audioDeviceDescriptors[0];
    outputDevice->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    outputDevice->networkId_ = LOCAL_NETWORK_ID;
    deviceDescriptorVector.push_back(outputDevice);
    auto ret = AudioSystemManager::GetInstance()->SelectOutputDevice(audioRendererFilter, deviceDescriptorVector);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name    : Test SelectOutputDevice API
* @tc.number  : SelectOutputDevice_005
* @tc.desc    : Test SelectOutputDevice interface, set deviceDescriptorVector.size() to zero.
* @tc.require : issueI5NZAQ
*/
HWTEST(AudioManagerUnitTest, SelectOutputDevice_005, TestSize.Level1)
{
    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    auto ret = AudioSystemManager::GetInstance()->SelectOutputDevice(deviceDescriptorVector);
    EXPECT_LT(ret, SUCCESS);
}

/**
* @tc.name    : Test SelectOutputDevice API
* @tc.number  : SelectOutputDevice_006
* @tc.desc    : Test SelectOutputDevice interface, set networkId_ to "".
* @tc.require : issueI5NZAQ
*/
HWTEST(AudioManagerUnitTest, SelectOutputDevice_006, TestSize.Level1)
{
    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;

    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    auto outputDevice =  audioDeviceDescriptors[0];
    outputDevice->deviceRole_ = DeviceRole::INPUT_DEVICE;
    outputDevice->networkId_ = std::string("");
    deviceDescriptorVector.push_back(outputDevice);
    auto ret = AudioSystemManager::GetInstance()->SelectOutputDevice(deviceDescriptorVector);
    EXPECT_LT(ret, SUCCESS);
}

/**
* @tc.name    : Test SelectOutputDevice API
* @tc.number  : SelectOutputDevice_007
* @tc.desc    : Test SelectOutputDevice interface, set audioRendererFilter to nullptr.
* @tc.require : issueI5NZAQ
*/
HWTEST(AudioManagerUnitTest, SelectOutputDevice_007, TestSize.Level1)
{
    sptr<AudioRendererFilter> audioRendererFilter = nullptr;
    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;

    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    auto outputDevice =  audioDeviceDescriptors[0];
    outputDevice->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    outputDevice->networkId_ = LOCAL_NETWORK_ID;
    deviceDescriptorVector.push_back(outputDevice);
    auto ret = AudioSystemManager::GetInstance()->SelectOutputDevice(audioRendererFilter, deviceDescriptorVector);
    EXPECT_LT(ret, SUCCESS);
}

/**
* @tc.name    : Test SelectOutputDevice API
* @tc.number  : SelectOutputDevice_008
* @tc.desc    : Test SelectOutputDevice interface, set audioDeviceDescriptors[0] to nullptr.
* @tc.require : issueI5NZAQ
*/
HWTEST(AudioManagerUnitTest, SelectOutputDevice_008, TestSize.Level1)
{
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    audioRendererFilter->uid = 20010041;
    audioRendererFilter->rendererInfo.contentType   = ContentType::CONTENT_TYPE_MUSIC;
    audioRendererFilter->rendererInfo.streamUsage   = StreamUsage::STREAM_USAGE_MEDIA;
    audioRendererFilter->rendererInfo.rendererFlags = 0;
    audioRendererFilter->streamId = 0;

    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    deviceDescriptorVector.push_back(nullptr);
    auto ret = AudioSystemManager::GetInstance()->SelectOutputDevice(audioRendererFilter, deviceDescriptorVector);
    EXPECT_LT(ret, SUCCESS);
}

/**
* @tc.name    : Test SelectOutputDevice API
* @tc.number  : SelectOutputDevice_009
* @tc.desc    : Test SelectOutputDevice interface, set deviceRole_ to INPUT_DEVICE.
* @tc.require : issueI5NZAQ
*/
HWTEST(AudioManagerUnitTest, SelectOutputDevice_009, TestSize.Level1)
{
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    audioRendererFilter->uid = 20010041;
    audioRendererFilter->rendererInfo.contentType   = ContentType::CONTENT_TYPE_MUSIC;
    audioRendererFilter->rendererInfo.streamUsage   = StreamUsage::STREAM_USAGE_MEDIA;
    audioRendererFilter->rendererInfo.rendererFlags = 0;
    audioRendererFilter->streamId = 0;

    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    auto outputDevice =  audioDeviceDescriptors[0];
    outputDevice->deviceRole_ = DeviceRole::INPUT_DEVICE;
    outputDevice->networkId_ = LOCAL_NETWORK_ID;
    deviceDescriptorVector.push_back(outputDevice);
    auto ret = AudioSystemManager::GetInstance()->SelectOutputDevice(audioRendererFilter, deviceDescriptorVector);
    EXPECT_LT(ret, SUCCESS);
}

/**
* @tc.name    : Test SelectOutputDevice API
* @tc.number  : SelectOutputDevice_010
* @tc.desc    : Test SelectOutputDevice interface, set networkId_ to "".
* @tc.require : issueI5NZAQ
*/
HWTEST(AudioManagerUnitTest, SelectOutputDevice_010, TestSize.Level1)
{
    sptr<AudioRendererFilter> audioRendererFilter = new(std::nothrow) AudioRendererFilter();
    audioRendererFilter->uid = 20010041;
    audioRendererFilter->rendererInfo.contentType   = ContentType::CONTENT_TYPE_MUSIC;
    audioRendererFilter->rendererInfo.streamUsage   = StreamUsage::STREAM_USAGE_MEDIA;
    audioRendererFilter->rendererInfo.rendererFlags = 0;
    audioRendererFilter->streamId = 0;

    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    auto outputDevice =  audioDeviceDescriptors[0];
    outputDevice->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    outputDevice->networkId_ = "";
    deviceDescriptorVector.push_back(outputDevice);
    auto ret = AudioSystemManager::GetInstance()->SelectOutputDevice(audioRendererFilter, deviceDescriptorVector);
    EXPECT_LT(ret, SUCCESS);
}

#ifdef TEMP_DISABLE
/**
* @tc.name   : Test SelectInputDevice API
* @tc.number : SelectInputDevice_001
* @tc.desc   : Test SelectInputDevice interface. deviceRole_ set to INPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, SelectInputDevice_001, TestSize.Level1)
{
    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;

    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    auto inputDevice =  audioDeviceDescriptors[0];
    inputDevice->deviceRole_ = DeviceRole::INPUT_DEVICE;
    inputDevice->networkId_ = LOCAL_NETWORK_ID;
    deviceDescriptorVector.push_back(inputDevice);
    auto ret = AudioSystemManager::GetInstance()->SelectInputDevice(deviceDescriptorVector);
    EXPECT_EQ(SUCCESS, ret);
}
#endif

/**
* @tc.name   : Test SelectInputDevice API
* @tc.number : SelectInputDevice_002
* @tc.desc   : Test SelectInputDevice interface. deviceRole_ set to OUTPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, SelectInputDevice_002, TestSize.Level1)
{
    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;

    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    auto inputDevice =  audioDeviceDescriptors[0];
    inputDevice->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    inputDevice->networkId_ = LOCAL_NETWORK_ID;
    deviceDescriptorVector.push_back(inputDevice);
    auto ret = AudioSystemManager::GetInstance()->SelectInputDevice(deviceDescriptorVector);
    EXPECT_LT(ret, SUCCESS);
}

/**
* @tc.name   : Test SelectInputDevice API
* @tc.number : SelectInputDevice_003
* @tc.desc   : Test SelectInputDevice interface. deviceDescriptorVector[0] set to nullptr
*/
HWTEST(AudioManagerUnitTest, SelectInputDevice_003, TestSize.Level1)
{
    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    auto ret = AudioSystemManager::GetInstance()->SelectInputDevice(deviceDescriptorVector);
    EXPECT_LT(ret, SUCCESS);
}

#ifdef TEMP_DISABLE
/**
* @tc.name   : Test SelectInputDevice API
* @tc.number : SelectInputDevice_004
* @tc.desc   : Test SelectInputDevice interface. normal
*/
HWTEST(AudioManagerUnitTest, SelectInputDevice_004, TestSize.Level1)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = 20010041;

    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    auto inputDevice =  audioDeviceDescriptors[0];
    inputDevice->deviceRole_ = DeviceRole::INPUT_DEVICE;
    inputDevice->networkId_ = LOCAL_NETWORK_ID;
    deviceDescriptorVector.push_back(inputDevice);
    auto ret = AudioSystemManager::GetInstance()->SelectInputDevice(audioCapturerFilter, deviceDescriptorVector);
    EXPECT_EQ(SUCCESS, ret);
}
#endif

/**
* @tc.name   : Test SelectInputDevice API
* @tc.number : SelectInputDevice_005
* @tc.desc   : Test SelectInputDevice interface. audioCapturerFilter set to nullptr
*/
HWTEST(AudioManagerUnitTest, SelectInputDevice_005, TestSize.Level1)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = nullptr;

    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    auto inputDevice =  audioDeviceDescriptors[0];
    inputDevice->deviceRole_ = DeviceRole::INPUT_DEVICE;
    inputDevice->networkId_ = LOCAL_NETWORK_ID;
    deviceDescriptorVector.push_back(inputDevice);
    auto ret = AudioSystemManager::GetInstance()->SelectInputDevice(audioCapturerFilter, deviceDescriptorVector);
    EXPECT_LT(ret, SUCCESS);
}

/**
* @tc.name   : Test SelectInputDevice API
* @tc.number : SelectInputDevice_006
* @tc.desc   : Test SelectInputDevice interface. deviceDescriptorVector.size() set to 0
*/
HWTEST(AudioManagerUnitTest, SelectInputDevice_006, TestSize.Level1)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = 20010041;

    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    auto ret = AudioSystemManager::GetInstance()->SelectInputDevice(audioCapturerFilter, deviceDescriptorVector);
    EXPECT_LT(ret, SUCCESS);
}

/**
* @tc.name   : Test SelectInputDevice API
* @tc.number : SelectInputDevice_007
* @tc.desc   : Test SelectInputDevice interface. deviceDescriptorVector[0] set to nullptr
*/
HWTEST(AudioManagerUnitTest, SelectInputDevice_007, TestSize.Level1)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = 20010041;

    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    deviceDescriptorVector.push_back(nullptr);
    auto ret = AudioSystemManager::GetInstance()->SelectInputDevice(audioCapturerFilter, deviceDescriptorVector);
    EXPECT_LT(ret, SUCCESS);
}

/**
* @tc.name   : Test SelectInputDevice API
* @tc.number : SelectInputDevice_008
* @tc.desc   : Test SelectInputDevice interface. deviceDescriptorVector[0] set to nullptr
*/
HWTEST(AudioManagerUnitTest, SelectInputDevice_008, TestSize.Level1)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = 20010041;

    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    deviceDescriptorVector.push_back(nullptr);
    auto ret = AudioSystemManager::GetInstance()->SelectInputDevice(audioCapturerFilter, deviceDescriptorVector);
    EXPECT_LT(ret, SUCCESS);
}

/**
* @tc.name   : Test SelectInputDevice API
* @tc.number : SelectInputDevice_009
* @tc.desc   : Test SelectInputDevice interface. deviceRole_ set to DeviceRole::OUTPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, SelectInputDevice_009, TestSize.Level1)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = 20010041;

    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    auto inputDevice =  audioDeviceDescriptors[0];
    inputDevice->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    inputDevice->networkId_ = LOCAL_NETWORK_ID;
    deviceDescriptorVector.push_back(inputDevice);
    auto ret = AudioSystemManager::GetInstance()->SelectInputDevice(audioCapturerFilter, deviceDescriptorVector);
    EXPECT_LT(ret, SUCCESS);
}

#ifdef TEMP_DISABLE
/**
* @tc.name   : Test SelectInputDevice API
* @tc.number : SelectInputDevice_010
* @tc.desc   : Test SelectInputDevice interface. uid set to -1
*/
HWTEST(AudioManagerUnitTest, SelectInputDevice_010, TestSize.Level1)
{
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    audioCapturerFilter->uid = -1;

    vector<std::shared_ptr<AudioDeviceDescriptor>> deviceDescriptorVector;
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    auto inputDevice =  audioDeviceDescriptors[0];
    inputDevice->deviceRole_ = DeviceRole::INPUT_DEVICE;
    inputDevice->networkId_ = LOCAL_NETWORK_ID;
    deviceDescriptorVector.push_back(inputDevice);
    auto ret = AudioSystemManager::GetInstance()->SelectInputDevice(audioCapturerFilter, deviceDescriptorVector);
    EXPECT_EQ(ret, SUCCESS);
}
#endif

/**
* @tc.name   : Test GetActiveOutputDeviceDescriptors API
* @tc.number : GetActiveOutputDeviceDescriptors_001
* @tc.desc   : Test GetActiveOutputDeviceDescriptors interface.
*/
HWTEST(AudioManagerUnitTest, GetActiveOutputDeviceDescriptors_001, TestSize.Level1)
{
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetActiveOutputDeviceDescriptors();
    auto ret = audioDeviceDescriptors.size();
    EXPECT_GE(ret, MIN_DEVICE_NUM);
}

/**
* @tc.name   : Test RegisterVolumeKeyEventCallback API
* @tc.number : RegisterVolumeKeyEventCallback_001
* @tc.desc   : Test RegisterVolumeKeyEventCallback interface.
*/
HWTEST(AudioManagerUnitTest, RegisterVolumeKeyEventCallback_001, TestSize.Level1)
{
    int32_t clientPid = 1;
    std::shared_ptr<VolumeKeyEventCallback> callback = nullptr;
    auto ret = AudioSystemManager::GetInstance()->RegisterVolumeKeyEventCallback(clientPid, callback, API_8);
    EXPECT_LT(ret, SUCCESS);
}

/**
* @tc.name   : Test SetAudioManagerCallback API
* @tc.number : SetAudioManagerCallback_001
* @tc.desc   : Test SetAudioManagerCallback interface.
*/
HWTEST(AudioManagerUnitTest, SetAudioManagerCallback_001, TestSize.Level1)
{
    AudioVolumeType streamType = AudioVolumeType::STREAM_VOICE_CALL;
    std::shared_ptr<AudioManagerCallback> callback;
    auto ret = AudioSystemManager::GetInstance()->SetAudioManagerCallback(streamType, callback);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name   : Test UnsetAudioManagerCallback API
* @tc.number : UnsetAudioManagerCallback_001
* @tc.desc   : Test UnsetAudioManagerCallback interface.
*/
HWTEST(AudioManagerUnitTest, UnsetAudioManagerCallback_001, TestSize.Level1)
{
    AudioVolumeType streamType = AudioVolumeType::STREAM_VOICE_CALL;
    auto ret = AudioSystemManager::GetInstance()->UnsetAudioManagerCallback(streamType);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test SetForegroudList API
 * @tc.number : SetForegroudList_001
 * @tc.desc   : Test SetForegroudList_001 interface.
 */
HWTEST(AudioManagerUnitTest, SetForegroudList_001, TestSize.Level1)
{
    std::vector<std::string> list = {};
    auto ret = AudioSystemManager::GetInstance()->SetForegroundList(list);
    EXPECT_EQ(ERR_NOT_SUPPORTED, ret);
}

#ifdef TEMP_DISABLE
/**
* @tc.name   : Test GetStandbyStatus API
* @tc.number : GetStandbyStatus_001
* @tc.desc   : Test GetStandbyStatus_001 interface.
*/
HWTEST(AudioManagerUnitTest, GetStandbyStatus_001, TestSize.Level1)
{
    uint32_t sessionId = 0;
    bool isStandby = false;
    int64_t enterStandbyTime = 0;
    auto ret = AudioSystemManager::GetInstance()->GetStandbyStatus(sessionId, isStandby, enterStandbyTime);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name   : Test GetStandbyStatus API
* @tc.number : GetStandbyStatus_002
* @tc.desc   : Test GetStandbyStatus_002 interface.
*/
HWTEST(AudioManagerUnitTest, GetStandbyStatus_002, TestSize.Level1)
{
    AudioRendererOptions rendererOptions = {};
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_RINGTONE;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_NOTIFICATION_RINGTONE;
    rendererOptions.rendererInfo.rendererFlags = 0;
    unique_ptr<AudioRenderer> renderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, renderer);

    renderer->Start();
    std::unique_ptr<uint8_t[]> tempBuffer = std::make_unique<uint8_t[]>(WRTTE_BUFFER_SIZE);
    renderer->Write(tempBuffer.get(), WRTTE_BUFFER_SIZE);

    uint32_t sessionId = 0;
    renderer->GetAudioStreamId(sessionId);
    bool isStandby = false;
    int64_t enterStandbyTime = 0;
    auto ret = AudioSystemManager::GetInstance()->GetStandbyStatus(sessionId, isStandby, enterStandbyTime);
    ASSERT_EQ(ret, SUCCESS) << "GetStandbyStatus call failed";
    ASSERT_EQ(isStandby, false) << "renderer should not be standby";
}

/**
* @tc.name   : Test GetStandbyStatus API
* @tc.number : GetStandbyStatus_003
* @tc.desc   : Test GetStandbyStatus_003 interface.
*/
HWTEST(AudioManagerUnitTest, GetStandbyStatus_003, TestSize.Level1)
{
    AudioRendererOptions rendererOptions = {};
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_RINGTONE;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_NOTIFICATION_RINGTONE;
    rendererOptions.rendererInfo.rendererFlags = 0;
    unique_ptr<AudioRenderer> renderer = AudioRenderer::Create(rendererOptions);
    EXPECT_NE(renderer, nullptr);

    renderer->Start();
    std::unique_ptr<uint8_t[]> tempBuffer = std::make_unique<uint8_t[]>(WRTTE_BUFFER_SIZE);
    renderer->Write(tempBuffer.get(), WRTTE_BUFFER_SIZE);

    usleep(2000000); // 2000000 for sleep 2s, wait for steam enter standby

    uint32_t sessionId = 0;
    renderer->GetAudioStreamId(sessionId);
    bool isStandby = false;
    int64_t enterStandbyTime = 0;
    auto ret = AudioSystemManager::GetInstance()->GetStandbyStatus(sessionId, isStandby, enterStandbyTime);
    ASSERT_EQ(ret, SUCCESS) << "GetStandbyStatus call failed";
    ASSERT_EQ(isStandby, true) << "renderer should be in standby";
}
#endif

/**
* @tc.name   : Test GenerateSessionId API
* @tc.number : GenerateSessionId_001
* @tc.desc   : Test GenerateSessionId_001 interface.
*/
HWTEST(AudioManagerUnitTest, GenerateSessionId_001, TestSize.Level1)
{
    uint32_t sessionId = 0;
    auto ret = AudioSystemManager::GetInstance()->GenerateSessionId(sessionId);
    EXPECT_EQ(AUDIO_ERR, ret);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_001
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_NONE,
* deviceRole set to DEVICE_ROLE_NONE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_001, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_NONE;
    DeviceRole deviceRole = DeviceRole::DEVICE_ROLE_NONE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_NONE);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_002
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_INVALID,
* deviceRole set to DEVICE_ROLE_NONE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_002, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_INVALID;
    DeviceRole deviceRole = DeviceRole::DEVICE_ROLE_NONE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_NONE);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_003
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_DEFAULT,
* deviceRole set to INPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_003, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_DEFAULT;
    DeviceRole deviceRole = DeviceRole::INPUT_DEVICE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_IN_DAUDIO_DEFAULT);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_004
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_DEFAULT,
* deviceRole set to DEVICE_ROLE_NONE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_004, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_DEFAULT;
    DeviceRole deviceRole = DeviceRole::DEVICE_ROLE_NONE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_OUT_DAUDIO_DEFAULT);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_005
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_SPEAKER,
* deviceRole set to DEVICE_ROLE_NONE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_005, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    DeviceRole deviceRole = DeviceRole::DEVICE_ROLE_NONE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_OUT_SPEAKER);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_006
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_MIC,
* deviceRole set to DEVICE_ROLE_NONE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_006, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_MIC;
    DeviceRole deviceRole = DeviceRole::DEVICE_ROLE_NONE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_IN_MIC);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_007
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_WIRED_HEADSET,
* deviceRole set to INPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_007, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    DeviceRole deviceRole = DeviceRole::INPUT_DEVICE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_IN_HS_MIC);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_008
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_WIRED_HEADSET,
* deviceRole set to OUTPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_008, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_WIRED_HEADSET;
    DeviceRole deviceRole = DeviceRole::OUTPUT_DEVICE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_OUT_HEADSET);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_009
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_USB_HEADSET,
* deviceRole set to OUTPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_009, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_USB_HEADSET;
    DeviceRole deviceRole = DeviceRole::OUTPUT_DEVICE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_OUT_USB_HEADSET);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_010
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_FILE_SINK,
* deviceRole set to OUTPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_010, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_FILE_SINK;
    DeviceRole deviceRole = DeviceRole::OUTPUT_DEVICE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_NONE);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_011
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_FILE_SOURCE,
* deviceRole set to OUTPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_011, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_FILE_SOURCE;
    DeviceRole deviceRole = DeviceRole::OUTPUT_DEVICE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_NONE);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_012
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_BLUETOOTH_SCO,
* deviceRole set to OUTPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_012, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_SCO;
    DeviceRole deviceRole = DeviceRole::OUTPUT_DEVICE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_NONE);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_013
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_BLUETOOTH_A2DP,
* deviceRole set to OUTPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_013, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    DeviceRole deviceRole = DeviceRole::OUTPUT_DEVICE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_NONE);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_014
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_MAX,
* deviceRole set to OUTPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_014, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_MAX;
    DeviceRole deviceRole = DeviceRole::OUTPUT_DEVICE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_NONE);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_015
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_DEFAULT,
* deviceRole set to OUTPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_015, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_DEFAULT;
    DeviceRole deviceRole = DeviceRole::OUTPUT_DEVICE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_OUT_DAUDIO_DEFAULT);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_016
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to DEVICE_TYPE_USB_HEADSET,
* deviceRole set to INPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_016, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_USB_HEADSET;
    DeviceRole deviceRole = DeviceRole::INPUT_DEVICE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AudioPin::AUDIO_PIN_IN_USB_HEADSET);
}

/**
* @tc.name   : Test GetPinValueFromType API
* @tc.number : GetPinValueFromType_017
* @tc.desc   : Test GetPinValueFromType interface. deviceType set to AUDIO_PIN_OUT_DP,
* deviceRole set to INPUT_DEVICE
*/
HWTEST(AudioManagerUnitTest, GetPinValueFromType_017, TestSize.Level1)
{
    DeviceType deviceType = DeviceType::DEVICE_TYPE_DP;
    DeviceRole deviceRole = DeviceRole::INPUT_DEVICE;
    AudioPin ret = AudioSystemManager::GetInstance()->GetPinValueFromType(deviceType, deviceRole);
    EXPECT_EQ(ret, AUDIO_PIN_OUT_DP);
}

/**
* @tc.name   : Test GetTypeValueFromPin API
* @tc.number : GetTypeValueFromPin_001
* @tc.desc   : Test GetTypeValueFromPin interface. pin set to AUDIO_PIN_NONE
*/
HWTEST(AudioManagerUnitTest, GetTypeValueFromPin_001, TestSize.Level1)
{
    AudioPin pin = AudioPin::AUDIO_PIN_NONE;
    DeviceType ret = AudioSystemManager::GetInstance()->GetTypeValueFromPin(pin);
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_NONE);
}

/**
* @tc.name   : Test GetTypeValueFromPin API
* @tc.number : GetTypeValueFromPin_002
* @tc.desc   : Test GetTypeValueFromPin interface. pin set to AUDIO_PIN_OUT_SPEAKER
*/
HWTEST(AudioManagerUnitTest, GetTypeValueFromPin_002, TestSize.Level1)
{
    AudioPin pin = AudioPin::AUDIO_PIN_OUT_SPEAKER;
    DeviceType ret = AudioSystemManager::GetInstance()->GetTypeValueFromPin(pin);
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_SPEAKER);
}

/**
* @tc.name   : Test GetTypeValueFromPin API
* @tc.number : GetTypeValueFromPin_003
* @tc.desc   : Test GetTypeValueFromPin interface. pin set to AUDIO_PIN_OUT_HEADSET
*/
HWTEST(AudioManagerUnitTest, GetTypeValueFromPin_003, TestSize.Level1)
{
    AudioPin pin = AudioPin::AUDIO_PIN_OUT_HEADSET;
    DeviceType ret = AudioSystemManager::GetInstance()->GetTypeValueFromPin(pin);
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_NONE);
}

/**
* @tc.name   : Test GetTypeValueFromPin API
* @tc.number : GetTypeValueFromPin_004
* @tc.desc   : Test GetTypeValueFromPin interface. pin set to AUDIO_PIN_OUT_LINEOUT
*/
HWTEST(AudioManagerUnitTest, GetTypeValueFromPin_004, TestSize.Level1)
{
    AudioPin pin = AudioPin::AUDIO_PIN_OUT_LINEOUT;
    DeviceType ret = AudioSystemManager::GetInstance()->GetTypeValueFromPin(pin);
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_NONE);
}

/**
* @tc.name   : Test GetTypeValueFromPin API
* @tc.number : GetTypeValueFromPin_005
* @tc.desc   : Test GetTypeValueFromPin interface. pin set to AUDIO_PIN_OUT_HDMI
*/
HWTEST(AudioManagerUnitTest, GetTypeValueFromPin_005, TestSize.Level1)
{
    AudioPin pin = AudioPin::AUDIO_PIN_OUT_HDMI;
    DeviceType ret = AudioSystemManager::GetInstance()->GetTypeValueFromPin(pin);
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_NONE);
}

/**
* @tc.name   : Test GetTypeValueFromPin API
* @tc.number : GetTypeValueFromPin_006
* @tc.desc   : Test GetTypeValueFromPin interface. pin set to AUDIO_PIN_OUT_USB
*/
HWTEST(AudioManagerUnitTest, GetTypeValueFromPin_006, TestSize.Level1)
{
    AudioPin pin = AudioPin::AUDIO_PIN_OUT_USB;
    DeviceType ret = AudioSystemManager::GetInstance()->GetTypeValueFromPin(pin);
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_NONE);
}

/**
* @tc.name   : Test GetTypeValueFromPin API
* @tc.number : GetTypeValueFromPin_007
* @tc.desc   : Test GetTypeValueFromPin interface. pin set to AUDIO_PIN_OUT_USB_EXT
*/
HWTEST(AudioManagerUnitTest, GetTypeValueFromPin_007, TestSize.Level1)
{
    AudioPin pin = AudioPin::AUDIO_PIN_OUT_USB_EXT;
    DeviceType ret = AudioSystemManager::GetInstance()->GetTypeValueFromPin(pin);
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_NONE);
}

/**
* @tc.name   : Test GetTypeValueFromPin API
* @tc.number : GetTypeValueFromPin_008
* @tc.desc   : Test GetTypeValueFromPin interface. pin set to AUDIO_PIN_OUT_DAUDIO_DEFAULT
*/
HWTEST(AudioManagerUnitTest, GetTypeValueFromPin_008, TestSize.Level1)
{
    AudioPin pin = AudioPin::AUDIO_PIN_OUT_DAUDIO_DEFAULT;
    DeviceType ret = AudioSystemManager::GetInstance()->GetTypeValueFromPin(pin);
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_DEFAULT);
}

/**
* @tc.name   : Test GetTypeValueFromPin API
* @tc.number : GetTypeValueFromPin_009
* @tc.desc   : Test GetTypeValueFromPin interface. pin set to AUDIO_PIN_IN_MIC
*/
HWTEST(AudioManagerUnitTest, GetTypeValueFromPin_009, TestSize.Level1)
{
    AudioPin pin = AudioPin::AUDIO_PIN_IN_MIC;
    DeviceType ret = AudioSystemManager::GetInstance()->GetTypeValueFromPin(pin);
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_MIC);
}

/**
* @tc.name   : Test GetTypeValueFromPin API
* @tc.number : GetTypeValueFromPin_010
* @tc.desc   : Test GetTypeValueFromPin interface. pin set to AUDIO_PIN_IN_HS_MIC
*/
HWTEST(AudioManagerUnitTest, GetTypeValueFromPin_010, TestSize.Level1)
{
    AudioPin pin = AudioPin::AUDIO_PIN_IN_HS_MIC;
    DeviceType ret = AudioSystemManager::GetInstance()->GetTypeValueFromPin(pin);
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_WIRED_HEADSET);
}

/**
* @tc.name   : Test GetTypeValueFromPin API
* @tc.number : GetTypeValueFromPin_011
* @tc.desc   : Test GetTypeValueFromPin interface. pin set to AUDIO_PIN_IN_LINEIN
*/
HWTEST(AudioManagerUnitTest, GetTypeValueFromPin_011, TestSize.Level1)
{
    AudioPin pin = AudioPin::AUDIO_PIN_IN_LINEIN;
    DeviceType ret = AudioSystemManager::GetInstance()->GetTypeValueFromPin(pin);
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_NONE);
}

/**
* @tc.name   : Test GetTypeValueFromPin API
* @tc.number : GetTypeValueFromPin_012
* @tc.desc   : Test GetTypeValueFromPin interface. pin set to AUDIO_PIN_IN_USB_EXT
*/
HWTEST(AudioManagerUnitTest, GetTypeValueFromPin_012, TestSize.Level1)
{
    AudioPin pin = AudioPin::AUDIO_PIN_IN_USB_EXT;
    DeviceType ret = AudioSystemManager::GetInstance()->GetTypeValueFromPin(pin);
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_NONE);
}

/**
* @tc.name   : Test GetTypeValueFromPin API
* @tc.number : GetTypeValueFromPin_013
* @tc.desc   : Test GetTypeValueFromPin interface. pin set to AUDIO_PIN_IN_DAUDIO_DEFAULT
*/
HWTEST(AudioManagerUnitTest, GetTypeValueFromPin_013, TestSize.Level1)
{
    AudioPin pin = AudioPin::AUDIO_PIN_IN_DAUDIO_DEFAULT;
    DeviceType ret = AudioSystemManager::GetInstance()->GetTypeValueFromPin(pin);
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_DEFAULT);
}

/**
* @tc.name   : Test GetTypeValueFromPin API
* @tc.number : GetTypeValueFromPin_014
* @tc.desc   : Test GetTypeValueFromPin interface. pin set to INVALID data
*/
HWTEST(AudioManagerUnitTest, GetTypeValueFromPin_014, TestSize.Level1)
{
    int32_t invalid_value = 1000;
    AudioPin pin = AudioPin(invalid_value);
    DeviceType ret = AudioSystemManager::GetInstance()->GetTypeValueFromPin(pin);
    EXPECT_EQ(ret, DeviceType::DEVICE_TYPE_NONE);
}

/**
* @tc.name   : Test SetDeviceActive API
* @tc.number : SetDeviceActive_001
* @tc.desc   : Test SetDeviceActive interface. Activate bluetooth sco device by deactivating speaker
*/
HWTEST(AudioManagerUnitTest, SetDeviceActive_001, TestSize.Level1)
{
    auto isActive = AudioSystemManager::GetInstance()->IsDeviceActive(DeviceType::DEVICE_TYPE_SPEAKER);
    EXPECT_TRUE(isActive);
}

/**
* @tc.name   : Test SetDeviceActive API
* @tc.number : SetDeviceActive_002
* @tc.desc   : Test SetDeviceActive interface. Speaker should not be disable since its the only active device
*/
HWTEST(AudioManagerUnitTest, SetDeviceActive_002, TestSize.Level1)
{
    auto ret = AudioSystemManager::GetInstance()->SetDeviceActive(DeviceType::DEVICE_TYPE_SPEAKER, false);
    EXPECT_EQ(SUCCESS, ret);

    auto isActive = AudioSystemManager::GetInstance()->IsDeviceActive(DeviceType::DEVICE_TYPE_SPEAKER);
    EXPECT_TRUE(isActive);
}

/**
* @tc.name   : Test SetDeviceActive API
* @tc.number : SetDeviceActive_003
* @tc.desc   : Test SetDeviceActive interface. Actiavting invalid device should fail
*/
HWTEST(AudioManagerUnitTest, SetDeviceActive_003, TestSize.Level1)
{
    auto ret = AudioSystemManager::GetInstance()->SetDeviceActive(DeviceType::DEVICE_TYPE_NONE, true);
    EXPECT_NE(SUCCESS, ret);

    // On bootup sco won't be connected. Hence activation should fail
    ret = AudioSystemManager::GetInstance()->SetDeviceActive(DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, true);
    EXPECT_NE(SUCCESS, ret);

    auto isActive = AudioSystemManager::GetInstance()->IsDeviceActive(DeviceType::DEVICE_TYPE_SPEAKER);
    EXPECT_TRUE(isActive);
}

/**
* @tc.name   : Test IsStreamActive API
* @tc.number : IsStreamActive_001
* @tc.desc   : Test IsStreamActive interface. set AudioVolumeType return true
*/
HWTEST(AudioManagerUnitTest, IsStreamActive_001, TestSize.Level1)
{
    auto isActive = AudioSystemManager::GetInstance()->IsStreamActive(AudioVolumeType::STREAM_MUSIC);
    EXPECT_FALSE(isActive);
    isActive = AudioSystemManager::GetInstance()->IsStreamActive(AudioVolumeType::STREAM_RING);
    EXPECT_FALSE(isActive);
    isActive = AudioSystemManager::GetInstance()->IsStreamActive(AudioVolumeType::STREAM_VOICE_CALL);
    EXPECT_FALSE(isActive);
    isActive = AudioSystemManager::GetInstance()->IsStreamActive(AudioVolumeType::STREAM_VOICE_ASSISTANT);
    EXPECT_FALSE(isActive);
    isActive = AudioSystemManager::GetInstance()->IsStreamActive(AudioVolumeType::STREAM_ULTRASONIC);
    EXPECT_FALSE(isActive);
    isActive = AudioSystemManager::GetInstance()->IsStreamActive(AudioVolumeType::STREAM_ALL);
    EXPECT_FALSE(isActive);
}

/**
* @tc.name   : Test IsStreamActive API
* @tc.number : IsStreamActive_002
* @tc.desc   : Test IsStreamActive interface. set AudioVolumeType return false
*/
HWTEST(AudioManagerUnitTest, IsStreamActive_002, TestSize.Level1)
{
    auto isActive = AudioSystemManager::GetInstance()->IsStreamActive(AudioVolumeType::STREAM_DEFAULT);
    EXPECT_FALSE(isActive);
}

/**
* @tc.name   : Test IsStreamMute API
* @tc.number : IsStreamMute_001
* @tc.desc   : Test IsStreamMute interface. set AudioVolumeType return false
*/
HWTEST(AudioManagerUnitTest, IsStreamMute_001, TestSize.Level1)
{
    auto isActive = AudioSystemManager::GetInstance()->IsStreamMute(AudioVolumeType::STREAM_DEFAULT);
    EXPECT_FALSE(isActive);
}

#ifdef TEMP_DISABLE
/**
* @tc.name   : Test AudioVolume API
* @tc.number : AudioVolume_001
* @tc.desc   : Test AudioVolume manager interface multiple requests
*/
HWTEST(AudioManagerUnitTest, AudioVolume_001, TestSize.Level1)
{
    int32_t volume = 10;
    bool mute = true;

    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    ASSERT_NE(nullptr, audioRenderer);

    auto ret = AudioSystemManager::GetInstance()->SetVolume(AudioVolumeType::STREAM_ALL, volume);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->GetVolume(AudioVolumeType::STREAM_ALL);
    EXPECT_EQ(volume, ret);
    ret = AudioSystemManager::GetInstance()->SetMute(AudioVolumeType::STREAM_ALL, mute);
    EXPECT_EQ(SUCCESS, ret);
    ret = AudioSystemManager::GetInstance()->IsStreamMute(AudioVolumeType::STREAM_ALL);
    EXPECT_EQ(true, ret);

    audioRenderer->Release();
}
#endif

/**
* @tc.name   : Test SetVolume API
* @tc.number : SetVolumeTest_001
* @tc.desc   : Test setting volume of ringtone stream with max volume
*/
HWTEST(AudioManagerUnitTest, SetVolumeTest_001, TestSize.Level1)
{
    auto ret = AudioSystemManager::GetInstance()->SetVolume(AudioVolumeType::STREAM_RING, MAX_VOL);
    EXPECT_EQ(SUCCESS, ret);

    int32_t volume = AudioSystemManager::GetInstance()->GetVolume(AudioVolumeType::STREAM_RING);
    EXPECT_EQ(MAX_VOL, volume);
}

#ifdef TEMP_DISABLE
/**
* @tc.name   : Test SetVolume API
* @tc.number : SetVolumeTest_002
* @tc.desc   : Test setting volume of ringtone stream with min volume
*/
HWTEST(AudioManagerUnitTest, SetVolumeTest_002, TestSize.Level1)
{
    auto ret = AudioSystemManager::GetInstance()->SetVolume(AudioVolumeType::STREAM_RING, MIN_VOL);
    EXPECT_EQ(SUCCESS, ret);

    int32_t volume = AudioSystemManager::GetInstance()->GetVolume(AudioVolumeType::STREAM_RING);
    EXPECT_EQ(MIN_VOL, volume);
}

/**
* @tc.name   : Test SetVolume API
* @tc.number : SetVolumeTest_003
* @tc.desc   : Test setting volume of media stream with max volume
*/
HWTEST(AudioManagerUnitTest, SetVolumeTest_003, TestSize.Level1)
{
    auto ret = AudioSystemManager::GetInstance()->SetVolume(AudioVolumeType::STREAM_MUSIC, MAX_VOL);
    EXPECT_EQ(SUCCESS, ret);

    int32_t mediaVol = AudioSystemManager::GetInstance()->GetVolume(AudioVolumeType::STREAM_MUSIC);
    EXPECT_EQ(MAX_VOL, mediaVol);

    int32_t ringVolume = AudioSystemManager::GetInstance()->GetVolume(AudioVolumeType::STREAM_RING);
    EXPECT_EQ(MIN_VOL, ringVolume);
}
#endif

/**
* @tc.name   : Test SetVolume API
* @tc.number : SetVolumeTest_004
* @tc.desc   : Test setting volume of default stream with max volume
*/
HWTEST(AudioManagerUnitTest, SetVolumeTest_004, TestSize.Level1)
{
    auto ret = AudioSystemManager::GetInstance()->SetVolume(AudioVolumeType::STREAM_DEFAULT, MAX_VOL);
    EXPECT_LT(ret, SUCCESS);
    int32_t mediaVol = AudioSystemManager::GetInstance()->GetVolume(AudioVolumeType::STREAM_DEFAULT);
    EXPECT_LT(mediaVol, SUCCESS);
}

/**
* @tc.name   : Test SetRingerModeCallbak API
* @tc.number : SetRingerModeCallbak_001
* @tc.desc   : Test setting of callback to nullptr
*/
HWTEST(AudioManagerUnitTest, SetRingerModeCallbak_001, TestSize.Level1)
{
    int32_t clientId = 1;
    std::shared_ptr<AudioRingerModeCallback> callback = nullptr;
    auto ret = AudioSystemManager::GetInstance()->SetRingerModeCallback(clientId, callback);
    EXPECT_LT(ret, SUCCESS);
}

/**
* @tc.name   : Test SetRingerMode API
* @tc.number : SetRingerModeTest_001
* @tc.desc   : Test setting of ringer mode to SILENT
*/
HWTEST(AudioManagerUnitTest, SetRingerModeTest_001, TestSize.Level1)
{
    auto ret = AudioSystemManager::GetInstance()->SetRingerMode(AudioRingerMode::RINGER_MODE_SILENT);
    EXPECT_EQ(SUCCESS, ret);

    AudioRingerMode ringerMode = AudioSystemManager::GetInstance()->GetRingerMode();
    EXPECT_EQ(ringerMode, AudioRingerMode::RINGER_MODE_SILENT);
}

/**
* @tc.name   : Test SetRingerMode API
* @tc.number : SetRingerModeTest_002
* @tc.desc   : Test setting of ringer mode to NORMAL
*/
HWTEST(AudioManagerUnitTest, SetRingerModeTest_002, TestSize.Level1)
{
    auto ret = AudioSystemManager::GetInstance()->SetRingerMode(AudioRingerMode::RINGER_MODE_NORMAL);
    EXPECT_EQ(SUCCESS, ret);

    AudioRingerMode ringerMode = AudioSystemManager::GetInstance()->GetRingerMode();
    EXPECT_EQ(ringerMode, AudioRingerMode::RINGER_MODE_NORMAL);
}

/**
* @tc.name   : Test SetRingerMode API
* @tc.number : SetRingerModeTest_003
* @tc.desc   : Test setting of ringer mode to VIBRATE
*/
HWTEST(AudioManagerUnitTest, SetRingerModeTest_003, TestSize.Level1)
{
    auto ret = AudioSystemManager::GetInstance()->SetRingerMode(AudioRingerMode::RINGER_MODE_VIBRATE);
    EXPECT_EQ(SUCCESS, ret);

    AudioRingerMode ringerMode = AudioSystemManager::GetInstance()->GetRingerMode();
    EXPECT_EQ(ringerMode, AudioRingerMode::RINGER_MODE_VIBRATE);
}

/**
* @tc.name   : Test SetRingerMode API
* @tc.number : SetRingerModeTest_004
* @tc.desc   : Test setting of ringer mode to VIBRATE
*/
HWTEST(AudioManagerUnitTest, SetRingerModeTest_004, TestSize.Level1)
{
    VolumeUtils::SetPCVolumeEnable(true);
    auto ret = AudioSystemManager::GetInstance()->SetRingerMode(AudioRingerMode::RINGER_MODE_VIBRATE);
    EXPECT_EQ(SUCCESS, ret);

    AudioRingerMode ringerMode = AudioSystemManager::GetInstance()->GetRingerMode();
    EXPECT_EQ(ringerMode, AudioRingerMode::RINGER_MODE_VIBRATE);
}

/**
* @tc.name   : Test SetRingerMode API
* @tc.number : SetRingerModeTest_005
* @tc.desc   : Test setting of ringer mode to SILENT
*/
HWTEST(AudioManagerUnitTest, SetRingerModeTest_005, TestSize.Level1)
{
    VolumeUtils::SetPCVolumeEnable(true);
    auto ret = AudioSystemManager::GetInstance()->SetRingerMode(AudioRingerMode::RINGER_MODE_SILENT);
    EXPECT_EQ(SUCCESS, ret);

    AudioRingerMode ringerMode = AudioSystemManager::GetInstance()->GetRingerMode();
    EXPECT_EQ(ringerMode, AudioRingerMode::RINGER_MODE_SILENT);
}

#ifdef TEMP_DISABLE
/**
* @tc.name   : Test SetMicrophoneMute API
* @tc.number : SetMicrophoneMute_001
* @tc.desc   : Test muting of microphone to true
*/
HWTEST(AudioManagerUnitTest, SetMicrophoneMute_001, TestSize.Level1)
{
    int32_t ret = AudioSystemManager::GetInstance()->SetMicrophoneMute(true);
    EXPECT_EQ(SUCCESS, ret);

    bool isMicrophoneMuted = AudioSystemManager::GetInstance()->IsMicrophoneMute();
    EXPECT_EQ(isMicrophoneMuted, true);
}

/**
* @tc.name   : Test SetMicrophoneMute API
* @tc.number : SetMicrophoneMute_002
* @tc.desc   : Test muting of microphone to false
*/
HWTEST(AudioManagerUnitTest, SetMicrophoneMute_002, TestSize.Level1)
{
    int32_t ret = AudioSystemManager::GetInstance()->SetMicrophoneMute(false);
    EXPECT_EQ(SUCCESS, ret);

    ret =  AudioSystemManager::GetInstance()->GetGroupManager(DEFAULT_VOLUME_GROUP_ID)->
        SetMicrophoneMutePersistent(false, PolicyType::PRIVACY_POLCIY_TYPE);
    EXPECT_EQ(ERROR, ret);
    bool isMicrophoneMuted = AudioSystemManager::GetInstance()->IsMicrophoneMute();
    EXPECT_EQ(isMicrophoneMuted, false);
}

/**
* @tc.name   : Test SetMute API
* @tc.number : SetMute_001
* @tc.desc   : Test mute functionality of ring stream
*/
HWTEST(AudioManagerUnitTest, SetMute_001, TestSize.Level1)
{
    int32_t ret = AudioSystemManager::GetInstance()->SetMute(AudioVolumeType::STREAM_RING, true);
    EXPECT_EQ(SUCCESS, ret);
}
#endif

/**
* @tc.name   : Test SetMute API
* @tc.number : SetMute_002
* @tc.desc   : Test unmute functionality of ring stream
*/
HWTEST(AudioManagerUnitTest, SetMute_002, TestSize.Level1)
{
    int32_t ret = AudioSystemManager::GetInstance()->SetMute(AudioVolumeType::STREAM_RING, false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name   : Test SetMute API
* @tc.number : SetMute_003
* @tc.desc   : Test mute functionality of music stream
*/
HWTEST(AudioManagerUnitTest, SetMute_003, TestSize.Level1)
{
    int32_t ret = AudioSystemManager::GetInstance()->SetMute(AudioVolumeType::STREAM_MUSIC, true);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name   : Test SetMute API
* @tc.number : SetMute_004
* @tc.desc   : Test unmute functionality of music stream
*/
HWTEST(AudioManagerUnitTest, SetMute_004, TestSize.Level1)
{
    int32_t ret = AudioSystemManager::GetInstance()->SetMute(AudioVolumeType::STREAM_MUSIC, false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name   : Test SetMute API
* @tc.number : SetMute_005
* @tc.desc   : Test mute functionality of default stream
*/
HWTEST(AudioManagerUnitTest, SetMute_005, TestSize.Level1)
{
    int32_t ret = AudioSystemManager::GetInstance()->SetMute(AudioVolumeType::STREAM_DEFAULT, true);
    EXPECT_LT(ret, SUCCESS);
}

/**
* @tc.name   : Test SetMute API
* @tc.number : SetMute_006
* @tc.desc   : Test unmute functionality of default stream
*/
HWTEST(AudioManagerUnitTest, SetMute_006, TestSize.Level1)
{
    int32_t ret = AudioSystemManager::GetInstance()->SetMute(AudioVolumeType::STREAM_DEFAULT, false);
    EXPECT_LT(ret, SUCCESS);
}

#ifdef TEMP_DISABLE
/**
* @tc.name   : Test SetMute API
* @tc.number : SetMute_007
* @tc.desc   : Test mute functionality of medie stream
*/
HWTEST(AudioManagerUnitTest, SetMute_007, TestSize.Level1)
{
    VolumeUtils::SetPCVolumeEnable(true);
    int32_t ret = AudioSystemManager::GetInstance()->SetMute(AudioVolumeType::STREAM_ALL, true);
    EXPECT_EQ(ret, SUCCESS);
    auto isActive = AudioSystemManager::GetInstance()->IsStreamMute(AudioVolumeType::STREAM_SYSTEM);
    EXPECT_TRUE(isActive);
}
#endif

/**
* @tc.name   : Test SetMute API
* @tc.number : SetMute_008
* @tc.desc   : Test unmute functionality of media stream
*/
HWTEST(AudioManagerUnitTest, SetMute_008, TestSize.Level1)
{
    VolumeUtils::SetPCVolumeEnable(true);
    int32_t ret = AudioSystemManager::GetInstance()->SetMute(AudioVolumeType::STREAM_ALL, false);
    EXPECT_EQ(ret, SUCCESS);
    auto isActive = AudioSystemManager::GetInstance()->IsStreamMute(AudioVolumeType::STREAM_SYSTEM);
    EXPECT_FALSE(isActive);
}

/**
 * @tc.name : SetLowPowerVolume_001
 * @tc.desc : Test set the volume discount coefficient of a single stream
 * @tc.type : FUNC
 * @tc.require : issueI5NXAE
 */
HWTEST(AudioManagerUnitTest, SetLowPowerVolume_001, TestSize.Level1)
{
    int32_t streamId = 0;
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    AudioRendererOptions rendererOptions = {};
    AppInfo appInfo = {};
    appInfo.appUid = static_cast<int32_t>(getuid());
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions, appInfo);
    ASSERT_NE(nullptr, audioRenderer);
    int32_t ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);

    for (auto it = audioRendererChangeInfos.begin(); it != audioRendererChangeInfos.end(); it++) {
        AudioRendererChangeInfo audioRendererChangeInfos_ = **it;
        if (audioRendererChangeInfos_.clientUID == appInfo.appUid) {
            streamId = audioRendererChangeInfos_.sessionId;
        }
    }
    ASSERT_NE(0, streamId);

    ret = AudioSystemManager::GetInstance()->SetLowPowerVolume(streamId, DISCOUNT_VOLUME);
    EXPECT_FALSE(ret == SUCCESS || ret == AUDIO_ERR);

    audioRenderer->Release();
}

/**
 * @tc.name : SetLowPowerVolume_002
 * @tc.desc : Test set the volume invalid value
 * @tc.type : FUNC
 * @tc.require : issueI5NXAE
 */
HWTEST(AudioManagerUnitTest, SetLowPowerVolume_002, TestSize.Level1)
{
    int32_t streamId = 0;
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    AudioRendererOptions rendererOptions = {};
    AppInfo appInfo = {};
    appInfo.appUid = static_cast<int32_t>(getuid());
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions, appInfo);
    ASSERT_NE(nullptr, audioRenderer);
    int32_t ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);

    for (auto it = audioRendererChangeInfos.begin(); it != audioRendererChangeInfos.end(); it++) {
        AudioRendererChangeInfo audioRendererChangeInfos_ = **it;
        if (audioRendererChangeInfos_.clientUID == appInfo.appUid) {
            streamId = audioRendererChangeInfos_.sessionId;
        }
    }
    ASSERT_NE(0, streamId);

    ret = AudioSystemManager::GetInstance()->SetLowPowerVolume(streamId, INVALID_VOLUME);
    EXPECT_LT(ret, SUCCESS);

    audioRenderer->Release();
}

#ifdef TEMP_DISABLE
/**
 * @tc.name   : Test SetLowPowerVolume API
 * @tc.number : SetLowPowerVolume_003
 * @tc.desc   : Test function SetLowPowerVolume in the recording scene
 */
HWTEST(AudioManagerUnitTest, SetLowPowerVolume_003, TestSize.Level1)
{
    int32_t streamId = 0;
    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    AudioCapturerOptions capturerOptions = {};
    AppInfo appInfo = {};
    appInfo.appUid = static_cast<int32_t>(getuid());
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions, appInfo);
    ASSERT_NE(nullptr, audioCapturer);
    int32_t ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);

    for (auto it = audioCapturerChangeInfos.begin(); it != audioCapturerChangeInfos.end(); it++) {
        AudioCapturerChangeInfo audioCapturerChangeInfos_ = **it;
        if (audioCapturerChangeInfos_.clientUID == appInfo.appUid) {
            streamId = audioCapturerChangeInfos_.sessionId;
        }
    }
    ASSERT_NE(0, streamId);

    ret = AudioSystemManager::GetInstance()->SetLowPowerVolume(streamId, DISCOUNT_VOLUME);
    EXPECT_TRUE(ret == SUCCESS || ret == AUDIO_ERR);

    audioCapturer->Release();
}
#endif

/**
 * @tc.name : GetLowPowerVolume_001
 * @tc.desc : Test get the volume discount coefficient of a single stream
 * @tc.type : FUNC
 * @tc.require : issueI5NXAE
 */
HWTEST(AudioManagerUnitTest, GetLowPowerVolume_001, TestSize.Level1)
{
    int32_t streamId = 0;
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    AudioRendererOptions rendererOptions = {};
    AppInfo appInfo = {};
    appInfo.appUid = static_cast<int32_t>(getuid());
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions, appInfo);
    ASSERT_NE(nullptr, audioRenderer);
    int32_t ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);

    for (auto it = audioRendererChangeInfos.begin(); it != audioRendererChangeInfos.end(); it++) {
        AudioRendererChangeInfo audioRendererChangeInfos_ = **it;
        if (audioRendererChangeInfos_.clientUID == appInfo.appUid) {
            streamId = audioRendererChangeInfos_.sessionId;
        }
    }
    ASSERT_NE(0, streamId);

    float vol = AudioSystemManager::GetInstance()->GetLowPowerVolume(streamId);
    EXPECT_FALSE((vol < VOLUME_MIN || vol > VOLUME_MAX));
    audioRenderer->Release();
}

#ifdef TEMP_DISABLE
/**
 * @tc.name   : Test GetLowPowerVolume API
 * @tc.number : GetLowPowerVolume_002
 * @tc.desc   : Test function GetLowPowerVolume in the recording scene
 */
HWTEST(AudioManagerUnitTest, GetLowPowerVolume_002, TestSize.Level1)
{
    int32_t streamId = 0;
    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    AudioCapturerOptions capturerOptions = {};
    AppInfo appInfo = {};
    appInfo.appUid = static_cast<int32_t>(getuid());
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions, appInfo);
    ASSERT_NE(nullptr, audioCapturer);
    int32_t ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    EXPECT_EQ(SUCCESS, ret);

    for (auto it = audioCapturerChangeInfos.begin(); it != audioCapturerChangeInfos.end(); it++) {
        AudioCapturerChangeInfo audioCapturerChangeInfos_ = **it;
        if (audioCapturerChangeInfos_.clientUID == appInfo.appUid) {
            streamId = audioCapturerChangeInfos_.sessionId;
        }
    }
    ASSERT_NE(0, streamId);

    float vol = AudioSystemManager::GetInstance()->GetLowPowerVolume(streamId);
    EXPECT_FALSE((vol < VOLUME_MIN || vol > VOLUME_MAX));
    audioCapturer->Release();
}
#endif

/**
 * @tc.name    : GetSingleStreamVolume_001
 * @tc.desc    : Test get single stream volume.
 * @tc.type    : FUNC
 * @tc.require : issueI5NXAE
 */
HWTEST(AudioManagerUnitTest, GetSingleStreamVolume_001, TestSize.Level1)
{
    int32_t streamId = 0;
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    AudioRendererOptions rendererOptions = {};
    AppInfo appInfo = {};
    appInfo.appUid = static_cast<int32_t>(getuid());
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MEDIA;
    rendererOptions.rendererInfo.rendererFlags = 0;

    unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions, appInfo);
    ASSERT_NE(nullptr, audioRenderer);
    int32_t ret = AudioStreamManager::GetInstance()->GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    EXPECT_EQ(SUCCESS, ret);

    for (auto it = audioRendererChangeInfos.begin(); it != audioRendererChangeInfos.end(); it++) {
        AudioRendererChangeInfo audioRendererChangeInfos_ = **it;
        if (audioRendererChangeInfos_.clientUID == appInfo.appUid) {
            streamId = audioRendererChangeInfos_.sessionId;
        }
    }
    ASSERT_NE(0, streamId);

    float vol = AudioSystemManager::GetInstance()->GetSingleStreamVolume(streamId);
    EXPECT_FALSE((vol < VOLUME_MIN || vol > VOLUME_MAX));
    audioRenderer->Release();
}

#ifdef TEMP_DISABLE
/**
 * @tc.name   : Test GetSingleStreamVolume API
 * @tc.number : GetSingleStreamVolume_002
 * @tc.desc   : Test function GetSingleStreamVolume in the recording scene
 */
HWTEST(AudioManagerUnitTest, GetSingleStreamVolume_002, TestSize.Level1)
{
    int32_t streamId = 0;
    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfo;
    AudioCapturerOptions capturerOptions = {};
    AppInfo appInfo = {};
    appInfo.appUid = static_cast<int32_t>(getuid());
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_8000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_U8;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = CAPTURER_FLAG;

    unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(capturerOptions, appInfo);
    ASSERT_NE(nullptr, audioCapturer);
    int32_t ret = AudioStreamManager::GetInstance()->GetCurrentCapturerChangeInfos(audioCapturerChangeInfo);
    EXPECT_EQ(SUCCESS, ret);

    for (auto it = audioCapturerChangeInfo.begin(); it != audioCapturerChangeInfo.end(); it++) {
        AudioCapturerChangeInfo audioCapturerChangeInfo_ = **it;
        if (audioCapturerChangeInfo_.clientUID == appInfo.appUid) {
            streamId = audioCapturerChangeInfo_.sessionId;
        }
    }
    ASSERT_NE(0, streamId);

    float vol = AudioSystemManager::GetInstance()->GetSingleStreamVolume(streamId);
    EXPECT_FALSE((vol < VOLUME_MIN || vol > VOLUME_MAX));
    audioCapturer->Release();
}
#endif

/**
* @tc.name   : Test SetPauseOrResumeStream API
* @tc.number : SetPauseOrResumeStream_001
* @tc.desc   : Test Puase functionality of media stream
*/
HWTEST(AudioManagerUnitTest, SetPauseOrResumeStream_001, TestSize.Level1)
{
    int32_t ret = AudioSystemManager::GetInstance()->UpdateStreamState(0,
        StreamSetState::STREAM_PAUSE, STREAM_USAGE_MEDIA);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name   : Test SetPauseOrResumeStream API
* @tc.number : SetPauseOrResumeStream_002
* @tc.desc   : Test Resume functionality of media stream
*/
HWTEST(AudioManagerUnitTest, SetPauseOrResumeStream_002, TestSize.Level1)
{
    int32_t ret = AudioSystemManager::GetInstance()->UpdateStreamState(0,
        StreamSetState::STREAM_RESUME, STREAM_USAGE_MEDIA);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name   : Test GetAudioEffectInfoArray API
* @tc.number : GetAudioEffectInfoArray_001
* @tc.desc   : Test GetAudioEffectInfoArray interface.
*/
HWTEST(AudioManagerUnitTest, GetAudioEffectInfoArray_001, TestSize.Level1)
{
    // STREAM_MUSIC
    int32_t ret;
    AudioSceneEffectInfo audioSceneEffectInfo = {};
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo, streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);

    audioSceneEffectInfo = {};
    streamUsage = STREAM_USAGE_MEDIA;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo, streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);

    audioSceneEffectInfo = {};
    streamUsage = STREAM_USAGE_MUSIC;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo, streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);

    audioSceneEffectInfo = {};
    streamUsage = STREAM_USAGE_VOICE_COMMUNICATION;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo, streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);

    audioSceneEffectInfo = {};
    streamUsage = STREAM_USAGE_VOICE_ASSISTANT;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo, streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);
}

/**
* @tc.name   : Test GetAudioEffectInfoArray API
* @tc.number : GetAudioEffectInfoArray_002
* @tc.desc   : Test GetAudioEffectInfoArray interface.
*/
HWTEST(AudioManagerUnitTest, GetAudioEffectInfoArray_002, TestSize.Level1)
{
    // STREAM_MUSIC
    int32_t ret;
    AudioSceneEffectInfo audioSceneEffectInfo = {};
    StreamUsage streamUsage = STREAM_USAGE_ALARM;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo,  streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);

    audioSceneEffectInfo = {};
    streamUsage = STREAM_USAGE_VOICE_MESSAGE;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo,  streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);

    audioSceneEffectInfo = {};
    streamUsage = STREAM_USAGE_NOTIFICATION_RINGTONE;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo,  streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);

    audioSceneEffectInfo = {};
    streamUsage = STREAM_USAGE_RINGTONE;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo, streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);

    audioSceneEffectInfo = {};
    streamUsage = STREAM_USAGE_NOTIFICATION;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo, streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);
}

/**
* @tc.name   : Test GetAudioEffectInfoArray API
* @tc.number : GetAudioEffectInfoArray_003
* @tc.desc   : Test GetAudioEffectInfoArray interface.
*/
HWTEST(AudioManagerUnitTest, GetAudioEffectInfoArray_003, TestSize.Level1)
{
    int32_t ret;
    AudioSceneEffectInfo audioSceneEffectInfo = {};
    StreamUsage streamUsage = STREAM_USAGE_ACCESSIBILITY;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo,  streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);

    audioSceneEffectInfo = {};
    streamUsage = STREAM_USAGE_SYSTEM;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo,  streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);

    audioSceneEffectInfo = {};
    streamUsage = STREAM_USAGE_GAME;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo,  streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);

    audioSceneEffectInfo = {};
    streamUsage = STREAM_USAGE_AUDIOBOOK;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo,  streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);

    audioSceneEffectInfo = {};
    streamUsage = STREAM_USAGE_NAVIGATION;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo,  streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);
}

/**
* @tc.name   : Test GetAudioEffectInfoArray API
* @tc.number : GetAudioEffectInfoArray_004
* @tc.desc   : Test GetAudioEffectInfoArray interface.
*/
HWTEST(AudioManagerUnitTest, GetAudioEffectInfoArray_004, TestSize.Level1)
{
    // STREAM_MUSIC
    int32_t ret;
    AudioSceneEffectInfo audioSceneEffectInfo = {};
    StreamUsage streamUsage = STREAM_USAGE_DTMF;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo,  streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);

    audioSceneEffectInfo = {};
    streamUsage = STREAM_USAGE_ENFORCED_TONE;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo,  streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);

    audioSceneEffectInfo = {};
    streamUsage = STREAM_USAGE_ULTRASONIC;
    ret = AudioStreamManager::GetInstance()->GetEffectInfoArray(audioSceneEffectInfo,  streamUsage);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(EFFECT_NONE, audioSceneEffectInfo.mode[0]);
    EXPECT_EQ(EFFECT_DEFAULT, audioSceneEffectInfo.mode[1]);
}

/**
 * @tc.name   : Test SetDeviceAbsVolumeSupported API
 * @tc.number : SetDeviceAbsVolumeSupported_001
 * @tc.desc   : Test SetDeviceAbsVolumeSupported interface.
 */
HWTEST(AudioManagerUnitTest, SetDeviceAbsVolumeSupported_001, TestSize.Level1)
{
    int32_t ret;
    bool support = true;
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);

    for (auto outputDevice : audioDeviceDescriptors) {
        EXPECT_EQ(outputDevice->deviceRole_, DeviceRole::OUTPUT_DEVICE);
        if (outputDevice->deviceType_ != DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP) {
            continue;
        }
        EXPECT_GE(outputDevice->deviceId_, MIN_DEVICE_ID);
        DeviceStreamInfo audioStreamInfo = outputDevice->GetDeviceStreamInfo();
        EXPECT_THAT(audioStreamInfo.samplingRate, Each(AllOf(Le(SAMPLE_RATE_96000), Ge(SAMPLE_RATE_8000))));
        EXPECT_EQ(audioStreamInfo.encoding, AudioEncodingType::ENCODING_PCM);
        std::set<AudioChannel> channels = audioStreamInfo.GetChannels();
        EXPECT_THAT(channels, Each(AllOf(Le(CHANNEL_8), Ge(MONO))));

        EXPECT_EQ(true, (audioStreamInfo.format >= SAMPLE_U8) && ((audioStreamInfo.format <= SAMPLE_F32LE)));
        if ((outputDevice->macAddress_).c_str()!= nullptr) {
            ret = AudioSystemManager::GetInstance()->SetDeviceAbsVolumeSupported(outputDevice->macAddress_, support, 0);
            EXPECT_EQ(SUCCESS, ret);

            ret = AudioSystemManager::GetInstance()->SetA2dpDeviceVolume(outputDevice->macAddress_, 2, support);
            EXPECT_EQ(SUCCESS, ret);
        }
    }
    std::string macAddress = "";
    support = false;
    ret = AudioSystemManager::GetInstance()->SetDeviceAbsVolumeSupported(macAddress, support, 0);
    EXPECT_EQ(ERROR, ret);

    ret = AudioSystemManager::GetInstance()->SetA2dpDeviceVolume(macAddress, 0, support);
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name   : Test SetAvailableDeviceChangeCallback API
 * @tc.number : SetAvailableDeviceChangeCallback_001
 * @tc.desc   : Test SetAvailableDeviceChangeCallback interface.
 */
HWTEST(AudioManagerUnitTest, SetAvailableDeviceChangeCallback_001, TestSize.Level1)
{
    int32_t ret;
    AudioDeviceUsage usage = AudioDeviceUsage::MEDIA_OUTPUT_DEVICES;
    shared_ptr<AudioManagerAvailableDeviceChangeCallback> callback = nullptr;
    ret = AudioSystemManager::GetInstance()->SetAvailableDeviceChangeCallback(usage, callback);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    ret = AudioSystemManager::GetInstance()->UnsetAvailableDeviceChangeCallback(usage);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name   : Test SetAvailableDeviceChangeCallback API
 * @tc.number : SetAvailableDeviceChangeCallback_002
 * @tc.desc   : Test SetAvailableDeviceChangeCallback interface.
 */
HWTEST(AudioManagerUnitTest, SetAvailableDeviceChangeCallback_002, TestSize.Level1)
{
    int32_t ret;
    AudioDeviceUsage usage = MEDIA_INPUT_DEVICES;
    shared_ptr<AudioManagerAvailableDeviceChangeCallback> callback
        = make_shared<AudioManagerAvailableDeviceChangeCallbackImpl>();
    ret = AudioSystemManager::GetInstance()->SetAvailableDeviceChangeCallback(usage, callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioSystemManager::GetInstance()->UnsetAvailableDeviceChangeCallback(usage);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name   : Test SetDistributedRoutingRoleCallback API
 * @tc.number : SetDistributedRoutingRoleCallback_001
 * @tc.desc   : Test SetDistributedRoutingRoleCallback interface.
 */
HWTEST(AudioManagerUnitTest, SetDistributedRoutingRoleCallback_001, TestSize.Level1)
{
    int32_t ret;
    shared_ptr<AudioDistributedRoutingRoleCallback> callback = nullptr;
    ret = AudioSystemManager::GetInstance()->SetDistributedRoutingRoleCallback(callback);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);

    ret = AudioSystemManager::GetInstance()->UnsetDistributedRoutingRoleCallback(callback);
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name   : Test SetDistributedRoutingRoleCallback API
 * @tc.number : SetDistributedRoutingRoleCallback_002
 * @tc.desc   : Test SetDistributedRoutingRoleCallback interface.
 */
HWTEST(AudioManagerUnitTest, SetDistributedRoutingRoleCallback_002, TestSize.Level1)
{
    int32_t ret;
    shared_ptr<AudioDistributedRoutingRoleCallback> callback
        = make_shared<AudioDistributedRoutingRoleCallbackTest>();
    ret = AudioSystemManager::GetInstance()->SetDistributedRoutingRoleCallback(callback);
    EXPECT_EQ(SUCCESS, ret);

    ret = AudioSystemManager::GetInstance()->UnsetDistributedRoutingRoleCallback(callback);
    EXPECT_EQ(ERROR, ret);
}

bool GetOffloadAvailable()
{
    cout << "GetOffloadAvailable enter" << endl;
    ifstream ifs;
    ifs.open(CONFIG_FILE_NEW, ios::in);
    if (!ifs.is_open()) {
        ifs.open(CONFIG_FILE, ios::in);
        if (!ifs.is_open()) {
            cout << "open CONFIG_FILE failed!" << endl;
            return false;
        }
    }
    string s;
    while (getline(ifs, s)) {
        if (s.find("pipe name=\"offload_output\"") != string::npos) {
            ifs.close();
            return true;
        }
    }
    ifs.close();
    return false;
}

/**
* @tc.name   : Test ConfigDistributedRoutingRole API
* @tc.number : ConfigDistributedRoutingRoleTest_001
* @tc.desc   : Test ConfigDistributedRoutingRole inner api, when audioDeviceDescriptors is INPUT_DEVICES
*/
HWTEST(AudioManagerUnitTest, ConfigDistributedRoutingRoleTest_001, TestSize.Level1)
{
    int32_t ret;
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::INPUT_DEVICES_FLAG);
    ret = audioDeviceDescriptors.size();
    EXPECT_GE(ret, MIN_DEVICE_NUM);
    CastType castType = CAST_TYPE_ALL;
    ret = AudioSystemManager::GetInstance()->ConfigDistributedRoutingRole(audioDeviceDescriptors[0], castType);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

#ifdef TEMP_DISABLE
/**
* @tc.name   : Test ConfigDistributedRoutingRole API
* @tc.number : ConfigDistributedRoutingRoleTest_002
* @tc.desc   : Test ConfigDistributedRoutingRole inner api, when audioDeviceDescriptors is OUTPUT_DEVICES
*/
HWTEST(AudioManagerUnitTest, ConfigDistributedRoutingRoleTest_002, TestSize.Level1)
{
    int32_t ret;
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    ret = audioDeviceDescriptors.size();
    EXPECT_GE(ret, MIN_DEVICE_NUM);
    CastType castType = CAST_TYPE_ALL;
    ret = AudioSystemManager::GetInstance()->ConfigDistributedRoutingRole(audioDeviceDescriptors[0], castType);
    EXPECT_EQ(SUCCESS, ret);
}
#endif

/**
* @tc.name   : Test ConfigDistributedRoutingRole API
* @tc.number : ConfigDistributedRoutingRoleTest_003
* @tc.desc   : Test ConfigDistributedRoutingRole inner api, when networkid is REMOTE_NETWORK_ID
*/
HWTEST(AudioManagerUnitTest, ConfigDistributedRoutingRoleTest_003, TestSize.Level1)
{
    int32_t ret;
    auto audioDeviceDescriptors = AudioSystemManager::GetInstance()->GetDevices(DeviceFlag::OUTPUT_DEVICES_FLAG);
    ret = audioDeviceDescriptors.size();
    EXPECT_GE(ret, MIN_DEVICE_NUM);
    CastType castType = CAST_TYPE_ALL;
    audioDeviceDescriptors[0]->networkId_ = REMOTE_NETWORK_ID;
    ret = AudioSystemManager::GetInstance()->ConfigDistributedRoutingRole(audioDeviceDescriptors[0], castType);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name   : Test SetCallDeviceActive API
 * @tc.number : SetCallDeviceActive_001
 * @tc.desc   : Test SetCallDeviceActive interface.
 */
HWTEST(AudioManagerUnitTest, SetCallDeviceActive_001, TestSize.Level1)
{
    // On bootup sco won't be connected. Hence setup should fail.
    std::string address = "";
    auto ret = AudioSystemManager::GetInstance()->SetCallDeviceActive(
        DeviceType::DEVICE_TYPE_BLUETOOTH_SCO, true, address);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name   : Test LoadSplitModule API
 * @tc.number : LoadSplitModule_001
 * @tc.desc   : Test LoadSplitModule interface, no permission, DT uid is 0(ROOT), not hicar uid: 65872
 */
HWTEST(AudioManagerUnitTest, LoadSplitModule_001, TestSize.Level1)
{
    auto ret = AudioSystemManager::GetInstance()->LoadSplitModule("", "");
    EXPECT_EQ(ERR_PERMISSION_DENIED, ret);
}

/**
 * @tc.name   : Test LoadSplitModule API
 * @tc.number : LoadSplitModule_002
 * @tc.desc   : Test LoadSplitModule interface, ERR_INVALID_PARAM return, the "splitArgs" is empty.
 */
HWTEST(AudioManagerUnitTest, LoadSplitModule_002, TestSize.Level1)
{
    int32_t setUidRet = setuid(UID_CAR_DISTRIBUTED_ENGINE_SA);
    std::cout << "stUidRet: " << setUidRet << std::endl;
    auto ret = AudioSystemManager::GetInstance()->LoadSplitModule("", TEST_NETWORK_ID);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name   : Test LoadSplitModule API
 * @tc.number : LoadSplitModule_003
 * @tc.desc   : Test LoadSplitModule interface, ERR_INVALID_PARAM return, the "networkId" is empty.
 */
HWTEST(AudioManagerUnitTest, LoadSplitModule_003, TestSize.Level1)
{
    int32_t setUidRet = setuid(UID_CAR_DISTRIBUTED_ENGINE_SA);
    std::cout << "stUidRet: " << setUidRet << std::endl;
    auto ret = AudioSystemManager::GetInstance()->LoadSplitModule(TEST_SPLIT_ARGS, "");
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
 * @tc.name   : Test LoadSplitModule API
 * @tc.number : LoadSplitModule_004
 * @tc.desc   : Test LoadSplitModule interface, ERR_INVALID_HANDLE return. OpenPortAndInsertIOHandle failed.
 */
HWTEST(AudioManagerUnitTest, LoadSplitModule_004, TestSize.Level1)
{
    int32_t setUidRet = setuid(UID_CAR_DISTRIBUTED_ENGINE_SA);
    std::cout << "stUidRet: " << setUidRet << std::endl;
    auto ret = AudioSystemManager::GetInstance()->LoadSplitModule(TEST_SPLIT_ARGS, TEST_NETWORK_ID);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name   : Test NotifySessionStateChange API
 * @tc.number : NotifySessionStateChange_001
 * @tc.desc   : Test NotifySessionStateChange interface.
 */
HWTEST(AudioManagerUnitTest, NotifySessionStateChange_001, TestSize.Level1)
{
    int32_t ret;
    int32_t uid = 1;
    int32_t pid = 1;
    ret = AudioSystemManager::GetInstance()->NotifySessionStateChange(uid, pid, true);
    EXPECT_EQ(ERROR, ret);

    ret = AudioSystemManager::GetInstance()->NotifySessionStateChange(uid, pid, false);
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name   : Test NotifyFreezeStateChange API
 * @tc.number : NotifyFreezeStateChange_001
 * @tc.desc   : Test NotifyFreezeStateChange interface.
 */
HWTEST(AudioManagerUnitTest, NotifyFreezeStateChange_001, TestSize.Level1)
{
    int32_t ret;
    int32_t pid = 1;
    std::set<int32_t> pidList;
    pidList.insert(pid);
    ret = AudioSystemManager::GetInstance()->NotifyFreezeStateChange(pidList, true);
    EXPECT_EQ(ERROR, ret);

    ret = AudioSystemManager::GetInstance()->NotifyFreezeStateChange(pidList, false);
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name   : Test ResetAllProxy API
 * @tc.number : ResetAllProxy_001
 * @tc.desc   : Test ResetAllProxy interface.
 */
HWTEST(AudioManagerUnitTest, ResetAllProxy_001, TestSize.Level1)
{
    int32_t ret;
    ret = AudioSystemManager::GetInstance()->ResetAllProxy();
    EXPECT_EQ(ERROR, ret);

    ret = AudioSystemManager::GetInstance()->ResetAllProxy();
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name   : Test NotifyProcessBackgroundState API
 * @tc.number : NotifyProcessBackgroundState_001
 * @tc.desc   : Test NotifyProcessBackgroundState interface.
 */
HWTEST(AudioManagerUnitTest, NotifyProcessBackgroundState_001, TestSize.Level1)
{
    int32_t uid = 1001;
    int32_t pid = 2001;
    int32_t ret;
    ret = AudioSystemManager::GetInstance()->NotifyProcessBackgroundState(uid, pid);
    EXPECT_NE(SUCCESS, ret);
}

/**
 * @tc.name  : Test SetVolumeDegree API
 * @tc.number: SetVolumeDegree_001
 * @tc.desc  : SetVolumeDegree
 * @tc.require:
 */
HWTEST(AudioManagerUnitTest, SetVolumeDegree_001, TestSize.Level0)
{
    auto manager = AudioSystemManager::GetInstance();

    int32_t degree = 44;
    AudioStreamType streamType = STREAM_MUSIC;
    int32_t ret = manager->SetVolumeDegree(streamType, degree);
    EXPECT_EQ(SUCCESS, ret);

    AudioStreamType streamType2 = STREAM_ULTRASONIC;
    ret = manager->SetVolumeDegree(streamType2, degree);
    EXPECT_EQ(SUCCESS, ret);

    AudioStreamType streamType3 = STREAM_APP;
    ret = manager->SetVolumeDegree(streamType3, degree);
    EXPECT_EQ(ERR_NOT_SUPPORTED, ret);
}

/**
 * @tc.name  : Test GetVolumeDegree API
 * @tc.number: GetVolumeDegree_001
 * @tc.desc  : GetVolumeDegree
 * @tc.require:
 */
HWTEST(AudioManagerUnitTest, GetVolumeDegree_001, TestSize.Level0)
{
    auto manager = AudioSystemManager::GetInstance();
    int32_t degree = 44;
    AudioStreamType streamType = STREAM_ALARM;
    int32_t ret = manager->SetVolumeDegree(streamType, degree);
    EXPECT_EQ(SUCCESS, ret);

    ret = manager->GetVolumeDegree(streamType);
    EXPECT_EQ(ret, degree);

    AudioStreamType streamType3 = STREAM_APP;
    ret = manager->GetVolumeDegree(streamType3);
    EXPECT_EQ(ERR_NOT_SUPPORTED, ret);
}

/**
 * @tc.name  : Test GetMinVolumeDegree API
 * @tc.number: GetMinVolumeDegree_001
 * @tc.desc  : GetMinVolumeDegree
 * @tc.require:
 */
HWTEST(AudioManagerUnitTest, GetMinVolumeDegree_001, TestSize.Level0)
{
    auto manager = AudioSystemManager::GetInstance();

    AudioVolumeType streamType = STREAM_ALL;
    int32_t ret = manager->GetMinVolumeDegree(streamType);
    EXPECT_EQ(SUCCESS, ret);

    AudioVolumeType streamType2 = STREAM_ULTRASONIC;
    ret = manager->GetMinVolumeDegree(streamType2);
    EXPECT_EQ(SUCCESS, ret);

    AudioVolumeType streamType3 = STREAM_MUSIC;
    ret = manager->GetMinVolumeDegree(streamType3);
    EXPECT_EQ(SUCCESS, ret);
}
} // namespace AudioStandard
} // namespace OHOS
