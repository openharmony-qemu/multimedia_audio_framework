/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioEffectChainManagerSecondUnitTest"
#endif

#include <gtest/gtest.h>

#include "audio_effect.h"
#include "audio_effect_log.h"
#include "audio_effect_chain_manager.h"
#include "audio_effect_rotation.h"
#include "audio_errors.h"
#include "audio_effect_chain.h"
#include "parameter.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t INFOCHANNELS = 2;
constexpr uint64_t INFOCHANNELLAYOUT = 0x3;
constexpr int32_t DEFAULT_DSP_STREAMUSAGE = 1;
constexpr int32_t DEFAULT_STREAM_OR_VOLUME_TYPE = 1;
const std::string ENHANCED_ROUTING_PARA = "const.multimedia.audio.sys_multidevice_capability.enable";

vector<EffectChain> SECOND_DEFAULT_EFFECT_CHAINS = {
    {"EFFECTCHAIN_SPK_MUSIC", {"apply1", "apply2", "apply3"}, ""},
    {"EFFECTCHAIN_BT_MUSIC", {}, ""}
};

EffectChainManagerParam SECOND_DEFAULT_EFFECT_CHAIN_MANAGER_PARAM{
    3,
    "SCENE_DEFAULT",
    {},
    {{"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"},
        {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_BLUETOOTH_A2DP", "EFFECTCHAIN_BT_MUSIC"}},
    {{"effect1", "property1"}, {"effect4", "property5"}, {"effect1", "property4"}}
};

vector<shared_ptr<AudioEffectLibEntry>> SECOND_DEFAULT_EFFECT_LIBRARY_LIST = {};

SessionEffectInfo SECOND_DEFAULT_INFO = {
    "EFFECT_DEFAULT",
    "SCENE_MOVIE",
    INFOCHANNELS,
    INFOCHANNELLAYOUT,
    DEFAULT_DSP_STREAMUSAGE,
    DEFAULT_STREAM_OR_VOLUME_TYPE,
};
}

class AudioEffectChainManagerSecondUnitTest : public testing::Test {
public:
    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test cases
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void);
    // TearDown: Called after each test cases
    void TearDown(void);
};

void AudioEffectChainManagerSecondUnitTest::SetUpTestCase(void) {}
void AudioEffectChainManagerSecondUnitTest::TearDownTestCase(void) {}
void AudioEffectChainManagerSecondUnitTest::SetUp(void) {}
void AudioEffectChainManagerSecondUnitTest::TearDown(void) {}

/**
* @tc.name   : Test HasRunningSessionForEffectChain enhanced routing key
* @tc.number : HasRunningSessionForEffectChain_002
* @tc.desc   : Enhanced routing session matches the chain by its route key
*              DEVICE_TYPE_&_SCENE_TYPE_&_RENDER_ID instead of the legacy key.
*/
HWTEST(AudioEffectChainManagerSecondUnitTest, HasRunningSessionForEffectChain_002, TestSize.Level1)
{
    SetParameter(ENHANCED_ROUTING_PARA.c_str(), "true");
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(SECOND_DEFAULT_EFFECT_CHAINS,
        SECOND_DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, SECOND_DEFAULT_EFFECT_LIBRARY_LIST);
    auto audioEffectChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_MOVIE", true);

    SessionEffectInfo info = SECOND_DEFAULT_INFO;
    info.deviceType = DEVICE_TYPE_SPEAKER;
    info.renderId = 7;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["111"] = info;
    // chain is registered under the enhanced routing key
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["DEVICE_TYPE_SPEAKER_&_SCENE_MOVIE_&_7"] =
        audioEffectChain;

    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->HasRunningSessionForEffectChain(audioEffectChain));

    // legacy key does not match any session under enhanced routing
    auto otherChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_GAME", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["SCENE_GAME_&_DEVICE_TYPE_SPEAKER"] =
        otherChain;
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->HasRunningSessionForEffectChain(otherChain));

    AudioEffectChainManager::GetInstance()->ResetInfo();
    SetParameter(ENHANCED_ROUTING_PARA.c_str(), "false");
}

