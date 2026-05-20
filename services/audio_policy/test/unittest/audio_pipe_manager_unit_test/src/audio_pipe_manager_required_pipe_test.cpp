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

#include "audio_pipe_manager_unit_test.h"

#include <memory>
#include <string>
#include <vector>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
std::shared_ptr<AudioPipeInfo> MakeRequiredPipeInfo(
    const std::string &adapterName, const std::string &pipeName, AudioPipeRole pipeRole)
{
    auto pipeInfo = std::make_shared<AudioPipeInfo>();
    pipeInfo->adapterName_ = adapterName;
    pipeInfo->name_ = pipeName;
    pipeInfo->pipeRole_ = pipeRole;
    pipeInfo->routeFlag_ = pipeRole == PIPE_ROLE_OUTPUT ? AUDIO_OUTPUT_FLAG_NORMAL : AUDIO_INPUT_FLAG_NORMAL;
    return pipeInfo;
}
}

/**
 * @tc.name: AddAudioPipeInfo_RequiredPipeReady_001
 * @tc.desc: Test AddAudioPipeInfo notifies after all required pipes are added.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, AddAudioPipeInfo_RequiredPipeReady_001, TestSize.Level1)
{
    auto audioPipeManager = std::make_shared<AudioPipeManager>();
    int32_t notifyCount = 0;
    audioPipeManager->SetRequiredPipesReadyCallback([&notifyCount]() {
        notifyCount++;
    });
    audioPipeManager->requiredPipesInited_ = true;
    audioPipeManager->requiredPipeKeys_.insert("primary#" + std::to_string(AUDIO_OUTPUT_FLAG_NORMAL));
    audioPipeManager->requiredPipeKeys_.insert("primary#" + std::to_string(AUDIO_INPUT_FLAG_NORMAL));

    audioPipeManager->AddAudioPipeInfo(MakeRequiredPipeInfo("primary", "primary_output", PIPE_ROLE_OUTPUT));
    EXPECT_EQ(notifyCount, 0);
    audioPipeManager->AddAudioPipeInfo(MakeRequiredPipeInfo("primary", "primary_input", PIPE_ROLE_INPUT));
    EXPECT_EQ(notifyCount, 1);
    audioPipeManager->AddAudioPipeInfo(MakeRequiredPipeInfo("primary", "primary_input", PIPE_ROLE_INPUT));
    EXPECT_EQ(notifyCount, 1);
}

/**
 * @tc.name: UpdateRendererPipeInfos_RequiredPipeReady_001
 * @tc.desc: Test UpdateRendererPipeInfos notifies when renderer pipes satisfy required pipes.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, UpdateRendererPipeInfos_RequiredPipeReady_001, TestSize.Level1)
{
    auto audioPipeManager = std::make_shared<AudioPipeManager>();
    int32_t notifyCount = 0;
    audioPipeManager->SetRequiredPipesReadyCallback([&notifyCount]() {
        notifyCount++;
    });
    audioPipeManager->requiredPipesInited_ = true;
    audioPipeManager->requiredPipeKeys_.insert("primary#" + std::to_string(AUDIO_OUTPUT_FLAG_NORMAL));

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = {
        MakeRequiredPipeInfo("primary", "primary_output", PIPE_ROLE_OUTPUT)
    };
    audioPipeManager->UpdateRendererPipeInfos(pipeInfos);
    EXPECT_EQ(notifyCount, 1);
    audioPipeManager->UpdateRendererPipeInfos(pipeInfos);
    EXPECT_EQ(notifyCount, 1);
}

/**
 * @tc.name: UpdateCapturerPipeInfos_RequiredPipeReady_001
 * @tc.desc: Test UpdateCapturerPipeInfos notifies when capturer pipes satisfy required pipes.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, UpdateCapturerPipeInfos_RequiredPipeReady_001, TestSize.Level1)
{
    auto audioPipeManager = std::make_shared<AudioPipeManager>();
    int32_t notifyCount = 0;
    audioPipeManager->SetRequiredPipesReadyCallback([&notifyCount]() {
        notifyCount++;
    });
    audioPipeManager->requiredPipesInited_ = true;
    audioPipeManager->requiredPipeKeys_.insert("primary#" + std::to_string(AUDIO_INPUT_FLAG_NORMAL));

    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = {
        MakeRequiredPipeInfo("primary", "primary_input", PIPE_ROLE_INPUT)
    };
    audioPipeManager->UpdateCapturerPipeInfos(pipeInfos);
    EXPECT_EQ(notifyCount, 1);
    audioPipeManager->UpdateCapturerPipeInfos(pipeInfos);
    EXPECT_EQ(notifyCount, 1);
}

/**
 * @tc.name: UpdateRequiredPipesReadyStateLocked_001
 * @tc.desc: Test required pipe state update returns false before init, after ready, and for non-required pipes.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, UpdateRequiredPipesReadyStateLocked_001, TestSize.Level1)
{
    auto audioPipeManager = std::make_shared<AudioPipeManager>();
    auto requiredPipe = MakeRequiredPipeInfo("primary", "primary_output", PIPE_ROLE_OUTPUT);
    auto normalPipe = MakeRequiredPipeInfo("primary", "primary_input", PIPE_ROLE_INPUT);
    audioPipeManager->requiredPipeKeys_.insert("primary#" + std::to_string(AUDIO_OUTPUT_FLAG_NORMAL));

    EXPECT_FALSE(audioPipeManager->UpdateRequiredPipesReadyStateLocked(requiredPipe));

    audioPipeManager->requiredPipesInited_ = true;
    audioPipeManager->requiredPipesReady_ = true;
    EXPECT_FALSE(audioPipeManager->UpdateRequiredPipesReadyStateLocked(requiredPipe));

    audioPipeManager->requiredPipesReady_ = false;
    EXPECT_FALSE(audioPipeManager->UpdateRequiredPipesReadyStateLocked(normalPipe));
    EXPECT_FALSE(audioPipeManager->UpdateRequiredPipeAddedStateLocked(nullptr));
}

/**
 * @tc.name: UpdateRequiredPipesReadyStateLocked_002
 * @tc.desc: Test vector state update returns false when no required pipe is added.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, UpdateRequiredPipesReadyStateLocked_002, TestSize.Level1)
{
    auto audioPipeManager = std::make_shared<AudioPipeManager>();
    audioPipeManager->requiredPipesInited_ = true;
    audioPipeManager->requiredPipeKeys_.insert("primary#" + std::to_string(AUDIO_OUTPUT_FLAG_NORMAL));
    std::vector<std::shared_ptr<AudioPipeInfo>> pipeInfos = {
        MakeRequiredPipeInfo("primary", "primary_input", PIPE_ROLE_INPUT),
        nullptr
    };

    EXPECT_FALSE(audioPipeManager->UpdateRequiredPipesReadyStateLocked(pipeInfos));
    EXPECT_TRUE(audioPipeManager->addedRequiredPipeKeys_.empty());
}

/**
 * @tc.name: InitRequiredPipes_001
 * @tc.desc: Test InitRequiredPipes notifies immediately when no required pipe is configured.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, InitRequiredPipes_001, TestSize.Level1)
{
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto oldRequiredPipes = configManager.requiredPipes_;
    configManager.requiredPipes_.clear();
    auto audioPipeManager = std::make_shared<AudioPipeManager>();
    int32_t notifyCount = 0;
    audioPipeManager->SetRequiredPipesReadyCallback([&notifyCount]() {
        notifyCount++;
    });

    audioPipeManager->InitRequiredPipes();

    EXPECT_TRUE(audioPipeManager->requiredPipesReady_);
    EXPECT_EQ(notifyCount, 1);
    configManager.requiredPipes_ = oldRequiredPipes;
}

/**
 * @tc.name: InitRequiredPipes_002
 * @tc.desc: Test InitRequiredPipes waits when configured required pipes are not yet added.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, InitRequiredPipes_002, TestSize.Level1)
{
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto oldRequiredPipes = configManager.requiredPipes_;
    configManager.requiredPipes_ = { { "primary", AUDIO_OUTPUT_FLAG_NORMAL } };
    auto audioPipeManager = std::make_shared<AudioPipeManager>();
    int32_t notifyCount = 0;
    audioPipeManager->SetRequiredPipesReadyCallback([&notifyCount]() {
        notifyCount++;
    });

    audioPipeManager->InitRequiredPipes();

    EXPECT_FALSE(audioPipeManager->requiredPipesReady_);
    EXPECT_EQ(notifyCount, 0);
    configManager.requiredPipes_ = oldRequiredPipes;
}

/**
 * @tc.name: NotifyRequiredPipesReady_001
 * @tc.desc: Test NotifyRequiredPipesReady returns when callback is not set.
 * @tc.type: FUNC
 */
HWTEST_F(AudioPipeManagerUnitTest, NotifyRequiredPipesReady_001, TestSize.Level1)
{
    auto audioPipeManager = std::make_shared<AudioPipeManager>();

    EXPECT_NO_THROW(audioPipeManager->NotifyRequiredPipesReady());
}
} // namespace AudioStandard
} // namespace OHOS
