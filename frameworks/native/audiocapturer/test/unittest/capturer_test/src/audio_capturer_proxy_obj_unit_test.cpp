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

#include "audio_capturer_proxy_obj.h"
#include "audio_capturer.h"

#include <gtest/gtest.h>

using namespace std;
using namespace testing;
using namespace testing::ext;
using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {

// Mock class to stub all pure virtual methods of abstract AudioCapturer for unit testing
class MockAudioCapturer : public AudioCapturer {
public:
    int32_t SetParams(const AudioCapturerParams params) override { return 0; }
    int32_t UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config) override { return 0; }
    int32_t SetCapturerCallback(const std::shared_ptr<AudioCapturerCallback> &callback) override { return 0; }
    void SetAudioCapturerErrorCallback(std::shared_ptr<AudioCapturerErrorCallback> errorCallback) override {}
    void SetFastStatusChangeCallback(
        const std::shared_ptr<AudioCapturerFastStatusChangeCallback> &callback) override {}
    int32_t GetParams(AudioCapturerParams &params) const override { return 0; }
    int32_t GetCapturerInfo(AudioCapturerInfo &capturerInfo) const override { return 0; }
    int32_t GetStreamInfo(AudioStreamInfo &streamInfo) const override { return 0; }
    bool Start() override { return true; }
    int32_t Read(uint8_t &buffer, size_t userSize, bool isBlockingRead) override { return 0; }
    CapturerState GetStatus() const override { return CAPTURER_NEW; }
    bool GetTimeStampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const override { return false; }
    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const override { return false; }
    bool Pause() const override { return true; }
    bool Stop() const override { return true; }
    bool Flush() const override { return true; }
    bool Release() override { return true; }
    int32_t GetBufferSize(size_t &bufferSize) const override { return 0; }
    int32_t GetAudioStreamId(uint32_t &sessionID) const override { return 0; }
    int32_t GetFrameCount(uint32_t &frameCount) const override { return 0; }
    int32_t SetCapturerPositionCallback(int64_t markPosition,
        const std::shared_ptr<CapturerPositionCallback> &callback) override { return 0; }
    void UnsetCapturerPositionCallback() override {}
    int32_t SetCapturerPeriodPositionCallback(int64_t frameNumber,
        const std::shared_ptr<CapturerPeriodPositionCallback> &callback) override { return 0; }
    void UnsetCapturerPeriodPositionCallback() override {}
    int32_t RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
        const std::shared_ptr<AudioCapturerPolicyServiceDiedCallback> &callback) override { return 0; }
    int32_t SetBufferDuration(uint64_t bufferDuration) const override { return 0; }
    int32_t SetCaptureMode(AudioCaptureMode captureMode) override { return 0; }
    AudioCaptureMode GetCaptureMode() const override { return CAPTURE_MODE_NORMAL; }
    int32_t SetCapturerReadCallback(
        const std::shared_ptr<AudioCapturerReadCallback> &callback) override { return 0; }
    int32_t GetBufferDesc(BufferDesc &bufDesc) override { return 0; }
    int32_t Enqueue(const BufferDesc &bufDesc) override { return 0; }
    int32_t Clear() const override { return 0; }
    int32_t GetBufQueueState(BufferQueueState &bufState) const override { return 0; }
    void SetValid(bool valid) override {}
    int64_t GetFramesRead() const override { return 0; }
    int32_t SetAudioCapturerDeviceChangeCallback(
        const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback) override { return 0; }
    int32_t RemoveAudioCapturerDeviceChangeCallback(
        const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback) override { return 0; }
    int32_t SetAudioCapturerInfoChangeCallback(
        const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback) override { return 0; }
    int32_t RemoveAudioCapturerInfoChangeCallback(
        const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback) override { return 0; }
    int32_t RegisterAudioCapturerEventListener() override { return 0; }
    int32_t UnregisterAudioCapturerEventListener() override { return 0; }
    int32_t GetCurrentInputDevices(AudioDeviceDescriptor &deviceInfo) const override { return 0; }
    int32_t GetCurrentCapturerChangeInfo(AudioCapturerChangeInfo &changeInfo) const override { return 0; }
    std::vector<sptr<MicrophoneDescriptor>> GetCurrentMicrophones() const override { return {}; }
    int32_t GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const override { return 0; }
    uint32_t GetOverflowCount() const override { return 0; }
    int32_t SetInputDevice(DeviceType deviceType) const override { return 0; }
    int32_t SetAudioSourceConcurrency(const std::vector<SourceType> &targetSources) override { return 0; }
    int32_t SetInterruptStrategy(InterruptStrategy strategy) override { return 0; }
};

class AudioCapturerProxyObjUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp(void) {}
    void TearDown(void) {}
};

