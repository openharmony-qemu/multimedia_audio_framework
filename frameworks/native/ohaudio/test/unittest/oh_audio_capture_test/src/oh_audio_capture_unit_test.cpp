/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "oh_audio_capture_unit_test.h"

#include <cstdlib>
#include <vector>

#include "audio_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class MuteHintReturnCapturer : public AudioCapturerPrivate {
public:
    explicit MuteHintReturnCapturer(int32_t muteHintRet)
        : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false), muteHintRet_(muteHintRet) {}

    int32_t SetMuteHint(bool mute) override
    {
        (void)mute;
        return muteHintRet_;
    }

private:
    int32_t muteHintRet_ = SUCCESS;
};

class NoiseReductionReturnCapturer : public AudioCapturerPrivate {
public:
    NoiseReductionReturnCapturer(int32_t setRet, int32_t getRet, int32_t getSupportedRet)
        : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false), setRet_(setRet), getRet_(getRet),
          getSupportedRet_(getSupportedRet) {}

    int32_t SetNoiseReductionMode(NoiseReductionMode mode) override
    {
        mode_ = mode;
        return setRet_;
    }

    int32_t GetNoiseReductionMode(NoiseReductionMode &mode) override
    {
        mode = mode_;
        return getRet_;
    }

    int32_t GetSupportedNoiseReductionModes(std::vector<NoiseReductionMode> &modes) override
    {
        modes = supportedModes_;
        return getSupportedRet_;
    }

private:
    int32_t setRet_ = SUCCESS;
    int32_t getRet_ = SUCCESS;
    int32_t getSupportedRet_ = SUCCESS;
    NoiseReductionMode mode_ = NOISE_REDUCTION_MODE_STANDARD;
    std::vector<NoiseReductionMode> supportedModes_ = {
        NOISE_REDUCTION_MODE_FIDELITY,
        NOISE_REDUCTION_MODE_STANDARD,
    };
};

class InvalidNoiseReductionModeCapturer : public AudioCapturerPrivate {
public:
    InvalidNoiseReductionModeCapturer() : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false) {}

    int32_t GetNoiseReductionMode(NoiseReductionMode &mode) override
    {
        mode = NOISE_REDUCTION_MODE_MAX;
        return SUCCESS;
    }

    int32_t GetSupportedNoiseReductionModes(std::vector<NoiseReductionMode> &modes) override
    {
        modes = {NOISE_REDUCTION_MODE_FIDELITY, NOISE_REDUCTION_MODE_MAX};
        return SUCCESS;
    }
};

FILE* g_file = nullptr;

class ReleaseFailureCapturer : public AudioCapturerPrivate {
public:
    ReleaseFailureCapturer() : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false) {}

    int32_t ReleaseImpl() override { return ERR_ILLEGAL_STATE; }
};

class StartPlaybackCaptureFailureCapturer : public AudioCapturerPrivate {
public:
    StartPlaybackCaptureFailureCapturer() : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false) {}

    int32_t StartPlaybackCapture() override { return ERR_ILLEGAL_STATE; }
};

class SetInputDeviceNotSupportedCapturer : public AudioCapturerPrivate {
public:
    SetInputDeviceNotSupportedCapturer() : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false) {}

    int32_t SetInputDevice(DeviceType deviceType) const override { return ERR_NOT_SUPPORTED; }
};

class SetInputDeviceErrorCapturer : public AudioCapturerPrivate {
public:
    SetInputDeviceErrorCapturer() : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false) {}

    int32_t SetInputDevice(DeviceType deviceType) const override { return ERR_INVALID_PARAM; }
};

class MuteHintIllegalStateCapturer : public AudioCapturerPrivate {
public:
    MuteHintIllegalStateCapturer() : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false) {}

    int32_t SetMuteHint(bool mute) override { return ERROR_ILLEGAL_STATE; }
};

class MuteHintInvalidParamCapturer : public AudioCapturerPrivate {
public:
    MuteHintInvalidParamCapturer() : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false) {}

    int32_t SetMuteHint(bool mute) override { return ERROR_INVALID_PARAM; }
};

class GetFastStatusInvalidCapturer : public AudioCapturerPrivate {
public:
    GetFastStatusInvalidCapturer() : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false) {}

    FastStatus GetFastStatus() override { return FastStatus::FASTSTATUS_INVALID; }
};

class GetAudioTimeFailureCapturer : public AudioCapturerPrivate {
public:
    GetAudioTimeFailureCapturer() : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false) {}

    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const override { return false; }
};

class StartSuccessCapturer : public AudioCapturerPrivate {
public:
    StartSuccessCapturer() : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false) {}

    int32_t StartImpl() override { return SUCCESS; }
};

class StopSuccessCapturer : public AudioCapturerPrivate {
public:
    StopSuccessCapturer() : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false) {}

    int32_t StopImpl() const override { return SUCCESS; }
};

class PauseSuccessCapturer : public AudioCapturerPrivate {
public:
    PauseSuccessCapturer() : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false) {}

    bool Pause() const override { return true; }
};

class FlushSuccessCapturer : public AudioCapturerPrivate {
public:
    FlushSuccessCapturer() : AudioCapturerPrivate(STREAM_MUSIC, AppInfo {}, false) {}

    bool Flush() const override { return true; }
};

static OH_AudioStream_Result SetNoiseReductionModeWithRegisteredCapturer(
    OHAudioCapturer &ohAudioCapturer, OH_AudioNoiseReductionMode mode)
{
    ObjectRefMap<OHAudioCapturer>::Insert(&ohAudioCapturer);
    OH_AudioCapturer *audioCapturer = reinterpret_cast<OH_AudioCapturer *>(&ohAudioCapturer);
    OH_AudioStream_Result result = OH_AudioCapturer_SetNoiseReductionMode(audioCapturer, mode);
    ObjectRefMap<OHAudioCapturer>::Erase(&ohAudioCapturer);
    return result;
}

void OHAudioCaptureUnitTest::TearDownTestCase(void) { }

void OHAudioCaptureUnitTest::SetUp(void) { }

void OHAudioCaptureUnitTest::TearDown(void) { }

#ifdef SUPPOR_PERMISSION
static int32_t AudioCapturerOnReadData(OH_AudioCapturer* capturer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    printf("Get callback buffer, bufferLen:%d  \n", bufferLen);
    size_t count = 1;
    if (fwrite(buffer, bufferLen, count, g_file) != count) {
        printf("buffer fwrite err");
    }

    return 0;
}

static int32_t AudioErrCallback(OH_AudioCapturer* renderer,
    void* userData,
    OH_AudioStream_Result error)
{
    printf("recv err : code %d \n", error);
    return 0;
}

static int32_t AudioInterruptCallback(OH_AudioCapturer* renderer,
    void* userData,
    OH_AudioInterrupt_ForceType type,
    OH_AudioInterrupt_Hint hint)
{
    printf("recv interrupt event : type: %d hint: %d \n", type, hint);
    return 0;
}

static int32_t AudioEventCallback(OH_AudioCapturer* renderer,
    void* userData,
    OH_AudioStream_Event event)
{
    printf("recv event : event: %d \n", event);
    return 0;
}
#endif

void InitializeCapturerOptions(AudioCapturerOptions &capturerOptions)
{
    int32_t flag = 0;
    capturerOptions.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    capturerOptions.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    capturerOptions.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    capturerOptions.streamInfo.channels = AudioChannel::MONO;
    capturerOptions.capturerInfo.sourceType = SourceType::SOURCE_TYPE_MIC;
    capturerOptions.capturerInfo.capturerFlags = flag;
    return;
}

OH_AudioStreamBuilder* OHAudioCaptureUnitTest::CreateCapturerBuilder()
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStreamBuilder_Create(&builder, type);
    return builder;
}

