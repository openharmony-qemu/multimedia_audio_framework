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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <unistd.h>
#include <string>

#include "audio_debug_manager_private.h"
#include "audio_debug_callback.h"
#include "audio_debug_info.h"
#include "audio_errors.h"

using namespace testing::ext;
using namespace std;

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr int32_t TEST_UID = 1234;
constexpr int32_t TEST_PID = 1000;
constexpr size_t BUF_SIZE = 8192;
constexpr uint32_t TEST_CAP_RATE = 16000;
constexpr size_t TEST_CAP_BUF = 4096;
constexpr uint32_t TEST_SESSION_ID = 200;
constexpr uint32_t TEST_STREAM_ID = 300;
}

class TestCapturerCallback : public AudioCapturerDebugCallback {
public:
    int32_t GetCapturerDebugInfo(AudioCapturerDebugInfo &info) override
    {
        info = info_;
        return ret_;
    }
    AudioCapturerDebugInfo info_;
    int32_t ret_ = SUCCESS;
};

class TestLoopbackCallback : public AudioLoopbackDebugCallback {
public:
    int32_t GetLoopbackDebugInfo(AudioLoopbackDebugInfo &info) override
    {
        info = info_;
        return ret_;
    }
    AudioLoopbackDebugInfo info_;
    int32_t ret_ = SUCCESS;
};

class TestSessionCallback : public AudioSessionDebugCallback {
public:
    int32_t GetSessionDebugInfo(AudioSessionDebugInfo &info) override
    {
        info = info_;
        return ret_;
    }
    AudioSessionDebugInfo info_;
    int32_t ret_ = SUCCESS;
};

AudioCapturerDebugInfo MakeCapturerInfo()
{
    AudioCapturerDebugInfo info;
    info.sessionId = TEST_SESSION_ID + 1;
    info.samplingRate = TEST_CAP_RATE;
    info.channels = AudioChannel::MONO;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.encoding = AudioEncodingType::ENCODING_PCM;
    info.channelLayout = AudioChannelLayout::CH_LAYOUT_MONO;
    info.sourceType = SOURCE_TYPE_MIC;
    info.capturerFlag = AUDIO_FLAG_NORMAL;
    info.bufferSize = TEST_CAP_BUF;
    info.pipeRole = PIPE_ROLE_INPUT;
    info.focusState = AudioFocuState::ACTIVE;
    return info;
}

AudioSessionDebugInfo MakeSessionInfo()
{
    AudioSessionDebugInfo info;
    info.strategy.concurrencyMode = AudioConcurrencyMode::MIX_WITH_OTHERS;
    info.audioSessionScene = AudioSessionScene::MEDIA;
    info.defaultDeviceType = DEVICE_TYPE_SPEAKER;
    info.state = AudioSessionState::SESSION_ACTIVE;
    info.fakeFocusState = AudioFocuState::ACTIVE;
    info.pid = TEST_PID;
    info.uid = TEST_UID;
    AudioStreamDebugInfo stream;
    stream.streamId = TEST_STREAM_ID;
    stream.streamType = STREAM_MUSIC;
    stream.focusState = AudioFocuState::ACTIVE;
    info.streamInfos.push_back(stream);
    return info;
}

bool MakePipe(int fd[2]) { return pipe(fd) == 0; }
string ReadPipe(int fd)
{
    char buf[BUF_SIZE] = {0};
    ssize_t n = read(fd, buf, sizeof(buf) - 1);
    return n <= 0 ? "" : string(buf, n);
}

class AudioDebugManagerUnitTestB : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

