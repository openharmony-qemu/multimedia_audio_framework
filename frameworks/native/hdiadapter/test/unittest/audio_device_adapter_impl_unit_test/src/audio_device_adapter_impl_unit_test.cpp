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

#ifndef LOG_TAG
#define LOG_TAG "AudioDeviceAdapterImplUnitTest"
#endif

#include "audio_device_adapter_impl_unit_test.h"

#include <chrono>
#include <thread>

#include "audio_errors.h"
#include "audio_device_adapter_impl.h"
#include "i_audio_device_manager.h"

using namespace std;
using namespace std::chrono;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

namespace {
    std::string adapterName = "adapterName";
    sptr<IAudioAdapter> audioAdapter = nullptr;
    std::shared_ptr<AudioDeviceAdapterImpl> audioDeviceAdapterImpl =
        std::make_shared<AudioDeviceAdapterImpl>(adapterName, audioAdapter);
}

class IAudioAdapterFork : public IAudioAdapter {
public:
    DECLARE_HDI_DESCRIPTOR(u"ohos.hdi.distributed_audio.audio.v1_0.IAudioAdapter");

    IAudioAdapterFork() {};
    ~IAudioAdapterFork() {};

    int32_t InitAllPorts() override
    {
        return SUCCESS;
    };

    int32_t CreateRender(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDeviceDescriptor& desc,
        const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes& attrs,
            sptr<OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioRender>& render, uint32_t& renderId) override
    {
        return SUCCESS;
    };

    int32_t DestroyRender(uint32_t renderId) override
    {
        return SUCCESS;
    };

    int32_t CreateCapture(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDeviceDescriptor& desc,
        const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes& attrs,
            sptr<OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCapture>& capture, uint32_t& captureId) override
    {
        return SUCCESS;
    };

    int32_t DestroyCapture(uint32_t captureId) override
    {
        return SUCCESS;
    };

    int32_t GetPortCapability(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPort& port,
        OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortCapability& capability) override
    {
        return SUCCESS;
    };

    int32_t SetPassthroughMode(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPort& port,
        OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortPassthroughMode mode) override
    {
        return SUCCESS;
    };

    int32_t GetPassthroughMode(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPort& port,
        OHOS::HDI::DistributedAudio::Audio::V1_0::AudioPortPassthroughMode& mode) override
    {
        return SUCCESS;
    };

    int32_t GetDeviceStatus(OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDeviceStatus& status) override
    {
        return SUCCESS;
    };

    int32_t UpdateAudioRoute(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioRoute& route,
        int32_t& routeHandle) override
    {
        return SUCCESS;
    };

    int32_t ReleaseAudioRoute(int32_t routeHandle) override
    {
        return SUCCESS;
    };

    int32_t SetMicMute(bool mute) override
    {
        return SUCCESS;
    };

    int32_t GetMicMute(bool& mute) override
    {
        return SUCCESS;
    };

    int32_t SetVoiceVolume(float volume) override
    {
        return SUCCESS;
    };

    int32_t SetExtraParams(OHOS::HDI::DistributedAudio::Audio::V1_0::AudioExtParamKey key,
        const std::string& condition, const std::string& value) override
    {
        return SUCCESS;
    };

    int32_t GetExtraParams(OHOS::HDI::DistributedAudio::Audio::V1_0::AudioExtParamKey key,
        const std::string& condition, std::string& value) override
    {
        return SUCCESS;
    };

    int32_t RegExtraParamObserver(const sptr<OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCallback>& audioCallback,
        int8_t cookie) override
    {
        return SUCCESS;
    };

    int32_t GetVersion(uint32_t& majorVer, uint32_t& minorVer) override
    {
        majorVer = 1;
        minorVer = 0;
        return HDF_SUCCESS;
    }

    bool IsProxy() override
    {
        return false;
    }

    const std::u16string GetDesc() override
    {
        return metaDescriptor_;
    }
};

class IAudioRenderFork : public IAudioRender {
public:
    DECLARE_HDI_DESCRIPTOR(u"ohos.hdi.distributed_audio.audio.v1_0.IAudioRender");

    IAudioRenderFork() {};
    ~IAudioRenderFork() {};

    int32_t GetLatency(uint32_t& ms) override
    {
        return SUCCESS;
    };

