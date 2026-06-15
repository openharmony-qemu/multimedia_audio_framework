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

#include "file_audio_render_sink_aiut.h"
#include "aiut_log.h"
#include "sink/file_audio_render_sink.h"
#include "audio_errors.h"
#include "audio_info.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t TEST_SAMPLE_RATE_48000 = 48000;
constexpr uint32_t TEST_CHANNEL_STEREO = 2;
constexpr uint32_t TEST_CHANNEL_LAYOUT_STEREO = 3;
constexpr float TEST_VOLUME_FULL = 1.0f;
constexpr size_t TEST_BUFFER_SIZE = 4096;
constexpr int32_t TEST_APP_UID_1 = 1001;
constexpr int32_t TEST_APP_UID_2 = 1002;
} // namespace

// ==================== FileAudioRenderSinkAiut ====================

void FileAudioRenderSinkAiut::SetUpTestCase(void) {}
void FileAudioRenderSinkAiut::TearDownTestCase(void) {}

void FileAudioRenderSinkAiut::SetUp(void) {}
void FileAudioRenderSinkAiut::TearDown(void) {}

IAudioSinkAttr FileAudioRenderSinkAiut::CreateDefaultAttr()
{
    IAudioSinkAttr attr = {};
    attr.filePath = "/data/log/hilog/test.txt";
    attr.sampleRate = TEST_SAMPLE_RATE_48000;
    attr.channel = TEST_CHANNEL_STEREO;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = TEST_CHANNEL_LAYOUT_STEREO;
    attr.volume = TEST_VOLUME_FULL;
    return attr;
}

IAudioSinkAttr FileAudioRenderSinkAiut::CreateInvalidPathAttr()
{
    IAudioSinkAttr attr = {};
    attr.filePath = "/invalid_path/test.txt";
    attr.sampleRate = TEST_SAMPLE_RATE_48000;
    attr.channel = TEST_CHANNEL_STEREO;
    attr.format = SAMPLE_S16LE;
    attr.channelLayout = TEST_CHANNEL_LAYOUT_STEREO;
    attr.volume = TEST_VOLUME_FULL;
    return attr;
}

// ==================== Construction / Destruction Tests ====================