/**
* @tc.name   : Test HasRunningSessionForEffectChain enhanced routing with unmapped device
* @tc.number : HasRunningSessionForEffectChain_003
* @tc.desc   : Session device type missing from the supported map yields empty device name;
*              chain keyed by that empty name still matches.
*/
HWTEST(AudioEffectChainManagerSecondUnitTest, HasRunningSessionForEffectChain_003, TestSize.Level1)
{
    SetParameter(ENHANCED_ROUTING_PARA.c_str(), "true");
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(SECOND_DEFAULT_EFFECT_CHAINS,
        SECOND_DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, SECOND_DEFAULT_EFFECT_LIBRARY_LIST);
    auto audioEffectChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_MOVIE", true);

    SessionEffectInfo info = SECOND_DEFAULT_INFO;
    info.deviceType = DEVICE_TYPE_INVALID;
    info.renderId = 9;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["111"] = info;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["DEVICE_TYPE_INVALID_&_SCENE_MOVIE_&_9"] =
        audioEffectChain;

    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->HasRunningSessionForEffectChain(audioEffectChain));

    auto otherChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_GAME", true);
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->HasRunningSessionForEffectChain(otherChain));

    AudioEffectChainManager::GetInstance()->ResetInfo();
    SetParameter(ENHANCED_ROUTING_PARA.c_str(), "false");
}

/**
* @tc.name   : Test SetSpatializationEnabledToChains with enhanced routing session
* @tc.number : SetSpatializationEnabledToChains_003
* @tc.desc   : Chain whose route key matches a running session uses fading update,
*              and the log path with key is exercised.
*/
HWTEST(AudioEffectChainManagerSecondUnitTest, SetSpatializationEnabledToChains_003, TestSize.Level1)
{
    SetParameter(ENHANCED_ROUTING_PARA.c_str(), "true");
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(SECOND_DEFAULT_EFFECT_CHAINS,
        SECOND_DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, SECOND_DEFAULT_EFFECT_LIBRARY_LIST);
    std::string routeKey = "DEVICE_TYPE_SPEAKER_&_SCENE_MOVIE_&_7";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        std::make_shared<AudioEffectChain>("SCENE_MOVIE", headTracker);

    SessionEffectInfo info = SECOND_DEFAULT_INFO;
    info.deviceType = DEVICE_TYPE_SPEAKER;
    info.renderId = 7;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["111"] = info;
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[routeKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;

    // running session on the chain takes the fading branch
    AudioEffectChainManager::GetInstance()->SetSpatializationEnabledToChains();
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->spatializationEnabled_,
        audioEffectChain->spatializationEnabledFading_);

    // remove session info so the chain falls to the non-fading branch
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->SetSpatializationEnabledToChains();
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->spatializationEnabled_,
        audioEffectChain->spatializationEnabled_);

    AudioEffectChainManager::GetInstance()->ResetInfo();
    SetParameter(ENHANCED_ROUTING_PARA.c_str(), "false");
}

/**
* @tc.name   : Test SetSpatializationEnabledToChains legacy path with new logs
* @tc.number : SetSpatializationEnabledToChains_004
* @tc.desc   : Without enhanced routing both branches (fading / non-fading) are exercised
*              with the key-tagged log statements.
*/
HWTEST(AudioEffectChainManagerSecondUnitTest, SetSpatializationEnabledToChains_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(SECOND_DEFAULT_EFFECT_CHAINS,
        SECOND_DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, SECOND_DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        std::make_shared<AudioEffectChain>("SCENE_MOVIE", headTracker);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["111"] = SECOND_DEFAULT_INFO;
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;

    // running session: fading branch
    AudioEffectChainManager::GetInstance()->SetSpatializationEnabledToChains();
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->spatializationEnabled_,
        audioEffectChain->spatializationEnabledFading_);

    // no running session: direct branch
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->SetSpatializationEnabledToChains();
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->spatializationEnabled_,
        audioEffectChain->spatializationEnabled_);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}
} // namespace AudioStandard
} // namespace OHOS
