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
#include "audio_zone_types.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_zone_types.h"
#include "ipc_skeleton.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

bool g_hasPermission = false;

void GetPermission()
{
    if (!g_hasPermission) {
        uint64_t tokenId;
        constexpr int perNum = 10;
        const char *perms[perNum] = {
            "ohos.permission.MICROPHONE",
            "ohos.permission.MANAGE_INTELLIGENT_VOICE",
            "ohos.permission.MANAGE_AUDIO_CONFIG",
            "ohos.permission.MICROPHONE_CONTROL",
            "ohos.permission.MODIFY_AUDIO_SETTINGS",
            "ohos.permission.ACCESS_NOTIFICATION_POLICY",
            "ohos.permission.USE_BLUETOOTH",
            "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
            "ohos.permission.RECORD_VOICE_CALL",
            "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 10,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audiofuzztest",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

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

    void SetSystemVolume(const DeviceType deviceType, const AudioVolumeType volumeType,
        const int32_t volumeLevel) override {}

    int32_t GetSystemVolume(const DeviceType deviceType, const AudioVolumeType volumeType) override {return 10;}

    void SetSystemVolumeDegree(const DeviceType deviceType, const AudioVolumeType volumeType,
        int32_t volumeDegree) override {}

    int32_t GetSystemVolumeDegree(const DeviceType deviceType, const AudioVolumeType volumeType) override { return 10;}
};

class AudioZoneInterruptCallbackStub : public AudioZoneInterruptCallback {
public:
    ~AudioZoneInterruptCallbackStub() {}

    void OnInterruptEvent(const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts,
        AudioZoneInterruptReason reason) override {}
};

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
* @tc.number: AudiZoneClientUnitTest_006
* @tc.desc  : Test GetCurrentRendererChangeInfosForZone invalid param.
*/
HWTEST_F(AudiZoneClientUnitTest, AudiZoneClientUnitTest_006, TestSize.Level4)
{
    auto zoneManager = AudioZoneManager::GetInstance();
    ASSERT_NE(zoneManager, nullptr);

    std::vector<std::shared_ptr<AudioRendererChangeInfo>> infos;
    int32_t ret = zoneManager->GetCurrentRendererChangeInfosForZone(-1, infos);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

} // namespace AudioStandard
} // namespace OHOS