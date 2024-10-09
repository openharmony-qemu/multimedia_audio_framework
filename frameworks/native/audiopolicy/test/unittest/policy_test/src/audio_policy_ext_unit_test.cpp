/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AudioPolicyUnitTest"
#endif

#include <thread>
#include "audio_errors.h"
#include "audio_info.h"
#include "parcel.h"
#include "audio_policy_client.h"
#include "audio_policy_unit_test.h"
#include "audio_system_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"
#include "audio_client_tracker_callback_stub.h"
#include "audio_policy_client_stub_impl.h"
#include "audio_adapter_manager.h"

using namespace std;
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
    constexpr uint32_t MAX_RENDERER_INSTANCES = 128;
}

class AudioPolicyExtUnitTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void AudioPolicyExtUnitTest::SetUpTestCase(void)
{
    // input testsuit setup step，setup invoked before all testcases
}

void AudioPolicyExtUnitTest::TearDownTestCase(void)
{
    // input testsuit teardown step，teardown invoked after all testcases
}

void AudioPolicyExtUnitTest::SetUp(void)
{
    // input testcase setup step，setup invoked before each testcases
}

void AudioPolicyExtUnitTest::TearDown(void)
{
    // input testcase teardown step，teardown invoked after each testcases
}

/**
 * @tc.name  : Test UpdateStreamState
 * @tc.number: UpdateStreamState_001
 * @tc.desc  : Test UpdateStreamState interface. Returns ret.
 */