    int32_t RenderFrame(const std::vector<int8_t>& frame, uint64_t& replyBytes) override
    {
        return SUCCESS;
    };

    int32_t GetRenderPosition(uint64_t& frames,
         OHOS::HDI::DistributedAudio::Audio::V1_0::AudioTimeStamp& time) override
    {
        return SUCCESS;
    };

    int32_t SetRenderSpeed(float speed) override
    {
        return SUCCESS;
    };

    int32_t GetRenderSpeed(float& speed) override
    {
        return SUCCESS;
    };

    int32_t SetChannelMode(OHOS::HDI::DistributedAudio::Audio::V1_0::AudioChannelMode mode) override
    {
        return SUCCESS;
    };

    int32_t GetChannelMode(OHOS::HDI::DistributedAudio::Audio::V1_0::AudioChannelMode& mode) override
    {
        return SUCCESS;
    };

    int32_t RegCallback(const sptr<OHOS::HDI::DistributedAudio::Audio::V1_0::IAudioCallback>& audioCallback,
        int8_t cookie) override
    {
        return SUCCESS;
    };

    int32_t DrainBuffer(OHOS::HDI::DistributedAudio::Audio::V1_0::AudioDrainNotifyType& type) override
    {
        return SUCCESS;
    };

    int32_t IsSupportsDrain(bool& support) override
    {
        return SUCCESS;
    };

    int32_t CheckSceneCapability(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSceneDescriptor& scene,
        bool& supported) override
    {
        return SUCCESS;
    };

    int32_t SelectScene(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSceneDescriptor& scene) override
    {
        return SUCCESS;
    };

    int32_t SetMute(bool mute) override
    {
        return SUCCESS;
    };

    int32_t GetMute(bool& mute) override
    {
        return SUCCESS;
    };

    int32_t SetVolume(float volume) override
    {
        return SUCCESS;
    };

    int32_t GetVolume(float& volume) override
    {
        return SUCCESS;
    };

    int32_t GetGainThreshold(float& min, float& max) override
    {
        return SUCCESS;
    };

    int32_t GetGain(float& gain) override
    {
        return SUCCESS;
    };

    int32_t SetGain(float gain) override
    {
        return SUCCESS;
    };

    int32_t GetFrameSize(uint64_t& size) override
    {
        return SUCCESS;
    };

    int32_t GetFrameCount(uint64_t& count) override
    {
        return SUCCESS;
    };

    int32_t SetSampleAttributes(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes& attrs) override
    {
        return SUCCESS;
    };

    int32_t GetSampleAttributes(OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes& attrs) override
    {
        return SUCCESS;
    };

    int32_t GetCurrentChannelId(uint32_t& channelId) override
    {
        return SUCCESS;
    };

    int32_t SetExtraParams(const std::string& keyValueList) override
    {
        return SUCCESS;
    };

    int32_t GetExtraParams(std::string& keyValueList) override
    {
        return SUCCESS;
    };

    int32_t ReqMmapBuffer(int32_t reqSize,
        OHOS::HDI::DistributedAudio::Audio::V1_0::AudioMmapBufferDescriptor& desc) override
    {
        return SUCCESS;
    };

    int32_t GetMmapPosition(uint64_t& frames,
        OHOS::HDI::DistributedAudio::Audio::V1_0::AudioTimeStamp& time) override
    {
        return SUCCESS;
    };

    int32_t AddAudioEffect(uint64_t effectid) override
    {
        return SUCCESS;
    };

    int32_t RemoveAudioEffect(uint64_t effectid) override
    {
        return SUCCESS;
    };

    int32_t GetFrameBufferSize(uint64_t& bufferSize) override
    {
        return SUCCESS;
    };

    int32_t Start() override
    {
        return SUCCESS;
    };

    int32_t Stop() override
    {
        return SUCCESS;
    };

    int32_t Pause() override
    {
        return SUCCESS;
    };

    int32_t Resume() override
    {
        return SUCCESS;
    };

    int32_t Flush() override
    {
        return SUCCESS;
    };

    int32_t TurnStandbyMode() override
    {
        return SUCCESS;
    };

    int32_t AudioDevDump(int32_t range, int32_t fd) override
    {
        return SUCCESS;
    };

    int32_t IsSupportsPauseAndResume(bool& supportPause, bool& supportResume) override
    {
        return SUCCESS;
    };

