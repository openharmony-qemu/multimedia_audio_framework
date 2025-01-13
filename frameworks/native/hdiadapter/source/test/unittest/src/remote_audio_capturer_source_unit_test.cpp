/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "remote_audio_capturer_source.h"

#include "i_audio_capturer_source.h"
#include "audio_hdiadapter_info.h"
#include "securec.h"
#include <cinttypes>
#include <dlfcn.h>
#include <sstream>
#include "audio_hdi_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class RemoteAudioCapturerSourceUnitTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

class RemoteAudioCapturerSourceCallback : public IAudioSourceCallback {
public:
    virtual ~RemoteAudioCapturerSourceCallback() {}

    void OnWakeupClose() override
    {
        std::cout << "Wakeup has been closed." << std::endl;
    }

    void OnAudioSourceParamChange(const std::string& netWorkId, const AudioParamKey key,
        const std::string& condition, const std::string& value) override
    {
        std::cout << "Audio source parameter changed. Network ID: " << netWorkId << ", Key: " << key << ", Condition: "
        << condition << ", Value: " << value << std::endl;
    }
};

namespace {
    IAudioSourceAttr g_iAttr = {};
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;
    AudioScene audioScene = AUDIO_SCENE_DEFAULT;
} // namespace

void RemoteAudioCapturerSourceUnitTest::SetUpTestCase()
{}

void RemoteAudioCapturerSourceUnitTest::TearDownTestCase()
{}

void RemoteAudioCapturerSourceUnitTest::SetUp()
{}

void RemoteAudioCapturerSourceUnitTest::TearDown()
{}

/**
 * @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
 * @tc.number: remote_audio_capturer_source_unittest001.
 * @tc.desc  : Test Init Function.
 */
HWTEST(RemoteAudioCapturerSourceUnitTest, remote_audio_capturer_source_unittest001, TestSize.Level1)
{
    const std::string deviceNetworkId = "device_network_id";
    auto remoteCapturer = RemoteAudioCapturerSource::GetInstance(deviceNetworkId);
    EXPECT_NE(nullptr, remoteCapturer);
    remoteCapturer->Init(g_iAttr);
    bool ret = remoteCapturer->IsInited();
    EXPECT_EQ(false, ret);
    remoteCapturer->DeInit();
}

/**
 * @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
 * @tc.number: remote_audio_capturer_source_unittest002.
 * @tc.desc  : Test CaptureFrame Function.
 */
HWTEST(RemoteAudioCapturerSourceUnitTest, remote_audio_capturer_source_unittest002, TestSize.Level1)
{
    const std::string deviceNetworkId = "device_network_id";
    auto remoteCapturer = RemoteAudioCapturerSource::GetInstance(deviceNetworkId);
    EXPECT_NE(nullptr, remoteCapturer);
    remoteCapturer->Init(g_iAttr);
    char* frame = nullptr;
    uint64_t requestBytes = 0;
    uint64_t replyBytes = 20;
    int32_t ret = remoteCapturer->CaptureFrame(frame, requestBytes, replyBytes);
    EXPECT_EQ(ERR_INVALID_HANDLE, ret);
}

/**
 * @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
 * @tc.number: remote_audio_capturer_source_unittest003.
 * @tc.desc  : Test Start Function.
 */