// ==================== Capturer Register/Unregister ====================

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_RegCapturer_001, TestSize.Level0)
{
    uint32_t key = 0;
    EXPECT_EQ(AudioDebugManager::GetInstance().RegisterAudioCapturer(key, nullptr), ERR_INVALID_PARAM);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_RegCapturer_002, TestSize.Level0)
{
    auto cb = make_shared<TestCapturerCallback>();
    uint32_t key = 0;
    EXPECT_EQ(AudioDebugManager::GetInstance().RegisterAudioCapturer(key, cb), SUCCESS);
    EXPECT_NE(key, 0u);
    AudioDebugManager::GetInstance().UnregisterAudioCapturer(key);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_UnregCapturer_001, TestSize.Level0)
{
    EXPECT_EQ(AudioDebugManager::GetInstance().UnregisterAudioCapturer(99999), ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_UnregCapturer_002, TestSize.Level0)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestCapturerCallback>();
    uint32_t key = 0;
    mgr.RegisterAudioCapturer(key, cb);
    EXPECT_EQ(mgr.UnregisterAudioCapturer(key), SUCCESS);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_RegCapturer_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb1 = make_shared<TestCapturerCallback>();
    uint32_t k1 = 0;
    mgr.RegisterAudioCapturer(k1, cb1);
    mgr.UnregisterAudioCapturer(k1);
    auto cb2 = make_shared<TestCapturerCallback>();
    uint32_t k2 = 0;
    EXPECT_EQ(mgr.RegisterAudioCapturer(k2, cb2), SUCCESS);
    EXPECT_NE(k2, 0u);
    EXPECT_NE(k2, k1);
    mgr.UnregisterAudioCapturer(k2);
}

// ==================== Capturer Print ====================

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintCapturer_001, TestSize.Level0)
{
    EXPECT_EQ(AudioDebugManager::GetInstance().PrintAudioCapturerDebugInfo(99999, -1), ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintCapturer_002, TestSize.Level0)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestCapturerCallback>();
    cb->info_ = MakeCapturerInfo();
    uint32_t key = 0;
    mgr.RegisterAudioCapturer(key, cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    EXPECT_EQ(mgr.PrintAudioCapturerDebugInfo(key, fd[1]), SUCCESS);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("audioCapturer"), string::npos);
    EXPECT_NE(out.find("SOURCE_TYPE_MIC"), string::npos);
    EXPECT_NE(out.find("PIPE_ROLE_INPUT"), string::npos);
    mgr.UnregisterAudioCapturer(key);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintCapturer_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestCapturerCallback>();
    cb->info_ = MakeCapturerInfo();
    uint32_t key = 0;
    mgr.RegisterAudioCapturer(key, cb);
    EXPECT_EQ(mgr.PrintAudioCapturerDebugInfo(key, -1), SUCCESS);
    mgr.UnregisterAudioCapturer(key);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintCapturer_004, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestCapturerCallback>();
    cb->ret_ = ERR_ILLEGAL_STATE;
    uint32_t key = 0;
    mgr.RegisterAudioCapturer(key, cb);
    EXPECT_EQ(mgr.PrintAudioCapturerDebugInfo(key, -1), ERR_ILLEGAL_STATE);
    mgr.UnregisterAudioCapturer(key);
}

// ==================== PrintAllAudioCapturers ====================

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintAllCapturer_001, TestSize.Level0)
{
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    EXPECT_EQ(AudioDebugManager::GetInstance().PrintAllAudioCapturersDebugInfo(fd[1]), SUCCESS);
    close(fd[1]);
    close(fd[0]);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintAllCapturer_002, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb1 = make_shared<TestCapturerCallback>();
    cb1->info_ = MakeCapturerInfo();
    cb1->info_.sessionId = 2001;
    auto cb2 = make_shared<TestCapturerCallback>();
    cb2->info_ = MakeCapturerInfo();
    cb2->info_.sessionId = 2002;
    uint32_t k1 = 0, k2 = 0;
    mgr.RegisterAudioCapturer(k1, cb1);
    mgr.RegisterAudioCapturer(k2, cb2);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    EXPECT_EQ(mgr.PrintAllAudioCapturersDebugInfo(fd[1]), SUCCESS);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("2001"), string::npos);
    EXPECT_NE(out.find("2002"), string::npos);
    mgr.UnregisterAudioCapturer(k1);
    mgr.UnregisterAudioCapturer(k2);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintAllCapturer_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto good = make_shared<TestCapturerCallback>();
    good->info_ = MakeCapturerInfo();
    auto bad = make_shared<TestCapturerCallback>();
    bad->ret_ = ERR_ILLEGAL_STATE;
    uint32_t k1 = 0, k2 = 0;
    mgr.RegisterAudioCapturer(k1, good);
    mgr.RegisterAudioCapturer(k2, bad);
    EXPECT_EQ(mgr.PrintAllAudioCapturersDebugInfo(-1), ERR_OPERATION_FAILED);
    mgr.UnregisterAudioCapturer(k1);
    mgr.UnregisterAudioCapturer(k2);
}