    int32_t GetVersion(uint32_t& majorVer, uint32_t& minorVer) override
    {
        majorVer = 1;
        minorVer = 0;
        return HDF_SUCCESS;
    }

    bool IsProxy() override
    {
        return false;
    }

    const std::u16string GetDesc() override
    {
        return metaDescriptor_;
    }
};

class IAudioCaptureFork : public IAudioCapture {
public:
    DECLARE_HDI_DESCRIPTOR(u"ohos.hdi.distributed_audio.audio.v1_0.IAudioCapture");

    IAudioCaptureFork() {};
    ~IAudioCaptureFork() {};

    int32_t CaptureFrame(std::vector<int8_t>& frame, uint64_t& replyBytes) override
    {
        return SUCCESS;
    };

    int32_t GetCapturePosition(uint64_t& frames,
         OHOS::HDI::DistributedAudio::Audio::V1_0::AudioTimeStamp& time) override
    {
        return SUCCESS;
    };

    int32_t CheckSceneCapability(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSceneDescriptor& scene,
        bool& supported) override
    {
        return SUCCESS;
    };

    int32_t SelectScene(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSceneDescriptor& scene) override
    {
        return SUCCESS;
    };

    int32_t SetMute(bool mute) override
    {
        return SUCCESS;
    };

    int32_t GetMute(bool& mute) override
    {
        return SUCCESS;
    };

    int32_t SetVolume(float volume) override
    {
        return SUCCESS;
    };

    int32_t GetVolume(float& volume) override
    {
        return SUCCESS;
    };

    int32_t GetGainThreshold(float& min, float& max) override
    {
        return SUCCESS;
    };

    int32_t GetGain(float& gain) override
    {
        return SUCCESS;
    };

    int32_t SetGain(float gain) override
    {
        return SUCCESS;
    };

    int32_t GetFrameSize(uint64_t& size) override
    {
        return SUCCESS;
    };

    int32_t GetFrameCount(uint64_t& count) override
    {
        return SUCCESS;
    };

    int32_t SetSampleAttributes(const OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes& attrs) override
    {
        return SUCCESS;
    };

    int32_t GetSampleAttributes(OHOS::HDI::DistributedAudio::Audio::V1_0::AudioSampleAttributes& attrs) override
    {
        return SUCCESS;
    };

    int32_t GetCurrentChannelId(uint32_t& channelId) override
    {
        return SUCCESS;
    };

    int32_t SetExtraParams(const std::string& keyValueList) override
    {
        return SUCCESS;
    };

    int32_t GetExtraParams(std::string& keyValueList) override
    {
        return SUCCESS;
    };

    int32_t ReqMmapBuffer(int32_t reqSize,
        OHOS::HDI::DistributedAudio::Audio::V1_0::AudioMmapBufferDescriptor& desc) override
    {
        return SUCCESS;
    };

    int32_t GetMmapPosition(uint64_t& frames,
        OHOS::HDI::DistributedAudio::Audio::V1_0::AudioTimeStamp& time) override
    {
        return SUCCESS;
    };

    int32_t AddAudioEffect(uint64_t effectid) override
    {
        return SUCCESS;
    };

    int32_t RemoveAudioEffect(uint64_t effectid) override
    {
        return SUCCESS;
    };

    int32_t GetFrameBufferSize(uint64_t& bufferSize) override
    {
        return SUCCESS;
    };

    int32_t Start() override
    {
        return SUCCESS;
    };

    int32_t Stop() override
    {
        return SUCCESS;
    };

    int32_t Pause() override
    {
        return SUCCESS;
    };

    int32_t Resume() override
    {
        return SUCCESS;
    };

    int32_t Flush() override
    {
        return SUCCESS;
    };

    int32_t TurnStandbyMode() override
    {
        return SUCCESS;
    };

    int32_t AudioDevDump(int32_t range, int32_t fd) override
    {
        return SUCCESS;
    };

    int32_t IsSupportsPauseAndResume(bool& supportPause, bool& supportResume) override
    {
        return SUCCESS;
    };

    int32_t GetVersion(uint32_t& majorVer, uint32_t& minorVer) override
    {
        majorVer = 1;
        minorVer = 0;
        return HDF_SUCCESS;
    }

