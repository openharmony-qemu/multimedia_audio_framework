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
#include "hpae_soft_link.h"
#include "hpae_manager_impl.h"
#include "hpae_audio_service_callback_unit_test.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
static std::string g_rootPath = "/data/";
class HpaeSoftLinkTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();
    void OpenAudioPort(bool openSink = true);
    void CloseAudioPort(bool closeSink = true);

    static std::shared_ptr<HpaeAudioServiceCallbackUnitTest> callback_;
    int32_t sinkId_ = -1;
    int32_t sourceId_ = -1;
};

static AudioModuleInfo GetSinkAudioModeInfo(std::string name = "Speaker_File")
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-sink.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = name;
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "7680";
    audioModuleInfo.format = "s32le";
    audioModuleInfo.fixedLatency = "1";
    audioModuleInfo.offloadEnable = "0";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.fileName = g_rootPath + audioModuleInfo.adapterName + "_" + audioModuleInfo.rate + "_" +
                               audioModuleInfo.channels + "_" + audioModuleInfo.format + ".pcm";
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

static AudioModuleInfo GetSourceAudioModeInfo(std::string name = "mic")
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-source.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = name;
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "3840";
    audioModuleInfo.format = "s16le";
    audioModuleInfo.fixedLatency = "1";
    audioModuleInfo.offloadEnable = "0";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.fileName = g_rootPath + "source_" + audioModuleInfo.adapterName + "_" + audioModuleInfo.rate + "_" +
                               audioModuleInfo.channels + "_" + audioModuleInfo.format + ".pcm";
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_FILE_SOURCE);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

std::shared_ptr<HpaeAudioServiceCallbackUnitTest> HpaeSoftLinkTest::callback_ =
    std::make_shared<HpaeAudioServiceCallbackUnitTest>();
void HpaeSoftLinkTest::SetUpTestCase()
{
    IHpaeManager::GetHpaeManager().Init();
    IHpaeManager::GetHpaeManager().RegisterSerivceCallback(callback_);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 200ms for sleep
}

void HpaeSoftLinkTest::TearDownTestCase()
{
    IHpaeManager::GetHpaeManager().DeInit();
}

void HpaeSoftLinkTest::SetUp()
{
    OpenAudioPort(true);
    OpenAudioPort(false);
}

void HpaeSoftLinkTest::TearDown()
{
    CloseAudioPort(true);
    CloseAudioPort(false);
}