// ==================== Capturer Content ====================

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_CapturerContent_001, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestCapturerCallback>();
    cb->info_ = MakeCapturerInfo();
    cb->info_.muteWhenInterrupted = true;
    cb->info_.pauseWhenInterrupted = true;
    uint32_t key = 0;
    mgr.RegisterAudioCapturer(key, cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioCapturerDebugInfo(key, fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("true"), string::npos);
    mgr.UnregisterAudioCapturer(key);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_CapturerContent_002, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestCapturerCallback>();
    cb->info_ = MakeCapturerInfo();
    AudioFaultRecord rec;
    rec.uid = TEST_UID;
    rec.errorCode = 503;
    rec.errorReason = "cap_error";
    rec.timestamp = 1234567890;
    cb->info_.errorInfos.push_back(rec);
    uint32_t key = 0;
    mgr.RegisterAudioCapturer(key, cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioCapturerDebugInfo(key, fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("errorInfos"), string::npos);
    EXPECT_NE(out.find("cap_error"), string::npos);
    mgr.UnregisterAudioCapturer(key);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_CapturerContent_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestCapturerCallback>();
    cb->info_ = MakeCapturerInfo();
    cb->info_.sourceType = static_cast<SourceType>(9999);
    uint32_t key = 0;
    mgr.RegisterAudioCapturer(key, cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioCapturerDebugInfo(key, fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("UNKNOWN"), string::npos);
    mgr.UnregisterAudioCapturer(key);
}

// ==================== Loopback Register/Unregister ====================

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_RegLoopback_001, TestSize.Level0)
{
    uint32_t key = 0;
    EXPECT_EQ(AudioDebugManager::GetInstance().RegisterAudioLoopback(key, nullptr), ERR_INVALID_PARAM);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_RegLoopback_002, TestSize.Level0)
{
    auto cb = make_shared<TestLoopbackCallback>();
    uint32_t key = 0;
    EXPECT_EQ(AudioDebugManager::GetInstance().RegisterAudioLoopback(key, cb), SUCCESS);
    EXPECT_NE(key, 0u);
    AudioDebugManager::GetInstance().UnregisterAudioLoopback(key);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_UnregLoopback_001, TestSize.Level0)
{
    EXPECT_EQ(AudioDebugManager::GetInstance().UnregisterAudioLoopback(99999), SUCCESS);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_UnregLoopback_002, TestSize.Level0)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestLoopbackCallback>();
    uint32_t key = 0;
    mgr.RegisterAudioLoopback(key, cb);
    EXPECT_EQ(mgr.UnregisterAudioLoopback(key), SUCCESS);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_RegLoopback_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb1 = make_shared<TestLoopbackCallback>();
    uint32_t k1 = 0;
    mgr.RegisterAudioLoopback(k1, cb1);
    mgr.UnregisterAudioLoopback(k1);
    auto cb2 = make_shared<TestLoopbackCallback>();
    uint32_t k2 = 0;
    EXPECT_EQ(mgr.RegisterAudioLoopback(k2, cb2), SUCCESS);
    EXPECT_NE(k2, 0u);
    EXPECT_NE(k2, k1);
    mgr.UnregisterAudioLoopback(k2);
}

// ==================== Loopback Print ====================

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintLoopback_001, TestSize.Level0)
{
    EXPECT_EQ(AudioDebugManager::GetInstance().PrintAudioLoopbackDebugInfo(99999, -1), ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintLoopback_002, TestSize.Level0)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestLoopbackCallback>();
    cb->info_.appUid = TEST_UID;
    cb->info_.appName = "TestApp";
    cb->info_.mode = LOOPBACK_HARDWARE;
    cb->info_.currentState = LOOPBACK_STATE_RUNNING;
    cb->info_.activeOutputDevice = DEVICE_TYPE_USB_HEADSET;
    cb->info_.reverbPreset = REVERB_PRESET_KTV;
    cb->info_.equalizerPreset = EQUALIZER_PRESET_BRIGHT;
    cb->info_.volume = 75;
    cb->info_.uplinkStreamState = CAPTURER_RUNNING;
    cb->info_.downlinkStreamState = RENDERER_RUNNING;
    uint32_t key = 0;
    mgr.RegisterAudioLoopback(key, cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    EXPECT_EQ(mgr.PrintAudioLoopbackDebugInfo(key, fd[1]), SUCCESS);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("audioLoopback"), string::npos);
    EXPECT_NE(out.find("LOOPBACK_HARDWARE"), string::npos);
    EXPECT_NE(out.find("LOOPBACK_STATE_RUNNING"), string::npos);
    EXPECT_NE(out.find("CAPTURER_RUNNING"), string::npos);
    EXPECT_NE(out.find("RENDERER_RUNNING"), string::npos);
    mgr.UnregisterAudioLoopback(key);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintLoopback_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestLoopbackCallback>();
    cb->ret_ = ERR_ILLEGAL_STATE;
    uint32_t key = 0;
    mgr.RegisterAudioLoopback(key, cb);
    EXPECT_EQ(mgr.PrintAudioLoopbackDebugInfo(key, -1), ERR_ILLEGAL_STATE);
    mgr.UnregisterAudioLoopback(key);
}

