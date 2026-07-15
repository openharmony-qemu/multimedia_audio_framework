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
#include <sstream>

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
constexpr uint32_t TEST_RATE = 44100;
constexpr uint32_t TEST_CAP_RATE = 16000;
constexpr size_t TEST_CAP_BUF = 4096;
constexpr uint32_t TEST_PIPE_ID = 100;
constexpr uint32_t TEST_SESSION_ID = 200;
constexpr uint32_t TEST_STREAM_ID = 300;
constexpr float TEST_VOL = 0.5f;
constexpr float TEST_SPEED = 1.5f;
constexpr float TEST_PITCH = 1.0f;
}

class TestRendererCallback : public AudioRendererDebugCallback {
public:
    int32_t GetRendererDebugInfo(AudioRendererDebugInfo &info) override
    {
        info = info_;
        return ret_;
    }
    AudioRendererDebugInfo info_;
    int32_t ret_ = SUCCESS;
};

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

AudioRendererDebugInfo MakeRendererInfo()
{
    AudioRendererDebugInfo info;
    info.sessionId = TEST_SESSION_ID;
    info.samplingRate = TEST_RATE;
    info.channels = AudioChannel::STEREO;
    info.format = AudioSampleFormat::SAMPLE_S16LE;
    info.encoding = AudioEncodingType::ENCODING_PCM;
    info.streamUsage = STREAM_USAGE_MUSIC;
    info.rendererFlags = AUDIO_FLAG_NORMAL;
    info.speed = TEST_SPEED;
    info.pitch = TEST_PITCH;
    info.effectMode = EFFECT_DEFAULT;
    info.pipeId = TEST_PIPE_ID;
    info.pipeRole = PIPE_ROLE_OUTPUT;
    info.pipeName = "test_pipe";
    info.routeFlag = AUDIO_OUTPUT_FLAG_NORMAL;
    info.adapterName = "test_adapter";
    info.mainDeviceType = DEVICE_TYPE_SPEAKER;
    info.volumeType = STREAM_MUSIC;
    info.streamVolume = TEST_VOL;
    info.systemVolume = 0.8f;
    info.volume = 0.6f;
    info.focusState = AudioFocuState::ACTIVE;
    return info;
}

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
    info.overflowCount = 0;
    info.muteWhenInterrupted = false;
    info.pauseWhenInterrupted = false;
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

class AudioDebugManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    void SetUp() {}
    void TearDown() {}
};

// ==================== GetInstance ====================