HWTEST(RemoteAudioCapturerSourceUnitTest, remote_audio_capturer_source_unittest003, TestSize.Level1)
{
    const std::string deviceNetworkId = "device_network_id";
    auto remoteCapturer = RemoteAudioCapturerSource::GetInstance(deviceNetworkId);
    EXPECT_NE(nullptr, remoteCapturer);
    remoteCapturer->Init(g_iAttr);
    int32_t ret = remoteCapturer->Start();
    EXPECT_EQ(ERR_NOT_STARTED, ret);
    ret = remoteCapturer->Pause();
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
    ret = remoteCapturer->Resume();
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
    ret = remoteCapturer->Reset();
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
    ret = remoteCapturer->Flush();
    EXPECT_EQ(ERR_ILLEGAL_STATE, ret);
    ret = remoteCapturer->Stop();
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
 * @tc.number: remote_audio_capturer_source_unittest004.
 * @tc.desc  : Test SetVolume&GetVolume Function.
 */
HWTEST(RemoteAudioCapturerSourceUnitTest, remote_audio_capturer_source_unittest004, TestSize.Level1)
{
    const std::string deviceNetworkId = "device_network_id";
    auto remoteCapturer = RemoteAudioCapturerSource::GetInstance(deviceNetworkId);
    EXPECT_NE(nullptr, remoteCapturer);
    remoteCapturer->Init(g_iAttr);
    float left = 0.0;
    float right = 0.5;
    remoteCapturer->SetVolume(left, right);
    remoteCapturer->GetVolume(left, right);
}

/**
 * @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
 * @tc.number: remote_audio_capturer_source_unittest005.
 * @tc.desc  : Test SetMute&GetMute Function.
 */
HWTEST(RemoteAudioCapturerSourceUnitTest, remote_audio_capturer_source_unittest005, TestSize.Level1)
{
    const std::string deviceNetworkId = "device_network_id";
    auto remoteCapturer = RemoteAudioCapturerSource::GetInstance(deviceNetworkId);
    EXPECT_NE(nullptr, remoteCapturer);
    remoteCapturer->Init(g_iAttr);
    bool isMute = true;
    int32_t ret = remoteCapturer->SetMute(isMute);
    EXPECT_EQ(SUCCESS, ret);
    ret = remoteCapturer->GetMute(isMute);
    EXPECT_EQ(ERR_INVALID_HANDLE, ret);
}

/**
 * @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
 * @tc.number: remote_audio_capturer_source_unittest006.
 * @tc.desc  : Test SetInputRoute Function.
 */
HWTEST(RemoteAudioCapturerSourceUnitTest, remote_audio_capturer_source_unittest006, TestSize.Level1)
{
    const std::string deviceNetworkId = "device_network_id";
    auto remoteCapturer = RemoteAudioCapturerSource::GetInstance(deviceNetworkId);
    EXPECT_NE(nullptr, remoteCapturer);
    remoteCapturer->Init(g_iAttr);
    int32_t ret = remoteCapturer->SetInputRoute(deviceType);
    EXPECT_EQ(ERR_NOT_SUPPORTED, ret);
}

/**
 * @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
 * @tc.number: remote_audio_capturer_source_unittest007.
 * @tc.desc  : Test SetAudioScene Function.
 */
HWTEST(RemoteAudioCapturerSourceUnitTest, remote_audio_capturer_source_unittest007, TestSize.Level1)
{
    const std::string deviceNetworkId = "device_network_id";
    auto remoteCapturer = RemoteAudioCapturerSource::GetInstance(deviceNetworkId);
    EXPECT_NE(nullptr, remoteCapturer);
    remoteCapturer->Init(g_iAttr);
    int32_t ret = remoteCapturer->SetAudioScene(audioScene, deviceType);
    EXPECT_EQ(ERR_INVALID_HANDLE, ret);
}

/**
 * @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
 * @tc.number: remote_audio_capturer_source_unittest008.
 * @tc.desc  : Test RegisterParameterCallback Function.
 */
#define FEATURE_DISTRIBUTE_AUDIO
HWTEST(RemoteAudioCapturerSourceUnitTest, remote_audio_capturer_source_unittest008, TestSize.Level1)
{
    const std::string deviceNetworkId = "device_network_id";
    auto remoteCapturer = RemoteAudioCapturerSource::GetInstance(deviceNetworkId);
    EXPECT_NE(nullptr, remoteCapturer);
    remoteCapturer->Init(g_iAttr);
    RemoteAudioCapturerSourceCallback* callback = new RemoteAudioCapturerSourceCallback();
    remoteCapturer->RegisterParameterCallback(callback);
}
} // namespace AudioStandard
} // namespace OHOS
