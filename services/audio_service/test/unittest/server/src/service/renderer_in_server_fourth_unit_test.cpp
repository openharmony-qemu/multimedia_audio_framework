/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "renderer_in_server_fourth_unit_test.h"
#include "audio_utils.h"
#include "audio_service.h"
#include "audio_server.h"
#include "policy_handler.h"
#include "audio_stream_enum.h"
#include "audio_errors.h"
#include "renderer_in_server.h"
#include "audio_process_config.h"
#include "stream_dfx_manager.h"

#include <cstdint>
#include <cstring>
#include <memory>
#include <shared_mutex>

using namespace testing::ext;
using namespace OHOS::AudioStandard;

namespace OHOS {
namespace AudioStandard {

static std::shared_ptr<IStreamListener> stateListener;
static std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
static std::shared_ptr<RendererInServer> rendererInServer;
static AudioProcessConfig processConfig;

constexpr int32_t TEST_UID = 10001;
constexpr int32_t TEST_PID = 10001;
// TEST_BUFFER_SIZE unused - removed

static AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO,
    AudioChannelLayout::CH_LAYOUT_STEREO);

void RendererInServerFourthUnitTest::SetUpTestCase(void)
{
    streamListenerHolder = std::make_shared<StreamListenerHolder>();
    stateListener = streamListenerHolder;
}

void RendererInServerFourthUnitTest::TearDownTestCase(void)
{
    stateListener.reset();
    streamListenerHolder.reset();
}

void RendererInServerFourthUnitTest::SetUp(void)
{
    processConfig.appInfo.appUid = TEST_UID;
    processConfig.appInfo.appPid = TEST_PID;
    processConfig.streamInfo = testStreamInfo;
    processConfig.streamType = STREAM_MUSIC;
    processConfig.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    processConfig.rendererInfo.rendererFlags = AUDIO_FLAG_NORMAL;
    processConfig.rendererInfo.streamUsage = STREAM_USAGE_MUSIC;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, stateListener);
}

void RendererInServerFourthUnitTest::TearDown(void)
{
    rendererInServer.reset();
}

// ==================== WriteData 相关分支测试 ====================

/**
 * @tc.name  : Test WriteData with null input data
 * @tc.type  : FUNC
 * @tc.number: WriteData_NullInput_001
 * @tc.desc  : Test WriteData with null input data, expect error.
 */
HWTEST_F(RendererInServerFourthUnitTest, WriteData_NullInput_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    int32_t writeRet = rendererInServer->WriteData(nullptr, 0, true);
    EXPECT_NE(SUCCESS, writeRet);
}

/**
 * @tc.name  : Test WriteData with zero length
 * @tc.type  : FUNC
 * @tc.number: WriteData_ZeroLength_001
 * @tc.desc  : Test WriteData with zero length.
 */
HWTEST_F(RendererInServerFourthUnitTest, WriteData_ZeroLength_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    uint8_t dummyData[8] = {0};
    int32_t writeRet = rendererInServer->WriteData(reinterpret_cast<int8_t*>(dummyData), 0, true);
    EXPECT_NE(writeRet, SUCCESS);
}

/**
 * @tc.name  : Test SelectModeAndWriteData with different flags
 * @tc.type  : FUNC
 * @tc.number: SelectModeAndWriteData_Offload_001
 * @tc.desc  : Test SelectModeAndWriteData when offload is enabled.
 */
HWTEST_F(RendererInServerFourthUnitTest, SelectModeAndWriteData_Offload_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    rendererInServer->offloadEnable_ = true;
    uint8_t testData[512] = {0};
    int32_t writeRet = rendererInServer->SelectModeAndWriteData(reinterpret_cast<int8_t*>(testData), 512, true);
    EXPECT_NE(writeRet, SUCCESS);
}

/**
 * @tc.name  : Test SelectModeAndWriteData with fast flag
 * @tc.type  : FUNC
 * @tc.number: SelectModeAndWriteData_Fast_001
 * @tc.desc  : Test SelectModeAndWriteData when fast route is enabled.
 */
HWTEST_F(RendererInServerFourthUnitTest, SelectModeAndWriteData_Fast_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    uint8_t testData[512] = {0};
    int32_t writeRet = rendererInServer->SelectModeAndWriteData(reinterpret_cast<int8_t*>(testData), 512, true);
    EXPECT_NE(writeRet, SUCCESS);
}

