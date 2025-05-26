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

#include <thread>
#include <chrono>
#include "oh_audio_render_unit_test.h"

using namespace testing::ext;
using namespace std::chrono;

namespace {
    constexpr int32_t CHANNEL_2 = 2;
}

namespace OHOS {
namespace AudioStandard {

void OHAudioRenderUnitTest::SetUpTestCase(void) { }

void OHAudioRenderUnitTest::TearDownTestCase(void) { }

void OHAudioRenderUnitTest::SetUp(void) { }

void OHAudioRenderUnitTest::TearDown(void) { }

const int32_t SAMPLING_RATE = 48000; // 48000:SAMPLING_RATE value
const int32_t CHANNEL_COUNT = 2; // 2:CHANNEL_COUNT value
const int32_t LATENCY_MODE = 0;
const int32_t SAMPLE_FORMAT = 1;
const int32_t FRAME_SIZE = 240; // 240:FRAME_SIZE value
uint32_t g_flag = 0;
const float MAX_AUDIO_VOLUME = 1.0f; // volume range is between 0 to 1.
const float MIN_AUDIO_VOLUME = 0.0f; // volume range is between 0 to 1.
const int32_t DURATIONMS = 40; // 40:fade out latency ms

static int32_t AudioRendererOnWriteData(OH_AudioRenderer* capturer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    return 0;
}

static void AudioRendererOnMarkReachedCb(OH_AudioRenderer* renderer, uint32_t samplePos, void* userData)
{
    g_flag = samplePos;
    printf("AudioRendererOnMarkReachedCb samplePos: %d \n", samplePos);
}

class OHAudioRendererWriteCallbackMock {
public:
    void OnWriteData(OH_AudioRenderer* renderer, void* userData,
    void* buffer,
    int32_t bufferLen)
    {
        exeCount_++;
        if (executor_) {
            executor_(renderer, userData, buffer, bufferLen);
        }
    }

    void Install(std::function<void(OH_AudioRenderer*, void*, void*, int32_t)> executor)
    {
        executor_ = executor;
    }

    uint32_t GetExeCount()
    {
        return exeCount_;
    }
private:
    std::function<void(OH_AudioRenderer*, void*, void*, int32_t)> executor_;
    std::atomic<uint32_t> exeCount_ = 0;
};

static int32_t AudioRendererOnWriteDataMock(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    return 0;
}

static OH_AudioData_Callback_Result OnWriteDataCallbackWithValidData(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    return AUDIO_DATA_CALLBACK_RESULT_VALID;
}

static OH_AudioData_Callback_Result OnWriteDataCallbackWithInvalidData(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    return AUDIO_DATA_CALLBACK_RESULT_INVALID;
}

struct UserData {
public:
    enum {
        WRITE_DATA_CALLBACK,

        WRITE_DATA_CALLBACK_WITH_RESULT
    } writeDataCallbackType;
};

static int32_t OnWriteDataCbMock(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLer)
{
    UserData *u = static_cast<UserData*>(userData);
    u->writeDataCallbackType = UserData::WRITE_DATA_CALLBACK;
    return 0;
}

static OH_AudioData_Callback_Result OnWriteDataCbWithValidDataMock(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    UserData *u = static_cast<UserData*>(userData);
    u->writeDataCallbackType = UserData::WRITE_DATA_CALLBACK_WITH_RESULT;
    return AUDIO_DATA_CALLBACK_RESULT_VALID;
}

static OH_AudioData_Callback_Result OnWriteDataCbWithInvalidDataMock(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    UserData *u = static_cast<UserData*>(userData);
    u->writeDataCallbackType = UserData::WRITE_DATA_CALLBACK_WITH_RESULT;
    return AUDIO_DATA_CALLBACK_RESULT_INVALID;
}

OH_AudioStreamBuilder* OHAudioRenderUnitTest::CreateRenderBuilder()
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_RENDERER;
    OH_AudioStreamBuilder_Create(&builder, type);
    return builder;
}

OH_AudioStreamBuilder* InitRenderBuilder()
{
    // create builder
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    // set params and callbacks
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLING_RATE);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_COUNT);
    OH_AudioStreamBuilder_SetLatencyMode(builder, (OH_AudioStream_LatencyMode)LATENCY_MODE);
    OH_AudioStreamBuilder_SetSampleFormat(builder, (OH_AudioStream_SampleFormat)SAMPLE_FORMAT);
    OH_AudioStreamBuilder_SetFrameSizeInCallback(builder, FRAME_SIZE);

    return builder;
}

