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
#include "gmock/gmock.h"
#include "oh_audio_render_unit_test.h"

using namespace testing::ext;
using namespace testing;
using namespace std::chrono;
using namespace OHOS::AudioStandard::OHAudioRenderUT;

namespace OHOS {
namespace AudioStandard {
namespace OHAudioRenderUT {
int32_t AudioRendererOnWriteData(OH_AudioRenderer* capturer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    return 0;
}

class MockAudioRendererCallback {
public:
    virtual ~MockAudioRendererCallback() = default;
    virtual int32_t OnWriteData(
        OH_AudioRenderer* renderer, void* userData, void* audioData, int32_t audioDataSize) = 0;
};

class MockAudioRendererCallbackImpl : public MockAudioRendererCallback {
public:
    MOCK_METHOD(int32_t, OnWriteData,
        (OH_AudioRenderer* renderer, void* userData, void* audioData, int32_t audioDataSize),
        (override));
};

int32_t AdvancedWriteDataProxy(
    OH_AudioRenderer* renderer, void* userData, void* audioData, int32_t audioDataSize) {
    auto* mock = static_cast<MockAudioRendererCallback*>(userData);
    return mock->OnWriteData(renderer, userData, audioData, audioDataSize);
}

void AudioRendererOnMarkReachedCb(OH_AudioRenderer* renderer, uint32_t samplePos, void* userData)
{
    g_flag = samplePos;
    printf("AudioRendererOnMarkReachedCb samplePos: %d \n", samplePos);
}

int32_t AudioRendererOnWriteDataMock(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    return 0;
}

OH_AudioData_Callback_Result OnWriteDataCallbackWithValidData(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    return AUDIO_DATA_CALLBACK_RESULT_VALID;
}

OH_AudioData_Callback_Result OnWriteDataCallbackWithInvalidData(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    return AUDIO_DATA_CALLBACK_RESULT_INVALID;
}

int32_t OnWriteDataCbMock(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLer)
{
    UserData *u = static_cast<UserData*>(userData);
    u->writeDataCallbackType = UserData::WRITE_DATA_CALLBACK;
    return 0;
}

OH_AudioData_Callback_Result OnWriteDataCbWithValidDataMock(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    UserData *u = static_cast<UserData*>(userData);
    u->writeDataCallbackType = UserData::WRITE_DATA_CALLBACK_WITH_RESULT;
    return AUDIO_DATA_CALLBACK_RESULT_VALID;
}

OH_AudioData_Callback_Result OnWriteDataCbWithInvalidDataMock(OH_AudioRenderer* renderer,
    void* userData,
    void* buffer,
    int32_t bufferLen)
{
    UserData *u = static_cast<UserData*>(userData);
    u->writeDataCallbackType = UserData::WRITE_DATA_CALLBACK_WITH_RESULT;
    return AUDIO_DATA_CALLBACK_RESULT_INVALID;
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
} // namespace OHAudioRenderUT

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
    sleep(1);
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
    sleep(1);
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
    sleep(1);
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
    sleep(1);
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
    sleep(1);
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
    sleep(1);
    EXPECT_TRUE(userData.writeDataCallbackType == UserData::WRITE_DATA_CALLBACK);

