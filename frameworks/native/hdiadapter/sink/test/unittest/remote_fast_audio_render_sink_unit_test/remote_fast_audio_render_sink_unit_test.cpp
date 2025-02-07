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

#include "remote_fast_audio_renderer_sink.h"
#include "remote_fast_audio_renderer_sink.cpp"
#include "audio_hdiadapter_info.h"
#include "securec.h"
#include <cinttypes>
#include <dlfcn.h>
#include <sstream>
#include "audio_hdi_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

class RemoteFastAudioRenderSinkUnitTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

class IAudioDeviceManagerInner : public IAudioDeviceManager {
public:
    IAudioDeviceManagerInner() {};
    ~IAudioDeviceManagerInner() {};

    int32_t GetAllAdapters() override
    {
        return SUCCESS;
    };

    struct AudioAdapterDescriptor *GetTargetAdapterDesc(const std::string &adapterName, bool isMmap) override
    {
        return nullptr;
    };

    std::shared_ptr<IAudioDeviceAdapter> LoadAdapters(const std::string &adapterName, bool isMmap) override
    {
        return nullptr;
    };

    int32_t UnloadAdapter(const std::string &adapterName) override
    {
        return SUCCESS;
    };

    int32_t Release() override
    {
        return SUCCESS;
    };
};

class IAudioDeviceAdapterInner : public IAudioDeviceAdapter {
public:
    IAudioDeviceAdapterInner() {};
    ~IAudioDeviceAdapterInner() {};

    int32_t Init() override
    {
        return SUCCESS;
    };

    int32_t RegExtraParamObserver() override
    {
        return SUCCESS;
    };

    int32_t CreateRender(const AudioDeviceDescriptor &devDesc, const AudioSampleAttributes &attr,
        sptr<IAudioRender> &audioRender, IAudioDeviceAdapterCallback *renderCb, uint32_t &renderId) override
    {
        return SUCCESS;
    };

    void DestroyRender(sptr<IAudioRender> audioRender, uint32_t &renderId) override
    {
    };

    int32_t CreateCapture(const AudioDeviceDescriptor &devDesc, const AudioSampleAttributes &attr,
        sptr<IAudioCapture> &audioCapture, IAudioDeviceAdapterCallback *captureCb, uint32_t &captureId) override
    {
        return SUCCESS;
    };

    void DestroyCapture(sptr<IAudioCapture> audioCapture, uint32_t &captureId) override
    {
    };

    void SetAudioParameter(const AudioParamKey key, const std::string &condition,
        const std::string &value) override
    {
    };

    std::string GetAudioParameter(const AudioParamKey key, const std::string &condition) override
    {
        return nullptr;
    };

    int32_t UpdateAudioRoute(const AudioRoute &route) override
    {
        return SUCCESS;
    };

    int32_t Release() override
    {
        return SUCCESS;
    };
};

namespace {
    const std::string DEVICE_DEFAULT_ID = "12345";
    IAudioSinkAttr audioAttr = {};
    const struct AudioPort renderPort = {
        .portId = 1,
        .portName = "default",
    };

} // namespace

void RemoteFastAudioRenderSinkUnitTest::SetUpTestCase()
{}

void RemoteFastAudioRenderSinkUnitTest::TearDownTestCase()
{}

void RemoteFastAudioRenderSinkUnitTest::SetUp()
{}

void RemoteFastAudioRenderSinkUnitTest::TearDown()
{}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_001
 * @tc.desc  : Test Template RemoteFastAudioRendererSink ClearRender Interface.
 */
