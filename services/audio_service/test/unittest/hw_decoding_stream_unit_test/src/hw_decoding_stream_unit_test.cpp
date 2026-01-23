/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "hw_decoding_stream_unit_test.h"
#include <gtest/gtest.h>
#include "audio_info.h"
#include "audio_errors.h"
#include "hw_decoding_renderer_impl.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
constexpr int32_t DEFAULT_STREAM_ID = 10;
void HWDecodingRendererStreamImplUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void HWDecodingRendererStreamImplUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void HWDecodingRendererStreamImplUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void HWDecodingRendererStreamImplUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

static AudioProcessConfig InitProcessConfig()
{
    AudioProcessConfig config;
    config.appInfo.appUid = DEFAULT_STREAM_ID;
    config.appInfo.appPid = DEFAULT_STREAM_ID;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    return config;
}

static IAudioSinkAttr InitAttr()
{
    IAudioSinkAttr attr = {};
    attr.adapterName = "dp";
    attr.encodingType = ENCODING_EAC3; // used for HW decoding
    attr.sampleRate = SAMPLE_RATE_48000;
    attr.channel = STEREO;
    attr.format = SAMPLE_S32LE;
    attr.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    attr.deviceType = DEVICE_TYPE_DP; // in plan
    attr.volume = 1.0f;
    return attr;
}
class MockSink : public IAudioRenderSink {
public:
    MockSink() {}
    ~MockSink() {}

    int32_t Init(const IAudioSinkAttr &attr) override { return mockInitRet_; }
    void DeInit(void) override {}
    bool IsInited(void) override { return true; }

    int32_t Start(void) override { return SUCCESS; }
    int32_t Stop(void) override { return SUCCESS; }
    int32_t Resume(void) override { return SUCCESS; }
    int32_t Pause(void) override { return SUCCESS; }
    int32_t Flush(void) override { return SUCCESS; }
    int32_t Reset(void) override { return SUCCESS; }
    int32_t RenderFrame(char &data, uint64_t len, uint64_t &writeLen) override { return SUCCESS; }
    int32_t GetVolumeDataCount(int64_t &volumeData) override { return SUCCESS; }

    int32_t SetVolume(float left, float right) override { return SUCCESS; }
    int32_t GetVolume(float &left, float &right) override { return SUCCESS; }

    int32_t GetLatency(uint32_t &latency) override { return SUCCESS; }
    int32_t GetTransactionId(uint64_t &transactionId) override { return SUCCESS; }
    int32_t GetPresentationPosition(uint64_t &frames, int64_t &timeSec, int64_t &timeNanoSec) override { return 0; }
    float GetMaxAmplitude(void) override { return 0.0; }
    void SetAudioMonoState(bool audioMono) override {}
    void SetAudioBalanceValue(float audioBalance) override {}

    int32_t UpdateAppsUid(const int32_t appsUid[MAX_MIX_CHANNELS], const size_t size) override { return SUCCESS; }
    int32_t UpdateAppsUid(const std::vector<int32_t> &appsUid) override { return SUCCESS; }

    void DumpInfo(std::string &dumpString) override {}

    int32_t mockInitRet_ = SUCCESS;
};

/**
 * @tc.name  : Test IsSinkInitted API
 * @tc.type  : FUNC
 * @tc.number: InitTest_001
 */
HWTEST(HWDecodingRendererStreamImplUnitTest, InitTest_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    std::shared_ptr<HWDecodingRendererStream> stream = std::make_shared<HWDecodingRendererStream>(processConfig);

    IAudioSinkAttr attr = InitAttr();
    int32_t ret = stream->IsSinkInitted(attr);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test IsSinkInitted API
 * @tc.type  : FUNC
 * @tc.number: InitTest_002
 */
HWTEST(HWDecodingRendererStreamImplUnitTest, InitTest_002, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    std::shared_ptr<HWDecodingRendererStream> stream = std::make_shared<HWDecodingRendererStream>(processConfig);

    IAudioSinkAttr attr = InitAttr();
    stream->sink_ = std::make_shared<MockSink>();
    int32_t ret = stream->IsSinkInitted(attr);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test IsSinkInitted API
 * @tc.type  : FUNC
 * @tc.number: InitTest_003
 */
HWTEST(HWDecodingRendererStreamImplUnitTest, InitTest_003, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    std::shared_ptr<HWDecodingRendererStream> stream = std::make_shared<HWDecodingRendererStream>(processConfig);

    IAudioSinkAttr attr = InitAttr();
    std::shared_ptr<MockSink> sink = std::make_shared<MockSink>();
    sink->mockInitRet_ = ERR_INVALID_HANDLE;
    stream->sink_ = static_pointer_cast<IAudioRenderSink>(sink);
    int32_t ret = stream->IsSinkInitted(attr);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test Release API
 * @tc.type  : FUNC
 * @tc.number: Release_001
 */
HWTEST(HWDecodingRendererStreamImplUnitTest, Release_001, TestSize.Level1)
{
    AudioProcessConfig processConfig = InitProcessConfig();
    std::shared_ptr<HWDecodingRendererStream> stream = std::make_shared<HWDecodingRendererStream>(processConfig);

    int32_t ret = stream->Release();
    EXPECT_EQ(ret, SUCCESS);

    stream->sink_ = std::make_shared<MockSink>();
    ret = stream->Release();
    EXPECT_EQ(stream->sink_, nullptr);
}
} // namespace AudioStandard
} // namespace OHOS