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

#include <iostream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_utils.h"
#include "common/hdi_adapter_info.h"
#include "manager/hdi_adapter_manager.h"
#include "sink/multichannel_audio_render_sink.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
class MultichannelAudioRenderSinkUnitTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    virtual void SetUp();
    virtual void TearDown();

protected:
    static uint32_t id_;
    static std::shared_ptr<IAudioRenderSink> sink_;
    static IAudioSinkAttr attr_;
};

uint32_t MultichannelAudioRenderSinkUnitTest::id_ = HDI_INVALID_ID;
std::shared_ptr<IAudioRenderSink> MultichannelAudioRenderSinkUnitTest::sink_ = nullptr;
IAudioSinkAttr MultichannelAudioRenderSinkUnitTest::attr_ = {};

void MultichannelAudioRenderSinkUnitTest::SetUpTestCase()
{
    id_ = HdiAdapterManager::GetInstance().GetId(HDI_ID_BASE_RENDER, HDI_ID_TYPE_MULTICHANNEL, HDI_ID_INFO_DEFAULT,
        true);
}

void MultichannelAudioRenderSinkUnitTest::TearDownTestCase()
{
    HdiAdapterManager::GetInstance().ReleaseId(id_);
}

void MultichannelAudioRenderSinkUnitTest::SetUp()
{
    sink_ = HdiAdapterManager::GetInstance().GetRenderSink(id_, true);
    if (sink_ == nullptr) {
        return;
    }
}