#define DEBUG_DIRECT_USE_HDI
#define FEATURE_DISTRIBUTE_AUDIO
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_001, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    audioRendererInner->ClearRender();
    audioRendererInner->bufferFd_ = 2;
    audioRendererInner->ClearRender();
    std::shared_ptr<IAudioDeviceAdapter> audioAdapter = std::make_shared<IAudioDeviceAdapterInner>();
    audioRendererInner->audioAdapter_ = audioAdapter;
    std::shared_ptr<IAudioDeviceManager> audioManager = std::make_shared<IAudioDeviceManagerInner>();
    audioRendererInner->audioManager_ = audioManager;
    audioRendererInner->ClearRender();
}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_002
 * @tc.desc  : Test Template RemoteFastAudioRendererSink Init&DeInit Interface.
 */
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_002, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    audioRendererInner->DeInit();
    std::shared_ptr<IAudioDeviceAdapter> audioAdapter = std::make_shared<IAudioDeviceAdapterInner>();
    audioRendererInner->audioAdapter_ = audioAdapter;
    std::shared_ptr<IAudioDeviceManager> audioManager = std::make_shared<IAudioDeviceManagerInner>();
    audioRendererInner->audioManager_ = audioManager;
    int32_t ret = audioRendererInner->CreateRender(renderPort);
    EXPECT_EQ(ret, SUCCESS);
    audioRendererInner->DeInit();
    audioRendererInner->ClearRender();
}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_003
 * @tc.desc  : Test Template RemoteFastAudioRendererSink CreateRender Interface.
 */
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_003, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    std::shared_ptr<IAudioDeviceAdapter> audioAdapter = std::make_shared<IAudioDeviceAdapterInner>();
    audioRendererInner->audioAdapter_ = audioAdapter;
    std::shared_ptr<IAudioDeviceManager> audioManager = std::make_shared<IAudioDeviceManagerInner>();
    audioRendererInner->audioManager_ = audioManager;
    audioRendererInner->attr_.audioStreamFlag = AUDIO_FLAG_DIRECT;
    int32_t ret = audioRendererInner->CreateRender(renderPort);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_004
 * @tc.desc  : Test Template RemoteFastAudioRendererSink PrepareMmapBuffer Interface.
 */
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_004, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    std::shared_ptr<IAudioDeviceAdapter> audioAdapter = std::make_shared<IAudioDeviceAdapterInner>();
    audioRendererInner->audioAdapter_ = audioAdapter;
    std::shared_ptr<IAudioDeviceManager> audioManager = std::make_shared<IAudioDeviceManagerInner>();
    audioRendererInner->audioManager_ = audioManager;
    int32_t ret = audioRendererInner->CreateRender(renderPort);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioRendererInner->PrepareMmapBuffer();
}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_005
 * @tc.desc  : Test Template RemoteFastAudioRendererSink PcmFormatToBits Interface.
 */
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_005, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    HdiAdapterFormat format = static_cast<HdiAdapterFormat>(SAMPLE_U8);
    audioRendererInner->PcmFormatToBits(format);
    format = SAMPLE_S16;
    audioRendererInner->PcmFormatToBits(format);
    format = SAMPLE_S24;
    audioRendererInner->PcmFormatToBits(format);
    format = SAMPLE_S32;
    audioRendererInner->PcmFormatToBits(format);
    format = SAMPLE_F32;
    audioRendererInner->PcmFormatToBits(format);
    format = static_cast<HdiAdapterFormat>(INVALID_WIDTH);
    audioRendererInner->PcmFormatToBits(format);
}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_006
 * @tc.desc  : Test Template RemoteFastAudioRendererSink ConvertToHdiFormat Interface.
 */
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_006, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    HdiAdapterFormat format = static_cast<HdiAdapterFormat>(SAMPLE_U8);
    audioRendererInner->ConvertToHdiFormat(format);
    format = SAMPLE_S16;
    audioRendererInner->ConvertToHdiFormat(format);
    format = SAMPLE_S24;
    audioRendererInner->ConvertToHdiFormat(format);
    format = SAMPLE_S32;
    audioRendererInner->ConvertToHdiFormat(format);
    format = SAMPLE_F32;
    audioRendererInner->ConvertToHdiFormat(format);
    format = static_cast<HdiAdapterFormat>(INVALID_WIDTH);
    audioRendererInner->ConvertToHdiFormat(format);
}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_007
 * @tc.desc  : Test Template RemoteFastAudioRendererSink CheckPositionTime Interface.
 */
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_007, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    int32_t ret = audioRendererInner->CheckPositionTime();
    EXPECT_EQ(ret, ERROR);
    std::shared_ptr<IAudioDeviceAdapter> audioAdapter = std::make_shared<IAudioDeviceAdapterInner>();
    audioRendererInner->audioAdapter_ = audioAdapter;
    std::shared_ptr<IAudioDeviceManager> audioManager = std::make_shared<IAudioDeviceManagerInner>();
    audioRendererInner->audioManager_ = audioManager;
    ret = audioRendererInner->CreateRender(renderPort);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioRendererInner->CheckPositionTime();
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_008
 * @tc.desc  : Test Template RemoteFastAudioRendererSink Start&Stop&Pause&Resume Interface.
 */
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_008, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    std::shared_ptr<IAudioDeviceAdapter> audioAdapter = std::make_shared<IAudioDeviceAdapterInner>();
    audioRendererInner->audioAdapter_ = audioAdapter;
    std::shared_ptr<IAudioDeviceManager> audioManager = std::make_shared<IAudioDeviceManagerInner>();
    audioRendererInner->audioManager_ = audioManager;
    int32_t ret = audioRendererInner->CreateRender(renderPort);
    EXPECT_EQ(ret, SUCCESS);
    ret = audioRendererInner->Start();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    audioRendererInner->isRenderCreated_ = true;
    audioRendererInner->started_ = true;
    ret = audioRendererInner->Start();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_009
 * @tc.desc  : Test Template RemoteFastAudioRendererSink Start&Stop&Pause&Resume Interface.
 */
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_009, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    int32_t ret = audioRendererInner->Stop();
    EXPECT_EQ(ret, SUCCESS);
    std::shared_ptr<IAudioDeviceAdapter> audioAdapter = std::make_shared<IAudioDeviceAdapterInner>();
    audioRendererInner->audioAdapter_ = audioAdapter;
    std::shared_ptr<IAudioDeviceManager> audioManager = std::make_shared<IAudioDeviceManagerInner>();
    audioRendererInner->audioManager_ = audioManager;
    ret = audioRendererInner->CreateRender(renderPort);
    EXPECT_EQ(ret, SUCCESS);
    audioRendererInner->isRenderCreated_ = true;
    audioRendererInner->started_ = true;
    ret = audioRendererInner->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = audioRendererInner->Stop();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_0010
 * @tc.desc  : Test Template RemoteFastAudioRendererSink Start&Stop&Pause&Resume Interface.
 */
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_0010, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    int32_t ret = audioRendererInner->Pause();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    std::shared_ptr<IAudioDeviceAdapter> audioAdapter = std::make_shared<IAudioDeviceAdapterInner>();
    audioRendererInner->audioAdapter_ = audioAdapter;
    std::shared_ptr<IAudioDeviceManager> audioManager = std::make_shared<IAudioDeviceManagerInner>();
    audioRendererInner->audioManager_ = audioManager;
    ret = audioRendererInner->CreateRender(renderPort);
    EXPECT_EQ(ret, SUCCESS);
    audioRendererInner->isRenderCreated_ = true;
    audioRendererInner->started_ = true;
    ret = audioRendererInner->Start();
    EXPECT_EQ(ret, SUCCESS);
    audioRendererInner->started_ = true;
    ret = audioRendererInner->Pause();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    audioRendererInner->paused_ = true;
    ret = audioRendererInner->Pause();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_0011
 * @tc.desc  : Test Template RemoteFastAudioRendererSink Start&Stop&Pause&Resume Interface.
 */
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_0011, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    int32_t ret = audioRendererInner->Reset();
    EXPECT_EQ(ret, ERR_ILLEGAL_STATE);
    std::shared_ptr<IAudioDeviceAdapter> audioAdapter = std::make_shared<IAudioDeviceAdapterInner>();
    audioRendererInner->audioAdapter_ = audioAdapter;
    std::shared_ptr<IAudioDeviceManager> audioManager = std::make_shared<IAudioDeviceManagerInner>();
    audioRendererInner->audioManager_ = audioManager;
    ret = audioRendererInner->CreateRender(renderPort);
    EXPECT_EQ(ret, SUCCESS);
    audioRendererInner->isRenderCreated_ = true;
    audioRendererInner->started_ = true;
    ret = audioRendererInner->Start();
    EXPECT_EQ(ret, SUCCESS);
    ret = audioRendererInner->Reset();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = audioRendererInner->Reset();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = audioRendererInner->Resume();
    EXPECT_EQ(ret, SUCCESS);
    audioRendererInner->paused_ = true;
    ret = audioRendererInner->Resume();
}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_0012
 * @tc.desc  : Test Template RemoteFastAudioRendererSink SetVolume Interface.
 */
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_0012, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    std::shared_ptr<IAudioDeviceAdapter> audioAdapter = std::make_shared<IAudioDeviceAdapterInner>();
    audioRendererInner->audioAdapter_ = audioAdapter;
    std::shared_ptr<IAudioDeviceManager> audioManager = std::make_shared<IAudioDeviceManagerInner>();
    audioRendererInner->audioManager_ = audioManager;
    audioRendererInner->CreateRender(renderPort);
    audioRendererInner->isRenderCreated_ = true;
    audioRendererInner->started_ = true;
    int32_t ret = audioRendererInner->Start();
    float left = 0.0;
    float right = 0.0;
    ret = audioRendererInner->SetVolume(left, right);
    right = 0.5;
    ret = audioRendererInner->SetVolume(left, right);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    left = 0.5;
    right = 0.0;
    ret = audioRendererInner->SetVolume(left, right);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_0013
 * @tc.desc  : Test Template RemoteFastAudioRendererSink OnAudioParamChange Interface.
 */
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_0013, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    const std::string adapterName = "default";
    AudioParamKey key = BT_HEADSET_NREC;
    const std::string condition = "condition";
    const std::string value = "false";
    audioRendererInner->OnAudioParamChange(adapterName, key, condition, value);
    key = PARAM_KEY_STATE;
    audioRendererInner->OnAudioParamChange(adapterName, key, condition, value);
}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_0014
 * @tc.desc  : Test Template RemoteFastAudioRendererSink GetInstance Interface.
 */
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_0014, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    IMmapAudioRendererSink* instance = audioRendererInner->GetInstance(DEVICE_DEFAULT_ID);
    EXPECT_NE(instance, nullptr);
    instance = audioRendererInner->GetInstance(DEVICE_DEFAULT_ID);
    EXPECT_NE(instance, nullptr);
    audioRendererInner->rendererInited_ = true;
}

/**
 * @tc.name  : Test Template RemoteFastAudioRendererSink
 * @tc.number: RemoteFastAudioRendererSink_0015
 * @tc.desc  : Test Template RemoteFastAudioRendererSink GetInstance Interface.
 */
HWTEST(RemoteFastAudioRenderSinkUnitTest, RemoteFastAudioRenderSinkUnitTest_0015, TestSize.Level1)
{
    std::shared_ptr<RemoteFastAudioRendererSinkInner> audioRendererInner =
        std::make_shared<RemoteFastAudioRendererSinkInner>(DEVICE_DEFAULT_ID);
    EXPECT_NE(audioRendererInner, nullptr);
    IMmapAudioRendererSink* instance = audioRendererInner->GetInstance(DEVICE_DEFAULT_ID);
    EXPECT_NE(instance, nullptr);
    auto ret = instance->GetAudioScene();
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}
} // namespace AudioStandard
} // namespace OHOS