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

#include <gtest/gtest.h>
#include "hpae_capturer_stream_impl.h"
#include "audio_errors.h"
#include "hpae_adapter_manager.h"
#include "audio_system_manager.h"

using namespace testing::ext;
namespace OHOS {
namespace AudioStandard {
const int32_t CAPTURER_FLAG = 10;
static std::shared_ptr<HpaeAdapterManager> adapterManager;

class HpaeCapturerStreamUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    std::shared_ptr<HpaeCapturerStreamImpl> CreateHpaeCapturerStreamImpl();
};

void HpaeCapturerStreamUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void HpaeCapturerStreamUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void HpaeCapturerStreamUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void HpaeCapturerStreamUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

static AudioProcessConfig GetInnerCapConfig()
{
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
    config.capturerInfo.sourceType = SOURCE_TYPE_WAKEUP;
    config.innerCapId = 1;
    config.originalSessionId = 123456; // 123456: session id
    return config;
}

std::shared_ptr<HpaeCapturerStreamImpl> HpaeCapturerStreamUnitTest::CreateHpaeCapturerStreamImpl()
{
    adapterManager = std::make_shared<HpaeAdapterManager>(DUP_PLAYBACK);
    AudioProcessConfig processConfig = GetInnerCapConfig();
    std::string deviceName = "";
    std::shared_ptr<ICapturerStream> capturerStream = adapterManager->CreateCapturerStream(processConfig, deviceName);
    std::shared_ptr<HpaeCapturerStreamImpl> capturerStreamImpl =
        std::static_pointer_cast<HpaeCapturerStreamImpl>(capturerStream);
    return capturerStreamImpl;
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_001
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_001, TestSize.Level1)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    uint64_t framesReadRet = 0;
    capturerStreamImplRet->byteSizePerFrame_ = 0;
    auto ret = capturerStreamImplRet->GetStreamFramesRead(framesReadRet);
    EXPECT_EQ(ret, SUCCESS);

    capturerStreamImplRet->byteSizePerFrame_ = 1;
    ret = capturerStreamImplRet->GetStreamFramesRead(framesReadRet);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_002
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_002, TestSize.Level1)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    uint64_t timestampRet = 0;
    auto ret = capturerStreamImplRet->GetCurrentTimeStamp(timestampRet);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_003
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_003, TestSize.Level1)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    uint64_t latencyRet;
    bool isStandbyRet = false;

    auto ret = capturerStreamImplRet->GetLatency(latencyRet);
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Flush();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Stop();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Start();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Pause(isStandbyRet);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_004
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_004, TestSize.Level1)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    bool isStandbyRet = false;

    auto ret = capturerStreamImplRet->Start();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Pause(isStandbyRet);
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Flush();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_005
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_005, TestSize.Level1)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    capturerStreamImplRet->state_ = RUNNING;

    auto ret = capturerStreamImplRet->Release();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_006
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_006, TestSize.Level1)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    capturerStreamImplRet->statusCallback_ = std::weak_ptr<IStatusCallback>();
    capturerStreamImplRet->state_ = STOPPED;

    auto ret = capturerStreamImplRet->Release();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_007
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_007, TestSize.Level1)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    std::string deviceName = "";
    auto ret = capturerStreamImplRet->InitParams(deviceName);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_008
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_008, TestSize.Level1)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    int32_t ret = capturerStreamImplRet->Start();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_009
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_009, TestSize.Level1)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    bool isStandbyRet = false;
    auto ret = capturerStreamImplRet->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = capturerStreamImplRet->Pause(isStandbyRet);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_010
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_010, TestSize.Level1)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    bool isStandbyRet = false;
    auto ret = capturerStreamImplRet->Start();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Pause(isStandbyRet);
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Flush();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_011
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_011, TestSize.Level1)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    bool isStandbyRet = false;

    auto ret = capturerStreamImplRet->Start();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Pause(isStandbyRet);
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Flush();
    EXPECT_EQ(ret, SUCCESS);

    ret = capturerStreamImplRet->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_012
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_012, TestSize.Level1)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    capturerStreamImplRet->state_ = RUNNING;
    auto ret = capturerStreamImplRet->Release();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_013
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_013, TestSize.Level1)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    capturerStreamImplRet->capturerServerDumpFile_ = fopen("/data/data/.pulse_dir/capturer_impl.pcm", "wb+");
    size_t length = 1;
    capturerStreamImplRet->DequeueBuffer(length);
    EXPECT_EQ(capturerStreamImplRet != nullptr, true);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_014
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_014, TestSize.Level1)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    AudioCallBackCapturerStreamInfo info;
    info.timestamp = 123456789;
    info.latency = 100;
    info.framesRead = 44100;
    info.outputData = nullptr;
    info.requestDataLen = 44100 * sizeof(int16_t);

    auto ret = capturerStreamImplRet->OnStreamData(info);
    EXPECT_EQ(ret, SUCCESS);

    capturerStreamImplRet->readCallback_.reset();
    ret = capturerStreamImplRet->OnStreamData(info);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_016
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_016, TestSize.Level2)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();

    auto ret = capturerStreamImplRet->DropBuffer();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test HpaeCapturerStreamImpl API
 * @tc.type  : FUNC
 * @tc.number: HpaeCapturerStream_017
 * @tc.desc  : Test HpaeCapturerStreamImpl interface.
 */
HWTEST_F(HpaeCapturerStreamUnitTest, HpaeCapturerStream_017, TestSize.Level2)
{
    auto capturerStreamImplRet = CreateHpaeCapturerStreamImpl();
    size_t minBufferSize = 10;
    int32_t abortTimes = 0;

    capturerStreamImplRet->AbortCallback(abortTimes);
    auto ret = capturerStreamImplRet->GetMinimumBufferSize(minBufferSize);
    EXPECT_EQ(ret, SUCCESS);
}
}
}