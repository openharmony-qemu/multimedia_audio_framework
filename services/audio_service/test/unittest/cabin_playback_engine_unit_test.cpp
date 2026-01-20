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

#include <gtest/gtest.h>
#include "audio_errors.h"
#include "audio_utils.h"
#include "sink/i_audio_render_sink.h"
#include "cabin_playback_engine.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "pro_renderer_stream_impl.h"
#include  "parameter.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
    constexpr int32_t DEFAULT_STREAM_ID = 10;
class CabinPlayBackEngineUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

protected:
    AudioProcessConfig InitProcessConfig();

protected:
    std::unique_ptr<CabinPlayBackEngine> playbackEngine_;
};

void CabinPlayBackEngineUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void CabinPlayBackEngineUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void CabinPlayBackEngineUnitTest::SetUp(void)
{
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    deviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    playbackEngine_ = std::make_unique<CabinPlayBackEngine>();
    if (playbackEngine_) {
        playbackEngine_->Init(deviceInfo, false);
    }
}

void CabinPlayBackEngineUnitTest::TearDown(void)
{
    if (playbackEngine_) {
        playbackEngine_->Stop();
        playbackEngine_ = nullptr;
    }
}

AudioProcessConfig CabinPlayBackEngineUnitTest::InitProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = CHANNEL_6;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_5POINT1POINT2;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_SPEAKER;
    return config;
}

/**
 * @tc.name  : Test 3da direct audio playback engine state
 * @tc.type  : FUNC
 * @tc.number: CabinAudioPlayBackEngineState_001
 * @tc.desc  : Test 3da direct audio playback engine state
 */

HWTEST_F(CabinPlayBackEngineUnitTest, CabinAudioPlayBackEngineState_001, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    auto rendererStream = std::make_shared<ProRendererStreamImpl>(config, false);
    rendererStream->InitParams();
    rendererStream->SetStreamIndex(DEFAULT_STREAM_ID);

    int32_t ret = playbackEngine_->AddRenderer(rendererStream);
    EXPECT_EQ(SUCCESS, ret);
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);

    AudioDeviceDescriptor deviceDesc(AudioDeviceDescriptor::DEVICE_INFO);
    deviceDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    ret = playbackEngine_->Init(deviceDesc, false);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_FALSE(playbackEngine_->isVoip_);
    EXPECT_EQ(playbackEngine_->device_.deviceType_, DEVICE_TYPE_SPEAKER);

    ret = playbackEngine_->Start();
    EXPECT_NE(SUCCESS, ret);
    EXPECT_FALSE(playbackEngine_->isStart_);

    ret = playbackEngine_->Pause(false);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_FALSE(playbackEngine_->isStart_);

    ret = playbackEngine_->Stop();
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(playbackEngine_->writeCount_, 0);
}

 /**
 * @tc.name  : Test CabinPlayabackEngine state roubustness
 * @tc.type  : FUNC
 * @tc.number: CabinAudioPlayBackEngineState_002
 * @tc.desc  : feedback on the status when 3da playback engine init repeatedly or performs invalid operations
 */