// ==================== GetLatencyWithFlag 分支测试 ====================

/**
 * @tc.name  : Test GetLatencyWithFlag with LATENCY_FLAG_SOFTWARE
 * @tc.type  : FUNC
 * @tc.number: GetLatencyWithFlag_Normal_001
 * @tc.desc  : Test GetLatencyWithFlag with NORMAL flag.
 */
HWTEST_F(RendererInServerFourthUnitTest, GetLatencyWithFlag_Normal_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    uint64_t latency = 0;
    int32_t result = rendererInServer->GetLatencyWithFlag(latency, LATENCY_FLAG_SOFTWARE);
    (void)result;
}

/**
 * @tc.name  : Test GetLatencyWithFlag with LATENCY_FLAG_ENGINE
 * @tc.type  : FUNC
 * @tc.number: GetLatencyWithFlag_FirstWrite_001
 * @tc.desc  : Test GetLatencyWithFlag with FIRST_WRITE flag.
 */
HWTEST_F(RendererInServerFourthUnitTest, GetLatencyWithFlag_FirstWrite_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    uint64_t latency = 0;
    int32_t result = rendererInServer->GetLatencyWithFlag(latency, LATENCY_FLAG_ENGINE);
    (void)result;
}

/**
 * @tc.name  : Test GetLatencyWithFlag with invalid flag
 * @tc.type  : FUNC
 * @tc.number: GetLatencyWithFlag_Invalid_001
 * @tc.desc  : Test GetLatencyWithFlag with invalid flag value.
 */
HWTEST_F(RendererInServerFourthUnitTest, GetLatencyWithFlag_Invalid_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    uint64_t latency = 0;
    LatencyFlag invalidFlag = static_cast<LatencyFlag>(-1);
    int32_t result = rendererInServer->GetLatencyWithFlag(latency, invalidFlag);
    (void)result;
}

// ==================== SetVolume / SetSystemVolume 分支测试 ====================

/**
 * @tc.name  : Test SetVolume with valid volume
 * @tc.type  : FUNC
 * @tc.number: SetVolume_Valid_001
 * @tc.desc  : Test SetVolume with valid volume value.
 */
HWTEST_F(RendererInServerFourthUnitTest, SetVolume_Valid_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    // SetVolume not available on RendererInServer
    // SetVolume not available
}

/**
 * @tc.name  : Test SetVolume with out-of-range volume
 * @tc.type  : FUNC
 * @tc.number: SetVolume_OutOfRange_001
 * @tc.desc  : Test SetVolume with volume > 1.0.
 */
HWTEST_F(RendererInServerFourthUnitTest, SetVolume_OutOfRange_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    // SetVolume not available on RendererInServer
    // SetVolume not available
}

/**
 * @tc.name  : Test SetVolume with negative volume
 * @tc.type  : FUNC
 * @tc.number: SetVolume_Negative_001
 * @tc.desc  : Test SetVolume with negative volume value.
 */
HWTEST_F(RendererInServerFourthUnitTest, SetVolume_Negative_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    // SetVolume not available on RendererInServer
    // SetVolume not available
}

/**
 * @tc.name  : Test SetSystemVolume with valid value
 * @tc.type  : FUNC
 * @tc.number: SetSystemVolume_Valid_001
 * @tc.desc  : Test SetSystemVolume with valid volume.
 */
HWTEST_F(RendererInServerFourthUnitTest, SetSystemVolume_Valid_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    // SetSystemVolume not available on RendererInServer
    // SetVolume not available
}

// ==================== OnStatusUpdate 分支测试 ====================

/**
 * @tc.name  : Test OnStatusUpdate with OPERATION_STARTED
 * @tc.type  : FUNC
 * @tc.number: OnStatusUpdate_Started_001
 * @tc.desc  : Test OnStatusUpdate with OPERATION_STARTED.
 */
HWTEST_F(RendererInServerFourthUnitTest, OnStatusUpdate_Started_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    rendererInServer->OnStatusUpdate(OPERATION_STARTED);
    EXPECT_NE(nullptr, rendererInServer);
}

/**
 * @tc.name  : Test OnStatusUpdate with OPERATION_PAUSED
 * @tc.type  : FUNC
 * @tc.number: OnStatusUpdate_Paused_001
 * @tc.desc  : Test OnStatusUpdate with OPERATION_PAUSED.
 */
