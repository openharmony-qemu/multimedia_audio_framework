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
#include <string>
#include <thread>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <unistd.h>
#include "test_case_common.h"
#include "audio_errors.h"
#include "hpae_audio_service_dump_callback_unit_test.h"
#include "hpae_manager_unit_test.h"
#include "hpae_manager.h"

using namespace OHOS;
using namespace AudioStandard;
using namespace HPAE;
using namespace testing::ext;
using namespace testing;

namespace {
static std::string g_rootPath = "/data/";
const std::string ROOT_PATH = "/data/source_file_io_48000_2_s16le.pcm";
constexpr int32_t TEST_SLEEP_TIME_20 = 20;
constexpr int32_t TEST_SLEEP_TIME_40 = 40;
constexpr uint32_t DEFAULT_FRAME_LEN_MS = 20;
static constexpr uint32_t BASE_TEN = 10;
constexpr uint32_t MS_PER_SECOND = 1000;
static std::map<std::string, uint32_t> g_formatFromParserStrToEnum = {
    {"s16", SAMPLE_S16LE},
    {"s16le", SAMPLE_S16LE},
    {"s24", SAMPLE_S24LE},
    {"s24le", SAMPLE_S24LE},
    {"s32", SAMPLE_S32LE},
    {"s32le", SAMPLE_S32LE},
    {"f32", SAMPLE_F32LE},
    {"f32le", SAMPLE_F32LE},
};

class HpaeManagerAudioParametersUnitTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    void TestAudioParameters(const std::string& rate, const std::string& format, const std::string& channels);
    std::shared_ptr<HpaeManager> hpaeManager_ = nullptr;
};

static long StringToNum(const std::string &str)
{
    char *endptr;
    long num = strtol(str.c_str(), &endptr, BASE_TEN);
    CHECK_AND_RETURN_RET_LOG(endptr != nullptr && *endptr == '\0', 0,
        "trans str \"%{public}s\" to num failed", str.c_str());
    return num;
}

AudioSampleFormat TransFormatFromStringToEnum(std::string format)
{
    return static_cast<AudioSampleFormat>(g_formatFromParserStrToEnum[format]);
}

void HpaeManagerAudioParametersUnitTest::SetUp()
{
    hpaeManager_ = std::make_shared<HPAE::HpaeManager>();
}

void HpaeManagerAudioParametersUnitTest::TearDown()
{
    hpaeManager_->DeInit();
    hpaeManager_ = nullptr;
}

void WaitForMsgProcessing(std::shared_ptr<HpaeManager> &hpaeManager)
{
    int waitCount = 0;
    const int waitCountThd = 5;
    while (hpaeManager->IsMsgProcessing()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_20));
        waitCount++;
        if (waitCount >= waitCountThd) {
            break;
        }
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(TEST_SLEEP_TIME_40));
    EXPECT_EQ(hpaeManager->IsMsgProcessing(), false);
    EXPECT_EQ(waitCount < waitCountThd, true);
}

AudioModuleInfo GetSinkAudioModeInfo(std::string name = "Speaker_File")
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
    audioModuleInfo.needEmptyChunk = true;
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

