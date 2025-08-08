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

#include "audio_zone_client_unit_test.h"
#include "audio_zone_manager.h"
#include "audio_errors.h"
#include "audio_policy_log.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudiZoneClientUnitTest::SetUpTestCase(void) {}
void AudiZoneClientUnitTest::TearDownTestCase(void) {}
void AudiZoneClientUnitTest::SetUp(void) {}
void AudiZoneClientUnitTest::TearDown(void) {}

class AudioZoneCallbackStub : public AudioZoneCallback {
public:
    ~AudioZoneCallbackStub() {}

    void OnAudioZoneAdd(const AudioZoneDescriptor &zoneDescriptor) override {}

    void OnAudioZoneRemove(int32_t zoneId) override {}
};

class AudioZoneChangeCallbackStub : public AudioZoneChangeCallback {
public:
    ~AudioZoneChangeCallbackStub() override {}

    void OnAudioZoneChange(const AudioZoneDescriptor &zoneDescriptor,
        AudioZoneChangeReason reason) override {}
};

class AudioZoneVolumeProxyStub : public AudioZoneVolumeProxy {
public:
    ~AudioZoneVolumeProxyStub() override {}

    void SetSystemVolume(const AudioVolumeType volumeType, const int32_t volumeLevel) override {}

    int32_t GetSystemVolume(AudioVolumeType volumeType) override {return 0;}

    void SetSystemVolumeDegree(AudioVolumeType volumeType, int32_t volumeDegree) override {}

    int32_t GetSystemVolumeDegree(AudioVolumeType volumeType) override { return 0;}
};

class AudioZoneInterruptCallbackStub : public AudioZoneInterruptCallback {
public:
    ~AudioZoneInterruptCallbackStub() {}

    void OnInterruptEvent(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
        AudioZoneInterruptReason reason) override {}
};

