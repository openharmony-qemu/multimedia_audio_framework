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

#include "audio_endpoint_plus_unit_test.h"

#include "accesstoken_kit.h"
#include "audio_device_info.h"
#include "audio_endpoint.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_process_config.h"
#include "audio_server.h"
#include "audio_service.h"
#include "audio_stream_info.h"
#include "audio_utils.h"
#include "policy_handler.h"
#include "audio_endpoint.cpp"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {


void AudioEndpointPlusUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioEndpointPlusUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioEndpointPlusUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioEndpointPlusUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

const size_t BIGNUMBER = 2808348670;

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_001
 * @tc.desc  : Test AudioEndpointInner::CheckStandBy()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_001, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    audioEndpointInner->endpointStatus_.store(AudioEndpoint::EndpointStatus::RUNNING);

    audioEndpointInner->CheckStandBy();
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_002
 * @tc.desc  : Test AudioEndpointInner::CheckStandBy()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_002, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    audioEndpointInner->endpointStatus_.store(AudioEndpoint::EndpointStatus::IDEL);

    audioEndpointInner->CheckStandBy();
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_003
 * @tc.desc  : Test AudioEndpointInner::CheckAllBufferReady()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_003, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    int64_t checkTime = 0;
    uint64_t curWritePos = 0;

    audioEndpointInner->CheckAllBufferReady(checkTime, curWritePos);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_004
 * @tc.desc  : Test AudioEndpointInner::CheckAllBufferReady()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_004, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    int64_t checkTime = 0;
    uint64_t curWritePos = 0;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    std::shared_ptr<OHAudioBuffer> processBuffer = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    BasicBufferInfo basicBufferInfo;
    processBuffer->basicBufferInfo_ = &basicBufferInfo;
    processBuffer->basicBufferInfo_->streamStatus.store(StreamStatus::STREAM_RUNNING);
    audioEndpointInner->processBufferList_.push_back(processBuffer);

    audioEndpointInner->CheckAllBufferReady(checkTime, curWritePos);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_005
 * @tc.desc  : Test AudioEndpointInner::CheckAllBufferReady()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_005, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    int64_t checkTime = 0;
    uint64_t curWritePos = 0;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    std::shared_ptr<OHAudioBuffer> processBuffer = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    BasicBufferInfo basicBufferInfo;
    processBuffer->basicBufferInfo_ = &basicBufferInfo;
    processBuffer->basicBufferInfo_->streamStatus.store(StreamStatus::STREAM_STARTING);
    audioEndpointInner->processBufferList_.push_back(processBuffer);

    audioEndpointInner->CheckAllBufferReady(checkTime, curWritePos);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_006
 * @tc.desc  : Test AudioEndpointInner::MixToDupStream()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_006, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    std::vector<AudioStreamData> srcDataList;
    AudioStreamData audioStreamData;
    audioStreamData.isInnerCaped = false;
    srcDataList.push_back(audioStreamData);
    audioEndpointInner->dupBuffer_ = std::make_unique<uint8_t []>(1);
    EXPECT_NE(nullptr, audioEndpointInner->dupBuffer_);

    audioEndpointInner->MixToDupStream(srcDataList);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_007
 * @tc.desc  : Test AudioEndpointInner::MixToDupStream()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_007, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    std::vector<AudioStreamData> srcDataList;
    AudioStreamData audioStreamData;
    audioStreamData.isInnerCaped = true;
    srcDataList.push_back(audioStreamData);
    audioEndpointInner->dupBuffer_ = std::make_unique<uint8_t []>(1);
    EXPECT_NE(nullptr, audioEndpointInner->dupBuffer_);

    audioEndpointInner->MixToDupStream(srcDataList);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_008
 * @tc.desc  : Test AudioEndpointInner::ProcessData()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_008, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    std::vector<AudioStreamData> srcDataList;
    AudioStreamData dstData;
    AudioStreamData audioStreamData;
    audioStreamData.streamInfo.format = AudioSampleFormat::SAMPLE_S24LE;
    audioStreamData.streamInfo.channels = AudioChannel::CHANNEL_3;
    srcDataList.push_back(audioStreamData);

    audioEndpointInner->ProcessData(srcDataList, dstData);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_009
 * @tc.desc  : Test AudioEndpointInner::ProcessData()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_009, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    std::vector<AudioStreamData> srcDataList;
    AudioStreamData dstData;
    AudioStreamData audioStreamData;
    audioStreamData.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamData.streamInfo.channels = AudioChannel::CHANNEL_3;
    srcDataList.push_back(audioStreamData);

    audioEndpointInner->ProcessData(srcDataList, dstData);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_010
 * @tc.desc  : Test AudioEndpointInner::ProcessData()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_010, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    std::vector<AudioStreamData> srcDataList;
    AudioStreamData dstData;
    AudioStreamData audioStreamData;
    audioStreamData.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamData.streamInfo.channels = AudioChannel::STEREO;
    audioStreamData.bufferDesc.bufLength = 1;
    dstData.bufferDesc.bufLength = 2;
    srcDataList.push_back(audioStreamData);

    audioEndpointInner->ProcessData(srcDataList, dstData);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_011
 * @tc.desc  : Test AudioEndpointInner::ProcessData()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_011, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    std::vector<AudioStreamData> srcDataList;
    AudioStreamData dstData;
    AudioStreamData audioStreamData;
    audioStreamData.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamData.streamInfo.channels = AudioChannel::STEREO;
    audioStreamData.bufferDesc.bufLength = 1;
    dstData.bufferDesc.bufLength = 1;
    audioStreamData.bufferDesc.dataLength = 1;
    dstData.bufferDesc.dataLength = 2;
    srcDataList.push_back(audioStreamData);

    audioEndpointInner->ProcessData(srcDataList, dstData);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_012
 * @tc.desc  : Test AudioEndpointInner::ProcessData()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_012, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    std::vector<AudioStreamData> srcDataList;
    AudioStreamData dstData;
    AudioStreamData audioStreamData;
    audioStreamData.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioStreamData.streamInfo.channels = AudioChannel::STEREO;
    audioStreamData.bufferDesc.bufLength = 1;
    dstData.bufferDesc.bufLength = 1;
    audioStreamData.bufferDesc.dataLength = 1;
    dstData.bufferDesc.dataLength = 1;
    srcDataList.push_back(audioStreamData);

    audioEndpointInner->ProcessData(srcDataList, dstData);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_013
 * @tc.desc  : Test AudioEndpointInner::HandleRendererDataParams()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_013, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    AudioStreamData srcData;
    AudioStreamData dstData;
    srcData.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    dstData.streamInfo.encoding = AudioEncodingType::ENCODING_AUDIOVIVID;

    audioEndpointInner->HandleRendererDataParams(srcData, dstData);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_014
 * @tc.desc  : Test AudioEndpointInner::HandleRendererDataParams()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_014, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    AudioStreamData srcData;
    AudioStreamData dstData;
    srcData.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    dstData.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    srcData.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    srcData.streamInfo.channels = AudioChannel::STEREO;

    audioEndpointInner->HandleRendererDataParams(srcData, dstData);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_015
 * @tc.desc  : Test AudioEndpointInner::HandleRendererDataParams()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_015, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    AudioStreamData srcData;
    AudioStreamData dstData;
    srcData.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    dstData.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    srcData.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    srcData.streamInfo.channels = AudioChannel::CHANNEL_3;

    audioEndpointInner->HandleRendererDataParams(srcData, dstData);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_016
 * @tc.desc  : Test AudioEndpointInner::HandleRendererDataParams()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_016, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    AudioStreamData srcData;
    AudioStreamData dstData;
    srcData.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    dstData.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    srcData.streamInfo.format = AudioSampleFormat::SAMPLE_S24LE;
    srcData.streamInfo.channels = AudioChannel::CHANNEL_3;

    audioEndpointInner->HandleRendererDataParams(srcData, dstData);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_017
 * @tc.desc  : Test AudioEndpointInner::HandleRendererDataParams()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_017, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    AudioStreamData srcData;
    AudioStreamData dstData;
    srcData.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    dstData.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    srcData.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    srcData.streamInfo.channels = AudioChannel::MONO;

    audioEndpointInner->HandleRendererDataParams(srcData, dstData);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_018
 * @tc.desc  : Test AudioEndpointInner::GetAllReadyProcessData()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_018, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    std::vector<AudioStreamData> audioDataList;
    AudioStreamData audioStreamData;
    audioDataList.push_back(audioStreamData);

    audioEndpointInner->GetAllReadyProcessData(audioDataList);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_019
 * @tc.desc  : Test AudioEndpointInner::GetPredictNextReadTime()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_019, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    uint64_t posInFrame = 0;
    audioEndpointInner->dstSpanSizeInframe_ = 1;

    audioEndpointInner->GetPredictNextReadTime(posInFrame);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_020
 * @tc.desc  : Test AudioEndpointInner::GetPredictNextReadTime()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_020, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    uint64_t posInFrame = 400;
    audioEndpointInner->dstSpanSizeInframe_ = 1;

    audioEndpointInner->GetPredictNextReadTime(posInFrame);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_021
 * @tc.desc  : Test AudioEndpointInner::GetPredictNextReadTime()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_021, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    uint64_t posInFrame = 401;
    audioEndpointInner->dstSpanSizeInframe_ = 1;
    audioEndpointInner->readTimeModel_.isConfiged = true;
    audioEndpointInner->posInFrame_.store(13);

    audioEndpointInner->GetPredictNextReadTime(posInFrame);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_022
 * @tc.desc  : Test AudioEndpointInner::GetPredictNextReadTime()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_022, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    uint64_t posInFrame = 401;
    audioEndpointInner->dstSpanSizeInframe_ = 1;
    audioEndpointInner->readTimeModel_.isConfiged = true;
    audioEndpointInner->posInFrame_.store(0);

    audioEndpointInner->GetPredictNextReadTime(posInFrame);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_023
 * @tc.desc  : Test AudioEndpointInner::GetPredictNextReadTime()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_023, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    uint64_t posInFrame = 401;
    audioEndpointInner->dstSpanSizeInframe_ = 1;
    audioEndpointInner->readTimeModel_.isConfiged = false;

    audioEndpointInner->GetPredictNextReadTime(posInFrame);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_024
 * @tc.desc  : Test AudioEndpointInner::CheckPlaySignal()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_024, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    uint8_t buffer = 1;
    size_t bufferSize = 1;
    audioEndpointInner->latencyMeasEnabled_ = false;

    audioEndpointInner->CheckPlaySignal(&buffer, bufferSize);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_025
 * @tc.desc  : Test AudioEndpointInner::CheckPlaySignal()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_025, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    uint8_t buffer = 1;
    size_t bufferSize = BIGNUMBER;
    audioEndpointInner->dstStreamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    audioEndpointInner->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    audioEndpointInner->signalDetectAgent_->signalDetected_ = true;
    audioEndpointInner->signalDetectAgent_->dspTimestampGot_ = false;
    audioEndpointInner->latencyMeasEnabled_ = true;

    EXPECT_NE(nullptr, audioEndpointInner->signalDetectAgent_);

    audioEndpointInner->CheckPlaySignal(&buffer, bufferSize);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_026
 * @tc.desc  : Test AudioEndpointInner::CheckPlaySignal()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_026, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    uint8_t buffer = 1;
    size_t bufferSize = BIGNUMBER;
    audioEndpointInner->dstStreamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    audioEndpointInner->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    audioEndpointInner->signalDetectAgent_->signalDetected_ = false;
    audioEndpointInner->signalDetectAgent_->dspTimestampGot_ = false;
    audioEndpointInner->latencyMeasEnabled_ = true;

    EXPECT_NE(nullptr, audioEndpointInner->signalDetectAgent_);

    audioEndpointInner->CheckPlaySignal(&buffer, bufferSize);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_027
 * @tc.desc  : Test AudioEndpointInner::CheckPlaySignal()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_027, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    uint8_t buffer = 1;
    size_t bufferSize = BIGNUMBER;
    audioEndpointInner->dstStreamInfo_.format = AudioSampleFormat::SAMPLE_S16LE;
    audioEndpointInner->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    audioEndpointInner->signalDetectAgent_->signalDetected_ = true;
    audioEndpointInner->signalDetectAgent_->dspTimestampGot_ = true;
    audioEndpointInner->latencyMeasEnabled_ = true;

    EXPECT_NE(nullptr, audioEndpointInner->signalDetectAgent_);

    audioEndpointInner->CheckPlaySignal(&buffer, bufferSize);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_028
 * @tc.desc  : Test AudioEndpointInner::HandleCapturerDataParams()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_028, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    BufferDesc writeBuf;
    BufferDesc readBuf;
    BufferDesc convertedBuffer;
    audioEndpointInner->clientConfig_.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioEndpointInner->clientConfig_.streamInfo.channels = AudioChannel::STEREO;

    audioEndpointInner->HandleCapturerDataParams(writeBuf, readBuf, convertedBuffer);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_029
 * @tc.desc  : Test AudioEndpointInner::HandleCapturerDataParams()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_029, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    BufferDesc writeBuf;
    BufferDesc readBuf;
    BufferDesc convertedBuffer;
    audioEndpointInner->clientConfig_.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioEndpointInner->clientConfig_.streamInfo.channels = AudioChannel::CHANNEL_3;

    audioEndpointInner->HandleCapturerDataParams(writeBuf, readBuf, convertedBuffer);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_030
 * @tc.desc  : Test AudioEndpointInner::HandleCapturerDataParams()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_030, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    BufferDesc writeBuf;
    BufferDesc readBuf;
    BufferDesc convertedBuffer;
    audioEndpointInner->clientConfig_.streamInfo.format = AudioSampleFormat::SAMPLE_S24LE;
    audioEndpointInner->clientConfig_.streamInfo.channels = AudioChannel::CHANNEL_3;

    audioEndpointInner->HandleCapturerDataParams(writeBuf, readBuf, convertedBuffer);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_031
 * @tc.desc  : Test AudioEndpointInner::HandleCapturerDataParams()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_031, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    EXPECT_NE(nullptr, audioEndpointInner);

    BufferDesc writeBuf;
    BufferDesc readBuf;
    BufferDesc convertedBuffer;
    audioEndpointInner->clientConfig_.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioEndpointInner->clientConfig_.streamInfo.channels = AudioChannel::STEREO;

    audioEndpointInner->HandleCapturerDataParams(writeBuf, readBuf, convertedBuffer);
}
} // namespace AudioStandard
} // namespace OHOS