void HpaeManagerAudioParametersUnitTest::TestAudioParameters(const std::string& rate,
    const std::string& format, const std::string& channels)
{
    EXPECT_NE(hpaeManager_, nullptr);
    hpaeManager_->Init();
    EXPECT_EQ(hpaeManager_->IsInit(), true);
    sleep(1);
    EXPECT_EQ(hpaeManager_->IsRunning(), true);

    std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback = std::make_shared<HpaeAudioServiceCallbackUnitTest>();
    hpaeManager_->RegisterSerivceCallback(callback);

    AudioModuleInfo audioModuleInfo = GetSinkAudioModeInfo();
    audioModuleInfo.rate = rate;
    audioModuleInfo.format = format;
    audioModuleInfo.channels = channels;
    AudioSamplingRate samplingRateTest = static_cast<AudioSamplingRate>(StringToNum(rate));
    AudioSampleFormat formatTest = static_cast<AudioSampleFormat>(TransFormatFromStringToEnum(audioModuleInfo.format));
    AudioChannel channelsTest = static_cast<AudioChannel>(StringToNum(audioModuleInfo.channels));
    size_t bufferSize = samplingRateTest * channelsTest * DEFAULT_FRAME_LEN_MS *
        static_cast<size_t>(GetSizeFromFormat(formatTest)) / MS_PER_SECOND;
    audioModuleInfo.bufferSize = std::to_string(bufferSize);
    audioModuleInfo.fileName = g_rootPath + audioModuleInfo.adapterName + "_" +
                             rate + "_" + channels + "_" + format + ".pcm";

    EXPECT_EQ(hpaeManager_->OpenAudioPort(audioModuleInfo), SUCCESS);
    WaitForMsgProcessing(hpaeManager_);
    int32_t portId = callback->GetPortId();

    hpaeManager_->CloseAudioPort(portId);
    WaitForMsgProcessing(hpaeManager_);
    EXPECT_EQ(callback->GetCloseAudioPortResult(), SUCCESS);

    hpaeManager_->DeInit();
    EXPECT_EQ(hpaeManager_->IsInit(), false);
    EXPECT_EQ(hpaeManager_->IsRunning(), false);
}

