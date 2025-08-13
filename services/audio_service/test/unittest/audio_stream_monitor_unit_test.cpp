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
#include "audio_errors.h"
#include "audio_stream_checker.h"
#include "audio_stream_monitor.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class DataTransferStateChangeCallbackForMonitorTest : public DataTransferStateChangeCallbackForMonitor {
public:
    void OnDataTransferStateChange(const int32_t &pid, const int32_t & callbackId,
        const AudioRendererDataTransferStateChangeInfo& info) override {}
    void OnMuteStateChange(const int32_t &pid, const int32_t &callbackId,
        const int32_t &uid, const uint32_t &sessionId, const bool &isMuted) override;

    int32_t pid_ = 0;
    int32_t callbackId_ = 0;
    int32_t uid_ = 0;
    uint32_t sessionId_ = 0;
    bool isMuted_ = false;
};

void DataTransferStateChangeCallbackForMonitorTest::OnMuteStateChange(const int32_t &pid,
    const int32_t &callbackId, const int32_t &uid, const uint32_t &sessionId, const bool &isMuted)
{
    pid_ = pid;
    callbackId_ = callbackId;
    uid_ = uid;
    sessionId_ = sessionId;
    isMuted_ = isMuted;
}

class AudioStreamMonitorTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioStreamMonitorTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioStreamMonitorTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioStreamMonitorTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioStreamMonitorTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test RegisterAudioRendererDataTransferStateListener API
 * @tc.type  : FUNC
 * @tc.number: RegisterAudioRendererDataTransferStateListener_001
 */
