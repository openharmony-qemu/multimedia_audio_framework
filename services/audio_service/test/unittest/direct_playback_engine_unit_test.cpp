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

#include <gtest/gtest.h>
#include "direct_playback_engine.h"
#include "sink/i_audio_render_sink.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "pro_renderer_stream_impl.h"
#include "audio_errors.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
class DirectPlayBackEngineUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    AudioProcessConfig InitProcessConfig();

protected:
    std::unique_ptr<AudioPlaybackEngine> playbackEngine_;
};

void DirectPlayBackEngineUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void DirectPlayBackEngineUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void DirectPlayBackEngineUnitTest::SetUp(void)
{
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    playbackEngine_ = std::make_unique<DirectPlayBackEngine>();
    if (playbackEngine_) {
        playbackEngine_->Init(deviceInfo, false);
    }
}

void DirectPlayBackEngineUnitTest::TearDown(void)
{
    if (playbackEngine_) {
        playbackEngine_->Stop();
        playbackEngine_ = nullptr;
    }
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_001
 * @tc.desc  : Test DirectPlayBackEngine::Init().
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_001, TestSize.Level2)
{
    DirectPlayBackEngine engine;
    AudioDeviceDescriptor deviceDesc(AudioDeviceDescriptor::DEVICE_INFO);
    bool isVoip = true;
    engine.isInit_ = true;
    deviceDesc.deviceType_ = DEVICE_TYPE_INVALID;
    engine.device_.deviceType_ = DEVICE_TYPE_NONE;
    engine.renderId_ = HDI_INVALID_ID;

    auto ret = engine.Init(deviceDesc, isVoip);
    EXPECT_EQ(ret, SUCCESS);

    engine.isInit_ = false;
    ret = engine.Init(deviceDesc, isVoip);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_002
 * @tc.desc  : Test DirectPlayBackEngine::Init().
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_002, TestSize.Level2)
{
    DirectPlayBackEngine engine;
    AudioDeviceDescriptor deviceDesc(AudioDeviceDescriptor::DEVICE_INFO);
    bool isVoip = true;
    engine.isInit_ = true;
    deviceDesc.deviceType_ = DEVICE_TYPE_INVALID;
    engine.device_.deviceType_ = DEVICE_TYPE_INVALID;
    engine.renderId_ = HDI_INVALID_ID;

    auto ret = engine.Init(deviceDesc, isVoip);
    EXPECT_EQ(ret, SUCCESS);

    deviceDesc.deviceType_ = DEVICE_TYPE_NONE;
    ret = engine.Init(deviceDesc, isVoip);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_003
 * @tc.desc  : Test DirectPlayBackEngine::AddRenderer().
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_003, TestSize.Level2)
{
    DirectPlayBackEngine engine;
    engine.isStart_ = true;
    uint32_t idx1 = 0;
    uint32_t idx2 = 1;
    AudioProcessConfig config;
    std::shared_ptr<ProRendererStreamImpl> rendererStream1 = std::make_shared<ProRendererStreamImpl>(config, true);
    if (rendererStream1) {
        rendererStream1->SetStreamIndex(idx1);
    }
    engine.stream_ = rendererStream1;
    auto ret = engine.AddRenderer(rendererStream1);
    EXPECT_EQ(ret, SUCCESS);

    std::shared_ptr<ProRendererStreamImpl> rendererStream2 = std::make_shared<ProRendererStreamImpl>(config, true);
    if (rendererStream2) {
        rendererStream2->SetStreamIndex(idx2);
    }
    ret = engine.AddRenderer(rendererStream2);
    EXPECT_EQ(ret, ERROR_UNSUPPORTED);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_004
 * @tc.desc  : Test DirectPlayBackEngine::RemoveRenderer().
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_004, TestSize.Level2)
{
    DirectPlayBackEngine engine;
    engine.isStart_ = true;
    uint32_t idx1 = 0;
    uint32_t idx2 = 1;
    AudioProcessConfig config;
    std::shared_ptr<ProRendererStreamImpl> rendererStream1 = std::make_shared<ProRendererStreamImpl>(config, true);
    if (rendererStream1) {
        rendererStream1->SetStreamIndex(idx1);
    }
    engine.stream_ = nullptr;
    engine.RemoveRenderer(rendererStream1);
    
    engine.stream_ = rendererStream1;
    engine.RemoveRenderer(rendererStream1);
    EXPECT_EQ(engine.stream_, nullptr);

    std::shared_ptr<ProRendererStreamImpl> rendererStream2 = std::make_shared<ProRendererStreamImpl>(config, true);
    if (rendererStream2) {
        rendererStream2->SetStreamIndex(idx2);
    }
    engine.stream_ = rendererStream1;
    EXPECT_NE(engine.stream_->GetStreamIndex(), rendererStream2->GetStreamIndex());
    engine.RemoveRenderer(rendererStream2);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_005
 * @tc.desc  : Test DirectPlayBackEngine::MixStreams()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_005, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    EXPECT_NE(enginePtr, nullptr);

    AudioProcessConfig configRet;
    enginePtr->stream_ = std::make_shared<ProRendererStreamImpl>(configRet, true);
    EXPECT_NE(enginePtr->stream_, nullptr);
    enginePtr->MixStreams();

    enginePtr->stream_ = nullptr;
    enginePtr->MixStreams();
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_006
 * @tc.desc  : Test DirectPlayBackEngine::MixStreams()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_006, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    EXPECT_NE(enginePtr, nullptr);

    AudioProcessConfig config;
    enginePtr->stream_ = std::make_shared<ProRendererStreamImpl>(config, true);
    EXPECT_NE(enginePtr->stream_, nullptr);
    enginePtr->failedCount_.store(10);
    enginePtr->MixStreams();

    enginePtr->failedCount_.store(100);
    enginePtr->MixStreams();
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_007
 * @tc.desc  : Test DirectPlayBackEngine::GetDirectFormatByteSize().
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_007, TestSize.Level2)
{
    DirectPlayBackEngine engine;
    AudioSampleFormat format = SAMPLE_S16LE;
    auto ret = engine.GetDirectFormatByteSize(format);
    EXPECT_EQ(ret, sizeof(int16_t));

    format = SAMPLE_S32LE;
    ret = engine.GetDirectFormatByteSize(format);
    EXPECT_EQ(ret, sizeof(int32_t));

    format = SAMPLE_F32LE;
    ret = engine.GetDirectFormatByteSize(format);
    EXPECT_EQ(ret, sizeof(int32_t));

    format = INVALID_WIDTH;
    ret = engine.GetDirectFormatByteSize(format);
    EXPECT_EQ(ret, sizeof(int32_t));
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_008
 * @tc.desc  : Test DirectPlayBackEngine::InitSink()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_008, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    EXPECT_NE(enginePtr, nullptr);

    AudioStreamInfo streamInfo;
    enginePtr->isInit_ = false;
    enginePtr->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_EAC3,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(enginePtr->renderId_, HDI_INVALID_ID);

    auto ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);
}


