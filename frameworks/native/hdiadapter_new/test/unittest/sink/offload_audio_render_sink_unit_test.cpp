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

#include <atomic>
#include <chrono>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "sink/offload_audio_render_sink.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class OffloadAudioRenderSinkUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp();
    virtual void TearDown();

protected:
    static uint32_t id_;
    static std::shared_ptr<IAudioRenderSink> sink_;
    static IAudioSinkAttr attr_;
};

uint32_t OffloadAudioRenderSinkUnitTest::id_ = HDI_INVALID_ID;
std::shared_ptr<IAudioRenderSink> OffloadAudioRenderSinkUnitTest::sink_ = nullptr;
IAudioSinkAttr OffloadAudioRenderSinkUnitTest::attr_ = {};

void OffloadAudioRenderSinkUnitTest::SetUpTestCase()
{
    id_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_OFFLOAD, HDI_ID_INFO_DEFAULT, true);
}

void OffloadAudioRenderSinkUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(id_);
}

void OffloadAudioRenderSinkUnitTest::SetUp()
{
    sink_ = HdiAdapterManager::GetInstance().GetRenderSink(id_, true);
    if (sink_ == nullptr) {
        return;
    }
}

void OffloadAudioRenderSinkUnitTest::TearDown()
{
    sink_ = nullptr;
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_001
 * @tc.desc   : Test offload sink create
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_001, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_002
 * @tc.desc   : Test offload sink deinit
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_002, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    if (sink_->IsInited()) {
        sink_->DeInit();
    }
    EXPECT_FALSE(sink_->IsInited());
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_003
 * @tc.desc   : Test offload sink start, stop, resume, pause, flush, reset
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_003, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    int32_t ret = sink_->Start();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Resume();
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
    ret = sink_->Pause();
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
    ret = sink_->Flush();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = sink_->Reset();
    EXPECT_EQ(ret, ERR_OPERATION_FAILED);
    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_004
 * @tc.desc   : Test offload sink set/get volume
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_004, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    int32_t ret = sink_->SetVolume(0.0f, 0.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->SetVolume(0.0f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->SetVolume(1.0f, 0.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->SetVolume(1.0f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    float left;
    float right;
    ret = sink_->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_005
 * @tc.desc   : Test offload sink set audio scene
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_005, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    int32_t ret = sink_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_006
 * @tc.desc   : Test offload sink update active device
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_006, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    std::vector<DeviceType> deviceTypes = { DEVICE_TYPE_SPEAKER };
    int32_t ret = sink_->UpdateActiveDevice(deviceTypes);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_007
 * @tc.desc   : Test offload sink set speed
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_007, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    sink_->SetSpeed(0.25f); // test cover
    sink_->SetSpeed(0.5f);
    sink_->SetSpeed(1.0f);
    sink_->SetSpeed(3.0f);
    sink_->SetSpeed(3.25f);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_008
 * @tc.desc   : Test GetCurrentOutputDevice returns current active device
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_008, TestSize.Level1)
{
    auto offloadAudioRenderSink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(offloadAudioRenderSink, nullptr);

    offloadAudioRenderSink->currentActiveDevice_ = DEVICE_TYPE_SPEAKER;
    DeviceType device = offloadAudioRenderSink->GetCurrentOutputDevice();
    EXPECT_EQ(device, DEVICE_TYPE_SPEAKER);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_009
 * @tc.desc   : Test GetCurrentOutputDevice returns default device when not set
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_009, TestSize.Level1)
{
    auto offloadAudioRenderSink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(offloadAudioRenderSink, nullptr);

    offloadAudioRenderSink->currentActiveDevice_ = DEVICE_TYPE_NONE;
    DeviceType device = offloadAudioRenderSink->GetCurrentOutputDevice();
    EXPECT_EQ(device, DEVICE_TYPE_NONE);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_010
 * @tc.desc   : Test GetCurrentOutputDevice returns bluetooth a2dp device
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_010, TestSize.Level1)
{
    auto offloadAudioRenderSink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(offloadAudioRenderSink, nullptr);

    offloadAudioRenderSink->currentActiveDevice_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    DeviceType device = offloadAudioRenderSink->GetCurrentOutputDevice();
    EXPECT_EQ(device, DEVICE_TYPE_BLUETOOTH_A2DP);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_011
 * @tc.desc   : Test GetCurrentOutputDevice returns wired headset device
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_011, TestSize.Level1)
{
    auto offloadAudioRenderSink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(offloadAudioRenderSink, nullptr);

    offloadAudioRenderSink->currentActiveDevice_ = DEVICE_TYPE_WIRED_HEADSET;
    DeviceType device = offloadAudioRenderSink->GetCurrentOutputDevice();
    EXPECT_EQ(device, DEVICE_TYPE_WIRED_HEADSET);
}

// ---- control-plane actor state machine tests (fake HDI render + call recorder) ----
// Sinks below are manually constructed without Init(), so ctrlRunning_ is false and
// every public control API runs HandleCtrlCmd inline on the test thread: fully
// deterministic, no ctrl thread involved. Group E starts the real ctrl thread.
using OffloadCtrlState = OffloadAudioRenderSink::CtrlState;
using OffloadCtrlCmd = OffloadAudioRenderSink::CtrlCmd;
using OffloadPending = OffloadAudioRenderSink::PendingAfterFlush;

static constexpr int32_t CTRL_POLL_STEP_MS = 10;
static constexpr int32_t CTRL_TEST_WAIT_MS = 2000;
static constexpr int32_t CTRL_TEST_REPLY_SHORT_MS = 100;
static constexpr int32_t CTRL_TEST_REPLY_MS = 1000;
static constexpr uint32_t FAKE_FRAME_BYTES = 4;
static constexpr uint32_t FAKE_BUFFER_SIZE_MS = 100;

static std::mutex g_hdiCallMtx;
static std::vector<std::string> g_hdiCalls;
static std::atomic<bool> g_drainShouldBlock{false};
static std::atomic<bool> g_drainEntered{false};
static std::atomic<bool> g_drainExited{false};
static std::atomic<bool> g_drainRelease{false};
static std::atomic<int32_t> g_hdiStartRet{SUCCESS};
static std::atomic<int32_t> g_hdiStopRet{SUCCESS};
static std::atomic<int32_t> g_hdiFlushRet{SUCCESS};
static std::atomic<int32_t> g_hdiDrainRet{SUCCESS};
static struct IAudioRender g_fakeRender = {}; // zero-init: untouched members stay null

static void RecordHdiCall(const std::string &name)
{
    std::lock_guard<std::mutex> lock(g_hdiCallMtx);
    g_hdiCalls.push_back(name);
}

static int32_t FakeStart(struct IAudioRender *self)
{
    (void)self;
    RecordHdiCall("Start");
    return g_hdiStartRet.load();
}

static int32_t FakeStop(struct IAudioRender *self)
{
    (void)self;
    RecordHdiCall("Stop");
    return g_hdiStopRet.load();
}

static int32_t FakeFlush(struct IAudioRender *self)
{
    (void)self;
    RecordHdiCall("Flush");
    return g_hdiFlushRet.load();
}

static int32_t FakeDrainBuffer(struct IAudioRender *self, AudioDrainNotifyType *type)
{
    (void)self;
    (void)type;
    RecordHdiCall("Drain");
    g_drainEntered.store(true);
    while (g_drainShouldBlock.load() && !g_drainRelease.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(CTRL_POLL_STEP_MS));
    }
    g_drainExited.store(true);
    return g_hdiDrainRet.load();
}

static void ResetHdiRecorder()
{
    std::lock_guard<std::mutex> lock(g_hdiCallMtx);
    g_hdiCalls.clear();
    g_drainShouldBlock.store(false);
    g_drainEntered.store(false);
    g_drainExited.store(false);
    g_drainRelease.store(false);
    g_hdiStartRet.store(SUCCESS);
    g_hdiStopRet.store(SUCCESS);
    g_hdiFlushRet.store(SUCCESS);
    g_hdiDrainRet.store(SUCCESS);
}

static void InstallFakeHdi()
{
    ResetHdiRecorder();
    g_fakeRender.Start = FakeStart;
    g_fakeRender.Stop = FakeStop;
    g_fakeRender.Flush = FakeFlush;
    g_fakeRender.DrainBuffer = FakeDrainBuffer;
}

static std::vector<std::string> HdiCalls()
{
    std::lock_guard<std::mutex> lock(g_hdiCallMtx);
    return g_hdiCalls;
}

static size_t CountHdiCall(const std::string &name)
{
    std::lock_guard<std::mutex> lock(g_hdiCallMtx);
    size_t count = 0;
    for (auto &call : g_hdiCalls) {
        if (call == name) {
            count++;
        }
    }
    return count;
}

static void ExpectHdiSequence(const std::vector<std::string> &expected)
{
    EXPECT_EQ(HdiCalls(), expected);
}

static bool WaitForFlag(std::atomic<bool> &flag, int32_t timeoutMs)
{
    for (int32_t waitedMs = 0; waitedMs < timeoutMs; waitedMs += CTRL_POLL_STEP_MS) {
        if (flag.load()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(CTRL_POLL_STEP_MS));
    }
    return flag.load();
}

static bool WaitForHdiCall(const std::string &name, int32_t timeoutMs)
{
    for (int32_t waitedMs = 0; waitedMs < timeoutMs; waitedMs += CTRL_POLL_STEP_MS) {
        if (CountHdiCall(name) > 0) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(CTRL_POLL_STEP_MS));
    }
    return CountHdiCall(name) > 0;
}

static bool WaitForCtrlState(const std::shared_ptr<OffloadAudioRenderSink> &sink,
    OffloadCtrlState state, int32_t timeoutMs)
{
    for (int32_t waitedMs = 0; waitedMs < timeoutMs; waitedMs += CTRL_POLL_STEP_MS) {
        if (sink->ctrlState_.load() == state) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(CTRL_POLL_STEP_MS));
    }
    return sink->ctrlState_.load() == state;
}

static bool WaitForWeakPtrExpired(const std::weak_ptr<OffloadAudioRenderSink> &weakSink,
    int32_t timeoutMs)
{
    for (int32_t waitedMs = 0; waitedMs < timeoutMs; waitedMs += CTRL_POLL_STEP_MS) {
        if (weakSink.expired()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(CTRL_POLL_STEP_MS));
    }
    return weakSink.expired();
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_012
 * @tc.desc   : Test ctrl state machine IDLE plus Start success transition
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_012, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;

    EXPECT_EQ(sink->Start(), SUCCESS);
    EXPECT_TRUE(sink->started_.load());
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_STARTED);
    ExpectHdiSequence({ "Start" });
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_013
 * @tc.desc   : Test ctrl state machine IDLE plus Start with HDI Start failure
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_013, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    g_hdiStartRet.store(ERR_OPERATION_FAILED);

    EXPECT_EQ(sink->Start(), ERR_NOT_STARTED);
    EXPECT_FALSE(sink->started_.load());
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_IDLE);
    ExpectHdiSequence({ "Start" });
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_014
 * @tc.desc   : Test ctrl state machine STARTED plus Start is idempotent
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_014, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->started_.store(true);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_STARTED);

    EXPECT_EQ(sink->Start(), SUCCESS);
    EXPECT_TRUE(sink->started_.load());
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_STARTED);
    ExpectHdiSequence({}); // already started: zero HDI calls
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_015
 * @tc.desc   : Test ctrl state machine STARTED plus Stop flushes first then stops
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_015, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->started_.store(true);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_STARTED);

    EXPECT_EQ(sink->Stop(), SUCCESS);
    EXPECT_FALSE(sink->started_.load());
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_IDLE);
    ExpectHdiSequence({ "Flush", "Stop" }); // legacy stop flushes first
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_016
 * @tc.desc   : Test ctrl state machine STARTED plus Stop with HDI Stop failure
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_016, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->started_.store(true);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_STARTED);
    g_hdiStopRet.store(ERR_OPERATION_FAILED);

    EXPECT_EQ(sink->Stop(), ERR_NOT_STARTED);
    EXPECT_TRUE(sink->started_.load()); // legacy contract: started_ stays true on failure
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_STARTED);
    ExpectHdiSequence({ "Flush", "Stop" });
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_017
 * @tc.desc   : Test ctrl state machine Stop without started touches no HDI
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_017, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;

    EXPECT_EQ(sink->Stop(), SUCCESS);
    EXPECT_FALSE(sink->started_.load());
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_IDLE);
    ExpectHdiSequence({}); // not started: zero HDI calls
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_018
 * @tc.desc   : Test ctrl state machine Stop with started flag but null render
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_018, TestSize.Level1)
{
    ResetHdiRecorder();
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    sink->audioRender_ = nullptr;
    sink->started_.store(true);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_STARTED);

    EXPECT_EQ(sink->Stop(), ERR_INVALID_HANDLE);
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_STARTED); // legacy contract
    EXPECT_TRUE(sink->started_.load());
    ExpectHdiSequence({});
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_019
 * @tc.desc   : Test ctrl state machine STARTED plus Flush keeps sink started
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_019, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->started_.store(true);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_STARTED);

    EXPECT_EQ(sink->Flush(), SUCCESS);
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_STARTED);
    EXPECT_TRUE(sink->started_.load());
    ExpectHdiSequence({ "Flush" });
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_020
 * @tc.desc   : Test ctrl state machine Flush in IDLE is rejected
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_020, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;

    EXPECT_EQ(sink->Flush(), ERR_OPERATION_FAILED);
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_IDLE);
    ExpectHdiSequence({}); // zero HDI calls
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_021
 * @tc.desc   : Test ctrl state machine STARTED plus Reset flushes and reconciles to STARTED
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_021, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->started_.store(true);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_STARTED);

    EXPECT_EQ(sink->Reset(), SUCCESS);
    // the forced restart short-circuits in ExecStart: started_ survives the flush,
    // so the HDI-level restart is a reconciliation, not a second HDI Start
    ExpectHdiSequence({ "Flush" });
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_STARTED);
    EXPECT_TRUE(sink->started_.load());
    EXPECT_EQ(sink->pendingAfterFlush_, OffloadPending::PENDING_NONE);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_022
 * @tc.desc   : Test ctrl state machine Reset in IDLE is rejected
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_022, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;

    EXPECT_EQ(sink->Reset(), ERR_OPERATION_FAILED);
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_IDLE);
    ExpectHdiSequence({}); // zero HDI calls
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_023
 * @tc.desc   : Test Reset reports success even when the restart itself fails
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_023, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    // white-box seed: flush carrier in flight with started_ already cleared, so the
    // forced restart really reaches the HDI Start call instead of short-circuiting
    sink->started_.store(false);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_FLUSHING);
    g_hdiStartRet.store(ERR_OPERATION_FAILED);

    EXPECT_EQ(sink->Reset(), SUCCESS); // carrier completion is success by design
    ExpectHdiSequence({ "Flush", "Start" });
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_IDLE);
    EXPECT_FALSE(sink->started_.load());
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_024
 * @tc.desc   : Test Start during FLUSHING is parked as pending and rides the flush carrier
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_024, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->started_.store(true);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_FLUSHING); // flush carrier in flight

    EXPECT_EQ(sink->Start(), SUCCESS); // parked as pending, returns immediately
    ExpectHdiSequence({}); // zero HDI calls while the gate is closed
    EXPECT_EQ(sink->pendingAfterFlush_, OffloadPending::PENDING_START);
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_FLUSHING);

    // land the flush carrier
    EXPECT_EQ(sink->HandleCtrlCmd(OffloadCtrlCmd::CMD_FLUSH, 0), SUCCESS);
    ExpectHdiSequence({ "Flush" }); // deferred start reconciles; no second HDI Start
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_STARTED);
    EXPECT_TRUE(sink->started_.load());
    EXPECT_EQ(sink->pendingAfterFlush_, OffloadPending::PENDING_NONE);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_025
 * @tc.desc   : Test Stop during FLUSHING rides the carrier and flushes exactly once
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_025, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->started_.store(true);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_FLUSHING); // flush carrier in flight

    EXPECT_EQ(sink->Stop(), SUCCESS); // parked as pending, returns immediately
    ExpectHdiSequence({}); // zero HDI calls while the gate is closed
    EXPECT_EQ(sink->pendingAfterFlush_, OffloadPending::PENDING_STOP);
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_FLUSHING);

    // land the flush carrier: ExecStop(false) must not flush a second time
    EXPECT_EQ(sink->HandleCtrlCmd(OffloadCtrlCmd::CMD_FLUSH, 0), SUCCESS);
    ExpectHdiSequence({ "Flush", "Stop" });
    EXPECT_EQ(CountHdiCall("Flush"), 1u);
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_IDLE);
    EXPECT_FALSE(sink->started_.load());
    EXPECT_EQ(sink->pendingAfterFlush_, OffloadPending::PENDING_NONE);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_026
 * @tc.desc   : Test pending last-writer-wins from parked Start to parked Stop
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_026, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->started_.store(true);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_FLUSHING); // flush carrier in flight

    EXPECT_EQ(sink->Start(), SUCCESS); // parked first
    EXPECT_EQ(sink->Stop(), SUCCESS); // last writer wins
    EXPECT_EQ(sink->pendingAfterFlush_, OffloadPending::PENDING_STOP);

    EXPECT_EQ(sink->HandleCtrlCmd(OffloadCtrlCmd::CMD_FLUSH, 0), SUCCESS);
    ExpectHdiSequence({ "Flush", "Stop" });
    EXPECT_EQ(CountHdiCall("Start"), 0u); // the parked Start never happened
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_IDLE);
    EXPECT_FALSE(sink->started_.load());
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_027
 * @tc.desc   : Test pending last-writer-wins from parked Stop to parked Start
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_027, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    // white-box seed: started_ cleared so the deferred start genuinely reaches HDI
    sink->started_.store(false);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_FLUSHING);

    EXPECT_EQ(sink->Stop(), SUCCESS); // parked first
    EXPECT_EQ(sink->Start(), SUCCESS); // last writer wins
    EXPECT_EQ(sink->pendingAfterFlush_, OffloadPending::PENDING_START);

    EXPECT_EQ(sink->HandleCtrlCmd(OffloadCtrlCmd::CMD_FLUSH, 0), SUCCESS);
    ExpectHdiSequence({ "Flush", "Start" });
    EXPECT_EQ(CountHdiCall("Stop"), 0u); // the parked Stop never happened
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_STARTED);
    EXPECT_TRUE(sink->started_.load());
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_028
 * @tc.desc   : Test Reset overrides a parked pending Stop
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_028, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->started_.store(true);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_FLUSHING); // flush carrier in flight

    EXPECT_EQ(sink->Stop(), SUCCESS); // parks PENDING_STOP
    EXPECT_EQ(sink->Reset(), SUCCESS); // reset is never absorbed into the pending slot

    // the forced restart wins over the parked stop: no HDI Stop is ever issued and
    // the reconciliation keeps the sink started (restart short-circuits in ExecStart)
    ExpectHdiSequence({ "Flush" });
    EXPECT_EQ(CountHdiCall("Stop"), 0u);
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_STARTED);
    EXPECT_TRUE(sink->started_.load());
    EXPECT_EQ(sink->pendingAfterFlush_, OffloadPending::PENDING_NONE);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_029
 * @tc.desc   : Test repeated flush carriers are executed without absorption
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_029, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->started_.store(true);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_FLUSHING); // flush carrier in flight

    EXPECT_EQ(sink->HandleCtrlCmd(OffloadCtrlCmd::CMD_FLUSH, 0), SUCCESS);
    EXPECT_EQ(sink->HandleCtrlCmd(OffloadCtrlCmd::CMD_FLUSH, 0), SUCCESS);
    ExpectHdiSequence({ "Flush", "Flush" }); // both carriers run, no merging
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_STARTED);
    EXPECT_TRUE(sink->started_.load());
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_030
 * @tc.desc   : Test write gate rejects RenderFrame and GetPresentationPosition while flushing
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_030, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->started_.store(true);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_FLUSHING);

    char data = 0;
    uint64_t writeLen = 0;
    uint64_t frames = 0;
    int64_t timeSec = 0;
    int64_t timeNanoSec = 0;
    EXPECT_EQ(sink->RenderFrame(data, FAKE_FRAME_BYTES, writeLen), ERR_OPERATION_FAILED);
    EXPECT_EQ(sink->GetPresentationPosition(frames, timeSec, timeNanoSec), ERR_OPERATION_FAILED);
    ExpectHdiSequence({}); // the gate rejects before any HDI call

    sink->ctrlState_.store(OffloadCtrlState::CTRL_IDLE);
    EXPECT_EQ(sink->RenderFrame(data, FAKE_FRAME_BYTES, writeLen), ERR_OPERATION_FAILED); // not-start branch
    ExpectHdiSequence({});
    // note: GetPresentationPosition is not probed in IDLE here, its gate only
    // rejects FLUSHING and it would forward to the null GetRenderPosition member
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_031
 * @tc.desc   : Test RenderFrame with null render handle
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_031, TestSize.Level1)
{
    ResetHdiRecorder();
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    sink->audioRender_ = nullptr;

    char data = 0;
    uint64_t writeLen = 0;
    EXPECT_EQ(sink->RenderFrame(data, FAKE_FRAME_BYTES, writeLen), ERR_INVALID_HANDLE);
    ExpectHdiSequence({});
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_032
 * @tc.desc   : Test Flush closes the write gate before it returns on the ctrl thread
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_032, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->StartCtrlThread();
    EXPECT_EQ(sink->Start(), SUCCESS); // real start through the ctrl thread

    g_drainShouldBlock.store(true);
    EXPECT_EQ(sink->PostCtrlCmd(OffloadCtrlCmd::CMD_DRAIN, 0, 0), SUCCESS); // fire and forget
    EXPECT_TRUE(WaitForFlag(g_drainEntered, CTRL_TEST_WAIT_MS));

    // the drain is still blocking the ctrl thread, so the flush command is queued
    // behind it: at this instant the pre-closed gate is the only state change
    EXPECT_EQ(sink->Flush(), SUCCESS);
    EXPECT_EQ(sink->ctrlState_.load(), OffloadCtrlState::CTRL_FLUSHING);

    g_drainRelease.store(true);
    EXPECT_TRUE(WaitForCtrlState(sink, OffloadCtrlState::CTRL_STARTED, CTRL_TEST_WAIT_MS));
    EXPECT_EQ(CountHdiCall("Flush"), 1u); // queued flush ran exactly once
    sink->DeInit();
    g_drainShouldBlock.store(false);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_033
 * @tc.desc   : Test PostCtrlCmd timeout degrades the caller but keeps the queue alive
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_033, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->StartCtrlThread();
    g_drainShouldBlock.store(true);

    // the drain blocks the only executor, so the bounded wait gives up
    EXPECT_EQ(sink->PostCtrlCmd(OffloadCtrlCmd::CMD_DRAIN, 0, CTRL_TEST_REPLY_SHORT_MS), ERR_OPERATION_FAILED);

    g_drainRelease.store(true);
    EXPECT_TRUE(WaitForHdiCall("Drain", CTRL_TEST_WAIT_MS)); // command still ran

    // the queue is still served: the next command gets its reply
    EXPECT_EQ(sink->PostCtrlCmd(OffloadCtrlCmd::CMD_STOP, 0, CTRL_TEST_REPLY_MS), SUCCESS);
    sink->DeInit();
    g_drainShouldBlock.store(false);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_034
 * @tc.desc   : Test DeInit shutdown handshake joins the ctrl thread cleanly
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_034, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->StartCtrlThread();
    EXPECT_EQ(sink->PostCtrlCmd(OffloadCtrlCmd::CMD_STOP, 0, CTRL_TEST_REPLY_MS), SUCCESS); // not started

    sink->DeInit();
    EXPECT_FALSE(sink->ctrlRunning_.load());
    EXPECT_FALSE(sink->ctrlThread_.joinable());

    // after DeInit commands run inline again, and the teardown cleared the render
    EXPECT_EQ(sink->Start(), ERR_INVALID_HANDLE);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_035
 * @tc.desc   : Test DeInit abandons a stuck ctrl thread and rejects later commands
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_035, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->StartCtrlThread();
    g_drainShouldBlock.store(true);
    EXPECT_EQ(sink->PostCtrlCmd(OffloadCtrlCmd::CMD_DRAIN, 0, 0), SUCCESS); // fire and forget
    EXPECT_TRUE(WaitForFlag(g_drainEntered, CTRL_TEST_WAIT_MS));

    sink->DeInit(); // bounded shutdown wait expires, executor is abandoned
    EXPECT_TRUE(sink->ctrlAbandoned_.load());
    EXPECT_FALSE(sink->ctrlRunning_.load());
    EXPECT_FALSE(sink->ctrlThread_.joinable()); // detached

    // the abandoned plane rejects every later command
    EXPECT_EQ(sink->PostCtrlCmd(OffloadCtrlCmd::CMD_STOP, 0, CTRL_TEST_REPLY_MS), ERR_ILLEGAL_STATE);

    // release the stub: the zombie then consumes the CMD_SHUTDOWN DeInit queued before
    // the abandon, exits CtrlThreadMain and drops its self-ref; the expired weak_ptr
    // proves the exit deterministically (no heuristic wind-down sleep needed)
    g_drainRelease.store(true);
    EXPECT_TRUE(WaitForFlag(g_drainExited, CTRL_TEST_WAIT_MS));
    std::weak_ptr<OffloadAudioRenderSink> weakSink = sink;
    sink = nullptr;
    EXPECT_TRUE(WaitForWeakPtrExpired(weakSink, CTRL_TEST_WAIT_MS));
    g_drainShouldBlock.store(false);
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_036
 * @tc.desc   : Test double StartCtrlThread skips the restart instead of terminating
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_036, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->StartCtrlThread();
    sink->StartCtrlThread(); // second call only warns and skips

    // passing here means no terminate() happened and the single executor still works
    EXPECT_EQ(sink->PostCtrlCmd(OffloadCtrlCmd::CMD_STOP, 0, CTRL_TEST_REPLY_MS), SUCCESS);
    sink->DeInit();
    EXPECT_FALSE(sink->ctrlRunning_.load());
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_037
 * @tc.desc   : Test Drain with null render handle
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_037, TestSize.Level1)
{
    ResetHdiRecorder();
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    sink->audioRender_ = nullptr;

    EXPECT_EQ(sink->Drain(AUDIO_DRAIN_ALL), ERR_INVALID_HANDLE);
    ExpectHdiSequence({});
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_038
 * @tc.desc   : Test Drain with HDI DrainBuffer failure
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_038, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    g_hdiDrainRet.store(ERR_OPERATION_FAILED);

    EXPECT_EQ(sink->Drain(AUDIO_DRAIN_ALL), ERR_OPERATION_FAILED);
    ExpectHdiSequence({ "Drain" });
}

/**
 * @tc.name   : Test OffloadSink API
 * @tc.number : OffloadSinkUnitTest_039
 * @tc.desc   : Test write gate rejects SetVolume and SetBufferSize while flushing
 */
HWTEST_F(OffloadAudioRenderSinkUnitTest, OffloadSinkUnitTest_039, TestSize.Level1)
{
    std::shared_ptr<OffloadAudioRenderSink> sink = std::make_shared<OffloadAudioRenderSink>();
    ASSERT_NE(sink, nullptr);
    InstallFakeHdi();
    sink->audioRender_ = &g_fakeRender;
    sink->started_.store(true);
    sink->ctrlState_.store(OffloadCtrlState::CTRL_FLUSHING);

    // part of the lock-free write gate contract: control-adjacent setters are also
    // rejected while a flush carrier is in flight
    EXPECT_EQ(sink->SetVolume(0.5f, 0.5f), ERR_OPERATION_FAILED);
    EXPECT_EQ(sink->SetBufferSize(FAKE_BUFFER_SIZE_MS), ERR_OPERATION_FAILED);
    ExpectHdiSequence({});
}
} // namespace AudioStandard
} // namespace OHOS