void HpaeSoftLinkTest::OpenAudioPort(bool openSink)
{
    AudioModuleInfo moduleInfo = openSink ? GetSinkAudioModeInfo() : GetSourceAudioModeInfo();
    EXPECT_EQ(IHpaeManager::GetHpaeManager().OpenAudioPort(moduleInfo), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 200ms for sleep
    if (openSink) {
        sinkId_ = callback_->GetPortId();
    } else {
        sourceId_ = callback_->GetPortId();
    }
}

void HpaeSoftLinkTest::CloseAudioPort(bool closeSink)
{
    IHpaeManager::GetHpaeManager().CloseAudioPort(closeSink ? sinkId_ : sourceId_);
    std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 200ms for sleep
    if (closeSink) {
        sinkId_ = -1;
    } else {
        sourceId_ = -1;
    }
}

TEST_F(HpaeSoftLinkTest, testSoftLink)
{
    std::shared_ptr<HpaeSoftLink> softLink_ =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink_, nullptr);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::NEW);

    EXPECT_EQ(softLink_->Init(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::PREPARED);

    EXPECT_EQ(softLink_->Init(), SUCCESS); // init after init
    EXPECT_EQ(softLink_->Stop(), ERR_ILLEGAL_STATE); // stop after init

    EXPECT_EQ(softLink_->Start(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::RUNNING);

    EXPECT_EQ(softLink_->Init(), ERR_ILLEGAL_STATE); // init after start
    EXPECT_EQ(softLink_->Start(), SUCCESS); // start after start

    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 2000ms for sleep

    EXPECT_EQ(softLink_->Stop(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::STOPPED);

    EXPECT_EQ(softLink_->Stop(), SUCCESS); // stop after stop

    EXPECT_EQ(softLink_->Release(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::RELEASED);
}

TEST_F(HpaeSoftLinkTest, testCapturerOverFlow)
{
    std::shared_ptr<HpaeSoftLink> softLink_ =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink_, nullptr);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::NEW);
    EXPECT_EQ(softLink_->Init(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::PREPARED);
    EXPECT_EQ(softLink_->Start(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::RUNNING);

    auto &capturerSessionId = softLink_->capturerStreamInfo_.sessionId;
    if (softLink_->state_ == HpaeSoftLinkState::RUNNING &&
        softLink_->streamStateMap_.find(capturerSessionId) != softLink_->streamStateMap_.end()) {
        EXPECT_EQ(softLink_->GetStreamStateById(capturerSessionId), HpaeSoftLinkState::RUNNING);
    }

    CloseAudioPort();
    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 2000ms for sleep
    if (softLink_->state_ == HpaeSoftLinkState::RUNNING &&
        softLink_->streamStateMap_.find(capturerSessionId) != softLink_->streamStateMap_.end()) {
        AUDIO_INFO_LOG("capturer has stopped");
        EXPECT_EQ(softLink_->GetStreamStateById(capturerSessionId), HpaeSoftLinkState::STOPPED);
    }
    EXPECT_EQ(softLink_->Release(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::RELEASED);
}

TEST_F(HpaeSoftLinkTest, testRendererUnderRun)
{
    std::shared_ptr<HpaeSoftLink> softLink_ =
        std::make_shared<HpaeSoftLink>(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink_, nullptr);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::NEW);
    EXPECT_EQ(softLink_->Init(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::PREPARED);
    EXPECT_EQ(softLink_->Start(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::RUNNING);

    auto &rendererSessionId = softLink_->rendererStreamInfo_.sessionId;
    if (softLink_->state_ == HpaeSoftLinkState::RUNNING &&
        softLink_->streamStateMap_.find(rendererSessionId) != softLink_->streamStateMap_.end()) {
        EXPECT_EQ(softLink_->GetStreamStateById(rendererSessionId), HpaeSoftLinkState::RUNNING);
    }

    CloseAudioPort(false);
    std::this_thread::sleep_for(std::chrono::milliseconds(2000)); // 2000ms for sleep
    if (softLink_->state_ == HpaeSoftLinkState::RUNNING &&
        softLink_->streamStateMap_.find(rendererSessionId) != softLink_->streamStateMap_.end()) {
        AUDIO_INFO_LOG("renderer has stopped");
        EXPECT_EQ(softLink_->GetStreamStateById(rendererSessionId), HpaeSoftLinkState::STOPPED);
    }
    EXPECT_EQ(softLink_->Release(), SUCCESS);
    EXPECT_EQ(softLink_->state_, HpaeSoftLinkState::RELEASED);
}

TEST_F(HpaeSoftLinkTest, testStaticFunc)
{
    std::shared_ptr<IHpaeSoftLink> softLink1 =
        IHpaeSoftLink::CreateSoftLink(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_NE(softLink1, nullptr);
    softLink1->SetVolume(0.f);

    CloseAudioPort(false);
    std::shared_ptr<IHpaeSoftLink> softLink2 =
        IHpaeSoftLink::CreateSoftLink(sinkId_, sourceId_, SoftLinkMode::HEARING_AID);
    EXPECT_EQ(softLink2, nullptr);

    std::shared_ptr<IHpaeSoftLink> softLink3 =
        IHpaeSoftLink::CreateSoftLink(sinkId_, -1, SoftLinkMode::HEARING_AID);
    EXPECT_EQ(softLink3, nullptr);

    HpaeSoftLink::g_sessionId = 99999; // 99999 for max sessionId
    EXPECT_EQ(HpaeSoftLink::GenerateSessionId(), 99999); // 99999 for max sessionId
    EXPECT_EQ(HpaeSoftLink::g_sessionId, 90000); // 90000 for min sessionId
}
} // namespace HPAE
} // namespace AudioStandard
} // namespace OHOS