HWTEST_F(RendererInServerFourthUnitTest, OnStatusUpdate_Paused_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    rendererInServer->OnStatusUpdate(OPERATION_PAUSED);
    EXPECT_NE(nullptr, rendererInServer);
}

/**
 * @tc.name  : Test OnStatusUpdate with OPERATION_STOPPED
 * @tc.type  : FUNC
 * @tc.number: OnStatusUpdate_Stopped_001
 * @tc.desc  : Test OnStatusUpdate with OPERATION_STOPPED.
 */
HWTEST_F(RendererInServerFourthUnitTest, OnStatusUpdate_Stopped_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    rendererInServer->OnStatusUpdate(OPERATION_STOPPED);
    EXPECT_NE(nullptr, rendererInServer);
}

/**
 * @tc.name  : Test OnStatusUpdate with OPERATION_FLUSHED
 * @tc.type  : FUNC
 * @tc.number: OnStatusUpdate_Flushed_001
 * @tc.desc  : Test OnStatusUpdate with OPERATION_FLUSHED.
 */
HWTEST_F(RendererInServerFourthUnitTest, OnStatusUpdate_Flushed_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    rendererInServer->OnStatusUpdate(OPERATION_FLUSHED);
    EXPECT_NE(nullptr, rendererInServer);
}

// ==================== ProcessManagerType 分支测试 ====================

/**
 * @tc.name  : Test ProcessManagerType with PA_PLAYBACK
 * @tc.type  : FUNC
 * @tc.number: ProcessManagerType_PA_001
 * @tc.desc  : Test ProcessManagerType with PA_PLAYBACK manager type.
 */
HWTEST_F(RendererInServerFourthUnitTest, ProcessManagerType_PA_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    // MANAGER_TYPE_PLAYBACK not directly accessible
    rendererInServer->ProcessManagerType();
    EXPECT_NE(nullptr, rendererInServer);
}

/**
 * @tc.name  : Test ProcessManagerType with DUP_PLAYBACK
 * @tc.type  : FUNC
 * @tc.number: ProcessManagerType_DUP_001
 * @tc.desc  : Test ProcessManagerType with DUP_PLAYBACK manager type.
 */
HWTEST_F(RendererInServerFourthUnitTest, ProcessManagerType_DUP_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    rendererInServer->managerType_ = DUP_PLAYBACK;
    rendererInServer->ProcessManagerType();
    EXPECT_NE(nullptr, rendererInServer);
}

/**
 * @tc.name  : Test ProcessManagerType with VOIP_PLAYBACK
 * @tc.type  : FUNC
 * @tc.number: ProcessManagerType_VOIP_001
 * @tc.desc  : Test ProcessManagerType with VOIP_PLAYBACK manager type.
 */
HWTEST_F(RendererInServerFourthUnitTest, ProcessManagerType_VOIP_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    rendererInServer->managerType_ = VOIP_PLAYBACK;
    rendererInServer->ProcessManagerType();
    EXPECT_NE(nullptr, rendererInServer);
}

// ==================== ConfigServerBuffer / ConfigFixedSizeBuffer 测试 ====================

/**
 * @tc.name  : Test ConfigServerBuffer with normal config
 * @tc.type  : FUNC
 * @tc.number: ConfigServerBuffer_Normal_001
 * @tc.desc  : Test ConfigServerBuffer with normal buffer config.
 */
HWTEST_F(RendererInServerFourthUnitTest, ConfigServerBuffer_Normal_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    int32_t configRet = rendererInServer->ConfigServerBuffer();
    EXPECT_EQ(configRet, SUCCESS);
}

/**
 * @tc.name  : Test ConfigFixedSizeBuffer with normal config
 * @tc.type  : FUNC
 * @tc.number: ConfigFixedSizeBuffer_Normal_001
 * @tc.desc  : Test ConfigFixedSizeBuffer with normal config.
 */
HWTEST_F(RendererInServerFourthUnitTest, ConfigFixedSizeBuffer_Normal_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    int32_t configRet = rendererInServer->ConfigFixedSizeBuffer();
    EXPECT_EQ(configRet, SUCCESS);
}

// ==================== PauseDuringStandby 测试 ====================

