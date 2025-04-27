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
#include "audio_renderer_private.h"
#include "audio_renderer_proxy_obj.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class AudioRendererProxyObjUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioRendererProxyObjUnitTest::SetUpTestCase(void) {}
void AudioRendererProxyObjUnitTest::TearDownTestCase(void) {}
void AudioRendererProxyObjUnitTest::SetUp(void) {}
void AudioRendererProxyObjUnitTest::TearDown(void) {}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_001
 * @tc.desc  : Test MuteStreamImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_001, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    AppInfo appInfo = {};
    std::shared_ptr<AudioRenderer> rendererOj = std::make_shared<AudioRendererPrivate>(
        AudioStreamType::STREAM_GAME, appInfo, true);
    ASSERT_TRUE(rendererOj != nullptr);

    proxyObj->SaveRendererObj(rendererOj);
    StreamSetStateEventInternal streamSetStateEventInternal;
    proxyObj->MuteStreamImpl(streamSetStateEventInternal);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_002
 * @tc.desc  : Test MuteStreamImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_002, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    std::shared_ptr<AudioRenderer> rendererOj = nullptr;

    proxyObj->SaveRendererObj(rendererOj);
    StreamSetStateEventInternal streamSetStateEventInternal;
    proxyObj->MuteStreamImpl(streamSetStateEventInternal);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_003
 * @tc.desc  : Test UnmuteStreamImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_003, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    AppInfo appInfo = {};
    std::shared_ptr<AudioRenderer> rendererOj = std::make_shared<AudioRendererPrivate>(
        AudioStreamType::STREAM_GAME, appInfo, true);
    ASSERT_TRUE(rendererOj != nullptr);

    proxyObj->SaveRendererObj(rendererOj);
    StreamSetStateEventInternal streamSetStateEventInternal;
    proxyObj->UnmuteStreamImpl(streamSetStateEventInternal);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_004
 * @tc.desc  : Test UnmuteStreamImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_004, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    std::shared_ptr<AudioRenderer> rendererOj = nullptr;

    proxyObj->SaveRendererObj(rendererOj);
    StreamSetStateEventInternal streamSetStateEventInternal;
    proxyObj->UnmuteStreamImpl(streamSetStateEventInternal);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_005
 * @tc.desc  : Test PausedStreamImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_005, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    AppInfo appInfo = {};
    std::shared_ptr<AudioRenderer> rendererOj = std::make_shared<AudioRendererPrivate>(
        AudioStreamType::STREAM_GAME, appInfo, true);
    ASSERT_TRUE(rendererOj != nullptr);

    proxyObj->SaveRendererObj(rendererOj);
    StreamSetStateEventInternal streamSetStateEventInternal;
    proxyObj->PausedStreamImpl(streamSetStateEventInternal);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_006
 * @tc.desc  : Test PausedStreamImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_006, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    std::shared_ptr<AudioRenderer> rendererOj = nullptr;

    proxyObj->SaveRendererObj(rendererOj);
    StreamSetStateEventInternal streamSetStateEventInternal;
    proxyObj->PausedStreamImpl(streamSetStateEventInternal);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_007
 * @tc.desc  : Test ResumeStreamImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_007, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    AppInfo appInfo = {};
    std::shared_ptr<AudioRenderer> rendererOj = std::make_shared<AudioRendererPrivate>(
        AudioStreamType::STREAM_GAME, appInfo, true);
    ASSERT_TRUE(rendererOj != nullptr);

    proxyObj->SaveRendererObj(rendererOj);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_008
 * @tc.desc  : Test ResumeStreamImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_008, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    std::shared_ptr<AudioRenderer> rendererOj = nullptr;

    proxyObj->SaveRendererObj(rendererOj);
    StreamSetStateEventInternal streamSetStateEventInternal;
    proxyObj->ResumeStreamImpl(streamSetStateEventInternal);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_009
 * @tc.desc  : Test SetLowPowerVolumeImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_009, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    AppInfo appInfo = {};
    std::shared_ptr<AudioRenderer> rendererOj = std::make_shared<AudioRendererPrivate>(
        AudioStreamType::STREAM_GAME, appInfo, true);
    ASSERT_TRUE(rendererOj != nullptr);

    float volume = 0.1;
    proxyObj->SaveRendererObj(rendererOj);
    proxyObj->SetLowPowerVolumeImpl(volume);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_010
 * @tc.desc  : Test SetLowPowerVolumeImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_010, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    std::shared_ptr<AudioRenderer> rendererOj = nullptr;

    float volume = 0.1;
    proxyObj->SaveRendererObj(rendererOj);
    proxyObj->SetLowPowerVolumeImpl(volume);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_009
 * @tc.desc  : Test GetLowPowerVolumeImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_011, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    AppInfo appInfo = {};
    std::shared_ptr<AudioRenderer> rendererOj = std::make_shared<AudioRendererPrivate>(
        AudioStreamType::STREAM_GAME, appInfo, true);
    ASSERT_TRUE(rendererOj != nullptr);

    float volume;
    proxyObj->SaveRendererObj(rendererOj);
    proxyObj->GetLowPowerVolumeImpl(volume);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_010
 * @tc.desc  : Test GetLowPowerVolumeImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_012, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    std::shared_ptr<AudioRenderer> rendererOj = nullptr;

    float volume;
    proxyObj->SaveRendererObj(rendererOj);
    proxyObj->GetLowPowerVolumeImpl(volume);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_013
 * @tc.desc  : Test SetOffloadModeImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_013, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    AppInfo appInfo = {};
    std::shared_ptr<AudioRenderer> rendererOj = std::make_shared<AudioRendererPrivate>(
        AudioStreamType::STREAM_GAME, appInfo, true);
    ASSERT_TRUE(rendererOj != nullptr);

    int32_t state = 0;
    bool isAppBack = true;
    proxyObj->SaveRendererObj(rendererOj);
    proxyObj->SetOffloadModeImpl(state, isAppBack);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_014
 * @tc.desc  : Test SetOffloadModeImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_014, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    std::shared_ptr<AudioRenderer> rendererOj = nullptr;

    int32_t state = 0;
    bool isAppBack = true;
    proxyObj->SaveRendererObj(rendererOj);
    proxyObj->SetOffloadModeImpl(state, isAppBack);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_015
 * @tc.desc  : Test UnsetOffloadModeImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_015, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    AppInfo appInfo = {};
    std::shared_ptr<AudioRenderer> rendererOj = std::make_shared<AudioRendererPrivate>(
        AudioStreamType::STREAM_GAME, appInfo, true);
    ASSERT_TRUE(rendererOj != nullptr);

    proxyObj->SaveRendererObj(rendererOj);
    proxyObj->UnsetOffloadModeImpl();
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_016
 * @tc.desc  : Test UnsetOffloadModeImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_016, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    std::shared_ptr<AudioRenderer> rendererOj = nullptr;

    proxyObj->SaveRendererObj(rendererOj);
    proxyObj->UnsetOffloadModeImpl();
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_017
 * @tc.desc  : Test GetSingleStreamVolumeImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_017, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    AppInfo appInfo = {};
    std::shared_ptr<AudioRenderer> rendererOj = std::make_shared<AudioRendererPrivate>(
        AudioStreamType::STREAM_GAME, appInfo, true);
    ASSERT_TRUE(rendererOj != nullptr);

    float volume;
    proxyObj->SaveRendererObj(rendererOj);
    proxyObj->GetSingleStreamVolumeImpl(volume);
}

/**
 * @tc.name  : Test AudioRendererProxyObj.
 * @tc.number: AudioRendererProxyObj_018
 * @tc.desc  : Test GetSingleStreamVolumeImpl api
 */
HWTEST(AudioRendererProxyObjUnitTest, AudioRendererProxyObj_018, TestSize.Level1)
{
    auto proxyObj = std::make_shared<AudioRendererProxyObj>();
    ASSERT_TRUE(proxyObj != nullptr);

    std::shared_ptr<AudioRenderer> rendererOj = nullptr;

    float volume;
    proxyObj->SaveRendererObj(rendererOj);
    proxyObj->GetSingleStreamVolumeImpl(volume);
}
} // namespace AudioStandard
} // namespace OHOS