/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_009
 * @tc.desc  : Test DirectPlayBackEngine::InitSink()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_009, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    EXPECT_NE(enginePtr, nullptr);

    AudioStreamInfo streamInfo;
    enginePtr->isInit_ = true;
    enginePtr->renderId_ = HDI_INVALID_ID;

    auto ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_010
 * @tc.desc  : Test DirectPlayBackEngine::InitSink()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_010, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    EXPECT_NE(enginePtr, nullptr);

    AudioStreamInfo streamInfo;
    streamInfo.channels = AudioChannel::STEREO;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;
    enginePtr->isInit_ = true;
    enginePtr->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_EAC3,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(enginePtr->renderId_, HDI_INVALID_ID);
    auto ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);

    enginePtr->uChannel_ = AudioChannel::CHANNEL_UNKNOW;
    ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);

    enginePtr->uChannel_ = AudioChannel::STEREO;
    ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);

    enginePtr->format_ = SAMPLE_S16LE;
    ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);

    enginePtr->format_ = SAMPLE_S32LE;
    ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);

    enginePtr->uSampleRate_ = AudioSamplingRate::SAMPLE_RATE_32000;
    ret = enginePtr->InitSink(streamInfo);
    EXPECT_NE(ret, SUCCESS);

    enginePtr->uSampleRate_ = AudioSamplingRate::SAMPLE_RATE_48000;
    ret = enginePtr->InitSink(streamInfo);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_011
 * @tc.desc  : Test DirectPlayBackEngine::StopAudioSink()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_011, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    enginePtr->renderId_ = -1;
    auto ret = enginePtr->StopAudioSink();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_012
 * @tc.desc  : Test DirectPlayBackEngine::StopAudioSink()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_012, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    enginePtr->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_EAC3,
        HDI_ID_INFO_DEFAULT, true);
    auto sink = HdiAdapterManager::GetInstance().GetRenderSink(enginePtr->renderId_, true);
    ASSERT_TRUE(sink != nullptr);

    auto ret = enginePtr->StopAudioSink();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_013
 * @tc.desc  : Test DirectPlayBackEngine::Start()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_013, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    EXPECT_NE(enginePtr, nullptr);

    enginePtr->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_EAC3,
        HDI_ID_INFO_DEFAULT, true);
    EXPECT_NE(enginePtr->renderId_, HDI_INVALID_ID);
    
    enginePtr->isStart_ = true;
    auto ret = enginePtr->Start();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);

    enginePtr->isStart_ = false;
    ret = enginePtr->Start();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_014
 * @tc.desc  : Test DirectPlayBackEngine::Stop()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_014, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    enginePtr->isStart_ = true;
    auto ret = enginePtr->Stop();
    EXPECT_EQ(ret, SUCCESS);

    enginePtr->isStart_ = false;
    ret = enginePtr->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_015
 * @tc.desc  : Test DirectPlayBackEngine::Pause()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_015, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);

    enginePtr->isStart_ = true;
    auto ret = enginePtr->Pause();
    EXPECT_EQ(ret, SUCCESS);

    enginePtr->isStart_ = false;
    ret = enginePtr->Pause();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DirectPlayBackEngine API
 * @tc.type  : FUNC
 * @tc.number: DirectPlayBackEngine_016
 * @tc.desc  : Test DirectPlayBackEngine::DirectCallback()
 */
HWTEST_F(DirectPlayBackEngineUnitTest, DirectPlayBackEngine_016, TestSize.Level2)
{
    auto enginePtr = std::make_shared<DirectPlayBackEngine>();
    ASSERT_TRUE(enginePtr != nullptr);
    enginePtr->DirectCallback(CB_NONBLOCK_WRITE_COMPLETED);
    enginePtr->DirectCallback(CB_DRAIN_COMPLETED);
    enginePtr->DirectCallback(CB_FLUSH_COMPLETED);
    enginePtr->DirectCallback(CB_RENDER_FULL);
    enginePtr->DirectCallback(CB_ERROR_OCCUR);
}
} // namespace AudioStandard
} // namespace OHOS
