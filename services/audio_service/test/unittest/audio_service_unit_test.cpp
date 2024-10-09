/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "audio_service_log.h"
#include "audio_errors.h"
#include "audio_system_manager.h"

#include "audio_manager_proxy.h"
#include "audio_manager_listener_stub.h"
#include "audio_process_proxy.h"
#include "audio_process_in_client.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
std::unique_ptr<AudioManagerProxy> audioManagerProxy;
std::shared_ptr<AudioProcessInClient> processClient_;
const int32_t TEST_RET_NUM = 0;
const int32_t RENDERER_FLAGS = 0;
constexpr int32_t ERROR_62980101 = -62980101;

class AudioServiceUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

class AudioParameterCallbackTest : public AudioParameterCallback {
    virtual void OnAudioParameterChange(const std::string networkId, const AudioParamKey key,
        const std::string& condition, const std::string& value) {}
};

void AudioServiceUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioServiceUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioServiceUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioServiceUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test AudioProcessProxy API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessProxy_001
 * @tc.desc  : Test AudioProcessProxy interface.
 */
HWTEST(AudioServiceUnitTest, AudioProcessProxy_001, TestSize.Level1)
{
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(nullptr, samgr);
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    EXPECT_NE(nullptr, object);
    std::unique_ptr<AudioProcessProxy> audioProcessProxy = std::make_unique<AudioProcessProxy>(object);

    int32_t ret = -1;
    std::shared_ptr<OHAudioBuffer> buffer;
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame - 1;
    uint32_t byteSizePerFrame = 1000;
    buffer = OHAudioBuffer::CreateFromLocal(totalSizeInFrame, spanSizeInFrame, byteSizePerFrame);

    ret=audioProcessProxy->ResolveBuffer(buffer);
    EXPECT_LT(ret, TEST_RET_NUM);

    ret = audioProcessProxy->Start();
    EXPECT_LT(ret, TEST_RET_NUM);

    bool isFlush = true;
    ret = audioProcessProxy->Pause(isFlush);
    EXPECT_LT(ret, TEST_RET_NUM);

    ret = audioProcessProxy->Resume();
    EXPECT_LT(ret, TEST_RET_NUM);

    ret = audioProcessProxy->Stop();
    EXPECT_LT(ret, TEST_RET_NUM);

    ret = audioProcessProxy->RequestHandleInfo();
    EXPECT_EQ(ret, SUCCESS);

    ret = audioProcessProxy->Release();
    EXPECT_LT(ret, TEST_RET_NUM);
}

/**
 * @tc.name  : Test AudioManagerProxy API
 * @tc.type  : FUNC
 * @tc.number: AudioManagerProxy_001
 * @tc.desc  : Test AudioManagerProxy interface.
 */
HWTEST(AudioServiceUnitTest, AudioManagerProxy_001, TestSize.Level1)
{
    int32_t ret = -1;

    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_NE(nullptr, samgr);
    sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
    EXPECT_NE(nullptr, object);

    audioManagerProxy = std::make_unique<AudioManagerProxy>(object);

    bool isMute = true;
    ret = audioManagerProxy->SetMicrophoneMute(isMute);
    EXPECT_EQ(ret, SUCCESS);

    ret = audioManagerProxy->RegiestPolicyProvider(object);
    EXPECT_EQ(SUCCESS, ret);

    bool state = false;
    ret = audioManagerProxy->SetCaptureSilentState(state);
    EXPECT_EQ(ERROR_62980101, ret);

    bool result = audioManagerProxy->CreatePlaybackCapturerManager();
    EXPECT_EQ(result, SUCCESS);

    int32_t deviceType = 1;
    std::string sinkName = "test";
    audioManagerProxy->SetOutputDeviceSink(deviceType, sinkName);
}

/**
 * @tc.name  : Test AudioManagerProxy API
 * @tc.type  : FUNC
 * @tc.number: AudioManagerProxy_002
 * @tc.desc  : Test AudioManagerProxy interface.
 */
HWTEST(AudioServiceUnitTest, AudioManagerProxy_002, TestSize.Level1)
{
    int32_t ret = -1;

    float volume = 0.1;
    ret = audioManagerProxy->SetVoiceVolume(volume);

    const std::string networkId = "LocalDevice";
    const AudioParamKey key = AudioParamKey::VOLUME;
    AudioVolumeType volumeType =AudioVolumeType::STREAM_MEDIA;
    int32_t groupId = 0;
    std::string condition = "EVENT_TYPE=1;VOLUME_GROUP_ID=" + std::to_string(groupId) + ";AUDIO_VOLUME_TYPE="
        + std::to_string(volumeType) + ";";
    std::string value = std::to_string(volume);
    audioManagerProxy->SetAudioParameter(networkId, key, condition, value);
    const std::string retStr = audioManagerProxy->GetAudioParameter(networkId, key, condition);
    EXPECT_NE(retStr, value);

    bool connected = true;
    audioManagerProxy->NotifyDeviceInfo(networkId, connected);
    ret = audioManagerProxy->CheckRemoteDeviceState(networkId, DeviceRole::OUTPUT_DEVICE, true);
    EXPECT_LT(ret, TEST_RET_NUM);
}

