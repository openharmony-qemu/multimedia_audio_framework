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
#include "audio_system_manager.h"
#include "audio_utils.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class MockAudioProcessStream : public IAudioProcessStream {
public:
    // Pure virtual methods
    MOCK_METHOD(std::shared_ptr<OHAudioBufferBase>, GetStreamBuffer, (), (override));
    MOCK_METHOD(AudioStreamInfo, GetStreamInfo, (), (override));
    MOCK_METHOD(uint32_t, GetAudioSessionId, (), (override));
    MOCK_METHOD(AudioStreamType, GetAudioStreamType, (), (override));

    MOCK_METHOD(void, SetInnerCapState, (bool isInnerCapped, int32_t innerCapId), (override));
    MOCK_METHOD(bool, GetInnerCapState, (int32_t innerCapId), (override));

    using RetTypeUnorderedMap = std::unordered_map<int32_t, bool>;
    MOCK_METHOD(RetTypeUnorderedMap, GetInnerCapState, (), (override));

    MOCK_METHOD(AppInfo, GetAppInfo, (), (override));
    MOCK_METHOD(BufferDesc&, GetConvertedBuffer, (), (override));
    MOCK_METHOD(bool, GetMuteState, (), (override));
    MOCK_METHOD(AudioProcessConfig, GetAudioProcessConfig, (), (override));
    MOCK_METHOD(void, WriteDumpFile, (void* buffer, size_t bufferSize), (override));

    MOCK_METHOD(int32_t, SetDefaultOutputDevice, (int32_t defaultOutputDevice, bool skipForce), (override));
    MOCK_METHOD(int32_t, SetSilentModeAndMixWithOthers, (bool on), (override));

    MOCK_METHOD(uint32_t, GetSpanSizeInFrame, (), (override));
    MOCK_METHOD(uint32_t, GetByteSizePerFrame, (), (override));

    // Non-pure virtual methods (with default implementations in interface)
    MOCK_METHOD(void, EnableStandby, (), (override));

    // Time and state control
    MOCK_METHOD(std::time_t, GetStartMuteTime, (), (override));
    MOCK_METHOD(void, SetStartMuteTime, (std::time_t time), (override));

    MOCK_METHOD(bool, GetSilentState, (), (override));
    MOCK_METHOD(void, SetSilentState, (bool state), (override));

    MOCK_METHOD(void, AddMuteWriteFrameCnt, (int64_t muteFrameCnt), (override));
    MOCK_METHOD(void, AddMuteFrameSize, (int64_t muteFrameCnt), (override));
    MOCK_METHOD(void, AddNormalFrameSize, (), (override));
    MOCK_METHOD(void, AddNoDataFrameSize, (), (override));

    MOCK_METHOD(StreamStatus, GetStreamStatus, (), (override));

    // Audio-Haptics sync
    MOCK_METHOD(int32_t, SetAudioHapticsSyncId, (int32_t audioHapticsSyncId), (override));
    MOCK_METHOD(int32_t, GetAudioHapticsSyncId, (), (override));
};

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
#ifdef SUPPORT_OLD_ENGINE
static constexpr uint32_t MORE_SESSIONID = MAX_STREAMID + 1;
static const int32_t CAPTURER_FLAG = 10;
static const uint32_t SESSIONID = 123456;
#endif

constexpr int32_t DEFAULT_STREAM_ID = 10;

static AudioProcessConfig InitServerProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

