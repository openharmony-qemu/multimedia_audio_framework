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

#include "audio_core_service_entry_unit_test.h"
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

static constexpr int32_t NUM5 = 5;
/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_001
 * @tc.desc  : Test AudioCoreService::EventEntry::UpdateSessionOperation()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_001, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = 0;
    SessionOperation operation = SESSION_OPERATION_START;

    auto ret = eventEntry->UpdateSessionOperation(sessionId, operation);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_002
 * @tc.desc  : Test AudioCoreService::EventEntry::UpdateSessionOperation()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_002, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioPipeInfo> audioPipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->sessionId_ = SESSION_OPERATION_PAUSE;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioPipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_->curPipeList_.push_back(audioPipeInfo);
    EXPECT_NE(&(audioCoreService->audioActiveDevice_), nullptr);

    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = SESSION_OPERATION_PAUSE;
    SessionOperation operation = SESSION_OPERATION_PAUSE;

    auto ret = eventEntry->UpdateSessionOperation(sessionId, operation);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_003
 * @tc.desc  : Test AudioCoreService::EventEntry::UpdateSessionOperation()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_003, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioPipeInfo> audioPipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->sessionId_ = SESSION_OPERATION_STOP;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioPipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_->curPipeList_.push_back(audioPipeInfo);
    EXPECT_NE(&(audioCoreService->audioActiveDevice_), nullptr);

    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = SESSION_OPERATION_STOP;
    SessionOperation operation = SESSION_OPERATION_STOP;

    auto ret = eventEntry->UpdateSessionOperation(sessionId, operation);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_004
 * @tc.desc  : Test AudioCoreService::EventEntry::UpdateSessionOperation()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_004, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    audioCoreService->pipeManager_ = std::make_shared<AudioPipeManager>();
    EXPECT_NE(audioCoreService->pipeManager_, nullptr);
    std::shared_ptr<AudioPipeInfo> audioPipeInfo = std::make_shared<AudioPipeInfo>();
    std::shared_ptr<AudioStreamDescriptor> audioStreamDescriptor = std::make_shared<AudioStreamDescriptor>();
    audioStreamDescriptor->sessionId_ = SESSION_OPERATION_RELEASE;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    audioStreamDescriptor->newDeviceDescs_.push_back(audioDeviceDescriptor);
    audioPipeInfo->streamDescriptors_.push_back(audioStreamDescriptor);
    audioCoreService->pipeManager_->curPipeList_.push_back(audioPipeInfo);
    EXPECT_NE(&(audioCoreService->audioActiveDevice_), nullptr);

    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = SESSION_OPERATION_RELEASE;
    SessionOperation operation = SESSION_OPERATION_RELEASE;

    auto ret = eventEntry->UpdateSessionOperation(sessionId, operation);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_005
 * @tc.desc  : Test AudioCoreService::EventEntry::UpdateSessionOperation()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_005, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    uint32_t sessionId = 0;
    SessionOperation operation = static_cast<SessionOperation>(NUM5);

    auto ret = eventEntry->UpdateSessionOperation(sessionId, operation);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test AudioCoreService.
 * @tc.number: AudioCoreService_006
 * @tc.desc  : Test AudioCoreService::EventEntry::OnServiceConnected()
 */
HWTEST(AudioCoreServiceEntryTest, AudioCoreService_006, TestSize.Level1)
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    EXPECT_NE(audioCoreService, nullptr);
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    EXPECT_NE(eventEntry, nullptr);

    AudioServiceIndex serviceIndex = HDI_SERVICE_INDEX;

    eventEntry->OnServiceConnected(serviceIndex);
}
} // namespace AudioStandard
} // namespace OHOS