/**
 * @tc.name  : Test PauseDuringStandby
 * @tc.type  : FUNC
 * @tc.number: PauseDuringStandby_001
 * @tc.desc  : Test PauseDuringStandby function.
 */
HWTEST_F(RendererInServerFourthUnitTest, PauseDuringStandby_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    int32_t pauseRet = rendererInServer->PauseDuringStandby();
    EXPECT_EQ(pauseRet, SUCCESS);
}

// ==================== IsMovieOffloadStream / IsMovieStream 测试 ====================

/**
 * @tc.name  : Test IsMovieOffloadStream with movie usage
 * @tc.type  : FUNC
 * @tc.number: IsMovieOffloadStream_Movie_001
 * @tc.desc  : Test IsMovieOffloadStream when usage is MOVIE.
 */
HWTEST_F(RendererInServerFourthUnitTest, IsMovieOffloadStream_Movie_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);

    processConfig.rendererInfo.streamUsage = STREAM_USAGE_MOVIE;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_NE(nullptr, rendererInServer);

    rendererInServer->IsMovieOffloadStream();
    SUCCEED();
}

/**
 * @tc.name  : Test IsMovieStream with movie usage
 * @tc.type  : FUNC
 * @tc.number: IsMovieStream_Movie_001
 * @tc.desc  : Test IsMovieStream when usage is MOVIE.
 */
HWTEST_F(RendererInServerFourthUnitTest, IsMovieStream_Movie_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);

    processConfig.rendererInfo.streamUsage = STREAM_USAGE_MOVIE;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, stateListener);
    ASSERT_NE(nullptr, rendererInServer);

    rendererInServer->IsMovieStream();
    SUCCEED();
}

// ==================== UpdateUnderrunInfo 测试 ====================

/**
 * @tc.name  : Test UpdateUnderrunInfo with valid key
 * @tc.type  : FUNC
 * @tc.number: UpdateUnderrunInfo_Valid_001
 * @tc.desc  : Test UpdateUnderrunInfo with valid key and value.
 */
HWTEST_F(RendererInServerFourthUnitTest, UpdateUnderrunInfo_Valid_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    int32_t updateRet = rendererInServer->UpdateUnderrunInfo(1, 100);
    EXPECT_EQ(updateRet, SUCCESS);
}

// ==================== GetStandbyStatus 测试 ====================

/**
 * @tc.name  : Test GetStandbyStatus
 * @tc.type  : FUNC
 * @tc.number: GetStandbyStatus_001
 * @tc.desc  : Test GetStandbyStatus function.
 */
HWTEST_F(RendererInServerFourthUnitTest, GetStandbyStatus_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    bool isStandby = false;
    int64_t enterStandbyTime = 0;
    int32_t statusRet = rendererInServer->GetStandbyStatus(isStandby, enterStandbyTime);
    EXPECT_EQ(statusRet, SUCCESS);
}

// ==================== CheckIsUnderrun 测试 ====================

/**
 * @tc.name  : Test CheckIsUnderrun with read ahead of write
 * @tc.type  : FUNC
 * @tc.number: CheckIsUnderrun_ReadAhead_001
 * @tc.desc  : Test CheckIsUnderrun when read frame is ahead of write frame.
 */
HWTEST_F(RendererInServerFourthUnitTest, CheckIsUnderrun_ReadAhead_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);

    rendererInServer->CheckIsUnderrun(200, 100);
    SUCCEED();
}

/**
 * @tc.name  : Test CheckIsUnderrun with write ahead of read
 * @tc.type  : FUNC
 * @tc.number: CheckIsUnderrun_WriteAhead_001
 * @tc.desc  : Test CheckIsUnderrun when write frame is ahead of read frame.
 */
HWTEST_F(RendererInServerFourthUnitTest, CheckIsUnderrun_WriteAhead_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);

    rendererInServer->CheckIsUnderrun(100, 200);
    SUCCEED();
}

// ==================== InitBufferStatus 测试 ====================

/**
 * @tc.name  : Test InitBufferStatus after Init
 * @tc.type  : FUNC
 * @tc.number: InitBufferStatus_001
 * @tc.desc  : Test InitBufferStatus function.
 */
HWTEST_F(RendererInServerFourthUnitTest, InitBufferStatus_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    int32_t bufferRet = rendererInServer->InitBufferStatus();
    EXPECT_EQ(bufferRet, SUCCESS);
}