// ==================== PrintAllAudioLoopbacks ====================

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintAllLoopback_001, TestSize.Level0)
{
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    EXPECT_EQ(AudioDebugManager::GetInstance().PrintAllAudioLoopbacksDebugInfo(fd[1]), SUCCESS);
    close(fd[1]);
    close(fd[0]);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintAllLoopback_002, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb1 = make_shared<TestLoopbackCallback>();
    cb1->info_.appName = "App1";
    auto cb2 = make_shared<TestLoopbackCallback>();
    cb2->info_.appName = "App2";
    uint32_t k1 = 0, k2 = 0;
    mgr.RegisterAudioLoopback(k1, cb1);
    mgr.RegisterAudioLoopback(k2, cb2);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    EXPECT_EQ(mgr.PrintAllAudioLoopbacksDebugInfo(fd[1]), SUCCESS);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("App1"), string::npos);
    EXPECT_NE(out.find("App2"), string::npos);
    mgr.UnregisterAudioLoopback(k1);
    mgr.UnregisterAudioLoopback(k2);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintAllLoopback_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto good = make_shared<TestLoopbackCallback>();
    good->info_.appName = "Good";
    auto bad = make_shared<TestLoopbackCallback>();
    bad->ret_ = ERR_ILLEGAL_STATE;
    uint32_t k1 = 0, k2 = 0;
    mgr.RegisterAudioLoopback(k1, good);
    mgr.RegisterAudioLoopback(k2, bad);
    EXPECT_EQ(mgr.PrintAllAudioLoopbacksDebugInfo(-1), ERR_OPERATION_FAILED);
    mgr.UnregisterAudioLoopback(k1);
    mgr.UnregisterAudioLoopback(k2);
}

// ==================== Loopback States ====================

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_LoopbackStates_001, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestLoopbackCallback>();
    cb->info_.currentState = LOOPBACK_STATE_IDLE;
    uint32_t key = 0;
    mgr.RegisterAudioLoopback(key, cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioLoopbackDebugInfo(key, fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("LOOPBACK_STATE_IDLE"), string::npos);
    mgr.UnregisterAudioLoopback(key);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_LoopbackStates_002, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestLoopbackCallback>();
    cb->info_.currentState = LOOPBACK_STATE_DESTROYING;
    uint32_t key = 0;
    mgr.RegisterAudioLoopback(key, cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioLoopbackDebugInfo(key, fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("LOOPBACK_STATE_DESTROYING"), string::npos);
    mgr.UnregisterAudioLoopback(key);
}

// ==================== Session SetCallback ====================

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_SetSessionCb_001, TestSize.Level0)
{
    EXPECT_EQ(AudioDebugManager::GetInstance().SetAudioSessionCallback(nullptr), ERR_INVALID_PARAM);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_SetSessionCb_002, TestSize.Level0)
{
    auto cb = make_shared<TestSessionCallback>();
    EXPECT_EQ(AudioDebugManager::GetInstance().SetAudioSessionCallback(cb), SUCCESS);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_SetSessionCb_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb1 = make_shared<TestSessionCallback>();
    auto cb2 = make_shared<TestSessionCallback>();
    EXPECT_EQ(mgr.SetAudioSessionCallback(cb1), SUCCESS);
    EXPECT_EQ(mgr.SetAudioSessionCallback(cb2), SUCCESS);
}

