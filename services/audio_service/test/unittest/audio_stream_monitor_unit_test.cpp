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
    DataTransferMonitorParam para = {0};
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
}
}