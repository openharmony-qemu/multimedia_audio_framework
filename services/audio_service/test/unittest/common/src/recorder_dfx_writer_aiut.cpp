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

#include "recorder_dfx_writer.h"
#include "audio_info.h"

#include "aiut_log.h"

#include <gtest/gtest.h>

using namespace testing::ext;

namespace {
constexpr int32_t TEST_APP_UID = 1001;
constexpr int32_t TEST_APP_PID = 2001;
constexpr int32_t TEST_APP_TOKEN_ID = 3001;
} // namespace

namespace OHOS {
namespace AudioStandard {

class RecorderDfxWriterAiut : public testing::Test {
public:
    void SetUp() override {}
    void TearDown() override {}

    static AppInfo GetTestAppInfo()
    {
        AppInfo appInfo{};
        appInfo.appUid = TEST_APP_UID;
        appInfo.appPid = TEST_APP_PID;
        appInfo.appTokenId = TEST_APP_TOKEN_ID;
        return appInfo;
    }

    static AudioProcessConfig GetTestProcessConfig()
    {
        AudioProcessConfig config{};
        config.appInfo = GetTestAppInfo();
        config.audioMode = AUDIO_MODE_RECORD;
        config.capturerInfo.sourceType = SOURCE_TYPE_MIC;
        config.capturerInfo.capturerFlags = 0;
        config.streamInfo.format = AudioSampleFormat::SAMPLE_S16LE;
        config.streamInfo.channels = AudioChannel::STEREO;
        config.streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_48000;
        return config;
    }
};

/**
 * @tc.name  : RecorderDfxWriter_Constructor_ValidAppInfo
 * @tc.number: RecorderDfxWriter_Constructor_ValidAppInfo
 * @tc.desc  : Test RecorderDfxWriter constructor with valid AppInfo and index.
 */
HWTEST_F(RecorderDfxWriterAiut, RecorderDfxWriter_Constructor_ValidAppInfo, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo = GetTestAppInfo();
    uint32_t index = 1;
    auto writer = std::make_unique<RecorderDfxWriter>(appInfo, index);
    AIUT_ASSERT_NE_INFO_LOG(writer, nullptr, "RecorderDfxWriter create failed, ptr=%p", writer.get());
    AIUT_EXPECT_NE_INFO_LOG(writer->dfxCollector_, nullptr, "dfxCollector_ should not be nullptr");
}

/**
 * @tc.name  : RecorderDfxWriter_WriteDfxStartMsg_StartOk
 * @tc.number: RecorderDfxWriter_WriteDfxStartMsg_StartOk
 * @tc.desc  : Test WriteDfxStartMsg with CAPTURER_STAGE_START_OK stage.
 */
HWTEST_F(RecorderDfxWriterAiut, RecorderDfxWriter_WriteDfxStartMsg_StartOk, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo = GetTestAppInfo();
    auto writer = std::make_unique<RecorderDfxWriter>(appInfo, 1);
    AIUT_ASSERT_NE_INFO_LOG(writer, nullptr, "writer is null");
    AudioProcessConfig config = GetTestProcessConfig();
    writer->WriteDfxStartMsg(1, CAPTURER_STAGE_START_OK, config);
    AIUT_EXPECT_NE_INFO_LOG(writer->dfxCollector_, nullptr,
        "dfxCollector_ should still be valid after WriteDfxStartMsg");
    AIUT_INFO_LOG("WriteDfxStartMsg with START_OK completed");
}

/**
 * @tc.name  : RecorderDfxWriter_WriteDfxStartMsg_StartFail
 * @tc.number: RecorderDfxWriter_WriteDfxStartMsg_StartFail
 * @tc.desc  : Test WriteDfxStartMsg with CAPTURER_STAGE_START_FAIL stage.
 */
HWTEST_F(RecorderDfxWriterAiut, RecorderDfxWriter_WriteDfxStartMsg_StartFail, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo = GetTestAppInfo();
    auto writer = std::make_unique<RecorderDfxWriter>(appInfo, 1);
    AIUT_ASSERT_NE_INFO_LOG(writer, nullptr, "writer is null");
    AudioProcessConfig config = GetTestProcessConfig();
    writer->WriteDfxStartMsg(1, CAPTURER_STAGE_START_FAIL, config);
    AIUT_EXPECT_NE_INFO_LOG(writer->dfxCollector_, nullptr,
        "dfxCollector_ should still be valid after WriteDfxStartMsg");
    AIUT_INFO_LOG("WriteDfxStartMsg with START_FAIL completed");
}

/**
 * @tc.name  : RecorderDfxWriter_WriteDfxStartMsg_InvalidStages
 * @tc.number: RecorderDfxWriter_WriteDfxStartMsg_InvalidStages
 * @tc.desc  : Test WriteDfxStartMsg with invalid stage (should return early).
 */
HWTEST_F(RecorderDfxWriterAiut, RecorderDfxWriter_WriteDfxStartMsg_InvalidStages, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo = GetTestAppInfo();
    auto writer = std::make_unique<RecorderDfxWriter>(appInfo, 1);
    AIUT_ASSERT_NE_INFO_LOG(writer, nullptr, "writer is null");
    AudioProcessConfig config = GetTestProcessConfig();
    writer->WriteDfxStartMsg(1, CAPTURER_STAGE_PAUSE_OK, config);
    writer->WriteDfxStartMsg(1, CAPTURER_STAGE_STOP_OK, config);
    writer->WriteDfxStartMsg(1, CAPTURER_STAGE_STOP_BY_RELEASE, config);
    AIUT_EXPECT_NE_INFO_LOG(writer->dfxCollector_, nullptr,
        "dfxCollector_ should still be valid after WriteDfxStartMsg");
    AIUT_INFO_LOG("WriteDfxStartMsg with invalid stages completed");
}

/**
 * @tc.name  : RecorderDfxWriter_WriteDfxStartMsg_NullDfxCollector
 * @tc.number: RecorderDfxWriter_WriteDfxStartMsg_NullDfxCollector
 * @tc.desc  : Test WriteDfxStartMsg when dfxCollector_ is nullptr (should return early).
 */
HWTEST_F(RecorderDfxWriterAiut, RecorderDfxWriter_WriteDfxStartMsg_NullDfxCollector, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo = GetTestAppInfo();
    auto writer = std::make_unique<RecorderDfxWriter>(appInfo, 1);
    AIUT_ASSERT_NE_INFO_LOG(writer, nullptr, "writer is null");
    writer->dfxCollector_ = nullptr;
    AudioProcessConfig config = GetTestProcessConfig();
    writer->WriteDfxStartMsg(1, CAPTURER_STAGE_START_OK, config);
    AIUT_EXPECT_EQ_INFO_LOG(writer->dfxCollector_, nullptr, "dfxCollector_ should remain null after WriteDfxStartMsg");
    AIUT_INFO_LOG("WriteDfxStartMsg with null dfxCollector_ completed");
}

/**
 * @tc.name  : RecorderDfxWriter_WriteDfxStopMsg_StopOk
 * @tc.number: RecorderDfxWriter_WriteDfxStopMsg_StopOk
 * @tc.desc  : Test WriteDfxStopMsg with CAPTURER_STAGE_STOP_OK stage.
 */
HWTEST_F(RecorderDfxWriterAiut, RecorderDfxWriter_WriteDfxStopMsg_StopOk, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo = GetTestAppInfo();
    auto writer = std::make_unique<RecorderDfxWriter>(appInfo, 1);
    AIUT_ASSERT_NE_INFO_LOG(writer, nullptr, "writer is null");
    AudioProcessConfig config = GetTestProcessConfig();
    int64_t duration = 1000;
    writer->WriteDfxStopMsg(1, CAPTURER_STAGE_STOP_OK, duration, config);
    AIUT_EXPECT_NE_INFO_LOG(writer->dfxCollector_, nullptr,
        "dfxCollector_ should still be valid after WriteDfxStopMsg");
    AIUT_INFO_LOG("WriteDfxStopMsg with STOP_OK completed");
}

/**
 * @tc.name  : RecorderDfxWriter_WriteDfxStopMsg_PauseOk
 * @tc.number: RecorderDfxWriter_WriteDfxStopMsg_PauseOk
 * @tc.desc  : Test WriteDfxStopMsg with CAPTURER_STAGE_PAUSE_OK stage.
 */
HWTEST_F(RecorderDfxWriterAiut, RecorderDfxWriter_WriteDfxStopMsg_PauseOk, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo = GetTestAppInfo();
    auto writer = std::make_unique<RecorderDfxWriter>(appInfo, 1);
    AIUT_ASSERT_NE_INFO_LOG(writer, nullptr, "writer is null");
    AudioProcessConfig config = GetTestProcessConfig();
    int64_t duration = 500;
    writer->WriteDfxStopMsg(1, CAPTURER_STAGE_PAUSE_OK, duration, config);
    AIUT_EXPECT_NE_INFO_LOG(writer->dfxCollector_, nullptr,
        "dfxCollector_ should still be valid after WriteDfxStopMsg");
    AIUT_INFO_LOG("WriteDfxStopMsg with PAUSE_OK completed");
}

/**
 * @tc.name  : RecorderDfxWriter_WriteDfxStopMsg_StopByRelease
 * @tc.number: RecorderDfxWriter_WriteDfxStopMsg_StopByRelease
 * @tc.desc  : Test WriteDfxStopMsg with CAPTURER_STAGE_STOP_BY_RELEASE stage.
 */
HWTEST_F(RecorderDfxWriterAiut, RecorderDfxWriter_WriteDfxStopMsg_StopByRelease, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo = GetTestAppInfo();
    auto writer = std::make_unique<RecorderDfxWriter>(appInfo, 1);
    AIUT_ASSERT_NE_INFO_LOG(writer, nullptr, "writer is null");
    AudioProcessConfig config = GetTestProcessConfig();
    int64_t duration = 2000;
    writer->WriteDfxStopMsg(1, CAPTURER_STAGE_STOP_BY_RELEASE, duration, config);
    AIUT_EXPECT_NE_INFO_LOG(writer->dfxCollector_, nullptr,
        "dfxCollector_ should still be valid after WriteDfxStopMsg");
    AIUT_INFO_LOG("WriteDfxStopMsg with STOP_BY_RELEASE completed");
}

/**
 * @tc.name  : RecorderDfxWriter_WriteDfxStopMsg_InvalidStages
 * @tc.number: RecorderDfxWriter_WriteDfxStopMsg_InvalidStages
 * @tc.desc  : Test WriteDfxStopMsg with invalid stage (should return early).
 */
HWTEST_F(RecorderDfxWriterAiut, RecorderDfxWriter_WriteDfxStopMsg_InvalidStages, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo = GetTestAppInfo();
    auto writer = std::make_unique<RecorderDfxWriter>(appInfo, 1);
    AIUT_ASSERT_NE_INFO_LOG(writer, nullptr, "writer is null");
    AudioProcessConfig config = GetTestProcessConfig();
    int64_t duration = 1000;
    writer->WriteDfxStopMsg(1, CAPTURER_STAGE_START_OK, duration, config);
    writer->WriteDfxStopMsg(1, CAPTURER_STAGE_START_FAIL, duration, config);
    AIUT_EXPECT_NE_INFO_LOG(writer->dfxCollector_, nullptr,
        "dfxCollector_ should still be valid after WriteDfxStopMsg");
    AIUT_INFO_LOG("WriteDfxStopMsg with invalid stages completed");
}

/**
 * @tc.name  : RecorderDfxWriter_WriteDfxStopMsg_NullDfxCollector
 * @tc.number: RecorderDfxWriter_WriteDfxStopMsg_NullDfxCollector
 * @tc.desc  : Test WriteDfxStopMsg when dfxCollector_ is nullptr (should return early).
 */
HWTEST_F(RecorderDfxWriterAiut, RecorderDfxWriter_WriteDfxStopMsg_NullDfxCollector, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo = GetTestAppInfo();
    auto writer = std::make_unique<RecorderDfxWriter>(appInfo, 1);
    AIUT_ASSERT_NE_INFO_LOG(writer, nullptr, "writer is null");
    writer->dfxCollector_ = nullptr;
    AudioProcessConfig config = GetTestProcessConfig();
    int64_t duration = 1000;
    writer->WriteDfxStopMsg(1, CAPTURER_STAGE_STOP_OK, duration, config);
    AIUT_EXPECT_EQ_INFO_LOG(writer->dfxCollector_, nullptr, "dfxCollector_ should remain null after WriteDfxStopMsg");
    AIUT_INFO_LOG("WriteDfxStopMsg with null dfxCollector_ completed");
}

/**
 * @tc.name  : RecorderDfxWriter_WriteDfxActionMsg_ValidDfxCollector
 * @tc.number: RecorderDfxWriter_WriteDfxActionMsg_ValidDfxCollector
 * @tc.desc  : Test WriteDfxActionMsg with valid dfxCollector_.
 */
HWTEST_F(RecorderDfxWriterAiut, RecorderDfxWriter_WriteDfxActionMsg_ValidDfxCollector, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo = GetTestAppInfo();
    auto writer = std::make_unique<RecorderDfxWriter>(appInfo, 1);
    AIUT_ASSERT_NE_INFO_LOG(writer, nullptr, "writer is null");
    writer->WriteDfxActionMsg(1, CAPTURER_STAGE_START_OK);
    writer->WriteDfxActionMsg(1, CAPTURER_STAGE_STOP_OK);
    AIUT_INFO_LOG("WriteDfxActionMsg with valid dfxCollector_ completed");
}

/**
 * @tc.name  : RecorderDfxWriter_WriteDfxActionMsg_NullDfxCollector
 * @tc.number: RecorderDfxWriter_WriteDfxActionMsg_NullDfxCollector
 * @tc.desc  : Test WriteDfxActionMsg when dfxCollector_ is nullptr (should return early).
 */
HWTEST_F(RecorderDfxWriterAiut, RecorderDfxWriter_WriteDfxActionMsg_NullDfxCollector, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo = GetTestAppInfo();
    auto writer = std::make_unique<RecorderDfxWriter>(appInfo, 1);
    AIUT_ASSERT_NE_INFO_LOG(writer, nullptr, "writer is null");
    writer->dfxCollector_ = nullptr;
    writer->WriteDfxActionMsg(1, CAPTURER_STAGE_START_OK);
    AIUT_INFO_LOG("WriteDfxActionMsg with null dfxCollector_ completed");
}

/**
 * @tc.name  : RecorderDfxWriter_Destructor_ValidDfxCollector
 * @tc.number: RecorderDfxWriter_Destructor_ValidDfxCollector
 * @tc.desc  : Test RecorderDfxWriter destructor with valid dfxCollector_.
 */
HWTEST_F(RecorderDfxWriterAiut, RecorderDfxWriter_Destructor_ValidDfxCollector, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo = GetTestAppInfo();
    auto writer = std::make_unique<RecorderDfxWriter>(appInfo, 1);
    AIUT_ASSERT_NE_INFO_LOG(writer, nullptr, "writer is null");
    writer.reset();
    AIUT_INFO_LOG("destructor with valid dfxCollector_ completed");
}

/**
 * @tc.name  : RecorderDfxWriter_Destructor_NullDfxCollector
 * @tc.number: RecorderDfxWriter_Destructor_NullDfxCollector
 * @tc.desc  : Test RecorderDfxWriter destructor when dfxCollector_ is nullptr.
 */
HWTEST_F(RecorderDfxWriterAiut, RecorderDfxWriter_Destructor_NullDfxCollector, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    AppInfo appInfo = GetTestAppInfo();
    auto writer = std::make_unique<RecorderDfxWriter>(appInfo, 1);
    AIUT_ASSERT_NE_INFO_LOG(writer, nullptr, "writer is null");
    writer->dfxCollector_ = nullptr;
    writer.reset();
    AIUT_INFO_LOG("destructor with null dfxCollector_ completed");
}

} // namespace AudioStandard
} // namespace OHOS