HWTEST(AudioPolicyExtUnitTest, UpdateStreamState_001, TestSize.Level1)
{
    int32_t clientUid = 0;
    int32_t ret = AudioPolicyManager::GetInstance().UpdateStreamState(clientUid,
        StreamSetState::STREAM_PAUSE, StreamUsage::STREAM_USAGE_INVALID);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UpdateStreamState
 * @tc.number: UpdateStreamState_002
 * @tc.desc  : Test UpdateStreamState interface. Returns ret.
 */
HWTEST(AudioPolicyExtUnitTest, UpdateStreamState_002, TestSize.Level1)
{
    int32_t clientUid = 1;
    int32_t ret = AudioPolicyManager::GetInstance().UpdateStreamState(clientUid,
        StreamSetState::STREAM_PAUSE, StreamUsage::STREAM_USAGE_UNKNOWN);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UpdateStreamState
 * @tc.number: UpdateStreamState_003
 * @tc.desc  : Test UpdateStreamState interface. Returns ret.
 */
HWTEST(AudioPolicyExtUnitTest, UpdateStreamState_003, TestSize.Level1)
{
    int32_t clientUid = 2;
    int32_t ret = AudioPolicyManager::GetInstance().UpdateStreamState(clientUid,
        StreamSetState::STREAM_RESUME, StreamUsage::STREAM_USAGE_MEDIA);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test UpdateStreamState
 * @tc.number: UpdateStreamState_004
 * @tc.desc  : Test UpdateStreamState interface. Returns ret.
 */
HWTEST(AudioPolicyExtUnitTest, UpdateStreamState_004, TestSize.Level1)
{
    int32_t clientUid = 3;
    int32_t ret = AudioPolicyManager::GetInstance().UpdateStreamState(clientUid,
        StreamSetState::STREAM_RESUME, StreamUsage::STREAM_USAGE_MUSIC);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetVolumeGroupInfos via legal state
 * @tc.number: GetVolumeGroupInfos_001
 * @tc.desc  : Test GetVolumeGroupInfos interface. Get volume group infos and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, GetVolumeGroupInfos_001, TestSize.Level1)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    std::string networkId = "";
    int32_t ret = AudioPolicyManager::GetInstance().GetVolumeGroupInfos(networkId, infos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(infos.size(), 0);
}

/**
 * @tc.name  : Test GetVolumeGroupInfos via legal state
 * @tc.number: GetVolumeGroupInfos_002
 * @tc.desc  : Test GetVolumeGroupInfos interface. Get volume group infos and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, GetVolumeGroupInfos_002, TestSize.Level1)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    std::string networkId = LOCAL_NETWORK_ID;
    int32_t ret = AudioPolicyManager::GetInstance().GetVolumeGroupInfos(networkId, infos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_GT(infos.size(), 0);
}

/**
 * @tc.name  : Test GetNetworkIdByGroupId via legal state
 * @tc.number: GetNetworkIdByGroupId_001
 * @tc.desc  : Test GetNetworkIdByGroupId interface. Get networkId by groupId and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, GetNetworkIdByGroupId_001, TestSize.Level1)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    std::string networkId = LOCAL_NETWORK_ID;
    int32_t ret = AudioPolicyManager::GetInstance().GetVolumeGroupInfos(networkId, infos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_GT(infos.size(), 0);
    int32_t groupId = infos[0]->volumeGroupId_;
    ret = AudioPolicyManager::GetInstance().GetNetworkIdByGroupId(groupId, networkId);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name  : Test GetNetworkIdByGroupId via illegal state
 * @tc.number: GetNetworkIdByGroupId_002
 * @tc.desc  : Test GetNetworkIdByGroupId interface. Get networkId by groupId and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, GetNetworkIdByGroupId_002, TestSize.Level1)
{
    std::vector<sptr<VolumeGroupInfo>> infos;
    std::string networkId = "";
    int32_t ret = AudioPolicyManager::GetInstance().GetVolumeGroupInfos(networkId, infos);
    EXPECT_EQ(SUCCESS, ret);
    EXPECT_EQ(infos.size(), 0);
    int32_t groupId = -1;
    ret = AudioPolicyManager::GetInstance().GetNetworkIdByGroupId(groupId, networkId);
    EXPECT_EQ(ERROR, ret);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable via illegal state
 * @tc.number: SetClientCallbacksEnable_001
 * @tc.desc  : Test SetClientCallbacksEnable interface. Set callback enable and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, SetClientCallbacksEnable_001, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_UNKNOWN, false);
    EXPECT_EQ(AUDIO_ERR, ret);
    ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_UNKNOWN, true);
    EXPECT_EQ(AUDIO_ERR, ret);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable via illegal state
 * @tc.number: SetClientCallbacksEnable_002
 * @tc.desc  : Test SetClientCallbacksEnable interface. Set callback enable and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, SetClientCallbacksEnable_002, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_MAX, false);
    EXPECT_EQ(AUDIO_ERR, ret);
    ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_MAX, true);
    EXPECT_EQ(AUDIO_ERR, ret);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable via legal state
 * @tc.number: SetClientCallbacksEnable_003
 * @tc.desc  : Test SetClientCallbacksEnable interface. Set callback enable and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, SetClientCallbacksEnable_003, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_FOCUS_INFO_CHANGE, false);
    EXPECT_EQ(AUDIO_OK, ret);
    ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_FOCUS_INFO_CHANGE, true);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable via legal state
 * @tc.number: SetClientCallbacksEnable_004
 * @tc.desc  : Test SetClientCallbacksEnable interface. Set callback enable and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, SetClientCallbacksEnable_004, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_RENDERER_STATE_CHANGE, false);
    EXPECT_EQ(AUDIO_OK, ret);
    ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_RENDERER_STATE_CHANGE, true);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
 * @tc.name  : Test SetClientCallbacksEnable via legal state
 * @tc.number: SetClientCallbacksEnable_005
 * @tc.desc  : Test SetClientCallbacksEnable interface. Set callback enable and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, SetClientCallbacksEnable_005, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_CAPTURER_STATE_CHANGE, false);
    EXPECT_EQ(AUDIO_OK, ret);
    ret = AudioPolicyManager::GetInstance().SetClientCallbacksEnable(CALLBACK_CAPTURER_STATE_CHANGE, true);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
 * @tc.name  : Test GetMaxRendererInstances via legal state
 * @tc.number: GetMaxRendererInstances_001
 * @tc.desc  : Test GetMaxRendererInstances interface.Get max renderer instances and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, GetMaxRendererInstances_001, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().GetMaxRendererInstances();
    EXPECT_EQ(MAX_RENDERER_INSTANCES, ret);
}

/**
 * @tc.name  : Test QueryEffectSceneMode via legal state
 * @tc.number: QueryEffectSceneMode_001
 * @tc.desc  : Test QueryEffectSceneMode interface.Query effect scene mode and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, QueryEffectSceneMode_001, TestSize.Level1)
{
    SupportedEffectConfig supportedEffectConfig;
    int32_t ret = AudioPolicyManager::GetInstance().QueryEffectSceneMode(supportedEffectConfig);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name  : Test SetPlaybackCapturerFilterInfos via legal state
 * @tc.number: SetPlaybackCapturerFilterInfos_001
 * @tc.desc  : Test SetPlaybackCapturerFilterInfos interface.set playback capturer filter infos and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, SetPlaybackCapturerFilterInfos_001, TestSize.Level1)
{
    AudioPlaybackCaptureConfig audioPlaybackCaptureConfig;
    int32_t ret = AudioPolicyManager::GetInstance().SetPlaybackCapturerFilterInfos(audioPlaybackCaptureConfig, 0);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name  : Test GetMaxAmplitude via legal state
 * @tc.number: GetMaxAmplitude_001
 * @tc.desc  : Test GetMaxAmplitude interface.Query effect scene mode and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, GetMaxAmplitude_001, TestSize.Level1)
{
    int32_t ret = AudioPolicyManager::GetInstance().GetMaxAmplitude(0);
    EXPECT_EQ(0, ret);
}

/**
 * @tc.name  : Test GetMinStreamVolume via legal state
 * @tc.number: GetMinStreamVolume_001
 * @tc.desc  : Test GetMinStreamVolume interface.get min stream volume and return ret.
 */
HWTEST(AudioPolicyExtUnitTest, GetMinStreamVolume_001, TestSize.Level1)
{
    float minStreamVolume = AudioPolicyManager::GetInstance().GetMinStreamVolume();
    float maxStreamVolume = AudioPolicyManager::GetInstance().GetMaxStreamVolume();
    EXPECT_LT(minStreamVolume, maxStreamVolume);
}

} // namespace AudioStandard
} // namespace OHOS