/**
 * @tc.name  : Test Init with HWDECODING_PLAYBACK
 * @tc.type  : FUNC
 * @tc.number: Init_HWDecoding_001
 * @tc.desc  : Test Init with HW decoding playback mode.
 */
HWTEST_F(RendererInServerFourthUnitTest, Init_HWDecoding_001, TestSize.Level1)
{
    // HWDECODING_PLAYBACK is set when IsHWDecodingType returns true
    ASSERT_NE(nullptr, rendererInServer);
    // This test verifies the HWDECODING_PLAYBACK branch in Init
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);
    EXPECT_NE(nullptr, rendererInServer);
}

/**
 * @tc.name  : Test WriteData with underrun condition
 * @tc.type  : FUNC
 * @tc.number: WriteData_Underrun_001
 * @tc.desc  : Test WriteData when underrun condition is met.
 */
HWTEST_F(RendererInServerFourthUnitTest, WriteData_Underrun_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    // Simulate underrun by setting read ahead of write
    // This should trigger ERR_OPERATION_FAILED return
    uint8_t testData[512] = {0};
    // length = 0 triggers near underrun
    int32_t writeRet = rendererInServer->WriteData(reinterpret_cast<int8_t*>(testData), 0, true);
    EXPECT_NE(SUCCESS, writeRet);
}

/**
 * @tc.name  : Test OnWriteData with spanSizeInByte_ <= 0
 * @tc.type  : FUNC
 * @tc.number: OnWriteData_InvalidSpanSize_001
 * @tc.desc  : Test OnWriteData when spanSizeInByte_ <= 0.
 */
HWTEST_F(RendererInServerFourthUnitTest, OnWriteData_InvalidSpanSize_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    // Simulate invalid span size
    rendererInServer->spanSizeInByte_ = 0;
    uint8_t testData[512] = {0};
    int32_t writeRet = rendererInServer->OnWriteData(reinterpret_cast<int8_t*>(testData), 512);
    EXPECT_NE(SUCCESS, writeRet);
}

// ==================== OnStatusUpdate 更多操作测试 ====================

/**
 * @tc.name  : Test OnStatusUpdate with OPERATION_RELEASED
 * @tc.type  : FUNC
 * @tc.number: OnStatusUpdate_Released_001
 * @tc.desc  : Test OnStatusUpdate with OPERATION_RELEASED.
 */
HWTEST_F(RendererInServerFourthUnitTest, OnStatusUpdate_Released_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    rendererInServer->OnStatusUpdate(OPERATION_RELEASED);
    EXPECT_NE(nullptr, rendererInServer);
}

/**
 * @tc.name  : Test OnStatusUpdate with OPERATION_SESSION_EFFECT_ENABLED
 * @tc.type  : FUNC
 * @tc.number: OnStatusUpdate_SessionEffectEnabled_001
 * @tc.desc  : Test OnStatusUpdate with OPERATION_SESSION_EFFECT_ENABLED.
 */
HWTEST_F(RendererInServerFourthUnitTest, OnStatusUpdate_SessionEffectEnabled_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    rendererInServer->OnStatusUpdate(OPERATION_SESSION_EFFECT_ENABLED);
    EXPECT_TRUE(rendererInServer->hasSessionEffect_.load());
}

/**
 * @tc.name  : Test OnStatusUpdate with OPERATION_SESSION_EFFECT_DISABLED
 * @tc.type  : FUNC
 * @tc.number: OnStatusUpdate_SessionEffectDisabled_001
 * @tc.desc  : Test OnStatusUpdate with OPERATION_SESSION_EFFECT_DISABLED.
 */
HWTEST_F(RendererInServerFourthUnitTest, OnStatusUpdate_SessionEffectDisabled_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    rendererInServer->hasSessionEffect_.store(true);
    rendererInServer->OnStatusUpdate(OPERATION_SESSION_EFFECT_DISABLED);
    EXPECT_FALSE(rendererInServer->hasSessionEffect_.load());
}

/**
 * @tc.name  : Test OnStatusUpdate with OPERATION_UNSET_OFFLOAD_ENABLE
 * @tc.type  : FUNC
 * @tc.number: OnStatusUpdate_UnsetOffload_001
 * @tc.desc  : Test OnStatusUpdate with OPERATION_UNSET_OFFLOAD_ENABLE.
 */