HWTEST(AudioStreamMonitorTest, RegisterAudioRendererDataTransferStateListener_001, TestSize.Level1)
{
    int32_t ret = SUCCESS;
    DataTransferMonitorParam para = {};
    ret = AudioStreamMonitor::GetInstance().RegisterAudioRendererDataTransferStateListener(para, 10000, 10000);
    ret = AudioStreamMonitor::GetInstance().RegisterAudioRendererDataTransferStateListener(para, 10000, 10000);
    ret = AudioStreamMonitor::GetInstance().UnregisterAudioRendererDataTransferStateListener(10000, 10000);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test RegisterAudioRendererDataTransferStateListener API
 * @tc.type  : FUNC
 * @tc.number: RegisterAudioRendererDataTransferStateListener_002
 */
HWTEST(AudioStreamMonitorTest, RegisterAudioRendererDataTransferStateListener_002, TestSize.Level1)
{
    int32_t ret = SUCCESS;
    AudioProcessConfig cfg;
    cfg.originalSessionId = 100001;
    cfg.appInfo.appUid = 20002000;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    AudioStreamMonitor::GetInstance().AddCheckForMonitor(cfg.originalSessionId, checker);
    DataTransferMonitorParam para;
    para.clientUID = 20002000;
    ret = AudioStreamMonitor::GetInstance().RegisterAudioRendererDataTransferStateListener(para, 10000, 10000);
    AudioStreamMonitor::GetInstance().DeleteCheckForMonitor(cfg.originalSessionId);
    ret = AudioStreamMonitor::GetInstance().UnregisterAudioRendererDataTransferStateListener(10000, 10000);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AddCheckForMonitor API
 * @tc.type  : FUNC
 * @tc.number: AddCheckForMonitor_001
 */
HWTEST(AudioStreamMonitorTest, AddCheckForMonitor_001, TestSize.Level1)
{
    int32_t ret = SUCCESS;
    DataTransferMonitorParam para;
    para.clientUID = 20002000;
    ret = AudioStreamMonitor::GetInstance().RegisterAudioRendererDataTransferStateListener(para, 10000, 10000);
    AudioProcessConfig cfg;
    cfg.originalSessionId = 100001;
    cfg.appInfo.appUid = 20002000;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    AudioStreamMonitor::GetInstance().AddCheckForMonitor(cfg.originalSessionId, checker);
    AudioStreamMonitor::GetInstance().DeleteCheckForMonitor(cfg.originalSessionId);
    ret = AudioStreamMonitor::GetInstance().UnregisterAudioRendererDataTransferStateListener(10000, 10000);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test DeleteCheckForMonitor API
 * @tc.type  : FUNC
 * @tc.number: DeleteCheckForMonitor_001
 */
HWTEST(AudioStreamMonitorTest, DeleteCheckForMonitor_001, TestSize.Level1)
{
    AudioStreamMonitor::GetInstance().DeleteCheckForMonitor(100001);
    int32_t size = AudioStreamMonitor::GetInstance().audioStreamCheckers_.size();
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name  : Test OnCallbackAppDied API
 * @tc.type  : FUNC
 * @tc.number: OnCallbackAppDied_001
 */
HWTEST(AudioStreamMonitorTest, OnCallbackAppDied_001, TestSize.Level1)
{
    DataTransferMonitorParam para;
    para.clientUID = 20002000;
    AudioStreamMonitor::GetInstance().RegisterAudioRendererDataTransferStateListener(para, 10000, 10000);
    AudioProcessConfig cfg;
    cfg.originalSessionId = 100001;
    cfg.appInfo.appUid = 20002000;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    AudioStreamMonitor::GetInstance().AddCheckForMonitor(cfg.originalSessionId, checker);
    AudioStreamMonitor::GetInstance().OnCallbackAppDied(10000);
    int size = AudioStreamMonitor::GetInstance().registerInfo_.size();
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name  : Test NotifyAppStateChange API
 * @tc.type  : FUNC
 * @tc.number: NotifyAppStateChange_001
 */
HWTEST(AudioStreamMonitorTest, NotifyAppStateChange_001, TestSize.Level1)
{
    DataTransferMonitorParam para;
    para.clientUID = 20002000;
    AudioStreamMonitor::GetInstance().RegisterAudioRendererDataTransferStateListener(para, 10000, 10000);
    AudioProcessConfig cfg;
    cfg.originalSessionId = 100001;
    cfg.appInfo.appUid = 20002000;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    AudioStreamMonitor::GetInstance().AddCheckForMonitor(cfg.originalSessionId, checker);
    AudioStreamMonitor::GetInstance().NotifyAppStateChange(20002000, true);
    AudioStreamMonitor::GetInstance().NotifyAppStateChange(20002001, false);
    AudioStreamMonitor::GetInstance().DeleteCheckForMonitor(100001);
    int32_t size = AudioStreamMonitor::GetInstance().audioStreamCheckers_.size();
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name  : Test NotifyAppStateChange API
 * @tc.type  : FUNC
 * @tc.number: NotifyAppStateChange
 */
HWTEST(AudioStreamMonitorTest, UnregisterAudioRendedData_002, TestSize.Level1)
{
    int32_t pid = 123;
    int32_t callbackId = 456;

    int32_t result =
        AudioStreamMonitor::GetInstance().UnregisterAudioRendererDataTransferStateListener(pid, callbackId);
    EXPECT_EQ(result, SUCCESS);

    pid = -111;
    callbackId = -111;
    result =
        AudioStreamMonitor::GetInstance().UnregisterAudioRendererDataTransferStateListener(pid, callbackId);
    EXPECT_EQ(result, SUCCESS);
}

/**
 * @tc.name  : Test UpdateMonitorVolume API
 * @tc.type  : FUNC
 * @tc.number: UpdateMonitorVolume_001
 */
HWTEST(AudioStreamMonitorTest, UpdateMonitorVolume_001, TestSize.Level1)
{
    DataTransferMonitorParam para;
    para.clientUID = 20002000;
    AudioStreamMonitor::GetInstance().RegisterAudioRendererDataTransferStateListener(para, 10000, 10000);
    AudioProcessConfig cfg;
    cfg.originalSessionId = 0;
    std::shared_ptr<AudioStreamChecker> checker = std::make_shared<AudioStreamChecker>(cfg);
    AudioStreamMonitor::GetInstance().AddCheckForMonitor(cfg.originalSessionId, checker);
    AudioStreamMonitor::GetInstance().UpdateMonitorVolume(0, 0.5f);
    AudioStreamMonitor::GetInstance().UpdateMonitorVolume(1, 0.5f);
    AudioStreamMonitor::GetInstance().DeleteCheckForMonitor(0);
    int32_t size = AudioStreamMonitor::GetInstance().audioStreamCheckers_.size();
    EXPECT_EQ(size, 0);
}

/**
 * @tc.name  : Test OnMuteStateChange API
 * @tc.type  : FUNC
 * @tc.number: OnMuteStateChange_001
 */
HWTEST(AudioStreamMonitorTest, OnMuteStateChange_001, TestSize.Level1)
{
    std::shared_ptr<AudioStreamMonitor> monitor = std::make_shared<AudioStreamMonitor>();
    DataTransferStateChangeCallbackForMonitorTest *test =
        new DataTransferStateChangeCallbackForMonitorTest();

    int32_t pid = 1;
    int32_t callbackId = 1;
    int32_t uid = 1;
    uint32_t sessionId = 1;
    bool isMuted = true;
    monitor->OnMuteCallback(pid, callbackId, uid, sessionId, isMuted);
    EXPECT_EQ(test->pid_, 0);
    EXPECT_EQ(test->callbackId_, 0);
    EXPECT_EQ(test->uid_, 0);
    EXPECT_EQ(test->sessionId_, 0);
    EXPECT_EQ(test->isMuted_, false);

    monitor->SetAudioServerPtr(test);
    monitor->OnMuteCallback(pid, callbackId, uid, sessionId, isMuted);
    EXPECT_EQ(test->pid_, pid);
    EXPECT_EQ(test->callbackId_, callbackId);
    EXPECT_EQ(test->uid_, uid);
    EXPECT_EQ(test->sessionId_, sessionId);
    EXPECT_EQ(test->isMuted_, isMuted);
    delete test;
}
}
}