    bool IsProxy() override
    {
        return false;
    }

    const std::u16string GetDesc() override
    {
        return metaDescriptor_;
    }
};

void AudioDeviceAdapterImplUnitTest::SetUpTestCase(void) {}
void AudioDeviceAdapterImplUnitTest::TearDownTestCase(void) {}
void AudioDeviceAdapterImplUnitTest::SetUp(void) {}
void AudioDeviceAdapterImplUnitTest::TearDown(void) {}

/**
* @tc.name   : Test HandleRenderParamEvent API
* @tc.number : HandleRenderParamEvent_001
* @tc.desc   : Test HandleRenderParamEvent interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, HandleRenderParamEvent_001, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    sptr<IAudioRender> render;
    DevicePortInfo portInfo;
    portInfo.devAdpCb = nullptr;
    audioDeviceAdapterImpl->renderPorts_.emplace(render, portInfo);
    const char *condition = "condition";
    const char *value = "value";
    int32_t result = audioDeviceAdapterImpl->HandleRenderParamEvent(audioDeviceAdapterImpl, VOLUME, condition, value);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test HandleRenderParamEvent API
* @tc.number : HandleRenderParamEvent_002
* @tc.desc   : Test HandleRenderParamEvent interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, HandleRenderParamEvent_002, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    const char *condition = "condition";
    const char *value = "value";
    int32_t result = audioDeviceAdapterImpl->HandleRenderParamEvent(audioDeviceAdapterImpl, VOLUME, condition, value);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test HandleCaptureParamEvent API
* @tc.number : HandleCaptureParamEvent_001
* @tc.desc   : Test HandleCaptureParamEvent interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, HandleCaptureParamEvent_001, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    sptr<IAudioCapture> capture;
    DevicePortInfo portInfo;
    portInfo.devAdpCb = nullptr;
    audioDeviceAdapterImpl->capturePorts_.emplace(capture, portInfo);
    const char *condition = "condition";
    const char *value = "value";
    int32_t result = audioDeviceAdapterImpl->HandleCaptureParamEvent(audioDeviceAdapterImpl, VOLUME, condition, value);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test HandleCaptureParamEvent API
* @tc.number : HandleCaptureParamEvent_002
* @tc.desc   : Test HandleCaptureParamEvent interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, HandleCaptureParamEvent_002, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    const char *condition = "condition";
    const char *value = "value";
    int32_t result = audioDeviceAdapterImpl->HandleCaptureParamEvent(audioDeviceAdapterImpl, VOLUME, condition, value);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test HandleStateChangeEvent API
* @tc.number : HandleStateChangeEvent_001
* @tc.desc   : Test HandleStateChangeEvent interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, HandleStateChangeEvent_001, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    const char *condition = "ERR_EVENT;DEVICE_TYPE=SPK";
    const char *value = nullptr;
    int32_t result = audioDeviceAdapterImpl->HandleStateChangeEvent(audioDeviceAdapterImpl, VOLUME, condition, value);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test HandleStateChangeEvent API
* @tc.number : HandleStateChangeEvent_002
* @tc.desc   : Test HandleStateChangeEvent interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, HandleStateChangeEvent_002, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    const char *condition = "ERR_EVENT;DEVICE_TYPE=1";
    const char *value = nullptr;
    int32_t result = audioDeviceAdapterImpl->HandleStateChangeEvent(audioDeviceAdapterImpl, VOLUME, condition, value);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test HandleStateChangeEvent API
* @tc.number : HandleStateChangeEvent_003
* @tc.desc   : Test HandleStateChangeEvent interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, HandleStateChangeEvent_003, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    const char *condition = "ERR_EVENT;DEVICE_TYPE=2";
    const char *value = nullptr;
    int32_t result = audioDeviceAdapterImpl->HandleStateChangeEvent(audioDeviceAdapterImpl, VOLUME, condition, value);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test ParamEventCallback API
* @tc.number : ParamEventCallback_001
* @tc.desc   : Test ParamEventCallback interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, ParamEventCallback_001, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    AudioParamKey key = PARAM_KEY_STATE;
    AudioExtParamKey extKey = AudioExtParamKey(key);
    const char *condition = "condition";
    const char *value = "value";
    void *reserved = nullptr;
    int32_t result = audioDeviceAdapterImpl->
        ParamEventCallback(extKey, condition, value, reserved, audioDeviceAdapterImpl);
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test ParamEventCallback API
* @tc.number : ParamEventCallback_002
* @tc.desc   : Test ParamEventCallback interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, ParamEventCallback_002, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    AudioParamKey key = VOLUME;
    AudioExtParamKey extKey = AudioExtParamKey(key);
    const char *condition = "condition";
    const char *value = "value";
    void *reserved = nullptr;
    int32_t result = audioDeviceAdapterImpl->
        ParamEventCallback(extKey, condition, value, reserved, audioDeviceAdapterImpl);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test ParamEventCallback API
* @tc.number : ParamEventCallback_003
* @tc.desc   : Test ParamEventCallback interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, ParamEventCallback_003, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    AudioParamKey key = INTERRUPT;
    AudioExtParamKey extKey = AudioExtParamKey(key);
    const char *condition = "condition";
    const char *value = "value";
    void *reserved = nullptr;
    int32_t result = audioDeviceAdapterImpl->
        ParamEventCallback(extKey, condition, value, reserved, audioDeviceAdapterImpl);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test ParamEventCallback API
* @tc.number : ParamEventCallback_004
* @tc.desc   : Test ParamEventCallback interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, ParamEventCallback_004, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    AudioParamKey key = NONE;
    AudioExtParamKey extKey = AudioExtParamKey(key);
    const char *condition = "condition";
    const char *value = "value";
    void *reserved = nullptr;
    int32_t result = audioDeviceAdapterImpl->
        ParamEventCallback(extKey, condition, value, reserved, audioDeviceAdapterImpl);
    EXPECT_NE(SUCCESS, result);
}

#ifdef FEATURE_DISTRIBUTE_AUDIO
/**
* @tc.name   : Test RegExtraParamObserver API
* @tc.number : RegExtraParamObserver_001
* @tc.desc   : Test RegExtraParamObserver interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, RegExtraParamObserver_001, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    audioDeviceAdapterImpl->isParamCbReg_ = true;
    int32_t result = audioDeviceAdapterImpl->RegExtraParamObserver();
    EXPECT_EQ(SUCCESS, result);
}
#endif

/**
* @tc.name   : Test RegExtraParamObserver API
* @tc.number : RegExtraParamObserver_002
* @tc.desc   : Test RegExtraParamObserver interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, RegExtraParamObserver_002, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    audioDeviceAdapterImpl->isParamCbReg_ = false;
    int32_t result = audioDeviceAdapterImpl->RegExtraParamObserver();
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test CreateRender API
* @tc.number : CreateRender_001
* @tc.desc   : Test CreateRender interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, CreateRender_001, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    audioDeviceAdapterImpl->audioAdapter_ = new IAudioAdapterFork();
    AudioDeviceDescriptor devDesc;
    AudioSampleAttributes attr;
    sptr<IAudioRender> audioRender = new IAudioRenderFork();
    IAudioDeviceAdapterCallback *renderCb = nullptr;
    uint32_t renderId = 0;
    int32_t result = audioDeviceAdapterImpl->CreateRender(devDesc, attr, audioRender, renderCb, renderId);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test CreateRender API
* @tc.number : CreateRender_002
* @tc.desc   : Test CreateRender interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, CreateRender_002, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    audioDeviceAdapterImpl->audioAdapter_ = new IAudioAdapterFork();
    DevicePortInfo portInfo;
    AudioDeviceDescriptor devDesc;
    AudioSampleAttributes attr;
    sptr<IAudioRender> audioRender = new IAudioRenderFork();
    IAudioDeviceAdapterCallback *renderCb = nullptr;
    uint32_t renderId = 0;
    audioDeviceAdapterImpl->renderPorts_.emplace(audioRender, portInfo);
    int32_t result = audioDeviceAdapterImpl->CreateRender(devDesc, attr, audioRender, renderCb, renderId);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test DestroyRender API
* @tc.number : DestroyRender_001
* @tc.desc   : Test DestroyRender interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, DestroyRender_001, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    sptr<IAudioRender> audioRender = new IAudioRenderFork();
    uint32_t renderId = 0;
    audioDeviceAdapterImpl->DestroyRender(audioRender, renderId);
    EXPECT_NE(audioDeviceAdapterImpl, nullptr);
}

/**
* @tc.name   : Test DestroyRender API
* @tc.number : DestroyRender_002
* @tc.desc   : Test DestroyRender interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, DestroyRender_002, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    DevicePortInfo portInfo;
    sptr<IAudioRender> audioRender = new IAudioRenderFork();
    uint32_t renderId = 0;
    audioDeviceAdapterImpl->renderPorts_.emplace(audioRender, portInfo);
    audioDeviceAdapterImpl->DestroyRender(audioRender, renderId);
    EXPECT_NE(audioDeviceAdapterImpl, nullptr);
}

/**
* @tc.name   : Test CreateCapture API
* @tc.number : CreateCapture_001
* @tc.desc   : Test CreateCapture interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, CreateCapture_001, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    audioDeviceAdapterImpl->audioAdapter_ = new IAudioAdapterFork();
    DevicePortInfo portInfo;
    AudioDeviceDescriptor devDesc;
    AudioSampleAttributes attr;
    sptr<IAudioCapture> audioCapture = new IAudioCaptureFork();
    IAudioDeviceAdapterCallback *captureCb = nullptr;
    uint32_t captureId = 0;
    audioDeviceAdapterImpl->capturePorts_.emplace(audioCapture, portInfo);
    int32_t result = audioDeviceAdapterImpl->CreateCapture(devDesc, attr, audioCapture, captureCb, captureId);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test CreateCapture API
* @tc.number : CreateCapture_002
* @tc.desc   : Test CreateCapture interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, CreateCapture_002, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    audioDeviceAdapterImpl->audioAdapter_ = new IAudioAdapterFork();
    AudioDeviceDescriptor devDesc;
    AudioSampleAttributes attr;
    sptr<IAudioCapture> audioCapture = new IAudioCaptureFork();
    IAudioDeviceAdapterCallback *captureCb = nullptr;
    uint32_t captureId = 0;
    int32_t result = audioDeviceAdapterImpl->CreateCapture(devDesc, attr, audioCapture, captureCb, captureId);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test DestroyCapture API
* @tc.number : DestroyCapture_001
* @tc.desc   : Test DestroyCapture interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, DestroyCapture_001, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    sptr<IAudioCapture> audioCapture = new IAudioCaptureFork();
    uint32_t captureId = 0;
    audioDeviceAdapterImpl->DestroyCapture(audioCapture, captureId);
    EXPECT_NE(audioDeviceAdapterImpl, nullptr);
}

/**
* @tc.name   : Test DestroyCapture API
* @tc.number : DestroyCapture_002
* @tc.desc   : Test DestroyCapture interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, DestroyCapture_002, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    sptr<IAudioCapture> audioCapture = new IAudioCaptureFork();
    DevicePortInfo portInfo;
    uint32_t captureId = 0;
    audioDeviceAdapterImpl->capturePorts_.emplace(audioCapture, portInfo);
    audioDeviceAdapterImpl->DestroyCapture(audioCapture, captureId);
    EXPECT_NE(audioDeviceAdapterImpl, nullptr);
}

/**
* @tc.name   : Test Release API
* @tc.number : Release_001
* @tc.desc   : Test Release interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, Release_001, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    audioDeviceAdapterImpl->audioAdapter_ = nullptr;
    int32_t result = audioDeviceAdapterImpl->Release();
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test Release API
* @tc.number : Release_002
* @tc.desc   : Test Release interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, Release_002, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    audioDeviceAdapterImpl->audioAdapter_ = new IAudioAdapterFork();
    int32_t result = audioDeviceAdapterImpl->Release();
    EXPECT_NE(SUCCESS, result);
}

/**
* @tc.name   : Test Release API
* @tc.number : Release_003
* @tc.desc   : Test Release interface.
*/
HWTEST(AudioDeviceAdapterImplUnitTest, Release_003, TestSize.Level1)
{
    ASSERT_TRUE(audioDeviceAdapterImpl != nullptr);
    audioDeviceAdapterImpl->audioAdapter_ = new IAudioAdapterFork();
    audioDeviceAdapterImpl->routeHandle_ = 0;
    int32_t result = audioDeviceAdapterImpl->Release();
    EXPECT_NE(SUCCESS, result);
}
} // namespace AudioStandard
} // namespace OHOS