// ==================== Session Print ====================

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintSession_001, TestSize.Level0)
{
    int32_t ret = AudioDebugManager::GetInstance().PrintAudioSessionDebugInfo(-1);
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_ILLEGAL_STATE);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintSession_002, TestSize.Level0)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestSessionCallback>();
    cb->info_ = MakeSessionInfo();
    mgr.SetAudioSessionCallback(cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    EXPECT_EQ(mgr.PrintAudioSessionDebugInfo(fd[1]), SUCCESS);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("audioSession"), string::npos);
    EXPECT_NE(out.find("MIX_WITH_OTHERS"), string::npos);
    EXPECT_NE(out.find("SESSION_ACTIVE"), string::npos);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintSession_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestSessionCallback>();
    cb->info_ = MakeSessionInfo();
    mgr.SetAudioSessionCallback(cb);
    EXPECT_EQ(mgr.PrintAudioSessionDebugInfo(-1), SUCCESS);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_PrintSession_004, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestSessionCallback>();
    cb->ret_ = ERR_ILLEGAL_STATE;
    mgr.SetAudioSessionCallback(cb);
    EXPECT_EQ(mgr.PrintAudioSessionDebugInfo(-1), ERR_ILLEGAL_STATE);
}

// ==================== Session Scene Edge Cases ====================

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_SessionScene_001, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestSessionCallback>();
    cb->info_ = MakeSessionInfo();
    cb->info_.audioSessionScene = AudioSessionScene::INVALID;
    mgr.SetAudioSessionCallback(cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioSessionDebugInfo(fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("audioSession"), string::npos);
    EXPECT_NE(out.find("focusState"), string::npos);
    EXPECT_EQ(out.find("fakeFocusState"), string::npos);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_SessionScene_002, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestSessionCallback>();
    cb->info_ = MakeSessionInfo();
    cb->info_.audioSessionScene = AudioSessionScene::MEDIA;
    cb->info_.fakeFocusState = AudioFocuState::DUCK;
    mgr.SetAudioSessionCallback(cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioSessionDebugInfo(fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("fakeFocusState"), string::npos);
    EXPECT_NE(out.find("DUCK"), string::npos);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_SessionScene_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestSessionCallback>();
    cb->info_ = MakeSessionInfo();
    cb->info_.audioSessionScene = AudioSessionScene::VOICE_COMMUNICATION;
    cb->info_.fakeFocusState = AudioFocuState::PAUSE;
    mgr.SetAudioSessionCallback(cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioSessionDebugInfo(fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("VOICE_COMMUNICATION"), string::npos);
    EXPECT_NE(out.find("PAUSE"), string::npos);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_SessionScene_004, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestSessionCallback>();
    cb->info_ = MakeSessionInfo();
    cb->info_.strategy.concurrencyMode = AudioConcurrencyMode::PAUSE_OTHERS;
    mgr.SetAudioSessionCallback(cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioSessionDebugInfo(fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("PAUSE_OTHERS"), string::npos);
}

// ==================== Session Content ====================

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_SessionContent_001, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestSessionCallback>();
    cb->info_ = MakeSessionInfo();
    AudioStreamDebugInfo s2;
    s2.streamId = 400;
    s2.streamType = STREAM_VOICE_CALL;
    s2.focusState = AudioFocuState::DUCK;
    cb->info_.streamInfos.push_back(s2);
    mgr.SetAudioSessionCallback(cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioSessionDebugInfo(fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("STREAM_MUSIC"), string::npos);
    EXPECT_NE(out.find("STREAM_VOICE_CALL"), string::npos);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_SessionContent_002, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestSessionCallback>();
    cb->info_ = MakeSessionInfo();
    cb->info_.streamInfos.clear();
    mgr.SetAudioSessionCallback(cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    EXPECT_EQ(mgr.PrintAudioSessionDebugInfo(fd[1]), SUCCESS);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("audioSession"), string::npos);
    EXPECT_NE(out.find("streams"), string::npos);
}

HWTEST_F(AudioDebugManagerUnitTestB, AudioDebugManager_SessionContent_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestSessionCallback>();
    cb->info_ = MakeSessionInfo();
    cb->info_.pid = 5678;
    cb->info_.uid = 9012;
    mgr.SetAudioSessionCallback(cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioSessionDebugInfo(fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("5678"), string::npos);
    EXPECT_NE(out.find("9012"), string::npos);
}

} // namespace AudioStandard
} // namespace OHOS