/**
 * @tc.name  : Test FileAudioRenderSink default construction
 * @tc.number: FileSink_Constructor_001
 * @tc.desc  : Test FileAudioRenderSink default constructor creates uninitialized object.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_Constructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "newly constructed sink should not be inited");
}

/**
 * @tc.name  : Test FileAudioRenderSink destruction with uninitialized sink
 * @tc.number: FileSink_Destructor_001
 * @tc.desc  : Test FileAudioRenderSink destructor on uninitialized sink does not crash.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_Destructor_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink.reset();
}

// ==================== Init / DeInit Tests ====================

/**
 * @tc.name  : Test Init with valid file path
 * @tc.number: FileSink_Init_Success_001
 * @tc.desc  : Test Init with valid file path returns SUCCESS and sets IsInited to true.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_Init_Success_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    IAudioSinkAttr attr = CreateDefaultAttr();
    int32_t ret = sink->Init(attr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Init should return SUCCESS, ret=%d", ret);
    AIUT_EXPECT_TRUE_INFO_LOG(sink->IsInited(), "sink should be inited after successful Init");
    sink->DeInit();
}

/**
 * @tc.name  : Test Init with invalid file path
 * @tc.number: FileSink_Init_InvalidPath_001
 * @tc.desc  : Test Init with invalid file path still succeeds (file sink does not fail on init).
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_Init_InvalidPath_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    IAudioSinkAttr attr = CreateInvalidPathAttr();
    int32_t ret = sink->Init(attr);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Init should return SUCCESS, ret=%d", ret);
    sink->DeInit();
}

/**
 * @tc.name  : Test DeInit after Init sets IsInited to false
 * @tc.number: FileSink_DeInit_001
 * @tc.desc  : Test DeInit after successful Init sets IsInited to false.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_DeInit_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    IAudioSinkAttr attr = CreateDefaultAttr();
    sink->Init(attr);
    AIUT_ASSERT_TRUE_INFO_LOG(sink->IsInited(), "sink should be inited before DeInit");
    sink->DeInit();
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited after DeInit");
}

/**
 * @tc.name  : Test DeInit on uninitialized sink does not crash
 * @tc.number: FileSink_DeInit_NotInited_001
 * @tc.desc  : Test DeInit on uninitialized sink does not crash.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_DeInit_NotInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->DeInit();
    AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited after DeInit");
}

/**
 * @tc.name  : Test Init and DeInit multiple times
 * @tc.number: FileSink_InitDeInit_Multiple_001
 * @tc.desc  : Test Init and DeInit can be called multiple times in sequence.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_InitDeInit_Multiple_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    IAudioSinkAttr attr = CreateDefaultAttr();
    for (int32_t i = 0; i < 3; i++) {
        int32_t ret = sink->Init(attr);
        AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Init #%d should return SUCCESS, ret=%d", i, ret);
        AIUT_EXPECT_TRUE_INFO_LOG(sink->IsInited(), "sink should be inited after Init #%d", i);
        sink->DeInit();
        AIUT_EXPECT_FALSE_INFO_LOG(sink->IsInited(), "sink should not be inited after DeInit #%d", i);
    }
}

// ==================== Start / Stop Tests ====================

/**
 * @tc.name  : Test Start and Stop on initialized sink
 * @tc.number: FileSink_StartStop_001
 * @tc.desc  : Test Start and Stop on initialized file sink returns SUCCESS.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_StartStop_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    IAudioSinkAttr attr = CreateDefaultAttr();
    sink->Init(attr);
    AIUT_ASSERT_TRUE_INFO_LOG(sink->IsInited(), "sink should be inited");
    int32_t ret = sink->Start();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Start should return SUCCESS, ret=%d", ret);
    ret = sink->Stop();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Stop should return SUCCESS, ret=%d", ret);
    sink->DeInit();
}

/**
 * @tc.name  : Test Start with invalid path
 * @tc.number: FileSink_Start_InvalidPath_001
 * @tc.desc  : Test Start after Init with invalid file path returns ERR_INVALID_HANDLE.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_Start_InvalidPath_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    IAudioSinkAttr attr = CreateInvalidPathAttr();
    sink->Init(attr);
    int32_t ret = sink->Start();
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_INVALID_HANDLE,
        "Start with invalid path should return ERR_INVALID_HANDLE, ret=%d", ret);
    ret = sink->Stop();
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "Stop should return SUCCESS, ret=%d", ret);
    sink->DeInit();
}

// ==================== Resume / Pause / Flush / Reset Tests ====================

/**
 * @tc.name  : Test Resume/Pause/Flush/Reset on initialized sink
 * @tc.number: FileSink_Lifecycle_001
 * @tc.desc  : Test lifecycle methods on initialized file sink return SUCCESS.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_Lifecycle_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    IAudioSinkAttr attr = CreateDefaultAttr();
    sink->Init(attr);
    AIUT_ASSERT_TRUE_INFO_LOG(sink->IsInited(), "sink should be inited");
    AIUT_EXPECT_EQ_INFO_LOG(sink->Resume(), SUCCESS, "Resume should return SUCCESS");
    AIUT_EXPECT_EQ_INFO_LOG(sink->Pause(), SUCCESS, "Pause should return SUCCESS");
    AIUT_EXPECT_EQ_INFO_LOG(sink->Flush(), SUCCESS, "Flush should return SUCCESS");
    AIUT_EXPECT_EQ_INFO_LOG(sink->Reset(), SUCCESS, "Reset should return SUCCESS");
    sink->DeInit();
}

// ==================== RenderFrame Tests ====================

/**
 * @tc.name  : Test RenderFrame without Init returns ERROR_ILLEGAL_STATE
 * @tc.number: FileSink_RenderFrame_NotInited_001
 * @tc.desc  : Test RenderFrame on uninitialized sink returns ERROR_ILLEGAL_STATE.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_RenderFrame_NotInited_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    char data[TEST_BUFFER_SIZE] = {0};
    uint64_t writeLen = 0;
    int32_t ret = sink->RenderFrame(*data, TEST_BUFFER_SIZE, writeLen);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERROR_ILLEGAL_STATE,
        "RenderFrame without Init should return ERROR_ILLEGAL_STATE, ret=%d", ret);
}

/**
 * @tc.name  : Test RenderFrame after Init and Start
 * @tc.number: FileSink_RenderFrame_AfterInit_001
 * @tc.desc  : Test RenderFrame on initialized and started file sink returns SUCCESS.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_RenderFrame_AfterInit_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    IAudioSinkAttr attr = CreateDefaultAttr();
    sink->Init(attr);
    AIUT_ASSERT_TRUE_INFO_LOG(sink->IsInited(), "sink should be inited");
    sink->Start();
    char data[TEST_BUFFER_SIZE] = {0};
    uint64_t writeLen = 0;
    int32_t ret = sink->RenderFrame(*data, TEST_BUFFER_SIZE, writeLen);
    AIUT_EXPECT_EQ_INFO_LOG(ret, SUCCESS, "RenderFrame should return SUCCESS, ret=%d", ret);
    sink->Stop();
    sink->DeInit();
}

// ==================== Volume Tests ====================

/**
 * @tc.name  : Test SetVolume returns ERR_NOT_SUPPORTED
 * @tc.number: FileSink_SetVolume_001
 * @tc.desc  : Test SetVolume returns ERR_NOT_SUPPORTED on file sink.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_SetVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t ret = sink->SetVolume(TEST_VOLUME_FULL, TEST_VOLUME_FULL);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "SetVolume should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test GetVolume returns ERR_NOT_SUPPORTED
 * @tc.number: FileSink_GetVolume_001
 * @tc.desc  : Test GetVolume returns ERR_NOT_SUPPORTED on file sink.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_GetVolume_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    float left = 0.0f;
    float right = 0.0f;
    int32_t ret = sink->GetVolume(left, right);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "GetVolume should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

// ==================== Unsupported Interface Tests ====================

/**
 * @tc.name  : Test GetLatency returns ERR_NOT_SUPPORTED
 * @tc.number: FileSink_GetLatency_001
 * @tc.desc  : Test GetLatency returns ERR_NOT_SUPPORTED on file sink.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_GetLatency_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    uint32_t latency = 0;
    int32_t ret = sink->GetLatency(latency);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "GetLatency should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test GetPresentationPosition returns ERR_NOT_SUPPORTED
 * @tc.number: FileSink_GetPresentationPosition_001
 * @tc.desc  : Test GetPresentationPosition returns ERR_NOT_SUPPORTED on file sink.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_GetPresentationPosition_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    int32_t ret = sink->GetPresentationPosition(frames, timeSec, timeNanoSec);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "GetPresentationPosition should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test GetMaxAmplitude returns 0.0
 * @tc.number: FileSink_GetMaxAmplitude_001
 * @tc.desc  : Test GetMaxAmplitude returns 0.0 on file sink.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_GetMaxAmplitude_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    float amplitude = sink->GetMaxAmplitude();
    AIUT_EXPECT_EQ_INFO_LOG(amplitude, 0.0f,
        "GetMaxAmplitude should return 0.0, amplitude=%f", amplitude);
}

// ==================== AudioMono / AudioBalance Tests ====================

/**
 * @tc.name  : Test SetAudioMonoState does not crash
 * @tc.number: FileSink_SetAudioMonoState_001
 * @tc.desc  : Test SetAudioMonoState with true and false does not crash.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_SetAudioMonoState_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->SetAudioMonoState(true);
    sink->SetAudioMonoState(false);
}

/**
 * @tc.name  : Test SetAudioBalanceValue does not crash
 * @tc.number: FileSink_SetAudioBalanceValue_001
 * @tc.desc  : Test SetAudioBalanceValue with various values does not crash.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_SetAudioBalanceValue_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    sink->SetAudioBalanceValue(0.0f);
    sink->SetAudioBalanceValue(-1.0f);
    sink->SetAudioBalanceValue(1.0f);
}

// ==================== UpdateAppsUid Tests ====================

/**
 * @tc.name  : Test UpdateAppsUid with array input returns ERR_NOT_SUPPORTED
 * @tc.number: FileSink_UpdateAppsUid_Array_001
 * @tc.desc  : Test UpdateAppsUid with array input returns ERR_NOT_SUPPORTED on file sink.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_UpdateAppsUid_Array_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int32_t appsUid[MAX_MIX_CHANNELS] = {TEST_APP_UID_1, TEST_APP_UID_2};
    int32_t ret = sink->UpdateAppsUid(appsUid, MAX_MIX_CHANNELS);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "UpdateAppsUid array should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

/**
 * @tc.name  : Test UpdateAppsUid with vector input returns ERR_NOT_SUPPORTED
 * @tc.number: FileSink_UpdateAppsUid_Vector_001
 * @tc.desc  : Test UpdateAppsUid with vector input returns ERR_NOT_SUPPORTED on file sink.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_UpdateAppsUid_Vector_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    std::vector<int32_t> appsUid = {TEST_APP_UID_1, TEST_APP_UID_2};
    int32_t ret = sink->UpdateAppsUid(appsUid);
    AIUT_EXPECT_EQ_INFO_LOG(ret, ERR_NOT_SUPPORTED,
        "UpdateAppsUid vector should return ERR_NOT_SUPPORTED, ret=%d", ret);
}

// ==================== GetVolumeDataCount Tests ====================

/**
 * @tc.name  : Test GetVolumeDataCount returns 0
 * @tc.number: FileSink_GetVolumeDataCount_001
 * @tc.desc  : Test GetVolumeDataCount returns 0 on file sink.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_GetVolumeDataCount_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    int64_t volumeData = -1;
    int32_t ret = sink->GetVolumeDataCount(volumeData);
    AIUT_EXPECT_EQ_INFO_LOG(ret, 0, "GetVolumeDataCount should return 0, ret=%d", ret);
}

// ==================== DumpInfo Tests ====================

/**
 * @tc.name  : Test DumpInfo on initialized sink
 * @tc.number: FileSink_DumpInfo_001
 * @tc.desc  : Test DumpInfo on initialized sink produces non-empty output.
 */
HWTEST_F(FileAudioRenderSinkAiut, FileSink_DumpInfo_001, TestSize.Level0)
{
    AIUT_INFO_LOG("entry");
    auto sink = std::make_shared<FileAudioRenderSink>();
    AIUT_ASSERT_NE_INFO_LOG(sink, nullptr, "sink is null");
    IAudioSinkAttr attr = CreateDefaultAttr();
    sink->Init(attr);
    std::string dumpString;
    sink->DumpInfo(dumpString);
    AIUT_EXPECT_FALSE_INFO_LOG(dumpString.empty(), "dump string should not be empty");
    sink->DeInit();
}

} // namespace AudioStandard
} // namespace OHOS