/**
 * @tc.name   : AudioDebugManager_GetInstance_001
 * @tc.number : AudioDebugManager_GetInstance_001
 * @tc.desc   : Test GetInstance returns valid instance.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_GetInstance_001, TestSize.Level0)
{
    auto &inst = AudioDebugManager::GetInstance();
    ASSERT_NE(&inst, nullptr);
}

/**
 * @tc.name   : AudioDebugManager_GetInstance_002
 * @tc.number : AudioDebugManager_GetInstance_002
 * @tc.desc   : Test GetInstance returns same singleton.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_GetInstance_002, TestSize.Level0)
{
    auto &a = AudioDebugManager::GetInstance();
    auto &b = AudioDebugManager::GetInstance();
    EXPECT_EQ(&a, &b);
}

// ==================== PrintAppAudioDebugInfo ====================

/**
 * @tc.name   : AudioDebugManager_PrintApp_001
 * @tc.number : AudioDebugManager_PrintApp_001
 * @tc.desc   : Test PrintAppAudioDebugInfo with no callbacks.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_PrintApp_001, TestSize.Level0)
{
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    int32_t ret = AudioDebugManager::GetInstance().PrintAppAudioDebugInfo(fd[1]);
    close(fd[1]);
    close(fd[0]);
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_OPERATION_FAILED);
}

/**
 * @tc.name   : AudioDebugManager_PrintApp_002
 * @tc.number : AudioDebugManager_PrintApp_002
 * @tc.desc   : Test PrintAppAudioDebugInfo with all callback types registered.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_PrintApp_002, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto rCb = make_shared<TestRendererCallback>();
    rCb->info_ = MakeRendererInfo();
    uint32_t rKey = 0;
    mgr.RegisterAudioRenderer(rKey, rCb);

    auto cCb = make_shared<TestCapturerCallback>();
    cCb->info_ = MakeCapturerInfo();
    uint32_t cKey = 0;
    mgr.RegisterAudioCapturer(cKey, cCb);

    auto lCb = make_shared<TestLoopbackCallback>();
    lCb->info_.appUid = TEST_UID;
    lCb->info_.appName = "LoopbackApp";
    uint32_t lKey = 0;
    mgr.RegisterAudioLoopback(lKey, lCb);

    auto sCb = make_shared<TestSessionCallback>();
    sCb->info_ = MakeSessionInfo();
    mgr.SetAudioSessionCallback(sCb);

    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    int32_t ret = mgr.PrintAppAudioDebugInfo(fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_NE(out.find("audioRenderer"), string::npos);
    EXPECT_NE(out.find("audioCapturer"), string::npos);
    EXPECT_NE(out.find("audioLoopback"), string::npos);
    EXPECT_NE(out.find("audioSession"), string::npos);

    mgr.UnregisterAudioRenderer(rKey);
    mgr.UnregisterAudioCapturer(cKey);
    mgr.UnregisterAudioLoopback(lKey);
}

/**
 * @tc.name   : AudioDebugManager_PrintApp_003
 * @tc.number : AudioDebugManager_PrintApp_003
 * @tc.desc   : Test PrintAppAudioDebugInfo with negative fd (log fallback).
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_PrintApp_003, TestSize.Level1)
{
    int32_t ret = AudioDebugManager::GetInstance().PrintAppAudioDebugInfo(-1);
    EXPECT_EQ(ret, SUCCESS);
}

// ==================== DebugKey Generation ====================

/**
 * @tc.name   : AudioDebugManager_KeyGen_001
 * @tc.number : AudioDebugManager_KeyGen_001
 * @tc.desc   : Test keys are unique and non-zero across types.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_KeyGen_001, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto rCb = make_shared<TestRendererCallback>();
    auto cCb = make_shared<TestCapturerCallback>();
    auto lCb = make_shared<TestLoopbackCallback>();

    uint32_t r = 0, c = 0, l = 0;
    mgr.RegisterAudioRenderer(r, rCb);
    mgr.RegisterAudioCapturer(c, cCb);
    mgr.RegisterAudioLoopback(l, lCb);

    EXPECT_NE(r, 0u);
    EXPECT_NE(c, 0u);
    EXPECT_NE(l, 0u);
    EXPECT_NE(r, c);
    EXPECT_NE(r, l);
    EXPECT_NE(c, l);

    mgr.UnregisterAudioRenderer(r);
    mgr.UnregisterAudioCapturer(c);
    mgr.UnregisterAudioLoopback(l);
}

// ==================== Renderer Register/Unregister ====================

/**
 * @tc.name   : AudioDebugManager_RegRenderer_001
 * @tc.number : AudioDebugManager_RegRenderer_001
 * @tc.desc   : Test RegisterAudioRenderer with null callback.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_RegRenderer_001, TestSize.Level0)
{
    uint32_t key = 0;
    EXPECT_EQ(AudioDebugManager::GetInstance().RegisterAudioRenderer(key, nullptr), ERR_INVALID_PARAM);
}

/**
 * @tc.name   : AudioDebugManager_RegRenderer_002
 * @tc.number : AudioDebugManager_RegRenderer_002
 * @tc.desc   : Test RegisterAudioRenderer success and key generation.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_RegRenderer_002, TestSize.Level0)
{
    auto cb = make_shared<TestRendererCallback>();
    uint32_t key = 0;
    EXPECT_EQ(AudioDebugManager::GetInstance().RegisterAudioRenderer(key, cb), SUCCESS);
    EXPECT_NE(key, 0u);
    AudioDebugManager::GetInstance().UnregisterAudioRenderer(key);
}

/**
 * @tc.name   : AudioDebugManager_RegRenderer_003
 * @tc.number : AudioDebugManager_RegRenderer_003
 * @tc.desc   : Test multiple registrations generate unique keys.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_RegRenderer_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb1 = make_shared<TestRendererCallback>();
    auto cb2 = make_shared<TestRendererCallback>();
    uint32_t k1 = 0, k2 = 0;
    mgr.RegisterAudioRenderer(k1, cb1);
    mgr.RegisterAudioRenderer(k2, cb2);
    EXPECT_NE(k1, k2);
    mgr.UnregisterAudioRenderer(k1);
    mgr.UnregisterAudioRenderer(k2);
}

/**
 * @tc.name   : AudioDebugManager_UnregRenderer_001
 * @tc.number : AudioDebugManager_UnregRenderer_001
 * @tc.desc   : Test UnregisterAudioRenderer with non-existent key.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_UnregRenderer_001, TestSize.Level0)
{
    EXPECT_EQ(AudioDebugManager::GetInstance().UnregisterAudioRenderer(99999), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name   : AudioDebugManager_UnregRenderer_002
 * @tc.number : AudioDebugManager_UnregRenderer_002
 * @tc.desc   : Test UnregisterAudioRenderer success.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_UnregRenderer_002, TestSize.Level0)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestRendererCallback>();
    uint32_t key = 0;
    mgr.RegisterAudioRenderer(key, cb);
    EXPECT_EQ(mgr.UnregisterAudioRenderer(key), SUCCESS);
}

/**
 * @tc.name   : AudioDebugManager_UnregRenderer_003
 * @tc.number : AudioDebugManager_UnregRenderer_003
 * @tc.desc   : Test double unregister returns error.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_UnregRenderer_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestRendererCallback>();
    uint32_t key = 0;
    mgr.RegisterAudioRenderer(key, cb);
    EXPECT_EQ(mgr.UnregisterAudioRenderer(key), SUCCESS);
    EXPECT_EQ(mgr.UnregisterAudioRenderer(key), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name   : AudioDebugManager_RegRenderer_004
 * @tc.number : AudioDebugManager_RegRenderer_004
 * @tc.desc   : Test re-register after unregister.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_RegRenderer_004, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb1 = make_shared<TestRendererCallback>();
    uint32_t k1 = 0;
    mgr.RegisterAudioRenderer(k1, cb1);
    mgr.UnregisterAudioRenderer(k1);
    auto cb2 = make_shared<TestRendererCallback>();
    uint32_t k2 = 0;
    EXPECT_EQ(mgr.RegisterAudioRenderer(k2, cb2), SUCCESS);
    EXPECT_NE(k2, 0u);
    EXPECT_NE(k2, k1);
    mgr.UnregisterAudioRenderer(k2);
}

// ==================== Renderer Print ====================

/**
 * @tc.name   : AudioDebugManager_PrintRenderer_001
 * @tc.number : AudioDebugManager_PrintRenderer_001
 * @tc.desc   : Test PrintAudioRendererDebugInfo with invalid key.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_PrintRenderer_001, TestSize.Level0)
{
    EXPECT_EQ(AudioDebugManager::GetInstance().PrintAudioRendererDebugInfo(99999, -1), ERR_ILLEGAL_STATE);
}

/**
 * @tc.name   : AudioDebugManager_PrintRenderer_003
 * @tc.number : AudioDebugManager_PrintRenderer_003
 * @tc.desc   : Test PrintAudioRendererDebugInfo with negative fd.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_PrintRenderer_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestRendererCallback>();
    cb->info_ = MakeRendererInfo();
    uint32_t key = 0;
    mgr.RegisterAudioRenderer(key, cb);
    EXPECT_EQ(mgr.PrintAudioRendererDebugInfo(key, -1), SUCCESS);
    mgr.UnregisterAudioRenderer(key);
}

/**
 * @tc.name   : AudioDebugManager_PrintRenderer_004
 * @tc.number : AudioDebugManager_PrintRenderer_004
 * @tc.desc   : Test PrintAudioRendererDebugInfo when callback returns error.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_PrintRenderer_004, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestRendererCallback>();
    cb->ret_ = ERR_ILLEGAL_STATE;
    uint32_t key = 0;
    mgr.RegisterAudioRenderer(key, cb);
    EXPECT_EQ(mgr.PrintAudioRendererDebugInfo(key, -1), ERR_ILLEGAL_STATE);
    mgr.UnregisterAudioRenderer(key);
}

// ==================== PrintAllAudioRenderers ====================

/**
 * @tc.name   : AudioDebugManager_PrintAllRenderer_001
 * @tc.number : AudioDebugManager_PrintAllRenderer_001
 * @tc.desc   : Test PrintAllAudioRenderersDebugInfo with no renderers.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_PrintAllRenderer_001, TestSize.Level0)
{
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    EXPECT_EQ(AudioDebugManager::GetInstance().PrintAllAudioRenderersDebugInfo(fd[1]), SUCCESS);
    close(fd[1]);
    close(fd[0]);
}

/**
 * @tc.name   : AudioDebugManager_PrintAllRenderer_002
 * @tc.number : AudioDebugManager_PrintAllRenderer_002
 * @tc.desc   : Test PrintAllAudioRenderersDebugInfo with multiple callbacks.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_PrintAllRenderer_002, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb1 = make_shared<TestRendererCallback>();
    cb1->info_ = MakeRendererInfo();
    cb1->info_.sessionId = 1001;
    auto cb2 = make_shared<TestRendererCallback>();
    cb2->info_ = MakeRendererInfo();
    cb2->info_.sessionId = 1002;
    uint32_t k1 = 0, k2 = 0;
    mgr.RegisterAudioRenderer(k1, cb1);
    mgr.RegisterAudioRenderer(k2, cb2);

    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    EXPECT_EQ(mgr.PrintAllAudioRenderersDebugInfo(fd[1]), SUCCESS);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("1001"), string::npos);
    EXPECT_NE(out.find("1002"), string::npos);
    mgr.UnregisterAudioRenderer(k1);
    mgr.UnregisterAudioRenderer(k2);
}

/**
 * @tc.name   : AudioDebugManager_PrintAllRenderer_003
 * @tc.number : AudioDebugManager_PrintAllRenderer_003
 * @tc.desc   : Test PrintAllAudioRenderersDebugInfo with partial failure.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_PrintAllRenderer_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto good = make_shared<TestRendererCallback>();
    good->info_ = MakeRendererInfo();
    auto bad = make_shared<TestRendererCallback>();
    bad->ret_ = ERR_ILLEGAL_STATE;
    uint32_t k1 = 0, k2 = 0;
    mgr.RegisterAudioRenderer(k1, good);
    mgr.RegisterAudioRenderer(k2, bad);
    EXPECT_EQ(mgr.PrintAllAudioRenderersDebugInfo(-1), ERR_OPERATION_FAILED);
    mgr.UnregisterAudioRenderer(k1);
    mgr.UnregisterAudioRenderer(k2);
}

// ==================== Renderer Content Verification ====================

/**
 * @tc.name   : AudioDebugManager_RendererContent_001
 * @tc.number : AudioDebugManager_RendererContent_001
 * @tc.desc   : Test renderer output contains focusHistory.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_RendererContent_001, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestRendererCallback>();
    cb->info_ = MakeRendererInfo();
    AudioFocusBehavior behavior;
    behavior.streamId = 500;
    behavior.pid = 100;
    behavior.uid = TEST_UID;
    behavior.streamType = STREAM_VOICE_COMMUNICATION;
    behavior.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    behavior.hintType = INTERRUPT_HINT_PAUSE;
    behavior.timestamp = 1234567890;
    cb->info_.focusHistory.push_back(behavior);
    uint32_t key = 0;
    mgr.RegisterAudioRenderer(key, cb);

    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioRendererDebugInfo(key, fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("focusHistory"), string::npos);
    EXPECT_NE(out.find("INTERRUPT_HINT_PAUSE"), string::npos);
    mgr.UnregisterAudioRenderer(key);
}

/**
 * @tc.name   : AudioDebugManager_RendererContent_002
 * @tc.number : AudioDebugManager_RendererContent_002
 * @tc.desc   : Test renderer output contains errorInfos.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_RendererContent_002, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestRendererCallback>();
    cb->info_ = MakeRendererInfo();
    AudioFaultRecord rec;
    rec.uid = TEST_UID;
    rec.errorCode = 404;
    rec.errorReason = "test_error";
    rec.timestamp = 9876543210;
    cb->info_.errorInfos.push_back(rec);
    uint32_t key = 0;
    mgr.RegisterAudioRenderer(key, cb);

    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioRendererDebugInfo(key, fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("errorInfos"), string::npos);
    EXPECT_NE(out.find("test_error"), string::npos);
    mgr.UnregisterAudioRenderer(key);
}

/**
 * @tc.name   : AudioDebugManager_RendererContent_003
 * @tc.number : AudioDebugManager_RendererContent_003
 * @tc.desc   : Test renderer output contains volume fields.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_RendererContent_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestRendererCallback>();
    cb->info_ = MakeRendererInfo();
    uint32_t key = 0;
    mgr.RegisterAudioRenderer(key, cb);

    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioRendererDebugInfo(key, fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("streamVolume"), string::npos);
    EXPECT_NE(out.find("systemVolume"), string::npos);
    EXPECT_NE(out.find("volume"), string::npos);
    EXPECT_NE(out.find("speed"), string::npos);
    EXPECT_NE(out.find("pitch"), string::npos);
    EXPECT_NE(out.find("effectMode"), string::npos);
    mgr.UnregisterAudioRenderer(key);
}

// ==================== Renderer Enum Edge Cases ====================

/**
 * @tc.name   : AudioDebugManager_RendererEnum_001
 * @tc.number : AudioDebugManager_RendererEnum_001
 * @tc.desc   : Test renderer with unknown DeviceType produces UNKNOWN.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_RendererEnum_001, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestRendererCallback>();
    cb->info_ = MakeRendererInfo();
    cb->info_.mainDeviceType = static_cast<DeviceType>(9999);
    uint32_t key = 0;
    mgr.RegisterAudioRenderer(key, cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioRendererDebugInfo(key, fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("UNKNOWN"), string::npos);
    mgr.UnregisterAudioRenderer(key);
}

/**
 * @tc.name   : AudioDebugManager_RendererEnum_002
 * @tc.number : AudioDebugManager_RendererEnum_002
 * @tc.desc   : Test AudioRouteFlag with FLAG_NONE.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_RendererEnum_002, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestRendererCallback>();
    cb->info_ = MakeRendererInfo();
    cb->info_.routeFlag = AUDIO_FLAG_NONE;
    uint32_t key = 0;
    mgr.RegisterAudioRenderer(key, cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioRendererDebugInfo(key, fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("FLAG_NONE"), string::npos);
    mgr.UnregisterAudioRenderer(key);
}

/**
 * @tc.name   : AudioDebugManager_RendererEnum_003
 * @tc.number : AudioDebugManager_RendererEnum_003
 * @tc.desc   : Test AudioRouteFlag with combined flags.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_RendererEnum_003, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestRendererCallback>();
    cb->info_ = MakeRendererInfo();
    cb->info_.routeFlag = AUDIO_OUTPUT_FLAG_NORMAL | AUDIO_OUTPUT_FLAG_HD | AUDIO_OUTPUT_FLAG_FAST;
    uint32_t key = 0;
    mgr.RegisterAudioRenderer(key, cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioRendererDebugInfo(key, fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("OUTPUT_FLAG_NORMAL"), string::npos);
    EXPECT_NE(out.find("OUTPUT_FLAG_HD"), string::npos);
    EXPECT_NE(out.find("OUTPUT_FLAG_FAST"), string::npos);
    EXPECT_NE(out.find("|"), string::npos);
    mgr.UnregisterAudioRenderer(key);
}

/**
 * @tc.name   : AudioDebugManager_RendererEnum_004
 * @tc.number : AudioDebugManager_RendererEnum_004
 * @tc.desc   : Test VolumeType MEDIA_STREAM for STREAM_MUSIC.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_RendererEnum_004, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestRendererCallback>();
    cb->info_ = MakeRendererInfo();
    cb->info_.volumeType = STREAM_MUSIC;
    uint32_t key = 0;
    mgr.RegisterAudioRenderer(key, cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioRendererDebugInfo(key, fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("MEDIA_STREAM"), string::npos);
    mgr.UnregisterAudioRenderer(key);
}

/**
 * @tc.name   : AudioDebugManager_RendererEnum_005
 * @tc.number : AudioDebugManager_RendererEnum_005
 * @tc.desc   : Test VolumeType non-MUSIC returns stream type name.
 */
HWTEST_F(AudioDebugManagerUnitTest, AudioDebugManager_RendererEnum_005, TestSize.Level1)
{
    auto &mgr = AudioDebugManager::GetInstance();
    auto cb = make_shared<TestRendererCallback>();
    cb->info_ = MakeRendererInfo();
    cb->info_.volumeType = STREAM_VOICE_CALL;
    uint32_t key = 0;
    mgr.RegisterAudioRenderer(key, cb);
    int fd[2];
    ASSERT_TRUE(MakePipe(fd));
    mgr.PrintAudioRendererDebugInfo(key, fd[1]);
    close(fd[1]);
    string out = ReadPipe(fd[0]);
    close(fd[0]);
    EXPECT_NE(out.find("STREAM_VOICE_CALL"), string::npos);
    mgr.UnregisterAudioRenderer(key);
}

} // namespace AudioStandard
} // namespace OHOS
