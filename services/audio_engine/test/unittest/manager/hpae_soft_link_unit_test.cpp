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
#include "hpae_soft_link.h"
#include "hpae_manager_impl.h"
#include "hpae_audio_service_callback_unit_test.h"
using namespace testing::ext;
using namespace testing;
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static std::string g_rootPath = "/data/";
class HpaeSoftLinkTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void OpenAudioPort(bool openSink = true);
    void CloseAudioPort(bool closeSink = true);

    static std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback_;
    int32_t sinkId_ = -1;
    int32_t sourceId_ = -1;
};

class HpaeSoftLinkForTest : public HpaeSoftLink {
public:
    HpaeSoftLinkForTest(uint32_t sinkIdx, uint32_t sourceIdx, SoftLinkMode mode)
        : HpaeSoftLink(sinkIdx, sourceIdx, mode)
    {};
    virtual ~HpaeSoftLinkForTest() {};
    void OnStatusUpdate(IOperation operation, uint32_t streamIndex) override;
    void SetFalse(bool isRenderer, bool startFail);
private:
    bool rendererFail_ = false;
    bool capturerFail_ = false;
};

static AudioModuleInfo GetSinkAudioModeInfo(std::string name = "Speaker_File")
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-sink.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = name;
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "7680";
    audioModuleInfo.format = "s32le";
    audioModuleInfo.fixedLatency = "1";
    audioModuleInfo.offloadEnable = "0";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.fileName = g_rootPath + audioModuleInfo.adapterName + "_" + audioModuleInfo.rate + "_" +
                               audioModuleInfo.channels + "_" + audioModuleInfo.format + ".pcm";
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

static AudioModuleInfo GetSourceAudioModeInfo(std::string name = "mic")
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-source.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = name;
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "3840";
    audioModuleInfo.format = "s16le";
    audioModuleInfo.fixedLatency = "1";
    audioModuleInfo.offloadEnable = "0";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.fileName = g_rootPath + "source_" + audioModuleInfo.adapterName + "_" + audioModuleInfo.rate + "_" +
                               audioModuleInfo.channels + "_" + audioModuleInfo.format + ".pcm";
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_FILE_SOURCE);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

std::shared_ptr<HpaeAudioServiceCallbackUnitTest> HpaeSoftLinkTest::callback_ =
    std::make_shared<HpaeAudioServiceCallbackUnitTest>();
void HpaeSoftLinkTest::SetUpTestCase()
{
    IHpaeManager::GetHpaeManager().Init();
    IHpaeManager::GetHpaeManager().RegisterSerivceCallback(callback_);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 200ms for sleep
}

void HpaeSoftLinkTest::TearDownTestCase()
{
    IHpaeManager::GetHpaeManager().DeInit();
}

void HpaeSoftLinkTest::SetUp()
{
    OpenAudioPort(true);
    OpenAudioPort(false);
}

void HpaeSoftLinkTest::TearDown()
{
    CloseAudioPort(true);
    CloseAudioPort(false);
}

