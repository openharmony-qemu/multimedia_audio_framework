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

#include "audio_endpoint_plus_unit_test.h"
#include "pa_adapter_manager.h"
#include "accesstoken_kit.h"
#include "audio_device_info.h"
#include "audio_endpoint.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_process_config.h"
#include "audio_server.h"
#include "audio_service.h"
#include "audio_stream_info.h"
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

static const size_t BIGNUMBER = 2808348670;
static const size_t NUMFIVE = 5;
static constexpr uint32_t MORE_SESSIONID = MAX_STREAMID + 1;
static const int32_t CAPTURER_FLAG = 10;
static const uint32_t SESSIONID = 123456;

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

    int64_t checkTime = 0;
    uint64_t curWritePos = 0;

    auto result = audioEndpointInner->CheckAllBufferReady(checkTime, curWritePos);
    EXPECT_EQ(result, true);
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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    auto result = audioEndpointInner->CheckAllBufferReady(checkTime, curWritePos);
    EXPECT_EQ(result, true);
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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

    uint8_t buffer = 1;
    size_t bufferSize = 1;
    audioEndpointInner->latencyMeasEnabled_ = false;

    audioEndpointInner->CheckPlaySignal(&buffer, bufferSize);
}
#ifdef AUDIO_ENDPOINT_INNER_UNIT_TEST_DIFF
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

    ASSERT_NE(audioEndpointInner, nullptr);

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
#endif
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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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

    ASSERT_NE(audioEndpointInner, nullptr);

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
 * @tc.number: AudioEndpointInner_032
 * @tc.desc  : Test AudioEndpointInner::CheckRecordSignal()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_032, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    uint8_t* buffer = new uint8_t[NUMFIVE];
    size_t bufferSize = NUMFIVE;
    audioEndpointInner->latencyMeasEnabled_ = true;
    audioEndpointInner->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    EXPECT_NE(nullptr, audioEndpointInner->signalDetectAgent_);
    audioEndpointInner->signalDetected_ = true;

    audioEndpointInner->CheckRecordSignal(buffer, bufferSize);
    delete[] buffer;
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_033
 * @tc.desc  : Test AudioEndpointInner::CheckRecordSignal()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_033, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    uint8_t* buffer = new uint8_t[NUMFIVE];
    size_t bufferSize = NUMFIVE;
    audioEndpointInner->latencyMeasEnabled_ = true;
    audioEndpointInner->signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    EXPECT_NE(nullptr, audioEndpointInner->signalDetectAgent_);
    audioEndpointInner->signalDetected_ = false;

    audioEndpointInner->CheckRecordSignal(buffer, bufferSize);
    delete[] buffer;
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_034
 * @tc.desc  : Test AudioEndpointInner::WriteToSpecialProcBuf()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_034, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    const std::shared_ptr<OHAudioBuffer> procBuf = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    procBuf->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();

    EXPECT_NE(nullptr, procBuf);

    BufferDesc readBuf2;
    const BufferDesc readBuf = readBuf2;
    BufferDesc convertedBuffer2;
    const BufferDesc convertedBuffer = convertedBuffer2;
    bool muteFlag = true;

    audioEndpointInner->WriteToSpecialProcBuf(procBuf, readBuf, convertedBuffer, muteFlag);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_035
 * @tc.desc  : Test AudioEndpointInner::PrepareNextLoop()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_035, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    uint64_t curWritePos = 0;
    int64_t wakeUpTime = 0;
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    audioEndpointInner->dstAudioBuffer_ = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    audioEndpointInner->dstAudioBuffer_->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();

    auto result = audioEndpointInner->PrepareNextLoop(curWritePos, wakeUpTime);
    EXPECT_EQ(result, false);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_036
 * @tc.desc  : Test AudioEndpointInner::GetMaxAmplitude()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_036, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    auto result = audioEndpointInner->GetMaxAmplitude();

    EXPECT_EQ(audioEndpointInner->startUpdate_, true);
    EXPECT_NEAR(result, 0.0, 0.001);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_037
 * @tc.desc  : Test AudioEndpointInner::ProcessToDupStream()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_037, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    AudioStreamData dstStreamData;
    dstStreamData.isInnerCaped = true;
    const std::vector<AudioStreamData> audioDataList = {dstStreamData};
    audioEndpointInner->endpointType_ = AudioEndpoint::EndpointType::TYPE_VOIP_MMAP;

    EXPECT_EQ(audioDataList.size(), 1);

    audioEndpointInner->dupBufferSize_ = 3;

    PaAdapterManager *adapterManager = new PaAdapterManager(DUP_PLAYBACK);
    adapterManager->InitPaContext();
    AudioProcessConfig config;
    config.appInfo.appUid = CAPTURER_FLAG;
    config.appInfo.appPid = CAPTURER_FLAG;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    config.originalSessionId = MORE_SESSIONID;
    uint32_t sessionId = SESSIONID;
    pa_stream *stream = adapterManager->InitPaStream(config, sessionId, false);
    audioEndpointInner->dupStream_ = adapterManager->CreateRendererStream(config, stream);

    audioEndpointInner->ProcessToDupStream(audioDataList, dstStreamData);

    EXPECT_EQ(dstStreamData.bufferDesc.bufLength, audioEndpointInner->dupBufferSize_);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_039
 * @tc.desc  : Test AudioEndpointInner::GetAllReadyProcessData()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_039, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    AudioStreamData dstStreamData;
    std::vector<AudioStreamData> audioDataList = {dstStreamData};
    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    std::shared_ptr<OHAudioBuffer> processBuffer = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    EXPECT_NE(processBuffer, nullptr);

    processBuffer->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();
    EXPECT_NE(processBuffer->basicBufferInfo_, nullptr);

    audioEndpointInner->processBufferList_.push_back(processBuffer);

    audioEndpointInner->GetAllReadyProcessData(audioDataList);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_040
 * @tc.desc  : Test AudioEndpointInner::WaitAllProcessReady()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_040, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    uint64_t curWritePos = 0;

    audioEndpointInner->WaitAllProcessReady(curWritePos);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_041
 * @tc.desc  : Test AudioEndpointInner::ProcessToEndpointDataHandle()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_041, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    ASSERT_NE(audioEndpointInner, nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t spanSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;
    audioEndpointInner->dstAudioBuffer_ = std::make_shared<OHAudioBuffer>(bufferHolder, totalSizeInFrame,
        spanSizeInFrame, byteSizePerFrame);
    EXPECT_NE(audioEndpointInner->dstAudioBuffer_, nullptr);

    audioEndpointInner->dstAudioBuffer_->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();
    EXPECT_NE(audioEndpointInner->dstAudioBuffer_->basicBufferInfo_, nullptr);

    uint64_t curWritePos = 0;
    auto result = audioEndpointInner->ProcessToEndpointDataHandle(curWritePos);
    EXPECT_EQ(result, false);
}
} // namespace AudioStandard
} // namespace OHOS
