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
#include "remote_fast_audio_capturer_source.h"
#include "remote_fast_audio_capturer_source.cpp"
#include "i_audio_capturer_source.h"
#include "audio_hdiadapter_info.h"
#include "securec.h"
#include <cinttypes>
#include <dlfcn.h>
#include <sstream>
#include "audio_errors.h"
#include "audio_hdi_log.h"
#include "i_audio_device_adapter.h"
#include "i_audio_device_manager.h"
#include <v1_0/iaudio_manager.h>
#include <v1_0/iaudio_callback.h>
#include <v1_0/audio_types.h>
#include <audio_module_info.h>

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
// class RemoteFastAudioCapturerSourceInner;
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
class RemoteFastAudioCapturerSourceUnitTest : public ::testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() override;
    void TearDown() override;
};

void RemoteFastAudioCapturerSourceUnitTest::SetUpTestCase()
{}

void RemoteFastAudioCapturerSourceUnitTest::TearDownTestCase()
{}

void RemoteFastAudioCapturerSourceUnitTest::SetUp()
{}

void RemoteFastAudioCapturerSourceUnitTest::TearDown()
{}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest001
* @tc.desc  : Test GetInstance.
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest001, TestSize.Level0)
{
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>("exist_device_network_id");
    const std::string deviceNetworkId = "exist_device_network_id";
    auto rfCapturer = remoteFastAudioCapturerSourceInner->GetInstance(deviceNetworkId);
    EXPECT_NE(nullptr, rfCapturer);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest002
* @tc.desc  : Test GetInstance.
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest002, TestSize.Level0)
{
    const std::string deviceNetworkId = "device_network_id";
    auto rfCapturer = RemoteFastAudioCapturerSource::GetInstance(deviceNetworkId);
    EXPECT_NE(nullptr, rfCapturer);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest003
* @tc.desc  : Test ConvertToHdiFormat.
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest003, TestSize.Level0)
{
    auto format = HdiAdapterFormat::SAMPLE_S16;
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    AudioFormat hdiFormat = remoteFastAudioCapturerSourceInner->ConvertToHdiFormat(format);
    EXPECT_EQ(AudioFormat::AUDIO_FORMAT_TYPE_PCM_16_BIT, hdiFormat);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest004
* @tc.desc  : Test ConvertToHdiFormat.
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest004, TestSize.Level0)
{
    auto format = HdiAdapterFormat::SAMPLE_S24;
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    AudioFormat hdiFormat = remoteFastAudioCapturerSourceInner->ConvertToHdiFormat(format);
    EXPECT_EQ(AudioFormat::AUDIO_FORMAT_TYPE_PCM_24_BIT, hdiFormat);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest005
* @tc.desc  : Test ConvertToHdiFormat.
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest005, TestSize.Level0)
{
    auto format = HdiAdapterFormat::SAMPLE_S32;
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    AudioFormat hdiFormat = remoteFastAudioCapturerSourceInner->ConvertToHdiFormat(format);
    EXPECT_EQ(AudioFormat::AUDIO_FORMAT_TYPE_PCM_32_BIT, hdiFormat);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest006
* @tc.desc  : Test ConvertToHdiFormat.
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest006, TestSize.Level0)
{
    auto format = HdiAdapterFormat::INVALID_WIDTH;
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    AudioFormat hdiFormat = remoteFastAudioCapturerSourceInner->ConvertToHdiFormat(format);
    EXPECT_EQ(AudioFormat::AUDIO_FORMAT_TYPE_PCM_16_BIT, hdiFormat);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest007
* @tc.desc  : Test ConvertToHdiFormat.
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest007, TestSize.Level0)
{
    auto format = HdiAdapterFormat::SAMPLE_U8;
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    AudioFormat hdiFormat = remoteFastAudioCapturerSourceInner->ConvertToHdiFormat(format);
    EXPECT_EQ(AudioFormat::AUDIO_FORMAT_TYPE_PCM_8_BIT, hdiFormat);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest008
* @tc.desc  : Test store.
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest008, TestSize.Level0)
{
    const std::string deviceNetworkId = "device_network_id";
    RemoteFastAudioCapturerSourceInner *rfCapturer =
        new RemoteFastAudioCapturerSourceInner(deviceNetworkId);
    std::atomic<bool>* unittestPtr = new std::atomic<bool>();
    unittestPtr->store(false);
    EXPECT_NE(rfCapturer, nullptr);
    rfCapturer->capturerInited_ = unittestPtr;
    delete rfCapturer;
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest009
* @tc.desc  : Test delete RemoteFastAudioCapturerSourceInner
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest009, TestSize.Level0)
{
    const std::string deviceNetworkId = "device_network_id";
    RemoteFastAudioCapturerSourceInner *rfCapturer =
        new RemoteFastAudioCapturerSourceInner(deviceNetworkId);
    EXPECT_NE(rfCapturer, nullptr);
    delete rfCapturer;
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest010
* @tc.desc  : Test DeInit
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest010, TestSize.Level0)
{
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    remoteFastAudioCapturerSourceInner->DeInit();
    EXPECT_EQ(allRFSources.count(deviceNetworkId), 0);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest011
* @tc.desc  : Test DeInit
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest011, TestSize.Level0)
{
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    allRFSources[deviceNetworkId] =nullptr;
    remoteFastAudioCapturerSourceInner->DeInit();
    EXPECT_EQ(allRFSources.count(deviceNetworkId), 0);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest015
* @tc.desc  : Test Start
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest015, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    auto ret = remoteFastAudioCapturerSourceInner->Start();
    EXPECT_EQ(ret, ERR_NOT_STARTED);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest016
* @tc.desc  : Test Start
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest016, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    remoteFastAudioCapturerSourceInner->isCapturerCreated_ = true;
    auto ret = remoteFastAudioCapturerSourceInner->Start();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest017
* @tc.desc  : Test Start
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest017, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    remoteFastAudioCapturerSourceInner->isCapturerCreated_ = true;
    remoteFastAudioCapturerSourceInner->started_ = true;
    auto ret = remoteFastAudioCapturerSourceInner->Start();
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest018
* @tc.desc  : Test Stop
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest018, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    remoteFastAudioCapturerSourceInner->started_ = false;
    auto ret = remoteFastAudioCapturerSourceInner->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest018
* @tc.desc  : Test Stop
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest019, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    remoteFastAudioCapturerSourceInner->started_ = true;
    auto ret = remoteFastAudioCapturerSourceInner->Stop();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest020
* @tc.desc  : Test Pause
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest020, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    remoteFastAudioCapturerSourceInner->started_ = true;
    remoteFastAudioCapturerSourceInner->paused_ = false;
    auto ret = remoteFastAudioCapturerSourceInner->Pause();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest021
* @tc.desc  : Test Pause
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest021, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    remoteFastAudioCapturerSourceInner->started_ = true;
    remoteFastAudioCapturerSourceInner->paused_ = true;
    auto ret = remoteFastAudioCapturerSourceInner->Pause();
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest022
* @tc.desc  : Test Resume
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest022, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    remoteFastAudioCapturerSourceInner->started_ = true;
    remoteFastAudioCapturerSourceInner->paused_ = false;
    auto ret = remoteFastAudioCapturerSourceInner->Resume();
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest023
* @tc.desc  : Test Resume
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest023, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    remoteFastAudioCapturerSourceInner->started_ = true;
    remoteFastAudioCapturerSourceInner->paused_ = true;
    auto ret = remoteFastAudioCapturerSourceInner->Resume();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest028
* @tc.desc  : Test SetInputPortPin
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest028, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    AudioRouteNode source = {};
    auto ret = remoteFastAudioCapturerSourceInner->SetInputPortPin(DEVICE_TYPE_MIC, source);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest029
* @tc.desc  : Test SetInputPortPin
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest029, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    AudioRouteNode source = {};
    auto ret = remoteFastAudioCapturerSourceInner->SetInputPortPin(DEVICE_TYPE_WIRED_HEADSET, source);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest030
* @tc.desc  : Test SetInputPortPin
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest030, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    AudioRouteNode source = {};
    auto ret = remoteFastAudioCapturerSourceInner->SetInputPortPin(DEVICE_TYPE_USB_HEADSET, source);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest031
* @tc.desc  : Test SetInputPortPin
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest031, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    AudioRouteNode source = {};
    auto ret = remoteFastAudioCapturerSourceInner->SetInputPortPin(DEVICE_TYPE_FILE_SOURCE, source);
    EXPECT_EQ(ret, ERR_NOT_SUPPORTED);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest032
* @tc.desc  : Test GetAudioCategory
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest032, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    auto ret = remoteFastAudioCapturerSourceInner->GetAudioCategory(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(ret, AudioCategory::AUDIO_IN_MEDIA);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest033
* @tc.desc  : Test GetAudioCategory
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest033, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    auto ret = remoteFastAudioCapturerSourceInner->GetAudioCategory(AUDIO_SCENE_RINGING);
    EXPECT_EQ(ret, AudioCategory::AUDIO_IN_RINGTONE);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest034
* @tc.desc  : Test GetAudioCategory
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest034, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    auto ret = remoteFastAudioCapturerSourceInner->GetAudioCategory(AUDIO_SCENE_VOICE_RINGING);
    EXPECT_EQ(ret, AudioCategory::AUDIO_IN_RINGTONE);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest035
* @tc.desc  : Test GetAudioCategory
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest035, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    auto ret = remoteFastAudioCapturerSourceInner->GetAudioCategory(AUDIO_SCENE_PHONE_CALL);
    EXPECT_EQ(ret, AudioCategory::AUDIO_IN_CALL);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest036
* @tc.desc  : Test GetAudioCategory
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest036, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    auto ret = remoteFastAudioCapturerSourceInner->GetAudioCategory(AUDIO_SCENE_PHONE_CHAT);
    EXPECT_EQ(ret, AudioCategory::AUDIO_IN_COMMUNICATION);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest037
* @tc.desc  : Test GetAudioCategory
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest037, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    auto ret = remoteFastAudioCapturerSourceInner->GetAudioCategory(AUDIO_SCENE_MAX);
    EXPECT_EQ(ret, AudioCategory::AUDIO_IN_MEDIA);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest038
* @tc.desc  : Test PcmFormatToBits
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest038, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    auto ret = remoteFastAudioCapturerSourceInner->PcmFormatToBits(HdiAdapterFormat::SAMPLE_U8);
    EXPECT_EQ(ret, RemoteFastAudioCapturerSourceInner::PCM_8_BIT);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest039
* @tc.desc  : Test PcmFormatToBits
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest039, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    auto ret = remoteFastAudioCapturerSourceInner->PcmFormatToBits(HdiAdapterFormat::SAMPLE_S16);
    EXPECT_EQ(ret, RemoteFastAudioCapturerSourceInner::PCM_16_BIT);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest040
* @tc.desc  : Test PcmFormatToBits
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest040, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    auto ret = remoteFastAudioCapturerSourceInner->PcmFormatToBits(HdiAdapterFormat::SAMPLE_S24);
    EXPECT_EQ(ret, RemoteFastAudioCapturerSourceInner::PCM_24_BIT);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest041
* @tc.desc  : Test PcmFormatToBits
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest041, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    auto ret = remoteFastAudioCapturerSourceInner->PcmFormatToBits(HdiAdapterFormat::SAMPLE_S32);
    EXPECT_EQ(ret, RemoteFastAudioCapturerSourceInner::PCM_32_BIT);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest042
* @tc.desc  : Test PcmFormatToBits
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest042, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    auto ret = remoteFastAudioCapturerSourceInner->PcmFormatToBits(HdiAdapterFormat::SAMPLE_F32);
    EXPECT_EQ(ret, RemoteFastAudioCapturerSourceInner::PCM_32_BIT);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest043
* @tc.desc  : Test PcmFormatToBits
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest043, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    auto ret = remoteFastAudioCapturerSourceInner->PcmFormatToBits(HdiAdapterFormat::INVALID_WIDTH);
    EXPECT_EQ(ret, RemoteFastAudioCapturerSourceInner::PCM_16_BIT);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest044
* @tc.desc  : Test OnAudioParamChange
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest044, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    ASSERT_NE(nullptr, remoteFastAudioCapturerSourceInner);

    const std::string adapterName = "adapterName";
    const std::string condition = "condition";
    const std::string value = "value";
    remoteFastAudioCapturerSourceInner->OnAudioParamChange(adapterName,
        AudioParamKey::PARAM_KEY_STATE, condition, value);
    remoteFastAudioCapturerSourceInner->OnAudioParamChange(adapterName, AudioParamKey::VOLUME, condition, value);
    EXPECT_EQ(remoteFastAudioCapturerSourceInner->paramCb_, nullptr);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest045
* @tc.desc  : Test ClearCapture
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest045, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    ASSERT_NE(nullptr, remoteFastAudioCapturerSourceInner);

    remoteFastAudioCapturerSourceInner->bufferFd_ = RemoteFastAudioCapturerSourceInner::INVALID_FD;
    remoteFastAudioCapturerSourceInner->audioAdapter_ = nullptr;
    remoteFastAudioCapturerSourceInner->audioManager_ = nullptr;
    remoteFastAudioCapturerSourceInner->ClearCapture();
    EXPECT_EQ(remoteFastAudioCapturerSourceInner->capturerInited_.load(), false);
}

/**
* @tc.name  : Test RemoteFastAudioCapturerSourceUnitTest.
* @tc.number: remote_fast_audio_capturer_source_unittest046
* @tc.desc  : Test ClearCapture
*/
HWTEST_F(RemoteFastAudioCapturerSourceUnitTest, remote_fast_audio_capturer_source_unittest046, TestSize.Level0)
{
    const std::string capturePortName = "creatCapture";
    const std::string deviceNetworkId = "device_network_id";
    std::shared_ptr<RemoteFastAudioCapturerSourceInner> remoteFastAudioCapturerSourceInner =
        std::make_shared<RemoteFastAudioCapturerSourceInner>(deviceNetworkId);
    ASSERT_NE(nullptr, remoteFastAudioCapturerSourceInner);
    
    remoteFastAudioCapturerSourceInner->bufferFd_ = RemoteFastAudioCapturerSourceInner::HALF_FACTOR;
    std::shared_ptr<IAudioDeviceAdapter> audioAdapter = std::make_shared<IAudioDeviceAdapterInner>();
    remoteFastAudioCapturerSourceInner->audioAdapter_ = audioAdapter;
    std::shared_ptr<IAudioDeviceManager> audioManager = std::make_shared<IAudioDeviceManagerInner>();
    remoteFastAudioCapturerSourceInner->audioManager_ = audioManager;
    remoteFastAudioCapturerSourceInner->ClearCapture();
    EXPECT_EQ(remoteFastAudioCapturerSourceInner->capturerInited_.load(), false);
}
}
}