static sptr<AudioProcessInServer> CreateAudioProcessInServer()
{
    AudioService *audioServicePtr = AudioService::GetInstance();
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    AudioStreamInfo audioStreamInfo;
    audioStreamInfo.samplingRate = SAMPLE_RATE_48000;
    audioStreamInfo.channelLayout = CH_LAYOUT_STEREO;
    AudioProcessConfig serverConfig = InitServerProcessConfig();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(serverConfig, audioServicePtr);
    std::shared_ptr<OHAudioBufferBase> buffer = nullptr;
    uint32_t spanSizeInFrame = 1000;
    uint32_t totalSizeInFrame = spanSizeInFrame;
    processStream->ConfigProcessBuffer(totalSizeInFrame, spanSizeInFrame, audioStreamInfo, buffer);
    return processStream;
}

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
    uint32_t byteSizePerFrame = 0;
    std::shared_ptr<OHAudioBufferBase> processBuffer = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    BasicBufferInfo basicBufferInfo;
    processBuffer->basicBufferInfo_ = &basicBufferInfo;
    processBuffer->basicBufferInfo_->streamStatus.store(StreamStatus::STREAM_RUNNING);
    uint64_t readFrame = 0;
    processBuffer->SetCurReadFrame(readFrame);
    int64_t lastTime = 0;
    processBuffer->SetLastWrittenTime(lastTime);
    uint64_t pos = 0;
    processBuffer->basicBufferInfo_->basePosInFrame.store(pos);
    AudioProcessConfig config = {};
    config.privacyType = AudioPrivacyType::PRIVACY_TYPE_PUBLIC;
    sptr<AudioProcessInServer> audioProcess = AudioProcessInServer::Create(config, AudioService::GetInstance());
    audioEndpointInner->processList_.push_back(audioProcess);
    audioEndpointInner->processBufferList_.push_back(processBuffer);
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(clientConfig, g_audioServicePtr);
    audioEndpointInner->processList_.push_back(processStream);
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
    uint32_t byteSizePerFrame = 0;
    std::shared_ptr<OHAudioBufferBase> processBuffer = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    BasicBufferInfo basicBufferInfo;
    processBuffer->basicBufferInfo_ = &basicBufferInfo;
    processBuffer->basicBufferInfo_->streamStatus.store(StreamStatus::STREAM_STARTING);
    audioEndpointInner->processBufferList_.push_back(processBuffer);
    AudioService *g_audioServicePtr = AudioService::GetInstance();
    sptr<AudioProcessInServer> processStream = AudioProcessInServer::Create(clientConfig, g_audioServicePtr);
    audioEndpointInner->processList_.push_back(processStream);
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
    srcDataList.push_back(audioStreamData);
    audioEndpointInner->dupBuffer_ = std::make_unique<uint8_t []>(1);
    EXPECT_NE(nullptr, audioEndpointInner->dupBuffer_);

    audioEndpointInner->MixToDupStream(srcDataList, 1);
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
    audioStreamData.isInnerCapeds[1] = true;
    srcDataList.push_back(audioStreamData);
    audioEndpointInner->dupBuffer_ = std::make_unique<uint8_t []>(1);
    EXPECT_NE(nullptr, audioEndpointInner->dupBuffer_);

    audioEndpointInner->MixToDupStream(srcDataList, 1);
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

    clientConfig.streamInfo.samplingRate = SAMPLE_RATE_48000;
    clientConfig.streamInfo.encoding = AudioEncodingType::ENCODING_PCM;
    clientConfig.streamInfo.format = SAMPLE_F32LE;
    clientConfig.streamInfo.channels = STEREO;
    sptr<IAudioProcess> process = AudioService::GetInstance()->GetAudioProcess(clientConfig);
    srcData.streamInfo.format = AudioSampleFormat::SAMPLE_F32LE;

    audioEndpointInner->HandleRendererDataParams(srcData, dstData);

    srcData.streamInfo.channels = AudioChannel::STEREO;

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

    std::function<void()> moveClientIndex;
    audioEndpointInner->GetAllReadyProcessData(audioDataList, moveClientIndex);
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
    audioEndpointInner->readTimeModel_.sampleRate_ = 1000;
    audioEndpointInner->posInFrame_.store(13);
    audioEndpointInner->stopUpdateThread_ = true;

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
    audioEndpointInner->readTimeModel_.sampleRate_ = 1000;
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
    audioEndpointInner->readTimeModel_.sampleRate_ = 1000;

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
    audioEndpointInner->dstStreamInfo_.samplingRate = SAMPLE_RATE_8000;
    audioEndpointInner->dstStreamInfo_.channels = AudioChannel::STEREO;
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
    audioEndpointInner->dstStreamInfo_.samplingRate = SAMPLE_RATE_8000;
    audioEndpointInner->dstStreamInfo_.channels = AudioChannel::STEREO;
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
    int32_t ret = 0;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    RingBufferWrapper writeBuf;
    BufferDesc readBuf;
    BufferDesc convertedBuffer;
    audioEndpointInner->clientConfig_.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioEndpointInner->clientConfig_.streamInfo.channels = AudioChannel::STEREO;

    ret = audioEndpointInner->HandleCapturerDataParams(writeBuf, readBuf, convertedBuffer);
    EXPECT_EQ(ret, ERR_WRITE_FAILED);

    audioEndpointInner->clientConfig_.streamInfo.format = AudioSampleFormat::SAMPLE_F32LE;
    audioEndpointInner->clientConfig_.streamInfo.channels = AudioChannel::STEREO;

    ret = audioEndpointInner->HandleCapturerDataParams(writeBuf, readBuf, convertedBuffer);
    EXPECT_NE(ret, 0);

    audioEndpointInner->clientConfig_.streamInfo.format = AudioSampleFormat::SAMPLE_F32LE;
    audioEndpointInner->clientConfig_.streamInfo.channels = AudioChannel::MONO;

    ret = audioEndpointInner->HandleCapturerDataParams(writeBuf, readBuf, convertedBuffer);
    EXPECT_NE(ret, 0);
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
    int32_t ret = 0;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    RingBufferWrapper writeBuf;
    BufferDesc readBuf;
    BufferDesc convertedBuffer;
    audioEndpointInner->clientConfig_.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioEndpointInner->clientConfig_.streamInfo.channels = AudioChannel::CHANNEL_3;

    ret = audioEndpointInner->HandleCapturerDataParams(writeBuf, readBuf, convertedBuffer);
    EXPECT_NE(ret, 0);
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
    int32_t ret = 0;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    RingBufferWrapper writeBuf;
    BufferDesc readBuf;
    BufferDesc convertedBuffer;
    audioEndpointInner->clientConfig_.streamInfo.format = AudioSampleFormat::SAMPLE_S24LE;
    audioEndpointInner->clientConfig_.streamInfo.channels = AudioChannel::CHANNEL_3;

    ret = audioEndpointInner->HandleCapturerDataParams(writeBuf, readBuf, convertedBuffer);
    EXPECT_NE(ret, 0);
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
    int32_t ret = 0;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    RingBufferWrapper writeBuf;
    BufferDesc readBuf;
    BufferDesc convertedBuffer;
    audioEndpointInner->clientConfig_.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
    audioEndpointInner->clientConfig_.streamInfo.channels = AudioChannel::STEREO;

    ret = audioEndpointInner->HandleCapturerDataParams(writeBuf, readBuf, convertedBuffer);
    EXPECT_EQ(ret, ERR_WRITE_FAILED);
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
    const std::shared_ptr<OHAudioBufferBase> procBuf = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
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
    audioEndpointInner->dstAudioBuffer_->ohAudioBufferBase_.basicBufferInfo_ =
        std::make_shared<BasicBufferInfo>().get();

    std::function<void()> moveClientIndex;
    auto result = audioEndpointInner->PrepareNextLoop(curWritePos, wakeUpTime, moveClientIndex);
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
#ifdef HAS_FEATURE_INNERCAPTURER
#ifdef SUPPORT_OLD_ENGINE
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_037, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    AudioStreamData dstStreamData;
    dstStreamData.isInnerCapeds[1] = true;
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
    config.innerCapId = 1;
    uint32_t sessionId = SESSIONID;
    setuid(AUDIO_ID);
    AudioPlaybackCaptureConfig checkConfig;
    int32_t checkInnerCapId = 0;
    AudioSystemManager::GetInstance()->CheckCaptureLimit(checkConfig, checkInnerCapId);
    pa_stream *stream = adapterManager->InitPaStream(config, sessionId, false);
    auto &info = audioEndpointInner->fastCaptureInfos_[1];
    info.dupStream = adapterManager->CreateRendererStream(config, stream);
    audioEndpointInner->ProcessToDupStream(audioDataList, dstStreamData, 1);
    AudioSystemManager::GetInstance()->ReleaseCaptureLimit(1);
    EXPECT_EQ(dstStreamData.bufferDesc.bufLength, audioDataList[0].bufferDesc.bufLength);
}
#endif
#endif
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
    uint32_t byteSizePerFrame = 0;
    std::shared_ptr<OHAudioBufferBase> processBuffer = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    EXPECT_NE(processBuffer, nullptr);

    processBuffer->basicBufferInfo_ = std::make_shared<BasicBufferInfo>().get();
    EXPECT_NE(processBuffer->basicBufferInfo_, nullptr);

    audioEndpointInner->processBufferList_.push_back(processBuffer);
    MockAudioProcessStream mockProcessStream;
    audioEndpointInner->processList_.push_back(&mockProcessStream);

    std::function<void()> moveClientIndex;
    audioEndpointInner->GetAllReadyProcessData(audioDataList, moveClientIndex);
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

    audioEndpointInner->dstAudioBuffer_->ohAudioBufferBase_.basicBufferInfo_ =
        std::make_shared<BasicBufferInfo>().get();
    EXPECT_NE(audioEndpointInner->dstAudioBuffer_->ohAudioBufferBase_.basicBufferInfo_, nullptr);

    uint64_t curWritePos = 0;
    std::function<void()> moveClientIndex;
    auto result = audioEndpointInner->ProcessToEndpointDataHandle(curWritePos, moveClientIndex);
    EXPECT_EQ(result, false);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_042
 * @tc.desc  : Test AudioEndpointInner::WriteMuteDataSysEvent()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_042, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    ASSERT_NE(audioEndpointInner, nullptr);

    AudioBufferHolder bufferHolder = AudioBufferHolder::AUDIO_CLIENT;
    uint32_t totalSizeInFrame = 0;
    uint32_t byteSizePerFrame = 0;

    std::shared_ptr<OHAudioBufferBase> processBuffer1 = std::make_shared<OHAudioBufferBase>(bufferHolder,
        totalSizeInFrame, byteSizePerFrame);
    sptr<AudioProcessInServer> audioProcess1 = AudioProcessInServer::Create(clientConfig, AudioService::GetInstance());
    size_t len = 10;
    std::unique_ptr<int8_t[]> buffer1 = std::make_unique<int8_t[]>(len);
    for (size_t i = 0; i < len; ++i) {
        buffer1[i] = static_cast<int8_t>(i);
    }
    BufferDesc bufferDesc1 = {reinterpret_cast<uint8_t *>(buffer1.get()), len, len};
    bufferDesc1.buffer[0] = 1;
    bufferDesc1.buffer[1] = 1;
    audioEndpointInner->processList_.push_back(audioProcess1);
    audioEndpointInner->processBufferList_.push_back(processBuffer1);
    audioEndpointInner->WriteMuteDataSysEvent(bufferDesc1.buffer, bufferDesc1.bufLength, 0);
    EXPECT_EQ(false, audioEndpointInner->processList_[0]->GetSilentState());
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_043
 * @tc.desc  : Test AudioEndpointInner::GetEndpointType()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_043, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    AudioEndpoint::EndpointType endpointType = audioEndpointInner->GetEndpointType();
    EXPECT_EQ(endpointType, AudioEndpoint::TYPE_MMAP);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_044
 * @tc.desc  : Test AudioEndpointInner::GetBuffer()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_044, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    std::shared_ptr<OHAudioBufferBase> buffer = audioEndpointInner->GetBuffer();
    EXPECT_EQ(buffer, nullptr);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_045
 * @tc.desc  : Test AudioEndpointInner::GetDeviceInfo()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_045, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    AudioDeviceDescriptor audioDeviceDescriptor = audioEndpointInner->GetDeviceInfo();
    EXPECT_EQ(audioDeviceDescriptor.descriptorType_, AudioDeviceDescriptor::DEVICE_INFO);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_046
 * @tc.desc  : Test AudioEndpointInner::GetDeviceRole()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_046, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    audioEndpointInner->deviceInfo_.deviceRole_ = DeviceRole::INPUT_DEVICE;
    DeviceRole deviceRole = audioEndpointInner->GetDeviceRole();
    EXPECT_EQ(deviceRole, DeviceRole::INPUT_DEVICE);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_047
 * @tc.desc  : Test AudioEndpointInner::GetStatus()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_047, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    AudioEndpoint::EndpointStatus endpointStatus = audioEndpointInner->GetStatus();
    EXPECT_EQ(endpointStatus, AudioEndpoint::EndpointStatus::INVALID);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_048
 * @tc.desc  : Test AudioEndpointInner::GetFastSource()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_048, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    std::string networkId = "RemoteDevice";
    IAudioSourceAttr attr = {};
    audioEndpointInner->GetFastSource(networkId, type, attr);
}

