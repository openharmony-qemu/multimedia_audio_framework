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

#include "audio_renderer_unit_test.h"

#include <chrono>
#include <cinttypes>
#include <thread>

#include "aiut_log.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_renderer_proxy_obj.h"
#include "audio_policy_manager.h"
#include "audio_renderer_private.h"
#include "audio_stream_enum.h"
#include "audio_debug_info.h"
#include "audio_buffer_desc.h"
#include "securec.h"

using namespace testing::ext;

namespace {
constexpr int32_t TEST_SESSION_ID = 1000;
constexpr int32_t TEST_LOOP_TIMES = 10;
constexpr int32_t TEST_FRAME_SIZE = 1024;
constexpr int32_t TEST_HAPTICS_SYNC_ID = 100;
constexpr int32_t TEST_TARGET_UID = 1000;
constexpr int32_t TEST_TARGET_STREAM_ID = 2000;
constexpr int64_t TEST_SOURCE_DURATION = 10000;
constexpr uint32_t TEST_SAMPLING_RATE_48K = 48000;
constexpr float TEST_VOLUME_HALF = 0.5f;
constexpr float TEST_SPEED_NORMAL = 1.0f;
constexpr float TEST_SPEED_TOO_SMALL = 0.01f;
constexpr float TEST_SPEED_TOO_LARGE = 100.0f;
constexpr float TEST_PITCH_TOO_SMALL = 0.01f;
constexpr float TEST_PITCH_TOO_LARGE = 100.0f;
constexpr float TEST_LOUDNESS_TOO_LARGE = 100.0f;
constexpr float TEST_LOUDNESS_TOO_SMALL = -100.0f;
constexpr int32_t TEST_VOLUME_RAMP_DURATION = 100;
constexpr int32_t TEST_OFFLOAD_MODE = 1;
constexpr int32_t TEST_VOLUME_MODE = 1;
constexpr int32_t TEST_BUFFER_LENGTH = 100;
constexpr int32_t TEST_BUFFER_DATA_LENGTH = 50;
constexpr int32_t TEST_BUFFER_INDIVISIBLE_LENGTH = 7;
constexpr int32_t TEST_INVALID_ENUM = 999;
}