#define GENERATE_TEST_CASE(rate, format, channels) \
HWTEST_F(HpaeManagerAudioParametersUnitTest, HpaeRenderManagerTest_##rate##_##format##_##channels, TestSize.Level2) { \
    TestAudioParameters(#rate, #format, #channels); \
}

GENERATE_TEST_CASE(8000, s16le, 1)
GENERATE_TEST_CASE(8000, s16le, 2)
GENERATE_TEST_CASE(8000, s16le, 3)
GENERATE_TEST_CASE(8000, s16le, 4)
GENERATE_TEST_CASE(8000, s16le, 5)
GENERATE_TEST_CASE(8000, s16le, 6)
GENERATE_TEST_CASE(8000, s16le, 7)
GENERATE_TEST_CASE(8000, s16le, 8)
GENERATE_TEST_CASE(8000, s24le, 1)
GENERATE_TEST_CASE(8000, s24le, 2)
GENERATE_TEST_CASE(8000, s24le, 3)
GENERATE_TEST_CASE(8000, s24le, 4)
GENERATE_TEST_CASE(8000, s24le, 5)
GENERATE_TEST_CASE(8000, s24le, 6)
GENERATE_TEST_CASE(8000, s24le, 7)
GENERATE_TEST_CASE(8000, s24le, 8)
GENERATE_TEST_CASE(8000, s32le, 1)
GENERATE_TEST_CASE(8000, s32le, 2)
GENERATE_TEST_CASE(8000, s32le, 3)
GENERATE_TEST_CASE(8000, s32le, 4)
GENERATE_TEST_CASE(8000, s32le, 5)
GENERATE_TEST_CASE(8000, s32le, 6)
GENERATE_TEST_CASE(8000, s32le, 7)
GENERATE_TEST_CASE(8000, s32le, 8)
GENERATE_TEST_CASE(8000, f32le, 1)
GENERATE_TEST_CASE(8000, f32le, 2)
GENERATE_TEST_CASE(8000, f32le, 3)
GENERATE_TEST_CASE(8000, f32le, 4)
GENERATE_TEST_CASE(8000, f32le, 5)
GENERATE_TEST_CASE(8000, f32le, 6)
GENERATE_TEST_CASE(8000, f32le, 7)
GENERATE_TEST_CASE(8000, f32le, 8)
GENERATE_TEST_CASE(11025, s16le, 1)
GENERATE_TEST_CASE(11025, s16le, 2)
GENERATE_TEST_CASE(11025, s16le, 3)
GENERATE_TEST_CASE(11025, s16le, 4)
GENERATE_TEST_CASE(11025, s16le, 5)
GENERATE_TEST_CASE(11025, s16le, 6)
GENERATE_TEST_CASE(11025, s16le, 7)
GENERATE_TEST_CASE(11025, s16le, 8)
GENERATE_TEST_CASE(11025, s24le, 1)
GENERATE_TEST_CASE(11025, s24le, 2)
GENERATE_TEST_CASE(11025, s24le, 3)
GENERATE_TEST_CASE(11025, s24le, 4)
GENERATE_TEST_CASE(11025, s24le, 5)
GENERATE_TEST_CASE(11025, s24le, 6)
GENERATE_TEST_CASE(11025, s24le, 7)
GENERATE_TEST_CASE(11025, s24le, 8)
GENERATE_TEST_CASE(11025, s32le, 1)
GENERATE_TEST_CASE(11025, s32le, 2)
GENERATE_TEST_CASE(11025, s32le, 3)
GENERATE_TEST_CASE(11025, s32le, 4)
GENERATE_TEST_CASE(11025, s32le, 5)
GENERATE_TEST_CASE(11025, s32le, 6)
GENERATE_TEST_CASE(11025, s32le, 7)
GENERATE_TEST_CASE(11025, s32le, 8)
GENERATE_TEST_CASE(11025, f32le, 1)
GENERATE_TEST_CASE(11025, f32le, 2)
GENERATE_TEST_CASE(11025, f32le, 3)
GENERATE_TEST_CASE(11025, f32le, 4)
GENERATE_TEST_CASE(11025, f32le, 5)
GENERATE_TEST_CASE(11025, f32le, 6)
GENERATE_TEST_CASE(11025, f32le, 7)
GENERATE_TEST_CASE(11025, f32le, 8)
GENERATE_TEST_CASE(12000, s16le, 1)
GENERATE_TEST_CASE(12000, s16le, 2)
GENERATE_TEST_CASE(12000, s16le, 3)
GENERATE_TEST_CASE(12000, s16le, 4)
GENERATE_TEST_CASE(12000, s16le, 5)
GENERATE_TEST_CASE(12000, s16le, 6)
GENERATE_TEST_CASE(12000, s16le, 7)
GENERATE_TEST_CASE(12000, s16le, 8)
GENERATE_TEST_CASE(12000, s24le, 1)
GENERATE_TEST_CASE(12000, s24le, 2)
GENERATE_TEST_CASE(12000, s24le, 3)
GENERATE_TEST_CASE(12000, s24le, 4)
GENERATE_TEST_CASE(12000, s24le, 5)
GENERATE_TEST_CASE(12000, s24le, 6)
GENERATE_TEST_CASE(12000, s24le, 7)
GENERATE_TEST_CASE(12000, s24le, 8)
GENERATE_TEST_CASE(12000, s32le, 1)
GENERATE_TEST_CASE(12000, s32le, 2)
GENERATE_TEST_CASE(12000, s32le, 3)
GENERATE_TEST_CASE(12000, s32le, 4)
GENERATE_TEST_CASE(12000, s32le, 5)
GENERATE_TEST_CASE(12000, s32le, 6)
GENERATE_TEST_CASE(12000, s32le, 7)
GENERATE_TEST_CASE(12000, s32le, 8)
GENERATE_TEST_CASE(12000, f32le, 1)
GENERATE_TEST_CASE(12000, f32le, 2)
GENERATE_TEST_CASE(12000, f32le, 3)
GENERATE_TEST_CASE(12000, f32le, 4)
GENERATE_TEST_CASE(12000, f32le, 5)
GENERATE_TEST_CASE(12000, f32le, 6)
GENERATE_TEST_CASE(12000, f32le, 7)
GENERATE_TEST_CASE(12000, f32le, 8)
GENERATE_TEST_CASE(16000, s16le, 1)
GENERATE_TEST_CASE(16000, s16le, 2)
GENERATE_TEST_CASE(16000, s16le, 3)
GENERATE_TEST_CASE(16000, s16le, 4)
GENERATE_TEST_CASE(16000, s16le, 5)
GENERATE_TEST_CASE(16000, s16le, 6)
GENERATE_TEST_CASE(16000, s16le, 7)
GENERATE_TEST_CASE(16000, s16le, 8)
GENERATE_TEST_CASE(16000, s24le, 1)
GENERATE_TEST_CASE(16000, s24le, 2)
GENERATE_TEST_CASE(16000, s24le, 3)
GENERATE_TEST_CASE(16000, s24le, 4)
GENERATE_TEST_CASE(16000, s24le, 5)
GENERATE_TEST_CASE(16000, s24le, 6)
GENERATE_TEST_CASE(16000, s24le, 7)
GENERATE_TEST_CASE(16000, s24le, 8)
GENERATE_TEST_CASE(16000, s32le, 1)
GENERATE_TEST_CASE(16000, s32le, 2)
GENERATE_TEST_CASE(16000, s32le, 3)
GENERATE_TEST_CASE(16000, s32le, 4)
GENERATE_TEST_CASE(16000, s32le, 5)
GENERATE_TEST_CASE(16000, s32le, 6)
GENERATE_TEST_CASE(16000, s32le, 7)
GENERATE_TEST_CASE(16000, s32le, 8)
GENERATE_TEST_CASE(16000, f32le, 1)
GENERATE_TEST_CASE(16000, f32le, 2)
GENERATE_TEST_CASE(16000, f32le, 3)
GENERATE_TEST_CASE(16000, f32le, 4)
GENERATE_TEST_CASE(16000, f32le, 5)
GENERATE_TEST_CASE(16000, f32le, 6)
GENERATE_TEST_CASE(16000, f32le, 7)
GENERATE_TEST_CASE(16000, f32le, 8)
GENERATE_TEST_CASE(22050, s16le, 1)
GENERATE_TEST_CASE(22050, s16le, 2)
GENERATE_TEST_CASE(22050, s16le, 3)
GENERATE_TEST_CASE(22050, s16le, 4)
GENERATE_TEST_CASE(22050, s16le, 5)
GENERATE_TEST_CASE(22050, s16le, 6)
GENERATE_TEST_CASE(22050, s16le, 7)
GENERATE_TEST_CASE(22050, s16le, 8)
GENERATE_TEST_CASE(22050, s24le, 1)
GENERATE_TEST_CASE(22050, s24le, 2)
GENERATE_TEST_CASE(22050, s24le, 3)
GENERATE_TEST_CASE(22050, s24le, 4)
GENERATE_TEST_CASE(22050, s24le, 5)
GENERATE_TEST_CASE(22050, s24le, 6)
GENERATE_TEST_CASE(22050, s24le, 7)
GENERATE_TEST_CASE(22050, s24le, 8)
GENERATE_TEST_CASE(22050, s32le, 1)
GENERATE_TEST_CASE(22050, s32le, 2)
GENERATE_TEST_CASE(22050, s32le, 3)
GENERATE_TEST_CASE(22050, s32le, 4)
GENERATE_TEST_CASE(22050, s32le, 5)
GENERATE_TEST_CASE(22050, s32le, 6)
GENERATE_TEST_CASE(22050, s32le, 7)
GENERATE_TEST_CASE(22050, s32le, 8)
GENERATE_TEST_CASE(22050, f32le, 1)
GENERATE_TEST_CASE(22050, f32le, 2)
GENERATE_TEST_CASE(22050, f32le, 3)
GENERATE_TEST_CASE(22050, f32le, 4)
GENERATE_TEST_CASE(22050, f32le, 5)
GENERATE_TEST_CASE(22050, f32le, 6)
GENERATE_TEST_CASE(22050, f32le, 7)
GENERATE_TEST_CASE(22050, f32le, 8)
GENERATE_TEST_CASE(44100, s16le, 1)
GENERATE_TEST_CASE(44100, s16le, 2)
GENERATE_TEST_CASE(44100, s16le, 3)
GENERATE_TEST_CASE(44100, s16le, 4)
GENERATE_TEST_CASE(44100, s16le, 5)
GENERATE_TEST_CASE(44100, s16le, 6)
GENERATE_TEST_CASE(44100, s16le, 7)
GENERATE_TEST_CASE(44100, s16le, 8)
GENERATE_TEST_CASE(44100, s24le, 1)
GENERATE_TEST_CASE(44100, s24le, 2)
GENERATE_TEST_CASE(44100, s24le, 3)
GENERATE_TEST_CASE(44100, s24le, 4)
GENERATE_TEST_CASE(44100, s24le, 5)
GENERATE_TEST_CASE(44100, s24le, 6)
GENERATE_TEST_CASE(44100, s24le, 7)
GENERATE_TEST_CASE(44100, s24le, 8)
GENERATE_TEST_CASE(44100, s32le, 1)
GENERATE_TEST_CASE(44100, s32le, 2)
GENERATE_TEST_CASE(44100, s32le, 3)
GENERATE_TEST_CASE(44100, s32le, 4)
GENERATE_TEST_CASE(44100, s32le, 5)
GENERATE_TEST_CASE(44100, s32le, 6)
GENERATE_TEST_CASE(44100, s32le, 7)
GENERATE_TEST_CASE(44100, s32le, 8)
GENERATE_TEST_CASE(44100, f32le, 1)
GENERATE_TEST_CASE(44100, f32le, 2)
GENERATE_TEST_CASE(44100, f32le, 3)
GENERATE_TEST_CASE(44100, f32le, 4)
GENERATE_TEST_CASE(44100, f32le, 5)
GENERATE_TEST_CASE(44100, f32le, 6)
GENERATE_TEST_CASE(44100, f32le, 7)
GENERATE_TEST_CASE(44100, f32le, 8)
GENERATE_TEST_CASE(48000, s16le, 1)
GENERATE_TEST_CASE(48000, s16le, 2)
GENERATE_TEST_CASE(48000, s16le, 3)
GENERATE_TEST_CASE(48000, s16le, 4)
GENERATE_TEST_CASE(48000, s16le, 5)
GENERATE_TEST_CASE(48000, s16le, 6)
GENERATE_TEST_CASE(48000, s16le, 7)
GENERATE_TEST_CASE(48000, s16le, 8)
GENERATE_TEST_CASE(48000, s24le, 1)
GENERATE_TEST_CASE(48000, s24le, 2)
GENERATE_TEST_CASE(48000, s24le, 3)
GENERATE_TEST_CASE(48000, s24le, 4)
GENERATE_TEST_CASE(48000, s24le, 5)
GENERATE_TEST_CASE(48000, s24le, 6)
GENERATE_TEST_CASE(48000, s24le, 7)
GENERATE_TEST_CASE(48000, s24le, 8)
GENERATE_TEST_CASE(48000, s32le, 1)
GENERATE_TEST_CASE(48000, s32le, 2)
GENERATE_TEST_CASE(48000, s32le, 3)
GENERATE_TEST_CASE(48000, s32le, 4)
GENERATE_TEST_CASE(48000, s32le, 5)
GENERATE_TEST_CASE(48000, s32le, 6)
GENERATE_TEST_CASE(48000, s32le, 7)
GENERATE_TEST_CASE(48000, s32le, 8)
GENERATE_TEST_CASE(48000, f32le, 1)
GENERATE_TEST_CASE(48000, f32le, 2)
GENERATE_TEST_CASE(48000, f32le, 3)
GENERATE_TEST_CASE(48000, f32le, 4)
GENERATE_TEST_CASE(48000, f32le, 5)
GENERATE_TEST_CASE(48000, f32le, 6)
GENERATE_TEST_CASE(48000, f32le, 7)
GENERATE_TEST_CASE(48000, f32le, 8)
}  // namespace