/**
* @tc.name  : Test OH_AudioStreamBuilder_GenerateCapturer API via legal state.
* @tc.number: OH_Audio_Capture_Generate_001
* @tc.desc  : Test OH_AudioStreamBuilder_GenerateCapturer interface. Returns true, if the result is successful.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Generate_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    EXPECT_TRUE(result != AUDIOSTREAM_ERROR_INVALID_PARAM);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioStreamBuilder_GenerateCapturer API via illegal OH_AudioStream_Type.
* @tc.number: OH_Audio_Capture_Generate_002
* @tc.desc  : Test OH_AudioStreamBuilder_GenerateCapturer interface. Returns error code, if the stream type is
*             AUDIOSTREAM_TYPE_RENDERER.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Generate_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer* audioCapturer;
    result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioStreamBuilder_GenerateCapturer API via illegal OH_AudioStream_Type.
* @tc.number: OH_Audio_Capture_Generate_003
* @tc.desc  : Test OH_AudioStreamBuilder_GenerateCapturer interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
*             if builder is nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Generate_003, TestSize.Level0)
{
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(nullptr, &audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioStreamBuilder_GenerateCapturer API via illegal OH_AudioStream_Type.
* @tc.number: OH_Audio_Capture_Generate_004
* @tc.desc  : Test OH_AudioStreamBuilder_GenerateCapturer interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
*             if audioCapturer is nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Generate_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Start API via legal state.
* @tc.number: Audio_Capturer_Start_001
* @tc.desc  : Test OH_AudioCapturer_Start interface. Returns true if start is successful.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Start_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    result = OH_AudioCapturer_Start(audioCapturer);
    EXPECT_TRUE(result != AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Start API via illegal state.
* @tc.number: Audio_Capturer_Start_002
* @tc.desc  : Test OH_AudioCapturer_Start interface. Returns error code, if Start interface is called twice.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Start_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    result = OH_AudioCapturer_Start(audioCapturer);

    result = OH_AudioCapturer_Start(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Pause API via legal state.
* @tc.number: OH_Audio_Capture_Pause_001
* @tc.desc  : Test OH_AudioCapturer_Pause interface. Returns true if Pause is successful.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Pause_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    result = OH_AudioCapturer_Start(audioCapturer);

    result = OH_AudioCapturer_Pause(audioCapturer);
    EXPECT_TRUE(result != AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Pause API via illegal state, Pause without Start first.
* @tc.number: OH_Audio_Capture_Pause_002
* @tc.desc  : Test OH_AudioCapturer_Pause interface. Returns error code, if Pause without Start first.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Pause_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    result = OH_AudioCapturer_Pause(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Stop API via legal state.
* @tc.number: OH_Audio_Capture_Stop_001
* @tc.desc  : Test OH_AudioCapturer_Stop interface. Returns true if Stop is successful.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Stop_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    result = OH_AudioCapturer_Start(audioCapturer);

    result = OH_AudioCapturer_Stop(audioCapturer);
    EXPECT_TRUE(result != AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Stop API via illegal state, Stop without Start first.
* @tc.number: OH_Audio_Capture_Stop_002
* @tc.desc  : Test OH_AudioCapturer_Stop interface. Returns error code, if Stop without Start first.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Stop_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    result = OH_AudioCapturer_Stop(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Flush API via legal state.
* @tc.number: OH_Audio_Capture_Flush_001
* @tc.desc  : Test OH_AudioCapturer_Flush interface. Returns true if Flush is successful.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Flush_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    result = OH_AudioCapturer_Start(audioCapturer);

    result = OH_AudioCapturer_Flush(audioCapturer);
    EXPECT_TRUE(result != AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Flush API via illegal state.
* @tc.number: OH_Audio_Capture_Flush_002
* @tc.desc  : Test OH_AudioCapturer_Flush interface. Returns error code, if Flush without Start first.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Flush_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    result = OH_AudioCapturer_Flush(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Release API via legal state.
* @tc.number: OH_Audio_Capture_Release_001
* @tc.desc  : Test OH_AudioCapturer_Release interface. Returns true if Release is successful.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_Release_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();

    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    result = OH_AudioCapturer_Start(audioCapturer);

    result = OH_AudioCapturer_Release(audioCapturer);
    EXPECT_TRUE(result != AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetLatencyMode API via legal state.
* @tc.number: OH_Audio_Capture_GetParameter_001
* @tc.desc  : Test OH_AudioCapturer_GetLatencyMode interface. Returns true if latencyMode is
*             AUDIOSTREAM_LATENCY_MODE_NORMAL,because default latency mode is AUDIOSTREAM_LATENCY_MODE_NORMAL.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_GetParameter_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioStream_LatencyMode latencyMode = AUDIOSTREAM_LATENCY_MODE_NORMAL;
    result = OH_AudioCapturer_GetLatencyMode(audioCapturer, &latencyMode);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(latencyMode == AUDIOSTREAM_LATENCY_MODE_NORMAL);
    OH_AudioStreamBuilder_Destroy(builder);
}

/** @tc.desc: Native capture flags map explicitly to public latency modes, including VOIP_FAST. */
HWTEST(OHAudioCaptureUnitTest, GetLatencyModeExplicitFlags_001, TestSize.Level1)
{
    struct LatencyCase {
        int32_t originalFlag;
        int32_t currentFlag;
        OH_AudioStream_LatencyMode expected;
    };
    const std::vector<LatencyCase> cases = {
        {AUDIO_FLAG_NORMAL, AUDIO_FLAG_NORMAL, AUDIOSTREAM_LATENCY_MODE_NORMAL},
        {AUDIO_FLAG_NORMAL, AUDIO_FLAG_FORCED_NORMAL, AUDIOSTREAM_LATENCY_MODE_NORMAL},
        {AUDIO_FLAG_MMAP, AUDIO_FLAG_MMAP, AUDIOSTREAM_LATENCY_MODE_FAST},
        {AUDIO_FLAG_VOIP_FAST, AUDIO_FLAG_VOIP_FAST, AUDIOSTREAM_LATENCY_MODE_FAST},
        {AUDIO_FLAG_NORMAL, AUDIO_FLAG_ULTRA_FAST, AUDIOSTREAM_LATENCY_MODE_ULTRA_FAST},
        {AUDIO_FLAG_ULTRA_FAST, AUDIO_FLAG_ULTRA_FAST, AUDIOSTREAM_LATENCY_MODE_ULTRA_FAST},
        {AUDIO_FLAG_ULTRA_FAST, AUDIO_FLAG_MMAP, AUDIOSTREAM_LATENCY_MODE_ULTRA_FAST},
        {AUDIO_FLAG_ULTRA_FAST, AUDIO_FLAG_NORMAL, AUDIOSTREAM_LATENCY_MODE_ULTRA_FAST},
        {AUDIO_FLAG_NORMAL, 99, AUDIOSTREAM_LATENCY_MODE_NORMAL}
    };
    OHAudioCapturer wrapper;
    auto nativeCapturer = std::make_shared<AudioCapturerPrivate>(STREAM_MUSIC, AppInfo {}, false);
    wrapper.audioCapturer_ = nativeCapturer;
    auto *capturer = reinterpret_cast<OH_AudioCapturer *>(&wrapper);
    for (const auto &testCase : cases) {
        nativeCapturer->capturerInfo_.originalFlag = testCase.originalFlag;
        nativeCapturer->capturerInfo_.capturerFlags = testCase.currentFlag;
        OH_AudioStream_LatencyMode mode = AUDIOSTREAM_LATENCY_MODE_NORMAL;
        EXPECT_EQ(OH_AudioCapturer_GetLatencyMode(capturer, &mode), AUDIOSTREAM_SUCCESS);
        EXPECT_EQ(mode, testCase.expected) << testCase.originalFlag << ", " << testCase.currentFlag;
    }
    EXPECT_EQ(OH_AudioCapturer_GetLatencyMode(capturer, nullptr), AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStream_LatencyMode mode = AUDIOSTREAM_LATENCY_MODE_NORMAL;
    EXPECT_EQ(OH_AudioCapturer_GetLatencyMode(nullptr, &mode), AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetCurrentState API via legal state.
* @tc.number: OH_AudioCapturer_GetCurrentState_001
* @tc.desc  : Test OH_AudioCapturer_GetCurrentState interface. Return true if the result state is
*             AUDIOSTREAM_STATE_PREPARED.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetCurrentState_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioStream_State state;
    result = OH_AudioCapturer_GetCurrentState(audioCapturer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetCurrentState API via legal state.
* @tc.number: OH_AudioCapturer_GetCurrentState_002
* @tc.desc  : Test OH_AudioCapturer_GetCurrentState interface. Return true if the result state is
*             AUDIOSTREAM_STATE_RUNNING.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetCurrentState_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioCapturer_Start(audioCapturer);

    OH_AudioStream_State state;
    result = OH_AudioCapturer_GetCurrentState(audioCapturer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetCurrentState API via legal state.
* @tc.number: OH_AudioCapturer_GetCurrentState_003
* @tc.desc  : Test OH_AudioCapturer_GetCurrentState interface. Return true if the result state is
*             AUDIOSTREAM_STATE_PAUSED.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetCurrentState_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioCapturer_Start(audioCapturer);
    OH_AudioCapturer_Pause(audioCapturer);

    OH_AudioStream_State state;
    result = OH_AudioCapturer_GetCurrentState(audioCapturer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetCurrentState API via legal state.
* @tc.number: OH_AudioCapturer_GetCurrentState_004
* @tc.desc  : Test OH_AudioCapturer_GetCurrentState interface. Return true if the result state is
*             AUDIOSTREAM_STATE_STOPPED.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetCurrentState_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioCapturer_Start(audioCapturer);
    OH_AudioCapturer_Stop(audioCapturer);

    OH_AudioStream_State state;
    result = OH_AudioCapturer_GetCurrentState(audioCapturer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetStreamId API via legal state.
* @tc.number: OH_Audio_Capture_GetParameter_002
* @tc.desc  : Test OH_AudioCapturer_GetStreamId interface. Returns true if the result is AUDIOSTREAM_SUCCESS.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_GetParameter_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    uint32_t streamId;
    result = OH_AudioCapturer_GetStreamId(audioCapturer, &streamId);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetSamplingRate API via legal state.
* @tc.number: OH_Audio_Capture_GetSamplingRate_001
* @tc.desc  : Test OH_AudioCapturer_GetSamplingRate interface. Returns true if samplingRate is
*             SAMPLE_RATE_48000,because default samplingRate is SAMPLE_RATE_48000.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_GetSamplingRate_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    int32_t rate;
    result = OH_AudioCapturer_GetSamplingRate(audioCapturer, &rate);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetSampleFormat API via legal state.
* @tc.number: OH_Audio_Capture_GetSampleFormat_001
* @tc.desc  : Test OH_AudioCapturer_GetSampleFormat interface. Returns true if sampleFormat is
*             AUDIOSTREAM_SAMPLE_S16LE,because default sampleFormat is AUDIOSTREAM_SAMPLE_S16LE.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapture_GetSampleFormat_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioStream_SampleFormat sampleFormat;
    result = OH_AudioCapturer_GetSampleFormat(audioCapturer, &sampleFormat);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetEncodingType API via legal state.
* @tc.number: OH_Audio_Capture_GetEncodingType_001
* @tc.desc  : Test OH_AudioCapturer_GetEncodingType interface. Returns true if encodingType is
*             ENCODING_PCM,because default encodingType is ENCODING_PCM.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_GetEncodingType_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioStream_EncodingType encodingType;
    result = OH_AudioCapturer_GetEncodingType(audioCapturer, &encodingType);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetCapturerInfo API via legal state.
* @tc.number: OH_Audio_Capture_GetCapturerInfo_001
* @tc.desc  : Test OH_AudioCapturer_GetCapturerInfo interface. Returns true if sourceType is
*             SOURCE_TYPE_MIC,because default sourceType is SOURCE_TYPE_MIC.
*/
HWTEST(OHAudioCaptureUnitTest, OH_Audio_Capture_GetCapturerInfo_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);

    OH_AudioStream_SourceType sourceType;
    result = OH_AudioCapturer_GetCapturerInfo(audioCapturer, &sourceType);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetTimestamp API via illegal state.
* @tc.number: OH_AudioCapturer_GetTimestamp_001
* @tc.desc  : Test OH_AudioCapturer_GetTimestamp interface. Return false if clockId is not CLOCK_MONOTONIC
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetTimestamp_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    int64_t framePosition;
    int64_t timestamp;
    result = OH_AudioCapturer_GetTimestamp(audioCapturer, CLOCK_REALTIME, &framePosition, &timestamp);
    EXPECT_TRUE(result != AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetTimestamp API via illegal state.
* @tc.number: OH_AudioCapturer_GetTimestamp_002
* @tc.desc  : Test OH_AudioCapturer_GetTimestamp interface.Returns false if capturer is nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetTimestamp_002, TestSize.Level0)
{
    OH_AudioCapturer* audioCapturer = nullptr;
    int64_t framePosition;
    int64_t timestamp;
    OH_AudioStream_Result result = OH_AudioCapturer_GetTimestamp(audioCapturer, CLOCK_MONOTONIC,
        &framePosition, &timestamp);
    EXPECT_TRUE(result != AUDIOSTREAM_SUCCESS);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_001
* @tc.desc  : Test OHAudioCapturerErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_001, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    OH_AudioCapturer* audioCapturer = nullptr;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(callbacks, audioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_ILLEGAL_STATE;
    auto ret = oHAudioCapturerErrorCallback->GetErrorResult(errorCode);
    EXPECT_EQ(ret, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_002
* @tc.desc  : Test OHAudioCapturerErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_002, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    OH_AudioCapturer* audioCapturer = nullptr;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(callbacks, audioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_INVALID_PARAM;
    auto ret = oHAudioCapturerErrorCallback->GetErrorResult(errorCode);
    EXPECT_EQ(ret, AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_003
* @tc.desc  : Test OHAudioCapturerErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_003, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    OH_AudioCapturer* audioCapturer = nullptr;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(callbacks, audioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_SYSTEM;
    auto ret = oHAudioCapturerErrorCallback->GetErrorResult(errorCode);
    EXPECT_EQ(ret, AUDIOSTREAM_ERROR_SYSTEM);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_004
* @tc.desc  : Test OHAudioCapturerErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_004, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    OH_AudioCapturer* audioCapturer = nullptr;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(callbacks, audioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_TIMEOUT;
    auto ret = oHAudioCapturerErrorCallback->GetErrorResult(errorCode);
    EXPECT_EQ(ret, AUDIOSTREAM_ERROR_SYSTEM);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_005
* @tc.desc  : Test OHAudioCapturerErrorCallback::OnError()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_005, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnError = [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) ->
        int32_t { return 0; };
    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.errorCallbackType_ = ERROR_CALLBACK_COMBINED;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    EXPECT_NE(oHAudioCapturerErrorCallback->callbacks_.OH_AudioCapturer_OnError, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_TIMEOUT;
    oHAudioCapturerErrorCallback->OnError(errorCode);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_006
* @tc.desc  : Test OHAudioCapturerErrorCallback::OnError()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_006, TestSize.Level0)
{
    OH_AudioCapturer_OnErrorCallback errorCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) { return; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(errorCallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    EXPECT_NE(oHAudioCapturerErrorCallback->errorCallback_, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_TIMEOUT;
    oHAudioCapturerErrorCallback->OnError(errorCode);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_007
* @tc.desc  : Test OHAudioCapturerErrorCallback::OnError()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_007, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnError = nullptr;
    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.errorCallbackType_ = ERROR_CALLBACK_COMBINED;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_TIMEOUT;
    oHAudioCapturerErrorCallback->OnError(errorCode);
}

/**
* @tc.name  : Test OHAudioCapturerErrorCallback API
* @tc.number: OHAudioCapturerErrorCallback_008
* @tc.desc  : Test OHAudioCapturerErrorCallback::OnError()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerErrorCallback_008, TestSize.Level0)
{
    OH_AudioCapturer_OnErrorCallback errorCallback = nullptr;
    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto oHAudioCapturerErrorCallback =
        std::make_shared<OHAudioCapturerErrorCallback>(errorCallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerErrorCallback, nullptr);

    AudioErrors errorCode = AudioErrors::ERROR_TIMEOUT;
    oHAudioCapturerErrorCallback->OnError(errorCode);
}

/**
* @tc.name  : Test OHCapturerServiceDiedCallback API
* @tc.number: OHCapturerServiceDiedCallback_001
* @tc.desc  : Test OHCapturerServiceDiedCallback::OnAudioPolicyServiceDied()
*/
HWTEST(OHAudioCaptureUnitTest, OHCapturerServiceDiedCallback_001, TestSize.Level0)
{
    OH_AudioCapturer_OnErrorCallback errorCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) { return; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto oHCapturerServiceDiedCallback =
        std::make_shared<OHCapturerServiceDiedCallback>(errorCallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHCapturerServiceDiedCallback, nullptr);

    EXPECT_NE(oHCapturerServiceDiedCallback->errorCallback_, nullptr);

    oHCapturerServiceDiedCallback->OnAudioPolicyServiceDied();
}

/**
* @tc.name  : Test OHCapturerServiceDiedCallback API
* @tc.number: OHCapturerServiceDiedCallback_002
* @tc.desc  : Test OHCapturerServiceDiedCallback::OnAudioPolicyServiceDied()
*/
HWTEST(OHAudioCaptureUnitTest, OHCapturerServiceDiedCallback_002, TestSize.Level0)
{
    OH_AudioCapturer_OnErrorCallback errorCallback = nullptr;

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto oHCapturerServiceDiedCallback =
        std::make_shared<OHCapturerServiceDiedCallback>(errorCallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHCapturerServiceDiedCallback, nullptr);

    oHCapturerServiceDiedCallback->OnAudioPolicyServiceDied();
}

/**
* @tc.name  : Test OHCapturerServiceDiedCallback API
* @tc.number: OHCapturerServiceDiedCallback_003
* @tc.desc  : Test OHCapturerServiceDiedCallback::OnAudioPolicyServiceDied()
*/
HWTEST(OHAudioCaptureUnitTest, OHCapturerServiceDiedCallback_003, TestSize.Level0)
{
    OH_AudioCapturer_OnErrorCallback errorCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) { return; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.errorCallbackType_ = ERROR_CALLBACK_COMBINED;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto oHCapturerServiceDiedCallback =
        std::make_shared<OHCapturerServiceDiedCallback>(errorCallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHCapturerServiceDiedCallback, nullptr);

    oHCapturerServiceDiedCallback->OnAudioPolicyServiceDied();
}

/**
* @tc.name  : Test OHAudioCapturerModeCallback API
* @tc.number: OHAudioCapturerModeCallback_001
* @tc.desc  : Test OHAudioCapturerModeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerModeCallback_001, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnReadData =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) ->
        int32_t { return 0; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.readDataCallbackType_ = READ_DATA_CALLBACK_WITHOUT_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerModeCallback =
        std::make_shared<OHAudioCapturerModeCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerModeCallback, nullptr);

    size_t length = 0;

    oHAudioCapturerModeCallback->OnReadData(length);
}

/**
* @tc.name  : Test OHAudioCapturerModeCallback API
* @tc.number: OHAudioCapturerModeCallback_002
* @tc.desc  : Test OHAudioCapturerModeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerModeCallback_002, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnReadData =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) ->
        int32_t { return 0; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.readDataCallbackType_ = READ_DATA_CALLBACK_WITH_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerModeCallback =
        std::make_shared<OHAudioCapturerModeCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerModeCallback, nullptr);

    size_t length = 0;

    oHAudioCapturerModeCallback->OnReadData(length);
}

/**
* @tc.name  : Test OHAudioCapturerModeCallback API
* @tc.number: OHAudioCapturerModeCallback_003
* @tc.desc  : Test OHAudioCapturerModeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerModeCallback_003, TestSize.Level0)
{
    OH_AudioCapturer_OnReadDataCallback readcallback =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) { return; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.readDataCallbackType_ = READ_DATA_CALLBACK_WITH_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerModeCallback =
        std::make_shared<OHAudioCapturerModeCallback>(readcallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerModeCallback, nullptr);

    size_t length = 0;

    oHAudioCapturerModeCallback->OnReadData(length);
}

/**
* @tc.name  : Test OHAudioCapturerModeCallback API
* @tc.number: OHAudioCapturerModeCallback_004
* @tc.desc  : Test OHAudioCapturerModeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerModeCallback_004, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    OH_AudioCapturer_OnReadDataCallback readcallback =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) { return; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.readDataCallbackType_ = READ_DATA_CALLBACK_WITHOUT_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerModeCallback =
        std::make_shared<OHAudioCapturerModeCallback>(readcallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerModeCallback, nullptr);

    oHAudioCapturerModeCallback->callbacks_ = callbacks;
    EXPECT_EQ(oHAudioCapturerModeCallback->callbacks_.OH_AudioCapturer_OnReadData, nullptr);

    size_t length = 0;

    oHAudioCapturerModeCallback->OnReadData(length);
}

/**
* @tc.name  : Test OHAudioCapturerDeviceChangeCallback API
* @tc.number: OHAudioCapturerDeviceChangeCallback_001
* @tc.desc  : Test OHAudioCapturerDeviceChangeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerDeviceChangeCallback_001, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnStreamEvent =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Event event) ->
        int32_t { return 0; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.streamEventCallbackType_ = STREAM_EVENT_CALLBACK_COMBINED;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerDeviceChangeCallback =
        std::make_shared<OHAudioCapturerDeviceChangeCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerDeviceChangeCallback, nullptr);

    AudioDeviceDescriptor deviceInfo;

    oHAudioCapturerDeviceChangeCallback->OnStateChange(deviceInfo);
}

/**
* @tc.name  : Test OHAudioCapturerDeviceChangeCallback API
* @tc.number: OHAudioCapturerDeviceChangeCallback_002
* @tc.desc  : Test OHAudioCapturerDeviceChangeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerDeviceChangeCallback_002, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnStreamEvent =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Event event) ->
        int32_t { return 0; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.streamEventCallbackType_ = STREAM_EVENT_CALLBACK_SEPERATED;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerDeviceChangeCallback =
        std::make_shared<OHAudioCapturerDeviceChangeCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerDeviceChangeCallback, nullptr);

    AudioDeviceDescriptor deviceInfo;

    oHAudioCapturerDeviceChangeCallback->OnStateChange(deviceInfo);
}

/**
* @tc.name  : Test OHAudioCapturerDeviceChangeCallback API
* @tc.number: OHAudioCapturerDeviceChangeCallback_003
* @tc.desc  : Test OHAudioCapturerDeviceChangeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerDeviceChangeCallback_003, TestSize.Level0)
{
    OH_AudioCapturer_OnDeviceChangeCallback deviceChangeCallBack =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioDeviceDescriptorArray* array) { return; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.streamEventCallbackType_ = STREAM_EVENT_CALLBACK_SEPERATED;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerDeviceChangeCallback =
        std::make_shared<OHAudioCapturerDeviceChangeCallback>(deviceChangeCallBack, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerDeviceChangeCallback, nullptr);

    AudioDeviceDescriptor deviceInfo;

    oHAudioCapturerDeviceChangeCallback->OnStateChange(deviceInfo);
}

/**
* @tc.name  : Test OHAudioCapturerDeviceChangeCallback API
* @tc.number: OHAudioCapturerDeviceChangeCallback_004
* @tc.desc  : Test OHAudioCapturerDeviceChangeCallback::OnReadData()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerDeviceChangeCallback_004, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    OH_AudioCapturer_OnDeviceChangeCallback deviceChangeCallBack =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioDeviceDescriptorArray* array) { return; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.streamEventCallbackType_ = STREAM_EVENT_CALLBACK_COMBINED;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerDeviceChangeCallback =
        std::make_shared<OHAudioCapturerDeviceChangeCallback>(deviceChangeCallBack, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerDeviceChangeCallback, nullptr);

    oHAudioCapturerDeviceChangeCallback->callbacks_ = callbacks;
    EXPECT_EQ(oHAudioCapturerDeviceChangeCallback->callbacks_.OH_AudioCapturer_OnStreamEvent, nullptr);

    AudioDeviceDescriptor deviceInfo;

    oHAudioCapturerDeviceChangeCallback->OnStateChange(deviceInfo);
}

/**
* @tc.name  : Test OHAudioCapturerCallback API
* @tc.number: OHAudioCapturerCallback_001
* @tc.desc  : Test OHAudioCapturerCallback::OnInterrupt()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerCallback_001, TestSize.Level0)
{
    OH_AudioCapturer_OnInterruptCallback interruptCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioInterrupt_ForceType forceType,
        OH_AudioInterrupt_Hint hintType) { return; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_SEPERATED;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerCallback =
        std::make_shared<OHAudioCapturerCallback>(interruptCallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerCallback, nullptr);

    InterruptEvent interruptEvent;

    oHAudioCapturerCallback->OnInterrupt(interruptEvent);
}

/**
* @tc.name  : Test OHAudioCapturerCallback API
* @tc.number: OHAudioCapturerCallback_002
* @tc.desc  : Test OHAudioCapturerCallback::OnInterrupt()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerCallback_002, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    OH_AudioCapturer_OnInterruptCallback interruptCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioInterrupt_ForceType forceType,
        OH_AudioInterrupt_Hint hintType) { return; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_COMBINED;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerCallback =
        std::make_shared<OHAudioCapturerCallback>(interruptCallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerCallback, nullptr);

    oHAudioCapturerCallback->callbacks_ = callbacks;
    EXPECT_EQ(oHAudioCapturerCallback->callbacks_.OH_AudioCapturer_OnInterruptEvent, nullptr);


    InterruptEvent interruptEvent;

    oHAudioCapturerCallback->OnInterrupt(interruptEvent);
}

/**
* @tc.name  : Test OHAudioCapturerCallback API
* @tc.number: OHAudioCapturerCallback_003
* @tc.desc  : Test OHAudioCapturerCallback::OnInterrupt()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerCallback_003, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnInterruptEvent =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioInterrupt_ForceType forceType,
        OH_AudioInterrupt_Hint hintType) -> int32_t { return 0; };

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_COMBINED;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerCallback =
        std::make_shared<OHAudioCapturerCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerCallback, nullptr);

    InterruptEvent interruptEvent;

    oHAudioCapturerCallback->OnInterrupt(interruptEvent);
}

#ifdef TEMP_DISABLE
/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_001
* @tc.desc  : Test OHAudioCapturer::SetReadDataCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_001, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->readDataCallbackType_ = READ_DATA_CALLBACK_WITH_RESULT;
    capturerCallbacks.onReadDataCallback =
        [](OH_AudioCapturer* capturer, void* userData, void* audioData, int32_t audioDataSize) { return; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetReadDataCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_002
* @tc.desc  : Test OHAudioCapturer::SetReadDataCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_002, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->readDataCallbackType_ = READ_DATA_CALLBACK_WITHOUT_RESULT;
    capturerCallbacks.onReadDataCallback =
        [](OH_AudioCapturer* capturer, void* userData, void* audioData, int32_t audioDataSize) { return; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetReadDataCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_003
* @tc.desc  : Test OHAudioCapturer::SetReadDataCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_003, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->readDataCallbackType_ = READ_DATA_CALLBACK_WITHOUT_RESULT;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnReadData =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) ->
        int32_t { return 0; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetReadDataCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_004
* @tc.desc  : Test OHAudioCapturer::SetReadDataCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_004, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->readDataCallbackType_ = READ_DATA_CALLBACK_WITH_RESULT;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnReadData =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) ->
        int32_t { return 0; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetReadDataCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_005
* @tc.desc  : Test OHAudioCapturer::SetStreamEventCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_005, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->streamEventCallbackType_ = STREAM_EVENT_CALLBACK_SEPERATED;
    capturerCallbacks.onDeviceChangeCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioDeviceDescriptorArray* deviceArray) { return; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetStreamEventCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_006
* @tc.desc  : Test OHAudioCapturer::SetStreamEventCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_006, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->streamEventCallbackType_ = STREAM_EVENT_CALLBACK_COMBINED;
    capturerCallbacks.onDeviceChangeCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioDeviceDescriptorArray* deviceArray) { return; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetStreamEventCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_007
* @tc.desc  : Test OHAudioCapturer::SetStreamEventCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_007, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->streamEventCallbackType_ = STREAM_EVENT_CALLBACK_COMBINED;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnStreamEvent =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Event event) ->
        int32_t { return 0; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetStreamEventCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_008
* @tc.desc  : Test OHAudioCapturer::SetStreamEventCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_008, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->streamEventCallbackType_ = STREAM_EVENT_CALLBACK_COMBINED;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnStreamEvent =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Event event) ->
        int32_t { return 0; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetStreamEventCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_009
* @tc.desc  : Test OHAudioCapturer::SetInterruptCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_009, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_SEPERATED;
    capturerCallbacks.onInterruptEventCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioInterrupt_ForceType forceType,
        OH_AudioInterrupt_Hint hintType) { return; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_010
* @tc.desc  : Test OHAudioCapturer::SetInterruptCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_010, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_COMBINED;
    capturerCallbacks.onInterruptEventCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioInterrupt_ForceType forceType,
        OH_AudioInterrupt_Hint hintType) { return; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_011
* @tc.desc  : Test OHAudioCapturer::SetInterruptCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_011, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_COMBINED;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnInterruptEvent =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioInterrupt_ForceType forceType,
        OH_AudioInterrupt_Hint hintType) -> int32_t { return 0; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_012
* @tc.desc  : Test OHAudioCapturer::SetInterruptCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_012, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_SEPERATED;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnInterruptEvent =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioInterrupt_ForceType forceType,
        OH_AudioInterrupt_Hint hintType) -> int32_t { return 0; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_013
* @tc.desc  : Test OHAudioCapturer::SetErrorCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_013, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    capturerCallbacks.onErrorCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) { return; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetErrorCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_014
* @tc.desc  : Test OHAudioCapturer::SetErrorCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_014, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->errorCallbackType_ = ERROR_CALLBACK_COMBINED;
    capturerCallbacks.onErrorCallback =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) { return; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetErrorCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_015
* @tc.desc  : Test OHAudioCapturer::SetErrorCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_015, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->errorCallbackType_ = ERROR_CALLBACK_COMBINED;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnError =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) ->
        int32_t { return 0; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetErrorCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioCapturer API
* @tc.number: OHAudioCapturer_016
* @tc.desc  : Test OHAudioCapturer::SetErrorCallback()
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_016, TestSize.Level0)
{
    auto oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);

    CapturerCallback capturerCallbacks;
    void* userData = nullptr;

    oHAudioCapturer->errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnError =
        [](OH_AudioCapturer* capturer, void* userData, OH_AudioStream_Result error) ->
        int32_t { return 0; };

    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    ASSERT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->SetErrorCallback(capturerCallbacks, userData);
}
#endif

/**
* @tc.name  : Test OH_AudioCapturer_SetInputDevice
* @tc.number: OH_AudioCapturer_SetInputDevice_001
* @tc.desc  : Test OH_AudioCapturer_SetInputDevice
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_SetInputDevice_001, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioCapturer_SetInputDevice(audioCapturer, AUDIO_DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_SYSTEM);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetFastStatus
* @tc.number: OH_AudioCapturer_GetFastStatus_001
* @tc.desc  : Test OH_AudioCapturer_GetFastStatus
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetFastStatus_001, TestSize.Level0)
{
    OH_AudioStream_FastStatus status = AUDIOSTREAM_FASTSTATUS_FAST;
    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    OH_AudioStream_Result result = OH_AudioCapturer_GetFastStatus(audioCapturer, &status);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetTimestamp API via illegal state.
* @tc.number: OH_AudioCapturer_GetTimestamp_003
* @tc.desc  : Test OH_AudioCapturer_GetTimestamp interface.Returns false if capturer is nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetTimestamp_003, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* audioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    int64_t framePosition;
    int64_t timestamp;
    OH_AudioStream_Result result = OH_AudioCapturer_GetTimestamp(audioCapturer, CLOCK_MONOTONIC,
        &framePosition, &timestamp);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test OH_AudioCapturer_SetMuteHint API via invalid parameter.
* @tc.number: OH_AudioCapturer_SetMuteHint_001
* @tc.desc  : Test OH_AudioCapturer_SetMuteHint interface.Returns invalid param if capturer is nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_SetMuteHint_001, TestSize.Level0)
{
    OH_AudioStream_Result result = OH_AudioCapturer_SetMuteHint(nullptr, true);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioCapturer_SetMuteHint API via ERR_ILLEGAL_STATE.
* @tc.number: OH_AudioCapturer_SetMuteHint_003
* @tc.desc  : Test OH_AudioCapturer_SetMuteHint interface.Returns illegal state for ERR_ILLEGAL_STATE.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_SetMuteHint_003, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<MuteHintReturnCapturer>(ERR_ILLEGAL_STATE);
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    OH_AudioCapturer *audioCapturer = reinterpret_cast<OH_AudioCapturer *>(&oHAudioCapturer);
    OH_AudioStream_Result result = OH_AudioCapturer_SetMuteHint(audioCapturer, true);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test OH_AudioCapturer_SetMuteHint API via ERR_INVALID_PARAM.
* @tc.number: OH_AudioCapturer_SetMuteHint_004
* @tc.desc  : Test OH_AudioCapturer_SetMuteHint interface.Returns invalid param for ERR_INVALID_PARAM.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_SetMuteHint_004, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<MuteHintReturnCapturer>(ERR_INVALID_PARAM);
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    OH_AudioCapturer *audioCapturer = reinterpret_cast<OH_AudioCapturer *>(&oHAudioCapturer);
    OH_AudioStream_Result result = OH_AudioCapturer_SetMuteHint(audioCapturer, false);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioCapturer_SetNoiseReductionMode API via invalid parameter.
* @tc.number: OH_AudioCapturer_SetNoiseReductionMode_001
* @tc.desc  : Test SetNoiseReductionMode returns invalid param if capturer is nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_SetNoiseReductionMode_001, TestSize.Level0)
{
    OH_AudioStream_Result result = OH_AudioCapturer_SetNoiseReductionMode(nullptr,
        AUDIO_NOISE_REDUCTION_MODE_FIDELITY);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioCapturer_SetNoiseReductionMode API via invalid mode.
* @tc.number: OH_AudioCapturer_SetNoiseReductionMode_002
* @tc.desc  : Test SetNoiseReductionMode returns invalid param if mode is out of range.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_SetNoiseReductionMode_002, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl =
        std::make_shared<NoiseReductionReturnCapturer>(SUCCESS, SUCCESS, SUCCESS);
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    OH_AudioStream_Result result = SetNoiseReductionModeWithRegisteredCapturer(oHAudioCapturer,
        static_cast<OH_AudioNoiseReductionMode>(AUDIO_NOISE_REDUCTION_MODE_STANDARD + 1));
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    result = SetNoiseReductionModeWithRegisteredCapturer(oHAudioCapturer,
        static_cast<OH_AudioNoiseReductionMode>(AUDIO_NOISE_REDUCTION_MODE_FIDELITY - 1));
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioCapturer_SetNoiseReductionMode API via native error.
* @tc.number: OH_AudioCapturer_SetNoiseReductionMode_003
* @tc.desc  : Test SetNoiseReductionMode maps native invalid param, illegal state, unsupported and service died.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_SetNoiseReductionMode_003, TestSize.Level0)
{
    OHAudioCapturer invalidParamCapturer;
    invalidParamCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        ERR_INVALID_PARAM, SUCCESS, SUCCESS);
    OH_AudioStream_Result result = SetNoiseReductionModeWithRegisteredCapturer(invalidParamCapturer,
        AUDIO_NOISE_REDUCTION_MODE_STANDARD);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    OHAudioCapturer illegalStateCapturer;
    illegalStateCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        ERR_ILLEGAL_STATE, SUCCESS, SUCCESS);
    result = SetNoiseReductionModeWithRegisteredCapturer(illegalStateCapturer, AUDIO_NOISE_REDUCTION_MODE_STANDARD);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OHAudioCapturer unsupportedCapturer;
    unsupportedCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        ERR_NOT_SUPPORTED, SUCCESS, SUCCESS);
    result = SetNoiseReductionModeWithRegisteredCapturer(unsupportedCapturer, AUDIO_NOISE_REDUCTION_MODE_STANDARD);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_UNSUPPORTED_ABILITY);

    OHAudioCapturer serviceDiedCapturer;
    serviceDiedCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        ERROR, SUCCESS, SUCCESS);
    result = SetNoiseReductionModeWithRegisteredCapturer(serviceDiedCapturer, AUDIO_NOISE_REDUCTION_MODE_STANDARD);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_SERVICE_DIED);

    OHAudioCapturer errorInvalidParamCapturer;
    errorInvalidParamCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        ERROR_INVALID_PARAM, SUCCESS, SUCCESS);
    result = SetNoiseReductionModeWithRegisteredCapturer(errorInvalidParamCapturer,
        AUDIO_NOISE_REDUCTION_MODE_STANDARD);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);

    OHAudioCapturer errorIllegalStateCapturer;
    errorIllegalStateCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        ERROR_ILLEGAL_STATE, SUCCESS, SUCCESS);
    result = SetNoiseReductionModeWithRegisteredCapturer(errorIllegalStateCapturer,
        AUDIO_NOISE_REDUCTION_MODE_STANDARD);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OHAudioCapturer errorUnsupportedCapturer;
    errorUnsupportedCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        ERROR_UNSUPPORTED, SUCCESS, SUCCESS);
    result = SetNoiseReductionModeWithRegisteredCapturer(errorUnsupportedCapturer,
        AUDIO_NOISE_REDUCTION_MODE_STANDARD);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_UNSUPPORTED_ABILITY);
}

/**
* @tc.name  : Test OH_AudioCapturer_SetNoiseReductionMode API via success.
* @tc.number: OH_AudioCapturer_SetNoiseReductionMode_004
* @tc.desc  : Test SetNoiseReductionMode returns success if native call succeeds.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_SetNoiseReductionMode_004, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        SUCCESS, SUCCESS, SUCCESS);

    OH_AudioStream_Result result = SetNoiseReductionModeWithRegisteredCapturer(oHAudioCapturer,
        AUDIO_NOISE_REDUCTION_MODE_STANDARD);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioCapturer_SetNoiseReductionMode API via released capturer.
* @tc.number: OH_AudioCapturer_SetNoiseReductionMode_005
* @tc.desc  : Test SetNoiseReductionMode returns illegal state if capturer client is released.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_SetNoiseReductionMode_005, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    OH_AudioStream_Result result = SetNoiseReductionModeWithRegisteredCapturer(oHAudioCapturer,
        AUDIO_NOISE_REDUCTION_MODE_STANDARD);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test OH_AudioCapturer_SetNoiseReductionMode API via released handle.
* @tc.number: OH_AudioCapturer_SetNoiseReductionMode_006
* @tc.desc  : Test SetNoiseReductionMode returns invalid param if capturer handle has been released.
*/
/**
* @tc.name  : Test OH_AudioCapturer_GetNoiseReductionMode API.
* @tc.number: OH_AudioCapturer_GetNoiseReductionMode_001
* @tc.desc  : Test GetNoiseReductionMode parameter validation and success path.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetNoiseReductionMode_001, TestSize.Level0)
{
    OH_AudioNoiseReductionMode mode = AUDIO_NOISE_REDUCTION_MODE_FIDELITY;
    EXPECT_EQ(OH_AudioCapturer_GetNoiseReductionMode(nullptr, &mode), AUDIOSTREAM_ERROR_INVALID_PARAM);

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        SUCCESS, SUCCESS, SUCCESS);
    OH_AudioCapturer *audioCapturer = reinterpret_cast<OH_AudioCapturer *>(&oHAudioCapturer);
    EXPECT_EQ(OH_AudioCapturer_GetNoiseReductionMode(audioCapturer, nullptr), AUDIOSTREAM_ERROR_INVALID_PARAM);
    EXPECT_EQ(OH_AudioCapturer_GetNoiseReductionMode(audioCapturer, &mode), AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(mode, AUDIO_NOISE_REDUCTION_MODE_STANDARD);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetNoiseReductionMode API via native error.
* @tc.number: OH_AudioCapturer_GetNoiseReductionMode_002
* @tc.desc  : Test GetNoiseReductionMode returns invalid param if native call fails.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetNoiseReductionMode_002, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        SUCCESS, ERR_ILLEGAL_STATE, SUCCESS);
    OH_AudioCapturer *audioCapturer = reinterpret_cast<OH_AudioCapturer *>(&oHAudioCapturer);
    OH_AudioNoiseReductionMode mode = AUDIO_NOISE_REDUCTION_MODE_FIDELITY;

    EXPECT_EQ(OH_AudioCapturer_GetNoiseReductionMode(audioCapturer, &mode), AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetNoiseReductionMode API via invalid native mode.
* @tc.number: OH_AudioCapturer_GetNoiseReductionMode_003
* @tc.desc  : Test GetNoiseReductionMode returns invalid param if native mode is out of range.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetNoiseReductionMode_003, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.audioCapturer_ = std::make_shared<InvalidNoiseReductionModeCapturer>();
    OH_AudioCapturer *audioCapturer = reinterpret_cast<OH_AudioCapturer *>(&oHAudioCapturer);
    OH_AudioNoiseReductionMode mode = AUDIO_NOISE_REDUCTION_MODE_FIDELITY;

    EXPECT_EQ(OH_AudioCapturer_GetNoiseReductionMode(audioCapturer, &mode), AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetSupportedNoiseReductionModes API.
* @tc.number: OH_AudioCapturer_GetSupportedNoiseReductionModes_001
* @tc.desc  : Test GetSupportedNoiseReductionModes parameter validation and success path.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetSupportedNoiseReductionModes_001, TestSize.Level0)
{
    OH_AudioNoiseReductionMode modes[4] = {AUDIO_NOISE_REDUCTION_MODE_STANDARD};
    uint32_t outModeArraySize = 0;
    EXPECT_EQ(OH_AudioCapturer_GetSupportedNoiseReductionModes(nullptr, modes, 4, &outModeArraySize),
        AUDIOSTREAM_ERROR_INVALID_PARAM);

    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        SUCCESS, SUCCESS, SUCCESS);
    OH_AudioCapturer *audioCapturer = reinterpret_cast<OH_AudioCapturer *>(&oHAudioCapturer);
    EXPECT_EQ(OH_AudioCapturer_GetSupportedNoiseReductionModes(audioCapturer, nullptr, 4, &outModeArraySize),
        AUDIOSTREAM_ERROR_INVALID_PARAM);
    EXPECT_EQ(OH_AudioCapturer_GetSupportedNoiseReductionModes(audioCapturer, modes, 4, nullptr),
        AUDIOSTREAM_ERROR_INVALID_PARAM);
    EXPECT_EQ(OH_AudioCapturer_GetSupportedNoiseReductionModes(audioCapturer, modes, 0, &outModeArraySize),
        AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(outModeArraySize, 0);
    EXPECT_EQ(OH_AudioCapturer_GetSupportedNoiseReductionModes(audioCapturer, modes, 4, &outModeArraySize),
        AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(outModeArraySize, 2);
    EXPECT_EQ(modes[0], AUDIO_NOISE_REDUCTION_MODE_FIDELITY);
    EXPECT_EQ(modes[1], AUDIO_NOISE_REDUCTION_MODE_STANDARD);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetSupportedNoiseReductionModes API via native error.
* @tc.number: OH_AudioCapturer_GetSupportedNoiseReductionModes_002
* @tc.desc  : Test GetSupportedNoiseReductionModes returns service died if native call fails.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetSupportedNoiseReductionModes_002, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        SUCCESS, SUCCESS, ERR_ILLEGAL_STATE);
    OH_AudioCapturer *audioCapturer = reinterpret_cast<OH_AudioCapturer *>(&oHAudioCapturer);
    OH_AudioNoiseReductionMode modes[4] = {AUDIO_NOISE_REDUCTION_MODE_STANDARD};
    uint32_t outModeArraySize = 0;

    EXPECT_EQ(OH_AudioCapturer_GetSupportedNoiseReductionModes(audioCapturer, modes, 4, &outModeArraySize),
        AUDIOSTREAM_ERROR_SERVICE_DIED);

    OHAudioCapturer invalidParamCapturer;
    invalidParamCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        SUCCESS, SUCCESS, ERR_INVALID_PARAM);
    audioCapturer = reinterpret_cast<OH_AudioCapturer *>(&invalidParamCapturer);
    EXPECT_EQ(OH_AudioCapturer_GetSupportedNoiseReductionModes(audioCapturer, modes, 4, &outModeArraySize),
        AUDIOSTREAM_ERROR_SERVICE_DIED);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetSupportedNoiseReductionModes API via invalid native mode.
* @tc.number: OH_AudioCapturer_GetSupportedNoiseReductionModes_003
* @tc.desc  : Test GetSupportedNoiseReductionModes returns invalid param if native mode is out of range.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetSupportedNoiseReductionModes_003, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.audioCapturer_ = std::make_shared<InvalidNoiseReductionModeCapturer>();
    OH_AudioCapturer *audioCapturer = reinterpret_cast<OH_AudioCapturer *>(&oHAudioCapturer);
    OH_AudioNoiseReductionMode modes[4] = {AUDIO_NOISE_REDUCTION_MODE_STANDARD};
    uint32_t outModeArraySize = 0;

    EXPECT_EQ(OH_AudioCapturer_GetSupportedNoiseReductionModes(audioCapturer, modes, 4, &outModeArraySize),
        AUDIOSTREAM_ERROR_INVALID_PARAM);
    EXPECT_EQ(modes[0], AUDIO_NOISE_REDUCTION_MODE_FIDELITY);
}

/**
* @tc.name  : Test OHAudioCapturer::Release API via release failure.
* @tc.number: OHAudioCapturer_Release_002
* @tc.desc  : Test OHAudioCapturer::Release returns false when audioCapturer_->Release() fails.
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_Release_002, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<ReleaseFailureCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    bool result = oHAudioCapturer.Release();
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test OH_AudioCapturer_RequestPlaybackCaptureStart API via start failure.
* @tc.number: OH_AudioCapturer_RequestPlaybackCaptureStart_001
* @tc.desc  : Test OH_AudioCapturer_RequestPlaybackCaptureStart returns error when StartPlaybackCapture fails.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_RequestPlaybackCaptureStart_001, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<StartPlaybackCaptureFailureCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    OH_AudioCapturer* capturer = reinterpret_cast<OH_AudioCapturer*>(&oHAudioCapturer);
    OH_AudioCapturer_OnPlaybackCaptureStartCallback callback =
        [](OH_AudioCapturer* capturer, void* userData,
        OH_AudioStream_PlaybackCaptureStartState state) { return; };

    ObjectRefMap<OHAudioCapturer>::Insert(&oHAudioCapturer);
    OH_AudioStream_Result result = OH_AudioCapturer_RequestPlaybackCaptureStart(capturer, callback, nullptr);
    ObjectRefMap<OHAudioCapturer>::Erase(&oHAudioCapturer);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test OHAudioCapturerOnPlaybackCaptureStartCallback::OnPlaybackCaptureStartResult API via failure state.
* @tc.number: OHAudioCapturerOnPlaybackCaptureStartCallback_001
* @tc.desc  : Test OnPlaybackCaptureStartResult when state != START_STATE_SUCCESS.
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerOnPlaybackCaptureStartCallback_001, TestSize.Level0)
{
    OH_AudioCapturer_OnPlaybackCaptureStartCallback callback =
        [](OH_AudioCapturer* capturer, void* userData,
        OH_AudioStream_PlaybackCaptureStartState state) { return; };

    OHAudioCapturer oHAudioCapturer;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    void* userData = nullptr;

    auto playbackCaptureStartCallback =
        std::make_shared<OHAudioCapturerOnPlaybackCaptureStartCallback>(callback, oH_AudioCapturer, userData);
    EXPECT_NE(playbackCaptureStartCallback, nullptr);

    playbackCaptureStartCallback->OnPlaybackCaptureStartResult(START_STATE_FAILED);
}

/**
* @tc.name  : Test OHAudioCapturerModeCallback::OnReadData API via
*             READ_DATA_CALLBACK_WITH_RESULT with length <= bufLength.
* @tc.number: OHAudioCapturerModeCallback_005
* @tc.desc  : Test OnReadData when length is less than or equal to bufDesc.bufLength.
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerModeCallback_005, TestSize.Level0)
{
    OH_AudioCapturer_OnReadDataCallback readcallback =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) { return; };

    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<StartPlaybackCaptureFailureCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    oHAudioCapturer.readDataCallbackType_ = READ_DATA_CALLBACK_WITH_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerModeCallback =
        std::make_shared<OHAudioCapturerModeCallback>(readcallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerModeCallback, nullptr);

    size_t length = 512;
    oHAudioCapturerModeCallback->OnReadData(length);
}

/**
* @tc.name  : Test OHAudioCapturerModeCallback::OnReadData API via
*             READ_DATA_CALLBACK_WITH_RESULT with length > bufLength.
* @tc.number: OHAudioCapturerModeCallback_006
* @tc.desc  : Test OnReadData when length is greater than bufDesc.bufLength.
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerModeCallback_006, TestSize.Level0)
{
    OH_AudioCapturer_OnReadDataCallback readcallback =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) { return; };

    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<StartPlaybackCaptureFailureCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    oHAudioCapturer.readDataCallbackType_ = READ_DATA_CALLBACK_WITH_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerModeCallback =
        std::make_shared<OHAudioCapturerModeCallback>(readcallback, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerModeCallback, nullptr);

    size_t length = 2048;
    oHAudioCapturerModeCallback->OnReadData(length);
}

/**
* @tc.name  : Test OHAudioCapturerModeCallback::OnReadData API via READ_DATA_CALLBACK_WITHOUT_RESULT.
* @tc.number: OHAudioCapturerModeCallback_007
* @tc.desc  : Test OnReadData when callback type is READ_DATA_CALLBACK_WITHOUT_RESULT.
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturerModeCallback_007, TestSize.Level0)
{
    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnReadData =
        [](OH_AudioCapturer* capturer, void* userData, void* buffer, int32_t length) ->
        int32_t { return 0; };

    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<StartPlaybackCaptureFailureCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    oHAudioCapturer.readDataCallbackType_ = READ_DATA_CALLBACK_WITHOUT_RESULT;
    OH_AudioCapturer* oH_AudioCapturer = (OH_AudioCapturer*)&oHAudioCapturer;
    EXPECT_NE((OHAudioCapturer*)oH_AudioCapturer, nullptr);
    void* userData = nullptr;

    auto oHAudioCapturerModeCallback =
        std::make_shared<OHAudioCapturerModeCallback>(callbacks, oH_AudioCapturer, userData);
    EXPECT_NE(oHAudioCapturerModeCallback, nullptr);

    size_t length = 1024;
    oHAudioCapturerModeCallback->OnReadData(length);
}

/**
* @tc.name  : Test ConvertGetSupportedNoiseReductionReturnCode API via non-SUCCESS return.
* @tc.number: ConvertGetSupportedNoiseReductionReturnCode_001
* @tc.desc  : Test ConvertGetSupportedNoiseReductionReturnCode returns SERVICE_DIED for non-SUCCESS.
*/
HWTEST(OHAudioCaptureUnitTest, ConvertGetSupportedNoiseReductionReturnCode_001, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        SUCCESS, SUCCESS, ERR_ILLEGAL_STATE);
    OH_AudioCapturer *audioCapturer = reinterpret_cast<OH_AudioCapturer *>(&oHAudioCapturer);
    OH_AudioNoiseReductionMode modes[4] = {AUDIO_NOISE_REDUCTION_MODE_STANDARD};
    uint32_t outModeArraySize = 0;

    OH_AudioStream_Result result = OH_AudioCapturer_GetSupportedNoiseReductionModes(
        audioCapturer, modes, 4, &outModeArraySize);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_SERVICE_DIED);
}

/**
* @tc.name  : Test ConvertGetNoiseReductionReturnCode API via non-SUCCESS return.
* @tc.number: ConvertGetNoiseReductionReturnCode_001
* @tc.desc  : Test ConvertGetNoiseReductionReturnCode returns INVALID_PARAM for non-SUCCESS.
*/
HWTEST(OHAudioCaptureUnitTest, ConvertGetNoiseReductionReturnCode_001, TestSize.Level0)
{
    OHAudioCapturer oHAudioCapturer;
    oHAudioCapturer.audioCapturer_ = std::make_shared<NoiseReductionReturnCapturer>(
        SUCCESS, ERR_ILLEGAL_STATE, SUCCESS);
    OH_AudioCapturer *audioCapturer = reinterpret_cast<OH_AudioCapturer *>(&oHAudioCapturer);
    OH_AudioNoiseReductionMode mode = AUDIO_NOISE_REDUCTION_MODE_FIDELITY;

    OH_AudioStream_Result result = OH_AudioCapturer_GetNoiseReductionMode(audioCapturer, &mode);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
}

#ifdef SUPPOR_PERMISSION
/**
* @tc.name  : Test OH_AudioCapturer_Start API via normal state.
* @tc.number: OH_AudioCapturer_Start_001
* @tc.desc  : Test OH_AudioCapturer_Start interface.Returns SUCCESS.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_Start_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioStreamBuilder_SetSamplingRate(builder, AudioSamplingRate::SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, AudioChannel::STEREO);
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    result = OH_AudioCapturer_Start(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    result = OH_AudioCapturer_Pause(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    result = OH_AudioCapturer_Stop(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_SetInputDevice API via normal state.
* @tc.number: OH_AudioCapturer_SetInputDevice_002
* @tc.desc  : Test OH_AudioCapturer_SetInputDevice interface.Returns SUCCESS.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_SetInputDevice_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioStreamBuilder_SetSamplingRate(builder, AudioSamplingRate::SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, AudioChannel::STEREO);
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    result = OH_AudioCapturer_SetInputDevice(audioCapturer, AUDIO_DEVICE_TYPE_EARPIECE);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetFastStatus API via normal state.
* @tc.number: OH_AudioCapturer_GetFastStatus_002
* @tc.desc  : Test OH_AudioCapturer_GetFastStatus interface.Returns SUCCESS.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetFastStatus_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioStreamBuilder_SetSamplingRate(builder, AudioSamplingRate::SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, AudioChannel::STEREO);
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioStream_FastStatus status = AUDIOSTREAM_FASTSTATUS_NORMAL;
    result = OH_AudioCapturer_GetFastStatus(audioCapturer, &status);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_Flush API via normal state.
* @tc.number: OH_AudioCapturer_Flush_003
* @tc.desc  : Test OH_AudioCapturer_Flush interface.Returns SUCCESS.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_Flush_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioStreamBuilder_SetSamplingRate(builder, AudioSamplingRate::SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, AudioChannel::STEREO);
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer_Start(audioCapturer);
    result = OH_AudioCapturer_Flush(audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioCapturer_Stop(audioCapturer);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetTimestamp API via normal state.
* @tc.number: OH_AudioCapturer_GetTimestamp_004
* @tc.desc  : Test OH_AudioCapturer_GetTimestamp interface.Returns SUCCESS.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetTimestamp_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioStreamBuilder_SetSamplingRate(builder, AudioSamplingRate::SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, AudioChannel::STEREO);
    OH_AudioCapturer* audioCapturer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer_Start(audioCapturer);
    int64_t framePosition = 0;
    int64_t timestamp = 0;
    OH_AudioCapturer_GetTimestamp(audioCapturer, CLOCK_MONOTONIC, &framePosition, &timestamp);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioCapturer_Stop(audioCapturer);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test SetCapturerCallback API via register all callback.
* @tc.number: SetCapturerCallback_001
* @tc.desc  : Test SetCapturerCallback interface.Returns SUCCESS.
*/
HWTEST(OHAudioCaptureUnitTest, SetCapturerCallback_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioCaptureUnitTest::CreateCapturerBuilder();
    OH_AudioStreamBuilder_SetSamplingRate(builder, AudioSamplingRate::SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, AudioChannel::STEREO);

    OH_AudioCapturer_Callbacks callbacks;
    callbacks.OH_AudioCapturer_OnReadData = AudioCapturerOnReadData;
    callbacks.OH_AudioCapturer_OnError = AudioErrCallback;
    callbacks.OH_AudioCapturer_OnInterruptEvent = AudioInterruptCallback;
    callbacks.OH_AudioCapturer_OnStreamEvent = AudioEventCallback;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_SetCapturerCallback(builder, callbacks, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioCapturer* audioCapturer;
    result = OH_AudioStreamBuilder_GenerateCapturer(builder, &audioCapturer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioCapturer_Start(audioCapturer);
    OH_AudioCapturer_Stop(audioCapturer);

    OH_AudioCapturer_Release(audioCapturer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test SetReadDataCallback API.
* @tc.number: SetReadDataCallback_001
* @tc.desc  : Test SetReadDataCallback interface. When readDataCallbackType_ == READ_DATA_CALLBACK_WITH_RESULT &&
              capturerCallbacks.onReadDataCallback != nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, SetReadDataCallback_001, TestSize.Level4)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->readDataCallbackType_ = READ_DATA_CALLBACK_WITH_RESULT;
    CapturerCallback capturerCallbacks;
    capturerCallbacks.onReadDataCallback = nullptr;
    void* userData = nullptr;
    oHAudioCapturer->SetReadDataCallback(capturerCallbacks, userData);

    capturerCallbacks.onReadDataCallback = [](OH_AudioCapturer* capturer, void* userData,
        void* buffer, int32_t length) ->void { return; };
    oHAudioCapturer->SetReadDataCallback(capturerCallbacks, userData);

    EXPECT_EQ(oHAudioCapturer->GetCapturerReadDataCallbackType(), READ_DATA_CALLBACK_WITH_RESULT);
}

/**
* @tc.name  : Test SetReadDataCallback API with onReadDataCallback.
* @tc.number: SetReadDataCallback_003
* @tc.desc  : Test SetReadDataCallback when readDataCallbackType_ == READ_DATA_CALLBACK_WITH_RESULT &&
              onReadDataCallback != nullptr to cover if branch at line 1010.
*/
HWTEST(OHAudioCaptureUnitTest, SetReadDataCallback_003, TestSize.Level0)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->readDataCallbackType_ = READ_DATA_CALLBACK_WITH_RESULT;
    CapturerCallback capturerCallbacks;
    capturerCallbacks.onReadDataCallback = [](OH_AudioCapturer* capturer, void* userData,
        void* buffer, int32_t length) ->void { return; };
    void* userData = nullptr;
    oHAudioCapturer->SetReadDataCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test SetReadDataCallback API.
* @tc.number: SetReadDataCallback_002
* @tc.desc  : Test SetReadDataCallback interface. When readDataCallbackType_ == READ_DATA_CALLBACK_WITH_RESULT &&
              capturerCallbacks.onReadDataCallback != nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, SetReadDataCallback_002, TestSize.Level4)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->readDataCallbackType_ = READ_DATA_CALLBACK_WITHOUT_RESULT;
    CapturerCallback capturerCallbacks;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnReadData = nullptr;
    void* userData = nullptr;
    oHAudioCapturer->SetReadDataCallback(capturerCallbacks, userData);

    capturerCallbacks.callbacks.OH_AudioCapturer_OnReadData = AudioCapturerOnReadData;
    oHAudioCapturer->SetReadDataCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test SetStreamEventCallback API.
* @tc.number: SetStreamEventCallback_001
* @tc.desc  : Test SetStreamEventCallback interface. When streamEventCallbackType_ == STREAM_EVENT_CALLBACK_SEPERATED &&
              capturerCallbacks.onDeviceChangeCallback != nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, SetStreamEventCallback_001, TestSize.Level4)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->streamEventCallbackType_ = STREAM_EVENT_CALLBACK_SEPERATED;
    CapturerCallback capturerCallbacks;
    capturerCallbacks.onDeviceChangeCallback = nullptr;
    void* userData = nullptr;
    oHAudioCapturer->SetStreamEventCallback(capturerCallbacks, userData);

    capturerCallbacks.onDeviceChangeCallback = [](OH_AudioCapturer* capturer, void* userData,
        OH_AudioDeviceDescriptorArray* deviceArray) ->void { return; };
    oHAudioCapturer->SetStreamEventCallback(capturerCallbacks, userData);

    oHAudioCapturer->streamEventCallbackType_ = STREAM_EVENT_CALLBACK_COMBINED;
    oHAudioCapturer->SetStreamEventCallback(capturerCallbacks, userData);

    capturerCallbacks.onDeviceChangeCallback = nullptr;
    oHAudioCapturer->SetStreamEventCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test SetStreamEventCallback API with onDeviceChangeCallback.
* @tc.number: SetStreamEventCallback_002
* @tc.desc  : Test SetStreamEventCallback when streamEventCallbackType_ == STREAM_EVENT_CALLBACK_SEPERATED &&
              onDeviceChangeCallback != nullptr to cover if branch at line 1031.
*/
HWTEST(OHAudioCaptureUnitTest, SetStreamEventCallback_002, TestSize.Level0)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->streamEventCallbackType_ = STREAM_EVENT_CALLBACK_SEPERATED;
    CapturerCallback capturerCallbacks;
    capturerCallbacks.onDeviceChangeCallback = [](OH_AudioCapturer* capturer, void* userData,
        OH_AudioDeviceDescriptorArray* deviceArray) ->void { return; };
    void* userData = nullptr;
    oHAudioCapturer->SetStreamEventCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test SetInterruptCallback API.
* @tc.number: SetInterruptCallback_001
* @tc.desc  : Test SetInterruptCallback interface. When interruptCallbackType_ == INTERRUPT_EVENT_CALLBACK_SEPERATED &&
              capturerCallbacks.onInterruptEventCallback != nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, SetInterruptCallback_001, TestSize.Level4)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_SEPERATED;
    CapturerCallback capturerCallbacks;
    capturerCallbacks.onInterruptEventCallback = nullptr;
    void* userData = nullptr;
    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);

    capturerCallbacks.onReadDataCallback = [](OH_AudioCapturer* capturer, void* userData,
        void* buffer, int32_t length) ->void { return; };
    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test SetInterruptCallback API.
* @tc.number: SetInterruptCallback_002
* @tc.desc  : Test SetInterruptCallback interface. When interruptCallbackType_ == INTERRUPT_EVENT_CALLBACK_COMBINED &&
              capturerCallbacks.callbacks.OH_AudioCapturer_OnInterruptEvent != nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, SetInterruptCallback_002, TestSize.Level4)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_COMBINED;
    CapturerCallback capturerCallbacks;
    capturerCallbacks.onInterruptEventCallback = nullptr;
    void* userData = nullptr;
    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);

    capturerCallbacks.onReadDataCallback = [](OH_AudioCapturer* capturer, void* userData,
        void* buffer, int32_t length) ->void { return; };
    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);

    oHAudioCapturer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_DEFAULT;
    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);

    capturerCallbacks.onInterruptEventCallback = nullptr;
    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test SetErrorCallback API.
* @tc.number: SetErrorCallback_001
* @tc.desc  : Test SetErrorCallback interface. When errorCallbackType_ == ERROR_CALLBACK_SEPERATED &&
              capturerCallbacks.onErrorCallback != nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, SetErrorCallback_001, TestSize.Level4)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    CapturerCallback capturerCallbacks;
    capturerCallbacks.onErrorCallback = nullptr;
    void* userData = nullptr;
    oHAudioCapturer->SetErrorCallback(capturerCallbacks, userData);

    capturerCallbacks.onErrorCallback = [](OH_AudioCapturer* capturer, void* userData,
        OH_AudioStream_Result error) ->void { return; };
    oHAudioCapturer->SetErrorCallback(capturerCallbacks, userData);

    oHAudioCapturer->errorCallbackType_ = ERROR_CALLBACK_COMBINED;
    oHAudioCapturer->SetErrorCallback(capturerCallbacks, userData);

    capturerCallbacks.onErrorCallback = nullptr;
    oHAudioCapturer->SetErrorCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test SetErrorCallback API with onErrorCallback.
* @tc.number: SetErrorCallback_002
* @tc.desc  : Test SetErrorCallback when errorCallbackType_ == ERROR_CALLBACK_SEPERATED &&
              onErrorCallback != nullptr to cover if branch at line 1068.
*/
HWTEST(OHAudioCaptureUnitTest, SetErrorCallback_002, TestSize.Level4)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    CapturerCallback capturerCallbacks;
    capturerCallbacks.onErrorCallback = [](OH_AudioCapturer* capturer, void* userData,
        OH_AudioStream_Result error) ->void { return; };
    void* userData = nullptr;
    oHAudioCapturer->SetErrorCallback(capturerCallbacks, userData);
}

/**
* @tc.name  : Test SetInterruptCallback API with combined callback.
* @tc.number: SetInterruptCallback_003
* @tc.desc  : Test SetInterruptCallback when interruptCallbackType_ == INTERRUPT_EVENT_CALLBACK_COMBINED &&
              capturerCallbacks.callbacks.OH_AudioCapturer_OnInterruptEvent != nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, SetInterruptCallback_003, TestSize.Level0)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    AudioCapturerOptions capturerOptions;
    InitializeCapturerOptions(capturerOptions);
    oHAudioCapturer->audioCapturer_ = AudioCapturer::Create(capturerOptions);
    EXPECT_NE(oHAudioCapturer->audioCapturer_, nullptr);

    oHAudioCapturer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_COMBINED;
    CapturerCallback capturerCallbacks;
    capturerCallbacks.callbacks.OH_AudioCapturer_OnInterruptEvent =
        [](OH_AudioCapturer* capturer, void* userData,
        OH_AudioInterrupt_ForceType forceType, OH_AudioInterrupt_Hint hint) -> void { return; };
    void* userData = nullptr;
    oHAudioCapturer->SetInterruptCallback(capturerCallbacks, userData);
}
#endif

/**
* @tc.name  : Test OHAudioCapturer Start when audioCapturer_ is nullptr.
* @tc.number: OHAudioCapturer_Start_Nullptr_001
* @tc.desc  : Test OHAudioCapturer::Start returns false when audioCapturer_ is nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_Start_Nullptr_001, TestSize.Level1)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    EXPECT_EQ(oHAudioCapturer->audioCapturer_, nullptr);
    
    bool result = oHAudioCapturer->Start();
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test OHAudioCapturer Pause when audioCapturer_ is nullptr.
* @tc.number: OHAudioCapturer_Pause_Nullptr_001
* @tc.desc  : Test OHAudioCapturer::Pause returns false when audioCapturer_ is nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_Pause_Nullptr_001, TestSize.Level1)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    EXPECT_EQ(oHAudioCapturer->audioCapturer_, nullptr);
    
    bool result = oHAudioCapturer->Pause();
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test OHAudioCapturer Stop when audioCapturer_ is nullptr.
* @tc.number: OHAudioCapturer_Stop_Nullptr_001
* @tc.desc  : Test OHAudioCapturer::Stop returns false when audioCapturer_ is nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_Stop_Nullptr_001, TestSize.Level1)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    EXPECT_EQ(oHAudioCapturer->audioCapturer_, nullptr);
    
    bool result = oHAudioCapturer->Stop();
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test OHAudioCapturer Flush when audioCapturer_ is nullptr.
* @tc.number: OHAudioCapturer_Flush_Nullptr_001
* @tc.desc  : Test OHAudioCapturer::Flush returns false when audioCapturer_ is nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_Flush_Nullptr_001, TestSize.Level1)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    EXPECT_EQ(oHAudioCapturer->audioCapturer_, nullptr);
    
    bool result = oHAudioCapturer->Flush();
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test OHAudioCapturer Release when audioCapturer_ is nullptr.
* @tc.number: OHAudioCapturer_Release_Nullptr_001
* @tc.desc  : Test OHAudioCapturer::Release returns false when audioCapturer_ is nullptr.
*/
HWTEST(OHAudioCaptureUnitTest, OHAudioCapturer_Release_Nullptr_001, TestSize.Level1)
{
    std::shared_ptr<OHAudioCapturer> oHAudioCapturer = std::make_shared<OHAudioCapturer>();
    EXPECT_NE(oHAudioCapturer, nullptr);
    EXPECT_EQ(oHAudioCapturer->audioCapturer_, nullptr);
    
    bool result = oHAudioCapturer->Release();
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test OH_AudioCapturer_SetInputDevice returns ERR_NOT_SUPPORTED.
* @tc.number: OH_AudioCapturer_SetInputDevice_NotSupported_001
* @tc.desc  : Test SetInputDevice when device type is not supported.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_SetInputDevice_NotSupported_001, TestSize.Level1)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<SetInputDeviceNotSupportedCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    OH_AudioCapturer* capturer = reinterpret_cast<OH_AudioCapturer*>(&oHAudioCapturer);
    ObjectRefMap<OHAudioCapturer>::Insert(&oHAudioCapturer);
    OH_AudioStream_Result result = OH_AudioCapturer_SetInputDevice(capturer, AUDIO_DEVICE_TYPE_SPEAKER);
    ObjectRefMap<OHAudioCapturer>::Erase(&oHAudioCapturer);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test OH_AudioCapturer_SetInputDevice returns error.
* @tc.number: OH_AudioCapturer_SetInputDevice_Error_001
* @tc.desc  : Test SetInputDevice when SetInputDevice returns error.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_SetInputDevice_Error_001, TestSize.Level1)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<SetInputDeviceErrorCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    OH_AudioCapturer* capturer = reinterpret_cast<OH_AudioCapturer*>(&oHAudioCapturer);
    ObjectRefMap<OHAudioCapturer>::Insert(&oHAudioCapturer);
    OH_AudioStream_Result result = OH_AudioCapturer_SetInputDevice(capturer, AUDIO_DEVICE_TYPE_SPEAKER);
    ObjectRefMap<OHAudioCapturer>::Erase(&oHAudioCapturer);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_SYSTEM);
}

/**
* @tc.name  : Test OH_AudioCapturer_SetMuteHint returns ERROR_ILLEGAL_STATE.
* @tc.number: OH_AudioCapturer_SetMuteHint_IllegalState_001
* @tc.desc  : Test SetMuteHint when capturer is in illegal state.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_SetMuteHint_IllegalState_001, TestSize.Level1)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<MuteHintIllegalStateCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    OH_AudioCapturer* capturer = reinterpret_cast<OH_AudioCapturer*>(&oHAudioCapturer);
    ObjectRefMap<OHAudioCapturer>::Insert(&oHAudioCapturer);
    OH_AudioStream_Result result = OH_AudioCapturer_SetMuteHint(capturer, true);
    ObjectRefMap<OHAudioCapturer>::Erase(&oHAudioCapturer);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test OH_AudioCapturer_SetMuteHint returns ERROR_INVALID_PARAM.
* @tc.number: OH_AudioCapturer_SetMuteHint_InvalidParam_001
* @tc.desc  : Test SetMuteHint when parameter is invalid.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_SetMuteHint_InvalidParam_001, TestSize.Level1)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<MuteHintInvalidParamCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    OH_AudioCapturer* capturer = reinterpret_cast<OH_AudioCapturer*>(&oHAudioCapturer);
    ObjectRefMap<OHAudioCapturer>::Insert(&oHAudioCapturer);
    OH_AudioStream_Result result = OH_AudioCapturer_SetMuteHint(capturer, true);
    ObjectRefMap<OHAudioCapturer>::Erase(&oHAudioCapturer);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetFastStatus returns FASTSTATUS_INVALID.
* @tc.number: OH_AudioCapturer_GetFastStatus_Invalid_001
* @tc.desc  : Test GetFastStatus when fast status is invalid.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetFastStatus_Invalid_001, TestSize.Level1)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<GetFastStatusInvalidCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    FastStatus status = oHAudioCapturer.GetFastStatus();
    EXPECT_EQ(status, FastStatus::FASTSTATUS_INVALID);
}

/**
* @tc.name  : Test OH_AudioCapturer_GetTimestamp returns error.
* @tc.number: OH_AudioCapturer_GetTimestamp_Error_001
* @tc.desc  : Test GetTimestamp when GetAudioTime returns false.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_GetTimestamp_Error_001, TestSize.Level1)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<GetAudioTimeFailureCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    Timestamp stamp;
    Timestamp::Timestampbase base = Timestamp::Timestampbase::MONOTONIC;
    bool result = oHAudioCapturer.GetAudioTime(stamp, base);
    EXPECT_FALSE(result);
}

/**
* @tc.name  : Test OH_AudioCapturer_Start returns SUCCESS when Start succeeds.
* @tc.number: OH_AudioCapturer_Start_Success_001
* @tc.desc  : Test OH_AudioCapturer_Start when Start returns true.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_Start_Success_001, TestSize.Level1)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<StartSuccessCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    OH_AudioCapturer* capturer = reinterpret_cast<OH_AudioCapturer*>(&oHAudioCapturer);
    ObjectRefMap<OHAudioCapturer>::Insert(&oHAudioCapturer);
    OH_AudioStream_Result result = OH_AudioCapturer_Start(capturer);
    ObjectRefMap<OHAudioCapturer>::Erase(&oHAudioCapturer);

    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioCapturer_Pause returns SUCCESS when Pause succeeds.
* @tc.number: OH_AudioCapturer_Pause_Success_001
* @tc.desc  : Test OH_AudioCapturer_Pause when Pause returns true.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_Pause_Success_001, TestSize.Level1)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<PauseSuccessCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    OH_AudioCapturer* capturer = reinterpret_cast<OH_AudioCapturer*>(&oHAudioCapturer);
    ObjectRefMap<OHAudioCapturer>::Insert(&oHAudioCapturer);
    OH_AudioStream_Result result = OH_AudioCapturer_Pause(capturer);
    ObjectRefMap<OHAudioCapturer>::Erase(&oHAudioCapturer);

    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioCapturer_Stop returns SUCCESS when Stop succeeds.
* @tc.number: OH_AudioCapturer_Stop_Success_001
* @tc.desc  : Test OH_AudioCapturer_Stop when Stop returns true.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_Stop_Success_001, TestSize.Level1)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<StopSuccessCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    OH_AudioCapturer* capturer = reinterpret_cast<OH_AudioCapturer*>(&oHAudioCapturer);
    ObjectRefMap<OHAudioCapturer>::Insert(&oHAudioCapturer);
    OH_AudioStream_Result result = OH_AudioCapturer_Stop(capturer);
    ObjectRefMap<OHAudioCapturer>::Erase(&oHAudioCapturer);

    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
* @tc.name  : Test OH_AudioCapturer_Flush returns SUCCESS when Flush succeeds.
* @tc.number: OH_AudioCapturer_Flush_Success_001
* @tc.desc  : Test OH_AudioCapturer_Flush when Flush returns true.
*/
HWTEST(OHAudioCaptureUnitTest, OH_AudioCapturer_Flush_Success_001, TestSize.Level1)
{
    OHAudioCapturer oHAudioCapturer;
    std::shared_ptr<AudioCapturer> audioCapturerImpl = std::make_shared<FlushSuccessCapturer>();
    ASSERT_NE(audioCapturerImpl, nullptr);
    oHAudioCapturer.audioCapturer_ = audioCapturerImpl;

    OH_AudioCapturer* capturer = reinterpret_cast<OH_AudioCapturer*>(&oHAudioCapturer);
    ObjectRefMap<OHAudioCapturer>::Insert(&oHAudioCapturer);
    OH_AudioStream_Result result = OH_AudioCapturer_Flush(capturer);
    ObjectRefMap<OHAudioCapturer>::Erase(&oHAudioCapturer);

    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

} // namespace AudioStandard
} // namespace OHOS