void MultichannelAudioRenderSinkUnitTest::TearDown()
{
    sink_ = nullptr;
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_001
 * @tc.desc   : Test multichannel sink create
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_001, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_002
 * @tc.desc   : Test multichannel sink deinit
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_002, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    sink_->DeInit();
    EXPECT_FALSE(sink_->IsInited());
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_003
 * @tc.desc   : Test multichannel sink start, stop, resume, pause, flush, reset
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_003, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    int32_t ret = sink_->Start();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
    ret = sink_->Resume();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->Pause();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->Flush();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->Reset();
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->Stop();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_004
 * @tc.desc   : Test multichannel sink set/get volume
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_004, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    int32_t ret = sink_->SetVolume(0.0f, 0.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->SetVolume(0.0f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->SetVolume(1.0f, 0.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    ret = sink_->SetVolume(1.0f, 1.0f);
    EXPECT_EQ(ret, ERR_INVALID_HANDLE);
    float left;
    float right;
    ret = sink_->GetVolume(left, right);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_005
 * @tc.desc   : Test multichannel sink set audio scene
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_005, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    int32_t ret = sink_->SetAudioScene(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_006
 * @tc.desc   : Test multichannel sink update active device
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_006, TestSize.Level1)
{
    EXPECT_TRUE(sink_);
    std::vector<DeviceType> deviceTypes = { DEVICE_TYPE_SPEAKER };
    int32_t ret = sink_->UpdateActiveDevice(deviceTypes);
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_007
 * @tc.desc   : Test multichannel sink static function
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_007, TestSize.Level1)
{
    AudioFormat hdiFormat = MultichannelAudioRenderSink::ConvertToHdiFormat(SAMPLE_U8);
    EXPECT_EQ(hdiFormat, AUDIO_FORMAT_TYPE_PCM_8_BIT);
    hdiFormat = MultichannelAudioRenderSink::ConvertToHdiFormat(SAMPLE_S16LE);
    EXPECT_EQ(hdiFormat, AUDIO_FORMAT_TYPE_PCM_16_BIT);
    hdiFormat = MultichannelAudioRenderSink::ConvertToHdiFormat(SAMPLE_S24LE);
    EXPECT_EQ(hdiFormat, AUDIO_FORMAT_TYPE_PCM_24_BIT);
    hdiFormat = MultichannelAudioRenderSink::ConvertToHdiFormat(SAMPLE_S32LE);
    EXPECT_EQ(hdiFormat, AUDIO_FORMAT_TYPE_PCM_32_BIT);
    hdiFormat = MultichannelAudioRenderSink::ConvertToHdiFormat(SAMPLE_F32LE);
    EXPECT_EQ(hdiFormat, AUDIO_FORMAT_TYPE_PCM_16_BIT);

    uint32_t bitFormat = MultichannelAudioRenderSink::PcmFormatToBit(SAMPLE_U8);
    EXPECT_EQ(bitFormat, PCM_8_BIT);
    bitFormat = MultichannelAudioRenderSink::PcmFormatToBit(SAMPLE_S16LE);
    EXPECT_EQ(bitFormat, PCM_16_BIT);
    bitFormat = MultichannelAudioRenderSink::PcmFormatToBit(SAMPLE_S24LE);
    EXPECT_EQ(bitFormat, PCM_24_BIT);
    bitFormat = MultichannelAudioRenderSink::PcmFormatToBit(SAMPLE_S32LE);
    EXPECT_EQ(bitFormat, PCM_32_BIT);
    bitFormat = MultichannelAudioRenderSink::PcmFormatToBit(SAMPLE_F32LE);
    EXPECT_EQ(bitFormat, PCM_16_BIT);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_008
 * @tc.desc   : Test multichannel sink static function
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_008, TestSize.Level1)
{
    AudioSampleFormat sampleFormat = MultichannelAudioRenderSink::ParseAudioFormat("AUDIO_FORMAT_PCM_16_BIT");
    EXPECT_EQ(sampleFormat, SAMPLE_S16LE);
    sampleFormat = MultichannelAudioRenderSink::ParseAudioFormat("AUDIO_FORMAT_PCM_24_BIT");
    EXPECT_EQ(sampleFormat, SAMPLE_S24LE);
    sampleFormat = MultichannelAudioRenderSink::ParseAudioFormat("AUDIO_FORMAT_PCM_32_BIT");
    EXPECT_EQ(sampleFormat, SAMPLE_S32LE);
    sampleFormat = MultichannelAudioRenderSink::ParseAudioFormat("");
    EXPECT_EQ(sampleFormat, SAMPLE_S16LE);

    AudioCategory audioCategory = MultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_DEFAULT);
    EXPECT_EQ(audioCategory, AUDIO_IN_MEDIA);
    audioCategory = MultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_RINGING);
    EXPECT_EQ(audioCategory, AUDIO_IN_RINGTONE);
    audioCategory = MultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_PHONE_CALL);
    EXPECT_EQ(audioCategory, AUDIO_IN_CALL);
    audioCategory = MultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_PHONE_CHAT);
    EXPECT_EQ(audioCategory, AUDIO_IN_COMMUNICATION);
    audioCategory = MultichannelAudioRenderSink::GetAudioCategory(AUDIO_SCENE_MAX);
    EXPECT_EQ(audioCategory, AUDIO_IN_MEDIA);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_009
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_009, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    char *data = nullptr;
    uint64_t len = 0;
    multichannelAudioRenderSink->startUpdate_ = false;
    multichannelAudioRenderSink->CheckUpdateState(data, len);
    EXPECT_NE(multichannelAudioRenderSink, nullptr);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_010
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_010, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    char *data = nullptr;
    uint64_t len = 0;
    multichannelAudioRenderSink->startUpdate_ = true;
    multichannelAudioRenderSink->renderFrameNum_ = 0;
    multichannelAudioRenderSink->CheckUpdateState(data, len);
    EXPECT_EQ(multichannelAudioRenderSink->renderFrameNum_, 1);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_011
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_011, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    char *data = nullptr;
    uint64_t len = 0;
    multichannelAudioRenderSink->startUpdate_ = true;
    multichannelAudioRenderSink->renderFrameNum_ = 9;
    multichannelAudioRenderSink->lastGetMaxAmplitudeTime_ = -1;
    multichannelAudioRenderSink->CheckUpdateState(data, len);
    EXPECT_EQ(multichannelAudioRenderSink->renderFrameNum_, 0);
    EXPECT_EQ(multichannelAudioRenderSink->startUpdate_, false);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_012
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_012, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    char *data = nullptr;
    uint64_t len = 0;
    multichannelAudioRenderSink->attr_.channel = MultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    multichannelAudioRenderSink->attr_.format = AudioSampleFormat::SAMPLE_U8;

    multichannelAudioRenderSink->AdjustAudioBalance(data, len);
    EXPECT_EQ(multichannelAudioRenderSink->attr_.channel, 2);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_013
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_013, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    char *data = nullptr;
    uint64_t len = 0;
    multichannelAudioRenderSink->attr_.channel = MultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    multichannelAudioRenderSink->attr_.format = AudioSampleFormat::SAMPLE_S16LE;

    multichannelAudioRenderSink->AdjustAudioBalance(data, len);
    EXPECT_EQ(multichannelAudioRenderSink->attr_.channel, 2);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_014
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_014, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    char *data = nullptr;
    uint64_t len = 0;
    multichannelAudioRenderSink->attr_.channel = MultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    multichannelAudioRenderSink->attr_.format = AudioSampleFormat::SAMPLE_S24LE;

    multichannelAudioRenderSink->AdjustAudioBalance(data, len);
    EXPECT_EQ(multichannelAudioRenderSink->attr_.channel, 2);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_015
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_015, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    char *data = nullptr;
    uint64_t len = 0;
    multichannelAudioRenderSink->attr_.channel = MultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    multichannelAudioRenderSink->attr_.format = AudioSampleFormat::SAMPLE_S32LE;

    multichannelAudioRenderSink->AdjustAudioBalance(data, len);
    EXPECT_EQ(multichannelAudioRenderSink->attr_.channel, 2);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_016
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_016, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    char *data = nullptr;
    uint64_t len = 0;
    multichannelAudioRenderSink->attr_.channel = MultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    multichannelAudioRenderSink->attr_.format = AudioSampleFormat::SAMPLE_F32LE;

    multichannelAudioRenderSink->AdjustAudioBalance(data, len);
    EXPECT_EQ(multichannelAudioRenderSink->attr_.channel, 2);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_017
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_017, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    char *data = nullptr;
    uint64_t len = 0;
    multichannelAudioRenderSink->attr_.channel = MultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    multichannelAudioRenderSink->attr_.format = AudioSampleFormat::SAMPLE_U8;

    multichannelAudioRenderSink->AdjustStereoToMono(data, len);
    EXPECT_EQ(multichannelAudioRenderSink->attr_.channel, 2);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_018
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_018, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    char *data = nullptr;
    uint64_t len = 0;
    multichannelAudioRenderSink->attr_.channel = MultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    multichannelAudioRenderSink->attr_.format = AudioSampleFormat::SAMPLE_S16LE;

    multichannelAudioRenderSink->AdjustStereoToMono(data, len);
    EXPECT_EQ(multichannelAudioRenderSink->attr_.channel, 2);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_019
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_019, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    char *data = nullptr;
    uint64_t len = 0;
    multichannelAudioRenderSink->attr_.channel = MultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    multichannelAudioRenderSink->attr_.format = AudioSampleFormat::SAMPLE_S24LE;

    multichannelAudioRenderSink->AdjustStereoToMono(data, len);
    EXPECT_EQ(multichannelAudioRenderSink->attr_.channel, 2);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_020
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_020, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    char *data = nullptr;
    uint64_t len = 0;
    multichannelAudioRenderSink->attr_.channel = MultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    multichannelAudioRenderSink->attr_.format = AudioSampleFormat::SAMPLE_S32LE;

    multichannelAudioRenderSink->AdjustStereoToMono(data, len);
    EXPECT_EQ(multichannelAudioRenderSink->attr_.channel, 2);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_021
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_021, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    char *data = nullptr;
    uint64_t len = 0;
    multichannelAudioRenderSink->attr_.channel = MultichannelAudioRenderSink::STEREO_CHANNEL_COUNT;
    multichannelAudioRenderSink->attr_.format = AudioSampleFormat::SAMPLE_F32LE;

    multichannelAudioRenderSink->AdjustStereoToMono(data, len);
    EXPECT_EQ(multichannelAudioRenderSink->attr_.channel, 2);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_022
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_022, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    multichannelAudioRenderSink->renderInited_ = true;

    auto ret = multichannelAudioRenderSink->InitRender();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_023
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_023, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>();
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    multichannelAudioRenderSink->renderInited_ = true;

    auto ret = multichannelAudioRenderSink->InitRender();
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_024
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_024, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>(HDI_ID_INFO_USB);
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    struct AudioSceneDescriptor sceneDesc;
    AudioScene audioScene = AUDIO_SCENE_DEFAULT;

    multichannelAudioRenderSink->InitSceneDesc(sceneDesc, audioScene);
    EXPECT_EQ(sceneDesc.desc.pins, PIN_OUT_USB_HEADSET);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_025
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_025, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>(HDI_ID_INFO_DP);
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    struct AudioSceneDescriptor sceneDesc;
    AudioScene audioScene = AUDIO_SCENE_DEFAULT;

    multichannelAudioRenderSink->InitSceneDesc(sceneDesc, audioScene);
    EXPECT_EQ(sceneDesc.desc.pins, PIN_OUT_SPEAKER);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_026
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_026, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>(HDI_ID_INFO_USB);
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    struct AudioDeviceDescriptor deviceDesc;

    multichannelAudioRenderSink->InitDeviceDesc(deviceDesc);
    EXPECT_EQ(deviceDesc.pins, PIN_OUT_USB_HEADSET);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_027
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_027, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>(HDI_ID_INFO_DP);
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    struct AudioDeviceDescriptor deviceDesc;

    multichannelAudioRenderSink->halName_ = HDI_ID_INFO_USB;
    multichannelAudioRenderSink->InitDeviceDesc(deviceDesc);
    EXPECT_EQ(deviceDesc.pins, PIN_OUT_USB_HEADSET);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_028
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_028, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>(HDI_ID_INFO_DP);
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    DeviceType device = DEVICE_TYPE_WIRED_HEADSET;
    multichannelAudioRenderSink->currentActiveDevice_ = DEVICE_TYPE_WIRED_HEADSET;

    multichannelAudioRenderSink->ResetActiveDeviceForDisconnect(device);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_029
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_029, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>(HDI_ID_INFO_DP);
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    DeviceType device = DEVICE_TYPE_NONE;
    multichannelAudioRenderSink->currentActiveDevice_ = DEVICE_TYPE_WIRED_HEADSET;

    multichannelAudioRenderSink->ResetActiveDeviceForDisconnect(device);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_030
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_030, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>(HDI_ID_INFO_DP);
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    std::vector<DeviceType> outputDevices;
    outputDevices.push_back(DEVICE_TYPE_NONE);
    EXPECT_EQ(!outputDevices.empty() && outputDevices.size() == 1, true);

    multichannelAudioRenderSink->currentActiveDevice_ = DEVICE_TYPE_NONE;

    auto ret = multichannelAudioRenderSink->UpdateActiveDevice(outputDevices);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_031
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_031, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>(HDI_ID_INFO_DP);
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    std::vector<DeviceType> outputDevices;
    outputDevices.push_back(DEVICE_TYPE_NONE);
    EXPECT_EQ(!outputDevices.empty() && outputDevices.size() == 1, true);

    multichannelAudioRenderSink->currentActiveDevice_ = DEVICE_TYPE_SPEAKER;

    auto ret = multichannelAudioRenderSink->UpdateActiveDevice(outputDevices);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_032
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_032, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>(HDI_ID_INFO_DP);
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    AudioParamKey key = NONE;
    std::string condition = "get_usb_info";
    multichannelAudioRenderSink->adapterNameCase_ = "abc";

    multichannelAudioRenderSink->GetAudioParameter(key, condition);
}

/**
 * @tc.name   : Test MultichannelSink API
 * @tc.number : MultichannelSinkUnitTest_033
 * @tc.desc   : Test MultichannelAudioRenderSink
 */
HWTEST_F(MultichannelAudioRenderSinkUnitTest, MultichannelSinkUnitTest_033, TestSize.Level1)
{
    auto multichannelAudioRenderSink = std::make_shared<MultichannelAudioRenderSink>(HDI_ID_INFO_DP);
    EXPECT_NE(multichannelAudioRenderSink, nullptr);

    AudioParamKey key = NONE;
    std::string condition = "abc";

    multichannelAudioRenderSink->GetAudioParameter(key, condition);
    EXPECT_EQ(multichannelAudioRenderSink->adapterNameCase_, "");
}
} // namespace AudioStandard
} // namespace OHOS