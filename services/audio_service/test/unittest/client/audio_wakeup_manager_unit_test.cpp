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
#include "audio_log.h"
#include "audio_wakeup_client_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
class AudioWakeupClientManagerStateGuard {
public:
    explicit AudioWakeupClientManagerStateGuard(AudioWakeupClientManager &manager)
        : wakeUpManager_(manager),
          audioCapturerSourceCallbackBackup_(manager.audioCapturerSourceCallback_),
          audioWakeUpSourceCloseCallbackBackup_(manager.audioWakeUpSourceCloseCallback_),
          remoteWakeUpCallbackBackup_(manager.remoteWakeUpCallback_) {}

    ~AudioWakeupClientManagerStateGuard()
    {
        wakeUpManager_.audioCapturerSourceCallback_ = audioCapturerSourceCallbackBackup_;
        wakeUpManager_.audioWakeUpSourceCloseCallback_ = audioWakeUpSourceCloseCallbackBackup_;
        wakeUpManager_.remoteWakeUpCallback_ = remoteWakeUpCallbackBackup_;
    }

private:
    AudioWakeupClientManager &wakeUpManager_;
    std::shared_ptr<AudioCapturerSourceCallback> audioCapturerSourceCallbackBackup_;
    std::shared_ptr<WakeUpSourceCloseCallback> audioWakeUpSourceCloseCallbackBackup_;
    std::shared_ptr<WakeUpSourceCallback> remoteWakeUpCallbackBackup_;
};

class DummyCapturerCallback : public AudioCapturerSourceCallback {
public:
    void OnCapturerState(bool isActive) override
    {
        called = true;
        activeFlag = isActive;
    }
    bool called = false;
    bool activeFlag = false;
};

class DummyWakeupCloseCallback : public WakeUpSourceCloseCallback {
public:
    void OnWakeupClose() override
    {
        called = true;
    }
    bool called = false;
};
} // namespace

class AudioWakeupClientManagerUnitTest : public testing::Test {
public:
    void SetUp() override
    {
        manager_ = &AudioWakeupClientManager::GetInstance();
        guard_ = std::make_unique<AudioWakeupClientManagerStateGuard>(*manager_);
        manager_->audioCapturerSourceCallback_ = nullptr;
        manager_->audioWakeUpSourceCloseCallback_ = nullptr;
        manager_->remoteWakeUpCallback_ = nullptr;
    }

    void TearDown() override
    {
        guard_.reset();
    }

protected:
    AudioWakeupClientManager *manager_ = nullptr;
    std::unique_ptr<AudioWakeupClientManagerStateGuard> guard_;
};

/**
 * @tc.name   : OnCapturerState_WithCallback
 * @tc.number : AudioWakeupClientManager_OnCapturerState_001
 * @tc.desc   : Verify OnCapturerState forwards active flag when callback exists.
 */
HWTEST_F(AudioWakeupClientManagerUnitTest, AudioWakeupClientManager_OnCapturerState_001, TestSize.Level1)
{
    auto callback = std::make_shared<DummyCapturerCallback>();
    manager_->OnCapturerState(true);
    EXPECT_FALSE(callback->called);

    manager_->audioCapturerSourceCallback_ = callback;
    manager_->OnCapturerState(true);

    EXPECT_TRUE(callback->called);
    EXPECT_TRUE(callback->activeFlag);
}

/**
 * @tc.name   : OnWakeupClose_WithCallback
 * @tc.number : AudioWakeupClientManager_OnWakeupClose_001
 * @tc.desc   : Verify OnWakeupClose notifies close callback.
 */
HWTEST_F(AudioWakeupClientManagerUnitTest, AudioWakeupClientManager_OnWakeupClose_001, TestSize.Level1)
{
    auto callback = std::make_shared<DummyWakeupCloseCallback>();
    manager_->OnWakeupClose();
    EXPECT_FALSE(callback->called);

    manager_->audioWakeUpSourceCloseCallback_ = callback;
    manager_->OnWakeupClose();

    EXPECT_TRUE(callback->called);
}

/**
 * @tc.name  : Test RegisterWakeupSourceCallback API
 * @tc.type  : FUNC
 * @tc.number: RegisterWakeupSourceCallback_001
 * @tc.desc  : Test RegisterWakeupSourceCallback interface.
 */
HWTEST_F(AudioWakeupClientManagerUnitTest, RegisterWakeupSourceCallback_001, TestSize.Level1)
{
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest RegisterWakeupSourceCallback_001 start");
    int32_t result = AudioWakeupClientManager::GetInstance().RegisterWakeupSourceCallback();
    AUDIO_INFO_LOG("AudioSystemManagerUnitTest ->RegisterWakeupSourceCallback_001() result:%{public}d", result);
    EXPECT_NE(result, ERROR);
}
} // namespace AudioStandard
} // namespace OHOS