/*
 * @tc.name  : Test MockCallbacks API
 * @tc.type  : FUNC
 * @tc.number: MockCallbacks_049
 * @tc.desc  : Test MockCallbacks::OnStatusUpdate()
 */
HWTEST_F(AudioEndpointPlusUnitTest, MockCallbacks_049, TestSize.Level1)
{
    uint32_t streamIndex = 0;
    auto mockCallbacks = std::make_shared<MockCallbacks>(streamIndex);

    ASSERT_NE(mockCallbacks, nullptr);

    IOperation operation = IOperation::OPERATION_STARTED;
    mockCallbacks->OnStatusUpdate(operation);
}

/*
 * @tc.name  : Test MockCallbacks API
 * @tc.type  : FUNC
 * @tc.number: MockCallbacks_050
 * @tc.desc  : Test MockCallbacks::OnWriteData()
 */
HWTEST_F(AudioEndpointPlusUnitTest, MockCallbacks_050, TestSize.Level1)
{
    uint32_t streamIndex = 0;
    auto mockCallbacks = std::make_shared<MockCallbacks>(streamIndex);

    ASSERT_NE(mockCallbacks, nullptr);

    size_t length = 8;
    int32_t ret = mockCallbacks->OnWriteData(length);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_051
 * @tc.desc  : Test AudioEndpointInner::RecordReSyncPosition()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_051, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    audioEndpointInner->spanDuration_ = -999;
    audioEndpointInner->RecordReSyncPosition();
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_052
 * @tc.desc  : Test AudioEndpointInner::LinkProcessStream()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_052, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    sptr<AudioProcessInServer> processStream = CreateAudioProcessInServer();
    audioEndpointInner->endpointStatus_ = AudioEndpoint::STARTING;
    int32_t ret = audioEndpointInner->LinkProcessStream(processStream);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_053
 * @tc.desc  : Test AudioEndpointInner::LinkProcessStream()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_053, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    sptr<AudioProcessInServer> processStream = CreateAudioProcessInServer();
    audioEndpointInner->endpointStatus_ = AudioEndpoint::RUNNING;
    int32_t ret = audioEndpointInner->LinkProcessStream(processStream);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_054
 * @tc.desc  : Test AudioEndpointInner::LinkProcessStream()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_054, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    sptr<AudioProcessInServer> processStream = CreateAudioProcessInServer();
    audioEndpointInner->endpointStatus_ = AudioEndpoint::IDEL;
    audioEndpointInner->isDeviceRunningInIdel_ = true;
    int32_t ret = audioEndpointInner->LinkProcessStream(processStream);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_055
 * @tc.desc  : Test AudioEndpointInner::LinkProcessStream()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_055, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    sptr<AudioProcessInServer> processStream = CreateAudioProcessInServer();
    audioEndpointInner->endpointStatus_ = AudioEndpoint::IDEL;
    audioEndpointInner->isDeviceRunningInIdel_ = false;
    int32_t ret = audioEndpointInner->LinkProcessStream(processStream);
    EXPECT_EQ(ret, SUCCESS);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_056
 * @tc.desc  : Test AudioEndpointInner::CheckStandBy()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_056, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    audioEndpointInner->endpointStatus_ = AudioEndpoint::EndpointStatus::STARTING;
    audioEndpointInner->CheckStandBy();
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_057
 * @tc.desc  : Test AudioEndpointInner::LinkProcessStreamExt()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_057, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    sptr<AudioProcessInServer> processStream = CreateAudioProcessInServer();
    std::shared_ptr<OHAudioBufferBase> processBuffer;

    audioEndpointInner->LinkProcessStreamExt(processStream, processBuffer);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_058
 * @tc.desc  : Test AudioEndpointInner::GetDeviceHandleInfo()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_058, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    uint64_t frames = 0;
    int64_t nanoTime = 0;
    audioEndpointInner->deviceInfo_.deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    audioEndpointInner->fastRenderId_ = HDI_INVALID_ID;
    bool ret = audioEndpointInner->GetDeviceHandleInfo(frames, nanoTime);
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_059
 * @tc.desc  : Test AudioEndpointInner::IsBufferDataInsufficient()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_059, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    bool ret = audioEndpointInner->IsBufferDataInsufficient(0, 1);
    EXPECT_EQ(ret, true);

    ret = audioEndpointInner->IsBufferDataInsufficient(1, 1);
    EXPECT_EQ(ret, false);

    ret = audioEndpointInner->IsBufferDataInsufficient(-1, 1);
    EXPECT_EQ(ret, false);

    ret = audioEndpointInner->IsBufferDataInsufficient(ERROR, std::numeric_limits<int32_t>::max());
    EXPECT_EQ(ret, false);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_060
 * @tc.desc  : Test AudioEndpointInner::NeedUseTempBuffer()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_060, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    std::vector<uint8_t> buffer1(1, 0);
    std::vector<uint8_t> buffer2(1, 0);
    RingBufferWrapper ringBuffer = {
        {{
            {.buffer = buffer1.data(), .bufLength = 1},
            {.buffer = buffer2.data(), .bufLength = 1},
        }},
        // 1 + 1 = 2
        .dataLength = 2
    };
    auto ret = audioEndpointInner->NeedUseTempBuffer(ringBuffer, 1);
    EXPECT_EQ(ret, true);

    ringBuffer.dataLength = 1;
    ret = audioEndpointInner->NeedUseTempBuffer(ringBuffer, 1);
    EXPECT_EQ(ret, false);

    // 2 > 1
    ret = audioEndpointInner->NeedUseTempBuffer(ringBuffer, 2);
    EXPECT_EQ(ret, true);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: AudioEndpointInner_061
 * @tc.desc  : Test AudioEndpointInner::PrepareStreamDataBuffer()
 */
HWTEST_F(AudioEndpointPlusUnitTest, AudioEndpointInner_061, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);

    ASSERT_NE(audioEndpointInner, nullptr);

    audioEndpointInner->processTmpBufferList_.resize(1);

    std::vector<uint8_t> buffer1(1, 0);
    RingBufferWrapper ringBuffer = {
        {{
            {.buffer = buffer1.data(), .bufLength = 1},
            {.buffer = nullptr, .bufLength = 0},
        }},
        .dataLength = 1
    };
    AudioStreamData streamData;
    audioEndpointInner->PrepareStreamDataBuffer(0, 1, ringBuffer, streamData);
    // spansizeinframe == 2; spansizeinframe > datalenth
    audioEndpointInner->PrepareStreamDataBuffer(0, 2, ringBuffer, streamData);

    // processTmpBufferList[i] == spansizeinframe
    EXPECT_EQ(audioEndpointInner->processTmpBufferList_[0].size(), 2);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_001
 * @tc.desc  : Test AudioEndpointInner::IsInvalidBuffer()
 */
HWTEST_F(AudioEndpointPlusUnitTest, IsInvalidBuffer_001, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    uint8_t buffer[1] = {1};
    bool result = audioEndpointInner->IsInvalidBuffer(buffer, sizeof(buffer), SAMPLE_U8);
    EXPECT_FALSE(result);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_002
 * @tc.desc  : Test AudioEndpointInner::IsInvalidBuffer()
 */
HWTEST_F(AudioEndpointPlusUnitTest, IsInvalidBuffer_002, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    uint8_t buffer[1] = {0};
    bool result = audioEndpointInner->IsInvalidBuffer(buffer, sizeof(buffer), SAMPLE_U8);
    EXPECT_TRUE(result);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_003
 * @tc.desc  : Test AudioEndpointInner::IsInvalidBuffer()
 */
HWTEST_F(AudioEndpointPlusUnitTest, IsInvalidBuffer_003, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    int16_t buffer[1] = {1};
    bool result = audioEndpointInner->IsInvalidBuffer(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer), SAMPLE_S16LE);
    EXPECT_FALSE(result);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_004
 * @tc.desc  : Test AudioEndpointInner::IsInvalidBuffer()
 */
HWTEST_F(AudioEndpointPlusUnitTest, IsInvalidBuffer_004, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointInner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    int16_t buffer[1] = {0};
    bool result = audioEndpointInner->IsInvalidBuffer(reinterpret_cast<uint8_t*>(buffer), sizeof(buffer), SAMPLE_S16LE);
    EXPECT_TRUE(result);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: IsInvalidBuffer_005
 * @tc.desc  : Test AudioEndpointInner::IsInvalidBuffer()
 */
HWTEST_F(AudioEndpointPlusUnitTest, IsInvalidBuffer_005, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointnIner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    uint8_t buffer[1] = {0};
    bool result = audioEndpointnIner->IsInvalidBuffer(buffer, sizeof(buffer), static_cast<AudioSampleFormat>(-1));
    EXPECT_FALSE(result);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: CheckAudioHapticsSync_001
 * @tc.desc  : Test AudioEndpointInner::CheckAudioHapticsSync()
 */
HWTEST_F(AudioEndpointPlusUnitTest, CheckAudioHapticsSync_001, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointnIner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    audioEndpointnIner->audioHapticsSyncId_ = 1;
    audioEndpointnIner->fastRenderId_ = 1;
    audioEndpointnIner->dstSpanSizeInframe_ = 100;
    std::shared_ptr<IAudioRenderSink> sink = nullptr;
    HdiAdapterManager::GetInstance().DoSetSinkPrestoreInfo(sink);
    audioEndpointnIner->CheckAudioHapticsSync(10);
    EXPECT_EQ(audioEndpointnIner->audioHapticsSyncId_, 0);
}

/*
 * @tc.name  : Test AudioEndpointInner API
 * @tc.type  : FUNC
 * @tc.number: CheckAudioHapticsSync_002
 * @tc.desc  : Test AudioEndpointInner::CheckAudioHapticsSync()
 */
HWTEST_F(AudioEndpointPlusUnitTest, CheckAudioHapticsSync_002, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointnIner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    audioEndpointnIner->audioHapticsSyncId_ = 1;
    audioEndpointnIner->fastRenderId_ = 1;
    audioEndpointnIner->dstSpanSizeInframe_ = 100;

    HdiAdapterManager::GetInstance().DoSetSinkPrestoreInfo(nullptr);

    audioEndpointnIner->CheckAudioHapticsSync(10);

    EXPECT_NE(audioEndpointnIner->audioHapticsSyncId_, 1);
}

/*
 * @tc.name  : Test IsNearlinkAbsVolSupportStream API
 * @tc.type  : FUNC
 * @tc.number: IsNearlinkAbsVolSupportStream_001
 * @tc.desc  : Test AudioEndpointInner::IsNearlinkAbsVolSupportStream()
 */
HWTEST_F(AudioEndpointPlusUnitTest, IsNearlinkAbsVolSupportStream_001, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointnIner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    EXPECT_TRUE(audioEndpointnIner->IsNearlinkAbsVolSupportStream(DEVICE_TYPE_NEARLINK, STREAM_MUSIC));

    EXPECT_TRUE(audioEndpointnIner->IsNearlinkAbsVolSupportStream(DEVICE_TYPE_NEARLINK, STREAM_VOICE_CALL));
}

/*
 * @tc.name  : Test CheckSyncInfo API
 * @tc.type  : FUNC
 * @tc.number: CheckSyncInfo_001
 * @tc.desc  : Test AudioEndpointInner::CheckSyncInfo()
 */
HWTEST_F(AudioEndpointPlusUnitTest, CheckSyncInfo_001, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointnIner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    audioEndpointnIner->dstSpanSizeInframe_ = 0;
    audioEndpointnIner->CheckSyncInfo(100);
    EXPECT_EQ(audioEndpointnIner->dstSpanSizeInframe_, 0);
}

/*
 * @tc.name  : Test ProcessToDupStream API
 * @tc.type  : FUNC
 * @tc.number: ProcessToDupStream_001
 * @tc.desc  : Test AudioEndpointInner::ProcessToDupStream()
 */
HWTEST_F(AudioEndpointPlusUnitTest, ProcessToDupStream_001, TestSize.Level1)
{
    AudioEndpoint::EndpointType type = AudioEndpoint::TYPE_MMAP;
    uint64_t id = 123;
    AudioProcessConfig clientConfig = {};
    auto audioEndpointnIner = std::make_shared<AudioEndpointInner>(type, id, clientConfig);
    std::vector<AudioStreamData> audioDataList;
    AudioStreamData dstStreamData;
    int32_t innerCapId = 1;

    audioEndpointnIner->ProcessToDupStream(audioDataList, dstStreamData, innerCapId);
    EXPECT_EQ(innerCapId, 1);
}
} // namespace AudioStandard
} // namespace OHOS