/**
 * @tc.name  : Test AudioManagerProxy API
 * @tc.type  : FUNC
 * @tc.number: AudioManagerProxy_004
 * @tc.desc  : Test AudioManagerProxy interface.
 */
HWTEST(AudioServiceUnitTest, AudioManagerProxy_004, TestSize.Level1)
{
    std::vector<Library> libraries;
    Library library = {};
    library.name = "testname";
    library.path ="test.so";
    libraries.push_back(library);

    std::vector<Effect> effects;
    Effect effect = {};
    effect.name = "test";
    effect.libraryName = "test";
    effects.push_back(effect);

    std::vector<Effect> successEffects;
    bool ret = audioManagerProxy->LoadAudioEffectLibraries(libraries, effects, successEffects);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioManagerListenerStub API
 * @tc.type  : FUNC
 * @tc.number: AudioManagerListenerStub_001
 * @tc.desc  : Test AudioManagerListenerStub interface.
 */
HWTEST(AudioServiceUnitTest, AudioManagerListenerStub_001, TestSize.Level1)
{
    std::unique_ptr<AudioManagerListenerStub> audioManagerListenerStub = std::make_unique<AudioManagerListenerStub>();

    const std::weak_ptr<AudioParameterCallback> callback = std::make_shared<AudioParameterCallbackTest>();
    audioManagerListenerStub->SetParameterCallback(callback);
    float volume = 0.1;
    const std::string networkId = "LocalDevice";
    const AudioParamKey key = AudioParamKey::VOLUME;
    AudioVolumeType volumeType =AudioVolumeType::STREAM_MEDIA;
    int32_t groupId = 0;
    std::string condition = "EVENT_TYPE=1;VOLUME_GROUP_ID=" + std::to_string(groupId) + ";AUDIO_VOLUME_TYPE="
        + std::to_string(volumeType) + ";";
    std::string value = std::to_string(volume);
    audioManagerListenerStub->OnAudioParameterChange(networkId, key, condition, value);
    EXPECT_NE(value, "");
}


/**
 * @tc.name  : Test AudioProcessInClientInner API
 * @tc.type  : FUNC
 * @tc.number: AudioProcessInClientInner_001
 * @tc.desc  : Test AudioProcessInClientInner interface using unsupported parameters.
 */
HWTEST(AudioServiceUnitTest, AudioProcessInClientInner_001, TestSize.Level1)
{
    AudioProcessConfig config;
    config.appInfo.appPid = getpid();
    config.appInfo.appUid = getuid();

    config.audioMode = AUDIO_MODE_PLAYBACK;

    config.rendererInfo.contentType = CONTENT_TYPE_MUSIC;
    config.rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    config.rendererInfo.rendererFlags = RENDERER_FLAGS;

    config.streamInfo.channels = STEREO;
    config.streamInfo.encoding = ENCODING_PCM;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_64000;

    processClient_ = AudioProcessInClient::Create(config);
    EXPECT_EQ(processClient_, nullptr);
}

/**
 * @tc.name  : Test AudioDeviceDescriptor API
 * @tc.type  : FUNC
 * @tc.number: AudioDeviceDescriptor_001
 * @tc.desc  : Test AudioDeviceDescriptor interface.
 */
HWTEST(AudioServiceUnitTest, AudioDeviceDescriptor_001, TestSize.Level1)
{
    DeviceType type = DeviceType::DEVICE_TYPE_SPEAKER;
    DeviceRole role = DeviceRole::OUTPUT_DEVICE;
    int32_t interruptGroupId = 1;
    int32_t volumeGroupId = 1;
    std::string networkId = "LocalDevice";
    std::unique_ptr<AudioDeviceDescriptor> audioDeviceDescriptor =
        std::make_unique<AudioDeviceDescriptor>(type, role, interruptGroupId, volumeGroupId, networkId);
    EXPECT_NE(audioDeviceDescriptor, nullptr);

    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = type;
    deviceDescriptor.deviceRole_ = role;
    audioDeviceDescriptor = std::make_unique<AudioDeviceDescriptor>(deviceDescriptor);
    EXPECT_NE(audioDeviceDescriptor, nullptr);

    std::string deviceName = "";
    std::string macAddress = "";
    audioDeviceDescriptor->SetDeviceInfo(deviceName, macAddress);

    DeviceStreamInfo audioStreamInfo = {
        SAMPLE_RATE_48000,
        ENCODING_PCM,
        SAMPLE_S16LE,
        STEREO
    };
    int32_t channelMask = 1;
    audioDeviceDescriptor->SetDeviceCapability(audioStreamInfo, channelMask);

    DeviceStreamInfo streamInfo = audioDeviceDescriptor->audioStreamInfo_;
    EXPECT_EQ(streamInfo.channels, audioStreamInfo.channels);
    EXPECT_EQ(streamInfo.encoding, audioStreamInfo.encoding);
    EXPECT_EQ(streamInfo.format, audioStreamInfo.format);
    EXPECT_EQ(streamInfo.samplingRate, audioStreamInfo.samplingRate);
}
} // namespace AudioStandard
} // namespace OHOS