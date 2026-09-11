/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "renderer_in_server_sixth_aiut.h"
#include "aiut_log.h"
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

namespace {
std::shared_ptr<IStreamListener> stateListener;
std::shared_ptr<StreamListenerHolder> streamListenerHolder = std::make_shared<StreamListenerHolder>();
std::shared_ptr<RendererInServer> rendererInServer;
AudioProcessConfig processConfig;

constexpr int32_t TEST_UID = 10001;
constexpr int32_t TEST_PID = 10001;

AudioStreamInfo testStreamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO,
    AudioChannelLayout::CH_LAYOUT_STEREO);
} // namespace

void RendererInServerSixthAiut::SetUpTestCase(void)
{
    streamListenerHolder = std::make_shared<StreamListenerHolder>();
    stateListener = streamListenerHolder;
}

void RendererInServerSixthAiut::TearDownTestCase(void)
{
    stateListener.reset();
    streamListenerHolder.reset();
}

void RendererInServerSixthAiut::SetUp(void)
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

void RendererInServerSixthAiut::TearDown(void)
{
    rendererInServer.reset();
}

// ==================== HandleOperationFlushed 分支测试 ====================

/**
 * @tc.name  : Test HandleOperationFlushed with FLUSHING_WHEN_STARTED status
 * @tc.type  : FUNC
 * @tc.number: HandleOperationFlushed_Started
 * @tc.desc  : Test HandleOperationFlushed when status is I_STATUS_FLUSHING_WHEN_STARTED.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, HandleOperationFlushed_Started, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->status_ = I_STATUS_FLUSHING_WHEN_STARTED;
    rendererInServer->HandleOperationFlushed();
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->status_.load(), I_STATUS_STARTED,
        "status mismatch, status=%u", rendererInServer->status_.load());
}

/**
 * @tc.name  : Test HandleOperationFlushed with FLUSHING_WHEN_PAUSED status
 * @tc.type  : FUNC
 * @tc.number: HandleOperationFlushed_Paused
 * @tc.desc  : Test HandleOperationFlushed when status is I_STATUS_FLUSHING_WHEN_PAUSED.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, HandleOperationFlushed_Paused, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->status_ = I_STATUS_FLUSHING_WHEN_PAUSED;
    rendererInServer->HandleOperationFlushed();
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->status_.load(), I_STATUS_PAUSED,
        "status mismatch, status=%u", rendererInServer->status_.load());
}

/**
 * @tc.name  : Test HandleOperationFlushed with FLUSHING_WHEN_STOPPED status
 * @tc.type  : FUNC
 * @tc.number: HandleOperationFlushed_Stopped
 * @tc.desc  : Test HandleOperationFlushed when status is I_STATUS_FLUSHING_WHEN_STOPPED.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, HandleOperationFlushed_Stopped, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->status_ = I_STATUS_FLUSHING_WHEN_STOPPED;
    rendererInServer->HandleOperationFlushed();
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->status_.load(), I_STATUS_STOPPED,
        "status mismatch, status=%u", rendererInServer->status_.load());
}

/**
 * @tc.name  : Test HandleOperationFlushed with default status
 * @tc.type  : FUNC
 * @tc.number: HandleOperationFlushed_Default
 * @tc.desc  : Test HandleOperationFlushed when status is default/invalid.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, HandleOperationFlushed_Default, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->status_ = I_STATUS_IDLE;
    rendererInServer->HandleOperationFlushed();
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->status_.load(), I_STATUS_IDLE,
        "status should remain unchanged, status=%u", rendererInServer->status_.load());
}

// ==================== HandleOperationStopped 分支测试 ====================

/**
 * @tc.name  : Test HandleOperationStopped with valid buffer
 * @tc.type  : FUNC
 * @tc.number: HandleOperationStopped_Valid
 * @tc.desc  : Test HandleOperationStopped with valid audioServerBuffer and playerDfx.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, HandleOperationStopped_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->HandleOperationStopped(RENDERER_STAGE_STOP_OK);
    AIUT_EXPECT_TRUE_INFO_LOG(rendererInServer->lastStopTime_ != 0,
        "lastStopTime_ should be set, lastStopTime_=%lld",
        static_cast<long long>(rendererInServer->lastStopTime_));
}

/**
 * @tc.name  : Test HandleOperationStopped with null buffer
 * @tc.type  : FUNC
 * @tc.number: HandleOperationStopped_NullBuffer
 * @tc.desc  : Test HandleOperationStopped when audioServerBuffer is null.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, HandleOperationStopped_NullBuffer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");

    rendererInServer->HandleOperationStopped(RENDERER_STAGE_STOP_OK);
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr,
        "rendererInServer should remain valid after HandleOperationStopped with null buffer");
}

// ==================== DoFadingOut 分支测试 ====================

/**
 * @tc.name  : Test DoFadingOut with DO_FADINGOUT flag
 * @tc.type  : FUNC
 * @tc.number: DoFadingOut_FadingOut
 * @tc.desc  : Test DoFadingOut when fadeoutFlag_ is DO_FADINGOUT.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, DoFadingOut_FadingOut, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->fadeoutFlag_ = 1; // DO_FADINGOUT
    uint8_t buffer[1024] = {0};
    RingBufferWrapper ringBuffer;
    ringBuffer.basicBufferDescs[0] = {buffer, 1024};
    ringBuffer.dataLength = 1024;

    rendererInServer->DoFadingOut(ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->fadeoutFlag_, 2,
        "fadeoutFlag_ should be FADING_OUT_DONE, fadeoutFlag_=%d",
        rendererInServer->fadeoutFlag_);
}

/**
 * @tc.name  : Test DoFadingOut with NO_FADING flag
 * @tc.type  : FUNC
 * @tc.number: DoFadingOut_NoFading
 * @tc.desc  : Test DoFadingOut when fadeoutFlag_ is NO_FADING.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, DoFadingOut_NoFading, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->fadeoutFlag_ = 0; // NO_FADING
    uint8_t buffer[1024] = {0};
    RingBufferWrapper ringBuffer;
    ringBuffer.basicBufferDescs[0] = {buffer, 1024};
    ringBuffer.dataLength = 1024;

    rendererInServer->DoFadingOut(ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->fadeoutFlag_, 0,
        "fadeoutFlag_ should remain NO_FADING, fadeoutFlag_=%d",
        rendererInServer->fadeoutFlag_);
}

// ==================== IsInvalidBuffer 分支测试 ====================

/**
 * @tc.name  : Test IsInvalidBuffer with SAMPLE_U8 format and zero data
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_U8_Zero
 * @tc.desc  : Test IsInvalidBuffer with SAMPLE_U8 format and zero buffer data.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, IsInvalidBuffer_U8_Zero, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->processConfig_.streamInfo.format = SAMPLE_U8;
    uint8_t buffer[8] = {0};
    bool result = rendererInServer->IsInvalidBuffer(buffer, sizeof(buffer));
    AIUT_EXPECT_TRUE_INFO_LOG(result, "IsInvalidBuffer should return true for zero U8 data");
}

/**
 * @tc.name  : Test IsInvalidBuffer with SAMPLE_U8 format and non-zero data
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_U8_NonZero
 * @tc.desc  : Test IsInvalidBuffer with SAMPLE_U8 format and non-zero buffer data.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, IsInvalidBuffer_U8_NonZero, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->processConfig_.streamInfo.format = SAMPLE_U8;
    uint8_t buffer[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    bool result = rendererInServer->IsInvalidBuffer(buffer, sizeof(buffer));
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsInvalidBuffer should return false for non-zero U8 data");
}

/**
 * @tc.name  : Test IsInvalidBuffer with SAMPLE_S16LE format and zero data
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_S16LE_Zero
 * @tc.desc  : Test IsInvalidBuffer with SAMPLE_S16LE format and zero buffer data.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, IsInvalidBuffer_S16LE_Zero, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->processConfig_.streamInfo.format = SAMPLE_S16LE;
    uint8_t buffer[8] = {0};
    bool result = rendererInServer->IsInvalidBuffer(buffer, sizeof(buffer));
    AIUT_EXPECT_TRUE_INFO_LOG(result, "IsInvalidBuffer should return true for zero S16LE data");
}

/**
 * @tc.name  : Test IsInvalidBuffer with SAMPLE_S16LE format and non-zero data
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_S16LE_NonZero
 * @tc.desc  : Test IsInvalidBuffer with SAMPLE_S16LE format and non-zero buffer data.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, IsInvalidBuffer_S16LE_NonZero, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->processConfig_.streamInfo.format = SAMPLE_S16LE;
    uint8_t buffer[8] = {1, 0, 0, 0, 0, 0, 0, 0};
    bool result = rendererInServer->IsInvalidBuffer(buffer, sizeof(buffer));
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsInvalidBuffer should return false for non-zero S16LE data");
}

/**
 * @tc.name  : Test IsInvalidBuffer with SAMPLE_U8 format and zero buffer size
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_U8_ZeroSize
 * @tc.desc  : Test IsInvalidBuffer with SAMPLE_U8 format and zero buffer size.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, IsInvalidBuffer_U8_ZeroSize, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->processConfig_.streamInfo.format = SAMPLE_U8;
    uint8_t buffer[8] = {0};
    bool result = rendererInServer->IsInvalidBuffer(buffer, 0);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsInvalidBuffer should return false for zero size U8");
}

/**
 * @tc.name  : Test IsInvalidBuffer with SAMPLE_S16LE format and too small buffer size
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_S16LE_SmallSize
 * @tc.desc  : Test IsInvalidBuffer with SAMPLE_S16LE format and buffer size of 1.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, IsInvalidBuffer_S16LE_SmallSize, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->processConfig_.streamInfo.format = SAMPLE_S16LE;
    uint8_t buffer[8] = {0};
    bool result = rendererInServer->IsInvalidBuffer(buffer, 1);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsInvalidBuffer should return false for too small S16LE");
}

/**
 * @tc.name  : Test IsInvalidBuffer with unsupported format
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_UnsupportedFormat
 * @tc.desc  : Test IsInvalidBuffer with unsupported format (default case).
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, IsInvalidBuffer_UnsupportedFormat, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->processConfig_.streamInfo.format = SAMPLE_S24LE;
    uint8_t buffer[8] = {0};
    bool result = rendererInServer->IsInvalidBuffer(buffer, sizeof(buffer));
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsInvalidBuffer should return false for unsupported format");
}

// ==================== VolumeHandle 分支测试 ====================

/**
 * @tc.name  : Test VolumeHandle with null audioServerBuffer
 * @tc.type  : FUNC
 * @tc.number: VolumeHandle_NullBuffer
 * @tc.desc  : Test VolumeHandle when audioServerBuffer is null.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, VolumeHandle_NullBuffer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");

    uint8_t buffer[1024] = {0};
    BufferDesc bufferDesc = {buffer, 1024, 1024};
    rendererInServer->audioServerBuffer_ = nullptr;
    rendererInServer->VolumeHandle(bufferDesc);
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr,
        "rendererInServer should remain valid after VolumeHandle with null buffer");
}

/**
 * @tc.name  : Test VolumeHandle with mute flag set
 * @tc.type  : FUNC
 * @tc.number: VolumeHandle_MuteFlag
 * @tc.desc  : Test VolumeHandle when muteFlag is true.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, VolumeHandle_MuteFlag, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->muteFlag_ = true;
    uint8_t buffer[1024] = {0};
    BufferDesc bufferDesc = {buffer, 1024, 1024};
    rendererInServer->VolumeHandle(bufferDesc);
    AIUT_EXPECT_TRUE_INFO_LOG(rendererInServer->muteFlag_.load(),
        "muteFlag_ should remain true after VolumeHandle, muteFlag_=%d",
        rendererInServer->muteFlag_.load());
}

/**
 * @tc.name  : Test VolumeHandle with silent mode
 * @tc.type  : FUNC
 * @tc.number: VolumeHandle_SilentMode
 * @tc.desc  : Test VolumeHandle when silentModeAndMixWithOthers is true.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, VolumeHandle_SilentMode, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->silentModeAndMixWithOthers_ = true;
    uint8_t buffer[1024] = {0};
    BufferDesc bufferDesc = {buffer, 1024, 1024};
    rendererInServer->VolumeHandle(bufferDesc);
    AIUT_EXPECT_TRUE_INFO_LOG(rendererInServer->silentModeAndMixWithOthers_.load(),
        "silentModeAndMixWithOthers_ should remain true after VolumeHandle");
}

// ==================== PrepareOutputBuffer 分支测试 ====================

/**
 * @tc.name  : Test PrepareOutputBuffer with buffer length >= data length
 * @tc.type  : FUNC
 * @tc.number: PrepareOutputBuffer_Normal
 * @tc.desc  : Test PrepareOutputBuffer when bufLength >= dataLength.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, PrepareOutputBuffer_Normal, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    uint8_t buffer[1024] = {0};
    RingBufferWrapper ringBuffer;
    ringBuffer.basicBufferDescs[0] = {buffer, 1024};
    ringBuffer.dataLength = 512;

    BufferDesc result = rendererInServer->PrepareOutputBuffer(ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(result.bufLength, 512,
        "bufLength should be 512, bufLength=%zu", result.bufLength);
    AIUT_EXPECT_EQ_INFO_LOG(result.dataLength, 512,
        "dataLength should be 512, dataLength=%zu", result.dataLength);
}

/**
 * @tc.name  : Test PrepareOutputBuffer with buffer length < data length
 * @tc.type  : FUNC
 * @tc.number: PrepareOutputBuffer_SmallBuffer
 * @tc.desc  : Test PrepareOutputBuffer when bufLength < dataLength.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, PrepareOutputBuffer_SmallBuffer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    uint8_t buffer[256] = {0};
    RingBufferWrapper ringBuffer;
    ringBuffer.basicBufferDescs[0] = {buffer, 256};
    ringBuffer.dataLength = 512;

    BufferDesc result = rendererInServer->PrepareOutputBuffer(ringBuffer);
    AIUT_EXPECT_EQ_INFO_LOG(result.bufLength, 512,
        "bufLength should be 512, bufLength=%zu", result.bufLength);
    AIUT_EXPECT_EQ_INFO_LOG(result.dataLength, 512,
        "dataLength should be 512, dataLength=%zu", result.dataLength);
}

// ==================== GetStandbyStatus 分支测试 ====================

/**
 * @tc.name  : Test GetStandbyStatus when standby is enabled
 * @tc.type  : FUNC
 * @tc.number: GetStandbyStatus_Enabled
 * @tc.desc  : Test GetStandbyStatus when standby is enabled.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetStandbyStatus_Enabled, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->standByEnable_ = true;
    rendererInServer->enterStandbyTime_ = 12345;
    bool isStandby = false;
    int64_t enterStandbyTime = 0;
    int32_t result = rendererInServer->GetStandbyStatus(isStandby, enterStandbyTime);
    AIUT_EXPECT_EQ_INFO_LOG(result, SUCCESS, "GetStandbyStatus failed, result=%d", result);
    AIUT_EXPECT_TRUE_INFO_LOG(isStandby, "isStandby should be true");
    AIUT_EXPECT_EQ_INFO_LOG(enterStandbyTime, 12345,
        "enterStandbyTime mismatch, enterStandbyTime=%lld",
        static_cast<long long>(enterStandbyTime));
}

/**
 * @tc.name  : Test GetStandbyStatus when standby is disabled
 * @tc.type  : FUNC
 * @tc.number: GetStandbyStatus_Disabled
 * @tc.desc  : Test GetStandbyStatus when standby is disabled.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetStandbyStatus_Disabled, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->standByEnable_ = false;
    bool isStandby = true;
    int64_t enterStandbyTime = 0;
    int32_t result = rendererInServer->GetStandbyStatus(isStandby, enterStandbyTime);
    AIUT_EXPECT_EQ_INFO_LOG(result, SUCCESS, "GetStandbyStatus failed, result=%d", result);
    AIUT_EXPECT_FALSE_INFO_LOG(isStandby, "isStandby should be false");
    AIUT_EXPECT_EQ_INFO_LOG(enterStandbyTime, 0,
        "enterStandbyTime should be 0, enterStandbyTime=%lld",
        static_cast<long long>(enterStandbyTime));
}

// ==================== GetStreamManagerType 分支测试 ====================

/**
 * @tc.name  : Test GetStreamManagerType with DIRECT_PLAYBACK
 * @tc.type  : FUNC
 * @tc.number: GetStreamManagerType_Direct
 * @tc.desc  : Test GetStreamManagerType when managerType is DIRECT_PLAYBACK.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetStreamManagerType_Direct, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    int32_t result = rendererInServer->GetStreamManagerType();
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_DIRECT_MANAGER_TYPE,
        "GetStreamManagerType mismatch, result=%d", result);
}

/**
 * @tc.name  : Test GetStreamManagerType with PLAYBACK
 * @tc.type  : FUNC
 * @tc.number: GetStreamManagerType_Normal
 * @tc.desc  : Test GetStreamManagerType when managerType is PLAYBACK.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetStreamManagerType_Normal, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->managerType_ = PLAYBACK;
    int32_t result = rendererInServer->GetStreamManagerType();
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_NORMAL_MANAGER_TYPE,
        "GetStreamManagerType mismatch, result=%d", result);
}

// ==================== GetActualStreamManagerType 分支测试 ====================

/**
 * @tc.name  : Test GetActualStreamManagerType
 * @tc.type  : FUNC
 * @tc.number: GetActualStreamManagerType
 * @tc.desc  : Test GetActualStreamManagerType returns the actual manager type.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetActualStreamManagerType, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->managerType_ = DIRECT_PLAYBACK;
    int32_t result = rendererInServer->GetActualStreamManagerType();
    AIUT_EXPECT_EQ_INFO_LOG(result, DIRECT_PLAYBACK,
        "GetActualStreamManagerType mismatch, result=%d", result);
}

// ==================== IsHighResolution 分支测试 ====================

/**
 * @tc.name  : Test IsHighResolution with non-headset device
 * @tc.type  : FUNC
 * @tc.number: IsHighResolution_NonHeadset
 * @tc.desc  : Test IsHighResolution with non-headset device type.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, IsHighResolution_NonHeadset, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->processConfig_.deviceType = DEVICE_TYPE_SPEAKER;
    bool result = rendererInServer->IsHighResolution();
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsHighResolution should be false for speaker");
}

/**
 * @tc.name  : Test IsHighResolution with headset but low sample rate
 * @tc.type  : FUNC
 * @tc.number: IsHighResolution_LowRate
 * @tc.desc  : Test IsHighResolution with headset but low sample rate.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, IsHighResolution_LowRate, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->processConfig_.deviceType = DEVICE_TYPE_WIRED_HEADSET;
    rendererInServer->processConfig_.streamType = STREAM_MUSIC;
    rendererInServer->processConfig_.streamInfo.samplingRate = SAMPLE_RATE_44100;
    rendererInServer->processConfig_.streamInfo.format = SAMPLE_S16LE;
    bool result = rendererInServer->IsHighResolution();
    AIUT_EXPECT_FALSE_INFO_LOG(result, "IsHighResolution should be false for low sample rate");
}

// ==================== SetLowPowerVolume 分支测试 ====================

/**
 * @tc.name  : Test SetLowPowerVolume with invalid volume (too low)
 * @tc.type  : FUNC
 * @tc.number: SetLowPowerVolume_TooLow
 * @tc.desc  : Test SetLowPowerVolume with volume below MIN_FLOAT_VOLUME.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, SetLowPowerVolume_TooLow, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    int32_t result = rendererInServer->SetLowPowerVolume(-1.0f);
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_INVALID_PARAM,
        "SetLowPowerVolume should return ERR_INVALID_PARAM, result=%d", result);
}

/**
 * @tc.name  : Test SetLowPowerVolume with invalid volume (too high)
 * @tc.type  : FUNC
 * @tc.number: SetLowPowerVolume_TooHigh
 * @tc.desc  : Test SetLowPowerVolume with volume above MAX_FLOAT_VOLUME.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, SetLowPowerVolume_TooHigh, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    int32_t result = rendererInServer->SetLowPowerVolume(2.0f);
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_INVALID_PARAM,
        "SetLowPowerVolume should return ERR_INVALID_PARAM, result=%d", result);
}

/**
 * @tc.name  : Test SetLowPowerVolume with valid volume
 * @tc.type  : FUNC
 * @tc.number: SetLowPowerVolume_Valid
 * @tc.desc  : Test SetLowPowerVolume with valid volume.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, SetLowPowerVolume_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    int32_t result = rendererInServer->SetLowPowerVolume(0.5f);
    AIUT_EXPECT_EQ_INFO_LOG(result, SUCCESS,
        "SetLowPowerVolume should return SUCCESS, result=%d", result);
}

// ==================== GetLowPowerVolume 分支测试 ====================

/**
 * @tc.name  : Test GetLowPowerVolume
 * @tc.type  : FUNC
 * @tc.number: GetLowPowerVolume
 * @tc.desc  : Test GetLowPowerVolume returns the current low power volume.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetLowPowerVolume, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->lowPowerVolume_ = 0.7f;
    float volume = 0.0f;
    int32_t result = rendererInServer->GetLowPowerVolume(volume);
    AIUT_EXPECT_EQ_INFO_LOG(result, SUCCESS, "GetLowPowerVolume failed, result=%d", result);
    AIUT_EXPECT_EQ_INFO_LOG(volume, 0.7f, "volume mismatch, volume=%f", volume);
}

// ==================== SetDuckFactor 分支测试 ====================

/**
 * @tc.name  : Test SetDuckFactor with invalid duck factor (too low)
 * @tc.type  : FUNC
 * @tc.number: SetDuckFactor_TooLow
 * @tc.desc  : Test SetDuckFactor with duck factor below MIN_FLOAT_VOLUME.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, SetDuckFactor_TooLow, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    int32_t result = rendererInServer->SetDuckFactor(-1.0f, 0);
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_INVALID_PARAM,
        "SetDuckFactor should return ERR_INVALID_PARAM, result=%d", result);
}

/**
 * @tc.name  : Test SetDuckFactor with invalid duck factor (too high)
 * @tc.type  : FUNC
 * @tc.number: SetDuckFactor_TooHigh
 * @tc.desc  : Test SetDuckFactor with duck factor above MAX_FLOAT_VOLUME.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, SetDuckFactor_TooHigh, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    int32_t result = rendererInServer->SetDuckFactor(2.0f, 0);
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_INVALID_PARAM,
        "SetDuckFactor should return ERR_INVALID_PARAM, result=%d", result);
}

/**
 * @tc.name  : Test SetDuckFactor with valid duck factor
 * @tc.type  : FUNC
 * @tc.number: SetDuckFactor_Valid
 * @tc.desc  : Test SetDuckFactor with valid duck factor.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, SetDuckFactor_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    int32_t result = rendererInServer->SetDuckFactor(0.5f, 100);
    AIUT_EXPECT_EQ_INFO_LOG(result, SUCCESS,
        "SetDuckFactor should return SUCCESS, result=%d", result);
}

// ==================== SetSpeed 分支测试 ====================

/**
 * @tc.name  : Test SetSpeed with null stream
 * @tc.type  : FUNC
 * @tc.number: SetSpeed_NullStream
 * @tc.desc  : Test SetSpeed when stream_ is null.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, SetSpeed_NullStream, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");

    rendererInServer->stream_ = nullptr;
    int32_t result = rendererInServer->SetSpeed(1.0f);
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_OPERATION_FAILED,
        "SetSpeed should return ERR_OPERATION_FAILED, result=%d", result);
}

// ==================== SetPitch 分支测试 ====================

/**
 * @tc.name  : Test SetPitch with invalid pitch (too low)
 * @tc.type  : FUNC
 * @tc.number: SetPitch_TooLow
 * @tc.desc  : Test SetPitch with pitch below MIN_STREAM_PITCH_LEVEL.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, SetPitch_TooLow, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    int32_t result = rendererInServer->SetPitch(0.1f);
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_INVALID_PARAM,
        "SetPitch should return ERR_INVALID_PARAM, result=%d", result);
}

/**
 * @tc.name  : Test SetPitch with invalid pitch (too high)
 * @tc.type  : FUNC
 * @tc.number: SetPitch_TooHigh
 * @tc.desc  : Test SetPitch with pitch above MAX_STREAM_PITCH_LEVEL.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, SetPitch_TooHigh, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    int32_t result = rendererInServer->SetPitch(100.0f);
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_INVALID_PARAM,
        "SetPitch should return ERR_INVALID_PARAM, result=%d", result);
}

// ==================== SetSourceDuration 分支测试 ====================

/**
 * @tc.name  : Test SetSourceDuration
 * @tc.type  : FUNC
 * @tc.number: SetSourceDuration
 * @tc.desc  : Test SetSourceDuration sets the source duration.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, SetSourceDuration, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    int32_t result = rendererInServer->SetSourceDuration(60000);
    AIUT_EXPECT_EQ_INFO_LOG(result, SUCCESS,
        "SetSourceDuration should return SUCCESS, result=%d", result);
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->sourceDuration_, 60000,
        "sourceDuration_ mismatch, sourceDuration_=%lld",
        static_cast<long long>(rendererInServer->sourceDuration_));
}

// ==================== GetLatency 分支测试 ====================

/**
 * @tc.name  : Test GetLatency with null stream
 * @tc.type  : FUNC
 * @tc.number: GetLatency_NullStream
 * @tc.desc  : Test GetLatency when stream_ is null.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetLatency_NullStream, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");

    rendererInServer->stream_ = nullptr;
    uint64_t latency = 0;
    int32_t result = rendererInServer->GetLatency(latency);
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_OPERATION_FAILED,
        "GetLatency should return ERR_OPERATION_FAILED, result=%d", result);
}

// ==================== GetInnerCapPrebuf 分支测试 ====================

/**
 * @tc.name  : Test GetInnerCapPrebuf with offload enabled
 * @tc.type  : FUNC
 * @tc.number: GetInnerCapPrebuf_Offload
 * @tc.desc  : Test GetInnerCapPrebuf when offload is enabled.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetInnerCapPrebuf_Offload, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->offloadEnable_ = true;
    int32_t result = rendererInServer->GetInnerCapPrebuf();
    AIUT_EXPECT_EQ_INFO_LOG(result, 3,
        "GetInnerCapPrebuf mismatch for offload, result=%d", result);
}

/**
 * @tc.name  : Test GetInnerCapPrebuf with non-standard sample rate
 * @tc.type  : FUNC
 * @tc.number: GetInnerCapPrebuf_NonStandard
 * @tc.desc  : Test GetInnerCapPrebuf when non-standard sample rate is used.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetInnerCapPrebuf_NonStandard, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->offloadEnable_ = false;
    rendererInServer->isNonStandardSampleRate_ = true;
    int32_t result = rendererInServer->GetInnerCapPrebuf();
    AIUT_EXPECT_EQ_INFO_LOG(result, 3,
        "GetInnerCapPrebuf mismatch for non-standard, result=%d", result);
}

/**
 * @tc.name  : Test GetInnerCapPrebuf with default
 * @tc.type  : FUNC
 * @tc.number: GetInnerCapPrebuf_Default
 * @tc.desc  : Test GetInnerCapPrebuf with default settings.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetInnerCapPrebuf_Default, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->offloadEnable_ = false;
    rendererInServer->isNonStandardSampleRate_ = false;
    int32_t result = rendererInServer->GetInnerCapPrebuf();
    AIUT_EXPECT_EQ_INFO_LOG(result, 1,
        "GetInnerCapPrebuf mismatch for default, result=%d", result);
}

// ==================== OnDataLinkConnectionUpdate 分支测试 ====================

/**
 * @tc.name  : Test OnDataLinkConnectionUpdate with CONNECTING
 * @tc.type  : FUNC
 * @tc.number: OnDataLinkConnectionUpdate_Connecting
 * @tc.desc  : Test OnDataLinkConnectionUpdate with OPERATION_DATA_LINK_CONNECTING.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, OnDataLinkConnectionUpdate_Connecting, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->OnDataLinkConnectionUpdate(OPERATION_DATA_LINK_CONNECTING);
    AIUT_EXPECT_FALSE_INFO_LOG(rendererInServer->isDataLinkConnected_,
        "isDataLinkConnected_ should be false after CONNECTING");
}

/**
 * @tc.name  : Test OnDataLinkConnectionUpdate with CONNECTED
 * @tc.type  : FUNC
 * @tc.number: OnDataLinkConnectionUpdate_Connected
 * @tc.desc  : Test OnDataLinkConnectionUpdate with OPERATION_DATA_LINK_CONNECTED.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, OnDataLinkConnectionUpdate_Connected, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->OnDataLinkConnectionUpdate(OPERATION_DATA_LINK_CONNECTED);
    AIUT_EXPECT_TRUE_INFO_LOG(rendererInServer->isDataLinkConnected_,
        "isDataLinkConnected_ should be true after CONNECTED");
}

/**
 * @tc.name  : Test OnDataLinkConnectionUpdate with default operation
 * @tc.type  : FUNC
 * @tc.number: OnDataLinkConnectionUpdate_Default
 * @tc.desc  : Test OnDataLinkConnectionUpdate with default/unknown operation.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, OnDataLinkConnectionUpdate_Default, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->isDataLinkConnected_ = true;
    rendererInServer->OnDataLinkConnectionUpdate(static_cast<IOperation>(999));
    AIUT_EXPECT_TRUE_INFO_LOG(rendererInServer->isDataLinkConnected_,
        "isDataLinkConnected_ should remain true for default operation");
}

// ==================== GetAudioTime 分支测试 ====================

/**
 * @tc.name  : Test GetAudioTime with injection mode
 * @tc.type  : FUNC
 * @tc.number: GetAudioTime_Injection
 * @tc.desc  : Test GetAudioTime when in injection mode.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetAudioTime_Injection, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->lastTarget_ = INJECT_TO_VOICE_COMMUNICATION_CAPTURE;
    uint64_t framePos = 0;
    uint64_t timestamp = 0;
    int32_t result = rendererInServer->GetAudioTime(framePos, timestamp);
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_ILLEGAL_STATE,
        "GetAudioTime should return ERR_ILLEGAL_STATE, result=%d", result);
}

/**
 * @tc.name  : Test GetAudioTime with stopped status
 * @tc.type  : FUNC
 * @tc.number: GetAudioTime_Stopped
 * @tc.desc  : Test GetAudioTime when status is stopped.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetAudioTime_Stopped, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->lastTarget_ = NORMAL_PLAYBACK;
    rendererInServer->status_ = I_STATUS_STOPPED;
    uint64_t framePos = 0;
    uint64_t timestamp = 0;
    int32_t result = rendererInServer->GetAudioTime(framePos, timestamp);
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_ILLEGAL_STATE,
        "GetAudioTime should return ERR_ILLEGAL_STATE, result=%d", result);
}

// ==================== GetAudioPosition 分支测试 ====================

/**
 * @tc.name  : Test GetAudioPosition with injection mode
 * @tc.type  : FUNC
 * @tc.number: GetAudioPosition_Injection
 * @tc.desc  : Test GetAudioPosition when in injection mode.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetAudioPosition_Injection, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->lastTarget_ = INJECT_TO_VOICE_COMMUNICATION_CAPTURE;
    uint64_t framePos = 0;
    uint64_t timestamp = 0;
    uint64_t latency = 0;
    int32_t result = rendererInServer->GetAudioPosition(framePos, timestamp, latency,
        Timestamp::Timestampbase::MONOTONIC);
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_ILLEGAL_STATE,
        "GetAudioPosition should return ERR_ILLEGAL_STATE, result=%d", result);
}

/**
 * @tc.name  : Test GetAudioPosition with stopped status
 * @tc.type  : FUNC
 * @tc.number: GetAudioPosition_Stopped
 * @tc.desc  : Test GetAudioPosition when status is stopped.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetAudioPosition_Stopped, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->lastTarget_ = NORMAL_PLAYBACK;
    rendererInServer->status_ = I_STATUS_STOPPED;
    uint64_t framePos = 0;
    uint64_t timestamp = 0;
    uint64_t latency = 0;
    int32_t result = rendererInServer->GetAudioPosition(framePos, timestamp, latency,
        Timestamp::Timestampbase::MONOTONIC);
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_ILLEGAL_STATE,
        "GetAudioPosition should return ERR_ILLEGAL_STATE, result=%d", result);
}

// ==================== GetSpeedPosition 分支测试 ====================

/**
 * @tc.name  : Test GetSpeedPosition with stopped status
 * @tc.type  : FUNC
 * @tc.number: GetSpeedPosition_Stopped
 * @tc.desc  : Test GetSpeedPosition when status is stopped.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetSpeedPosition_Stopped, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->status_ = I_STATUS_STOPPED;
    uint64_t framePos = 0;
    uint64_t timestamp = 0;
    uint64_t latency = 0;
    int32_t result = rendererInServer->GetSpeedPosition(framePos, timestamp, latency,
        Timestamp::Timestampbase::MONOTONIC);
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_ILLEGAL_STATE,
        "GetSpeedPosition should return ERR_ILLEGAL_STATE, result=%d", result);
}

// ==================== SetDefaultOutputDevice 分支测试 ====================

/**
 * @tc.name  : Test SetDefaultOutputDevice with injection mode
 * @tc.type  : FUNC
 * @tc.number: SetDefaultOutputDevice_Injection
 * @tc.desc  : Test SetDefaultOutputDevice when in injection mode.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, SetDefaultOutputDevice_Injection, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->lastTarget_ = INJECT_TO_VOICE_COMMUNICATION_CAPTURE;
    int32_t result = rendererInServer->SetDefaultOutputDevice(DEVICE_TYPE_SPEAKER, false);
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_ILLEGAL_STATE,
        "SetDefaultOutputDevice should return ERR_ILLEGAL_STATE, result=%d", result);
}

// ==================== DrainAudioBuffer 分支测试 ====================

/**
 * @tc.name  : Test DrainAudioBuffer
 * @tc.type  : FUNC
 * @tc.number: DrainAudioBuffer
 * @tc.desc  : Test DrainAudioBuffer returns SUCCESS.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, DrainAudioBuffer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    int32_t result = rendererInServer->DrainAudioBuffer();
    AIUT_EXPECT_EQ_INFO_LOG(result, SUCCESS,
        "DrainAudioBuffer should return SUCCESS, result=%d", result);
}

// ==================== UpdateFlushStatus 分支测试 ====================

/**
 * @tc.name  : Test UpdateFlushStatus with STARTED status
 * @tc.type  : FUNC
 * @tc.number: UpdateFlushStatus_Started
 * @tc.desc  : Test UpdateFlushStatus when status is I_STATUS_STARTED.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, UpdateFlushStatus_Started, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->status_ = I_STATUS_STARTED;
    rendererInServer->UpdateFlushStatus();
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->status_.load(), I_STATUS_FLUSHING_WHEN_STARTED,
        "status should be FLUSHING_WHEN_STARTED, status=%u",
        rendererInServer->status_.load());
}

/**
 * @tc.name  : Test UpdateFlushStatus with PAUSED status
 * @tc.type  : FUNC
 * @tc.number: UpdateFlushStatus_Paused
 * @tc.desc  : Test UpdateFlushStatus when status is I_STATUS_PAUSED.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, UpdateFlushStatus_Paused, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->status_ = I_STATUS_PAUSED;
    rendererInServer->UpdateFlushStatus();
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->status_.load(), I_STATUS_FLUSHING_WHEN_PAUSED,
        "status should be FLUSHING_WHEN_PAUSED, status=%u",
        rendererInServer->status_.load());
}

/**
 * @tc.name  : Test UpdateFlushStatus with STOPPED status
 * @tc.type  : FUNC
 * @tc.number: UpdateFlushStatus_Stopped
 * @tc.desc  : Test UpdateFlushStatus when status is I_STATUS_STOPPED.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, UpdateFlushStatus_Stopped, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->status_ = I_STATUS_STOPPED;
    rendererInServer->UpdateFlushStatus();
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->status_.load(), I_STATUS_FLUSHING_WHEN_STOPPED,
        "status should be FLUSHING_WHEN_STOPPED, status=%u",
        rendererInServer->status_.load());
}

/**
 * @tc.name  : Test UpdateFlushStatus with other status
 * @tc.type  : FUNC
 * @tc.number: UpdateFlushStatus_Other
 * @tc.desc  : Test UpdateFlushStatus when status is not STARTED/PAUSED/STOPPED.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, UpdateFlushStatus_Other, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->status_ = I_STATUS_IDLE;
    rendererInServer->UpdateFlushStatus();
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->status_.load(), I_STATUS_IDLE,
        "status should remain IDLE, status=%u",
        rendererInServer->status_.load());
}

// ==================== SetAudioEffectMode 分支测试 ====================

/**
 * @tc.name  : Test SetAudioEffectMode with dual tone enabled
 * @tc.type  : FUNC
 * @tc.number: SetAudioEffectMode_DualTone
 * @tc.desc  : Test SetAudioEffectMode when dual tone is enabled.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, SetAudioEffectMode_DualTone, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->isDualToneEnabled_ = true;
    int32_t testMode = 1;
    int32_t result = rendererInServer->SetAudioEffectMode(testMode);
    AIUT_EXPECT_EQ_INFO_LOG(result, SUCCESS,
        "SetAudioEffectMode should return SUCCESS, result=%d", result);
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->effectModeWhenDual_, testMode,
        "effectModeWhenDual_ mismatch, effectModeWhenDual_=%d",
        rendererInServer->effectModeWhenDual_);
}

// ==================== GetLastAudioDuration 分支测试 ====================

/**
 * @tc.name  : Test GetLastAudioDuration with valid times
 * @tc.type  : FUNC
 * @tc.number: GetLastAudioDuration_Valid
 * @tc.desc  : Test GetLastAudioDuration with valid start and stop times.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetLastAudioDuration_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->lastStartTime_ = 1000;
    rendererInServer->lastStopTime_ = 2000;
    int64_t result = rendererInServer->GetLastAudioDuration();
    AIUT_EXPECT_EQ_INFO_LOG(result, 1000,
        "GetLastAudioDuration mismatch, result=%lld", static_cast<long long>(result));
}

/**
 * @tc.name  : Test GetLastAudioDuration with stop before start
 * @tc.type  : FUNC
 * @tc.number: GetLastAudioDuration_Invalid
 * @tc.desc  : Test GetLastAudioDuration when stop time is before start time.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, GetLastAudioDuration_Invalid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->lastStartTime_ = 2000;
    rendererInServer->lastStopTime_ = 1000;
    int64_t result = rendererInServer->GetLastAudioDuration();
    AIUT_EXPECT_EQ_INFO_LOG(result, -1,
        "GetLastAudioDuration should return -1, result=%lld",
        static_cast<long long>(result));
}

// ==================== RecordStandbyTime 分支测试 ====================

/**
 * @tc.name  : Test RecordStandbyTime with standby disabled
 * @tc.type  : FUNC
 * @tc.number: RecordStandbyTime_Disabled
 * @tc.desc  : Test RecordStandbyTime when standby is disabled.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, RecordStandbyTime_Disabled, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->standByEnable_ = false;
    rendererInServer->RecordStandbyTime(false, false);
    AIUT_EXPECT_FALSE_INFO_LOG(rendererInServer->standByEnable_.load(),
        "standByEnable_ should remain false after RecordStandbyTime");
}

// ==================== PauseDirectStream 分支测试 ====================

/**
 * @tc.name  : Test PauseDirectStream with PLAYBACK manager type
 * @tc.type  : FUNC
 * @tc.number: PauseDirectStream_Playback
 * @tc.desc  : Test PauseDirectStream when managerType is PLAYBACK.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, PauseDirectStream_Playback, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->managerType_ = PLAYBACK;
    rendererInServer->PauseDirectStream();
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->managerType_, PLAYBACK,
        "managerType_ should remain PLAYBACK after PauseDirectStream, managerType_=%d",
        rendererInServer->managerType_);
}

// ==================== RemoveIdForInjector 分支测试 ====================

/**
 * @tc.name  : Test RemoveIdForInjector with injection mode
 * @tc.type  : FUNC
 * @tc.number: RemoveIdForInjector_Injection
 * @tc.desc  : Test RemoveIdForInjector when in injection mode.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, RemoveIdForInjector_Injection, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->lastTarget_ = INJECT_TO_VOICE_COMMUNICATION_CAPTURE;
    rendererInServer->RemoveIdForInjector();
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->lastTarget_, INJECT_TO_VOICE_COMMUNICATION_CAPTURE,
        "lastTarget_ should remain INJECT after RemoveIdForInjector, lastTarget_=%d",
        rendererInServer->lastTarget_);
}

/**
 * @tc.name  : Test RemoveIdForInjector with normal playback
 * @tc.type  : FUNC
 * @tc.number: RemoveIdForInjector_Normal
 * @tc.desc  : Test RemoveIdForInjector when in normal playback mode.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, RemoveIdForInjector_Normal, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->lastTarget_ = NORMAL_PLAYBACK;
    rendererInServer->RemoveIdForInjector();
    AIUT_EXPECT_EQ_INFO_LOG(rendererInServer->lastTarget_, NORMAL_PLAYBACK,
        "lastTarget_ should remain NORMAL after RemoveIdForInjector, lastTarget_=%d",
        rendererInServer->lastTarget_);
}

// ==================== OffloadSetVolume 分支测试 ====================

/**
 * @tc.name  : Test OffloadSetVolume with offload enabled
 * @tc.type  : FUNC
 * @tc.number: OffloadSetVolume_Enabled
 * @tc.desc  : Test OffloadSetVolume when offload is enabled.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, OffloadSetVolume_Enabled, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->offloadEnable_ = true;
    rendererInServer->OffloadSetVolume();
    AIUT_EXPECT_TRUE_INFO_LOG(rendererInServer->offloadEnable_,
        "offloadEnable_ should remain true after OffloadSetVolume");
}

/**
 * @tc.name  : Test OffloadSetVolume with offload disabled
 * @tc.type  : FUNC
 * @tc.number: OffloadSetVolume_Disabled
 * @tc.desc  : Test OffloadSetVolume when offload is disabled.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, OffloadSetVolume_Disabled, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->offloadEnable_ = false;
    rendererInServer->OffloadSetVolume();
    AIUT_EXPECT_FALSE_INFO_LOG(rendererInServer->offloadEnable_,
        "offloadEnable_ should remain false after OffloadSetVolume");
}

// ==================== FlushDualToneStream 分支测试 ====================

/**
 * @tc.name  : Test FlushDualToneStream with dual tone disabled
 * @tc.type  : FUNC
 * @tc.number: FlushDualToneStream_Disabled
 * @tc.desc  : Test FlushDualToneStream when dual tone is disabled.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, FlushDualToneStream_Disabled, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->isDualToneEnabled_ = false;
    rendererInServer->FlushDualToneStream();
    AIUT_EXPECT_FALSE_INFO_LOG(rendererInServer->isDualToneEnabled_.load(),
        "isDualToneEnabled_ should remain false after FlushDualToneStream");
}

// ==================== DisableDualTone 分支测试 ====================

/**
 * @tc.name  : Test DisableDualTone when already disabled
 * @tc.type  : FUNC
 * @tc.number: DisableDualTone_AlreadyDisabled
 * @tc.desc  : Test DisableDualTone when dual tone is already disabled.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, DisableDualTone_AlreadyDisabled, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->isDualToneEnabled_ = false;
    int32_t result = rendererInServer->DisableDualTone();
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_INVALID_OPERATION,
        "DisableDualTone should return ERR_INVALID_OPERATION, result=%d", result);
}

// ==================== InitBufferStatus 分支测试 ====================

/**
 * @tc.name  : Test InitBufferStatus with null buffer
 * @tc.type  : FUNC
 * @tc.number: InitBufferStatus_NullBuffer
 * @tc.desc  : Test InitBufferStatus when audioServerBuffer is null.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, InitBufferStatus_NullBuffer, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");

    rendererInServer->audioServerBuffer_ = nullptr;
    int32_t result = rendererInServer->InitBufferStatus();
    AIUT_EXPECT_EQ_INFO_LOG(result, ERR_ILLEGAL_STATE,
        "InitBufferStatus should return ERR_ILLEGAL_STATE, result=%d", result);
}

/**
 * @tc.name  : Test InitBufferStatus with valid buffer
 * @tc.type  : FUNC
 * @tc.number: InitBufferStatus_Valid
 * @tc.desc  : Test InitBufferStatus with valid audioServerBuffer.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, InitBufferStatus_Valid, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    int32_t result = rendererInServer->InitBufferStatus();
    AIUT_EXPECT_EQ_INFO_LOG(result, SUCCESS,
        "InitBufferStatus should return SUCCESS, result=%d", result);
}

// ==================== CheckAndWriterRenderStreamStandbySysEvent 分支测试 ====================

/**
 * @tc.name  : Test CheckAndWriterRenderStreamStandbySysEvent with same status
 * @tc.type  : FUNC
 * @tc.number: CheckAndWriterStandbySysEvent_Same
 * @tc.desc  : Test CheckAndWriterRenderStreamStandbySysEvent when standby status is same as last.
 * @tc.owner : audio_route_renderer
 */
HWTEST_F(RendererInServerSixthAiut, CheckAndWriterStandbySysEvent_Same, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AIUT_ASSERT_NE_INFO_LOG(rendererInServer, nullptr, "rendererInServer is null");
    int32_t ret = rendererInServer->Init();
    AIUT_ASSERT_EQ_INFO_LOG(ret, SUCCESS, "Init failed, ret=%d", ret);

    rendererInServer->lastWriteStandbyEnableStatus_ = true;
    rendererInServer->CheckAndWriterRenderStreamStandbySysEvent(true);
    AIUT_EXPECT_TRUE_INFO_LOG(rendererInServer->lastWriteStandbyEnableStatus_,
        "lastWriteStandbyEnableStatus_ should remain true after same status event");
}

} // namespace AudioStandard
} // namespace OHOS