    CleanupAudioResources(builder, audioRenderer);
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

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallbackAdvanced API via legal state.
 * @tc.number: OH_Audio_Render_WriteDataCallbackAdvanced_001
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallbackAdvanced interface.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_WriteDataCallbackAdvanced_001, TestSize.Level0)
{
    std::atomic<bool> flagEndTest = false;
    MockAudioRendererCallbackImpl mockCallback;
    EXPECT_CALL(mockCallback, OnWriteData(
        _,          // renderer
        NotNull(),  // userData
        NotNull(),  // audioData
        Gt(0)       // audioDataSize > 0
    ))
    .Times(AtLeast(4))
    .WillOnce(Invoke([](OH_AudioRenderer*, void*, void* audioData, int32_t audioDataSize) {
        memset_s(audioData, audioDataSize, 0, audioDataSize);
        return 0;
    }))
    .WillOnce(Invoke([](OH_AudioRenderer*, void*, void* audioData, int32_t audioDataSize) {
        memset_s(audioData, audioDataSize, 0, audioDataSize);
        return CHANNEL_COUNT * FORMAT_SIZE; // a sampling point
    }))
    .WillOnce(Invoke([](OH_AudioRenderer*, void*, void* audioData, int32_t audioDataSize) {
        memset_s(audioData, audioDataSize, 0, audioDataSize);
        // Non-integer sampling points. Note that this is not a correct usage, it is only used to test robustness.
        return CHANNEL_COUNT * FORMAT_SIZE + 1;
    }))
    .WillOnce(Invoke([&flagEndTest](OH_AudioRenderer*, void*, void* audioData, int32_t audioDataSize) {
        memset_s(audioData, audioDataSize, 0, audioDataSize);
        flagEndTest = true;
        flagEndTest.notify_all();
        return audioDataSize;
    }))
    .WillRepeatedly(Return(0));

    OH_AudioStreamBuilder* builder = InitRenderBuilder();
    OH_AudioStreamBuilder_SetRendererWriteDataCallbackAdvanced(builder, AdvancedWriteDataProxy, &mockCallback);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    flagEndTest.wait(false);

    CleanupAudioResources(builder, audioRenderer);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallbackAdvanced API via legal state.
 * @tc.number: OH_Audio_Render_WriteDataCallbackAdvanced_002
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallbackAdvanced interface.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_WriteDataCallbackAdvanced_002, TestSize.Level0)
{
    std::atomic<bool> flagEndTest = false;
    MockAudioRendererCallbackImpl mockCallback;
    EXPECT_CALL(mockCallback, OnWriteData(
        _,          // renderer
        NotNull(),  // userData
        NotNull(),  // audioData
        Gt(0)       // audioDataSize > 0
    ))
    .Times(AtLeast(4))
    .WillOnce(Invoke([](OH_AudioRenderer*, void*, void* audioData, int32_t audioDataSize) {
        memset_s(audioData, audioDataSize, 0, audioDataSize);
        return 0;
    }))
    .WillOnce(Invoke([](OH_AudioRenderer*, void*, void* audioData, int32_t audioDataSize) {
        memset_s(audioData, audioDataSize, 0, audioDataSize);
        return CHANNEL_COUNT * FORMAT_SIZE; // a sampling point
    }))
    .WillOnce(Invoke([](OH_AudioRenderer*, void*, void* audioData, int32_t audioDataSize) {
        memset_s(audioData, audioDataSize, 0, audioDataSize);
        // Non-integer sampling points. Note that this is not a correct usage, it is only used to test robustness.
        return CHANNEL_COUNT * FORMAT_SIZE + 1;
    }))
    .WillOnce(Invoke([&flagEndTest](OH_AudioRenderer*, void*, void* audioData, int32_t audioDataSize) {
        memset_s(audioData, audioDataSize, 0, audioDataSize);
        flagEndTest = true;
        flagEndTest.notify_all();
        return audioDataSize;
    }))
    .WillRepeatedly(Return(0));

    OH_AudioStreamBuilder* builder = InitRenderBuilder();
    OH_AudioStreamBuilder_SetLatencyMode(builder, AUDIOSTREAM_LATENCY_MODE_FAST);
    OH_AudioStreamBuilder_SetRendererWriteDataCallbackAdvanced(builder, AdvancedWriteDataProxy, &mockCallback);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    flagEndTest.wait(false);

    CleanupAudioResources(builder, audioRenderer);
}

/**
 * @tc.name  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallbackAdvanced API via legal state.
 * @tc.number: OH_Audio_Render_WriteDataCallbackAdvanced_003
 * @tc.desc  : Test OH_AudioStreamBuilder_SetRendererWriteDataCallbackAdvanced interface.
 *             Returns true if result is successful.
 */
HWTEST(OHAudioRenderUnitTest, OH_Audio_Render_WriteDataCallbackAdvanced_003, TestSize.Level0)
{
    std::atomic<bool> flagEndTest = false;
    MockAudioRendererCallbackImpl mockCallback;
    EXPECT_CALL(mockCallback, OnWriteData(
        _,          // renderer
        NotNull(),  // userData
        NotNull(),  // audioData
        Gt(0)       // audioDataSize > 0
    ))
    .Times(AtLeast(4))
    .WillOnce(Invoke([](OH_AudioRenderer*, void*, void *audioData, int32_t audioDataSize) {
        memset_s(audioData, audioDataSize, 0, audioDataSize);
        return 0;
    }))
    .WillOnce(Invoke([](OH_AudioRenderer*, void*, void *audioData, int32_t audioDataSize) {
        memset_s(audioData, audioDataSize, 0, audioDataSize);
        return CHANNEL_COUNT * FORMAT_SIZE; // a sampling point
    }))
    .WillOnce(Invoke([](OH_AudioRenderer*, void*, void *audioData, int32_t audioDataSize) {
        memset_s(audioData, audioDataSize, 0, audioDataSize);
        // Non-integer sampling points. Note that this is not a correct usage, it is only used to test robustness.
        return CHANNEL_COUNT * FORMAT_SIZE + 1;
    }))
    .WillOnce(Invoke([&flagEndTest](OH_AudioRenderer*, void*, void *audioData, int32_t audioDataSize) {
        memset_s(audioData, audioDataSize, 0, audioDataSize);
        flagEndTest = true;
        flagEndTest.notify_all();
        return audioDataSize;
    }))
    .WillRepeatedly(Return(0));

    OH_AudioStreamBuilder* builder = InitRenderBuilder();
    OH_AudioStreamBuilder_SetRendererInfo(builder, AUDIOSTREAM_USAGE_VOICE_COMMUNICATION);
    OH_AudioStreamBuilder_SetRendererWriteDataCallbackAdvanced(builder, AdvancedWriteDataProxy, &mockCallback);

    OH_AudioRenderer* audioRenderer;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    result = OH_AudioRenderer_Start(audioRenderer);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);

