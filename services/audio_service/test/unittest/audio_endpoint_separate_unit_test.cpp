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

#include "audio_errors.h"
#include "audio_service_log.h"
#include "audio_info.h"
#include "audio_ring_cache.h"
#include "audio_process_config.h"
#include "linear_pos_time_model.h"
#include "oh_audio_buffer.h"
#include <gtest/gtest.h>
#include "audio_endpoint.h"
#include "sink/i_audio_render_sink.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "audio_process_in_server.h"
#include "audio_endpoint.h"
#include "audio_service.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
constexpr int32_t DEFAULT_STREAM_ID = 10;

static AudioProcessConfig InitProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

class AudioEndpointSeparateUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioEndpointSeparateUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioEndpointSeparateUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioEndpointSeparateUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioEndpointSeparateUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_001
 * @tc.desc  : Test AudioEndpointSeparate::SetVolume
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_001, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    float volume = 0.0f;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    ptr->fastRenderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST,
        "endpoint_sep_test", true);
    auto ret = ptr->SetVolume(streamType, volume);
    HdiAdapterManager::GetInstance().ReleaseId(ptr->fastRenderId_);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_002
 * @tc.desc  : Test AudioEndpointSeparate::SetVolume
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_002, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_VOICE_CALL;
    float volume = 0.0f;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    ptr->fastRenderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST,
        "endpoint_sep_test", true);
    streamType = AudioStreamType::STREAM_VOICE_CALL;
    ptr->streamType_ = STREAM_DEFAULT;
    auto ret = ptr->SetVolume(streamType, volume);
    EXPECT_EQ(ret, SUCCESS);
    HdiAdapterManager::GetInstance().ReleaseId(ptr->fastRenderId_);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_003
 * @tc.desc  : Test AudioEndpointSeparate::ResolveBuffer
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_003, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    ptr->isInited_ = false;
    std::shared_ptr<OHAudioBuffer> buffer;
    auto ret = ptr->ResolveBuffer(buffer);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_004
 * @tc.desc  : Test AudioEndpointSeparate::ResolveBuffer
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_004, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    ptr->isInited_ = true;
    std::shared_ptr<OHAudioBuffer> buffer;
    auto ret = ptr->ResolveBuffer(buffer);
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptr->dstAudioBuffer_ = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    ptr->isInited_ = true;
    ret = ptr->ResolveBuffer(buffer);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_005
 * @tc.desc  : Test AudioEndpointSeparate::Release
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_005, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    ptr->isInited_ = false;
    ptr->Release();
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_006
 * @tc.desc  : Test AudioEndpointSeparate::Release
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_006, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    ptr->isInited_ = true;
    ptr->fastRenderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST,
        "endpoint_sep_test", true);
    ptr->Release();
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_007
 * @tc.desc  : Test AudioEndpointSeparate::Release
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_007, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    ptr->isInited_ = true;
    ptr->fastRenderId_ = HDI_INVALID_ID;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptr->dstAudioBuffer_ = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    ptr->Release();
    ptr->isInited_ = true;
    ptr->fastRenderId_ = HDI_INVALID_ID;
    ptr->dstAudioBuffer_ = nullptr;
    ptr->Release();
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_008
 * @tc.desc  : Test AudioEndpointSeparate::~AudioEndpointSeparate
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_008, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    ptr->isInited_ = true;
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_009
 * @tc.desc  : Test AudioEndpointSeparate::Config
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_009, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    std::shared_ptr<AudioDeviceDescriptor> ptr2 =
        std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    ptr2->deviceRole_ = INPUT_DEVICE;
    ptr2->networkId_ = REMOTE_NETWORK_ID;
    auto ret = ptr->Config(*ptr2);
    EXPECT_EQ(ret, false);
    ptr2->deviceRole_ = INPUT_DEVICE;
    ptr2->networkId_ = LOCAL_NETWORK_ID;
    ret = ptr->Config(*ptr2);
    EXPECT_EQ(ret, false);
    ptr2->deviceRole_ = OUTPUT_DEVICE;
    ptr2->networkId_ = REMOTE_NETWORK_ID;
    ret = ptr->Config(*ptr2);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_010
 * @tc.desc  : Test AudioEndpointSeparate::GetAdapterBufferInfo
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_010, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    std::shared_ptr<AudioDeviceDescriptor> ptr2 =
        std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    auto ret = ptr->GetAdapterBufferInfo(*ptr2);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ptr->dstBufferFd_ = 0;
    ptr->dstTotalSizeInframe_ = 1;
    ptr->dstSpanSizeInframe_ = 1;
    ptr->dstBufferFd_ = 1;
    ptr->fastRenderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST,
        "endpoint_sep_test", true);
    ret = ptr->GetAdapterBufferInfo(*ptr2);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    HdiAdapterManager::GetInstance().ReleaseId(ptr->fastRenderId_);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_011
 * @tc.desc  : Test AudioEndpointSeparate::GetAdapterBufferInfo
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_011, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    std::shared_ptr<AudioDeviceDescriptor> ptr2 =
        std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    ptr->fastRenderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST,
        "endpoint_sep_test", true);
    ptr->dstBufferFd_ = -1;
    ptr->dstTotalSizeInframe_ = 1;
    ptr->dstSpanSizeInframe_ = 1;
    ptr->dstBufferFd_ = 1;
    auto ret = ptr->GetAdapterBufferInfo(*ptr2);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ptr->dstBufferFd_ = 0;
    ptr->dstTotalSizeInframe_ = 0;
    ptr->dstSpanSizeInframe_ = 1;
    ptr->dstBufferFd_ = 1;
    ret = ptr->GetAdapterBufferInfo(*ptr2);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ptr->dstBufferFd_ = 0;
    ptr->dstTotalSizeInframe_ = 1;
    ptr->dstSpanSizeInframe_ = 0;
    ptr->dstBufferFd_ = 1;
    ret = ptr->GetAdapterBufferInfo(*ptr2);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ptr->dstBufferFd_ = 0;
    ptr->dstTotalSizeInframe_ = 1;
    ptr->dstSpanSizeInframe_ = 1;
    ptr->dstBufferFd_ = 0;
    ret = ptr->GetAdapterBufferInfo(*ptr2);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    HdiAdapterManager::GetInstance().ReleaseId(ptr->fastRenderId_);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_012
 * @tc.desc  : Test AudioEndpointSeparate::PrepareDeviceBuffer
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_012, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    std::shared_ptr<AudioDeviceDescriptor> ptr2 =
        std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptr->dstAudioBuffer_ = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    auto ret = ptr->PrepareDeviceBuffer(*ptr2);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_013
 * @tc.desc  : Test AudioEndpointSeparate::PrepareDeviceBuffer
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_013, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    std::shared_ptr<AudioDeviceDescriptor> ptr2 =
        std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    ptr->dstAudioBuffer_ = nullptr;
    ptr->PrepareDeviceBuffer(*ptr2);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_014
 * @tc.desc  : Test AudioEndpointSeparate::InitAudiobuffer
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_014, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    bool resetReadWritePos = true;
    ptr->InitAudiobuffer(resetReadWritePos);
    resetReadWritePos = false;
    ptr->InitAudiobuffer(resetReadWritePos);
    ptr->deviceInfo_.deviceRole_ = INPUT_DEVICE;
    ptr->InitAudiobuffer(resetReadWritePos);
    ptr->deviceInfo_.deviceRole_ = OUTPUT_DEVICE;
    ptr->InitAudiobuffer(resetReadWritePos);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_015
 * @tc.desc  : Test AudioEndpointSeparate::IsAnyProcessRunning
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_015, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    ptr->processBufferList_.clear();
    auto ret = ptr->IsAnyProcessRunning();
    EXPECT_EQ(ret, false);
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    std::shared_ptr<OHAudioBuffer> processBuffer = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    ptr->processBufferList_.push_back(processBuffer);
    ret = ptr->IsAnyProcessRunning();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_016
 * @tc.desc  : Test AudioEndpointSeparate::ResyncPosition
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_016, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    ptr->ResyncPosition();
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_017
 * @tc.desc  : Test AudioEndpointSeparate::StartDevice
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_017, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    ptr->endpointStatus_.store(AudioEndpoint::EndpointStatus::INVALID);
    auto ret = ptr->StartDevice();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_018
 * @tc.desc  : Test AudioEndpointSeparate::StartDevice
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_018, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    ptr->endpointStatus_.store(AudioEndpoint::EndpointStatus::IDEL);
    ptr->fastRenderId_ = HDI_INVALID_ID;
    auto ret = ptr->StartDevice();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_019
 * @tc.desc  : Test AudioEndpointSeparate::StopDevice
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_019, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptr->dstAudioBuffer_ = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    ptr->StopDevice();
    ptr->dstAudioBuffer_ = nullptr;
    ptr->StopDevice();
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_020
 * @tc.desc  : Test AudioEndpointSeparate::OnStart
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_020, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    IAudioProcessStream *processStream = nullptr;
    ptr->endpointStatus_.store(AudioEndpoint::EndpointStatus::RUNNING);
    auto ret = ptr->OnStart(processStream);
    EXPECT_EQ(ret, SUCCESS);
    ptr->endpointStatus_.store(AudioEndpoint::EndpointStatus::IDEL);
    ptr->isDeviceRunningInIdel_ = false;
    ret = ptr->OnStart(processStream);
    EXPECT_EQ(ret, SUCCESS);
    ptr->endpointStatus_.store(AudioEndpoint::EndpointStatus::IDEL);
    ptr->isDeviceRunningInIdel_ = true;
    ret = ptr->OnStart(processStream);
    EXPECT_EQ(ret, SUCCESS);
    ptr->endpointStatus_.store(AudioEndpoint::EndpointStatus::STOPPING);
    ret = ptr->OnStart(processStream);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_021
 * @tc.desc  : Test AudioEndpointSeparate::OnPause
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_021, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    IAudioProcessStream *processStream = nullptr;
    ptr->endpointStatus_.store(AudioEndpoint::EndpointStatus::RUNNING);
    auto ret = ptr->OnPause(processStream);
    EXPECT_EQ(ret, SUCCESS);
    ptr->endpointStatus_.store(AudioEndpoint::EndpointStatus::IDEL);
    ptr->isDeviceRunningInIdel_ = false;
    ret = ptr->OnPause(processStream);
    EXPECT_EQ(ret, SUCCESS);
    ptr->endpointStatus_.store(AudioEndpoint::EndpointStatus::IDEL);
    ptr->isDeviceRunningInIdel_ = true;
    ret = ptr->OnPause(processStream);
    EXPECT_EQ(ret, SUCCESS);
    ptr->endpointStatus_.store(AudioEndpoint::EndpointStatus::STOPPING);
    ret = ptr->OnPause(processStream);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_022
 * @tc.desc  : Test AudioEndpointSeparate::OnUpdateHandleInfo
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_022, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    IAudioProcessStream *processStream = nullptr;
    ptr->OnUpdateHandleInfo(processStream);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_023
 * @tc.desc  : Test AudioEndpointSeparate::ProcessData
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_023, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    std::vector<AudioStreamData> srcDataList;
    AudioStreamData dstData;
    ptr->ProcessData(srcDataList, dstData);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_024
 * @tc.desc  : Test AudioEndpointSeparate::GetDeviceHandleInfo
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_024, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    uint64_t frames = 0;
    int64_t nanoTime = 0;
    ptr->fastRenderId_ = HDI_INVALID_ID;
    auto ret = ptr->GetDeviceHandleInfo(frames, nanoTime);
    EXPECT_EQ(ret, false);
    ptr->GetDeviceHandleInfo(frames, nanoTime);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_025
 * @tc.desc  : Test AudioEndpointSeparate::GetStatusStr
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_025, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    enum AudioEndpoint::EndpointStatus status = AudioEndpoint::EndpointStatus::INVALID;
    auto ret = ptr->GetStatusStr(status);
    EXPECT_EQ(ret, "INVALID");
    status = AudioEndpoint::EndpointStatus::UNLINKED;
    ret = ptr->GetStatusStr(status);
    EXPECT_EQ(ret, "UNLINKED");
    status = AudioEndpoint::EndpointStatus::IDEL;
    ret = ptr->GetStatusStr(status);
    EXPECT_EQ(ret, "IDEL");
    status = AudioEndpoint::EndpointStatus::STARTING;
    ret = ptr->GetStatusStr(status);
    EXPECT_EQ(ret, "STARTING");
    status = AudioEndpoint::EndpointStatus::RUNNING;
    ret = ptr->GetStatusStr(status);
    EXPECT_EQ(ret, "RUNNING");
    status = AudioEndpoint::EndpointStatus::STOPPING;
    ret = ptr->GetStatusStr(status);
    EXPECT_EQ(ret, "STOPPING");
    status = AudioEndpoint::EndpointStatus::STOPPED;
    ret = ptr->GetStatusStr(status);
    EXPECT_EQ(ret, "STOPPED");
    const int noSuchStatus = 7;
    status = static_cast<AudioEndpoint::EndpointStatus>(noSuchStatus);
    ret = ptr->GetStatusStr(status);
    EXPECT_EQ(ret, "NO_SUCH_STATUS");
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_026
 * @tc.desc  : Test AudioEndpointSeparate::WriteToProcessBuffers
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_026, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    struct BufferDesc readBuf;
    ptr->WriteToProcessBuffers(readBuf);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_027
 * @tc.desc  : Test static enum AudioSampleFormat ConvertToHdiAdapterFormat
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_027, TestSize.Level1)
{
    AudioSampleFormat format = SAMPLE_U8;
    auto ret = ConvertToHdiAdapterFormat(format);
    EXPECT_EQ(ret, SAMPLE_U8);
    format = SAMPLE_S16LE;
    ret = ConvertToHdiAdapterFormat(format);
    EXPECT_EQ(ret, SAMPLE_S16LE);
    format = SAMPLE_S24LE;
    ret = ConvertToHdiAdapterFormat(format);
    EXPECT_EQ(ret, SAMPLE_S24LE);
    format = SAMPLE_S32LE;
    ret = ConvertToHdiAdapterFormat(format);
    EXPECT_EQ(ret, SAMPLE_S32LE);
    format = SAMPLE_F32LE;
    ret = ConvertToHdiAdapterFormat(format);
    EXPECT_EQ(ret, SAMPLE_F32LE);
    format = INVALID_WIDTH;
    ret = ConvertToHdiAdapterFormat(format);
    EXPECT_EQ(ret, INVALID_WIDTH);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_028
 * @tc.desc  : Test AudioEndpointSeparate::Config
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_028, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    std::shared_ptr<AudioDeviceDescriptor> ptr2 =
        std::make_shared<AudioDeviceDescriptor>(AudioDeviceDescriptor::DEVICE_INFO);
    ptr2->deviceRole_ = OUTPUT_DEVICE;
    ptr2->networkId_ = LOCAL_NETWORK_ID;
    ptr->Config(*ptr2);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_029
 * @tc.desc  : Test AudioEndpointSeparate::InitAudiobuffer
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_029, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    ptr->dstAudioBuffer_ = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);

    bool resetReadWritePos = true;
    ptr->InitAudiobuffer(resetReadWritePos);
    resetReadWritePos = false;
    ptr->InitAudiobuffer(resetReadWritePos);
    ptr->deviceInfo_.deviceRole_ = INPUT_DEVICE;
    ptr->InitAudiobuffer(resetReadWritePos);
    ptr->deviceInfo_.deviceRole_ = OUTPUT_DEVICE;
    ptr->InitAudiobuffer(resetReadWritePos);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_040
 * @tc.desc  : Test AudioEndpointSeparate::ProcessData
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_030, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    AudioStreamInfo streamInfo;
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    streamInfo.channels = AudioChannel::STEREO;
    BufferDesc bufferDesc = {nullptr, 0, 0};
    int32_t volumeStart = 0;
    int32_t volumeEnd = 0;
    std::unordered_map<int32_t, bool> isInnerCaped;

    AudioStreamData tddData = {streamInfo, bufferDesc, volumeStart, volumeEnd, isInnerCaped};
    std::vector<AudioStreamData> srcDataList;
    srcDataList.push_back(tddData);

    BufferDesc bufferDesc_2 = {nullptr, 0, 1};
    AudioStreamData dstData = {streamInfo, bufferDesc_2, volumeStart, volumeEnd, isInnerCaped};

    ptr->ProcessData(srcDataList, dstData);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_031
 * @tc.desc  : Test AudioEndpointSeparate::StartDevice
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_031, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    ptr->endpointStatus_.store(AudioEndpoint::EndpointStatus::IDEL);
    ptr->fastRenderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST,
        "endpoint_sep_test", true);
    auto ret = ptr->StartDevice();
    EXPECT_EQ(ret, false);
    HdiAdapterManager::GetInstance().ReleaseId(ptr->fastRenderId_);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_032
 * @tc.desc  : Test AudioEndpointSeparate::ProcessData
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_032, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    AudioStreamInfo streamInfo;
    streamInfo.format = AudioSampleFormat::INVALID_WIDTH;
    BufferDesc bufferDesc;
    int32_t volumeStart = 0;
    int32_t volumeEnd = 0;
    std::unordered_map<int32_t, bool> isInnerCaped;

    AudioStreamData tddData = {streamInfo, bufferDesc, volumeStart, volumeEnd, isInnerCaped};

    std::vector<AudioStreamData> srcDataList;
    srcDataList.push_back(tddData);
    AudioStreamData dstData = tddData;

    ptr->ProcessData(srcDataList, dstData);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_033
 * @tc.desc  : Test AudioEndpointSeparate::ProcessData
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_033, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    AudioStreamInfo streamInfo;
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    streamInfo.channels = AudioChannel::STEREO;
    BufferDesc bufferDesc;
    int32_t volumeStart = 0;
    int32_t volumeEnd = 0;
    std::unordered_map<int32_t, bool> isInnerCaped;

    AudioStreamData tddData = {streamInfo, bufferDesc, volumeStart, volumeEnd, isInnerCaped};
    std::vector<AudioStreamData> srcDataList;
    srcDataList.push_back(tddData);

    AudioStreamInfo streamInfo_2;
    streamInfo_2.format = AudioSampleFormat::SAMPLE_S16LE;
    streamInfo_2.channels = AudioChannel::STEREO;
    AudioStreamData dstData = {streamInfo_2, bufferDesc, volumeStart, volumeEnd, isInnerCaped};

    ptr->ProcessData(srcDataList, dstData);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_034
 * @tc.desc  : Test AudioEndpointSeparate::ProcessData
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_034, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    AudioStreamInfo streamInfo;
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    streamInfo.channels = AudioChannel::STEREO;
    BufferDesc bufferDesc = {nullptr, 0, 0};
    int32_t volumeStart = 0;
    int32_t volumeEnd = 0;
    std::unordered_map<int32_t, bool> isInnerCaped;

    AudioStreamData tddData = {streamInfo, bufferDesc, volumeStart, volumeEnd, isInnerCaped};
    std::vector<AudioStreamData> srcDataList;
    srcDataList.push_back(tddData);

    BufferDesc bufferDesc_2 = {nullptr, 1, 0};
    AudioStreamData dstData = {streamInfo, bufferDesc_2, volumeStart, volumeEnd, isInnerCaped};

    ptr->ProcessData(srcDataList, dstData);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_035
 * @tc.desc  : Test AudioEndpointSeparate::ProcessData
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_035, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    AudioStreamInfo streamInfo;
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    streamInfo.channels = AudioChannel::MONO;
    BufferDesc bufferDesc;
    int32_t volumeStart = 0;
    int32_t volumeEnd = 0;
    std::unordered_map<int32_t, bool> isInnerCaped;

    AudioStreamData tddData = {streamInfo, bufferDesc, volumeStart, volumeEnd, isInnerCaped};

    std::vector<AudioStreamData> srcDataList;
    srcDataList.push_back(tddData);
    AudioStreamData dstData = tddData;

    ptr->ProcessData(srcDataList, dstData);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_036
 * @tc.desc  : Test AudioEndpointSeparate::ProcessData
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_036, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    AudioStreamInfo streamInfo;
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    streamInfo.channels = AudioChannel::STEREO;
    BufferDesc bufferDesc;
    int32_t volumeStart = 0;
    int32_t volumeEnd = 0;
    std::unordered_map<int32_t, bool> isInnerCaped;

    AudioStreamData tddData = {streamInfo, bufferDesc, volumeStart, volumeEnd, isInnerCaped};
    std::vector<AudioStreamData> srcDataList;
    srcDataList.push_back(tddData);

    AudioStreamInfo streamInfo_2;
    streamInfo_2.format = AudioSampleFormat::INVALID_WIDTH;
    AudioStreamData dstData = {streamInfo_2, bufferDesc, volumeStart, volumeEnd, isInnerCaped};

    ptr->ProcessData(srcDataList, dstData);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_037
 * @tc.desc  : Test AudioEndpointSeparate::ProcessData
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_037, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    AudioStreamInfo streamInfo;
    streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    streamInfo.channels = AudioChannel::STEREO;
    BufferDesc bufferDesc;
    int32_t volumeStart = 0;
    int32_t volumeEnd = 0;
    std::unordered_map<int32_t, bool> isInnerCaped;

    AudioStreamData tddData = {streamInfo, bufferDesc, volumeStart, volumeEnd, isInnerCaped};
    std::vector<AudioStreamData> srcDataList;
    srcDataList.push_back(tddData);

    AudioStreamInfo streamInfo_2;
    streamInfo_2.format = AudioSampleFormat::SAMPLE_S16LE;
    streamInfo_2.channels = AudioChannel::MONO;
    AudioStreamData dstData = {streamInfo_2, bufferDesc, volumeStart, volumeEnd, isInnerCaped};

    ptr->ProcessData(srcDataList, dstData);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_038
 * @tc.desc  : Test AudioEndpointSeparate::IsAnyProcessRunning
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_038, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    ptr->processBufferList_.clear();
    auto ret = ptr->IsAnyProcessRunning();
    EXPECT_EQ(ret, false);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    std::shared_ptr<OHAudioBuffer> processBuffer = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    processBuffer->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();
    processBuffer->basicBufferInfo_->streamStatus.store(StreamStatus::STREAM_STARTING);
    ptr->processBufferList_.push_back(processBuffer);
    ret = ptr->IsAnyProcessRunning();
    EXPECT_EQ(ret, false);

    ptr->processBufferList_.clear();
    processBuffer->basicBufferInfo_->streamStatus.store(StreamStatus::STREAM_RUNNING);
    ptr->processBufferList_.push_back(processBuffer);
    ret = ptr->IsAnyProcessRunning();
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_039
 * @tc.desc  : Test AudioEndpointSeparate::OnUpdateHandleInfo
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_039, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;

    processStream->isBufferConfiged_ = true;
    processStream->processBuffer_ = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);

    ptr->processList_.push_back(processStream);
    auto ret = ptr->OnUpdateHandleInfo(processStream);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_040
 * @tc.desc  : Test AudioEndpointSeparate::WriteToProcessBuffers
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_040, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    std::shared_ptr<OHAudioBuffer> processBuffer = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    processBuffer->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();
    processBuffer->basicBufferInfo_->streamStatus.store(StreamStatus::STREAM_RUNNING);
    ptr->processBufferList_.push_back(processBuffer);

    struct BufferDesc readBuf;
    ptr->WriteToProcessBuffers(readBuf);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_041
 * @tc.desc  : Test AudioEndpointSeparate::WriteToProcessBuffers
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_041, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    std::shared_ptr<OHAudioBuffer> processBuffer = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    processBuffer->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();
    processBuffer->basicBufferInfo_->streamStatus.store(StreamStatus::STREAM_STARTING);
    ptr->processBufferList_.push_back(processBuffer);

    struct BufferDesc readBuf;
    ptr->WriteToProcessBuffers(readBuf);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_042
 * @tc.desc  : Test AudioEndpointSeparate::WriteToProcessBuffers
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_042, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    std::shared_ptr<OHAudioBuffer> processBuffer = nullptr;
    ptr->processBufferList_.push_back(processBuffer);

    struct BufferDesc readBuf;
    ptr->WriteToProcessBuffers(readBuf);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_043
 * @tc.desc  : Test AudioEndpointSeparate::WriteToProcessBuffers
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_043, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    std::shared_ptr<OHAudioBuffer> processBuffer = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);

    ptr->processBufferList_.push_back(processBuffer);

    struct BufferDesc readBuf;
    ptr->WriteToProcessBuffers(readBuf);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_044
 * @tc.desc  : Test AudioEndpointSeparate::GetDeviceHandleInfo
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_044, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    uint64_t frames = 0;
    int64_t nanoTime = 0;
    ptr->fastRenderId_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_FAST,
        "endpoint_sep_test", true);
    ptr->GetDeviceHandleInfo(frames, nanoTime);
    HdiAdapterManager::GetInstance().ReleaseId(ptr->fastRenderId_);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_045
 * @tc.desc  : Test AudioEndpointSeparate::ShouldInnerCap
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_045, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    int32_t innerCapId = 0;
    bool ret = true;

    ret = ptr->ShouldInnerCap(innerCapId);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_046
 * @tc.desc  : Test AudioEndpointSeparate::EnableFastInnerCap
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_046, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    int32_t innerCapId = 0;
    int32_t ret = 0;

    ret = ptr->EnableFastInnerCap(innerCapId);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_047
 * @tc.desc  : Test AudioEndpointSeparate::GetBuffer
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_047, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    std::shared_ptr<OHAudioBuffer> ret;

    ret = ptr->GetBuffer();
    EXPECT_EQ(ret, nullptr);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_048
 * @tc.desc  : Test AudioEndpointSeparate::GetPreferBufferInfo
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_048, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    uint32_t totalSizeInframe = 0;
    uint32_t spanSizeInframe = 0;
    int32_t ret = 0;

    ret = ptr->GetPreferBufferInfo(totalSizeInframe, spanSizeInframe);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_049
 * @tc.desc  : Test AudioEndpointSeparate::GetMaxAmplitude
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_049, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    float ret;

    ret = ptr->GetMaxAmplitude();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_050
 * @tc.desc  : Test AudioEndpointSeparate::GetLinkedProcessCount
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_050, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    uint32_t ret;

    ret = ptr->GetLinkedProcessCount();
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_051
 * @tc.desc  : Test AudioEndpointSeparate::GetAudioMode
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_051, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    AudioMode ret;

    ret = ptr->GetAudioMode();
    EXPECT_EQ(ret, AUDIO_MODE_PLAYBACK);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_052
 * @tc.desc  : Test AudioEndpointSeparate::GetProcLastWriteDoneInfo
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_052, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);
    int32_t ret;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_SERVER_SHARED;
    uint32_t totalSizeInFrame = 96;
    uint32_t spanSizeInFrame = 16;
    uint32_t byteSizePerFrame = 4;
    std::shared_ptr<OHAudioBuffer> processBuffer = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    uint64_t curWriteFrame = 0;
    uint64_t proHandleFrame = 0;
    int64_t proHandleTime = 0;
    int dataFd = INVALID_FD;
    int infoFd = AUDIO_SERVER_SHARED;

    ret = processBuffer->Init(dataFd, infoFd);
    EXPECT_EQ(ret, SUCCESS);

    ptr->dstSpanSizeInframe_ = 4;
    ret = ptr->GetProcLastWriteDoneInfo(processBuffer, curWriteFrame, proHandleFrame, proHandleTime);
    EXPECT_EQ(ret, SUCCESS);

    curWriteFrame = 5;
    ret = ptr->GetProcLastWriteDoneInfo(processBuffer, curWriteFrame, proHandleFrame, proHandleTime);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_053
 * @tc.desc  : Test AudioEndpointSeparate::LinkProcessStream
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_053, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    bool startWhenLinking = false;
    int32_t ret = 0;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    std::shared_ptr<OHAudioBuffer> processBuffer = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);

    processBuffer->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();
    processBuffer->basicBufferInfo_->streamStatus.store(StreamStatus::STREAM_STARTING);
    processStream->isBufferConfiged_ = true;
    processStream->processBuffer_ = processBuffer;
    ptr->processList_.push_back(processStream);

    ptr->endpointStatus_ = AudioEndpoint::EndpointStatus::RUNNING;
    ret = ptr->LinkProcessStream(processStream, startWhenLinking);
    EXPECT_EQ(ret, SUCCESS);

    ptr->endpointStatus_ = AudioEndpoint::EndpointStatus::UNLINKED;
    ret = ptr->LinkProcessStream(processStream, startWhenLinking);
    EXPECT_EQ(ret, SUCCESS);

    processBuffer->basicBufferInfo_->streamStatus.store(StreamStatus::STREAM_RUNNING);
    ptr->isDeviceRunningInIdel_ = true;
    ptr->endpointStatus_ = AudioEndpoint::EndpointStatus::UNLINKED;
    ret = ptr->LinkProcessStream(processStream, startWhenLinking);
    EXPECT_EQ(ret, SUCCESS);

    processBuffer->basicBufferInfo_->streamStatus.store(StreamStatus::STREAM_RUNNING);
    ptr->isDeviceRunningInIdel_ = false;
    ptr->endpointStatus_ = AudioEndpoint::EndpointStatus::UNLINKED;
    ret = ptr->LinkProcessStream(processStream, startWhenLinking);
    EXPECT_EQ(ret, SUCCESS);

    processBuffer->basicBufferInfo_->streamStatus.store(StreamStatus::STREAM_RUNNING);
    ptr->isDeviceRunningInIdel_ = false;
    ptr->endpointStatus_ = AudioEndpoint::EndpointStatus::INVALID;
    ret = ptr->LinkProcessStream(processStream, startWhenLinking);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioEndpointSeparate API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointSeparate_054
 * @tc.desc  : Test AudioEndpointSeparate::UnlinkProcessStream
 */
HWTEST(AudioEndpointSeparateUnitTest, AudioEndpointSeparate_054, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::EndpointType::TYPE_MMAP;
    uint64_t id = 0;
    AudioStreamType streamType = AudioStreamType::STREAM_DEFAULT;
    std::shared_ptr<AudioEndpointSeparate> ptr = std::make_shared<AudioEndpointSeparate>(type, id, streamType);

    AudioProcessConfig config = InitProcessConfig();
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(config, g_audioServicePtr);
    int32_t ret = 0;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    std::shared_ptr<OHAudioBuffer> processBuffer = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);

    processBuffer->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();
    processBuffer->basicBufferInfo_->streamStatus.store(StreamStatus::STREAM_STARTING);
    processStream->isBufferConfiged_ = true;
    processStream->processBuffer_ = processBuffer;
    ptr->processList_.push_back(processStream);
    ptr->processBufferList_.push_back(processBuffer);

    ptr->endpointStatus_ = AudioEndpoint::EndpointStatus::RUNNING;
    ret = ptr->UnlinkProcessStream(processStream);
    EXPECT_EQ(ret, SUCCESS);
}
} // namespace AudioStandard
} // namespace OHOS