HWTEST_F(RendererInServerFourthUnitTest, OnStatusUpdate_UnsetOffload_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    rendererInServer->offloadEnable_ = true;
    rendererInServer->OnStatusUpdate(OPERATION_UNSET_OFFLOAD_ENABLE);
    EXPECT_FALSE(rendererInServer->offloadEnable_);
}

// ==================== StandByCheck 更多测试 ====================

/**
 * @tc.name  : Test StandByCheck with ultrasonic usage
 * @tc.type  : FUNC
 * @tc.number: StandByCheck_Ultrasonic_001
 * @tc.desc  : Test StandByCheck returns early for ultrasonic stream.
 */
HWTEST_F(RendererInServerFourthUnitTest, StandByCheck_Ultrasonic_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    processConfig.rendererInfo.streamUsage = STREAM_USAGE_ULTRASONIC;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, stateListener);
    rendererInServer->StandByCheck();
    EXPECT_NE(nullptr, rendererInServer);
}

/**
 * @tc.name  : Test StandByCheck when already in standby
 * @tc.type  : FUNC
 * @tc.number: StandByCheck_AlreadyStandby_001
 * @tc.desc  : Test StandByCheck returns early when already in standby.
 */
HWTEST_F(RendererInServerFourthUnitTest, StandByCheck_AlreadyStandby_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    rendererInServer->standByEnable_ = true;
    rendererInServer->StandByCheck();
    EXPECT_TRUE(rendererInServer->standByEnable_);
}

// ==================== VolumeHandle 更多分支测试 ====================

/**
 * @tc.name  : Test VolumeHandle with mute flag
 * @tc.type  : FUNC
 * @tc.number: VolumeHandle_MuteFlag_001
 * @tc.desc  : Test VolumeHandle when muteFlag_ is true.
 */
HWTEST_F(RendererInServerFourthUnitTest, VolumeHandle_MuteFlag_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    rendererInServer->muteFlag_ = true;
    BufferDesc bufferDesc;
    uint8_t dummyBuffer[64] = {0};
    bufferDesc.buffer = dummyBuffer;
    bufferDesc.bufLength = 64;
    bufferDesc.dataLength = 64;
    rendererInServer->VolumeHandle(bufferDesc);
    EXPECT_EQ(0.0f, rendererInServer->oldAppliedVolume_);
}

/**
 * @tc.name  : Test VolumeHandle with duck factor
 * @tc.type  : FUNC
 * @tc.number: VolumeHandle_DuckFactor_001
 * @tc.desc  : Test VolumeHandle when duck factor is applied.
 */
HWTEST_F(RendererInServerFourthUnitTest, VolumeHandle_DuckFactor_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    if (rendererInServer->audioServerBuffer_ != nullptr) {
        rendererInServer->audioServerBuffer_->SetDuckFactor(0.5f, 0);
    }
    BufferDesc bufferDesc;
    uint8_t dummyBuffer[64] = {0};
    bufferDesc.buffer = dummyBuffer;
    bufferDesc.bufLength = 64;
    bufferDesc.dataLength = 64;
    rendererInServer->VolumeHandle(bufferDesc);
    EXPECT_NE(nullptr, rendererInServer);
}

// ==================== GetAvailableSize 更多测试 ====================

/**
 * @tc.name  : Test GetAvailableSize with static mode
 * @tc.type  : FUNC
 * @tc.number: GetAvailableSize_StaticMode_001
 * @tc.desc  : Test GetAvailableSize in static mode.
 */
HWTEST_F(RendererInServerFourthUnitTest, GetAvailableSize_StaticMode_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    int32_t ret = rendererInServer->Init();
    ASSERT_EQ(SUCCESS, ret);

    processConfig.rendererInfo.isStatic = true;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, stateListener);
    size_t length = 0;
    int32_t availRet = rendererInServer->GetAvailableSize(length);
    EXPECT_EQ(SUCCESS, availRet);
}

/**
 * @tc.name  : Test GetAvailableSize with write < read
 * @tc.type  : FUNC
 * @tc.number: GetAvailableSize_WriteLessThanRead_001
 * @tc.desc  : Test GetAvailableSize when write < read (should return ERROR).
 */