    flagEndTest.wait(false);

    CleanupAudioResources(builder, audioRenderer);
}

/**
* @tc.name  : Test OH_AudioRenderer_GetFastStatus API
* @tc.number: OH_AudioRenderer_GetFastStatus_001
* @tc.desc  : Test OH_AudioRenderer_GetFastStatus
*/
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetFastStatus_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer *renderer;
    OH_AudioStream_FastStatus status = AUDIOSTREAM_FASTSTATUS_FAST;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    result = OH_AudioRenderer_GetFastStatus(nullptr, &status);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    result = OH_AudioRenderer_GetFastStatus(renderer, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    result = OH_AudioRenderer_GetFastStatus(renderer, &status);
    EXPECT_EQ(result, AUDIOSTREAM_SUCCESS);
}

/**
* @tc.name  : Test SetwriteDataCallback API
* @tc.number: SetwriteDataCallback_001
* @tc.desc  : Test SetwriteDataCallback
*/
HWTEST(OHAudioRenderUnitTest, SetwriteDataCallback_001, TestSize.Level0)
{
    auto oHAudioRenderer = std::make_shared<OHAudioRenderer>();
    EXPECT_NE(oHAudioRenderer, nullptr);

    RendererCallback rendererCallbacks;
    void* userData = nullptr;
    void *metadataUserData = nullptr;
    AudioEncodingType encodingType = AudioEncodingType::ENCODING_PCM;

    oHAudioRenderer->writeDataCallbackType_ = WRITE_DATA_CALLBACK_ADVANCED;
    rendererCallbacks.writeDataWithMetadataCallback =
        [](OH_AudioRenderer* renderer, void* userData, void* audioData, int32_t audioDataSize,
        void* metadata, int32_t metadataSize) -> int32_t { return 0; };

    oHAudioRenderer->SetWriteDataCallback(rendererCallbacks, userData, metadataUserData, encodingType);
}

/**
* @tc.name  : Test OH_AudioRenderer_GetAudioTimestampInfo
* @tc.number: OH_AudioRenderer_GetAudioTimestampInfo_001
* @tc.desc  : Test OH_AudioRenderer_GetAudioTimestampInfo
*/
HWTEST(OHAudioRenderUnitTest, OH_AudioRenderer_GetAudioTimestampInfo_001, TestSize.Level0)
{
    OH_AudioStreamBuilder* builder = OHAudioRenderUnitTest::CreateRenderBuilder();
    OH_AudioRenderer *renderer;
    int64_t framePosition = 0;
    int64_t timestamp = 0;
    OH_AudioStream_Result result = OH_AudioStreamBuilder_GenerateRenderer(builder, &renderer);
    result = OH_AudioRenderer_GetAudioTimestampInfo(nullptr, &framePosition, &timestamp);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    result = OH_AudioRenderer_GetAudioTimestampInfo(renderer, nullptr, &timestamp);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    result = OH_AudioRenderer_GetAudioTimestampInfo(renderer, &framePosition, nullptr);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_INVALID_PARAM);
    result = OH_AudioRenderer_GetAudioTimestampInfo(renderer, &framePosition, &timestamp);
    EXPECT_EQ(result, AUDIOSTREAM_ERROR_ILLEGAL_STATE);
}

/**
* @tc.name  : Test OHAudioRendererErrorCallback API
* @tc.number: OHAudioRenderer_036
* @tc.desc  : Test OHAudioRendererErrorCallback::GetErrorResult()
*/
HWTEST(OHAudioRenderUnitTest, OHAudioRenderer_036, TestSize.Level0)
{
    OH_AudioRenderer_OnWriteDataCallback onWriteDataCallback;
    onWriteDataCallback =
        [](OH_AudioRenderer* renderer, void* userData, void* audioData, int32_t audioDataSize) ->
        OH_AudioData_Callback_Result { return AUDIO_DATA_CALLBACK_RESULT_VALID; };

    OHAudioRenderer oHAudioRenderer;
    oHAudioRenderer.writeDataCallbackType_ = WRITE_DATA_CALLBACK_ADVANCED;
    OH_AudioRenderer* oH_AudioRenderer = (OH_AudioRenderer*)&oHAudioRenderer;
    EXPECT_NE((OHAudioRenderer*)oH_AudioRenderer, nullptr);
    void* userData = nullptr;
    AudioEncodingType encodingType = AudioEncodingType::ENCODING_PCM;

    auto oHAudioRendererModeCallback =
        std::make_shared<OHAudioRendererModeCallback>(onWriteDataCallback, oH_AudioRenderer, userData, encodingType);
    EXPECT_NE(oHAudioRendererModeCallback, nullptr);

    oHAudioRendererModeCallback->encodingType_ = ENCODING_AUDIOVIVID;
    oHAudioRendererModeCallback->writeDataWithMetadataCallback_ = {};
    oHAudioRendererModeCallback->onWriteDataAdvancedCallback_ = {};
    size_t length = 0;
    oHAudioRendererModeCallback->OnWriteData(length);
}
} // namespace AudioStandard
} // namespace OHOS