/**
 * @tc.name  : Test AudioCapturerProxyObj SaveCapturerObj
 * @tc.number: AudioCapturerProxyObj_SaveCapturerObj_001
 * @tc.desc  : Test SaveCapturerObj with valid parameter.
 */
HWTEST(AudioCapturerProxyObjUnitTest, AudioCapturerProxyObj_SaveCapturerObj_001, TestSize.Level0)
{
    AudioCapturerProxyObj proxyObj;
    auto capturer = std::make_shared<MockAudioCapturer>();
    std::weak_ptr<const AudioCapturer> weakCapturer(capturer);
    
    proxyObj.SaveCapturerObj(weakCapturer);
    
    // Should not crash
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioCapturerProxyObj MuteStreamImpl
 * @tc.number: AudioCapturerProxyObj_MuteStreamImpl_001
 * @tc.desc  : Test MuteStreamImpl.
 */
HWTEST(AudioCapturerProxyObjUnitTest, AudioCapturerProxyObj_MuteStreamImpl_001, TestSize.Level0)
{
    AudioCapturerProxyObj proxyObj;
    StreamSetStateEventInternal event;
    
    proxyObj.MuteStreamImpl(event);
    
    // Should not crash (empty implementation)
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioCapturerProxyObj UnmuteStreamImpl
 * @tc.number: AudioCapturerProxyObj_UnmuteStreamImpl_001
 * @tc.desc  : Test UnmuteStreamImpl.
 */
HWTEST(AudioCapturerProxyObjUnitTest, AudioCapturerProxyObj_UnmuteStreamImpl_001, TestSize.Level0)
{
    AudioCapturerProxyObj proxyObj;
    StreamSetStateEventInternal event;
    
    proxyObj.UnmuteStreamImpl(event);
    
    // Should not crash (empty implementation)
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioCapturerProxyObj PausedStreamImpl
 * @tc.number: AudioCapturerProxyObj_PausedStreamImpl_001
 * @tc.desc  : Test PausedStreamImpl.
 */
HWTEST(AudioCapturerProxyObjUnitTest, AudioCapturerProxyObj_PausedStreamImpl_001, TestSize.Level0)
{
    AudioCapturerProxyObj proxyObj;
    StreamSetStateEventInternal event;
    
    proxyObj.PausedStreamImpl(event);
    
    // Should not crash (empty implementation)
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioCapturerProxyObj ResumeStreamImpl
 * @tc.number: AudioCapturerProxyObj_ResumeStreamImpl_001
 * @tc.desc  : Test ResumeStreamImpl.
 */
HWTEST(AudioCapturerProxyObjUnitTest, AudioCapturerProxyObj_ResumeStreamImpl_001, TestSize.Level0)
{
    AudioCapturerProxyObj proxyObj;
    StreamSetStateEventInternal event;
    
    proxyObj.ResumeStreamImpl(event);
    
    // Should not crash (empty implementation)
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioCapturerProxyObj SetLowPowerVolumeImpl
 * @tc.number: AudioCapturerProxyObj_SetLowPowerVolumeImpl_001
 * @tc.desc  : Test SetLowPowerVolumeImpl.
 */
HWTEST(AudioCapturerProxyObjUnitTest, AudioCapturerProxyObj_SetLowPowerVolumeImpl_001, TestSize.Level0)
{
    AudioCapturerProxyObj proxyObj;
    
    proxyObj.SetLowPowerVolumeImpl(0.5f);
    
    // Should not crash (empty implementation)
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test AudioCapturerProxyObj GetLowPowerVolumeImpl
 * @tc.number: AudioCapturerProxyObj_GetLowPowerVolumeImpl_001
 * @tc.desc  : Test GetLowPowerVolumeImpl.
 */
HWTEST(AudioCapturerProxyObjUnitTest, AudioCapturerProxyObj_GetLowPowerVolumeImpl_001, TestSize.Level0)
{
    AudioCapturerProxyObj proxyObj;
    float volume = 0.0f;
    
    proxyObj.GetLowPowerVolumeImpl(volume);
    
    // Should set volume to 1.0
    EXPECT_FLOAT_EQ(volume, 1.0f);
}

/**
 * @tc.name  : Test AudioCapturerProxyObj GetSingleStreamVolumeImpl
 * @tc.number: AudioCapturerProxyObj_GetSingleStreamVolumeImpl_001
 * @tc.desc  : Test GetSingleStreamVolumeImpl.
 */
HWTEST(AudioCapturerProxyObjUnitTest, AudioCapturerProxyObj_GetSingleStreamVolumeImpl_001, TestSize.Level0)
{
    AudioCapturerProxyObj proxyObj;
    float volume = 1.0f;
    
    proxyObj.GetSingleStreamVolumeImpl(volume);
    
    // Should set volume to 0
    EXPECT_FLOAT_EQ(volume, 0.0f);
}

} // namespace AudioStandard
} // namespace OHOS