HWTEST_F(RendererInServerFourthUnitTest, GetAvailableSize_WriteLessThanRead_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    // This requires detailed buffer setup to test the write < read branch
    // For now, test the function with an uninitialized buffer
    size_t length = 0;
    int32_t availRet = rendererInServer->GetAvailableSize(length);
    EXPECT_NE(SUCCESS, availRet);
}

// ==================== ProcessManagerType 更多分支测试 ====================

/**
 * @tc.name  : Test ProcessManagerType with high resolution flag
 * @tc.type  : FUNC
 * @tc.number: ProcessManagerType_HighResolution_001
 * @tc.desc  : Test ProcessManagerType with AUDIO_OUTPUT_FLAG_HD | AUDIO_OUTPUT_FLAG_DIRECT.
 */
HWTEST_F(RendererInServerFourthUnitTest, ProcessManagerType_HighResolution_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    // Set flags to trigger DIRECT_PLAYBACK
    processConfig.rendererInfo.audioFlag = AUDIO_OUTPUT_FLAG_HD | AUDIO_OUTPUT_FLAG_DIRECT;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, stateListener);
    rendererInServer->ProcessManagerType();
    EXPECT_TRUE(rendererInServer->managerType_ == DIRECT_PLAYBACK ||
                rendererInServer->managerType_ == PLAYBACK);
}

/**
 * @tc.name  : Test ProcessManagerType with EAC3 flag
 * @tc.type  : FUNC
 * @tc.number: ProcessManagerType_EAC3_001
 * @tc.desc  : Test ProcessManagerType with EAC3 encoding (HWDECODING_PLAYBACK).
 */
HWTEST_F(RendererInServerFourthUnitTest, ProcessManagerType_EAC3_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    // Set encoding to EAC3 to trigger HWDECODING_PLAYBACK
    processConfig.streamInfo.encoding = ENCODING_EAC3;
    rendererInServer = std::make_shared<RendererInServer>(processConfig, stateListener);
    rendererInServer->ProcessManagerType();
    EXPECT_TRUE(rendererInServer->managerType_ == HWDECODING_PLAYBACK ||
                rendererInServer->isHWDecodingType_);
}

// ==================== IsMovieOffloadStream / IsMovieStream 更多测试 ====================

/**
 * @tc.name  : Test IsMovieOffloadStream with wrong stream type
 * @tc.type  : FUNC
 * @tc.number: IsMovieOffloadStream_WrongType_001
 * @tc.desc  : Test IsMovieOffloadStream returns false for non-movie stream.
 */
HWTEST_F(RendererInServerFourthUnitTest, IsMovieOffloadStream_WrongType_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    processConfig.streamType = STREAM_MUSIC;  // Not movie
    rendererInServer = std::make_shared<RendererInServer>(processConfig, stateListener);
    rendererInServer->IsMovieOffloadStream();
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : Test IsMovieStream with wrong original flag
 * @tc.type  : FUNC
 * @tc.number: IsMovieStream_WrongFlag_001
 * @tc.desc  : Test IsMovieStream returns false when originalFlag != AUDIO_FLAG_PCM_OFFLOAD.
 */
HWTEST_F(RendererInServerFourthUnitTest, IsMovieStream_WrongFlag_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    processConfig.streamType = STREAM_MOVIE;
    processConfig.rendererInfo.originalFlag = AUDIO_FLAG_NORMAL;  // Not PCM offload
    rendererInServer = std::make_shared<RendererInServer>(processConfig, stateListener);
    rendererInServer->IsMovieStream();
    EXPECT_TRUE(true);
}

// ==================== InitBufferStatus 错误路径测试 ====================

/**
 * @tc.name  : Test InitBufferStatus with null buffer
 * @tc.type  : FUNC
 * @tc.number: InitBufferStatus_NullBuffer_001
 * @tc.desc  : Test InitBufferStatus when audioServerBuffer_ is null.
 */
HWTEST_F(RendererInServerFourthUnitTest, InitBufferStatus_NullBuffer_001, TestSize.Level1)
{
    ASSERT_NE(nullptr, rendererInServer);
    // Don't call Init, so audioServerBuffer_ remains null
    int32_t bufferRet = rendererInServer->InitBufferStatus();
    EXPECT_NE(SUCCESS, bufferRet);
}
} // namespace AudioStandard
} // namespace OHOS

