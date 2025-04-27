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
#include "none_mix_engine.h"
#include "audio_engine_manager.h"
#include "pro_renderer_stream_impl.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioEngineManagerUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

/**
* @tc.name  : Test AudioEngineManager.
* @tc.type  : FUNC
* @tc.number: AudioEngineManager_001.
* @tc.desc  : Test AddRenderer API.
*/
HWTEST(AudioEngineManagerUnitTest, AudioEngineManager_001, TestSize.Level1)
{
    auto enginManager = std::make_shared<AudioEngineManager>();
    ASSERT_TRUE(enginManager != nullptr);

    AudioProcessConfig audioProcessConfig;
    audioProcessConfig.streamType = STREAM_MUSIC;
    std::shared_ptr<IRendererStream> stream = std::make_shared<ProRendererStreamImpl>(audioProcessConfig, true);
    ASSERT_TRUE(stream != nullptr);

    AudioDeviceDescriptor device;
    enginManager->AddRenderer(stream, device);
}

/**
* @tc.name  : Test AudioEngineManager.
* @tc.type  : FUNC
* @tc.number: AudioEngineManager_002.
* @tc.desc  : Test AddRenderer API.
*/
HWTEST(AudioEngineManagerUnitTest, AudioEngineManager_002, TestSize.Level1)
{
    auto enginManager = std::make_shared<AudioEngineManager>();
    ASSERT_TRUE(enginManager != nullptr);

    AudioProcessConfig audioProcessConfig;
    audioProcessConfig.streamType = STREAM_MUSIC;
    std::shared_ptr<IRendererStream> stream = std::make_shared<ProRendererStreamImpl>(audioProcessConfig, true);
    ASSERT_TRUE(stream != nullptr);
    AudioDeviceDescriptor device;

    std::shared_ptr<AudioPlaybackEngine> playbackEngine = std::make_shared<NoneMixEngine>();
    enginManager->renderEngines_.emplace(PlaybackType::DIRECT, playbackEngine);
    enginManager->AddRenderer(stream, device);
}

/**
* @tc.name  : Test AudioEngineManager.
* @tc.type  : FUNC
* @tc.number: AudioEngineManager_003.
* @tc.desc  : Test AddRenderer API.
*/
HWTEST(AudioEngineManagerUnitTest, AudioEngineManager_003, TestSize.Level1)
{
    auto enginManager = std::make_shared<AudioEngineManager>();
    ASSERT_TRUE(enginManager != nullptr);

    AudioProcessConfig audioProcessConfig;
    audioProcessConfig.streamType = STREAM_RING;
    std::shared_ptr<IRendererStream> stream = std::make_shared<ProRendererStreamImpl>(audioProcessConfig, true);
    ASSERT_TRUE(stream != nullptr);

    AudioDeviceDescriptor device;
    enginManager->AddRenderer(stream, device);
}

/**
* @tc.name  : Test AudioEngineManager.
* @tc.type  : FUNC
* @tc.number: AudioEngineManager_004.
* @tc.desc  : Test RemoveRenderer API.
*/
HWTEST(AudioEngineManagerUnitTest, AudioEngineManager_004, TestSize.Level1)
{
    auto enginManager = std::make_shared<AudioEngineManager>();
    ASSERT_TRUE(enginManager != nullptr);

    AudioProcessConfig audioProcessConfig;
    audioProcessConfig.streamType = STREAM_MUSIC;
    std::shared_ptr<IRendererStream> stream = std::make_shared<ProRendererStreamImpl>(audioProcessConfig, true);
    ASSERT_TRUE(stream != nullptr);

    enginManager->RemoveRenderer(stream);
}

/**
* @tc.name  : Test AudioEngineManager.
* @tc.type  : FUNC
* @tc.number: AudioEngineManager_005.
* @tc.desc  : Test RemoveRenderer API.
*/
HWTEST(AudioEngineManagerUnitTest, AudioEngineManager_005, TestSize.Level1)
{
    auto enginManager = std::make_shared<AudioEngineManager>();
    ASSERT_TRUE(enginManager != nullptr);

    AudioProcessConfig audioProcessConfig;
    audioProcessConfig.streamType = STREAM_MUSIC;
    std::shared_ptr<IRendererStream> stream = std::make_shared<ProRendererStreamImpl>(audioProcessConfig, true);
    ASSERT_TRUE(stream != nullptr);

    std::shared_ptr<AudioPlaybackEngine> playbackEngine = std::make_shared<NoneMixEngine>();
    enginManager->renderEngines_.emplace(PlaybackType::DIRECT, playbackEngine);
    enginManager->RemoveRenderer(stream);
}

/**
* @tc.name  : Test AudioEngineManager.
* @tc.type  : FUNC
* @tc.number: AudioEngineManager_006.
* @tc.desc  : Test RemoveRenderer API.
*/
HWTEST(AudioEngineManagerUnitTest, AudioEngineManager_006, TestSize.Level1)
{
    auto enginManager = std::make_shared<AudioEngineManager>();
    ASSERT_TRUE(enginManager != nullptr);

    AudioProcessConfig audioProcessConfig;
    audioProcessConfig.streamType = STREAM_RING;
    std::shared_ptr<IRendererStream> stream = std::make_shared<ProRendererStreamImpl>(audioProcessConfig, true);
    ASSERT_TRUE(stream != nullptr);

    enginManager->RemoveRenderer(stream);
}
} // namespace AudioStandard
} //