HWTEST_F(CabinPlayBackEngineUnitTest, CabinAudioPlayBackEngineState_002, TestSize.Level1)
{
    AudioDeviceDescriptor deviceDesc(AudioDeviceDescriptor::DEVICE_INFO);
    deviceDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    EXPECT_FALSE(playbackEngine_->isInit_);

    int32_t ret = playbackEngine_->Init(deviceDesc, false);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_FALSE(playbackEngine_->isInit_);

    ret = playbackEngine_->Init(deviceDesc, false);
    EXPECT_EQ(SUCCESS, ret);
    AudioDeviceDescriptor newDevice(AudioDeviceDescriptor::DEVICE_INFO);
    newDevice.deviceType_ = DEVICE_TYPE_USB_HEADSET;
    ret = playbackEngine_->Init(newDevice, false);
    EXPECT_EQ(SUCCESS, ret);
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, 
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);

    playbackEngine_->Start();
    EXPECT_FALSE(playbackEngine_->isStart_);
    ret = playbackEngine_->Start();
    EXPECT_NE(SUCCESS, ret);

    ret = playbackEngine_->Stop();
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_FALSE(playbackEngine_->isStart_);

    ret = playbackEngine_->Stop();
    EXPECT_EQ(SUCCESS, ret);
}

 /**
 * @tc.name  : Test CabinPlayabackEngine lifeCycle Robustness
 * @tc.type  : FUNC
 * @tc.number: CabinAudioPlayBackEngineState_003
 * @tc.desc  : Test state consistency after stop/destruction and verify invalid state transitions
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngineState_003, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    auto rendererStream = std::make_shared<ProRendererStreamImpl>(config, false);
    rendererStream->InitParams();
    playbackEngine_->AddRenderer(rendererStream);
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, 
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    int32_t ret = playbackEngine_->Start();
    EXPECT_NE(SUCCESS, ret);
    EXPECT_FALSE(playbackEngine_->isStart_);
    ret = playbackEngine_->Stop();
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_FALSE(playbackEngine_->isStart_);
    playbackEngine_->RemoveRenderer(rendererStream);
    EXPECT_TRUE(playbackEngine_->stream_ == nullptr);
    ret = playbackEngine_->Start();
    if (ret == SUCCESS) {
        playbackEngine_->PollAndWrite();
        EXPECT_EQ(playbackEngine_->writeCount_, 0);
    }
    ret = playbackEngine_->Stop();
    EXPECT_EQ(SUCCESS, ret);
}

 /**
 * @tc.name  : Test CabinPlayabackEngineState_004
 * @tc.type  : FUNC
 * @tc.number: CabinAudioPlayBackEngineState_004
 * @tc.desc  : Test Pause and Resume flow logic of CabinPlayBackEngine
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngineState_004, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    auto rendererStream = std::make_shared<ProRendererStreamImpl>(config, false);
    rendererStream->InitParams();
    playbackEngine_->AddRenderer(rendererStream);
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    int32_t ret = playbackEngine_->Start();
    EXPECT_NE(SUCCESS, ret);
    EXPECT_FALSE(playbackEngine_->isStart_);

    ret = playbackEngine_->Pause(false);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_FALSE(playbackEngine_->isStart_);

    playbackEngine_->PollAndWrite();

    ret = playbackEngine_->Start();
    EXPECT_FALSE(ret);
    EXPECT_FALSE(playbackEngine_->isStart_);

    ret = playbackEngine_->Stop();
    EXPECT_EQ(SUCCESS, ret);
    playbackEngine_->RemoveRenderer(rendererStream);
}

 /**
 * @tc.name  : Test CabinPlayabackEngineState_005_FlushLogic
 * @tc.type  : FUNC
 * @tc.number: CabinAudioPlayBackEngineState_005
 * @tc.desc  : Verify if Flush correctly resets write count and synchronization timestamps
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngineState_005, TestSize.Level1)
{
    AudioDeviceDescriptor deviceDesc(AudioDeviceDescriptor::DEVICE_INFO);
    deviceDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    playbackEngine_->Init(deviceDesc, false);
    
    playbackEngine_->fwkSyncTime_ = 1000000000ULL; // 假定一个旧的起始时间
    playbackEngine_->writeCount_ = 100;

    uint64_t beforeFlushNano = static_cast<uint64_t>(ClockTime::GetCurNano());
    int32_t ret = playbackEngine_->Flush();
    uint64_t afterFlushNano = static_cast<uint64_t>(ClockTime::GetCurNano());

    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(playbackEngine_->writeCount_, 0);
    EXPECT_GE(playbackEngine_->fwkSyncTime_, beforeFlushNano);
    EXPECT_LE(playbackEngine_->fwkSyncTime_, afterFlushNano);

    int64_t nextSleepTime = static_cast<int64_t>(playbackEngine_->fwkSyncTime_) + 2000000LL; // 20ms
    
    EXPECT_GT(nextSleepTime, static_cast<int64_t>(beforeFlushNano));
}

 /**
 * @tc.name  : CabinPlayabackEngineState_006
 * @tc.type  : FUNC
 * @tc.number: CabinAudioPlayBackEngineState_006
 * @tc.desc  : Test the cleanup logic after removing the renderer stream
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngineState_006, TestSize.Level1)
{
    AudioProcessConfig config = InitProcessConfig();
    auto rendererStream = std::make_shared<ProRendererStreamImpl>(config, true);
    rendererStream->InitParams();
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    
    playbackEngine_->AddRenderer(rendererStream);

    playbackEngine_->Start();
    EXPECT_TRUE(playbackEngine_->isStart_);

    int32_t ret = rendererStream->Drain();
    EXPECT_EQ(SUCCESS, ret);

    playbackEngine_->RemoveRenderer(rendererStream);
    EXPECT_EQ(playbackEngine_->stream_, nullptr);

    playbackEngine_->Stop();
    EXPECT_FALSE(playbackEngine_->isStart_);
}

 /**
 * @tc.name  : CabinPlayabackEngine_Init_002
 * @tc.type  : FUNC
 * @tc.number: CabinAudioPlayBackEngine_Init_002
 * @tc.desc  : Verify Init function behavior when no renderer stream is present(invalid renderId)
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_Init_002, TestSize.Level1)
{
    playbackEngine_->renderId_ = HDI_INVALID_ID;
    playbackEngine_->isInit_ = false;

    AudioDeviceDescriptor deviceDesc(AudioDeviceDescriptor::DEVICE_INFO);
    deviceDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    int32_t ret = playbackEngine_->Init(deviceDesc, false);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(playbackEngine_->device_.deviceType_, DEVICE_TYPE_SPEAKER);
    
    EXPECT_EQ(playbackEngine_->renderId_, HDI_INVALID_ID);

    auto sink = HdiAdapterManager::GetInstance().GetRenderSink(playbackEngine_->renderId_);
    EXPECT_EQ(sink, nullptr);
}

 /**
 * @tc.name  : CabinPlayabackEngine_Stop_003
 * @tc.type  : FUNC
 * @tc.number: CabinAudioPlayBackEngine_Stop_003
 * @tc.desc  : Test the safety of the stop logic in idle or initial states
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_Stop_003, TestSize.Level1)
{
    EXPECT_FALSE(playbackEngine_->isStart_);
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    int32_t ret = playbackEngine_->Stop();
    EXPECT_EQ(ret, SUCCESS);

    ret = playbackEngine_->StopAudioSink();
    EXPECT_EQ(ret, SUCCESS);

    AudioDeviceDescriptor deviceDesc(AudioDeviceDescriptor::DEVICE_INFO);
    deviceDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    playbackEngine_->Init(deviceDesc, false);
    
    ret = playbackEngine_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_FALSE(playbackEngine_->isStart_);
    EXPECT_EQ(playbackEngine_->writeCount_, 0);
}

 /**
 * @tc.name  : CabinPlayabackEngine_Pause_004
 * @tc.type  : FUNC
 * @tc.number: CabinAudioPlayBackEngine_Pause_004
 * @tc.desc  : Test Pause logic and state reset behavior of CabinPlayEngine
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_Pause_004, TestSize.Level1)
{
    playbackEngine_->renderId_ = 101;
    AudioDeviceDescriptor deviceDesc(AudioDeviceDescriptor::DEVICE_INFO);
    deviceDesc.deviceType_ = DEVICE_TYPE_SPEAKER;
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    playbackEngine_->Init(deviceDesc, false);
    playbackEngine_->Start();
    
    playbackEngine_->writeCount_ = 100;
    EXPECT_FALSE(playbackEngine_->isStart_);

    int32_t ret = playbackEngine_->Pause(true);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(playbackEngine_->writeCount_, 100);
    EXPECT_FALSE(playbackEngine_->isStart_);

    ret = playbackEngine_->Pause(false);
    EXPECT_EQ(ret, SUCCESS);

    playbackEngine_->isStart_ = true;
    playbackEngine_->playbackThread_ = nullptr;
    
    ret = playbackEngine_->Pause(true);
    EXPECT_EQ(ret, SUCCESS);
}

 /**
 * @tc.name  : CabinPlayabackEngine_Addrender_005
 * @tc.type  : FUNC
 * @tc.number: CabinPlayabackEngine_Addrender_005
 * @tc.desc  : Test AddRender stream management and Sink initialization trigger
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_AddRenderer_005, TestSize.Level1)
{
    int32_t ret = playbackEngine_->AddRenderer(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);

    AudioProcessConfig config1;
    auto stream1 = std::make_shared<ProRendererStreamImpl>(config1, false);
    stream1->SetStreamIndex(10);
    ret = playbackEngine_->AddRenderer(stream1);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(playbackEngine_->stream_, stream1);
    EXPECT_TRUE(playbackEngine_->isInit_);

    ret = playbackEngine_->AddRenderer(stream1);
    EXPECT_EQ(ret, SUCCESS);

    auto stream2 = std::make_shared<ProRendererStreamImpl>(config1, true);
    stream2->SetStreamIndex(20);
    ret = playbackEngine_->AddRenderer(stream2);
    EXPECT_EQ(ret, ERROR_UNSUPPORTED);
}

 /**
 * @tc.name  : CabinPlayabackEngine_RemoveRenderer_006
 * @tc.type  : FUNC
 * @tc.number: CabinPlayabackEngine_RemoveRenderer_006
 * @tc.desc  : Test RemoveRenderer logic and its linkage with engine stop state
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_RemoveRenderer_006, TestSize.Level1)
{
    AudioProcessConfig config;
    auto stream1 = std::make_shared<ProRendererStreamImpl>(config, false);
    stream1->SetStreamIndex(101);
    
    playbackEngine_->AddRenderer(stream1);
    playbackEngine_->Start();
    EXPECT_TRUE(playbackEngine_->isStart_);
    EXPECT_EQ(playbackEngine_->stream_, stream1);

    playbackEngine_->RemoveRenderer(nullptr);
    EXPECT_EQ(playbackEngine_->stream_, stream1);

    auto stream2 = std::make_shared<ProRendererStreamImpl>(config, false);
    stream2->SetStreamIndex(999); // 999 for index
    playbackEngine_->RemoveRenderer(stream2);
    EXPECT_EQ(playbackEngine_->stream_, stream1);
    EXPECT_TRUE(playbackEngine_->isStart_);
   
    playbackEngine_->RemoveRenderer(stream1);
    EXPECT_EQ(playbackEngine_->stream_, nullptr);
    EXPECT_FALSE(playbackEngine_->isStart_);

    playbackEngine_->RemoveRenderer(stream1);
    EXPECT_EQ(playbackEngine_->stream_, nullptr);
}

 /**
 * @tc.name  : CabinPlayabackEngine_Start_007
 * @tc.type  : FUNC
 * @tc.number: CabinPlayabackEngine_Start_007
 * @tc.desc  : Test Start logic including Sink validation, thread creatiion, and timestamp initialization
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_Start_007, TestSize.Level1)
{
    AudioProcessConfig config;
    auto stream = std::make_shared<ProRendererStreamImpl>(config, false);
    stream->SetStreamIndex(101);
    playbackEngine_->AddRenderer(stream);
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    int32_t ret = playbackEngine_->Start();
    EXPECT_NE(ret, SUCCESS);
    EXPECT_FALSE(playbackEngine_->isStart_);
    EXPECT_EQ(playbackEngine_->writeCount_, 0);
    if (playbackEngine_->playbackThread_) {
        EXPECT_TRUE(playbackEngine_->playbackThread_->CheckThreadIsRunning());
    }
}

 /**
 * @tc.name  : CabinPlayabackEngine_PollAndWrite_008
 * @tc.type  : FUNC
 * @tc.number: CabinPlayabackEngine_PollAndWrite_008
 * @tc.desc  : Test a single cycle of PollAndWrite, covering null stream handling and normal write increments
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_PollAndWrite_008, TestSize.Level1)
{
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    playbackEngine_->stream_ = nullptr;
    playbackEngine_->PollAndWrite();
    EXPECT_EQ(playbackEngine_->writeCount_, 0);

    AudioProcessConfig config;
    config.appInfo.appUid = 10001;
    config.streamInfo.channels = STEREO;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.encoding = ENCODING_AUDIOVIVID;
    
    auto stream = std::make_shared<ProRendererStreamImpl>(config, false);
    
    stream->InitParams(); // 内部会初始化 sinkBuffer_ 和 writeQueue_
    stream->SetStreamIndex(101);

    BufferDesc buffer;
    buffer.bufLength = 1024 * 2 * 2 + 19824;
    buffer.buffer = new uint8_t[buffer.bufLength];
    stream->EnqueueBuffer(buffer);

    playbackEngine_->AddRenderer(stream);

    uint32_t initialWriteCount = playbackEngine_->writeCount_;
    
    playbackEngine_->PollAndWrite();
    EXPECT_EQ(playbackEngine_->writeCount_, initialWriteCount);

    delete[] buffer.buffer;
}

/**
 * @tc.name  : CabinPlayabackEngine_PollAndWrite_009
 * @tc.type  : FUNC
 * @tc.number: CabinPlayabackEngine_PollAndWrite_009
 * @tc.desc  : Test a single cycle of PollAndWrite, covering null stream handling and normal write increments
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_PollAndWrite_009, TestSize.Level1)
{
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    playbackEngine_->stream_ = nullptr;
    playbackEngine_->PollAndWrite();
    EXPECT_EQ(playbackEngine_->writeCount_, 0);

    AudioProcessConfig config;
    config.appInfo.appUid = 10001;
    config.streamInfo.channels = STEREO;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.format = SAMPLE_S16LE;
    config.streamInfo.encoding = ENCODING_AUDIOVIVID;
    
    auto stream = std::make_shared<ProRendererStreamImpl>(config, false);
    stream->InitParams(); // 内部会初始化 sinkBuffer_ 和 writeQueue_
    stream->SetStreamIndex(101);

    int32_t writeIndex = stream->PopWriteBufferIndex();
    ASSERT_GE(writeIndex, 0);

    uint32_t dataLen = 1024*2*2+19824;
    stream->sinkBuffer_[writeIndex].resize(dataLen);
    uint8_t *testData = new uint8_t[dataLen];
    memset_s(testData, dataLen, 0xAA, dataLen);
    memcpy_s(stream->sinkBuffer_[writeIndex].data(), dataLen, testData, dataLen);

    BufferDesc buffer;
    buffer.buffer = testData;
    buffer.bufLength = dataLen;
    stream->EnqueueBuffer(buffer);

    playbackEngine_->AddRenderer(stream);

    uint32_t initialWriteCount = playbackEngine_->writeCount_;

    playbackEngine_->PollAndWrite();

    EXPECT_EQ(playbackEngine_->writeCount_, initialWriteCount);

    delete[] testData;
}

 /**
 * @tc.name  : Test CabinPlayabackEngine API
 * @tc.type  : FUNC
 * @tc.number: CabinPlayabackEngine_009
 * @tc.desc  : Test CabinPlayBackEngine::InitSink
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_009, TestSize.Level1)
{
    AudioStreamInfo streamInfo;
    streamInfo.channels = CHANNEL_8;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S16LE;

    playbackEngine_->isInit_ = true;
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    playbackEngine_->uChannel_ = CHANNEL_8;
    playbackEngine_->uSampleRate_ = SAMPLE_RATE_48000;
    playbackEngine_->uformat_ = SAMPLE_S16LE;

    auto ret = playbackEngine_->InitSink(streamInfo);
    EXPECT_EQ(ret, SUCCESS);
}

 /**
 * @tc.name  : Test CabinPlayabackEngine API
 * @tc.type  : FUNC
 * @tc.number: CabinPlayabackEngine_010
 * @tc.desc  : Test CabinPlayBackEngine::InitSink with different params
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_010, TestSize.Level1)
{
    AudioStreamInfo streamInfo;
    streamInfo.channels = CHANNEL_6; // 与缓存参数不同
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.format = SAMPLE_S32LE;

    playbackEngine_->isInit_ = true;
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    playbackEngine_->uChannel_ = CHANNEL_8;
    playbackEngine_->uSampleRate_ = SAMPLE_RATE_44100;
    playbackEngine_->uformat_ = SAMPLE_S16LE;

    auto ret = playbackEngine_->InitSink(streamInfo);
    EXPECT_EQ(ret, SUCCESS);
}

 /**
 * @tc.name  : Test CabinPlayabackEngine API
 * @tc.type  : FUNC
 * @tc.number: CabinPlayabackEngine_011
 * @tc.desc  : Test CabinPlayBackEngine::InitSink overload success
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_011, TestSize.Level1)
{
    uint32_t channel = CHANNEL_8;
    AudioSampleFormat format = SAMPLE_S16LE;
    uint32_t rate = SAMPLE_RATE_48000;
    AudioChannelLayout layout = CH_LAYOUT_STEREO;
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    playbackEngine_->device_.deviceType_ = DEVICE_TYPE_SPEAKER;

    auto ret = playbackEngine_->InitSink(channel, format, rate, layout);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(playbackEngine_->uChannel_, channel);
    EXPECT_EQ(playbackEngine_->uSampleRate_, rate);
}

 /**
 * @tc.name  : Test CabinPlayabackEngine API
 * @tc.type  : FUNC
 * @tc.number: CabinPlayabackEngine_013
 * @tc.desc  : Test CabinPlayBackEngine::GetLatency
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_013, TestSize.Level1)
{
    playbackEngine_->isStart_ = false;
    EXPECT_EQ(playbackEngine_->GetLatency(), 0);
    
    playbackEngine_->isStart_ = true;
    playbackEngine_->latency_ = 50000;
    EXPECT_EQ(playbackEngine_->GetLatency(), 50000);
    
    playbackEngine_->latency_ = 0;
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    auto retLatency = playbackEngine_->GetLatency();
    EXPECT_EQ(retLatency, 0);
}

 /**
 * @tc.name  : Test CabinPlayabackEngine API
 * @tc.type  : FUNC
 * @tc.number: CabinPlayabackEngine_014
 * @tc.desc  : Test CabinPlayBackEngine::RegisterSinkLatencyFetcher
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_014, TestSize.Level1)
{
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    playbackEngine_->RegisterSinkLatencyFetcher(playbackEngine_->renderId_);
}

 /**
 * @tc.name  : Test CabinPlayabackEngine API
 * @tc.type  : FUNC
 * @tc.number: CabinPlayabackEngine_015
 * @tc.desc  : Test CabinPlayBackEngine::RegisterSinkLatencyFetcherToStreamIfNeeded
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_015, TestSize.Level1)
{
    AudioProcessConfig config;
    auto stream = std::make_shared<ProRendererStreamImpl>(config, true);
    playbackEngine_->stream_ = stream;
    playbackEngine_->sinkLatencyFetcher_ = [](uint32_t &latency) {
        latency = 20;
        return 0;
    };
    playbackEngine_->RegisterSinkLatencyFetcherToStreamIfNeeded();
    EXPECT_NE(playbackEngine_->stream_, nullptr);
}


 /**
 * @tc.name  : Test CabinPlayabackEngine API
 * @tc.type  : FUNC
 * @tc.number: CabinPlayabackEngine_016
 * @tc.desc  : Test AdjustVolume logic covering first set, volume change, and no-change scenarios
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_016, TestSize.Level1)
{
    AudioProcessConfig config;
    config.streamType = STREAM_MUSIC;
    auto stream = std::make_shared<ProRendererStreamImpl>(config, false);
    stream->SetStreamIndex(101);
    playbackEngine_->stream_ = stream;
    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);
    playbackEngine_->firstSetVolume_ = true;
    
    playbackEngine_->AdjustVolume();
    EXPECT_TRUE(playbackEngine_->firstSetVolume_);

    playbackEngine_->firstSetVolume_ = false;
    playbackEngine_->AdjustVolume();
}

 /**
 * @tc.name  : Test CabinPlayabackEngine API
 * @tc.type  : FUNC
 * @tc.number: CabinPlayabackEngine_017
 * @tc.desc  : Test DoRenderFrame to ensure it calls sink render and returns index to stream
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_017, TestSize.Level1)
{
    SetParameter("persist.multimedia.3dadirecttest", std::to_string(1).c_str());
    int32_t realSysVal = 0;
    for (int i = 0; i < 50; i++) {
        GetSysPara("persist.multimedia.3dadirecttest", realSysVal);
        if (realSysVal == 1) {
            break;
        }
        usleep(10000);
    }
    ASSERT_EQ(realSysVal, 1);

    playbackEngine_->renderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER,
        HDI_ID_TYPE_AUDIO_VIVID_3DA_DIRECT, HDI_ID_INFO_DEFAULT, true);

    auto sink = HdiAdapterManager::GetInstance().GetRenderSink(playbackEngine_->renderId_);
    ASSERT_EQ(sink, nullptr);

    AudioProcessConfig config;
    auto stream = std::make_shared<ProRendererStreamImpl>(config, false);
    playbackEngine_->stream_ = stream;

    std::vector<char> audioBufferConverted = {0x01, 0x02, 0x03, 0x04};
    int32_t testIndex = 5;
    int32_t testAppUid = 10001;

    playbackEngine_->DoRenderFrame(audioBufferConverted, testIndex, testAppUid);
    EXPECT_EQ(audioBufferConverted.size(), 4);
    SetParameter("persist.multimedia.3dadirecttest", std::to_string(0).c_str());
}

 /**
 * @tc.name  : Test CabinPlayabackEngine API
 * @tc.type  : FUNC
 * @tc.number: CabinPlayabackEngine_018
 * @tc.desc  : Validate internal helper functions for format and bit depth conversion
 */
HWTEST_F(CabinPlayBackEngineUnitTest, CabinPlayBackEngine_018, TestSize.Level1)
{
    EXPECT_EQ(playbackEngine_->GetFormatByteSize(SAMPLE_S16LE), sizeof(int16_t));
    EXPECT_EQ(playbackEngine_->GetFormatByteSize(SAMPLE_S24LE), 3u);
    EXPECT_EQ(playbackEngine_->GetFormatByteSize(SAMPLE_S32LE), sizeof(int32_t));
}
}
}