void CleanupAudioResources(OH_AudioStreamBuilder* builder, OH_AudioRenderer* audioRenderer)
{
    // stop and release client
    OH_AudioStream_Result result = OH_AudioRenderer_Stop(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    result = OH_AudioRenderer_Release(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // destroy the builder
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_GenerateRenderer API via legal state.
 * @tc.number: OH_Audio_Capture_Generate_001
 * @tc.desc  : Test OH_AudioStreamBuilder_GenerateRenderer interface. Returns true, if the result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_Generate_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_GenerateRenderer API via illegal OH_AudioStream_Type.
 * @tc.number: OH_Audio_Render_Generate_002
 * @tc.desc  : Test OH_AudioStreamBuilder_GenerateRenderer interface. Returns error code, if the stream type is
 *             AUDIOSTREAM_TYPE_CAPTURER.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_Generate_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder;
    OH_AudioStream_Type type = AUDIOSTREAM_TYPE_CAPTURER;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_Create(&builder, type);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer* audioRenderer;
    result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_GenerateRenderer API via illegal OH_AudioStream_Type.
 * @tc.number: OH_Audio_Render_Generate_003
 * @tc.desc  : Test OH_AudioStreamBuilder_GenerateRenderer interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if builder is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_Generate_003, TestSize.Level0)
{
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(nullptr, &audioRenderer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_GenerateRenderer API via illegal OH_AudioStream_Type.
 * @tc.number: OH_Audio_Render_Generate_004
 * @tc.desc  : Test OH_AudioStreamBuilder_GenerateRenderer interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRendereris nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_Generate_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_Start API via legal state.
 * @tc.number: Audio_Capturer_Start_001
 * @tc.desc  : Test OH_AudioRenderer_Start interface. Returns true if start is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_Start_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(audioRenderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_Start API via illegal state.
 * @tc.number: Audio_Capturer_Start_002
 * @tc.desc  : Test OH_AudioRenderer_Start interface. Returns error code, if Start interface is called twice.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_Start_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_Start(audioRenderer);

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioRenderer_Release(audioRenderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_Pause API via legal state.
 * @tc.number: OH_Audio_Render_Pause_001
 * @tc.desc  : Test OH_AudioRenderer_Pause interface. Returns true if Pause is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_Pause_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    result = OH_AudioRenderer_Start(audioRenderer);

    result = OH_AudioRenderer_Pause(audioRenderer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(audioRenderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_Pause API via illegal state, Pause without Start first.
 * @tc.number: OH_Audio_Render_Pause_002
 * @tc.desc  : Test OH_AudioRenderer_Pause interface. Returns error code, if Pause without Start first.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_Pause_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_Pause(audioRenderer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioRenderer_Release(audioRenderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_Stop API via legal state.
 * @tc.number: OH_Audio_Render_Stop_001
 * @tc.desc  : Test OH_AudioRenderer_Stop interface. Returns true if Stop is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_Stop_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    result = OH_AudioRenderer_Start(audioRenderer);

    result = OH_AudioRenderer_Stop(audioRenderer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(audioRenderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_Stop API via illegal state, Stop without Start first.
 * @tc.number: OH_Audio_Render_Stop_002
 * @tc.desc  : Test OH_AudioRenderer_Stop interface. Returns error code, if Stop without Start first.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_Stop_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_Stop(audioRenderer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioRenderer_Release(audioRenderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_Flush API via legal state.
 * @tc.number: OH_Audio_Render_Flush_001
 * @tc.desc  : Test OH_AudioRenderer_Flush interface. Returns true if Flush is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_Flush_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    result = OH_AudioRenderer_Start(audioRenderer);

    result = OH_AudioRenderer_Flush(audioRenderer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Release(audioRenderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_Flush API via illegal state.
 * @tc.number: OH_Audio_Render_Flush_002
 * @tc.desc  : Test OH_AudioRenderer_Flush interface. Returns error code, if Flush without Start first.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_Flush_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_Flush(audioRenderer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    OH_AudioRenderer_Release(audioRenderer);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_Release API via legal state.
 * @tc.number: OH_Audio_Render_Release_001
 * @tc.desc  : Test OH_AudioRenderer_Release interface. Returns true if Release is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_Release_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    result = OH_AudioRenderer_Start(audioRenderer);

    result = OH_AudioRenderer_Release(audioRenderer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetCurrentState API via legal state.
 * @tc.number: OH_AudioRenderer_GetCurrentState_001
 * @tc.desc  : Test OH_AudioRenderer_GetCurrentState interface. Return true if the result state is
 *             AUDIOSTREAM_STATE_PREPARED.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetCurrentState_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_State state;
    result = OH_AudioRenderer_GetCurrentState(audioRenderer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(state == AUDIOSTREAM_STATE_PREPARED);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetCurrentState API via legal state.
 * @tc.number: OH_AudioRenderer_GetCurrentState_002
 * @tc.desc  : Test OH_AudioRenderer_GetCurrentState interface. Return true if the result state is
 *             AUDIOSTREAM_STATE_RUNNING.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetCurrentState_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioRenderer_Start(audioRenderer);

    OH_AudioStream_State state;
    result = OH_AudioRenderer_GetCurrentState(audioRenderer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(state == AUDIOSTREAM_STATE_RUNNING);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetCurrentState API via legal state.
 * @tc.number: OH_AudioRenderer_GetCurrentState_003
 * @tc.desc  : Test OH_AudioRenderer_GetCurrentState interface. Return true if the result state is
 *             AUDIOSTREAM_STATE_PAUSED.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetCurrentState_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioRenderer_Start(audioRenderer);
    OH_AudioRenderer_Pause(audioRenderer);

    OH_AudioStream_State state;
    result = OH_AudioRenderer_GetCurrentState(audioRenderer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(state == AUDIOSTREAM_STATE_PAUSED);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetCurrentState API via legal state.
 * @tc.number: OH_AudioRenderer_GetCurrentState_004
 * @tc.desc  : Test OH_AudioRenderer_GetCurrentState interface. Return true if the result state is
 *             AUDIOSTREAM_STATE_STOPPED.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetCurrentState_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioRenderer_Start(audioRenderer);
    OH_AudioRenderer_Stop(audioRenderer);

    OH_AudioStream_State state;
    result = OH_AudioRenderer_GetCurrentState(audioRenderer, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(state == AUDIOSTREAM_STATE_STOPPED);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetCurrentState API via legal state.
 * @tc.number: OH_AudioRenderer_GetCurrentState_005
 * @tc.desc  : Test OH_AudioRenderer_GetCurrentState interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetCurrentState_005, TestSize.Level0)
{
    OH_AudioStream_State state;
    OH_AudioStream_Result result = OH_AudioRenderer_GetCurrentState(nullptr, &state);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetCurrentState API via legal state.
 * @tc.number: OH_AudioRenderer_GetCurrentState_006
 * @tc.desc  : Test OH_AudioRenderer_GetCurrentState interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if state is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetCurrentState_006, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioRenderer_Start(audioRenderer);
    OH_AudioRenderer_Stop(audioRenderer);

    result = OH_AudioRenderer_GetCurrentState(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatencyMode API via legal state.
 * @tc.number: OH_AudioRenderer_GetLatencyMode_001
 * @tc.desc  : Test OH_AudioRenderer_GetLatencyMode interface. Returns true if latencyMode is
 *             AUDIOSTREAM_LATENCY_MODE_NORMAL,because default latency mode is AUDIOSTREAM_LATENCY_MODE_NORMAL.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetLatencyMode_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_LatencyMode latencyMode = AUDIOSTREAM_LATENCY_MODE_NORMAL;
    result = OH_AudioRenderer_GetLatencyMode(audioRenderer, &latencyMode);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(latencyMode == AUDIOSTREAM_LATENCY_MODE_NORMAL);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatencyMode API via legal state.
 * @tc.number: OH_AudioRenderer_GetLatencyMode_002
 * @tc.desc  : Test OH_AudioRenderer_GetLatencyMode interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetLatencyMode_002, TestSize.Level0)
{
    OH_AudioStream_LatencyMode latencyMode = AUDIOSTREAM_LATENCY_MODE_NORMAL;
    OH_AudioStream_Result result = OH_AudioRenderer_GetLatencyMode(nullptr, &latencyMode);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetLatencyMode API via legal state.
 * @tc.number: OH_AudioRenderer_GetLatencyMode_003
 * @tc.desc  : Test OH_AudioRenderer_GetLatencyMode interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if latencyMode is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetLatencyMode_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetLatencyMode(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetStreamId API via legal state.
 * @tc.number: OH_AudioRenderer_GetStreamId_001
 * @tc.desc  : Test OH_AudioRenderer_GetStreamId interface. Returns true if the result is AUDIOSTREAM_SUCCESS.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetStreamId_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    uint32_t streamId;
    result = OH_AudioRenderer_GetStreamId(audioRenderer, &streamId);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetStreamId API via legal state.
 * @tc.number: OH_AudioRenderer_GetStreamId_002
 * @tc.desc  : Test OH_AudioRenderer_GetStreamId interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetStreamId_002, TestSize.Level0)
{
    uint32_t streamId;
    OH_AudioStream_Result result = OH_AudioRenderer_GetStreamId(nullptr, &streamId);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetStreamId API via legal state.
 * @tc.number: OH_AudioRenderer_GetStreamId_003
 * @tc.desc  : Test OH_AudioRenderer_GetStreamId interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if streamId is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetStreamId_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetStreamId(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSamplingRate API via legal state.
 * @tc.number: OH_Audio_Render_GetSamplingRate_001
 * @tc.desc  : Test OH_AudioRenderer_GetSamplingRate interface. Returns true if samplingRate is
 *             SAMPLE_RATE_48000,because default samplingRate is SAMPLE_RATE_48000.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetSamplingRate_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    int32_t rate;
    result = OH_AudioRenderer_GetSamplingRate(audioRenderer, &rate);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(rate == SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSamplingRate API via legal state.
 * @tc.number: OH_Audio_Render_GetSamplingRate_002
 * @tc.desc  : Test OH_AudioRenderer_GetSamplingRate interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetSamplingRate_002, TestSize.Level0)
{
    int32_t rate;
    OH_AudioStream_Result result = OH_AudioRenderer_GetSamplingRate(nullptr, &rate);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSamplingRate API via legal state.
 * @tc.number: OH_Audio_Render_GetSamplingRate_003
 * @tc.desc  : Test OH_AudioRenderer_GetSamplingRate interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if rate is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetSamplingRate_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetSamplingRate(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSampleFormat API via legal state.
 * @tc.number: OH_Audio_Render_GetSampleFormat_001
 * @tc.desc  : Test OH_AudioRenderer_GetSampleFormat interface. Returns true if sampleFormat is
 *             AUDIOSTREAM_SAMPLE_S16LE,because default sampleFormat is AUDIOSTREAM_SAMPLE_S16LE.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetSampleFormat_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_SampleFormat sampleFormat;
    result = OH_AudioRenderer_GetSampleFormat(audioRenderer, &sampleFormat);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(sampleFormat == AUDIOSTREAM_SAMPLE_S16LE);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSampleFormat API via legal state.
 * @tc.number: OH_Audio_Render_GetSampleFormat_002
 * @tc.desc  : Test OH_AudioRenderer_GetSampleFormat interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetSampleFormat_002, TestSize.Level0)
{
    OH_AudioStream_SampleFormat sampleFormat;
    OH_AudioStream_Result result = OH_AudioRenderer_GetSampleFormat(nullptr, &sampleFormat);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSampleFormat API via legal state.
 * @tc.number: OH_Audio_Render_GetSampleFormat_003
 * @tc.desc  : Test OH_AudioRenderer_GetSampleFormat interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if sampleFormat is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetSampleFormat_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetSampleFormat(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetEncodingType API via legal state.
 * @tc.number: OH_Audio_Render_GetEncodingType_001
 * @tc.desc  : Test OH_AudioRenderer_GetEncodingType interface. Returns true if encodingType is
 *             ENCODING_PCM,because default encodingType is ENCODING_PCM.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetEncodingType_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_EncodingType encodingType;
    result = OH_AudioRenderer_GetEncodingType(audioRenderer, &encodingType);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(encodingType == AUDIOSTREAM_ENCODING_TYPE_RAW);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetEncodingType API via legal state.
 * @tc.number: OH_Audio_Render_GetEncodingType_002
 * @tc.desc  : Test OH_AudioRenderer_GetEncodingType interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetEncodingType_002, TestSize.Level0)
{
    OH_AudioStream_EncodingType encodingType;
    OH_AudioStream_Result result = OH_AudioRenderer_GetEncodingType(nullptr, &encodingType);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetEncodingType API via legal state.
 * @tc.number: OH_Audio_Render_GetEncodingType_003
 * @tc.desc  : Test OH_AudioRenderer_GetEncodingType interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if encodingType is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetEncodingType_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetEncodingType(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetFramesWritten API via legal state.
 * @tc.number: OH_AudioRenderer_GetFramesWritten_001
 * @tc.desc  : Test OH_AudioRenderer_GetFramesWritten interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetFramesWritten_001, TestSize.Level0)
{
    int64_t frames;
    OH_AudioStream_Result result = OH_AudioRenderer_GetFramesWritten(nullptr, &frames);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetFramesWritten API via legal state.
 * @tc.number: OH_AudioRenderer_GetFramesWritten_002
 * @tc.desc  : Test OH_AudioRenderer_GetFramesWritten interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if frames is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetFramesWritten_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetFramesWritten(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetFramesWritten API via legal state.
 * @tc.number: OH_AudioRenderer_GetFramesWritten_003
 * @tc.desc  : Test OH_AudioRenderer_GetFramesWritten interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetFramesWritten_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    int64_t frames;
    result = OH_AudioRenderer_GetFramesWritten(audioRenderer, &frames);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetRendererInfo API via legal state.
 * @tc.number: OH_Audio_Render_GetRendererInfo_001
 * @tc.desc  : Test OH_AudioRenderer_GetRendererInfo interface. Returns true if usage is STREAM_USAGE_MEDIA and content
 *             is CONTENT_TYPE_MUSIC.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetRendererInfo_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_Usage usage;
    result = OH_AudioRenderer_GetRendererInfo(audioRenderer, &usage);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(usage, AUDIOSTREAM_USAGE_MUSIC);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetRendererInfo API via legal state.
 * @tc.number: OH_Audio_Render_GetRendererInfo_002
 * @tc.desc  : Test OH_AudioRenderer_GetRendererInfo interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetRendererInfo_002, TestSize.Level0)
{
    OH_AudioStream_Usage usage;
    OH_AudioStream_Result result = OH_AudioRenderer_GetRendererInfo(nullptr, &usage);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetRendererInfo API via legal state.
 * @tc.number: OH_Audio_Render_GetRendererInfo_003
 * @tc.desc  : Test OH_AudioRenderer_GetRendererInfo interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if usage is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetRendererInfo_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetRendererInfo(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetRendererPrivacy API.
 * @tc.number: OH_AudioRenderer_GetRendererPrivacy_001
 * @tc.desc  : Test OH_AudioRenderer_GetRendererPrivacy interface with default privacy AUDIO_STREAM_PRIVACY_TYPE_PUBLIC.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetRendererPrivacy_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_PrivacyType privacyType;
    result = OH_AudioRenderer_GetRendererPrivacy(audioRenderer, &privacyType);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(privacyType, AUDIO_STREAM_PRIVACY_TYPE_PUBLIC);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetRendererPrivacy API.
 * @tc.number: OH_AudioRenderer_GetRendererPrivacy_002
 * @tc.desc  : Test OH_AudioRenderer_GetRendererPrivacy interface with privacy AUDIO_STREAM_PRIVACY_TYPE_PRIVATE.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetRendererPrivacy_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioStream_Result privacyResult = OH_AudioStreamBuilder_SetRendererPrivacy(builder,
        AUDIO_STREAM_PRIVACY_TYPE_PRIVATE);
    EXPECT_EQ(privacyResult, AUDIOSTREAM_SUCCESS);
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_PrivacyType privacyType;
    result = OH_AudioRenderer_GetRendererPrivacy(audioRenderer, &privacyType);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(privacyType, AUDIO_STREAM_PRIVACY_TYPE_PRIVATE);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetRendererPrivacy API.
 * @tc.number: OH_AudioRenderer_GetRendererPrivacy_003
 * @tc.desc  : Test OH_AudioRenderer_GetRendererPrivacy interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetRendererPrivacy_003, TestSize.Level0)
{
    OH_AudioStream_PrivacyType privacyType;
    OH_AudioStream_Result result = OH_AudioRenderer_GetRendererPrivacy(nullptr, &privacyType);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetRendererPrivacy API.
 * @tc.number: OH_AudioRenderer_GetRendererPrivacy_004
 * @tc.desc  : Test OH_AudioRenderer_GetRendererPrivacy interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if privacy is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetRendererPrivacy_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetRendererPrivacy(audioRenderer, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetChannelLayout API via legal state.
 * @tc.number: OH_AudioRenderer_GetChannelLayout_001
 * @tc.desc  : Test OH_AudioRenderer_GetChannelLayout interface. Returns true if channelLayout is
 *             CH_LAYOUT_UNKNOWN, because default channelLayout is CH_LAYOUT_UNKNOWN.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetChannelLayout_001, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer *audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioChannelLayout channelLayout;
    result = OH_AudioRenderer_GetChannelLayout(audioRenderer, &channelLayout);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(channelLayout == OH_AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetChannelLayout API via legal state.
 * @tc.number: OH_AudioRenderer_GetChannelLayout_002
 * @tc.desc  : Test OH_AudioRenderer_GetChannelLayout interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetChannelLayout_002, TestSize.Level0)
{
    OH_AudioChannelLayout channelLayout;
    OH_AudioStream_Result result = OH_AudioRenderer_GetChannelLayout(nullptr, &channelLayout);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetChannelLayout API via legal state.
 * @tc.number: OH_AudioRenderer_GetChannelLayout_003
 * @tc.desc  : Test OH_AudioRenderer_GetChannelLayout interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if channelLayout is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetChannelLayout_003, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer *audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetChannelLayout(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetEffectMode API via legal state.
 * @tc.number: OH_AudioRenderer_GetEffectMode_001
 * @tc.desc  : Test OH_AudioRenderer_GetEffectMode interface. Returns true if effect mode is the same as set.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetEffectMode_001, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer *audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    OH_AudioStream_AudioEffectMode effectMode;
    result = OH_AudioRenderer_SetEffectMode(audioRenderer, OH_AudioStream_AudioEffectMode::EFFECT_DEFAULT);
    EXPECT_TRUE(result == OH_AudioStream_Result::AUDIOSTREAM_SUCCESS);
    result = OH_AudioRenderer_GetEffectMode(audioRenderer, &effectMode);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_TRUE(effectMode == OH_AudioStream_AudioEffectMode::EFFECT_DEFAULT);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetEffectMode API via legal state.
 * @tc.number: OH_AudioRenderer_GetEffectMode_002
 * @tc.desc  : Test OH_AudioRenderer_GetEffectMode interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetEffectMode_002, TestSize.Level0)
{
    OH_AudioStream_AudioEffectMode effectMode;
    OH_AudioStream_Result result = OH_AudioRenderer_GetEffectMode(nullptr, &effectMode);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetEffectMode API via legal state.
 * @tc.number: OH_AudioRenderer_GetEffectMode_003
 * @tc.desc  : Test OH_AudioRenderer_GetEffectMode interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if effectMode is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetEffectMode_003, TestSize.Level0)
{
    OH_AudioStreamBuilder *builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer *audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetEffectMode(audioRenderer, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

#ifdef AUDIO_OH_RENDER_UNIT_TEST
/**
 * @tc.name  : Test OH_AudioRenderer_GetUnderflowCount API.
 * @tc.number: OH_AudioRenderer_GetUnderflowCount_001
 * @tc.desc  : Test OH_AudioRenderer_GetUnderflowCount interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetUnderflowCount_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_COMMUNICATION;
    OH_AudioStreamBuilder_SetRendererInfo(builder, usage);

    OHAudioRendererWriteCallbackMock writeCallbackMock;

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &writeCallbackMock);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    std::mutex mutex;
    std::condition_variable cv;
    int32_t count = 0;
    writeCallbackMock.Install([&count, &mutex, &cv](OH_AudioRenderer* renderer, void* userData,
        void* buffer,
        int32_t bufferLen) {
            std::lock_guard lock(mutex);
            cv.notify_one();

            // sleep time trigger underflow
            if (count == 1) {
                std::this_thread::sleep_for(200ms);
            }
            count++;
        });

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    std::unique_lock lock(mutex);
    cv.wait_for(lock, 1s, [&count] {
        // count > 1 ensure sleeped
        return count > 1;
    });
    lock.unlock();

    uint32_t underFlowCount;
    result = OH_AudioRenderer_GetUnderflowCount(audioRenderer, &underFlowCount);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    OH_AudioRenderer_Stop(audioRenderer);
    OH_AudioRenderer_Release(audioRenderer);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetUnderflowCount API.
 * @tc.number: OH_AudioRenderer_GetUnderflowCount_002
 * @tc.desc  : Test OH_AudioRenderer_GetUnderflowCount interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetUnderflowCount_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_COMMUNICATION;
    OH_AudioStreamBuilder_SetRendererInfo(builder, usage);

    OHAudioRendererWriteCallbackMock writeCallbackMock;

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &writeCallbackMock);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    std::mutex mutex;
    std::condition_variable cv;
    int32_t count = 0;
    writeCallbackMock.Install([&count, &mutex, &cv](OH_AudioRenderer* renderer, void* userData,
        void* buffer,
        int32_t bufferLen) {
            std::lock_guard lock(mutex);
            cv.notify_one();

            // sleep time trigger underflow
            if (count == 0) {
                std::this_thread::sleep_for(200ms);
            }
            count++;
        });

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    std::unique_lock lock(mutex);
    cv.wait_for(lock, 1s, [&count] {
        // count > 1 ensure sleeped
        return count > 1;
    });
    lock.unlock();

    uint32_t underFlowCount;
    result = OH_AudioRenderer_GetUnderflowCount(audioRenderer, &underFlowCount);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(underFlowCount, 0);

    OH_AudioRenderer_Stop(audioRenderer);
    OH_AudioRenderer_Release(audioRenderer);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetUnderflowCount API.
 * @tc.number: OH_AudioRenderer_GetUnderflowCount_003
 * @tc.desc  : Test OH_AudioRenderer_GetUnderflowCount interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetUnderflowCount_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_COMMUNICATION;
    OH_AudioStreamBuilder_SetRendererInfo(builder, usage);

    OHAudioRendererWriteCallbackMock writeCallbackMock;

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &writeCallbackMock);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    uint32_t underFlowCount;
    result = OH_AudioRenderer_GetUnderflowCount(audioRenderer, &underFlowCount);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(underFlowCount, 0);

    OH_AudioRenderer_Stop(audioRenderer);
    OH_AudioRenderer_Release(audioRenderer);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetUnderflowCount API.
 * @tc.number: OH_AudioRenderer_GetUnderflowCount_004
 * @tc.desc  : Test OH_AudioRenderer_GetUnderflowCount interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetUnderflowCount_004, TestSize.Level0)
{
    for (auto sleepTimes : {200ms, 400ms, 600ms}) {
        OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

        OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
        OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
        OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_COMMUNICATION;
        OH_AudioStreamBuilder_SetRendererInfo(builder, usage);

        OHAudioRendererWriteCallbackMock writeCallbackMock;

        OH_AudioRenderer_Callbacks callbacks;
        callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataMock;
        OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &writeCallbackMock);

        OH_AudioRenderer* audioRenderer;
        OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
        EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

        std::mutex mutex;
        std::condition_variable cv;
        int32_t count = 0;
        writeCallbackMock.Install([&count, &mutex, &cv, sleepTimes](OH_AudioRenderer* renderer, void* userData,
            void* buffer,
            int32_t bufferLen) {
                std::lock_guard lock(mutex);
                cv.notify_one();

                // sleep time trigger underflow
                if (count == 1) {
                    std::this_thread::sleep_for(sleepTimes);
                }
                count++;
            });

        result = OH_AudioRenderer_Start(audioRenderer);
        EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

        std::unique_lock lock(mutex);
        cv.wait_for(lock, 1s, [&count] {
            // count > 1 ensure sleeped
            return count > 10;
        });
        lock.unlock();

        uint32_t underFlowCount = 0;
        result = OH_AudioRenderer_GetUnderflowCount(audioRenderer, &underFlowCount);
        EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

        OH_AudioRenderer_Stop(audioRenderer);
        OH_AudioRenderer_Release(audioRenderer);

        OH_AudioStreamBuilder_Destroy(builder);
    }
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetUnderflowCount API.
 * @tc.number: OH_AudioRenderer_GetUnderflowCount_005
 * @tc.desc  : Test OH_AudioRenderer_GetUnderflowCount interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetUnderflowCount_005, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_COMMUNICATION;
    OH_AudioStreamBuilder_SetRendererInfo(builder, usage);

    OHAudioRendererWriteCallbackMock writeCallbackMock;

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &writeCallbackMock);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    std::this_thread::sleep_for(1s);

    uint32_t underFlowCount;
    result = OH_AudioRenderer_GetUnderflowCount(audioRenderer, &underFlowCount);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(underFlowCount, 0);

    OH_AudioRenderer_Stop(audioRenderer);
    OH_AudioRenderer_Release(audioRenderer);

    OH_AudioStreamBuilder_Destroy(builder);
}
#endif
/**
 * @tc.name  : Test OH_AudioRenderer_GetVolume API via illegal state.
 * @tc.number: OH_Audio_Render_GetVolume_001
 * @tc.desc  : Test OH_AudioRenderer_GetVolume interface with nullptr audioRenderer.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetVolume_001, TestSize.Level0)
{
    OH_AudioRenderer* audioRenderer = nullptr;
    float volume;
    OH_AudioStream_Result result = OH_AudioRenderer_GetVolume(audioRenderer, &volume);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetVolume API via legal state.
 * @tc.number: OH_Audio_Render_GetVolume_002
 * @tc.desc  : Test OH_AudioRenderer_GetVolume interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetVolume_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float volume;
    result = OH_AudioRenderer_GetVolume(audioRenderer, &volume);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetVolume API via legal state.
 * @tc.number: OH_Audio_Render_GetVolume_003
 * @tc.desc  : Test OH_AudioRenderer_GetVolume interface after set volume call.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetVolume_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float volumeSet = 0.5;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    float volumeGet;
    result = OH_AudioRenderer_GetVolume(audioRenderer, &volumeGet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(volumeGet, 0.5);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetVolume API via legal state.
 * @tc.number: OH_Audio_Render_GetVolume_004
 * @tc.desc  : Test OH_AudioRenderer_GetVolume interface after set volume fails.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetVolume_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float volumeSet = 0.5;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    volumeSet = 1.5;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    float volumeGet;
    result = OH_AudioRenderer_GetVolume(audioRenderer, &volumeGet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    EXPECT_EQ(volumeGet, 0.5);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetVolume API via illegal state.
 * @tc.number: OH_Audio_Render_SetVolume_001
 * @tc.desc  : Test OH_AudioRenderer_SetVolume interface with nullptr audioRenderer.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetVolume_001, TestSize.Level0)
{
    OH_AudioRenderer* audioRenderer = nullptr;
    float volumeSet = 0.5;
    OH_AudioStream_Result result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetVolume API via legal state.
 * @tc.number: OH_Audio_Render_SetVolume_002
 * @tc.desc  : Test OH_AudioRenderer_SetVolume interface between minimum and maximum volumes.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetVolume_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float volumeSet = 0.5;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetVolume API via legal state.
 * @tc.number: OH_Audio_Render_SetVolume_003
 * @tc.desc  : Test OH_AudioRenderer_SetVolume interface for minimum and maximum volumes.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetVolume_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float volumeSet = MIN_AUDIO_VOLUME;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    volumeSet = MAX_AUDIO_VOLUME;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetVolume API via illegal state.
 * @tc.number: OH_Audio_Render_SetVolume_004
 * @tc.desc  : Test OH_AudioRenderer_SetVolume interface out of volumes range.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetVolume_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float volumeSet = -0.5;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    volumeSet = -1.5;
    result = OH_AudioRenderer_SetVolume(audioRenderer, volumeSet);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetVolumeWithRamp API via illegal state.
 * @tc.number: OH_Audio_Render_SetVolumeWithRamp_001
 * @tc.desc  : Test OH_AudioRenderer_SetVolumeWithRamp interface with nullptr audioRenderer.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetVolumeWithRamp_001, TestSize.Level0)
{
    OH_AudioRenderer* audioRenderer = nullptr;
    float volumeSet = MIN_AUDIO_VOLUME;
    int32_t durationMs = DURATIONMS;
    OH_AudioStream_Result result = OH_AudioRenderer_SetVolumeWithRamp(audioRenderer, volumeSet, durationMs);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetVolumeWithRamp API via legal state.
 * @tc.number: OH_Audio_Render_SetVolumeWithRamp_002
 * @tc.desc  : Test OH_AudioRenderer_SetVolumeWithRamp interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetVolumeWithRamp_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    float volumeSet = MIN_AUDIO_VOLUME;
    int32_t durationMs = DURATIONMS;
    result = OH_AudioRenderer_SetVolumeWithRamp(audioRenderer, volumeSet, durationMs);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetMarkPosition API via illegal state.
 * @tc.number: OH_Audio_Render_SetMarkPosition_001
 * @tc.desc  : Test OH_AudioRenderer_SetMarkPosition interface with nullptr audioRenderer.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetMarkPosition_001, TestSize.Level0)
{
    OH_AudioRenderer* audioRenderer = nullptr;
    uint32_t samplePos = 1;
    OH_AudioRenderer_OnMarkReachedCallback callback = AudioRendererOnMarkReachedCb;
    OH_AudioStream_Result result = OH_AudioRenderer_SetMarkPosition(audioRenderer, samplePos, callback, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetMarkPosition API via legal state.
 * @tc.number: OH_Audio_Render_SetMarkPosition_002
 * @tc.desc  : Test OH_AudioRenderer_SetMarkPosition interface.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetMarkPosition_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    uint32_t samplePos = 1;
    OH_AudioRenderer_OnMarkReachedCallback callback = AudioRendererOnMarkReachedCb;
    result = OH_AudioRenderer_SetMarkPosition(audioRenderer, samplePos, callback, nullptr);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetMarkPosition API via illegal state.
 * @tc.number: OH_Audio_Render_SetMarkPosition_003
 * @tc.desc  : Test OH_AudioRenderer_SetMarkPosition interface with incorrect samplepos value.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetMarkPosition_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    uint32_t samplePos = 0;
    OH_AudioRenderer_OnMarkReachedCallback callback = AudioRendererOnMarkReachedCb;
    result = OH_AudioRenderer_SetMarkPosition(audioRenderer, samplePos, callback, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetMarkPosition API via legal state.
 * @tc.number: OH_Audio_Render_SetMarkPosition_004
 * @tc.desc  : Test OH_AudioRenderer_SetMarkPosition interface with callback.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetMarkPosition_004, TestSize.Level0)
{
    // 1. create
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    // 2. set params and callbacks
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLING_RATE);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_COUNT);
    OH_AudioStreamBuilder_SetLatencyMode(builder, (OH_AudioStream_LatencyMode)LATENCY_MODE);
    OH_AudioStreamBuilder_SetSampleFormat(builder, (OH_AudioStream_SampleFormat)SAMPLE_FORMAT);
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteData;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, nullptr);
    // 3. set buffer size to FRAME_SIZE
    result = OH_AudioStreamBuilder_SetFrameSizeInCallback(builder, FRAME_SIZE);

    OH_AudioRenderer* audioRenderer;
    result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(g_flag, 0);
    uint32_t samplePos = 1;
    OH_AudioRenderer_OnMarkReachedCallback callback = AudioRendererOnMarkReachedCb;
    result = OH_AudioRenderer_SetMarkPosition(audioRenderer, samplePos, callback, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // 4. start
    result = OH_AudioRenderer_Start(audioRenderer);
    sleep(2);
    EXPECT_EQ(g_flag, 1);
    // 5. stop and release client
    result = OH_AudioRenderer_Stop(audioRenderer);
    result = OH_AudioRenderer_Release(audioRenderer);

    // 6. destroy the builder
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetMarkPosition API via legal state.
 * @tc.number: OH_Audio_Render_SetMarkPosition_005
 * @tc.desc  : Test OH_AudioRenderer_SetMarkPosition interface multiple times.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_SetMarkPosition_005, TestSize.Level0)
{
    // 1. create
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    // 2. set params and callbacks
    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLING_RATE);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_COUNT);
    OH_AudioStreamBuilder_SetLatencyMode(builder, (OH_AudioStream_LatencyMode)LATENCY_MODE);
    OH_AudioStreamBuilder_SetSampleFormat(builder, (OH_AudioStream_SampleFormat)SAMPLE_FORMAT);
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteData;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, nullptr);
    // 3. set buffer size to FRAME_SIZE
    result = OH_AudioStreamBuilder_SetFrameSizeInCallback(builder, FRAME_SIZE);

    OH_AudioRenderer* audioRenderer;
    result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    uint32_t samplePos = 1;
    OH_AudioRenderer_OnMarkReachedCallback callback = AudioRendererOnMarkReachedCb;
    result = OH_AudioRenderer_SetMarkPosition(audioRenderer, samplePos, callback, nullptr);
    result = OH_AudioRenderer_SetMarkPosition(audioRenderer, samplePos, callback, nullptr);
    result = OH_AudioRenderer_SetMarkPosition(audioRenderer, samplePos, callback, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    // 4. start
    result = OH_AudioRenderer_Start(audioRenderer);
    sleep(2);
    EXPECT_EQ(g_flag, 1);
    // 5. stop and release client
    result = OH_AudioRenderer_Stop(audioRenderer);
    result = OH_AudioRenderer_Release(audioRenderer);

    // 6. destroy the builder
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_CancelMark API via illegal state.
 * @tc.number: OH_Audio_Render_CancelMark_001
 * @tc.desc  : Test OH_AudioRenderer_CancelMark interface with nullptr audioRenderer.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_CancelMark_001, TestSize.Level0)
{
    OH_AudioRenderer* audioRenderer = nullptr;
    OH_AudioStream_Result result = OH_AudioRenderer_CancelMark(audioRenderer);
    EXPECT_TRUE(result == AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : Test OH_AudioRenderer_CancelMark API via legal state.
 * @tc.number: OH_Audio_Render_CancelMark_002
 * @tc.desc  : Test OH_AudioRenderer_CancelMark interface without callback.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_CancelMark_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    result = OH_AudioRenderer_CancelMark(audioRenderer);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback API via legal state.
 * @tc.number: OH_Audio_Render_WriteDataCallback_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback interface with VALID result.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_WriteDataCallback_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = InitRenderBuilder();

    OH_AudioRenderer_OnWriteDataCallback callback = OnWriteDataCallbackWithValidData;
    OH_AudioStreamBuilder_SetRendererWriteDataCallback(builder, callback, nullptr);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    sleep(2);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    CleanupAudioResources(builder, audioRenderer);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback API via legal state.
 * @tc.number: OH_Audio_Render_WriteDataCallback_002
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback interface with INVALID result.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_WriteDataCallback_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = InitRenderBuilder();

    OH_AudioRenderer_OnWriteDataCallback callback = OnWriteDataCallbackWithInvalidData;
    OH_AudioStreamBuilder_SetRendererWriteDataCallback(builder, callback, nullptr);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    sleep(2);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    CleanupAudioResources(builder, audioRenderer);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback API via legal state.
 * @tc.number: OH_Audio_Render_WriteDataCallback_003
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback interface with VALID result
 *             overwrites OH_AudioStreamBuilder_SetRendererCallback interface.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_WriteDataCallback_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = InitRenderBuilder();

    UserData userData;
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = OnWriteDataCbMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &userData);

    OH_AudioRenderer_OnWriteDataCallback callback = OnWriteDataCbWithValidDataMock;
    OH_AudioStreamBuilder_SetRendererWriteDataCallback(builder, callback, &userData);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    sleep(2);
    EXPECT_TRUE(userData.writeDataCallbackType == UserData::WRITE_DATA_CALLBACK_WITH_RESULT);

    CleanupAudioResources(builder, audioRenderer);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback API via legal state.
 * @tc.number: OH_Audio_Render_WriteDataCallback_004
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback interface with INVALID result
 *             overwrites OH_AudioStreamBuilder_SetRendererCallback interface.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_WriteDataCallback_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = InitRenderBuilder();

    UserData userData;
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = OnWriteDataCbMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &userData);

    OH_AudioRenderer_OnWriteDataCallback callback = OnWriteDataCbWithInvalidDataMock;
    OH_AudioStreamBuilder_SetRendererWriteDataCallback(builder, callback, &userData);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    sleep(2);
    EXPECT_TRUE(userData.writeDataCallbackType == UserData::WRITE_DATA_CALLBACK_WITH_RESULT);

    CleanupAudioResources(builder, audioRenderer);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback API via legal state.
 * @tc.number: OH_Audio_Render_WriteDataCallback_005
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererCallback interface
 *             overwrites OH_AudioStreamBuilder_SetRendererWriteDataCallback interface with VALID result.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_WriteDataCallback_005, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = InitRenderBuilder();

    UserData userData;
    OH_AudioRenderer_OnWriteDataCallback callback = OnWriteDataCbWithValidDataMock;
    OH_AudioStreamBuilder_SetRendererWriteDataCallback(builder, callback, &userData);

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = OnWriteDataCbMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &userData);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    sleep(2);
    EXPECT_TRUE(userData.writeDataCallbackType == UserData::WRITE_DATA_CALLBACK);

    CleanupAudioResources(builder, audioRenderer);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallback API via legal state.
 * @tc.number: OH_Audio_Render_WriteDataCallback_006
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererCallback interface
 *             overwrites OH_AudioStreamBuilder_SetRendererWriteDataCallback interface with INVALID result.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_WriteDataCallback_006, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = InitRenderBuilder();

    UserData userData;
    OH_AudioRenderer_OnWriteDataCallback callback = OnWriteDataCbWithInvalidDataMock;
    OH_AudioStreamBuilder_SetRendererWriteDataCallback(builder, callback, &userData);
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = OnWriteDataCbMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &userData);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    sleep(2);
    EXPECT_TRUE(userData.writeDataCallbackType == UserData::WRITE_DATA_CALLBACK);

    CleanupAudioResources(builder, audioRenderer);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetDefaultOutputDevice API via legal state.
 * @tc.number: OH_AudioRenderer_SetDefaultOutputDevice
 * @tc.desc  : Test OH_AudioRenderer_SetDefaultOutputDevice interface
 *             overwrites OH_AudioRenderer_SetDefaultOutputDevice interface with valid result.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_SetDefaultOutputDevice_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_MESSAGE;
    OH_AudioStreamBuilder_SetRendererInfo(builder, usage);

    OHAudioRendererWriteCallbackMock writeCallbackMock;

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &writeCallbackMock);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    std::this_thread::sleep_for(1s);

    result = OH_AudioRenderer_SetDefaultOutputDevice(audioRenderer, AUDIO_DEVICE_TYPE_EARPIECE);
    EXPECT_TRUE(result == AUDIOSTREAM_SUCCESS || result == AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    std::this_thread::sleep_for(5s);

    OH_AudioRenderer_Stop(audioRenderer);
    OH_AudioRenderer_Release(audioRenderer);

    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetDefaultOutputDevice API via legal state.
 * @tc.number: OH_AudioRenderer_SetDefaultOutputDevice
 * @tc.desc  : Test OH_AudioRenderer_SetDefaultOutputDevice interface
 *             overwrites OH_AudioRenderer_SetDefaultOutputDevice interface with valid result.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_SetDefaultOutputDevice_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_VOICE_MESSAGE;
    OH_AudioStreamBuilder_SetRendererInfo(builder, usage);

    OHAudioRendererWriteCallbackMock writeCallbackMock;

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &writeCallbackMock);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetDefaultOutputDevice API via legal state.
 * @tc.number: OH_AudioRenderer_SetDefaultOutputDevice
 * @tc.desc  : Test OH_AudioRenderer_SetDefaultOutputDevice interface
 *             overwrites OH_AudioRenderer_SetDefaultOutputDevice interface with valid result.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_SetDefaultOutputDevice_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();

    OH_AudioStreamBuilder_SetSamplingRate(builder, SAMPLE_RATE_48000);
    OH_AudioStreamBuilder_SetChannelCount(builder, CHANNEL_2);
    OH_AudioStream_Usage usage = AUDIOSTREAM_USAGE_RINGTONE;
    OH_AudioStreamBuilder_SetRendererInfo(builder, usage);

    OHAudioRendererWriteCallbackMock writeCallbackMock;

    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData = AudioRendererOnWriteDataMock;
    OH_AudioStreamBuilder_SetRendererCallback(builder, callbacks, &writeCallbackMock);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    std::this_thread::sleep_for(1s);

    result = OH_AudioRenderer_SetDefaultOutputDevice(audioRenderer, AUDIO_DEVICE_TYPE_EARPIECE);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);

    std::this_thread::sleep_for(5s);

    OH_AudioRenderer_Stop(audioRenderer);
    OH_AudioRenderer_Release(audioRenderer);

    OH_AudioStreamBuilder_Destroy(builder);
}
/**
 * @tc.name  : Test OH_AudioRenderer_GetChannelCount API via illegal state.
 * @tc.number: OH_Audio_Render_GetChannelCount_001
 * @tc.desc  : Test OH_AudioRenderer_GetChannelCount interface with nullptr audioRenderer.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_GetChannelCount_001, TestSize.Level0)
{
    int32_t channelCount = 0;
    OH_AudioStream_Result result = OH_AudioRenderer_GetChannelCount(nullptr, &channelCount);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
}
/**
 * @tc.name  : Test OH_AudioRenderer_GetChannelCount API via legal state.
 * @tc.number: OH_AudioRenderer_GetChannelCount_002
 * @tc.desc  : Test OH_AudioRenderer_GetChannelCount interface. Returns AUDIOSTREAM_SUCCESS.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetChannelCount_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    int32_t channelCount = 0;
    result = OH_AudioRenderer_GetChannelCount(audioRenderer, &channelCount);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}
/**
 * @tc.name  : Test OH_AudioRenderer_GetChannelCount API via legal state.
 * @tc.number: OH_AudioRenderer_GetChannelCount_003
 * @tc.desc  : Test OH_AudioRenderer_GetChannelCount interface with nullptr channelCount.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetChannelCount_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetChannelCount(audioRenderer, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}
/**
 * @tc.name  : Test OH_AudioRenderer_GetTimestamp API via legal state.
 * @tc.number: OH_AudioRenderer_GetTimestamp_002
 * @tc.desc  : Test OH_AudioRenderer_GetTimestamp interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if GetAudioTime error
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetTimestamp_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    clockid_t clockId = CLOCK_MONOTONIC;
    int64_t framePosition = 0;
    int64_t timestamp = 0;

    result = OH_AudioRenderer_GetTimestamp(audioRenderer, clockId, &framePosition, &timestamp);

    EXPECT_NE(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}
/**
 * @tc.name  : Test OH_AudioRenderer_GetTimestamp API via legal state.
 * @tc.number: OH_AudioRenderer_GetTimestamp_003
 * @tc.desc  : Test OH_AudioRenderer_GetTimestamp interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetTimestamp_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    clockid_t clockId = CLOCK_MONOTONIC;
    int64_t framePosition = 0;
    int64_t timestamp = 0;

    result = OH_AudioRenderer_GetTimestamp(nullptr, clockId, &framePosition, &timestamp);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetTimestamp API via legal state.
 * @tc.number: OH_AudioRenderer_GetTimestamp_004
 * @tc.desc  : Test OH_AudioRenderer_GetTimestamp interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             when clockId is CLOCK_REALTIME instead of CLOCK_MONOTONIC .
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetTimestamp_004, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    clockid_t clockId = CLOCK_REALTIME;
    int64_t framePosition = 0;
    int64_t timestamp = 0;

    result = OH_AudioRenderer_GetTimestamp(audioRenderer, clockId, &framePosition, &timestamp);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetTimestamp API via legal state.
 * @tc.number: OH_AudioRenderer_GetTimestamp_005
 * @tc.desc  : Test OH_AudioRenderer_GetTimestamp interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if framePosition is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetTimestamp_005, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    clockid_t clockId = CLOCK_MONOTONIC;
    int64_t timestamp = 0;

    result = OH_AudioRenderer_GetTimestamp(audioRenderer, clockId, nullptr, &timestamp);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetTimestamp API via legal state.
 * @tc.number: OH_AudioRenderer_GetTimestamp_006
 * @tc.desc  : Test OH_AudioRenderer_GetTimestamp interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if timestamp is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetTimestamp_006, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    clockid_t clockId = CLOCK_MONOTONIC;
    int64_t framePosition = 0;

    result = OH_AudioRenderer_GetTimestamp(audioRenderer, clockId, &framePosition, nullptr);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetFrameSizeInCallback API via legal state.
 * @tc.number: OH_AudioRenderer_GetFrameSizeInCallback_001
 * @tc.desc  : Test OH_AudioRenderer_GetFrameSizeInCallback interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetFrameSizeInCallback_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    int32_t frameSize = 0;

    result = OH_AudioRenderer_GetFrameSizeInCallback(nullptr, &frameSize);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetFrameSizeInCallback API via legal state.
 * @tc.number: OH_AudioRenderer_GetFrameSizeInCallback_002
 * @tc.desc  : Test OH_AudioRenderer_GetFrameSizeInCallback interface. Returns  AUDIOSTREAM_SUCCESS
 *             if all is right.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetFrameSizeInCallback_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    int32_t frameSize = 0;

    result = OH_AudioRenderer_GetFrameSizeInCallback(audioRenderer, &frameSize);

    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetFrameSizeInCallback API via legal state.
 * @tc.number: OH_AudioRenderer_GetFrameSizeInCallback_003
 * @tc.desc  : Test OH_AudioRenderer_GetFrameSizeInCallback interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if frameSize is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetFrameSizeInCallback_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetFrameSizeInCallback(audioRenderer, nullptr);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSpeed API via legal state.
 * @tc.number: OH_AudioRenderer_GetSpeed_001
 * @tc.desc  : Test OH_AudioRenderer_GetSpeed interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetSpeed_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    float speed = 1.0 ;

    result = OH_AudioRenderer_GetSpeed(nullptr, &speed);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSpeed API via legal state.
 * @tc.number: OH_AudioRenderer_GetSpeed_002
 * @tc.desc  : Test OH_AudioRenderer_GetSpeed interface. Returns  AUDIOSTREAM_SUCCESS
 *             if all is right.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetSpeed_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    float speed = 1.0 ;

    result = OH_AudioRenderer_GetSpeed(audioRenderer, &speed);

    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetSpeed API via legal state.
 * @tc.number: OH_AudioRenderer_SetSpeed_001
 * @tc.desc  : Test OH_AudioRenderer_SetSpeed interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_SetSpeed_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    float speed = 1.0 ;

    result = OH_AudioRenderer_SetSpeed(nullptr, speed);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetSpeed API via legal state.
 * @tc.number: OH_AudioRenderer_SetSpeed_002
 * @tc.desc  : Test OH_AudioRenderer_SetSpeed interface. Returns  AUDIOSTREAM_SUCCESS
 *             if all is right.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_SetSpeed_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    float speed = 1.0;

    result = OH_AudioRenderer_SetSpeed(audioRenderer, speed);

    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    float expSpeed ;
    result =OH_AudioRenderer_GetSpeed(audioRenderer, &expSpeed);
    EXPECT_EQ(speed, expSpeed);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSpeed API via legal state.
 * @tc.number: OH_AudioRenderer_GetSpeed_003
 * @tc.desc  : Test OH_AudioRenderer_GetSpeed interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if speed is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetSpeed_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetSpeed(audioRenderer, nullptr);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetSilentModeAndMixWithOthers API via legal state.
 * @tc.number: OH_AudioRenderer_SetSilentModeAndMixWithOthers_001
 * @tc.desc  : Test OH_AudioRenderer_SetSilentModeAndMixWithOthers interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_SetSilentModeAndMixWithOthers_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    bool on = true;
    result = OH_AudioRenderer_SetSilentModeAndMixWithOthers(nullptr, on);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_SetSilentModeAndMixWithOthers API via legal state.
 * @tc.number: OH_AudioRenderer_SetSilentModeAndMixWithOthers_002
 * @tc.desc  : Test OH_AudioRenderer_SetSilentModeAndMixWithOthers interface. Returns  AUDIOSTREAM_SUCCESS
 *             if all is right.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_SetSilentModeAndMixWithOthers_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    bool on = true;
    result = OH_AudioRenderer_SetSilentModeAndMixWithOthers(audioRenderer, on);

    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSilentModeAndMixWithOthers API via legal state.
 * @tc.number: OH_AudioRenderer_GetSilentModeAndMixWithOthers_001
 * @tc.desc  : Test OH_AudioRenderer_GetSilentModeAndMixWithOthers interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if audioRenderer is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetSilentModeAndMixWithOthers_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    bool on = true;
    result = OH_AudioRenderer_GetSilentModeAndMixWithOthers(nullptr, &on);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSilentModeAndMixWithOthers API via legal state.
 * @tc.number: OH_AudioRenderer_GetSilentModeAndMixWithOthers_002
 * @tc.desc  : Test OH_AudioRenderer_GetSilentModeAndMixWithOthers interface. Returns  AUDIOSTREAM_SUCCESS
 *             if renderer is not nullptr .
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetSilentModeAndMixWithOthers_002, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    bool on = true;
    result = OH_AudioRenderer_SetSilentModeAndMixWithOthers(audioRenderer, on);
    result = OH_AudioRenderer_GetSilentModeAndMixWithOthers(audioRenderer, &on);

    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
 * @tc.name  : Test OH_AudioRenderer_GetSilentModeAndMixWithOthers API via legal state.
 * @tc.number: OH_AudioRenderer_GetSilentModeAndMixWithOthers_003
 * @tc.desc  : Test OH_AudioRenderer_GetSilentModeAndMixWithOthers interface. Returns  AUDIOSTREAM_ERROR_INVALID_PARAM
 *             if on is nullptr.
 */
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetSilentModeAndMixWithOthers_003, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);

    result = OH_AudioRenderer_GetSilentModeAndMixWithOthers(audioRenderer, nullptr);

    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    OH_AudioStreamBuilder_Destroy(builder);
}

/**
* @tc.name  : Test OHAudioRenderer API
* @tc.number: OHAudioRenderer_001
* @tc.desc  : Test OHAudioRenderer::SetInterruptCallback()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_001, TestSize.Level0)
{
    auto oHAudioRenderer = std::make_shared<OHAudioRenderer>();
    EXPECT_NE(oHAudioRenderer, nullptr);

    RendererCallback rendererCallbacks;
    void* userData = nullptr;

    oHAudioRenderer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_SEPERATED;
    rendererCallbacks.onInterruptEventCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioInterrupt_ForceType type,
        OH_AudioInterrupt_Hint hint) -> void { return; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    auto audioRendererPrivate = std::make_shared<AudioRendererPrivate>(audioStreamType, appInfo, false);
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::FAST_STREAM, tempParams,
        STREAM_MUSIC, getpid());
    audioRendererPrivate->audioStream_ = audioStream;
    EXPECT_NE(audioRendererPrivate->audioStream_, nullptr);
    oHAudioRenderer->audioRenderer_ = audioRendererPrivate;
    EXPECT_NE(oHAudioRenderer->audioRenderer_, nullptr);

    oHAudioRenderer->SetInterruptCallback(rendererCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioRenderer API
* @tc.number: OHAudioRenderer_002
* @tc.desc  : Test OHAudioRenderer::SetInterruptCallback()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_002, TestSize.Level0)
{
    auto oHAudioRenderer = std::make_shared<OHAudioRenderer>();
    EXPECT_NE(oHAudioRenderer, nullptr);

    RendererCallback rendererCallbacks;
    void* userData = nullptr;

    oHAudioRenderer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_COMBINED;
    rendererCallbacks.onInterruptEventCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioInterrupt_ForceType type,
        OH_AudioInterrupt_Hint hint) -> void { return; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    auto audioRendererPrivate = std::make_shared<AudioRendererPrivate>(audioStreamType, appInfo, false);
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::FAST_STREAM, tempParams,
        STREAM_MUSIC, getpid());
    audioRendererPrivate->audioStream_ = audioStream;
    EXPECT_NE(audioRendererPrivate->audioStream_, nullptr);
    oHAudioRenderer->audioRenderer_ = audioRendererPrivate;
    EXPECT_NE(oHAudioRenderer->audioRenderer_, nullptr);

    oHAudioRenderer->SetInterruptCallback(rendererCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioRenderer API
* @tc.number: OHAudioRenderer_003
* @tc.desc  : Test OHAudioRenderer::SetInterruptCallback()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_003, TestSize.Level0)
{
    auto oHAudioRenderer = std::make_shared<OHAudioRenderer>();
    EXPECT_NE(oHAudioRenderer, nullptr);

    RendererCallback rendererCallbacks;
    void* userData = nullptr;

    oHAudioRenderer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_COMBINED;
    rendererCallbacks.callbacks.OH_AudioRenderer_OnInterruptEvent =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioInterrupt_ForceType type,
        OH_AudioInterrupt_Hint hint) -> int32_t { return 0; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    auto audioRendererPrivate = std::make_shared<AudioRendererPrivate>(audioStreamType, appInfo, false);
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::FAST_STREAM, tempParams,
        STREAM_MUSIC, getpid());
    audioRendererPrivate->audioStream_ = audioStream;
    EXPECT_NE(audioRendererPrivate->audioStream_, nullptr);
    oHAudioRenderer->audioRenderer_ = audioRendererPrivate;
    EXPECT_NE(oHAudioRenderer->audioRenderer_, nullptr);

    oHAudioRenderer->SetInterruptCallback(rendererCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioRenderer API
* @tc.number: OHAudioRenderer_004
* @tc.desc  : Test OHAudioRenderer::SetInterruptCallback()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_004, TestSize.Level0)
{
    auto oHAudioRenderer = std::make_shared<OHAudioRenderer>();
    EXPECT_NE(oHAudioRenderer, nullptr);

    RendererCallback rendererCallbacks;
    void* userData = nullptr;

    oHAudioRenderer->interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_SEPERATED;
    rendererCallbacks.callbacks.OH_AudioRenderer_OnInterruptEvent =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioInterrupt_ForceType type,
        OH_AudioInterrupt_Hint hint) -> int32_t { return 0; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    auto audioRendererPrivate = std::make_shared<AudioRendererPrivate>(audioStreamType, appInfo, false);
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::FAST_STREAM, tempParams,
        STREAM_MUSIC, getpid());
    audioRendererPrivate->audioStream_ = audioStream;
    EXPECT_NE(audioRendererPrivate->audioStream_, nullptr);
    oHAudioRenderer->audioRenderer_ = audioRendererPrivate;
    EXPECT_NE(oHAudioRenderer->audioRenderer_, nullptr);

    oHAudioRenderer->SetInterruptCallback(rendererCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioRenderer API
* @tc.number: OHAudioRenderer_005
* @tc.desc  : Test OHAudioRenderer::SetWriteDataCallback()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_005, TestSize.Level0)
{
    auto oHAudioRenderer = std::make_shared<OHAudioRenderer>();
    EXPECT_NE(oHAudioRenderer, nullptr);

    RendererCallback rendererCallbacks;
    void* userData = nullptr;
    void *metadataUserData = nullptr;
    AudioEncodingType encodingType = AudioEncodingType::ENCODING_AUDIOVIVID;

    rendererCallbacks.writeDataWithMetadataCallback =
        [](OH_AudioRenderer* renderer, void* userData, void* audioData, int32_t audioDataSize,
        void* metadata, int32_t metadataSize) -> int32_t { return 0; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    auto audioRendererPrivate = std::make_shared<AudioRendererPrivate>(audioStreamType, appInfo, false);
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::FAST_STREAM, tempParams,
        STREAM_MUSIC, getpid());
    audioRendererPrivate->audioStream_ = audioStream;
    EXPECT_NE(audioRendererPrivate->audioStream_, nullptr);
    oHAudioRenderer->audioRenderer_ = audioRendererPrivate;
    EXPECT_NE(oHAudioRenderer->audioRenderer_, nullptr);

    oHAudioRenderer->SetWriteDataCallback(rendererCallbacks, userData, metadataUserData, encodingType);
}

/**
* @tc.name  : Test OHAudioRenderer API
* @tc.number: OHAudioRenderer_006
* @tc.desc  : Test OHAudioRenderer::SetWriteDataCallback()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_006, TestSize.Level0)
{
    auto oHAudioRenderer = std::make_shared<OHAudioRenderer>();
    EXPECT_NE(oHAudioRenderer, nullptr);

    RendererCallback rendererCallbacks;
    void* userData = nullptr;
    void *metadataUserData = nullptr;
    AudioEncodingType encodingType = AudioEncodingType::ENCODING_AUDIOVIVID;

    rendererCallbacks.writeDataWithMetadataCallback = nullptr;

    oHAudioRenderer->SetWriteDataCallback(rendererCallbacks, userData, metadataUserData, encodingType);
}

/**
* @tc.name  : Test OHAudioRenderer API
* @tc.number: OHAudioRenderer_007
* @tc.desc  : Test OHAudioRenderer::SetWriteDataCallback()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_007, TestSize.Level0)
{
    auto oHAudioRenderer = std::make_shared<OHAudioRenderer>();
    EXPECT_NE(oHAudioRenderer, nullptr);

    RendererCallback rendererCallbacks;
    void* userData = nullptr;
    void *metadataUserData = nullptr;
    AudioEncodingType encodingType = AudioEncodingType::ENCODING_PCM;

    rendererCallbacks.writeDataWithMetadataCallback = nullptr;

    oHAudioRenderer->SetWriteDataCallback(rendererCallbacks, userData, metadataUserData, encodingType);
}

/**
* @tc.name  : Test OHAudioRenderer API
* @tc.number: OHAudioRenderer_008
* @tc.desc  : Test OHAudioRenderer::SetErrorCallback()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_008, TestSize.Level0)
{
    auto oHAudioRenderer = std::make_shared<OHAudioRenderer>();
    EXPECT_NE(oHAudioRenderer, nullptr);

    RendererCallback rendererCallbacks;
    void* userData = nullptr;

    oHAudioRenderer->errorCallbackType_ = ERROR_CALLBACK_SEPERATED;

    rendererCallbacks.onErrorCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        void { return; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    auto audioRendererPrivate = std::make_shared<AudioRendererPrivate>(audioStreamType, appInfo, false);
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::FAST_STREAM, tempParams,
        STREAM_MUSIC, getpid());
    audioRendererPrivate->audioStream_ = audioStream;
    EXPECT_NE(audioRendererPrivate->audioStream_, nullptr);
    oHAudioRenderer->audioRenderer_ = audioRendererPrivate;
    EXPECT_NE(oHAudioRenderer->audioRenderer_, nullptr);

    oHAudioRenderer->SetErrorCallback(rendererCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioRenderer API
* @tc.number: OHAudioRenderer_009
* @tc.desc  : Test OHAudioRenderer::SetErrorCallback()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_009, TestSize.Level0)
{
    auto oHAudioRenderer = std::make_shared<OHAudioRenderer>();
    EXPECT_NE(oHAudioRenderer, nullptr);

    RendererCallback rendererCallbacks;
    void* userData = nullptr;

    oHAudioRenderer->errorCallbackType_ = ERROR_CALLBACK_COMBINED;

    rendererCallbacks.onErrorCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        void { return; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    auto audioRendererPrivate = std::make_shared<AudioRendererPrivate>(audioStreamType, appInfo, false);
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::FAST_STREAM, tempParams,
        STREAM_MUSIC, getpid());
    audioRendererPrivate->audioStream_ = audioStream;
    EXPECT_NE(audioRendererPrivate->audioStream_, nullptr);
    oHAudioRenderer->audioRenderer_ = audioRendererPrivate;
    EXPECT_NE(oHAudioRenderer->audioRenderer_, nullptr);

    oHAudioRenderer->SetErrorCallback(rendererCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioRenderer API
* @tc.number: OHAudioRenderer_010
* @tc.desc  : Test OHAudioRenderer::SetErrorCallback()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_010, TestSize.Level0)
{
    auto oHAudioRenderer = std::make_shared<OHAudioRenderer>();
    EXPECT_NE(oHAudioRenderer, nullptr);

    RendererCallback rendererCallbacks;
    void* userData = nullptr;

    oHAudioRenderer->errorCallbackType_ = ERROR_CALLBACK_COMBINED;

    rendererCallbacks.callbacks.OH_AudioRenderer_OnError =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        int32_t { return 0; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    auto audioRendererPrivate = std::make_shared<AudioRendererPrivate>(audioStreamType, appInfo, false);
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::FAST_STREAM, tempParams,
        STREAM_MUSIC, getpid());
    audioRendererPrivate->audioStream_ = audioStream;
    EXPECT_NE(audioRendererPrivate->audioStream_, nullptr);
    oHAudioRenderer->audioRenderer_ = audioRendererPrivate;
    EXPECT_NE(oHAudioRenderer->audioRenderer_, nullptr);

    oHAudioRenderer->SetErrorCallback(rendererCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioRenderer API
* @tc.number: OHAudioRenderer_011
* @tc.desc  : Test OHAudioRenderer::SetErrorCallback()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_011, TestSize.Level0)
{
    auto oHAudioRenderer = std::make_shared<OHAudioRenderer>();
    EXPECT_NE(oHAudioRenderer, nullptr);

    RendererCallback rendererCallbacks;
    void* userData = nullptr;

    oHAudioRenderer->errorCallbackType_ = ERROR_CALLBACK_SEPERATED;

    rendererCallbacks.callbacks.OH_AudioRenderer_OnError =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        int32_t { return 0; };

    AudioStreamType audioStreamType = AudioStreamType::STREAM_VOICE_CALL;
    AppInfo appInfo;
    auto audioRendererPrivate = std::make_shared<AudioRendererPrivate>(audioStreamType, appInfo, false);
    AudioStreamParams tempParams = {};
    auto audioStream = IAudioStream::GetRecordStream(IAudioStream::FAST_STREAM, tempParams,
        STREAM_MUSIC, getpid());
    audioRendererPrivate->audioStream_ = audioStream;
    EXPECT_NE(audioRendererPrivate->audioStream_, nullptr);
    oHAudioRenderer->audioRenderer_ = audioRendererPrivate;
    EXPECT_NE(oHAudioRenderer->audioRenderer_, nullptr);

    oHAudioRenderer->SetErrorCallback(rendererCallbacks, userData);
}

/**
* @tc.name  : Test OHAudioRendererModeCallback API
* @tc.number: OHAudioRenderer_012
* @tc.desc  : Test OHAudioRendererModeCallback::OnWriteData()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_012, TestSize.Level0)
{
    OH_AudioRenderer_WriteDataWithMetadataCallback writeDataCallBack =
        [](OH_AudioRenderer* renderer, void* userData, void* audioData, int32_t audioDataSize, void* metadata,
        int32_t metadataSize) -> int32_t { return 0; };

    OHAudioRenderer oHAudioRenderer;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;
    AudioEncodingType encodingType = AudioEncodingType::ENCODING_PCM;

    auto oHAudioRendererModeCallback =
        std::make_shared<OHAudioRendererModeCallback>(writeDataCallBack, oH_AudioRenderer, userData, encodingType);
    EXPECT_NE(oHAudioRendererModeCallback, nullptr);

    oHAudioRendererModeCallback->encodingType_ = ENCODING_AUDIOVIVID;
    size_t length = 0;
    oHAudioRendererModeCallback->OnWriteData(length);
}

/**
* @tc.name  : Test OHAudioRendererModeCallback API
* @tc.number: OHAudioRenderer_013
* @tc.desc  : Test OHAudioRendererModeCallback::OnWriteData()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_013, TestSize.Level0)
{
    OH_AudioRenderer_WriteDataWithMetadataCallback writeDataCallBack = nullptr;

    OHAudioRenderer oHAudioRenderer;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;
    AudioEncodingType encodingType = AudioEncodingType::ENCODING_PCM;

    auto oHAudioRendererModeCallback =
        std::make_shared<OHAudioRendererModeCallback>(writeDataCallBack, oH_AudioRenderer, userData, encodingType);
    EXPECT_NE(oHAudioRendererModeCallback, nullptr);

    oHAudioRendererModeCallback->encodingType_ = ENCODING_AUDIOVIVID;
    size_t length = 0;
    oHAudioRendererModeCallback->OnWriteData(length);
}

/**
* @tc.name  : Test OHAudioRendererModeCallback API
* @tc.number: OHAudioRenderer_014
* @tc.desc  : Test OHAudioRendererModeCallback::OnWriteData()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_014, TestSize.Level0)
{
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData =
        [](OH_AudioRenderer* renderer, void* userData, void* buffer, int32_t length) -> int32_t { return 0; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.writeDataCallbackType_ = WRITE_DATA_CALLBACK_WITHOUT_RESULT;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;
    AudioEncodingType encodingType = AudioEncodingType::ENCODING_PCM;

    auto oHAudioRendererModeCallback =
        std::make_shared<OHAudioRendererModeCallback>(callbacks, oH_AudioRenderer, userData, encodingType);
    EXPECT_NE(oHAudioRendererModeCallback, nullptr);

    oHAudioRendererModeCallback->encodingType_ = ENCODING_AUDIOVIVID;
    size_t length = 0;
    oHAudioRendererModeCallback->OnWriteData(length);
}

/**
* @tc.name  : Test OHAudioRendererModeCallback API
* @tc.number: OHAudioRenderer_015
* @tc.desc  : Test OHAudioRendererModeCallback::OnWriteData()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_015, TestSize.Level0)
{
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnWriteData =
        [](OH_AudioRenderer* renderer, void* userData, void* buffer, int32_t length) -> int32_t { return 0; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.writeDataCallbackType_ = WRITE_DATA_CALLBACK_WITH_RESULT;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;
    AudioEncodingType encodingType = AudioEncodingType::ENCODING_PCM;

    auto oHAudioRendererModeCallback =
        std::make_shared<OHAudioRendererModeCallback>(callbacks, oH_AudioRenderer, userData, encodingType);
    EXPECT_NE(oHAudioRendererModeCallback, nullptr);

    oHAudioRendererModeCallback->encodingType_ = ENCODING_AUDIOVIVID;
    size_t length = 0;
    oHAudioRendererModeCallback->OnWriteData(length);
}

/**
* @tc.name  : Test OHAudioRendererModeCallback API
* @tc.number: OHAudioRenderer_016
* @tc.desc  : Test OHAudioRendererModeCallback::OnWriteData()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_016, TestSize.Level0)
{
    OH_AudioRenderer_OnWriteDataCallback onWriteDataCallback;
    onWriteDataCallback =
        [](OH_AudioRenderer* renderer, void* userData, void* audioData, int32_t audioDataSize) ->
        OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.writeDataCallbackType_ = WRITE_DATA_CALLBACK_WITH_RESULT;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;
    AudioEncodingType encodingType = AudioEncodingType::ENCODING_PCM;

    auto oHAudioRendererModeCallback =
        std::make_shared<OHAudioRendererModeCallback>(onWriteDataCallback, oH_AudioRenderer, userData, encodingType);
    EXPECT_NE(oHAudioRendererModeCallback, nullptr);

    oHAudioRendererModeCallback->encodingType_ = ENCODING_AUDIOVIVID;
    size_t length = 0;
    oHAudioRendererModeCallback->OnWriteData(length);
}

/**
* @tc.name  : Test OHAudioRendererModeCallback API
* @tc.number: OHAudioRenderer_017
* @tc.desc  : Test OHAudioRendererModeCallback::OnWriteData()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_017, TestSize.Level0)
{
    OH_AudioRenderer_OnWriteDataCallback onWriteDataCallback;
    onWriteDataCallback =
        [](OH_AudioRenderer* renderer, void* userData, void* audioData, int32_t audioDataSize) ->
        OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.writeDataCallbackType_ = WRITE_DATA_CALLBACK_WITHOUT_RESULT;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;
    AudioEncodingType encodingType = AudioEncodingType::ENCODING_PCM;

    auto oHAudioRendererModeCallback =
        std::make_shared<OHAudioRendererModeCallback>(onWriteDataCallback, oH_AudioRenderer, userData, encodingType);
    EXPECT_NE(oHAudioRendererModeCallback, nullptr);

    oHAudioRendererModeCallback->encodingType_ = ENCODING_AUDIOVIVID;
    size_t length = 0;
    oHAudioRendererModeCallback->OnWriteData(length);
}

/**
* @tc.name  : Test OHAudioRendererModeCallback API
* @tc.number: OHAudioRenderer_018
* @tc.desc  : Test OHAudioRendererModeCallback::OnInterrupt()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_018, TestSize.Level0)
{
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnInterruptEvent =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioInterrupt_ForceType type,
        OH_AudioInterrupt_Hint hint) -> int32_t { return 0; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_COMBINED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHAudioRendererCallback =
        std::make_shared<OHAudioRendererCallback>(callbacks, oH_AudioRenderer, userData);
    EXPECT_NE(oHAudioRendererCallback, nullptr);

    InterruptEvent interruptEvent;
    oHAudioRendererCallback->OnInterrupt(interruptEvent);
}

/**
* @tc.name  : Test OHAudioRendererModeCallback API
* @tc.number: OHAudioRenderer_019
* @tc.desc  : Test OHAudioRendererModeCallback::OnInterrupt()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_019, TestSize.Level0)
{
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnInterruptEvent =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioInterrupt_ForceType type,
        OH_AudioInterrupt_Hint hint) -> int32_t { return 0; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_SEPERATED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHAudioRendererCallback =
        std::make_shared<OHAudioRendererCallback>(callbacks, oH_AudioRenderer, userData);
    EXPECT_NE(oHAudioRendererCallback, nullptr);

    InterruptEvent interruptEvent;
    oHAudioRendererCallback->OnInterrupt(interruptEvent);
}

/**
* @tc.name  : Test OHAudioRendererModeCallback API
* @tc.number: OHAudioRenderer_020
* @tc.desc  : Test OHAudioRendererModeCallback::OnInterrupt()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_020, TestSize.Level0)
{
    OH_AudioRenderer_OnInterruptCallback onInterruptEventCallback;
    onInterruptEventCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioInterrupt_ForceType type,
        OH_AudioInterrupt_Hint hint) -> void { return; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_SEPERATED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHAudioRendererCallback =
        std::make_shared<OHAudioRendererCallback>(onInterruptEventCallback, oH_AudioRenderer, userData);
    EXPECT_NE(oHAudioRendererCallback, nullptr);

    InterruptEvent interruptEvent;
    oHAudioRendererCallback->OnInterrupt(interruptEvent);
}

/**
* @tc.name  : Test OHAudioRendererModeCallback API
* @tc.number: OHAudioRenderer_021
* @tc.desc  : Test OHAudioRendererModeCallback::OnInterrupt()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_021, TestSize.Level0)
{
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnInterruptEvent = nullptr;
    OH_AudioRenderer_OnInterruptCallback onInterruptEventCallback;
    onInterruptEventCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioInterrupt_ForceType type,
        OH_AudioInterrupt_Hint hint) -> void { return; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.interruptCallbackType_ = INTERRUPT_EVENT_CALLBACK_COMBINED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHAudioRendererCallback =
        std::make_shared<OHAudioRendererCallback>(onInterruptEventCallback, oH_AudioRenderer, userData);
    EXPECT_NE(oHAudioRendererCallback, nullptr);
    oHAudioRendererCallback->callbacks_ = callbacks;

    InterruptEvent interruptEvent;
    oHAudioRendererCallback->OnInterrupt(interruptEvent);
}

/**
* @tc.name  : Test OHServiceDiedCallback API
* @tc.number: OHAudioRenderer_022
* @tc.desc  : Test OHServiceDiedCallback::OnAudioPolicyServiceDied()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_022, TestSize.Level0)
{
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnError =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) -> int32_t { return 0; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.errorCallbackType_ = ERROR_CALLBACK_COMBINED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHServiceDiedCallback =
        std::make_shared<OHServiceDiedCallback>(callbacks, oH_AudioRenderer, userData);
    EXPECT_NE(oHServiceDiedCallback, nullptr);

    oHServiceDiedCallback->OnAudioPolicyServiceDied();
}

/**
* @tc.name  : Test OHServiceDiedCallback API
* @tc.number: OHAudioRenderer_023
* @tc.desc  : Test OHServiceDiedCallback::OnAudioPolicyServiceDied()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_023, TestSize.Level0)
{
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnError =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) -> int32_t { return 0; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHServiceDiedCallback =
        std::make_shared<OHServiceDiedCallback>(callbacks, oH_AudioRenderer, userData);
    EXPECT_NE(oHServiceDiedCallback, nullptr);

    oHServiceDiedCallback->OnAudioPolicyServiceDied();
}

/**
* @tc.name  : Test OHServiceDiedCallback API
* @tc.number: OHAudioRenderer_024
* @tc.desc  : Test OHServiceDiedCallback::OnAudioPolicyServiceDied()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_024, TestSize.Level0)
{
    OH_AudioRenderer_OnErrorCallback errorCallback;
    errorCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        void { return; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHServiceDiedCallback =
        std::make_shared<OHServiceDiedCallback>(errorCallback, oH_AudioRenderer, userData);
    EXPECT_NE(oHServiceDiedCallback, nullptr);

    oHServiceDiedCallback->OnAudioPolicyServiceDied();
}

#ifdef AUDIO_OH_RENDER_UNIT_TEST
/**
* @tc.name  : Test OHServiceDiedCallback API
* @tc.number: OHAudioRenderer_025
* @tc.desc  : Test OHServiceDiedCallback::OnAudioPolicyServiceDied()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_025, TestSize.Level0)
{
    OH_AudioRenderer_OnErrorCallback errorCallback;
    errorCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        void { return; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.errorCallbackType_ = ERROR_CALLBACK_COMBINED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHServiceDiedCallback =
        std::make_shared<OHServiceDiedCallback>(errorCallback, oH_AudioRenderer, userData);
    EXPECT_NE(oHServiceDiedCallback, nullptr);

    oHServiceDiedCallback->OnAudioPolicyServiceDied();
}
#endif

/**
* @tc.name  : Test OHAudioRendererErrorCallback API
* @tc.number: OHAudioRenderer_026
* @tc.desc  : Test OHAudioRendererErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_026, TestSize.Level0)
{
    OH_AudioRenderer_OnErrorCallback errorCallback;
    errorCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        void { return; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHAudioRendererErrorCallback =
        std::make_shared<OHAudioRendererErrorCallback>(errorCallback, oH_AudioRenderer, userData);
    EXPECT_NE(oHAudioRendererErrorCallback, nullptr);
    AudioErrors errorCode = ERROR_ILLEGAL_STATE;

    auto ret = oHAudioRendererErrorCallback->GetErrorResult(errorCode);
    EXPECT_EQ(ret, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test OHAudioRendererErrorCallback API
* @tc.number: OHAudioRenderer_027
* @tc.desc  : Test OHAudioRendererErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_027, TestSize.Level0)
{
    OH_AudioRenderer_OnErrorCallback errorCallback;
    errorCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        void { return; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHAudioRendererErrorCallback =
        std::make_shared<OHAudioRendererErrorCallback>(errorCallback, oH_AudioRenderer, userData);
    EXPECT_NE(oHAudioRendererErrorCallback, nullptr);
    AudioErrors errorCode = ERROR_INVALID_PARAM;

    auto ret = oHAudioRendererErrorCallback->GetErrorResult(errorCode);
    EXPECT_EQ(ret, AUDIOSTREAM_ERROR_INVALID_PARAM);
}

/**
* @tc.name  : Test OHAudioRendererErrorCallback API
* @tc.number: OHAudioRenderer_028
* @tc.desc  : Test OHAudioRendererErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_028, TestSize.Level0)
{
    OH_AudioRenderer_OnErrorCallback errorCallback;
    errorCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        void { return; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHAudioRendererErrorCallback =
        std::make_shared<OHAudioRendererErrorCallback>(errorCallback, oH_AudioRenderer, userData);
    EXPECT_NE(oHAudioRendererErrorCallback, nullptr);
    AudioErrors errorCode = ERROR_SYSTEM;

    auto ret = oHAudioRendererErrorCallback->GetErrorResult(errorCode);
    EXPECT_EQ(ret, AUDIOSTREAM_ERROR_SYSTEM);
}

/**
* @tc.name  : Test OHAudioRendererErrorCallback API
* @tc.number: OHAudioRenderer_029
* @tc.desc  : Test OHAudioRendererErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_029, TestSize.Level0)
{
    OH_AudioRenderer_OnErrorCallback errorCallback;
    errorCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        void { return; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHAudioRendererErrorCallback =
        std::make_shared<OHAudioRendererErrorCallback>(errorCallback, oH_AudioRenderer, userData);
    EXPECT_NE(oHAudioRendererErrorCallback, nullptr);
    AudioErrors errorCode = ERROR_SYSTEM;

    auto ret = oHAudioRendererErrorCallback->GetErrorResult(errorCode);
    EXPECT_EQ(ret, AUDIOSTREAM_ERROR_SYSTEM);
}

/**
* @tc.name  : Test OHAudioRendererErrorCallback API
* @tc.number: OHAudioRenderer_030
* @tc.desc  : Test OHAudioRendererErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_030, TestSize.Level0)
{
    OH_AudioRenderer_OnErrorCallback errorCallback;
    errorCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        void { return; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHAudioRendererErrorCallback =
        std::make_shared<OHAudioRendererErrorCallback>(errorCallback, oH_AudioRenderer, userData);
    EXPECT_NE(oHAudioRendererErrorCallback, nullptr);
    AudioErrors errorCode = ERROR_NO_MEMORY;

    auto ret = oHAudioRendererErrorCallback->GetErrorResult(errorCode);
    EXPECT_EQ(ret, AUDIOSTREAM_ERROR_SYSTEM);
}

/**
* @tc.name  : Test OHAudioRendererErrorCallback API
* @tc.number: OHAudioRenderer_031
* @tc.desc  : Test OHAudioRendererErrorCallback::OnError()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_031, TestSize.Level0)
{
    OH_AudioRenderer_OnErrorCallback errorCallback;
    errorCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        void { return; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHAudioRendererErrorCallback =
        std::make_shared<OHAudioRendererErrorCallback>(errorCallback, oH_AudioRenderer, userData);
    EXPECT_NE(oHAudioRendererErrorCallback, nullptr);
    AudioErrors errorCode = ERROR_NO_MEMORY;

    oHAudioRendererErrorCallback->OnError(errorCode);
}

/**
* @tc.name  : Test OHAudioRendererErrorCallback API
* @tc.number: OHAudioRenderer_032
* @tc.desc  : Test OHAudioRendererErrorCallback::OnError()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_032, TestSize.Level0)
{
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnError = nullptr;
    OH_AudioRenderer_OnErrorCallback errorCallback;
    errorCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        void { return; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.errorCallbackType_ = ERROR_CALLBACK_COMBINED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHAudioRendererErrorCallback =
        std::make_shared<OHAudioRendererErrorCallback>(errorCallback, oH_AudioRenderer, userData);
    EXPECT_NE(oHAudioRendererErrorCallback, nullptr);
    oHAudioRendererErrorCallback->callbacks_ = callbacks;
    AudioErrors errorCode = ERROR_NO_MEMORY;

    oHAudioRendererErrorCallback->OnError(errorCode);
}

/**
* @tc.name  : Test OHAudioRendererErrorCallback API
* @tc.number: OHAudioRenderer_033
* @tc.desc  : Test OHAudioRendererErrorCallback::OnError()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_033, TestSize.Level0)
{
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnError =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        int32_t { return 0; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.errorCallbackType_ = ERROR_CALLBACK_COMBINED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHAudioRendererErrorCallback =
        std::make_shared<OHAudioRendererErrorCallback>(callbacks, oH_AudioRenderer, userData);
    EXPECT_NE(oHAudioRendererErrorCallback, nullptr);
    AudioErrors errorCode = ERROR_NO_MEMORY;

    oHAudioRendererErrorCallback->OnError(errorCode);
}

/**
* @tc.name  : Test OHAudioRendererErrorCallback API
* @tc.number: OHAudioRenderer_034
* @tc.desc  : Test OHAudioRendererErrorCallback::OnError()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_034, TestSize.Level0)
{
    OH_AudioRenderer_Callbacks callbacks;
    callbacks.OH_AudioRenderer_OnError =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        int32_t { return 0; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHAudioRendererErrorCallback =
        std::make_shared<OHAudioRendererErrorCallback>(callbacks, oH_AudioRenderer, userData);
    EXPECT_NE(oHAudioRendererErrorCallback, nullptr);
    AudioErrors errorCode = ERROR_NO_MEMORY;

    oHAudioRendererErrorCallback->OnError(errorCode);
}

/**
* @tc.name  : Test OHAudioRendererErrorCallback API
* @tc.number: OHAudioRenderer_035
* @tc.desc  : Test OHAudioRendererErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_035, TestSize.Level0)
{
    OH_AudioRenderer_OnErrorCallback errorCallback;
    errorCallback =
        [](OH_AudioRenderer* renderer, void* userData, OH_AudioStream_Result error) ->
        void { return; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.errorCallbackType_ = ERROR_CALLBACK_SEPERATED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;

    auto oHAudioRendererErrorCallback =
        std::make_shared<OHAudioRendererErrorCallback>(errorCallback, oH_AudioRenderer, userData);
    EXPECT_NE(oHAudioRendererErrorCallback, nullptr);
    AudioErrors errorCode = ERROR_UNSUPPORTED_FORMAT;

    auto ret = oHAudioRendererErrorCallback->GetErrorResult(errorCode);
    EXPECT_EQ(ret, AUDIOSTREAM_ERROR_UNSUPPORTED_FORMAT);
}
} // namespace AudioStandard
} // namespace OHOS