namespace OHOS {
namespace AudioStandard {

class AudioRendererFirstFrameWritingCallbackTest : public AudioRendererFirstFrameWritingCallback {
public:
    void OnFirstFrameWriting(uint64_t latency) override {}
};

class AudioRendererErrorCallbackTest : public AudioRendererErrorCallback {
public:
    void OnError(AudioErrors errorCode) override {}
};

class AudioRendererOutputDeviceChangeCallbackTest : public AudioRendererOutputDeviceChangeCallback {
public:
    void OnOutputDeviceChange(const AudioDeviceDescriptor &deviceInfo,
        const AudioStreamDeviceChangeReason reason) override {}
};

/**
 * @tc.name  : Test SetAudioPrivacyType API via legal input.
 * @tc.number: AudioRenderer_SetAudioPrivacyType_Public
 * @tc.desc  : Test SetAudioPrivacyType interface. Verify privacy type is set correctly.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetAudioPrivacyType_Public, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    audioRenderer->SetAudioPrivacyType(AudioPrivacyType::PRIVACY_TYPE_PUBLIC);
    AudioPrivacyType privacyType = audioRenderer->GetAudioPrivacyType();
    AIUT_EXPECT_EQ_INFO_LOG(privacyType, AudioPrivacyType::PRIVACY_TYPE_PUBLIC,
        "SetAudioPrivacyType failed, privacyType=%d", privacyType);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetAudioPrivacyType API with PRIVATE type.
 * @tc.number: AudioRenderer_SetAudioPrivacyType_Private
 * @tc.desc  : Test SetAudioPrivacyType interface with PRIVACY_TYPE_PRIVATE.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetAudioPrivacyType_Private, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    audioRenderer->SetAudioPrivacyType(AudioPrivacyType::PRIVACY_TYPE_PRIVATE);
    AudioPrivacyType privacyType = audioRenderer->GetAudioPrivacyType();
    AIUT_EXPECT_EQ_INFO_LOG(privacyType, AudioPrivacyType::PRIVACY_TYPE_PRIVATE,
        "SetAudioPrivacyType PRIVATE failed, privacyType=%d", privacyType);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioPrivacyType API via legal input.
 * @tc.number: AudioRenderer_GetAudioPrivacyType_Valid
 * @tc.desc  : Test GetAudioPrivacyType interface. Returns default privacy type.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_GetAudioPrivacyType_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    rendererOptions.privacyType = AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    AudioPrivacyType privacyType = audioRenderer->GetAudioPrivacyType();
    AIUT_EXPECT_EQ_INFO_LOG(privacyType, AudioPrivacyType::PRIVACY_TYPE_PUBLIC,
        "GetAudioPrivacyType failed, privacyType=%d", privacyType);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetLoopTimes API on non-static renderer.
 * @tc.number: AudioRenderer_SetLoopTimes_NonStatic
 * @tc.desc  : Test SetLoopTimes interface on non-static renderer. Returns ERR_NOT_SUPPORTED.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetLoopTimes_NonStatic, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetLoopTimes(TEST_LOOP_TIMES);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED, "SetLoopTimes should return ERR_NOT_SUPPORTED, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetLoopTimes API with INT64_MAX.
 * @tc.number: AudioRenderer_SetLoopTimes_Int64Max
 * @tc.desc  : Test SetLoopTimes interface with INT64_MAX value.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetLoopTimes_Int64Max, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetLoopTimes(INT64_MAX);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED, "SetLoopTimes should return ERR_NOT_SUPPORTED, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetLoopTimes API with negative value.
 * @tc.number: AudioRenderer_SetLoopTimes_Negative
 * @tc.desc  : Test SetLoopTimes interface with negative value.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetLoopTimes_Negative, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetLoopTimes(-1);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED, "SetLoopTimes should return ERR_NOT_SUPPORTED, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetLoudnessGain API with invalid stream usage.
 * @tc.number: AudioRenderer_SetLoudnessGain_UnsupportedUsage
 * @tc.desc  : Test SetLoudnessGain with unsupported stream usage. Returns ERROR_UNSUPPORTED.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetLoudnessGain_UnsupportedUsage, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_RINGTONE;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_NOTIFICATION_RINGTONE;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetLoudnessGain(1.0f);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR_UNSUPPORTED, "SetLoudnessGain should return ERROR_UNSUPPORTED, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetLoudnessGain API with invalid loudness value.
 * @tc.number: AudioRenderer_SetLoudnessGain_TooLarge
 * @tc.desc  : Test SetLoudnessGain with out-of-range value. Returns ERROR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetLoudnessGain_TooLarge, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MUSIC;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetLoudnessGain(TEST_LOUDNESS_TOO_LARGE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR_INVALID_PARAM, "Should return ERROR_INVALID_PARAM, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetLoudnessGain API with too small value.
 * @tc.number: AudioRenderer_SetLoudnessGain_TooSmall
 * @tc.desc  : Test SetLoudnessGain with value below minimum. Returns ERROR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetLoudnessGain_TooSmall, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_MUSIC;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_MUSIC;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetLoudnessGain(TEST_LOUDNESS_TOO_SMALL);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR_INVALID_PARAM, "Should return ERROR_INVALID_PARAM, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetLoudnessGain API with unsupported stream usage.
 * @tc.number: AudioRenderer_GetLoudnessGain_UnsupportedUsage
 * @tc.desc  : Test GetLoudnessGain with unsupported stream usage. Returns 0.0f.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_GetLoudnessGain_UnsupportedUsage, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    rendererOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_44100;
    rendererOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    rendererOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    rendererOptions.streamInfo.channels = AudioChannel::STEREO;
    rendererOptions.rendererInfo.contentType = ContentType::CONTENT_TYPE_RINGTONE;
    rendererOptions.rendererInfo.streamUsage = StreamUsage::STREAM_USAGE_NOTIFICATION_RINGTONE;
    rendererOptions.rendererInfo.rendererFlags = RenderUT::RENDERER_FLAG;
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    float gain = audioRenderer->GetLoudnessGain();
    AIUT_EXPECT_EQ_INFO_LOG(gain, 0.0f, "GetLoudnessGain should return 0.0f, gain=%f", gain);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetRendererSamplingRate API via legal input.
 * @tc.number: AudioRenderer_SetRendererSamplingRate_Valid
 * @tc.desc  : Test SetRendererSamplingRate interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetRendererSamplingRate_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetRendererSamplingRate(TEST_SAMPLING_RATE_48K);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetRendererSamplingRate failed, ret=%d", ret);

    uint32_t rate = audioRenderer->GetRendererSamplingRate();
    AIUT_EXPECT_EQ_INFO_LOG(rate, TEST_SAMPLING_RATE_48K, "GetRendererSamplingRate mismatch, rate=%u", rate);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetChannelBlendMode API via legal input.
 * @tc.number: AudioRenderer_SetChannelBlendMode_Valid
 * @tc.desc  : Test SetChannelBlendMode interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetChannelBlendMode_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetChannelBlendMode(ChannelBlendMode::MODE_DEFAULT);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetChannelBlendMode failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetInterruptMode API with invalid mode.
 * @tc.number: AudioRenderer_SetInterruptMode_InvalidMode
 * @tc.desc  : Test SetInterruptMode with invalid mode. Verify no crash and mode not changed.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetInterruptMode_InvalidMode, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    audioRenderer->SetInterruptMode(static_cast<InterruptMode>(TEST_INVALID_ENUM));
    AIUT_INFO_LOG("SetInterruptMode invalid mode completed");
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetSilentModeAndMixWithOthers API via legal input.
 * @tc.number: AudioRenderer_SetSilentModeAndMixWithOthers_Valid
 * @tc.desc  : Test SetSilentModeAndMixWithOthers with true. Verify no crash.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetSilentModeAndMixWithOthers_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    audioRenderer->SetSilentModeAndMixWithOthers(true);
    bool silentMode = audioRenderer->GetSilentModeAndMixWithOthers();
    AIUT_EXPECT_EQ_INFO_LOG(silentMode, true, "GetSilentModeAndMixWithOthers mismatch, val=%d", silentMode);

    audioRenderer->SetSilentModeAndMixWithOthers(false);
    silentMode = audioRenderer->GetSilentModeAndMixWithOthers();
    AIUT_EXPECT_EQ_INFO_LOG(silentMode, false, "GetSilentModeAndMixWithOthers mismatch, val=%d", silentMode);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetParallelPlayFlag API via legal input.
 * @tc.number: AudioRenderer_SetParallelPlayFlag_Valid
 * @tc.desc  : Test SetParallelPlayFlag with true and false. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetParallelPlayFlag_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetParallelPlayFlag(true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetParallelPlayFlag true failed, ret=%d", ret);

    ret = audioRenderer->SetParallelPlayFlag(false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetParallelPlayFlag false failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetLowPowerVolume API via legal input.
 * @tc.number: AudioRenderer_SetLowPowerVolume_Valid
 * @tc.desc  : Test SetLowPowerVolume interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetLowPowerVolume_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetLowPowerVolume(TEST_VOLUME_HALF);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetLowPowerVolume failed, ret=%d", ret);

    float volume = audioRenderer->GetLowPowerVolume();
    AIUT_EXPECT_EQ_INFO_LOG(volume, TEST_VOLUME_HALF, "GetLowPowerVolume mismatch, volume=%f", volume);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetOffloadAllowed API via legal input.
 * @tc.number: AudioRenderer_SetOffloadAllowed_Valid
 * @tc.desc  : Test SetOffloadAllowed interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetOffloadAllowed_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetOffloadAllowed(true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetOffloadAllowed true failed, ret=%d", ret);

    ret = audioRenderer->SetOffloadAllowed(false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetOffloadAllowed false failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetSingleStreamVolume API via legal input.
 * @tc.number: AudioRenderer_GetSingleStreamVolume_Valid
 * @tc.desc  : Test GetSingleStreamVolume interface. Returns valid volume.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_GetSingleStreamVolume_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    float volume = audioRenderer->GetSingleStreamVolume();
    AIUT_EXPECT_TRUE_INFO_LOG(volume >= 0.0f && volume <= 1.0f,
        "GetSingleStreamVolume out of range, volume=%f", volume);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetMinStreamVolume API via legal input.
 * @tc.number: AudioRenderer_GetMinStreamVolume_Valid
 * @tc.desc  : Test GetMinStreamVolume interface. Returns valid volume.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_GetMinStreamVolume_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    float volume = audioRenderer->GetMinStreamVolume();
    AIUT_EXPECT_TRUE_INFO_LOG(volume >= 0.0f, "GetMinStreamVolume invalid, volume=%f", volume);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetMaxStreamVolume API via legal input.
 * @tc.number: AudioRenderer_GetMaxStreamVolume_Valid
 * @tc.desc  : Test GetMaxStreamVolume interface. Returns valid volume.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_GetMaxStreamVolume_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    float volume = audioRenderer->GetMaxStreamVolume();
    AIUT_EXPECT_TRUE_INFO_LOG(volume >= 0.0f, "GetMaxStreamVolume invalid, volume=%f", volume);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetUnderflowCount API via legal input.
 * @tc.number: AudioRenderer_GetUnderflowCount_Valid
 * @tc.desc  : Test GetUnderflowCount interface. Returns valid count.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_GetUnderflowCount_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    uint32_t count = audioRenderer->GetUnderflowCount();
    AIUT_EXPECT_TRUE_INFO_LOG(count >= 0, "GetUnderflowCount invalid, count=%u", count);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetAudioRendererErrorCallback API via legal input.
 * @tc.number: AudioRenderer_SetAudioRendererErrorCallback_Valid
 * @tc.desc  : Test SetAudioRendererErrorCallback interface. Verify no crash.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetAudioRendererErrorCallback_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    std::shared_ptr<AudioRendererErrorCallback> callback = std::make_shared<AudioRendererErrorCallbackTest>();
    audioRenderer->SetAudioRendererErrorCallback(callback);
    AIUT_INFO_LOG("SetAudioRendererErrorCallback completed");
    audioRenderer->Release();
}

/**
 * @tc.name  : Test RegisterAudioPolicyServerDiedCb API with null callback.
 * @tc.number: AudioRenderer_RegisterAudioPolicyServerDiedCb_NullCallback
 * @tc.desc  : Test RegisterAudioPolicyServerDiedCb with null callback. Returns ERR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_RegisterAudioPolicyServerDiedCb_NullCallback, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    std::shared_ptr<AudioRendererPolicyServiceDiedCallback> callback = nullptr;
    int32_t ret = audioRenderer->RegisterAudioPolicyServerDiedCb(TEST_SESSION_ID, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "Should return ERR_INVALID_PARAM, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test RegisterAudioPolicyServerDiedCb API via legal input.
 * @tc.number: AudioRenderer_RegisterAudioPolicyServerDiedCb_Valid
 * @tc.desc  : Test RegisterAudioPolicyServerDiedCb with valid callback. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_RegisterAudioPolicyServerDiedCb_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    std::shared_ptr<AudioRendererPolicyServiceDiedCallback> callback =
        std::make_shared<AudioRendererPolicyServiceDiedCallbackTest>();
    int32_t ret = audioRenderer->RegisterAudioPolicyServerDiedCb(TEST_SESSION_ID, callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RegisterAudioPolicyServerDiedCb failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test UnregisterAudioPolicyServerDiedCb API via legal input.
 * @tc.number: AudioRenderer_UnregisterAudioPolicyServerDiedCb_Valid
 * @tc.desc  : Test UnregisterAudioPolicyServerDiedCb interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_UnregisterAudioPolicyServerDiedCb_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->UnregisterAudioPolicyServerDiedCb(TEST_SESSION_ID);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "UnregisterAudioPolicyServerDiedCb failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test RegisterOutputDeviceChangeWithInfoCallback API with null callback.
 * @tc.number: AudioRenderer_RegisterOutputDeviceChangeWithInfoCallback_NullCallback
 * @tc.desc  : Test RegisterOutputDeviceChangeWithInfoCallback with null callback. Returns ERR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_RegisterOutputDeviceChangeWithInfoCallback_NullCallback, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    std::shared_ptr<AudioRendererOutputDeviceChangeCallback> callback = nullptr;
    int32_t ret = audioRenderer->RegisterOutputDeviceChangeWithInfoCallback(callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "Should return ERR_INVALID_PARAM, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test UnregisterOutputDeviceChangeWithInfoCallback API via legal input.
 * @tc.number: AudioRenderer_UnregisterOutputDeviceChangeWithInfoCallback_Valid
 * @tc.desc  : Test UnregisterOutputDeviceChangeWithInfoCallback interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_UnregisterOutputDeviceChangeWithInfoCallback_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->UnregisterOutputDeviceChangeWithInfoCallback();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "UnregisterOutputDeviceChangeWithInfoCallback failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetFramesWritten API via legal input.
 * @tc.number: AudioRenderer_GetFramesWritten_Valid
 * @tc.desc  : Test GetFramesWritten interface. Returns valid frame count.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_GetFramesWritten_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int64_t frames = audioRenderer->GetFramesWritten();
    AIUT_EXPECT_TRUE_INFO_LOG(frames >= 0, "GetFramesWritten invalid, frames=%" PRId64, frames);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetVolumeWithRamp API via legal input.
 * @tc.number: AudioRenderer_SetVolumeWithRamp_Valid
 * @tc.desc  : Test SetVolumeWithRamp interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetVolumeWithRamp_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetVolumeWithRamp(TEST_VOLUME_HALF, TEST_VOLUME_RAMP_DURATION);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetVolumeWithRamp failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetPreferredFrameSize API via legal input.
 * @tc.number: AudioRenderer_SetPreferredFrameSize_Valid
 * @tc.desc  : Test SetPreferredFrameSize interface. Verify no crash.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetPreferredFrameSize_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    audioRenderer->SetPreferredFrameSize(TEST_FRAME_SIZE);
    AIUT_INFO_LOG("SetPreferredFrameSize completed");
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetSpeed API with invalid speed (too small).
 * @tc.number: AudioRenderer_SetSpeed_TooSmall
 * @tc.desc  : Test SetSpeed with speed < MIN_STREAM_SPEED_LEVEL. Returns ERR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetSpeed_TooSmall, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetSpeed(TEST_SPEED_TOO_SMALL);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "SetSpeed should return ERR_INVALID_PARAM, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetSpeed API with invalid speed (too large).
 * @tc.number: AudioRenderer_SetSpeed_TooLarge
 * @tc.desc  : Test SetSpeed with speed > MAX_STREAM_SPEED_LEVEL. Returns ERR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetSpeed_TooLarge, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetSpeed(TEST_SPEED_TOO_LARGE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "SetSpeed should return ERR_INVALID_PARAM, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetPitch API with invalid pitch (too small).
 * @tc.number: AudioRenderer_SetPitch_TooSmall
 * @tc.desc  : Test SetPitch with pitch < MIN_STREAM_PITCH_LEVEL. Returns ERR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetPitch_TooSmall, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetPitch(TEST_PITCH_TOO_SMALL);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "SetPitch should return ERR_INVALID_PARAM, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetPitch API with invalid pitch (too large).
 * @tc.number: AudioRenderer_SetPitch_TooLarge
 * @tc.desc  : Test SetPitch with pitch > MAX_STREAM_PITCH_LEVEL. Returns ERR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetPitch_TooLarge, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetPitch(TEST_PITCH_TOO_LARGE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "SetPitch should return ERR_INVALID_PARAM, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetPitch API via legal input.
 * @tc.number: AudioRenderer_SetPitch_Valid
 * @tc.desc  : Test SetPitch with valid pitch (1.0). Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetPitch_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetPitch(TEST_SPEED_NORMAL);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetPitch failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test EnableVoiceModemCommunicationStartStream API.
 * @tc.number: AudioRenderer_EnableVoiceModemCommunicationStartStream_Valid
 * @tc.desc  : Test EnableVoiceModemCommunicationStartStream interface. Verify no crash.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_EnableVoiceModemCommunicationStartStream_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    audioRenderer->EnableVoiceModemCommunicationStartStream(true);
    bool isNoStream = audioRenderer->IsNoStreamRenderer();
    AIUT_EXPECT_EQ_INFO_LOG(isNoStream, false, "IsNoStreamRenderer should be false, val=%d", isNoStream);

    audioRenderer->EnableVoiceModemCommunicationStartStream(false);
    AIUT_INFO_LOG("EnableVoiceModemCommunicationStartStream completed");
    audioRenderer->Release();
}

/**
 * @tc.name  : Test IsFastRenderer API via legal input.
 * @tc.number: AudioRenderer_IsFastRenderer_Valid
 * @tc.desc  : Test IsFastRenderer interface. Returns false for normal renderer.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_IsFastRenderer_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    bool isFast = audioRenderer->IsFastRenderer();
    AIUT_EXPECT_EQ_INFO_LOG(isFast, false, "IsFastRenderer should be false, val=%d", isFast);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test IsOffloadEnable API via legal input.
 * @tc.number: AudioRenderer_IsOffloadEnable_Valid
 * @tc.desc  : Test IsOffloadEnable interface. Returns false by default.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_IsOffloadEnable_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    bool isOffload = audioRenderer->IsOffloadEnable();
    AIUT_EXPECT_EQ_INFO_LOG(isOffload, false, "IsOffloadEnable should be false, val=%d", isOffload);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetSourceDuration API via legal input.
 * @tc.number: AudioRenderer_SetSourceDuration_Valid
 * @tc.desc  : Test SetSourceDuration interface. Verify no crash.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetSourceDuration_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    audioRenderer->SetSourceDuration(TEST_SOURCE_DURATION);
    AIUT_INFO_LOG("SetSourceDuration completed");
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetAudioHapticsSyncId API via legal input.
 * @tc.number: AudioRenderer_SetAudioHapticsSyncId_Valid
 * @tc.desc  : Test SetAudioHapticsSyncId interface. Verify no crash.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetAudioHapticsSyncId_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    audioRenderer->SetAudioHapticsSyncId(TEST_HAPTICS_SYNC_ID);
    AIUT_INFO_LOG("SetAudioHapticsSyncId completed");
    audioRenderer->Release();
}

/**
 * @tc.name  : Test ResetFirstFrameState API via legal input.
 * @tc.number: AudioRenderer_ResetFirstFrameState_Valid
 * @tc.desc  : Test ResetFirstFrameState interface. Verify no crash.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_ResetFirstFrameState_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    audioRenderer->ResetFirstFrameState();
    AIUT_INFO_LOG("ResetFirstFrameState completed");
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetInterruptEventCallbackType API via legal input.
 * @tc.number: AudioRenderer_SetInterruptEventCallbackType_Valid
 * @tc.desc  : Test SetInterruptEventCallbackType interface. Verify no crash.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetInterruptEventCallbackType_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    audioRenderer->SetInterruptEventCallbackType(InterruptEventCallbackType::INTERRUPT_EVENT_CALLBACK_COMBINED);
    AIUT_INFO_LOG("SetInterruptEventCallbackType completed");
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetIndependentAudioSessionStrategy API via legal input.
 * @tc.number: AudioRenderer_SetIndependentAudioSessionStrategy_PauseWhenInterrupted
 * @tc.desc  : Test SetIndependentAudioSessionStrategy interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetIndependentAudioSessionStrategy_PauseWhenInterrupted, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    AudioSessionStrategy strategy;
    strategy.concurrencyMode = AudioConcurrencyMode::DEFAULT;
    int32_t ret = audioRenderer->SetIndependentAudioSessionStrategy(strategy,
        static_cast<uint32_t>(AudioSessionBehaviorFlags::PAUSE_WHEN_INTERRUPTED));
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetIndependentAudioSessionStrategy failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetIndependentAudioSessionStrategy API with MUTE_WHEN_INTERRUPTED.
 * @tc.number: AudioRenderer_SetIndependentAudioSessionStrategy_MuteWhenInterrupted
 * @tc.desc  : Test SetIndependentAudioSessionStrategy with MUTE_WHEN_INTERRUPTED flag. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetIndependentAudioSessionStrategy_MuteWhenInterrupted, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    AudioSessionStrategy strategy;
    strategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    int32_t ret = audioRenderer->SetIndependentAudioSessionStrategy(strategy,
        static_cast<uint32_t>(AudioSessionBehaviorFlags::MUTE_WHEN_INTERRUPTED));
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetIndependentAudioSessionStrategy failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetMainUseArk API via legal input.
 * @tc.number: AudioRenderer_SetMainUseArk_Valid
 * @tc.desc  : Test SetMainUseArk interface. Returns SUCCESS on first call.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetMainUseArk_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetMainUseArk(true);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetMainUseArk first call failed, ret=%d", ret);

    ret = audioRenderer->SetMainUseArk(false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR_ILLEGAL_STATE, "SetMainUseArk second call should fail, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetAudioTimestampInfo API via legal input.
 * @tc.number: AudioRenderer_GetAudioTimestampInfo_Valid
 * @tc.desc  : Test GetAudioTimestampInfo interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_GetAudioTimestampInfo_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    Timestamp timestamp;
    int32_t ret = audioRenderer->GetAudioTimestampInfo(timestamp, Timestamp::Timestampbase::MONOTONIC);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetAudioTimestampInfo failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetTarget API via legal input.
 * @tc.number: AudioRenderer_GetTarget_Valid
 * @tc.desc  : Test GetTarget interface. Returns default target.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_GetTarget_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    RenderTarget target = audioRenderer->GetTarget();
    AIUT_EXPECT_EQ_INFO_LOG(target, NORMAL_PLAYBACK, "GetTarget mismatch, target=%d", target);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetKeepRunning API via legal input.
 * @tc.number: AudioRenderer_GetKeepRunning_Valid
 * @tc.desc  : Test GetKeepRunning interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_GetKeepRunning_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    bool keepRunning = false;
    int32_t ret = audioRenderer->GetKeepRunning(keepRunning);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetKeepRunning failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetLatencyWithFlag API via legal input.
 * @tc.number: AudioRenderer_GetLatencyWithFlag_Valid
 * @tc.desc  : Test GetLatencyWithFlag interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_GetLatencyWithFlag_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    uint64_t latency = 0;
    int32_t ret = audioRenderer->GetLatencyWithFlag(latency, LatencyFlag::LATENCY_FLAG_ALL);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetLatencyWithFlag failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test CheckSupportedSamplingRates API with valid rates.
 * @tc.number: AudioRenderer_CheckSupportedSamplingRates_ValidRates
 * @tc.desc  : Test CheckSupportedSamplingRates with valid sampling rates. Returns true.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_CheckSupportedSamplingRates_ValidRates, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    bool ret = AudioRenderer::CheckSupportedSamplingRates(44100);
    AIUT_EXPECT_EQ_INFO_LOG(ret, true, "CheckSupportedSamplingRates 44100 should be true, ret=%d", ret);

    ret = AudioRenderer::CheckSupportedSamplingRates(48000);
    AIUT_EXPECT_EQ_INFO_LOG(ret, true, "CheckSupportedSamplingRates 48000 should be true, ret=%d", ret);

    ret = AudioRenderer::CheckSupportedSamplingRates(11025);
    AIUT_EXPECT_EQ_INFO_LOG(ret, true, "CheckSupportedSamplingRates 11025 should be true, ret=%d", ret);
}

/**
 * @tc.name  : Test CheckSupportedSamplingRates API with invalid rates.
 * @tc.number: AudioRenderer_CheckSupportedSamplingRates_InvalidRates
 * @tc.desc  : Test CheckSupportedSamplingRates with invalid sampling rates. Returns false.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_CheckSupportedSamplingRates_InvalidRates, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    bool ret = AudioRenderer::CheckSupportedSamplingRates(7999);
    AIUT_EXPECT_EQ_INFO_LOG(ret, false, "CheckSupportedSamplingRates 7999 should be false, ret=%d", ret);

    ret = AudioRenderer::CheckSupportedSamplingRates(384001);
    AIUT_EXPECT_EQ_INFO_LOG(ret, false, "CheckSupportedSamplingRates 384001 should be false, ret=%d", ret);

    ret = AudioRenderer::CheckSupportedSamplingRates(16001);
    AIUT_EXPECT_EQ_INFO_LOG(ret, false, "CheckSupportedSamplingRates 16001 should be false, ret=%d", ret);
}

/**
 * @tc.name  : Test MuteAudioBuffer API with null addr.
 * @tc.number: AudioRenderer_MuteAudioBuffer_NullAddr
 * @tc.desc  : Test MuteAudioBuffer with null addr. Returns ERR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_MuteAudioBuffer_NullAddr, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    int32_t ret = AudioRenderer::MuteAudioBuffer(nullptr, 0, TEST_BUFFER_LENGTH, SAMPLE_S16LE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "MuteAudioBuffer null addr should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test MuteAudioBuffer API with zero length.
 * @tc.number: AudioRenderer_MuteAudioBuffer_ZeroLength
 * @tc.desc  : Test MuteAudioBuffer with zero length. Returns ERR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_MuteAudioBuffer_ZeroLength, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_LENGTH] = {0};
    int32_t ret = AudioRenderer::MuteAudioBuffer(buffer, 0, 0, SAMPLE_S16LE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "MuteAudioBuffer zero length should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test MuteAudioBuffer API with invalid format.
 * @tc.number: AudioRenderer_MuteAudioBuffer_InvalidFormat
 * @tc.desc  : Test MuteAudioBuffer with invalid format. Returns ERR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_MuteAudioBuffer_InvalidFormat, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_LENGTH] = {0};
    int32_t ret = AudioRenderer::MuteAudioBuffer(buffer, 0, TEST_BUFFER_LENGTH,
        static_cast<AudioSampleFormat>(TEST_INVALID_ENUM));
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "MuteAudioBuffer invalid format should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test MuteAudioBuffer API via legal input with SAMPLE_U8.
 * @tc.number: AudioRenderer_MuteAudioBuffer_SampleU8
 * @tc.desc  : Test MuteAudioBuffer with SAMPLE_U8 format. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_MuteAudioBuffer_SampleU8, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_LENGTH] = {0};
    int32_t ret = AudioRenderer::MuteAudioBuffer(buffer, 0, TEST_BUFFER_LENGTH, SAMPLE_U8);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "MuteAudioBuffer SAMPLE_U8 failed, ret=%d", ret);
}

/**
 * @tc.name  : Test MuteAudioBuffer API via legal input with SAMPLE_S16LE.
 * @tc.number: AudioRenderer_MuteAudioBuffer_SampleS16LE
 * @tc.desc  : Test MuteAudioBuffer with SAMPLE_S16LE format. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_MuteAudioBuffer_SampleS16LE, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_LENGTH] = {0};
    int32_t ret = AudioRenderer::MuteAudioBuffer(buffer, 0, TEST_BUFFER_LENGTH, SAMPLE_S16LE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "MuteAudioBuffer SAMPLE_S16LE failed, ret=%d", ret);
}

/**
 * @tc.name  : Test MuteAudioBuffer API with length not divisible by format size.
 * @tc.number: AudioRenderer_MuteAudioBuffer_IndivisibleLength
 * @tc.desc  : Test MuteAudioBuffer with length not divisible by bitWidthSize. Returns ERR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_MuteAudioBuffer_IndivisibleLength, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    uint8_t buffer[TEST_BUFFER_LENGTH] = {0};
    int32_t ret = AudioRenderer::MuteAudioBuffer(buffer, 0, TEST_BUFFER_INDIVISIBLE_LENGTH, SAMPLE_S32LE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "MuteAudioBuffer indivisible length should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test FadeInAudioBuffer API with null buffer.
 * @tc.number: AudioRenderer_FadeInAudioBuffer_NullBuffer
 * @tc.desc  : Test FadeInAudioBuffer with null buffer. Returns ERR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_FadeInAudioBuffer_NullBuffer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    BufferDesc buffer = {nullptr, 0, 0};
    int32_t ret = AudioRenderer::FadeInAudioBuffer(buffer, SAMPLE_S16LE, STEREO);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "FadeInAudioBuffer null buffer should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test FadeOutAudioBuffer API with null buffer.
 * @tc.number: AudioRenderer_FadeOutAudioBuffer_NullBuffer
 * @tc.desc  : Test FadeOutAudioBuffer with null buffer. Returns ERR_INVALID_PARAM.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_FadeOutAudioBuffer_NullBuffer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    BufferDesc buffer = {nullptr, 0, 0};
    int32_t ret = AudioRenderer::FadeOutAudioBuffer(buffer, SAMPLE_S16LE, STEREO);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_PARAM, "FadeOutAudioBuffer null buffer should fail, ret=%d", ret);
}

/**
 * @tc.name  : Test FadeInAudioBuffer API with bufLength > dataLength.
 * @tc.number: AudioRenderer_FadeInAudioBuffer_BufLengthGreaterThanDataLength
 * @tc.desc  : Test FadeInAudioBuffer with bufLength > dataLength. Verify no crash.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_FadeInAudioBuffer_BufLengthGreaterThanDataLength, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    uint8_t data[TEST_BUFFER_LENGTH] = {0};
    BufferDesc buffer = {data, TEST_BUFFER_LENGTH, TEST_BUFFER_DATA_LENGTH};
    int32_t ret = AudioRenderer::FadeInAudioBuffer(buffer, SAMPLE_S16LE, STEREO);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "FadeInAudioBuffer failed, ret=%d", ret);
}

/**
 * @tc.name  : Test FadeOutAudioBuffer API with bufLength > dataLength.
 * @tc.number: AudioRenderer_FadeOutAudioBuffer_BufLengthGreaterThanDataLength
 * @tc.desc  : Test FadeOutAudioBuffer with bufLength > dataLength. Verify no crash.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_FadeOutAudioBuffer_BufLengthGreaterThanDataLength, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    uint8_t data[TEST_BUFFER_LENGTH] = {0};
    BufferDesc buffer = {data, TEST_BUFFER_LENGTH, TEST_BUFFER_DATA_LENGTH};
    int32_t ret = AudioRenderer::FadeOutAudioBuffer(buffer, SAMPLE_S16LE, STEREO);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "FadeOutAudioBuffer failed, ret=%d", ret);
}

/**
 * @tc.name  : Test Create API with AudioRendererOptions and AppInfo.
 * @tc.number: AudioRenderer_Create_WithAppInfo
 * @tc.desc  : Test Create with AudioRendererOptions and AppInfo. Returns valid renderer.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_Create_WithAppInfo, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    AppInfo appInfo = {};
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions, appInfo);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "Create with AppInfo returned nullptr");
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Create API with cachePath and AudioRendererOptions.
 * @tc.number: AudioRenderer_Create_WithCachePath
 * @tc.desc  : Test Create with cachePath and AudioRendererOptions. Returns valid renderer.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_Create_WithCachePath, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create("/data/test_cache", rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "Create with cachePath returned nullptr");
    audioRenderer->Release();
}

/**
 * @tc.name  : Test Create API with cachePath, AudioRendererOptions and AppInfo.
 * @tc.number: AudioRenderer_Create_WithCachePathAndAppInfo
 * @tc.desc  : Test Create with cachePath, AudioRendererOptions and AppInfo. Returns valid renderer.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_Create_WithCachePathAndAppInfo, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    AppInfo appInfo = {};
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create("/data/test_cache", rendererOptions, appInfo);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "Create with cachePath and AppInfo returned nullptr");
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetCurrentOutputDevices API via legal input.
 * @tc.number: AudioRenderer_GetCurrentOutputDevices_Valid
 * @tc.desc  : Test GetCurrentOutputDevices interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_GetCurrentOutputDevices_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    AudioDeviceDescriptor deviceInfo;
    int32_t ret = audioRenderer->GetCurrentOutputDevices(deviceInfo);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetCurrentOutputDevices failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test GetRendererDebugInfo API via legal input.
 * @tc.number: AudioRenderer_GetRendererDebugInfo_Valid
 * @tc.desc  : Test GetRendererDebugInfo interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_GetRendererDebugInfo_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    AudioRendererDebugInfo debugInfo;
    int32_t ret = audioRenderer->GetRendererDebugInfo(debugInfo);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "GetRendererDebugInfo failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetTarget API via legal input.
 * @tc.number: AudioRenderer_SetTarget_Valid
 * @tc.desc  : Test SetTarget interface with NORMAL_PLAYBACK. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetTarget_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetTarget(NORMAL_PLAYBACK);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetTarget failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetTarget API with targetUid and targetStreamId.
 * @tc.number: AudioRenderer_SetTarget_WithUidAndStreamId
 * @tc.desc  : Test SetTarget interface with targetUid and targetStreamId. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetTarget_WithUidAndStreamId, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetTarget(NORMAL_PLAYBACK, TEST_TARGET_UID, TEST_TARGET_STREAM_ID);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetTarget with uid/streamId failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SelectOutputDevice API via legal input.
 * @tc.number: AudioRenderer_SelectOutputDevice_Valid
 * @tc.desc  : Test SelectOutputDevice interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SelectOutputDevice_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t ret = audioRenderer->SelectOutputDevice(desc);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SelectOutputDevice failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetOffloadMode API via legal input.
 * @tc.number: AudioRenderer_SetOffloadMode_Valid
 * @tc.desc  : Test SetOffloadMode interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetOffloadMode_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetOffloadMode(TEST_OFFLOAD_MODE, false);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetOffloadMode failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test UnsetOffloadMode API via legal input.
 * @tc.number: AudioRenderer_UnsetOffloadMode_Valid
 * @tc.desc  : Test UnsetOffloadMode interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_UnsetOffloadMode_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->UnsetOffloadMode();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "UnsetOffloadMode failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetRendererFirstFrameWritingCallback API via legal input.
 * @tc.number: AudioRenderer_SetRendererFirstFrameWritingCallback_Valid
 * @tc.desc  : Test SetRendererFirstFrameWritingCallback interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetRendererFirstFrameWritingCallback_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    audioRenderer->SetRenderMode(RENDER_MODE_CALLBACK);
    std::shared_ptr<AudioRendererFirstFrameWritingCallback> callback =
        std::make_shared<AudioRendererFirstFrameWritingCallbackTest>();
    int32_t ret = audioRenderer->SetRendererFirstFrameWritingCallback(callback);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetRendererFirstFrameWritingCallback failed, ret=%d", ret);
    audioRenderer->Release();
}

/**
 * @tc.name  : Test SetVolumeMode API via legal input.
 * @tc.number: AudioRenderer_SetVolumeMode_Valid
 * @tc.desc  : Test SetVolumeMode interface. Returns SUCCESS.
 */
HWTEST(AudioRendererUnitTest, AudioRenderer_SetVolumeMode_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AudioRendererOptions rendererOptions;
    AudioRendererUnitTest::InitializeRendererOptions(rendererOptions);
    std::unique_ptr<AudioRenderer> audioRenderer = AudioRenderer::Create(rendererOptions);
    AIUT_ASSERT_NE_INFO_LOG(audioRenderer, nullptr, "audioRenderer is nullptr");

    int32_t ret = audioRenderer->SetVolumeMode(TEST_VOLUME_MODE);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "SetVolumeMode failed, ret=%d", ret);
    audioRenderer->Release();
}

} // namespace AudioStandard
} // namespace OHOS