/**
* @tc.name  : Test AudiZoneClientUnitTest.
* @tc.number: AudiZoneClientUnitTest_001
* @tc.desc  : Test AddAudioZoneCallback interface.
*/
HWTEST_F(AudiZoneClientUnitTest, AudiZoneClientUnitTest_001, TestSize.Level4)
{
    auto audioZoneClient = std::make_shared<AudioZoneClient>();
    auto callback = std::make_shared<AudioZoneCallbackStub>();
    audioZoneClient->audioZoneCallback_ = callback;
    auto ret = audioZoneClient->AddAudioZoneCallback(callback);
    EXPECT_EQ(SUCCESS, ret);

    audioZoneClient->audioZoneCallback_ = nullptr;
    ret = audioZoneClient->AddAudioZoneCallback(callback);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudiZoneClientUnitTest.
* @tc.number: AudiZoneClientUnitTest_002
* @tc.desc  : Test AddAudioZoneChangeCallback interface.
*/
HWTEST_F(AudiZoneClientUnitTest, AudiZoneClientUnitTest_002, TestSize.Level4)
{
    auto audioZoneClient = std::make_shared<AudioZoneClient>();
    int32_t zoneId = 0;
    auto callback = std::make_shared<AudioZoneChangeCallbackStub>();
    audioZoneClient->audioZoneChangeCallbackMap_[zoneId] = callback;
    auto ret = audioZoneClient->AddAudioZoneChangeCallback(zoneId, callback);
    EXPECT_EQ(SUCCESS, ret);

    audioZoneClient->audioZoneChangeCallbackMap_.erase(zoneId);
    ret = audioZoneClient->AddAudioZoneChangeCallback(zoneId, callback);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test AudiZoneClientUnitTest.
* @tc.number: AudiZoneClientUnitTest_003
* @tc.desc  : Test AddAudioZoneVolumeProxy interface.
*/
HWTEST_F(AudiZoneClientUnitTest, AudiZoneClientUnitTest_003, TestSize.Level4)
{
    auto audioZoneClient = std::make_shared<AudioZoneClient>();
    int32_t zoneId = 0;
    auto callback = std::make_shared<AudioZoneVolumeProxyStub>();
    audioZoneClient->audioZoneVolumeProxyMap_[zoneId] = callback;
    auto ret = audioZoneClient->AddAudioZoneVolumeProxy(zoneId, callback);
    EXPECT_EQ(SUCCESS, ret);

    audioZoneClient->audioZoneVolumeProxyMap_.erase(zoneId);
    ret = audioZoneClient->AddAudioZoneVolumeProxy(zoneId, callback);
    EXPECT_NE(SUCCESS, ret);
}


/**
* @tc.name  : Test AudiZoneClientUnitTest.
* @tc.number: AudiZoneClientUnitTest_004
* @tc.desc  : Test AddAudioInterruptCallback interface.
*/
HWTEST_F(AudiZoneClientUnitTest, AudiZoneClientUnitTest_004, TestSize.Level4)
{
    auto audioZoneClient = std::make_shared<AudioZoneClient>();
    int32_t zoneId = 0;
    std::string deviceTag = "test";
    auto callback = std::make_shared<AudioZoneInterruptCallbackStub>();
    std::string key = std::to_string(zoneId) + "&" + deviceTag;
    audioZoneClient->audioZoneInterruptCallbackMap_[key] = callback;
    auto ret = audioZoneClient->AddAudioInterruptCallback(zoneId, deviceTag, callback);
    EXPECT_EQ(SUCCESS, ret);

    audioZoneClient->audioZoneInterruptCallbackMap_.erase(key);
    ret = audioZoneClient->AddAudioInterruptCallback(zoneId, deviceTag, callback);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test AudiZoneClientUnitTest.
* @tc.number: AudiZoneClientUnitTest_005
* @tc.desc  : Test Restore interface.
*/
HWTEST_F(AudiZoneClientUnitTest, AudiZoneClientUnitTest_005, TestSize.Level4)
{
    auto audioZoneClient = std::make_shared<AudioZoneClient>();
    auto callback = std::make_shared<AudioZoneCallbackStub>();
    audioZoneClient->Restore();
    EXPECT_EQ(nullptr, audioZoneClient->audioZoneCallback_);
    audioZoneClient->audioZoneCallback_ = callback;
    audioZoneClient->Restore();
    EXPECT_NE(nullptr, audioZoneClient->audioZoneCallback_);
}

/**
* @tc.name  : Test AudiZoneClientUnitTest.
* @tc.number: AudiZoneClientUnitTest_DegreeTest_001
* @tc.desc  : Test SetSystemVolumeDegree interface.
*/
HWTEST_F(AudiZoneClientUnitTest, AudiZoneClientUnitTest_DegreeTest_001, TestSize.Level4)
{
    auto audioZoneClient = std::make_shared<AudioZoneClient>();
    ASSERT_NE(audioZoneClient, nullptr);

    AudioVolumeType volumeType = STREAM_MUSIC;
    int32_t volumeDegree = 10;
    int32_t outVolumeDegree = -1;
    pid_t pid = 106;

    int32_t zoneId1 = 1;
    int32_t zoneId2 = 2;
    auto callback = std::make_shared<AudioZoneVolumeProxyStub>();
    audioZoneClient->audioZoneVolumeProxyMap_[zoneId1] = callback;
    audioZoneClient->audioZoneVolumeProxyMap_[zoneId2] = nullptr;

    EXPECT_EQ(audioZoneClient->SetSystemVolumeDegree(0, volumeType, volumeDegree, 0), ERR_OPERATION_FAILED);
    EXPECT_EQ(audioZoneClient->SetSystemVolumeDegree(zoneId2, volumeType, volumeDegree, 0), ERR_OPERATION_FAILED);
    EXPECT_EQ(audioZoneClient->SetSystemVolumeDegree(zoneId1, volumeType, volumeDegree, 0), 0);

    EXPECT_EQ(audioZoneClient->GetSystemVolumeDegree(0, volumeType, outVolumeDegree), ERR_OPERATION_FAILED);
    EXPECT_EQ(audioZoneClient->GetSystemVolumeDegree(zoneId2, volumeType, outVolumeDegree), ERR_OPERATION_FAILED);
    EXPECT_EQ(audioZoneClient->GetSystemVolumeDegree(zoneId1, volumeType, outVolumeDegree), 0);
}
} // namespace AudioStandard
} // namespace OHOS