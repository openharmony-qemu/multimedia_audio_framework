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
#include "gtest/gtest.h"
#include "audio_server_hpae_dump.h"
#include "hpae_manager_impl.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeAudioServerHpaeDumpTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp() {}
    void TearDown() {}
    static std::shared_ptr<AudioServerHpaeDump> audioServerHpaeDump_;
};

std::shared_ptr<AudioServerHpaeDump> HpaeAudioServerHpaeDumpTest::audioServerHpaeDump_ =
    std::make_shared<AudioServerHpaeDump>();

void HpaeAudioServerHpaeDumpTest::SetUpTestCase()
{
    IHpaeManager::GetHpaeManager().Init();
    ASSERT_NE(audioServerHpaeDump_, nullptr);
    IHpaeManager::GetHpaeManager().RegisterHpaeDumpCallback(audioServerHpaeDump_);
    std::this_thread::sleep_for(std::chrono::milliseconds(20)); // 20ms for sleep
}

void HpaeAudioServerHpaeDumpTest::TearDownTestCase()
{
    IHpaeManager::GetHpaeManager().DeInit();
}

TEST_F(HpaeAudioServerHpaeDumpTest, HpaeAudioServerHpaeDumpTest_001)
{
    std::string tempString;
    audioServerHpaeDump_->PlaybackSinkInputDump(tempString);
    EXPECT_EQ(tempString, "Playback Streams\n- 0 Playback stream (s) available:\n\n");

    tempString.clear();
    audioServerHpaeDump_->RecordSourceOutputDump(tempString);
    EXPECT_EQ(tempString, "Record Streams\n- 0 Record stream (s) available:\n\n");

    std::vector<HpaeInputOutputInfo> inputOutputInfo;
    inputOutputInfo.push_back({0, "", 0, 0, 0, false, PRIVACY_TYPE_PUBLIC, "", HPAE_SESSION_NEW, 0});

    audioServerHpaeDump_->dumpSinkInputsInfo_.clear();
    audioServerHpaeDump_->OnDumpSinkInputsInfoCb(inputOutputInfo, 1);
    EXPECT_EQ(audioServerHpaeDump_->dumpSinkInputsInfo_.empty(), true);

    audioServerHpaeDump_->dumpSinkInputsInfo_.clear();
    audioServerHpaeDump_->OnDumpSinkInputsInfoCb(inputOutputInfo, 0);
    EXPECT_EQ(audioServerHpaeDump_->dumpSinkInputsInfo_.empty(), false);

    audioServerHpaeDump_->dumpSourceOutputsInfo_.clear();
    audioServerHpaeDump_->OnDumpSourceOutputsInfoCb(inputOutputInfo, 1);
    EXPECT_EQ(audioServerHpaeDump_->dumpSourceOutputsInfo_.empty(), true);

    audioServerHpaeDump_->dumpSourceOutputsInfo_.clear();
    audioServerHpaeDump_->OnDumpSourceOutputsInfoCb(inputOutputInfo, 0);
    EXPECT_EQ(audioServerHpaeDump_->dumpSourceOutputsInfo_.empty(), false);
}

TEST_F(HpaeAudioServerHpaeDumpTest, HpaeAudioServerHpaeDumpTest_002)
{
    bool isTrue = audioServerHpaeDump_->GetDevicesInfo();
    EXPECT_EQ(isTrue, true);

    std::string dumpString;
    HpaeSinkSourceInfo sinkInfo = { "Speaker", "" };
    audioServerHpaeDump_->devicesInfo_.sinkInfos.push_back(sinkInfo);
    audioServerHpaeDump_->PlaybackSinkDump(dumpString);
    EXPECT_EQ(dumpString.empty(), false);

    HpaeSinkSourceInfo sourceInfo = { "Built_in_mic", "" };
    audioServerHpaeDump_->devicesInfo_.sourceInfos.push_back(sourceInfo);
    dumpString.clear();
    audioServerHpaeDump_->RecordSourceDump(dumpString);
    EXPECT_EQ(dumpString.empty(), false);

    std::queue<std::u16string> argQue;
    dumpString.clear();
    audioServerHpaeDump_->ArgDataDump(dumpString, argQue);
    EXPECT_EQ(dumpString.empty(), false);
}

TEST_F(HpaeAudioServerHpaeDumpTest, ArgDataDump_001)
{
    bool isTrue = audioServerHpaeDump_->GetDevicesInfo();
    EXPECT_EQ(isTrue, true);
    std::string dumpString;

    std::queue<std::u16string> argQue;
    argQue.push(u"-h");
    dumpString.clear();
    audioServerHpaeDump_->ArgDataDump(dumpString, argQue);
    EXPECT_EQ(dumpString.empty(), false);

    argQue.pop();
    argQue.push(u"-p");
    dumpString.clear();
    audioServerHpaeDump_->ArgDataDump(dumpString, argQue);
    EXPECT_EQ(dumpString.empty(), false);

    argQue.pop();
    argQue.push(u"-f");
    dumpString.clear();
    audioServerHpaeDump_->ArgDataDump(dumpString, argQue);
    EXPECT_EQ(dumpString.empty(), false);

    argQue.pop();
    argQue.push(u"-m");
    dumpString.clear();
    audioServerHpaeDump_->ArgDataDump(dumpString, argQue);
    EXPECT_EQ(dumpString.empty(), false);
}
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