void HpaeSoftLinkTest::OpenAudioPort(bool openSink)
{
    AudioModuleInfo moduleInfo = openSink ? GetSinkAudioModeInfo() : GetSourceAudioModeInfo();
    EXPECT_EQ(IHpaeManager::GetHpaeManager().OpenAudioPort(moduleInfo), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 200ms for sleep
    if (openSink) {
        sinkId_ = callback_->GetPortId();
    } else {
        sourceId_ = callback_->GetPortId();
    }
}

void HpaeSoftLinkTest::CloseAudioPort(bool closeSink)
{
    IHpaeManager::GetHpaeManager().CloseAudioPort(closeSink ? sinkId_ : sourceId_);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 200ms for sleep
    if (closeSink) {
        sinkId_ = -1;
    } else {
        sourceId_ = -1;
    }
}

void HpaeSoftLinkForTest::OnStatusUpdate(IOperation operation, uint32_t streamIndex)
{
    CHECK_AND_RETURN_LOG(operation != OPERATION_RELEASED, "stream already released");
    CHECK_AND_RETURN_LOG(streamIndex == rendererStreamInfo_.sessionId || streamIndex == capturerStreamInfo_.sessionId,
        "invalid streamIndex");
    if (operation == OPERATION_STARTED) {
        if ((streamIndex == rendererStreamInfo_.sessionId && !rendererFail_) ||
            (streamIndex == capturerStreamInfo_.sessionId && !capturerFail_)) {
            streamStateMap_[streamIndex] = HpaeSoftLinkState::RUNNING;
        }
    } else if (operation == OPERATION_STOPPED) {
        streamStateMap_[streamIndex] = HpaeSoftLinkState::STOPPED;
        if (streamIndex == capturerStreamInfo_.sessionId) {
            FlushRingCache();
        }
    } else if (operation == OPERATION_RELEASED) {
        streamStateMap_[streamIndex] = HpaeSoftLinkState::RELEASED;
    } else {
        return;
    }

    std::lock_guard<std::mutex> lock(callbackMutex_);
    isStreamOperationFinish_ |=
        (streamIndex == rendererStreamInfo_.sessionId ? SOFTLINK_RENDERER_OPERATION : SOFTLINK_CAPTURER_OPERATION);
    callbackCV_.notify_all();
}

void HpaeSoftLinkForTest::SetFalse(bool isRenderer, bool startFail)
{
    if (isRenderer) {
        rendererFail_ = startFail;
    } else {
        capturerFail_ = startFail;
    }
}

TEST_F(HpaeSoftLinkTest, testSoftLink)
{
    std::shared_ptr<HpaeSoftLink> softLink_ =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink_, nullptr);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::NEW);

    EXPECT_EQ(softLink_->Init(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::PREPARED);

    EXPECT_EQ(softLink_->Init(), SUCCESS); // init after init
    EXPECT_EQ(softLink_->Stop(), ERR_ILLEGAL_STATE); // stop after init

    EXPECT_EQ(softLink_->Start(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::RUNNING);

    EXPECT_EQ(softLink_->Init(), ERR_ILLEGAL_STATE); // init after start
    EXPECT_EQ(softLink_->Start(), SUCCESS); // start after start

    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 2000ms for sleep

    EXPECT_EQ(softLink_->Stop(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::STOPPED);

    EXPECT_EQ(softLink_->Stop(), SUCCESS); // stop after stop

    EXPECT_EQ(softLink_->Release(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::RELEASED);
}

TEST_F(HpaeSoftLinkTest, testCapturerOverFlow)
{
    std::shared_ptr<HpaeSoftLink> softLink_ =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink_, nullptr);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::NEW);
    EXPECT_EQ(softLink_->Init(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::PREPARED);
    EXPECT_EQ(softLink_->Start(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::RUNNING);
    CloseAudioPort();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 2000ms for sleep
    EXPECT_EQ(softLink_->Release(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::RELEASED);
}

TEST_F(HpaeSoftLinkTest, testRendererUnderRun)
{
    std::shared_ptr<HpaeSoftLink> softLink_ =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink_, nullptr);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::NEW);
    EXPECT_EQ(softLink_->Init(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::PREPARED);
    EXPECT_EQ(softLink_->Start(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::RUNNING);
    CloseAudioPort(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 2000ms for sleep
    EXPECT_EQ(softLink_->Release(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::RELEASED);
}

TEST_F(HpaeSoftLinkTest, testStaticFunc)
{
    std::shared_ptr<IHpaeSoftLink> softLink1 =
        IHpaeSoftLink::CreateSoftLink(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink1, nullptr);
    softLink1->SetVolume(0.f);

    CloseAudioPort(false);
    std::shared_ptr<IHpaeSoftLink> softLink2 =
        IHpaeSoftLink::CreateSoftLink(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_EQ(softLink2, nullptr);

    std::shared_ptr<IHpaeSoftLink> softLink3 =
        IHpaeSoftLink::CreateSoftLink(sinkId_, -1, SoftLinkMode::HEARING_AID);
    EXPECT_EQ(softLink3, nullptr);

    HpaeSoftLink::g_sessionId = 99999; // 99999 for max sessionId
    EXPECT_EQ(HpaeSoftLink::GenerateSessionId(), 99999); // 99999 for max sessionId
    EXPECT_EQ(HpaeSoftLink::g_sessionId, 90000); // 90000 for min sessionId
}

TEST_F(HpaeSoftLinkTest, testSoftLinkStart)
{
    std::shared_ptr<HpaeSoftLinkForTest> softLink_ =
        std::make_shared<HpaeSoftLinkForTest>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink_, nullptr);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::NEW);
    EXPECT_EQ(softLink_->Init(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::PREPARED);
    softLink_->SetFalse(true, true);
    EXPECT_NE(softLink_->Start(), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 200ms for sleep
    EXPECT_NE(softLink_->state_, HpaeSoftLinkState::RUNNING);

    softLink_->SetFalse(true, false);
    softLink_->SetFalse(false, true);
    EXPECT_NE(softLink_->Start(), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 200ms for sleep
    EXPECT_NE(softLink_->state_, HpaeSoftLinkState::RUNNING);

    EXPECT_EQ(softLink_->Release(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::RELEASED);
}

TEST_F(HpaeSoftLinkTest, testTransSinkInfoToStreamInfo)
{
    std::shared_ptr<HpaeSoftLinkForTest> softLink_ =
        std::make_shared<HpaeSoftLinkForTest>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink_, nullptr);

    HpaeStreamInfo streamInfo;
    softLink_->TransSinkInfoToStreamInfo(streamInfo, HPAE_STREAM_CLASS_TYPE_RECORD);
    softLink_->linkMode_ = SoftLinkMode::OFFLOADINNERCAP_AID;
    softLink_->TransSinkInfoToStreamInfo(streamInfo, HPAE_STREAM_CLASS_TYPE_RECORD);

    EXPECT_EQ(streamInfo.sourceType, SOURCE_TYPE_OFFLOAD_CAPTURE);
}

/*
 * @tc.name  : Test SetLoudnessGain
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkSetLoudnessGainTest
 * @tc.desc  : Test SetLoudnessGainTest API
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkSetLoudnessGainTest, TestSize.Level1)
{
    std::shared_ptr<HpaeSoftLinkForTest> softLink =
        std::make_shared<HpaeSoftLinkForTest>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink, nullptr);
    EXPECT_EQ(softLink->state_, HpaeSoftLinkState::NEW);
    EXPECT_EQ(softLink->Init(), SUCCESS);
    EXPECT_EQ(softLink->state_, HpaeSoftLinkState::PREPARED);
    EXPECT_EQ(softLink->SetLoudnessGain(1.f), SUCCESS);
}

/*
 * @tc.name  : Test SetVolumeDuckFactor
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkSetVolumeDuckFactor
 * @tc.desc  : Test SetVolumeDuckFactor API
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkSetVolumeDuckFactor, TestSize.Level1)
{
    std::shared_ptr<HpaeSoftLinkForTest> softLink =
        std::make_shared<HpaeSoftLinkForTest>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink, nullptr);
    EXPECT_EQ(softLink->state_, HpaeSoftLinkState::NEW);
    EXPECT_EQ(softLink->Init(), SUCCESS);
    EXPECT_EQ(softLink->state_, HpaeSoftLinkState::PREPARED);
    EXPECT_EQ(softLink->SetVolumeDuckFactor(1.f), SUCCESS);
}

/*
 * @tc.name  : Test SetVolumeMute
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkSetVolumeMute
 * @tc.desc  : Test SetVolumeMute API
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkSetVolumeMute, TestSize.Level1)
{
    std::shared_ptr<HpaeSoftLinkForTest> softLink =
        std::make_shared<HpaeSoftLinkForTest>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink, nullptr);
    EXPECT_EQ(softLink->state_, HpaeSoftLinkState::NEW);
    EXPECT_EQ(softLink->Init(), SUCCESS);
    EXPECT_EQ(softLink->state_, HpaeSoftLinkState::PREPARED);
    EXPECT_EQ(softLink->SetVolumeMute(true), SUCCESS);
}

/*
 * @tc.name  : Test SetVolumeLowPowerFactor
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkSetVolumeLowPowerFactor
 * @tc.desc  : Test SetVolumeLowPowerFactor API
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkSetVolumeLowPowerFactor, TestSize.Level1)
{
    std::shared_ptr<HpaeSoftLinkForTest> softLink =
        std::make_shared<HpaeSoftLinkForTest>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink, nullptr);
    EXPECT_EQ(softLink->state_, HpaeSoftLinkState::NEW);
    EXPECT_EQ(softLink->Init(), SUCCESS);
    EXPECT_EQ(softLink->state_, HpaeSoftLinkState::PREPARED);
    EXPECT_EQ(softLink->SetVolumeLowPowerFactor(1.f), SUCCESS);
}

/*
 * @tc.name  : Test CopyRightToLeft with valid S16LE stereo data
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkTestCopyR2LTest_001
 * @tc.desc  : Test CopyRightToLeft function with 16-bit stereo audio data
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkTestCopyR2LTest_001, TestSize.Level1)
{
    // Prepare stereo S16LE data: L1, R1, L2, R2, L3, R3
    int16_t stereoData[] = {100, 200, 300, 400, 500, 600};
    size_t dataSize = sizeof(stereoData);

    std::shared_ptr<HpaeSoftLink> softLink =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink, nullptr);
    EXPECT_EQ(softLink->Init(), SUCCESS);
    softLink->bufferQueue_->ResetBuffer();
    softLink->sinkInfo_.format = SAMPLE_S16LE;

    AudioCallBackCapturerStreamInfo callbackStreamInfo;
    callbackStreamInfo.outputData = (int8_t *)stereoData;
    callbackStreamInfo.requestDataLen = dataSize;

    softLink->OnStreamData(callbackStreamInfo);
    int16_t* result = reinterpret_cast<int16_t*>(stereoData);
    EXPECT_EQ(result[0], 200); // L1 should become R1 (200)
    EXPECT_EQ(result[1], 200); // R1 remains R1 (200)
    EXPECT_EQ(result[2], 400); // L2 should become R2 (400)
    EXPECT_EQ(result[3], 400); // R2 remains R2 (400)
    EXPECT_EQ(result[4], 600); // L3 should become R3 (600)
    EXPECT_EQ(result[5], 600); // R3 remains R3 (600)
}

/*
 * @tc.name  : Test CopyRightToLeft with valid S32LE stereo data
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkTestCopyR2LTest_002
 * @tc.desc  : Test CopyRightToLeft function with 32-bit stereo audio data
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkTestCopyR2LTest_002, TestSize.Level1)
{
    // Prepare stereo S32LE data: L1, R1, L2, R2
    int32_t stereoData[] = {1000, 2000, 3000, 4000};
    size_t dataSize = sizeof(stereoData);

    std::shared_ptr<HpaeSoftLink> softLink =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink, nullptr);
    EXPECT_EQ(softLink->Init(), SUCCESS);
    softLink->bufferQueue_->ResetBuffer();
    softLink->sinkInfo_.format = SAMPLE_S32LE;

    AudioCallBackCapturerStreamInfo callbackStreamInfo;
    callbackStreamInfo.outputData = (int8_t *)stereoData;
    callbackStreamInfo.requestDataLen = dataSize;

    softLink->OnStreamData(callbackStreamInfo);
    int32_t* result = reinterpret_cast<int32_t*>(stereoData);
    EXPECT_EQ(result[0], 2000); // L1 should become R1 (2000)
    EXPECT_EQ(result[1], 2000); // R1 remains R1 (2000)
    EXPECT_EQ(result[2], 4000); // L2 should become R2 (4000)
    EXPECT_EQ(result[3], 4000); // R2 remains R2 (4000)
}

/*
 * @tc.name  : Test CopyRightToLeft with null data pointer
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkTestCopyR2LTest_003
 * @tc.desc  : Test CopyRightToLeft function with null data pointer
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkTestCopyR2LTest_003, TestSize.Level2)
{
    std::shared_ptr<HpaeSoftLink> softLink =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink, nullptr);
    EXPECT_EQ(softLink->Init(), SUCCESS);
    softLink->bufferQueue_->ResetBuffer();

    AudioCallBackCapturerStreamInfo callbackStreamInfo;
    callbackStreamInfo.outputData = nullptr;
    callbackStreamInfo.requestDataLen = 1;
    softLink->OnStreamData(callbackStreamInfo);
}

/*
 * @tc.name  : Test CopyRightToLeft with zero size
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkTestCopyR2LTest_004
 * @tc.desc  : Test CopyRightToLeft function with zero data size
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkTestCopyR2LTest_004, TestSize.Level2)
{
    int8_t data[100] = {0};
    std::shared_ptr<HpaeSoftLink> softLink =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink, nullptr);
    EXPECT_EQ(softLink->Init(), SUCCESS);
    softLink->bufferQueue_->ResetBuffer();

    AudioCallBackCapturerStreamInfo callbackStreamInfo;
    callbackStreamInfo.outputData = data;
    callbackStreamInfo.requestDataLen = 0;
    softLink->OnStreamData(callbackStreamInfo);
}

/*
 * @tc.name  : Test CopyRightToLeft with F32LE format
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkTestCopyR2LTest_005
 * @tc.desc  : Test CopyRightToLeft function with 32-bit float stereo data
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkTestCopyR2LTest_005, TestSize.Level1)
{
    // Prepare stereo F32LE data: L1, R1, L2, R2
    float stereoData[] = {1.0f, 2.0f, 3.0f, 4.0f};
    size_t dataSize = sizeof(stereoData);

    std::shared_ptr<HpaeSoftLink> softLink =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink, nullptr);
    EXPECT_EQ(softLink->Init(), SUCCESS);
    softLink->bufferQueue_->ResetBuffer();
    softLink->sinkInfo_.format = SAMPLE_F32LE;

    AudioCallBackCapturerStreamInfo callbackStreamInfo;
    callbackStreamInfo.outputData = (int8_t *)stereoData;
    callbackStreamInfo.requestDataLen = dataSize;

    softLink->OnStreamData(callbackStreamInfo);
    float* result = reinterpret_cast<float*>(stereoData);
    EXPECT_FLOAT_EQ(result[0], 2.0f); // L1 should become R1 (2.0f)
    EXPECT_FLOAT_EQ(result[1], 2.0f); // R1 remains R1 (2.0f)
    EXPECT_FLOAT_EQ(result[2], 4.0f); // L2 should become R2 (4.0f)
    EXPECT_FLOAT_EQ(result[3], 4.0f); // R2 remains R2 (4.0f)
}

/*
 * @tc.name  : Test CopyRightToLeft with S24LE format
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkTestCopyR2LTest_006
 * @tc.desc  : Test CopyRightToLeft function with 24-bit stereo data
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkTestCopyR2LTest_006, TestSize.Level1)
{
    // Prepare stereo S24LE data (3 bytes per sample)
    // L1: 0x010203, R1: 0x040506, L2: 0x070809, R2: 0x0A0B0C
    uint8_t stereoData[] = {
        0x01, 0x02, 0x03, // L1
        0x04, 0x05, 0x06, // R1
        0x07, 0x08, 0x09, // L2
        0x0A, 0x0B, 0x0C  // R2
    };
    size_t dataSize = sizeof(stereoData);
    
    std::shared_ptr<HpaeSoftLink> softLink =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink, nullptr);
    EXPECT_EQ(softLink->Init(), SUCCESS);
    softLink->bufferQueue_->ResetBuffer();
    softLink->sinkInfo_.format = SAMPLE_S24LE;

    AudioCallBackCapturerStreamInfo callbackStreamInfo;
    callbackStreamInfo.outputData = (int8_t *)stereoData;
    callbackStreamInfo.requestDataLen = dataSize;

    softLink->OnStreamData(callbackStreamInfo);
    EXPECT_EQ(stereoData[0], 0x04); // L1 byte0 should become R1 byte0
    EXPECT_EQ(stereoData[1], 0x05); // L1 byte1 should become R1 byte1
    EXPECT_EQ(stereoData[2], 0x06); // L1 byte2 should become R1 byte2
    EXPECT_EQ(stereoData[3], 0x04); // R1 byte0 remains
    EXPECT_EQ(stereoData[4], 0x05); // R1 byte1 remains
    EXPECT_EQ(stereoData[5], 0x06); // R1 byte2 remains
    EXPECT_EQ(stereoData[6], 0x0A); // L2 byte0 should become R2 byte0
    EXPECT_EQ(stereoData[7], 0x0B); // L2 byte1 should become R2 byte1
    EXPECT_EQ(stereoData[8], 0x0C); // L2 byte2 should become R2 byte2
    EXPECT_EQ(stereoData[9], 0x0A); // R2 byte0 remains
    EXPECT_EQ(stereoData[10], 0x0B); // R2 byte1 remains
    EXPECT_EQ(stereoData[11], 0x0C); // R2 byte2 remains
}

/*
 * @tc.name  : Test FlushRingCache API
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkFlushRingCacheTest_001
 * @tc.desc  : Test FlushRingCache while ringCache is null
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkFlushRingCacheTest_001, TestSize.Level1)
{
    std::shared_ptr<HpaeSoftLink> softLink =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink, nullptr);
    softLink->FlushRingCache();
    SUCCEED();
}

/*
 * @tc.name  : Test FlushRingCache API
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkFlushRingCacheTest_002
 * @tc.desc  : Test FlushRingCache while ringCache is not null
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkFlushRingCacheTest_002, TestSize.Level1)
{
    std::shared_ptr<HpaeSoftLink> softLink =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink, nullptr);
    EXPECT_EQ(softLink->Init(), SUCCESS);

    size_t size = 10; // 10 for buffer size
    std::vector<char> vec(size);
    softLink->bufferQueue_->Enqueue({reinterpret_cast<uint8_t *>(vec.data()), vec.size()});
    OptResult result = softLink->bufferQueue_->GetReadableSize();
    EXPECT_EQ(result.ret == OPERATION_SUCCESS, true);
    EXPECT_EQ(result.size, size);
    softLink->FlushRingCache();
    result = softLink->bufferQueue_->GetReadableSize();
    EXPECT_EQ(result.ret == OPERATION_SUCCESS, true);
    EXPECT_EQ(result.size, 0);
}

/*
 * @tc.name  : Test HpaeSoftLink TransSinkInfoToStreamInfo with play stream class
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkTransStreamInfoTest_001
 * @tc.desc  : Test TransSinkInfoToStreamInfo for HPAE_STREAM_CLASS_TYPE_PLAY
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkTransStreamInfoTest_001, TestSize.Level1)
{
    std::shared_ptr<HpaeSoftLink> softLink =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink, nullptr);
    EXPECT_EQ(softLink->Init(), SUCCESS);
    HpaeSinkInfo &sinkInfo = softLink->sinkInfo_;

    HpaeStreamInfo streamInfo;
    softLink->TransSinkInfoToStreamInfo(streamInfo, HPAE_STREAM_CLASS_TYPE_PLAY);
    EXPECT_EQ(streamInfo.channels, sinkInfo.channels);
    EXPECT_EQ(streamInfo.format, sinkInfo.format);
    EXPECT_EQ(streamInfo.deviceName, sinkInfo.deviceName);
    EXPECT_EQ(streamInfo.sourceType, SOURCE_TYPE_INVALID);
    EXPECT_EQ(streamInfo.streamType, STREAM_VOICE_CALL);
    EXPECT_EQ(streamInfo.fadeType, DEFAULT_FADE);
}

/*
 * @tc.name  : Test HpaeSoftLink TransSinkInfoToStreamInfo with play stream class
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkTransStreamInfoTest_002
 * @tc.desc  : Test TransSinkInfoToStreamInfo for HPAE_STREAM_CLASS_TYPE_RECORD
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkTransStreamInfoTest_002, TestSize.Level1)
{
    std::shared_ptr<HpaeSoftLink> softLink =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink, nullptr);
    EXPECT_EQ(softLink->Init(), SUCCESS);
    HpaeSinkInfo &sinkInfo = softLink->sinkInfo_;
    HpaeSourceInfo &sourceInfo = softLink->sourceInfo_;

    HpaeStreamInfo streamInfo;
    softLink->TransSinkInfoToStreamInfo(streamInfo, HPAE_STREAM_CLASS_TYPE_RECORD);
    EXPECT_EQ(streamInfo.channels, sinkInfo.channels);
    EXPECT_EQ(streamInfo.format, sinkInfo.format);
    EXPECT_EQ(streamInfo.deviceName, sourceInfo.deviceName);
    EXPECT_EQ(streamInfo.sourceType, SOURCE_TYPE_MIC);
    EXPECT_EQ(streamInfo.streamType, STREAM_SOURCE_VOICE_CALL);
    EXPECT_EQ(streamInfo.fadeType, NONE_FADE);
}

/*
 * @tc.name  : Test HpaeSoftLink TransSinkInfoToStreamInfo with play stream class
 * @tc.type  : FUNC
 * @tc.number: HpaeSoftLinkTransStreamInfoTest_003
 * @tc.desc  : Test TransSinkInfoToStreamInfo for offload inner cap
 */
HWTEST_F(HpaeSoftLinkTest, HpaeSoftLinkTransStreamInfoTest_003, TestSize.Level1)
{
    std::shared_ptr<HpaeSoftLink> softLink =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::OFFLOADINNERCAP_AID);
    EXPECT_NE(softLink, nullptr);
    EXPECT_EQ(softLink->Init(), SUCCESS);
    HpaeSinkInfo &sinkInfo = softLink->sinkInfo_;
    HpaeSourceInfo &sourceInfo = softLink->sourceInfo_;

    HpaeStreamInfo streamInfo;
    softLink->TransSinkInfoToStreamInfo(streamInfo, HPAE_STREAM_CLASS_TYPE_RECORD);
    EXPECT_EQ(streamInfo.channels, sinkInfo.channels);
    EXPECT_EQ(streamInfo.format, sinkInfo.format);
    EXPECT_EQ(streamInfo.deviceName, sourceInfo.deviceName);
    EXPECT_EQ(streamInfo.sourceType, SOURCE_TYPE_OFFLOAD_CAPTURE);
    EXPECT_EQ(streamInfo.streamType, STREAM_SOURCE_VOICE_CALL);
    EXPECT_EQ(streamInfo.fadeType, NONE_FADE);
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
