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

#include "pulse/context.h"
#include "audio_server_dump_unit_test.h"
#include "audio_server_dump.h"
#include "accesstoken_kit.h"
#include "audio_device_info.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_process_config.h"
#include "audio_server.h"
#include "audio_service.h"
#include "audio_stream_info.h"
#include "policy_handler.h"
#include "pa_adapter_tools.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioServerDumpUnitTest::SetUpTestCase(void) {}

void AudioServerDumpUnitTest::TearDownTestCase(void) {}

void AudioServerDumpUnitTest::SetUp(void) {}

void AudioServerDumpUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test OnTimeOut
 * @tc.type  : FUNC
 * @tc.number: AudioServerOnTimeOut_001
 * @tc.desc  : Test OnTimeOut set true end string
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerOnTimeOut_001, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    audioServerDump.OnTimeOut();

    EXPECT_NE(nullptr, audioServerDump.mainLoop);
}

/**
 * @tc.name  : Test IsEndWith
 * @tc.type  : FUNC
 * @tc.number: AudioServerIsEndWith_001
 * @tc.desc  : Test IsEndWith set true end string
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerIsEndWith_001, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    bool ret = audioServerDump.IsEndWith("Hello World!", "World!");
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test IsEndWith
 * @tc.type  : FUNC
 * @tc.number: AudioServerIsEndWith_002
 * @tc.desc  : Test IsEndWith set false end string
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerIsEndWith_002, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    bool ret = audioServerDump.IsEndWith("Hello World!", "Hello");
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test PlaybackSinkDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerPlaybackSinkDump_001
 * @tc.desc  : Test PlaybackSinkDump not enter the for loop
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPlaybackSinkDump_001, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string dumpString;
    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    audioServerDump.PlaybackSinkDump(dumpString);
    std::string expectedOutput = "Playback Streams\n- 0 Playback stream (s) available:\n";
    EXPECT_EQ(dumpString, expectedOutput);
}

/**
 * @tc.name  : Test PlaybackSinkDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerPlaybackSinkDump_002
 * @tc.desc  : Test PlaybackSinkDump enter the for loop
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPlaybackSinkDump_002, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string PSDumpString;
    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    InputOutputInfo testPlaybackSinkDump = {
        .sessionId = "test_sessionId",
        .applicationName = "test_app",
        .processId = "test_processId_1",
        .userId = 1,
        .privacyType = "0",
        .sampleSpec = {},
        .corked = false,
        .sessionStartTime = "PlaybackSinkDump"
    };
    audioServerDump.streamData_.sinkInputs.push_back(testPlaybackSinkDump);
    audioServerDump.PlaybackSinkDump(PSDumpString);
    std::string endWith = "- Stream Start Time: PlaybackSinkDump\n\n";
    bool ret = audioServerDump.IsEndWith(PSDumpString, endWith);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test RecordSourceDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerRecordSourceDump_001
 * @tc.desc  : Test RecordSourceDump not enter the for loop
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerRecordSourceDump_001, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string dumpString;
    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    audioServerDump.RecordSourceDump(dumpString);
    std::string expectedOutput = "Record Streams \n- 0 Record stream (s) available:\n";
    EXPECT_EQ(dumpString, expectedOutput);
}

/**
 * @tc.name  : Test RecordSourceDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerRecordSourceDump_002
 * @tc.desc  : Test RecordSourceDump enter the for loop
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerRecordSourceDump_002, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string RSDumpString;
    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    InputOutputInfo testRecordSourceDump = {
        .sessionId = "AudioServerRecordSourceDump_002",
        .applicationName = "RecordSourceDump",
        .processId = "test_processId_2",
        .userId = 1,
        .privacyType = "0",
        .sampleSpec = {},
        .corked = false,
        .sessionStartTime = "RecordSourceDump"
    };
    audioServerDump.streamData_.sourceOutputs.push_back(testRecordSourceDump);
    audioServerDump.RecordSourceDump(RSDumpString);
    std::string endWith = "- Stream Start Time: RecordSourceDump\n\n";
    bool ret = audioServerDump.IsEndWith(RSDumpString, endWith);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name  : Test HDFModulesDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerHDFModulesDump_001
 * @tc.desc  : Test HDFModulesDump not enter the for loop
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerHDFModulesDump_001, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string HDFModulesDumpString;
    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    audioServerDump.HDFModulesDump(HDFModulesDumpString);
    std::string ret = "\nHDF Input Modules\n- 0 HDF Input Modules (s) available:\n"
                        "HDF Output Modules\n- 0 HDF Output Modules (s) available:\n";
    EXPECT_EQ(HDFModulesDumpString, ret);
}

/**
 * @tc.name  : Test HDFModulesDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerHDFModulesDump_002
 * @tc.desc  : Test HDFModulesDump enter the first for loop
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerHDFModulesDump_002, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string HDFModulesDumpString;
    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    SinkSourceInfo testSourceDevices = {
        .name = "testSourceDevices",
        .sampleSpec = {},
    };
    audioServerDump.streamData_.sourceDevices.push_back(testSourceDevices);
    audioServerDump.HDFModulesDump(HDFModulesDumpString);
    std::string head = "\nHDF Input Modules\n"
                        "- 1 HDF Input Modules (s) available:\n"
                        "  Module";
    int32_t ret = HDFModulesDumpString.rfind(head, 0);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name  : Test HDFModulesDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerHDFModulesDump_003
 * @tc.desc  : Test HDFModulesDump enter the second for loop
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerHDFModulesDump_003, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string HDFSinkDumpString;
    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    SinkSourceInfo testSinkDevices = {
        .name = "testSinkDevices",
        .sampleSpec = {},
    };
    audioServerDump.streamData_.sinkDevices.push_back(testSinkDevices);
    audioServerDump.HDFModulesDump(HDFSinkDumpString);
    std::string head = "\nHDF Input Modules\n"
                       "- 0 HDF Input Modules (s) available:\n"
                       "HDF Output Modules\n"
                       "- 1 HDF Output Modules (s) available:\n"
                       "  Module";
    int32_t ret = HDFSinkDumpString.rfind(head, 0);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name  : Test IsValidModule
 * @tc.type  : FUNC
 * @tc.number: AudioServerIsValidModule_001
 * @tc.desc  : Test IsValidModule set info begin with fifo
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerIsValidModule_001, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    bool ret;
    ret = audioServerDump.IsValidModule("fifo");
    EXPECT_EQ(false, ret);

    ret = audioServerDump.IsValidModule("fifotest");
    EXPECT_EQ(false, ret);

    ret = audioServerDump.IsValidModule("fifo123");
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test IsValidModule
 * @tc.type  : FUNC
 * @tc.number: AudioServerIsValidModule_002
 * @tc.desc  : Test IsValidModule set info end with monitor
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerIsValidModule_002, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    bool ret;
    ret = audioServerDump.IsValidModule("monitor");
    EXPECT_EQ(false, ret);

    ret = audioServerDump.IsValidModule("testmonitor");
    EXPECT_EQ(false, ret);

    ret = audioServerDump.IsValidModule("test.monitor");
    EXPECT_EQ(false, ret);

    ret = audioServerDump.IsValidModule("test_monitor");
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name  : Test IsValidModule
 * @tc.type  : FUNC
 * @tc.number: AudioServerIsValidModule_003
 * @tc.desc  : Test IsValidModule set right info
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerIsValidModule_003, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    bool ret;
    ret = audioServerDump.IsValidModule("test");
    EXPECT_EQ(true, ret);

    ret = audioServerDump.IsValidModule("module");
    EXPECT_EQ(true, ret);

    ret = audioServerDump.IsValidModule("valid_module");
    EXPECT_EQ(true, ret);

    ret = audioServerDump.IsValidModule("test_module");
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name  : Test ArgDataDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerArgDataDump_001
 * @tc.desc  : Test ArgDataDump set argQue.empty()
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerArgDataDump_001, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string dumpString;
    std::queue<std::u16string> argQue;
    audioServerDump.ArgDataDump(dumpString, argQue);
    EXPECT_TRUE(argQue.empty());
}

/**
 * @tc.name  : Test ArgDataDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerArgDataDump_002
 * @tc.desc  : Test ArgDataDump set para == u"-h"
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerArgDataDump_002, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string dumpString;
    std::queue<std::u16string> argQue;
    argQue.push(u"-h");
    std::string expectedHelpInfo = "usage:\n"
                                   "  -h\t\t\t|help text for hidumper audio\n"
                                   "  -p\t\t\t|dump pa playback streams\n"
                                   "  -r\t\t\t|dump pa record streams\n"
                                   "  -m\t\t\t|dump hdf input modules\n"
                                   "  -ep\t\t\t|dump policyhandler info\n"
                                   "  -ct\t\t\t|dump AudioCached time info\n"
                                   "  -cm\t\t\t|dump AudioCached memory info\n"
                                   "  -pm\t\t\t|dump AudioPerformMonitor info\n";
    audioServerDump.ArgDataDump(dumpString, argQue);
    EXPECT_NE(expectedHelpInfo, dumpString);
}

/**
 * @tc.name  : Test ArgDataDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerArgDataDump_003
 * @tc.desc  : Test ArgDataDump set para == u"invalid_param"
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerArgDataDump_003, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string dumpString;
    std::queue<std::u16string> argQue;
    argQue.push(u"invalid_param");
    std::string expectedHelpInfo = "Please input correct param:\n"
                                   "usage:\n"
                                   "  -h\t\t\t|help text for hidumper audio\n"
                                   "  -p\t\t\t|dump pa playback streams\n"
                                   "  -r\t\t\t|dump pa record streams\n"
                                   "  -m\t\t\t|dump hdf input modules\n"
                                   "  -ep\t\t\t|dump policyhandler info\n"
                                   "  -ct\t\t\t|dump AudioCached time info\n"
                                   "  -cm\t\t\t|dump AudioCached memory info\n"
                                   "  -pm\t\t\t|dump AudioPerformMonitor info\n";
    audioServerDump.ArgDataDump(dumpString, argQue);
    EXPECT_NE(expectedHelpInfo, dumpString);
}

/**
 * @tc.name  : Test ArgDataDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerArgDataDump_004
 * @tc.desc  : Test ArgDataDump set para == u"-p"
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerArgDataDump_004, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string dumpString;
    std::queue<std::u16string> argQue;
    argQue.push(u"-p");
    std::string expectedInfo = "AudioServer Data Dump:\n\n"
                               "Playback Streams\n- 0 Playback stream (s) available:\n";
    audioServerDump.ArgDataDump(dumpString, argQue);
    EXPECT_EQ(expectedInfo, dumpString);
}

/**
 * @tc.name  : Test ArgDataDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerArgDataDump_005
 * @tc.desc  : Test ArgDataDump set para == u"-r"
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerArgDataDump_005, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string dumpString;
    std::queue<std::u16string> argQue;
    argQue.push(u"-r");
    std::string expectedInfo = "AudioServer Data Dump:\n\n"
                               "Record Streams \n- 0 Record stream (s) available:\n";
    audioServerDump.ArgDataDump(dumpString, argQue);
    EXPECT_EQ(expectedInfo, dumpString);
}

/**
 * @tc.name  : Test ArgDataDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerArgDataDump_006
 * @tc.desc  : Test ArgDataDump set para == u"-m"
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerArgDataDump_006, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string dumpString;
    std::queue<std::u16string> argQue;
    argQue.push(u"-m");
    std::string expectedInfo = "AudioServer Data Dump:\n\n"
                               "\nHDF Input Modules\n- 0 HDF Input Modules (s) available:\n"
                               "HDF Output Modules\n- 0 HDF Output Modules (s) available:\n";
    audioServerDump.ArgDataDump(dumpString, argQue);
    EXPECT_EQ(expectedInfo, dumpString);
}

/**
 * @tc.name  : Test AudioDataDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerAudioDataDump_001
 * @tc.desc  : Test AudioDataDump set mainLoop and context are nullptr
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerAudioDataDump_001, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string dumpString;
    std::queue<std::u16string> argQue;
    audioServerDump.AudioDataDump(dumpString, argQue);
    EXPECT_EQ(nullptr, audioServerDump.mainLoop);
    EXPECT_EQ(nullptr, audioServerDump.context);
}

/**
 * @tc.name  : Test AudioDataDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerAudioDataDump_002
 * @tc.desc  : Test AudioDataDump set mainLoop and context are not nullptr
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerAudioDataDump_002, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string dumpString;
    std::queue<std::u16string> argQue;
    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    audioServerDump.AudioDataDump(dumpString, argQue);
    EXPECT_NE(nullptr, audioServerDump.mainLoop);
    EXPECT_NE(nullptr, audioServerDump.context);
}

/**
 * @tc.name  : Test AudioDataDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerAudioDataDump_003
 * @tc.desc  : Test AudioDataDump set mainLoop is nullptr and context is not nullptr
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerAudioDataDump_003, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string dumpString;
    std::queue<std::u16string> argQue;
    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    audioServerDump.mainLoop = nullptr;
    audioServerDump.AudioDataDump(dumpString, argQue);
    EXPECT_NE(nullptr, audioServerDump.context);
}

/**
 * @tc.name  : Test AudioDataDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerAudioDataDump_004
 * @tc.desc  : Test AudioDataDump set context is nullptr and mainLoop is not nullptr
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerAudioDataDump_004, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    std::string dumpString;
    std::queue<std::u16string> argQue;
    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    audioServerDump.context = nullptr;
    audioServerDump.AudioDataDump(dumpString, argQue);
    EXPECT_NE(nullptr, audioServerDump.mainLoop);
}

#ifdef TEMP_DISABLE
/**
 * @tc.name  : Test ResetPAAudioDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerResetPAAudioDump_001
 * @tc.desc  : Test ResetPAAudioDump set mainLoop is not nullptr and isMainLoopStarted_ is true
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerResetPAAudioDump_001, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    EXPECT_NE(nullptr, audioServerDump.mainLoop);
    EXPECT_TRUE(audioServerDump.isMainLoopStarted_);
    audioServerDump.ResetPAAudioDump();
    EXPECT_EQ(nullptr, audioServerDump.mainLoop);
    EXPECT_FALSE(audioServerDump.isMainLoopStarted_);
}
#endif

/**
 * @tc.name  : Test ResetPAAudioDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerResetPAAudioDump_002
 * @tc.desc  : Test ResetPAAudioDump set mainLoop is nullptr and isMainLoopStarted_ is false
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerResetPAAudioDump_002, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    EXPECT_EQ(nullptr, audioServerDump.mainLoop);
    EXPECT_FALSE(audioServerDump.isMainLoopStarted_);
    audioServerDump.ResetPAAudioDump();
    EXPECT_EQ(nullptr, audioServerDump.mainLoop);
    EXPECT_FALSE(audioServerDump.isMainLoopStarted_);
}

/**
 * @tc.name  : Test ResetPAAudioDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerResetPAAudioDump_003
 * @tc.desc  : Test ResetPAAudioDump set mainLoop is nullptr and isMainLoopStarted_ is true
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerResetPAAudioDump_003, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    EXPECT_NE(nullptr, audioServerDump.mainLoop);
    EXPECT_TRUE(audioServerDump.isMainLoopStarted_);
    audioServerDump.mainLoop = nullptr;
    audioServerDump.ResetPAAudioDump();
    EXPECT_FALSE(audioServerDump.isMainLoopStarted_);
}

/**
 * @tc.name  : Test ResetPAAudioDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerResetPAAudioDump_004
 * @tc.desc  : Test ResetPAAudioDump set mainLoop is not nullptr and isMainLoopStarted_ is false
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerResetPAAudioDump_004, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    EXPECT_NE(nullptr, audioServerDump.mainLoop);
    EXPECT_TRUE(audioServerDump.isMainLoopStarted_);
    audioServerDump.isMainLoopStarted_ = false;
    audioServerDump.ResetPAAudioDump();
    EXPECT_EQ(nullptr, audioServerDump.mainLoop);
    EXPECT_FALSE(audioServerDump.isMainLoopStarted_);
}

/**
 * @tc.name  : Test ResetPAAudioDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerResetPAAudioDump_005
 * @tc.desc  : Test ResetPAAudioDump set context is not nullptr and isContextConnected_ is true
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerResetPAAudioDump_005, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    EXPECT_NE(nullptr, audioServerDump.context);
    EXPECT_TRUE(audioServerDump.isContextConnected_);
    audioServerDump.ResetPAAudioDump();
    EXPECT_EQ(nullptr, audioServerDump.context);
    EXPECT_FALSE(audioServerDump.isContextConnected_);
}

/**
 * @tc.name  : Test ResetPAAudioDump
 * @tc.type  : FUNC
 * @tc.number: AudioServerResetPAAudioDump_006
 * @tc.desc  : Test ResetPAAudioDump set context is not nullptr and isContextConnected_ is false
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerResetPAAudioDump_006, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    int32_t res = audioServerDump.Initialize();
    ASSERT_EQ(res, AUDIO_DUMP_SUCCESS) << "Initialize failed, server may not start!";
    audioServerDump.isContextConnected_ = false;
    EXPECT_NE(nullptr, audioServerDump.context);
    EXPECT_FALSE(audioServerDump.isContextConnected_);
    audioServerDump.ResetPAAudioDump();
}

/**
 * @tc.name  : Test PASinkInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASinkInfoCallback_001
 * @tc.desc  : Test PASinkInfoCallback
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASinkInfoCallback_001, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_sink_info info;

    info.sample_spec = {PA_SAMPLE_S16NE, 44100, 2};
    info.name = "PASinkInfo";

    audioServerDump.PASinkInfoCallback(nullptr, &info, 0, &audioServerDump);
    const SinkSourceInfo& newInfo = audioServerDump.streamData_.sinkDevices.front();

    EXPECT_EQ("PASinkInfo", newInfo.name);
}

/**
 * @tc.name  : Test PASinkInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASinkInfoCallback_002
 * @tc.desc  : Test PASinkInfoCallback is null
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASinkInfoCallback_002, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_sink_info info;

    info.sample_spec = {};
    info.name = "fifotest";

    audioServerDump.PASinkInfoCallback(nullptr, &info, 0, &audioServerDump);

    bool ret = audioServerDump.IsValidModule(info.name);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test PASinkInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASinkInfoCallback_003
 * @tc.desc  : Test PASinkInfoCallback is null
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASinkInfoCallback_003, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_sink_info info;

    info.sample_spec = {};
    info.name = "testmonitor";

    audioServerDump.PASinkInfoCallback(nullptr, &info, 0, &audioServerDump);

    bool ret = audioServerDump.IsValidModule(info.name);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test PASinkInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASinkInfoCallback_004
 * @tc.desc  : Test PASinkInfoCallback is null
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASinkInfoCallback_004, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_sink_info info;

    info.sample_spec = {};
    info.name = "test";

    audioServerDump.PASinkInfoCallback(nullptr, &info, 0, &audioServerDump);

    const SinkSourceInfo& newInfo = audioServerDump.streamData_.sinkDevices.front();

    EXPECT_EQ("test", newInfo.name);
}

/**
 * @tc.name  : Test PASinkInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASinkInfoCallback_005
 * @tc.desc  : Test PASinkInfoCallback is null
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASinkInfoCallback_005, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_sink_info info;

    audioServerDump.PASinkInfoCallback(nullptr, &info, 0, &audioServerDump);

    EXPECT_TRUE(info.name == nullptr);
}

/**
 * @tc.name  : Test PASinkInputInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASinkInputInfoCallback_001
 * @tc.desc  : Test PASinkInputInfoCallback
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASinkInputInfoCallback_001, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_proplist *proplist = pa_proplist_new();

    pa_proplist_sets(proplist, "application.name", "test_name");
    pa_proplist_sets(proplist, "application.process.id", "process_id");
    pa_proplist_sets(proplist, "application.process.user", "123");
    pa_proplist_sets(proplist, "stream.sessionID", "session_id");
    pa_proplist_sets(proplist, "stream.startTime", "session_start_time");
    pa_proplist_sets(proplist, "stream.privacyType", "test_type");

    pa_sink_input_info info;

    info.proplist = proplist;

    info.sample_spec = {PA_SAMPLE_S16NE, 44100, 2};
    info.corked = 1;

    audioServerDump.PASinkInputInfoCallback(nullptr, &info, 0, &audioServerDump);
    const InputOutputInfo& newInfo = audioServerDump.streamData_.sinkInputs.front();

    EXPECT_EQ("test_name", newInfo.applicationName);
    EXPECT_EQ("process_id", newInfo.processId);
    EXPECT_EQ(0, newInfo.userId);
    EXPECT_EQ("session_id", newInfo.sessionId);
    EXPECT_EQ("session_start_time", newInfo.sessionStartTime);
    EXPECT_EQ("test_type", newInfo.privacyType);
}

/**
 * @tc.name  : Test PASinkInputInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASinkInputInfoCallback_002
 * @tc.desc  : Test PASinkInputInfoCallback
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASinkInputInfoCallback_002, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_proplist *proplist = pa_proplist_new();

    pa_sink_input_info info;

    info.proplist = proplist;

    audioServerDump.PASinkInputInfoCallback(nullptr, &info, 0, &audioServerDump);
    const InputOutputInfo& firstSinkInput = audioServerDump.streamData_.sinkInputs.front();

    EXPECT_EQ("", firstSinkInput.applicationName);
}

/**
 * @tc.name  : Test PASinkInputInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASinkInputInfoCallback_003
 * @tc.desc  : Test PASinkInputInfoCallback
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASinkInputInfoCallback_003, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_proplist *proplist = nullptr;

    pa_sink_input_info info;

    info.proplist = proplist;

    audioServerDump.PASinkInputInfoCallback(nullptr, &info, 0, &audioServerDump);
    const InputOutputInfo& firstSinkInput = audioServerDump.streamData_.sinkInputs.front();

    EXPECT_EQ("", firstSinkInput.applicationName);
}

/**
 * @tc.name  : Test PASourceInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASourceInfoCallback_001
 * @tc.desc  : Test PASourceInfoCallback
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASourceInfoCallback_001, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_source_info info;

    info.sample_spec = {PA_SAMPLE_S16NE, 44100, 2};
    info.name = "PASourceInfo";

    audioServerDump.PASourceInfoCallback(nullptr, &info, 0, &audioServerDump);
    const SinkSourceInfo& newInfo = audioServerDump.streamData_.sourceDevices.front();

    EXPECT_EQ("PASourceInfo", newInfo.name);
}

/**
 * @tc.name  : Test PASourceInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASourceInfoCallback_002
 * @tc.desc  : Test PASourceInfoCallback is null
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASourceInfoCallback_002, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_source_info info;

    info.sample_spec = {};
    info.name = "fifotest";

    audioServerDump.PASourceInfoCallback(nullptr, &info, 0, &audioServerDump);

    bool ret = audioServerDump.IsValidModule(info.name);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test PASourceInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASourceInfoCallback_003
 * @tc.desc  : Test PASourceInfoCallback is null
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASourceInfoCallback_003, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_source_info info;

    info.sample_spec = {};
    info.name = "testmonitor";

    audioServerDump.PASourceInfoCallback(nullptr, &info, 0, &audioServerDump);
    bool ret = audioServerDump.IsValidModule(info.name);
    EXPECT_FALSE(ret);
}

/**
 * @tc.name  : Test PASourceInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASourceInfoCallback_004
 * @tc.desc  : Test PASourceInfoCallback
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASourceInfoCallback_004, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_source_info info;
    info.sample_spec = {PA_SAMPLE_S16NE, 44100, 2};
    info.name = "test";

    audioServerDump.PASourceInfoCallback(nullptr, &info, 0, &audioServerDump);
    const SinkSourceInfo& newInfo = audioServerDump.streamData_.sourceDevices.front();
    EXPECT_EQ("test", newInfo.name);
}

/**
 * @tc.name  : Test PASourceInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASourceInfoCallback_005
 * @tc.desc  : Test PASourceInfoCallback is null
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASourceInfoCallback_005, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_source_info info;

    audioServerDump.PASourceInfoCallback(nullptr, &info, 0, &audioServerDump);
    EXPECT_TRUE(info.name == nullptr);
}

/**
 * @tc.name  : Test PASourceOutputInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASourceOutputInfoCallback_001
 * @tc.desc  : Test PASourceOutputInfoCallback
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASourceOutputInfoCallback_001, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_proplist *proplist = pa_proplist_new();

    pa_proplist_sets(proplist, "application.name", "test_name");
    pa_proplist_sets(proplist, "application.process.id", "process_id");
    pa_proplist_sets(proplist, "application.process.user", "123");
    pa_proplist_sets(proplist, "stream.sessionID", "session_id");
    pa_proplist_sets(proplist, "stream.startTime", "session_start_time");
    pa_proplist_sets(proplist, "stream.privacyType", "test_type");

    pa_source_output_info info;
    info.proplist = proplist;
    info.sample_spec = {PA_SAMPLE_S16NE, 44100, 2};
    info.corked = 1;

    audioServerDump.PASourceOutputInfoCallback(nullptr, &info, 0, &audioServerDump);
    const InputOutputInfo& newInfo = audioServerDump.streamData_.sourceOutputs.front();

    EXPECT_EQ("test_name", newInfo.applicationName);
    EXPECT_EQ("process_id", newInfo.processId);
    EXPECT_EQ(0, newInfo.userId);
    EXPECT_EQ("session_id", newInfo.sessionId);
    EXPECT_EQ("session_start_time", newInfo.sessionStartTime);
    EXPECT_EQ("test_type", newInfo.privacyType);
}

/**
 * @tc.name  : Test PASourceOutputInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASourceOutputInfoCallback_002
 * @tc.desc  : Test PASourceOutputInfoCallback is nullptr
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASourceOutputInfoCallback_002, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_proplist *proplist = pa_proplist_new();
    pa_source_output_info info;
    info.proplist = proplist;

    audioServerDump.PASourceOutputInfoCallback(nullptr, &info, 0, &audioServerDump);
    EXPECT_TRUE(pa_proplist_gets(info.proplist, "application.name") == nullptr);
}

/**
 * @tc.name  : Test PASourceOutputInfoCallback
 * @tc.type  : FUNC
 * @tc.number: AudioServerPASourceOutputInfoCallback_003
 * @tc.desc  : Test PASourceOutputInfoCallback is nullptr
 */
HWTEST_F(AudioServerDumpUnitTest, AudioServerPASourceOutputInfoCallback_003, TestSize.Level1)
{
    AudioServerDump audioServerDump;

    pa_proplist *proplist = nullptr;
    pa_source_output_info info;
    info.proplist = proplist;

    audioServerDump.PASourceOutputInfoCallback(nullptr, &info, 0, &audioServerDump);
    const InputOutputInfo& newInfo = audioServerDump.streamData_.sourceOutputs.front();
    EXPECT_EQ("", newInfo.applicationName);
}
} // namespace AudioStandard
} // namespace OHOS