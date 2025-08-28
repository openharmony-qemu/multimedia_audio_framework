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

#ifndef LOG_TAG
#define LOG_TAG "AudioEffectChainManagerUnitTest"
#endif

#include "audio_effect_chain_manager_unit_test.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_effect.h"
#include "audio_effect_log.h"
#include "audio_effect_chain_manager.h"
#include "audio_effect_rotation.h"
#include "audio_errors.h"
#include "audio_effect_chain.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t INFOCHANNELS = 2;
constexpr uint64_t INFOCHANNELLAYOUT = 0x3;

vector<EffectChain> DEFAULT_EFFECT_CHAINS = {
    {"EFFECTCHAIN_SPK_MUSIC", {"apply1", "apply2", "apply3"}, ""},
    {"EFFECTCHAIN_BT_MUSIC", {}, ""}
};

EffectChainManagerParam DEFAULT_EFFECT_CHAIN_MANAGER_PARAM{
    3,
    "SCENE_DEFAULT",
    {},
    {{"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"},
        {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_BLUETOOTH_A2DP", "EFFECTCHAIN_BT_MUSIC"}},
    {{"effect1", "property1"}, {"effect4", "property5"}, {"effect1", "property4"}}
};

vector<shared_ptr<AudioEffectLibEntry>> DEFAULT_EFFECT_LIBRARY_LIST = {};

SessionEffectInfo DEFAULT_INFO = {
    "EFFECT_DEFAULT",
    "SCENE_MOVIE",
    INFOCHANNELS,
    INFOCHANNELLAYOUT,
};
}

void AudioEffectChainManagerUnitTest::SetUpTestCase(void) {}
void AudioEffectChainManagerUnitTest::TearDownTestCase(void) {}
void AudioEffectChainManagerUnitTest::SetUp(void) {}
void AudioEffectChainManagerUnitTest::TearDown(void) {}

/**
* @tc.name   : Test CreateAudioEffectChainDynamic API
* @tc.number : CreateAudioEffectChainDynamic_001
* @tc.desc   : Test CreateAudioEffectChainDynamic interface(using empty use case).
*              Test GetDeviceTypeName interface and SetAudioEffectChainDynamic interface simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChainDynamic_001, TestSize.Level1)
{
    string sceneType = "";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    int32_t result = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CreateAudioEffectChainDynamic API
* @tc.number : CreateAudioEffectChainDynamic_002
* @tc.desc   : Test CreateAudioEffectChainDynamic interface(using abnormal use case).
*              Test GetDeviceTypeName interface and SetAudioEffectChainDynamic interface simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChainDynamic_002, TestSize.Level1)
{
    string sceneType = "123";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t result = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CreateAudioEffectChainDynamic API
* @tc.number : CreateAudioEffectChainDynamic_003
* @tc.desc   : Test CreateAudioEffectChainDynamic interface(using correct use case).
*              Test GetDeviceTypeName interface and SetAudioEffectChainDynamic interface simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChainDynamic_003, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t result =  AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CreateAudioEffectChainDynamic API
* @tc.number : CreateAudioEffectChainDynamic_004
* @tc.desc   : Test CreateAudioEffectChainDynamic interface(using correct use case).
*              Test GetDeviceTypeName interface and SetAudioEffectChainDynamic interface simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChainDynamic_004, TestSize.Level1)
{
    string sceneType = "COMMON_SCENE_TYPE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    std::string sceneTypeAndDeviceKey = "COMMON_SCENE_TYPE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result =  AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 3;
    result =  AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 0;
    result =  AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CreateAudioEffectChainDynamic API
* @tc.number : CreateAudioEffectChainDynamic_005
* @tc.desc   : Test CreateAudioEffectChainDynamic interface(using correct use case).
*              Test GetDeviceTypeName interface and SetAudioEffectChainDynamic interface simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChainDynamic_005, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    SessionEffectInfo info = {
        "EFFECT_DEFAULT",
        "SCENE_MOVIE",
        INFOCHANNELS,
        INFOCHANNELLAYOUT,
        10,
    };

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->maxSessionID_ = 123456;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["123456"] = info;
    int32_t result =  AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndAddSessionID API
* @tc.number : CheckAndAddSessionID_001
* @tc.desc   : Test CheckAndAddSessionID interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndAddSessionID_001, TestSize.Level1)
{
    string sessionID = "123456";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    bool result = AudioEffectChainManager::GetInstance()->CheckAndAddSessionID(sessionID);
    EXPECT_EQ(true, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndAddSessionID API
* @tc.number : CheckAndAddSessionID_002
* @tc.desc   : Test CheckAndAddSessionID interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndAddSessionID_002, TestSize.Level1)
{
    string sessionID = "123456";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->sessionIDSet_.insert("123456");
    AudioEffectChainManager::GetInstance()->sessionIDSet_.insert("abcdef");
    bool result = AudioEffectChainManager::GetInstance()->CheckAndAddSessionID(sessionID);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndRemoveSessionID API
* @tc.number : CheckAndRemoveSessionID_001
* @tc.desc   : Test CheckAndRemoveSessionID interface(using incorrect use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndRemoveSessionID_001, TestSize.Level1)
{
    string sessionID = "123456";
    AudioEffectChainManager::GetInstance()->CheckAndAddSessionID(sessionID);

    bool result = AudioEffectChainManager::GetInstance()->CheckAndRemoveSessionID("123");
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndRemoveSessionID API
* @tc.number : CheckAndRemoveSessionID_002
* @tc.desc   : Test CheckAndRemoveSessionID interface(using correct use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndRemoveSessionID_002, TestSize.Level1)
{
    string sessionID = "123456";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CheckAndAddSessionID(sessionID);

    bool result = AudioEffectChainManager::GetInstance()->CheckAndRemoveSessionID(sessionID);
    EXPECT_EQ(true, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndRemoveSessionID API
* @tc.number : CheckAndRemoveSessionID_003
* @tc.desc   : Test CheckAndRemoveSessionID interface(without using CheckAndAddSessionID interface).
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndRemoveSessionID_003, TestSize.Level1)
{
    string sessionID = "123456";

    bool result = AudioEffectChainManager::GetInstance()->CheckAndRemoveSessionID(sessionID);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReleaseAudioEffectChainDynamic API
* @tc.number : ReleaseAudioEffectChainDynamic_001
* @tc.desc   : Test ReleaseAudioEffectChainDynamic interface(using empty use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamic_001, TestSize.Level1)
{
    string sceneType = "";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    int32_t result =  AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReleaseAudioEffectChainDynamic API
* @tc.number : ReleaseAudioEffectChainDynamic_002
* @tc.desc   : Test ReleaseAudioEffectChainDynamic interface(using incorrect use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamic_002, TestSize.Level1)
{
    string sceneType = "123";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    int32_t result =  AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReleaseAudioEffectChainDynamic API
* @tc.number : ReleaseAudioEffectChainDynamic_003
* @tc.desc   : Test ReleaseAudioEffectChainDynamic interface(using correct use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamic_003, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    int32_t result =  AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReleaseAudioEffectChainDynamic API
* @tc.number : ReleaseAudioEffectChainDynamic_004
* @tc.desc   : Test ReleaseAudioEffectChainDynamic interface(using correct use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamic_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->UpdateDefaultAudioEffect();
    std::string sceneType = "SCENE_DEFAULT";
    const char *sceneType1 = "SCENE_DEFAULT";
    std::string sceneTypeAndDeviceKey1 = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
        std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType1, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey1] = audioEffectChain;
    uint32_t ret = AudioEffectChainManager::GetInstance()->GetSceneTypeToChainCount(sceneType);
    EXPECT_EQ(ret, 0);
    int32_t result =  AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    sceneType = "SCENE_MOVIE";
    result =  AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test ExistAudioEffectChain API
* @tc.number : ExistAudioEffectChain_001
* @tc.desc   : Test ExistAudioEffectChain interface(without using InitAudioEffectChainManager).
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChain_001, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    string effectMode = "EFFECT_DEFAULT";

    AudioEffectChainManager::GetInstance()->isInitialized_ = false;
    AudioEffectChainManager::GetInstance()->initializedLogFlag_ = true;
    bool result =  AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChain API
* @tc.number : ExistAudioEffectChain_002
* @tc.desc   : Test ExistAudioEffectChain interface(using correct use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChain_002, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    string effectMode = "EFFECT_DEFAULT";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    bool result =  AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode);
    EXPECT_EQ(false, result);  // Use 'false' as the criterion for judgment because of the empty effect chain.
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChain API
* @tc.number : ExistAudioEffectChain_003
* @tc.desc   : Test ExistAudioEffectChain interface(using empty use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChain_003, TestSize.Level1)
{
    string sceneType = "";
    string effectMode = "";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    bool result =  AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChain API
* @tc.number : ExistAudioEffectChain_004
* @tc.desc   : Test ExistAudioEffectChain interface(using incorrect use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChain_004, TestSize.Level1)
{
    string sceneType = "123";
    string effectMode = "123";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    bool result =  AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChain API
* @tc.number : ExistAudioEffectChain_005
* @tc.desc   : Test ExistAudioEffectChain interface(without using CreateAudioEffectChainDynamic).
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChain_005, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    string effectMode = "EFFECT_DEFAULT";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    bool result =  AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChain API
* @tc.number : ExistAudioEffectChain_006
* @tc.desc   : Test ExistAudioEffectChain interface(without using CreateAudioEffectChainDynamic).
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChain_006, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    string effectMode = "EFFECT_DEFAULT";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    bool result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChain API
* @tc.number : ExistAudioEffectChain_007
* @tc.desc   : Test ExistAudioEffectChain interface(without using CreateAudioEffectChainDynamic).
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChain_007, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    string effectMode = "EFFECT_DEFAULT";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = true;

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    bool result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChain API
* @tc.number : ExistAudioEffectChain_008
* @tc.desc   : Test ExistAudioEffectChain interface(without using CreateAudioEffectChainDynamic).
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChain_008, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    string effectMode = "EFFECT_DEFAULT";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = false;

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    bool result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChain API
* @tc.number : ExistAudioEffectChain_009
* @tc.desc   : Test ExistAudioEffectChain interface(without using CreateAudioEffectChainDynamic).
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChain_009, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    string effectMode = "EFFECT_DEFAULT";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    bool result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChain API
* @tc.number : ExistAudioEffectChain_010
* @tc.desc   : Test ExistAudioEffectChain interface(without using CreateAudioEffectChainDynamic).
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChain_010, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    string effectMode = "EFFECT_DEFAULT";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    bool result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ApplyAudioEffectChain API
* @tc.number : ApplyAudioEffectChain_001
* @tc.desc   : Test ApplyAudioEffectChain interface(using empty use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, ApplyAudioEffectChain_001, TestSize.Level1)
{
    float* bufIn;
    float* bufOut;
    vector<float> bufInVector;
    vector<float> bufOutVector;
    bufInVector.resize(10000, 0);
    bufOutVector.resize(10000, 0);
    bufIn = bufInVector.data();
    bufOut = bufOutVector.data();
    int numChans = 2;
    int frameLen = 960;
    uint32_t outChannels = INFOCHANNELS;
    uint64_t outChannelLayout = INFOCHANNELLAYOUT;
    auto eBufferAttr = make_unique<EffectBufferAttr>(bufIn, bufOut, numChans, frameLen, outChannels, outChannelLayout);
    string sceneType = "";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(sceneType, eBufferAttr);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ApplyAudioEffectChain API
* @tc.number : ApplyAudioEffectChain_002
* @tc.desc   : Test ApplyAudioEffectChain interface(using correct use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, ApplyAudioEffectChain_002, TestSize.Level1)
{
    float* bufIn;
    float* bufOut;
    vector<float> bufInVector;
    vector<float> bufOutVector;
    bufInVector.resize(10000, 0);
    bufOutVector.resize(10000, 0);
    bufIn = bufInVector.data();
    bufOut = bufOutVector.data();
    int numChans = 2;
    int frameLen = 960;
    uint32_t outChannels = INFOCHANNELS;
    uint64_t outChannelLayout = INFOCHANNELLAYOUT;
    auto eBufferAttr = make_unique<EffectBufferAttr>(bufIn, bufOut, numChans, frameLen, outChannels, outChannelLayout);
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(sceneType, eBufferAttr);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ApplyAudioEffectChain API
* @tc.number : ApplyAudioEffectChain_003
* @tc.desc   : Test ApplyAudioEffectChain interface(using abnormal use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, ApplyAudioEffectChain_003, TestSize.Level1)
{
    float* bufIn;
    float* bufOut;
    vector<float> bufInVector;
    vector<float> bufOutVector;
    bufInVector.resize(10000, 0);
    bufOutVector.resize(10000, 0);
    bufIn = bufInVector.data();
    bufOut = bufOutVector.data();
    int numChans = 2;
    int frameLen = 960;
    uint32_t outChannels = INFOCHANNELS;
    uint64_t outChannelLayout = INFOCHANNELLAYOUT;
    auto eBufferAttr = make_unique<EffectBufferAttr>(bufIn, bufOut, numChans, frameLen, outChannels, outChannelLayout);
    string sceneType = "123";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(sceneType, eBufferAttr);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ApplyAudioEffectChain API
* @tc.number : ApplyAudioEffectChain_004
* @tc.desc   : Test ApplyAudioEffectChain interface(without using CreateAudioEffectChainDynamic interface).
*/
HWTEST(AudioEffectChainManagerUnitTest, ApplyAudioEffectChain_004, TestSize.Level1)
{
    float* bufIn;
    float* bufOut;
    vector<float> bufInVector;
    vector<float> bufOutVector;
    bufInVector.resize(10000, 0);
    bufOutVector.resize(10000, 0);
    bufIn = bufInVector.data();
    bufOut = bufOutVector.data();
    int numChans = 2;
    int frameLen = 960;
    uint32_t outChannels = INFOCHANNELS;
    uint64_t outChannelLayout = INFOCHANNELLAYOUT;
    auto eBufferAttr = make_unique<EffectBufferAttr>(bufIn, bufOut, numChans, frameLen, outChannels, outChannelLayout);
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(sceneType, eBufferAttr);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetOutputDeviceSink API
* @tc.number : SetOutputDeviceSink_001
* @tc.desc   : Test SetOutputDeviceSink interface(using correct use case),
*              test SetSpkOffloadState interface simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetOutputDeviceSink_001, TestSize.Level1)
{
    int32_t device = 2;
    string sinkName = "Speaker";
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->SetOutputDeviceSink(device, sinkName);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetOutputDeviceSink API
* @tc.number : SetOutputDeviceSink_002
* @tc.desc   : Test SetOutputDeviceSink interface(using empty use case),
*              test SetSpkOffloadState interface simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetOutputDeviceSink_002, TestSize.Level1)
{
    int32_t device = 2;
    string sinkName = "";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->SetOutputDeviceSink(device, sinkName);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetOutputDeviceSink API
* @tc.number : SetOutputDeviceSink_003
* @tc.desc   : Test SetOutputDeviceSink interface(using abnormal use case),
*              test SetSpkOffloadState interface simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetOutputDeviceSink_003, TestSize.Level1)
{
    int32_t device = 2;
    string sinkName = "123";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->SetOutputDeviceSink(device, sinkName);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetOffloadEnabled API
* @tc.number : GetOffloadEnabled_001
* @tc.desc   : Test GetOffloadEnabled interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetOffloadEnabled_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    bool result = AudioEffectChainManager::GetInstance()->GetOffloadEnabled();
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetOffloadEnabled API
* @tc.number : GetOffloadEnabled_002
* @tc.desc   : Test GetOffloadEnabled interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetOffloadEnabled_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    bool result = AudioEffectChainManager::GetInstance()->GetOffloadEnabled();
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateMultichannelConfig API
* @tc.number : UpdateMultichannelConfig_001
* @tc.desc   : Test UpdateMultichannelConfig interface(using correct use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateMultichannelConfig_001, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateMultichannelConfig(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateMultichannelConfig API
* @tc.number : UpdateMultichannelConfig_002
* @tc.desc   : Test UpdateMultichannelConfig interface(using abnormal use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateMultichannelConfig_002, TestSize.Level1)
{
    string sceneType = "123";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateMultichannelConfig(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateMultichannelConfig API
* @tc.number : UpdateMultichannelConfig_003
* @tc.desc   : Test UpdateMultichannelConfig interface(using empty use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateMultichannelConfig_003, TestSize.Level1)
{
    string sceneType = "";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateMultichannelConfig(sceneType);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateMultichannelConfig API
* @tc.number : UpdateMultichannelConfig_004
* @tc.desc   : Test UpdateMultichannelConfig interface(without using CreateAudioEffectChainDynamic interface).
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateMultichannelConfig_004, TestSize.Level1)
{
    string sceneType = "";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateMultichannelConfig(sceneType);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test InitAudioEffectChainDynamic API
* @tc.number : InitAudioEffectChainDynamic_001
* @tc.desc   : Test InitAudioEffectChainDynamic interface(without using InitAudioEffectChainManager interface).
*/
HWTEST(AudioEffectChainManagerUnitTest, InitAudioEffectChainDynamic_001, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";

    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test InitAudioEffectChainDynamic API
* @tc.number : InitAudioEffectChainDynamic_002
* @tc.desc   : Test InitAudioEffectChainDynamic interface(using correct use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, InitAudioEffectChainDynamic_002, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test InitAudioEffectChainDynamic API
* @tc.number : InitAudioEffectChainDynamic_003
* @tc.desc   : Test InitAudioEffectChainDynamic interface(using incorrect use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, InitAudioEffectChainDynamic_003, TestSize.Level1)
{
    string sceneType = "123";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test InitAudioEffectChainDynamic API
* @tc.number : InitAudioEffectChainDynamic_004
* @tc.desc   : Test InitAudioEffectChainDynamic interface(using empty use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, InitAudioEffectChainDynamic_004, TestSize.Level1)
{
    string sceneType = "";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test InitAudioEffectChainDynamic API
* @tc.number : InitAudioEffectChainDynamic_005
* @tc.desc   : Test InitAudioEffectChainDynamic interface(Using audioEffectChain = nullptr use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, InitAudioEffectChainDynamic_005, TestSize.Level1)
{
    string sceneType = "SCENE_MUSIC";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain = nullptr;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateSpatializationState API
* @tc.number : UpdateSpatializationState_001
* @tc.desc   : Test UpdateSpatializationState interface.Test UpdateSensorState,
*              DeleteAllChains and RecoverAllChains interface simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSpatializationState_001, TestSize.Level1)
{
    AudioSpatializationState spatializationState = {false, false};

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->btOffloadSupported_ = false;
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateSpatializationState(spatializationState);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateSpatializationState API
* @tc.number : UpdateSpatializationState_002
* @tc.desc   : Test UpdateSpatializationState interface.Test UpdateSensorState,
*              DeleteAllChains and RecoverAllChains interface simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSpatializationState_002, TestSize.Level1)
{
    AudioSpatializationState spatializationState = {true, true};

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->btOffloadSupported_ = false;
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateSpatializationState(spatializationState);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateSpatializationState API
* @tc.number : UpdateSpatializationState_003
* @tc.desc   : Test UpdateSpatializationState interface.Test UpdateSensorState,
*              DeleteAllChains and RecoverAllChains interface simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSpatializationState_003, TestSize.Level1)
{
    AudioSpatializationState spatializationState = {true, false};

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->headTrackingEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = true;
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateSpatializationState(spatializationState);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateSpatializationState API
* @tc.number : UpdateSpatializationState_004
* @tc.desc   : Test UpdateSpatializationState interface.Test UpdateSensorState,
*              DeleteAllChains and RecoverAllChains interface simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSpatializationState_004, TestSize.Level1)
{
    AudioSpatializationState spatializationState(false, true);

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->headTrackingEnabled_ = true;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateSpatializationState(spatializationState);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateSensorState API
* @tc.number : UpdateSpatializationState_001
* @tc.desc   : Test UpdateSensorState interface
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSensorState_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->headTrackingEnabled_ = true;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = true;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = true;
    AudioEffectChainManager::GetInstance()->UpdateSensorState();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateSensorState API
* @tc.number : UpdateSpatializationState_002
* @tc.desc   : Test UpdateSensorState interface
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSensorState_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->headTrackingEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = true;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->UpdateSensorState();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateSensorState API
* @tc.number : UpdateSpatializationState_002
* @tc.desc   : Test UpdateSensorState interface
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSensorState_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->headTrackingEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->UpdateSensorState();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetHdiParam API
* @tc.number : SetHdiParam_001
* @tc.desc   : Test SetHdiParam interface(without using InitAudioEffectChainManager interface).
*/
HWTEST(AudioEffectChainManagerUnitTest, SetHdiParam_001, TestSize.Level1)
{
    AudioEffectScene sceneType = SCENE_MUSIC;

    int32_t result = AudioEffectChainManager::GetInstance()->SetHdiParam(sceneType);
    EXPECT_TRUE(result == SUCCESS || result == ERROR);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetHdiParam API
* @tc.number : SetHdiParam_002
* @tc.desc   : Test SetHdiParam interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetHdiParam_002, TestSize.Level1)
{
    AudioEffectScene sceneType = SCENE_MUSIC;

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t result = AudioEffectChainManager::GetInstance()->SetHdiParam(sceneType);
    EXPECT_TRUE(result == SUCCESS || result == ERROR);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetHdiParam API
* @tc.number : SetHdiParam_003
* @tc.desc   : Test SetHdiParam interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetHdiParam_003, TestSize.Level1)
{
    AudioEffectScene sceneType = SCENE_OTHERS;

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t result = AudioEffectChainManager::GetInstance()->SetHdiParam(sceneType);
    EXPECT_TRUE(result == SUCCESS || result == ERROR);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SessionInfoMapAdd API
* @tc.number : SessionInfoMapAdd_001
* @tc.desc   : Test SessionInfoMapAdd interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SessionInfoMapAdd_001, TestSize.Level1)
{
    string sessionID = "123456";

    int32_t result = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SessionInfoMapAdd API
* @tc.number : SessionInfoMapAdd_002
* @tc.desc   : Test SessionInfoMapAdd interface(using empty use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, SessionInfoMapAdd_002, TestSize.Level1)
{
    string sessionID = "";

    int32_t result = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SessionInfoMapDelete API
* @tc.number : SessionInfoMapDelete_001
* @tc.desc   : Test SessionInfoMapDelete interface(without using SessionInfoMapAdd interface).
*/
HWTEST(AudioEffectChainManagerUnitTest, SessionInfoMapDelete_001, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    string sessionID = "123456";

    int32_t result = AudioEffectChainManager::GetInstance()->SessionInfoMapDelete(sceneType, sessionID);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SessionInfoMapDelete API
* @tc.number : SessionInfoMapDelete_002
* @tc.desc   : Test SessionInfoMapDelete interface(using correct use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, SessionInfoMapDelete_002, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    string sessionID = "123456";

    int32_t addRes = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    EXPECT_EQ(SUCCESS, addRes);

    int32_t result = AudioEffectChainManager::GetInstance()->SessionInfoMapDelete(sceneType, sessionID);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SessionInfoMapDelete API
* @tc.number : SessionInfoMapDelete_003
* @tc.desc   : Test SessionInfoMapDelete interface(using incorrect use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, SessionInfoMapDelete_003, TestSize.Level1)
{
    string sceneType = "123";
    string sessionID = "123456";

    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    int32_t result = AudioEffectChainManager::GetInstance()->SessionInfoMapDelete(sceneType, sessionID);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SessionInfoMapDelete API
* @tc.number : SessionInfoMapDelete_004
* @tc.desc   : Test SessionInfoMapDelete interface(using empty use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, SessionInfoMapDelete_004, TestSize.Level1)
{
    string sceneType = "";
    string sessionID = "";

    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    int32_t result = AudioEffectChainManager::GetInstance()->SessionInfoMapDelete(sceneType, sessionID);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReturnEffectChannelInfo API
* @tc.number : ReturnEffectChannelInfo_001
* @tc.desc   : Test ReturnEffectChannelInfo interface(without using SessionInfoMapAdd interface).
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnEffectChannelInfo_001, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    uint32_t channels = 2;
    uint64_t channelLayout = 0x3;

    int32_t result = AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfo(sceneType, channels,
        channelLayout);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReturnEffectChannelInfo API
* @tc.number : ReturnEffectChannelInfo_002
* @tc.desc   : Test ReturnEffectChannelInfo interface(using correct use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnEffectChannelInfo_002, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    uint32_t channels = 2;
    uint64_t channelLayout = 0x3;
    string sessionID = "123456";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);

    int32_t addRes = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    EXPECT_EQ(SUCCESS, addRes);

    int32_t result = AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfo(sceneType, channels,
        channelLayout);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReturnEffectChannelInfo API
* @tc.number : ReturnEffectChannelInfo_003
* @tc.desc   : Test ReturnEffectChannelInfo interface(using incorrect use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnEffectChannelInfo_003, TestSize.Level1)
{
    string sceneType = "123";
    uint32_t channels = 2;
    uint64_t channelLayout = 0x3;
    string sessionID = "123456";

    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    int32_t result = AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfo(sceneType, channels,
        channelLayout);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReturnEffectChannelInfo API
* @tc.number : ReturnEffectChannelInfo_004
* @tc.desc   : Test ReturnEffectChannelInfo interface(using empty use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnEffectChannelInfo_004, TestSize.Level1)
{
    string sceneType = "";
    uint32_t channels = 2;
    uint64_t channelLayout = 0x3;
    string sessionID = "123456";

    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    int32_t result = AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfo(sceneType, channels,
        channelLayout);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReturnMultiChannelInfo API
* @tc.number : ReturnMultiChannelInfo_001
* @tc.desc   : Test ReturnMultiChannelInfo interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnMultiChannelInfo_001, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    uint32_t channels = 2;
    uint64_t channelLayout = 0x3;
    string sessionID = "123456";

    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    int32_t result = AudioEffectChainManager::GetInstance()->ReturnMultiChannelInfo(&channels, &channelLayout);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReturnMultiChannelInfo API
* @tc.number : ReturnMultiChannelInfo_002
* @tc.desc   : Test ReturnMultiChannelInfo interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnMultiChannelInfo_002, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    uint32_t channels = 3;
    uint64_t channelLayout = 0x3;
    string sessionID = "123456";

    const char *sceneType2 = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType2, true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType2);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    result = AudioEffectChainManager::GetInstance()->ReturnMultiChannelInfo(&channels, &channelLayout);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReturnMultiChannelInfo API
* @tc.number : ReturnMultiChannelInfo_003
* @tc.desc   : Test ReturnMultiChannelInfo interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnMultiChannelInfo_003, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    uint32_t channels = 3;
    uint64_t channelLayout = 0x3;
    string sessionID = "123456";

    AudioEffectChainManager::GetInstance()->isInitialized_ = false;
    AudioEffectChainManager::GetInstance()->initializedLogFlag_ = false;
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    int32_t result = AudioEffectChainManager::GetInstance()->ReturnMultiChannelInfo(&channels, &channelLayout);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateParamExtra API
* @tc.number : UpdateParamExtra_001
* @tc.desc   : Test UpdateParamExtra interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateParamExtra_001, TestSize.Level1)
{
    const std::string mainkey = "audio_effect";
    const std::string subkey = "update_audio_effect_type";
    const std::string extraSceneType = "0";

    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, extraSceneType);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(ERROR, result);

    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, extraSceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test EffectRotationUpdate API
* @tc.number : EffectRotationUpdate_001
* @tc.desc   : Test EffectRotationUpdate interface.
*              Test EffectDspRotationUpdate and EffectApRotationUpdate simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, EffectRotationUpdate_001, TestSize.Level1)
{
    uint32_t rotationState = 0;

    int32_t result = AudioEffectChainManager::GetInstance()->EffectRotationUpdate(rotationState);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test EffectRotationUpdate API
* @tc.number : EffectRotationUpdate_002
* @tc.desc   : Test EffectRotationUpdate interface.
*              Test EffectDspRotationUpdate and EffectApRotationUpdate simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, EffectRotationUpdate_002, TestSize.Level1)
{
    uint32_t rotationState = 1;

    int32_t result = AudioEffectChainManager::GetInstance()->EffectRotationUpdate(rotationState);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test EffectVolumeUpdate API
* @tc.number : EffectVolumeUpdate_001
* @tc.desc   : Test EffectVolumeUpdate interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, EffectVolumeUpdate_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = AudioEffectVolume::GetInstance();

    const std::string sessionIDString = "12345";
    const float streamVolume = 0.5;
    audioEffectVolume->SetStreamVolume(sessionIDString, streamVolume);
    int32_t ret = AudioEffectChainManager::GetInstance()->EffectVolumeUpdate();
    EXPECT_EQ(ret, SUCCESS);

    const std::string sessionIDString1 = "123456";
    audioEffectVolume->SetStreamVolume(sessionIDString1, streamVolume);
    ret = AudioEffectChainManager::GetInstance()->DeleteStreamVolume(sessionIDString1);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name   : Test GetLatency API
* @tc.number : GetLatency_001
* @tc.desc   : Test GetLatency interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetLatency_001, TestSize.Level1)
{
    string sessionID = "123456" ;

    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    uint32_t result = AudioEffectChainManager::GetInstance()->GetLatency(sessionID);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetLatency API
* @tc.number : GetLatency_002
* @tc.desc   : Test GetLatency interface(using empty use case).
*/
HWTEST(AudioEffectChainManagerUnitTest, GetLatency_002, TestSize.Level1)
{
    string sessionID = "" ;

    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    uint32_t result = AudioEffectChainManager::GetInstance()->GetLatency(sessionID);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetLatency API
* @tc.number : GetLatency_003
* @tc.desc   : Test GetLatency interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetLatency_003, TestSize.Level1)
{
    string sessionID = "123456" ;

    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;
    uint32_t result = AudioEffectChainManager::GetInstance()->GetLatency(sessionID);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetLatency API
* @tc.number : GetLatency_004
* @tc.desc   : Test GetLatency interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetLatency_004, TestSize.Level1)
{
    string sessionID = "123456" ;

    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = true;
    uint32_t result = AudioEffectChainManager::GetInstance()->GetLatency(sessionID);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetLatency API
* @tc.number : GetLatency_005
* @tc.desc   : Test GetLatency interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetLatency_005, TestSize.Level1)
{
    string sessionID = "123456" ;

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_[sessionID].sceneMode = "";
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    uint32_t result = AudioEffectChainManager::GetInstance()->GetLatency(sessionID);
    EXPECT_EQ(0, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetLatency API
* @tc.number : GetLatency_006
* @tc.desc   : Test GetLatency interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetLatency_006, TestSize.Level1)
{
    string sessionID = "123456" ;

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_[sessionID].sceneMode = "None";
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    uint32_t result = AudioEffectChainManager::GetInstance()->GetLatency(sessionID);
    EXPECT_EQ(0, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetLatency API
* @tc.number : GetLatency_007
* @tc.desc   : Test GetLatency interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetLatency_007, TestSize.Level1)
{
    string sessionID = "123456" ;

    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    uint32_t result = AudioEffectChainManager::GetInstance()->GetLatency(sessionID);
    EXPECT_EQ(0, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetLatency API
* @tc.number : GetLatency_008
* @tc.desc   : Test GetLatency interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetLatency_008, TestSize.Level1)
{
    string sessionID = "123456" ;

    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    uint32_t result = AudioEffectChainManager::GetInstance()->GetLatency(sessionID);
    EXPECT_EQ(0, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}


/**
* @tc.name   : Test SetSpatializationSceneType API
* @tc.number : SetSpatializationSceneType_001
* @tc.desc   : Test SetSpatializationSceneType interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetSpatializationSceneType_001, TestSize.Level1)
{
    AudioSpatializationSceneType spatializationSceneType = SPATIALIZATION_SCENE_TYPE_DEFAULT;

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    int32_t result = AudioEffectChainManager::GetInstance()->SetSpatializationSceneType(spatializationSceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetSpatializationSceneType API
* @tc.number : SetSpatializationSceneType_002
* @tc.desc   : Test SetSpatializationSceneType interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetSpatializationSceneType_002, TestSize.Level1)
{
    AudioSpatializationSceneType spatializationSceneType = SPATIALIZATION_SCENE_TYPE_DEFAULT;

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    int32_t result = AudioEffectChainManager::GetInstance()->SetSpatializationSceneType(spatializationSceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetSpkOffloadState API
* @tc.number : SetSpkOffloadState_001
* @tc.desc   : Test SetSpkOffloadState interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetSpkOffloadState_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;
    AudioEffectChainManager::GetInstance()->SetSpkOffloadState();

    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    bool result = AudioEffectChainManager::GetInstance()->GetOffloadEnabled();
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetSpkOffloadState API
* @tc.number : SetSpkOffloadState_002
* @tc.desc   : Test SetSpkOffloadState interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetSpkOffloadState_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_WIRED_HEADPHONES;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->SetSpkOffloadState();

    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    bool result = AudioEffectChainManager::GetInstance()->GetOffloadEnabled();
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetSpkOffloadState API
* @tc.number : SetSpkOffloadState_003
* @tc.desc   : Test SetSpkOffloadState interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetSpkOffloadState_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;
    AudioEffectChainManager::GetInstance()->SetSpkOffloadState();

    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    bool result = AudioEffectChainManager::GetInstance()->GetOffloadEnabled();
    EXPECT_EQ(true, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetSpkOffloadState API
* @tc.number : SetSpkOffloadState_004
* @tc.desc   : Test SetSpkOffloadState interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetSpkOffloadState_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    // use spkOffloadEnabled_ to differentiate platforms
    if (AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ == true) {
        // the algorithm can be loaded on the DSP platform
        AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
        AudioEffectChainManager::GetInstance()->SetSpkOffloadState();
        bool result = AudioEffectChainManager::GetInstance()->GetOffloadEnabled();
        EXPECT_EQ(true, result);
    } else {
        // the algorithm cannot be loaded on the DSP platform
        AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
        AudioEffectChainManager::GetInstance()->SetSpkOffloadState();
        bool result = AudioEffectChainManager::GetInstance()->GetOffloadEnabled();
        EXPECT_EQ(false, result);
    }
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateSpatialDeviceType API
* @tc.number : UpdateSpatialDeviceType_001
* @tc.desc   : Test UpdateSpatialDeviceType interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSpatialDeviceType_001, TestSize.Level1)
{
    AudioSpatialDeviceType spatialDeviceType = EARPHONE_TYPE_INEAR;

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    result = AudioEffectChainManager::GetInstance()->UpdateSpatialDeviceType(spatialDeviceType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckSceneTypeMatch API
* @tc.number : CheckSceneTypeMatch_001
* @tc.desc   : Test CheckSceneTypeMatch interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckSceneTypeMatch_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    const std::string sceneType = "SCENE_MUSIC";
    const std::string sinkSceneType = "SCENE_MOVIE";
    bool result = AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch(sinkSceneType, sceneType);
    EXPECT_EQ(false, result);

    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    result = AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch(sinkSceneType, sceneType);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();

    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    std::string sceneTypeAndDeviceKey2 = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain2 =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sinkSceneType, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey2] = audioEffectChain2;
    result = AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch(sinkSceneType, sceneType);
    EXPECT_EQ(false, result);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    result = AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch(sinkSceneType, sceneType);
    EXPECT_EQ(false, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckSceneTypeMatch API
* @tc.number : CheckSceneTypeMatch_002
* @tc.desc   : Test CheckSceneTypeMatch interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckSceneTypeMatch_002, TestSize.Level1)
{
    const std::string sceneType = "SCENE_MUSIC";
    const std::string sinkSceneType = "SCENE_MUSIC";

    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;

    bool result = AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch(sinkSceneType, sceneType);
    EXPECT_EQ(false, result);

    std::string sceneTypeAndDeviceKey2 = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain2 =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_MOVIE", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey2] = audioEffectChain2;
    result = AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch("SCENE_MOVIE", sceneType);
    EXPECT_EQ(false, result);

    result = AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch("", sceneType);
    EXPECT_EQ(false, result);

    result = AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch(sinkSceneType, "");
    EXPECT_EQ(false, result);

    AudioEffectChainManager::GetInstance()->sceneTypeToSpecialEffectSet_.insert(sceneType);
    result = AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch(sinkSceneType, sceneType);
    EXPECT_EQ(true, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckSceneTypeMatch API
* @tc.number : CheckSceneTypeMatch_003
* @tc.desc   : Test CheckSceneTypeMatch interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckSceneTypeMatch_003, TestSize.Level1)
{
    string sinkSceneType = "";
    string sceneType = "123";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    bool result = AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch(sinkSceneType, sceneType);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckSceneTypeMatch API
* @tc.number : CheckSceneTypeMatch_004
* @tc.desc   : Test CheckSceneTypeMatch interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckSceneTypeMatch_004, TestSize.Level1)
{
    string sinkSceneType = "SCENE_MOVIE";
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    bool result = AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch(sinkSceneType, sceneType);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateCurrSceneType API
* @tc.number : UpdateCurrSceneType_001
* @tc.desc   : Test UpdateCurrSceneType interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateCurrSceneType_001, TestSize.Level1)
{
    AudioEffectScene currSceneType = SCENE_OTHERS;
    string sceneType = "SCENE_MUSIC";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = false;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->UpdateCurrSceneType(currSceneType, sceneType);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->UpdateCurrSceneType(currSceneType, sceneType);

    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->UpdateCurrSceneType(currSceneType, sceneType);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->UpdateCurrSceneType(currSceneType, sceneType);

    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_MAX;
    AudioEffectChainManager::GetInstance()->UpdateCurrSceneType(currSceneType, sceneType);

    EXPECT_NE(SCENE_OTHERS, currSceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateCurrSceneType API
* @tc.number : UpdateCurrSceneType_002
* @tc.desc   : Test UpdateCurrSceneType interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateCurrSceneType_002, TestSize.Level1)
{
    AudioEffectScene currSceneType = SCENE_OTHERS;
    string sceneType = "SCENE_MUSIC";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;
    AudioEffectChainManager::GetInstance()->UpdateCurrSceneType(currSceneType, sceneType);
    EXPECT_NE(SCENE_OTHERS, currSceneType);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndReleaseCommonEffectChain API
* @tc.number : CheckAndReleaseCommonEffectChain_001
* @tc.desc   : Test CheckAndReleaseCommonEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndReleaseCommonEffectChain_001, TestSize.Level1)
{
    string sceneType = "SCENE_MUSIC";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    auto ret = AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(sceneType);
    EXPECT_EQ(ERROR, ret);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndReleaseCommonEffectChain API
* @tc.number : CheckAndReleaseCommonEffectChain_002
* @tc.desc   : Test CheckAndReleaseCommonEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndReleaseCommonEffectChain_002, TestSize.Level1)
{
    string sceneType = "SCENE_MUSIC";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = false;
    auto ret = AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(sceneType);
    EXPECT_EQ(ERROR, ret);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test NotifyAndCreateAudioEffectChain API
* @tc.number : NotifyAndCreateAudioEffectChain_001
* @tc.desc   : Test NotifyAndCreateAudioEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, NotifyAndCreateAudioEffectChain_001, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t result = AudioEffectChainManager::GetInstance()->NotifyAndCreateAudioEffectChain(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 0;
    result = AudioEffectChainManager::GetInstance()->NotifyAndCreateAudioEffectChain(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test WaitAndReleaseEffectChain API
* @tc.number : WaitAndReleaseEffectChain_001
* @tc.desc   : Test WaitAndReleaseEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, WaitAndReleaseEffectChain_001, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->WaitAndReleaseEffectChain(sceneType, sceneTypeAndDeviceKey,
        defaultSceneTypeAndDeviceKey, 0);
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, false);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 1;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey],
        audioEffectChain);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 0;
    AudioEffectChainManager::GetInstance()->WaitAndReleaseEffectChain(sceneType, sceneTypeAndDeviceKey,
        defaultSceneTypeAndDeviceKey, 1);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey], nullptr);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateDeviceInfo API
* @tc.number : UpdateDeviceInfo_001
* @tc.desc   : Test UpdateDeviceInfo interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateDeviceInfo_001, TestSize.Level1)
{
    int32_t device = 2;
    string sinkName = "Speaker";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateDeviceInfo(device, sinkName);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateDeviceInfo API
* @tc.number : UpdateDeviceInfo_002
* @tc.desc   : Test UpdateDeviceInfo interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateDeviceInfo_002, TestSize.Level1)
{
    int32_t device = 3;
    string sinkName = "Speaker";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->isInitialized_ = false;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateDeviceInfo(device, sinkName);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateDeviceInfo API
* @tc.number : UpdateDeviceInfo_003
* @tc.desc   : Test UpdateDeviceInfo interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateDeviceInfo_003, TestSize.Level1)
{
    int32_t device = 3;
    string sinkName = "Speaker";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->isInitialized_ = false;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateDeviceInfo(device, sinkName);
    EXPECT_EQ(ERROR, result);
    device = 2;
    result = AudioEffectChainManager::GetInstance()->UpdateDeviceInfo(device, sinkName);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test InitHdiState API
* @tc.number : InitHdiState_001
* @tc.desc   : Test InitHdiState interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, InitHdiState_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = nullptr;
    AudioEffectChainManager::GetInstance()->InitHdiState();
    std::shared_ptr<AudioEffectHdiParam> audioEffectHdiParam = std::make_shared<AudioEffectHdiParam>();
    AudioEffectChainManager::GetInstance()->InitHdiState();
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = audioEffectHdiParam;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    bool result = AudioEffectChainManager::GetInstance()->GetOffloadEnabled();
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateEffectBtOffloadSupported API
* @tc.number : UpdateEffectBtOffloadSupported_001
* @tc.desc   : Test UpdateEffectBtOffloadSupported interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateEffectBtOffloadSupported_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->btOffloadSupported_ = true;
    AudioEffectChainManager::GetInstance()->UpdateEffectBtOffloadSupported(true);
    bool result = AudioEffectChainManager::GetInstance()->btOffloadSupported_;
    EXPECT_EQ(true, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateEffectBtOffloadSupported API
* @tc.number : UpdateEffectBtOffloadSupported_002
* @tc.desc   : Test UpdateEffectBtOffloadSupported interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateEffectBtOffloadSupported_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->btOffloadSupported_ = true;
    AudioEffectChainManager::GetInstance()->UpdateEffectBtOffloadSupported(false);
    bool result = AudioEffectChainManager::GetInstance()->btOffloadSupported_;
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateEffectBtOffloadSupported API
* @tc.number : UpdateEffectBtOffloadSupported_003
* @tc.desc   : Test UpdateEffectBtOffloadSupported interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateEffectBtOffloadSupported_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->btOffloadSupported_ = false;
    AudioEffectChainManager::GetInstance()->UpdateEffectBtOffloadSupported(true);
    bool result = AudioEffectChainManager::GetInstance()->btOffloadSupported_;
    EXPECT_EQ(true, result);
    AudioEffectChainManager::GetInstance()->btOffloadSupported_ = false;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;
    AudioEffectChainManager::GetInstance()->UpdateEffectBtOffloadSupported(true);
    result = AudioEffectChainManager::GetInstance()->btOffloadSupported_;
    EXPECT_EQ(true, result);
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = false;
    AudioEffectChainManager::GetInstance()->UpdateEffectBtOffloadSupported(true);
    result = AudioEffectChainManager::GetInstance()->btOffloadSupported_;
    EXPECT_EQ(true, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateSpatializationEnabled API
* @tc.number : UpdateSpatializationEnabled_001
* @tc.desc   : Test UpdateSpatializationEnabled interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSpatializationEnabled_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioSpatializationState audioSpatializationState(true, false);
    AudioEffectChainManager::GetInstance()->UpdateSpatializationEnabled(audioSpatializationState);
    audioSpatializationState.spatializationEnabled = false;
    AudioEffectChainManager::GetInstance()->UpdateSpatializationEnabled(audioSpatializationState);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->UpdateSpatializationEnabled(audioSpatializationState);
}

/**
* @tc.name   : Test UpdateDefaultAudioEffect API
* @tc.number : UpdateDefaultAudioEffect_001
* @tc.desc   : Test UpdateDefaultAudioEffect interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateDefaultAudioEffect_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->UpdateDefaultAudioEffect();
    const std::string sessionID = "12345";
    int32_t ret = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    EXPECT_EQ(ret, SUCCESS);
    const std::string sessionID2 = "10000";
    ret = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID2, DEFAULT_INFO);
    EXPECT_EQ(ret, SUCCESS);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ =true;
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->UpdateDefaultAudioEffect();
}

/**
* @tc.name   : Test GetSceneTypeToChainCount API
* @tc.number : GetSceneTypeToChainCount_001
* @tc.desc   : Test GetSceneTypeToChainCount interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetSceneTypeToChainCount_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->UpdateDefaultAudioEffect();
    std::string sceneType = "DEFAULT_SCENE_TYPE";
    uint32_t ret = AudioEffectChainManager::GetInstance()->GetSceneTypeToChainCount(sceneType);
    EXPECT_EQ(ret, 0);

    const char *sceneType2 = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType2, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType2);
    EXPECT_EQ(SUCCESS, result);
    std::string sceneType3 = "SCENE_MUSIC";
    ret = AudioEffectChainManager::GetInstance()->GetSceneTypeToChainCount(sceneType3);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name   : Test GetSceneTypeToChainCount API
* @tc.number : GetSceneTypeToChainCount_002
* @tc.desc   : Test GetSceneTypeToChainCount interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetSceneTypeToChainCount_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->UpdateDefaultAudioEffect();
    std::string sceneType = "SCENE_DEFAULT";
    const char *sceneType1 = "SCENE_DEFAULT";
    std::string sceneTypeAndDeviceKey1 = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
        std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType1, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey1] = audioEffectChain;
    uint32_t ret = AudioEffectChainManager::GetInstance()->GetSceneTypeToChainCount(sceneType);
    EXPECT_EQ(ret, 0);

    const char *sceneType2 = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey2 = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain2 =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType2, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey2] = audioEffectChain2;
    ret = AudioEffectChainManager::GetInstance()->GetSceneTypeToChainCount(sceneType2);
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType2);
    EXPECT_EQ(SUCCESS, result);
    std::string sceneType3 = "SCENE_MUSIC";
    ret = AudioEffectChainManager::GetInstance()->GetSceneTypeToChainCount(sceneType3);
    EXPECT_EQ(ret, 0);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey2] = audioEffectChain;
    ret = AudioEffectChainManager::GetInstance()->GetSceneTypeToChainCount(sceneType3);
    EXPECT_EQ(ret, 0);
}

/**
* @tc.name   : Test UpdateSceneTypeList API
* @tc.number : UpdateSceneTypeList_001
* @tc.desc   : Test UpdateSceneTypeList interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSceneTypeList_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->UpdateDefaultAudioEffect();
    std::string sceneType = "DEFAULT_SCENE_TYPE";
    uint32_t ret = AudioEffectChainManager::GetInstance()->GetSceneTypeToChainCount(sceneType);
    EXPECT_EQ(ret, 0);
    const std::string sceneTypeupdate = "DEFAULT_SCENE_TYPE";
    SceneTypeOperation operation = ADD_SCENE_TYPE;
    AudioEffectChainManager::GetInstance()->sceneTypeCountList_.push_back(std::make_pair("SCENE_MUSIC", 1));
    AudioEffectChainManager::GetInstance()->UpdateSceneTypeList(sceneTypeupdate, operation);
    operation = REMOVE_SCENE_TYPE;
    AudioEffectChainManager::GetInstance()->UpdateSceneTypeList(sceneTypeupdate, operation);
}

/**
* @tc.name   : Test UpdateStreamUsage API
* @tc.number : UpdateStreamUsage_001
* @tc.desc   : Test UpdateStreamUsage interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateStreamUsage_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    const std::string sessionID = "12345";
    int32_t ret = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    EXPECT_EQ(ret, ERROR);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->sceneTypeToSpecialEffectSet_.insert(sceneType);
    AudioEffectChainManager::GetInstance()->UpdateStreamUsage();

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ =true;
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->UpdateStreamUsage();
}

/**
* @tc.name   : Test SessionInfoMapAdd API
* @tc.number : SessionInfoMapAdd_003
* @tc.desc   : Test SessionInfoMapAdd interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SessionInfoMapAdd_003, TestSize.Level1)
{
    const std::string sessionID = "12345";
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_[sessionID] = DEFAULT_INFO;
    int32_t ret = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    EXPECT_EQ(ret, ERROR);
    SessionEffectInfo info = {
        "EFFECT_DEFAULT1",
        "SCENE_MOVIE",
        INFOCHANNELS,
        INFOCHANNELLAYOUT,
    };
    ret = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, info);
    EXPECT_EQ(ret, SUCCESS);
    SessionEffectInfo info2 = {
        "EFFECT_DEFAULT",
        "SCENE_MOVIE",
        INFOCHANNELS,
        INFOCHANNELLAYOUT,
    };
    ret = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, info2);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name   : Test EffectApVolumeUpdate API
* @tc.number : EffectApVolumeUpdate_001
* @tc.desc   : Test EffectApVolumeUpdate interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, EffectApVolumeUpdate_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    const std::string sessionID = "12345";
    int32_t ret = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    EXPECT_EQ(ret, ERROR);
    const std::string sessionID2 = "10000";
    ret = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID2, DEFAULT_INFO);
    EXPECT_EQ(ret, ERROR);
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    ret = AudioEffectChainManager::GetInstance()->EffectApVolumeUpdate(audioEffectVolume);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name   : Test EffectDspVolumeUpdate API
* @tc.number : EffectDspVolumeUpdate_001
* @tc.desc   : Test EffectDspVolumeUpdate interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, EffectDspVolumeUpdate_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    const std::string sessionID = "12345";
    int32_t ret = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    EXPECT_EQ(ret, ERROR);
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    ret = AudioEffectChainManager::GetInstance()->EffectDspVolumeUpdate(audioEffectVolume);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name   : Test StreamVolumeUpdate API
* @tc.number : StreamVolumeUpdate_001
* @tc.desc   : Test StreamVolumeUpdate interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, StreamVolumeUpdate_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    const std::string sessionID = "12345";
    int32_t ret = AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    EXPECT_EQ(ret, ERROR);
    const std::string sessionIDString = "12345";
    const float streamVolume = 0.5;
    ret = AudioEffectChainManager::GetInstance()->StreamVolumeUpdate(sessionIDString, streamVolume);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name   : Test FindMaxSessionID
* @tc.number : FindMaxSessionID_001
* @tc.desc   : Test FindMaxSessionID interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, FindMaxSessionID_001, TestSize.Level1)
{
    std::set<std::string> sessions = {"12345", "67890", "34567"};
    uint32_t maxSessionID = 1;
    const std::string sessionID = "12345";
    std::string sceneType = "SCENE_MUSIC";
    SessionEffectInfo sessionEffectInfo = {
        "EFFECT_DEFAULT",
        "SCENE_MOVIE",
        INFOCHANNELS,
        INFOCHANNELLAYOUT,
    };
    EXPECT_NE(AudioEffectChainManager::GetInstance(), nullptr);
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_[sessionID] = sessionEffectInfo;
    const std::string scenePairType = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->FindMaxSessionID(maxSessionID, sceneType, scenePairType, sessions);
    maxSessionID = 99999;
    AudioEffectChainManager::GetInstance()->FindMaxSessionID(maxSessionID, sceneType, scenePairType, sessions);
}

/**
* @tc.name   : Test FindMaxSessionID
* @tc.number : FindMaxSessionID_002
* @tc.desc   : Test FindMaxSessionID interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, FindMaxSessionID_002, TestSize.Level1)
{
    std::set<std::string> sessions = {"12345", "67890", "34567"};
    uint32_t maxSessionID = 1;
    const std::string sessionID = "12345";
    std::string sceneType = "EFFECT_NONE";
    SessionEffectInfo sessionEffectInfo = {
        "EFFECT_NONE",
        "SCENE_MOVIE",
        INFOCHANNELS,
        INFOCHANNELLAYOUT,
    };

    EXPECT_NE(AudioEffectChainManager::GetInstance(), nullptr);
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_[sessionID] = sessionEffectInfo;
    const std::string scenePairType = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->FindMaxSessionID(maxSessionID, sceneType, scenePairType, sessions);
    maxSessionID = 99999;
    AudioEffectChainManager::GetInstance()->FindMaxSessionID(maxSessionID, sceneType, scenePairType, sessions);
}

/**
* @tc.name   : Test SetAudioEffectProperty
* @tc.number : SetAudioEffectProperty_001
* @tc.desc   : Test SetAudioEffectProperty interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetAudioEffectProperty_001, TestSize.Level1)
{
    AudioEffectProperty  audioEffectProperty1 = {
        .effectClass = "testClass1",
        .effectProp = "testProp1",
    };

    AudioEffectProperty  audioEffectProperty2 = {
        .effectClass = "testClass2",
        .effectProp = "testProp2",
    };

    AudioEffectPropertyArray audioEffectPropertyArray = {};
    audioEffectPropertyArray.property.push_back(audioEffectProperty1);
    audioEffectPropertyArray.property.push_back(audioEffectProperty2);

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t ret = AudioEffectChainManager::GetInstance()->SetAudioEffectProperty(audioEffectPropertyArray);
    EXPECT_EQ(AUDIO_OK, ret);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    ret = AudioEffectChainManager::GetInstance()->SetAudioEffectProperty(audioEffectPropertyArray);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name   : Test SetAudioEffectProperty
* @tc.number : SetAudioEffectProperty_002
* @tc.desc   : Test SetAudioEffectProperty interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetAudioEffectProperty_002, TestSize.Level1)
{
    AudioEffectPropertyV3  audioEffectPropertyV31 = {
        .name = "testName1",
        .category = "testCategory1",
    };

    AudioEffectPropertyV3  audioEffectPropertyV32 = {
        .name = "testName2",
        .category = "testCategory2",
    };

    AudioEffectPropertyArrayV3 audioEffectPropertyArrayV3 = {};
    audioEffectPropertyArrayV3.property.push_back(audioEffectPropertyV31);
    audioEffectPropertyArrayV3.property.push_back(audioEffectPropertyV32);

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t ret = AudioEffectChainManager::GetInstance()->SetAudioEffectProperty(audioEffectPropertyArrayV3);
    EXPECT_EQ(AUDIO_OK, ret);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    ret = AudioEffectChainManager::GetInstance()->SetAudioEffectProperty(audioEffectPropertyArrayV3);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name   : Test GetAudioEffectProperty
* @tc.number : GetAudioEffectProperty_001
* @tc.desc   : Test GetAudioEffectProperty interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetAudioEffectProperty_001, TestSize.Level1)
{
    AudioEffectPropertyArray audioEffectPropertyArray = {};
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t ret = AudioEffectChainManager::GetInstance()->GetAudioEffectProperty(audioEffectPropertyArray);
    EXPECT_EQ(AUDIO_OK, ret);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->effectPropertyMap_.insert(std::make_pair("SCENE_MUSIC", "property"));
    ret = AudioEffectChainManager::GetInstance()->GetAudioEffectProperty(audioEffectPropertyArray);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name   : Test GetAudioEffectProperty
* @tc.number : GetAudioEffectProperty_002
* @tc.desc   : Test GetAudioEffectProperty interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetAudioEffectProperty_002, TestSize.Level1)
{
    AudioEffectPropertyArrayV3 audioEffectPropertyArrayV3 = {};
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t ret = AudioEffectChainManager::GetInstance()->GetAudioEffectProperty(audioEffectPropertyArrayV3);
    EXPECT_EQ(AUDIO_OK, ret);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->effectPropertyMap_.insert(std::make_pair("SCENE_MUSIC", "property"));
    ret = AudioEffectChainManager::GetInstance()->GetAudioEffectProperty(audioEffectPropertyArrayV3);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name   : Test CheckIfSpkDsp
* @tc.number : CheckIfSpkDsp_001
* @tc.desc   : Test CheckIfSpkDsp interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckIfSpkDsp_001, TestSize.Level1)
{
    bool ret = AudioEffectChainManager::GetInstance()->CheckIfSpkDsp();
    EXPECT_EQ(false, ret);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_MIC;
    ret = AudioEffectChainManager::GetInstance()->CheckIfSpkDsp();
    EXPECT_EQ(false, ret);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    ret = AudioEffectChainManager::GetInstance()->CheckIfSpkDsp();
    EXPECT_EQ(true, ret);

    const char *sceneType1 = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey1 = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain1 =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType1, true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey1] = audioEffectChain1;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType1);
    EXPECT_EQ(SUCCESS, result);

    const char *sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    ret = AudioEffectChainManager::GetInstance()->CheckIfSpkDsp();
    EXPECT_EQ(true, ret);
}

/**
* @tc.name   : Test SetSpatializationSceneTypeToChains
* @tc.number : SetSpatializationSceneTypeToChains_001
* @tc.desc   : Test SetSpatializationSceneTypeToChains interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetSpatializationSceneTypeToChains_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->SetSpatializationSceneTypeToChains();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = nullptr;
    AudioEffectChainManager::GetInstance()->SetSpatializationSceneTypeToChains();
}

/**
* @tc.name   : Test InitEffectBuffer
* @tc.number : InitEffectBuffer_001
* @tc.desc   : Test InitEffectBuffer interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, InitEffectBuffer_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    string sessionID1 = "123456";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID1, DEFAULT_INFO);
    SessionEffectInfo NONE_INFO = DEFAULT_INFO;
    NONE_INFO.sceneMode = "EFFECT_NONE";
    std::string sessionID2 = "123457";
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID2, NONE_INFO);
    int32_t result = AudioEffectChainManager::GetInstance()->InitEffectBuffer(sessionID1);
    EXPECT_EQ(SUCCESS, result);
    result = AudioEffectChainManager::GetInstance()->InitEffectBuffer(sessionID2);
    EXPECT_EQ(SUCCESS, result);

    string sessionID3 = "111111";
    result = AudioEffectChainManager::GetInstance()->InitEffectBuffer(sessionID3);
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID3, DEFAULT_INFO);
    result = AudioEffectChainManager::GetInstance()->InitEffectBuffer(sessionID3);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test IsEffectChainStop
* @tc.number : IsEffectChainStop_001
* @tc.desc   : Test IsEffectChainStop interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, IsEffectChainStop_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    string sessionID1 = "123456";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID1, DEFAULT_INFO);
    bool result = AudioEffectChainManager::GetInstance()->IsEffectChainStop(sceneType, sessionID1);
    EXPECT_EQ(true, result);

    string sessionID2 = "111111";
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID2, DEFAULT_INFO);
    result = AudioEffectChainManager::GetInstance()->IsEffectChainStop(sceneType, sessionID2);
    EXPECT_EQ(false, result);
}

/**
* @tc.name   : Test SetSpatializationEnabledToChains API
* @tc.number : SetSpatializationEnabledToChains_003
* @tc.desc   : Test SetSpatializationEnabledToChains interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetSpatializationEnabledToChains_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->ResetInfo();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({"test", nullptr});
    AudioEffectChainManager::GetInstance()->SetSpatializationEnabledToChains();
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.begin()->second == nullptr);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateStreamUsage API
* @tc.number : UpdateStreamUsage_002
* @tc.desc   : Test UpdateStreamUsage interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateStreamUsage_002, TestSize.Level1)
{
    std::vector<std::string> effects = {"test1", "test2"};
    AudioEffectChainManager::GetInstance()->sceneTypeToSpecialEffectSet_.insert(effects.begin(), effects.end());
    AudioEffectChainManager::GetInstance()->priorSceneList_.push_back("test1");
    AudioEffectChainManager::GetInstance()->priorSceneList_.push_back("test2");

    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>("123", headTracker);
    ASSERT_TRUE(audioEffectChain != nullptr);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({"123", audioEffectChain});

    AudioEffectChainManager::GetInstance()->UpdateStreamUsage();
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test FindMaxEffectChannels API
* @tc.number : FindMaxEffectChannels_001
* @tc.desc   : Test FindMaxEffectChannels interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, FindMaxEffectChannels_001, TestSize.Level1)
{
    std::string sceneType = "";
    std::set<std::string> sessions = {"test1"};
    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_NONE;
    AudioEffectChainManager::GetInstance()->FindMaxEffectChannels(sceneType, sessions, channels, channelLayout);
    EXPECT_EQ(channels, STEREO);

    channels = 10;
    AudioEffectChainManager::GetInstance()->FindMaxEffectChannels(sceneType, sessions, channels, channelLayout);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CreateAudioEffectChain API
* @tc.number : CreateAudioEffectChain_002
* @tc.desc   : Test CreateAudioEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChain_002, TestSize.Level1)
{
    std::string sceneType = "test";
    bool isPriorScene = false;

    AudioEffectChainManager::GetInstance()->maxEffectChainCount_ = 0;
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = false;
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, isPriorScene);
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_);

    auto ret = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, isPriorScene);
    EXPECT_TRUE(ret == nullptr);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndReleaseCommonEffectChain API
* @tc.number : CheckAndReleaseCommonEffectChain_003
* @tc.desc   : Test CheckAndReleaseCommonEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndReleaseCommonEffectChain_003, TestSize.Level1)
{
    std::string sceneType = "test";
    std::string  scene = "SCENE_DEFAULT";
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    std::string deviceTypeName = AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string effectChain1 = sceneType + "_&_" + deviceTypeName;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain1, nullptr});
    auto ret = AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(sceneType);
    EXPECT_EQ(ret, ERROR);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndReleaseCommonEffectChain API
* @tc.number : CheckAndReleaseCommonEffectChain_004
* @tc.desc   : Test CheckAndReleaseCommonEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndReleaseCommonEffectChain_004, TestSize.Level1)
{
    std::string sceneType = "test";
    std::string  scene = "SCENE_DEFAULT";
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = false;
    auto ret = AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(sceneType);
    EXPECT_EQ(ret, ERROR);

    std::string deviceTypeName = AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string effectChain0 = scene + "_&_" + deviceTypeName;
    std::string effectChain1 = sceneType + "_&_" + deviceTypeName;
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain0, audioEffectChain});
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain1, audioEffectChain});

    AudioEffectChainManager::GetInstance()->defaultEffectChainCount_ = 2;
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    ret = AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(sceneType);
    EXPECT_EQ(ret, ERROR);

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    ret = AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(sceneType);
    EXPECT_EQ(ret, SUCCESS);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndReleaseCommonEffectChain API
* @tc.number : CheckAndReleaseCommonEffectChain_005
* @tc.desc   : Test CheckAndReleaseCommonEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndReleaseCommonEffectChain_005, TestSize.Level1)
{
    std::string sceneType = "test";
    std::string  scene = "SCENE_DEFAULT";

    std::string deviceTypeName = AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string effectChain0 = scene + "_&_" + deviceTypeName;
    std::string effectChain1 = sceneType + "_&_" + deviceTypeName;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain0, nullptr});
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain1, nullptr});

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    auto ret = AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(sceneType);
    EXPECT_EQ(ret, ERROR);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetSceneTypeToChainCount API
* @tc.number : GetSceneTypeToChainCount_003
* @tc.desc   : Test GetSceneTypeToChainCount interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetSceneTypeToChainCount_003, TestSize.Level1)
{
    std::string sceneType = "test";
    std::string  scene = "SCENE_DEFAULT";
    std::string deviceTypeName = AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string effectChain0 = scene + "_&_" + deviceTypeName;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain0, nullptr});
    auto ret = AudioEffectChainManager::GetInstance()->GetSceneTypeToChainCount(sceneType);
    EXPECT_EQ(ret, 0);

    std::string effectChain1 = sceneType + "_&_" + deviceTypeName;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain1, nullptr});
    ret = AudioEffectChainManager::GetInstance()->GetSceneTypeToChainCount(sceneType);
    EXPECT_EQ(ret, 0);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetSceneTypeToChainCount API
* @tc.number : GetSceneTypeToChainCount_004
* @tc.desc   : Test GetSceneTypeToChainCount interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetSceneTypeToChainCount_004, TestSize.Level1)
{
    std::string sceneType = "test";
    std::string  scene = "SCENE_DEFAULT";
    std::string deviceTypeName = AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string effectChain0 = scene + "_&_" + deviceTypeName;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain0, nullptr});
    auto ret = AudioEffectChainManager::GetInstance()->GetSceneTypeToChainCount(sceneType);
    EXPECT_EQ(ret, 0);

    std::string effectChain1 = sceneType + "_&_" + deviceTypeName;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain1, nullptr});
    ret = AudioEffectChainManager::GetInstance()->GetSceneTypeToChainCount(sceneType);
    EXPECT_EQ(ret, 0);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckProcessClusterInstances API
* @tc.number : CheckProcessClusterInstances_001
* @tc.desc   : Test CheckProcessClusterInstances interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckProcessClusterInstances_001, TestSize.Level1)
{
    std::string sceneType = "test";
    AudioEffectChainManager::GetInstance()->maxEffectChainCount_ = 0;

    auto ret = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(sceneType);
    EXPECT_EQ(ret, CREATE_DEFAULT_PROCESSCLUSTER);

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    ret = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(sceneType);
    EXPECT_EQ(ret, USE_DEFAULT_PROCESSCLUSTER);

    AudioEffectChainManager::GetInstance()->maxEffectChainCount_ = 10;
    ret = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(sceneType);
    EXPECT_EQ(ret, CREATE_NEW_PROCESSCLUSTER);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckProcessClusterInstances API
* @tc.number : CheckProcessClusterInstances_002
* @tc.desc   : Test CheckProcessClusterInstances interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckProcessClusterInstances_002, TestSize.Level1)
{
    std::string sceneType = "test";
    AudioEffectChainManager::GetInstance()->priorSceneList_.push_back(sceneType);
    auto ret = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(sceneType);
    EXPECT_EQ(ret, CREATE_NEW_PROCESSCLUSTER);

    std::string effect = sceneType + "_&_" + AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effect, nullptr});
    ret = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(sceneType);
    EXPECT_EQ(ret, CREATE_NEW_PROCESSCLUSTER);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/*
* @tc.name   : Test InitEffectBufferInner
* @tc.number : InitEffectBufferInner_001
* @tc.desc   : Test InitEffectBufferInner interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, InitEffectBufferInner_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    string sessionID1 = "123456";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID1, DEFAULT_INFO);
    int32_t result = AudioEffectChainManager::GetInstance()->InitEffectBufferInner(sessionID1);
    EXPECT_EQ(SUCCESS, result);

    string sessionID2 = "111111";
    result = AudioEffectChainManager::GetInstance()->InitEffectBufferInner(sessionID2);
    EXPECT_NE(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ConfigureAudioEffectChain
* @tc.number : ConfigureAudioEffectChain_001
* @tc.desc   : Test ConfigureAudioEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, ConfigureAudioEffectChain_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    string sessionID1 = "123456";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID1, DEFAULT_INFO);
    std::string effectMode = "EFFECT_MODE_NORMAL";
    AudioEffectChainManager::GetInstance()->ConfigureAudioEffectChain(audioEffectChain, effectMode);
    EXPECT_NE(audioEffectChain, nullptr);
}

/**
* @tc.name   : Test InitHdiStateInner
* @tc.number : InitHdiStateInner_001
* @tc.desc   : Test InitHdiStateInner interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, InitHdiStateInner_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->InitHdiStateInner();
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->InitHdiStateInner();
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->InitHdiStateInner();
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->spkOffloadEnabled_, false);
}

/**
* @tc.name   : Test EffectVolumeUpdateInner
* @tc.number : EffectVolumeUpdateInner_001
* @tc.desc   : Test EffectVolumeUpdateInner interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, EffectVolumeUpdateInner_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    const char *sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    string sessionID1 = "123456";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID1, DEFAULT_INFO);
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    int32_t result = AudioEffectChainManager::GetInstance()->EffectVolumeUpdateInner(audioEffectVolume);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test ReturnEffectChannelInfoInner
* @tc.number : ReturnEffectChannelInfoInner_001
* @tc.desc   : Test ReturnEffectChannelInfoInner interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnEffectChannelInfoInner_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    string sessionID1 = "123456";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID1, DEFAULT_INFO);
    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    int32_t result = AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfoInner(sceneType,
        channels, channelLayout);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test UpdateSpatializationStateInner
* @tc.number : UpdateSpatializationStateInner_001
* @tc.desc   : Test UpdateSpatializationStateInner interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSpatializationStateInner_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioSpatializationState spatializationState = {true, true};
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateSpatializationStateInner(spatializationState);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = false;
    result = AudioEffectChainManager::GetInstance()->UpdateSpatializationStateInner(spatializationState);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test UpdateMultichannelConfigInner
* @tc.number : UpdateMultichannelConfigInner_001
* @tc.desc   : Test UpdateMultichannelConfigInner interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateMultichannelConfigInner_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    string sessionID1 = "123456";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID1, DEFAULT_INFO);
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateMultichannelConfigInner(sceneType);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test ExistAudioEffectChainInner
* @tc.number : ExistAudioEffectChainInner_001
* @tc.desc   : Test ExistAudioEffectChainInner interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChainInner_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string effectMode = "EFFECT_MODE_NORMAL";
    bool result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChainInner(sceneType, effectMode);
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChainInner(sceneType, effectMode);
    EXPECT_EQ(false, result);
}

/**
 * @tc.name   : Test ReleaseAudioEffectChainDynamicInner
 * @tc.number : ReleaseAudioEffectChainDynamicInner_001
 * @tc.desc   : Test ReleaseAudioEffectChainDynamicInner interface.
 */
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamicInner_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    string sessionID1 = "123456";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID1, DEFAULT_INFO);
    int32_t result = AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamicInner(sceneType);
    EXPECT_EQ(SUCCESS, result);

    sceneType = "";
    result = AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamicInner(sceneType);
    EXPECT_EQ(ERROR, result);
}

/**
 * @tc.name   : Test CreateAudioEffectChainDynamicInner
 * @tc.number : CreateAudioEffectChainDynamicInner_001
 * @tc.desc   : Test CreateAudioEffectChainDynamicInner interface.
 */
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChainDynamicInner_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    string sessionID1 = "123456";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID1, DEFAULT_INFO);
    int32_t result = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamicInner(sceneType);
    EXPECT_EQ(SUCCESS, result);

    sceneType = "";
    result = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamicInner(sceneType);
    EXPECT_EQ(ERROR, result);
}

/**
 * @tc.name   : Test QueryEffectChannelInfoInner
 * @tc.number : QueryEffectChannelInfoInner_001
 * @tc.desc   : Test QueryEffectChannelInfoInner interface.
 */
HWTEST(AudioEffectChainManagerUnitTest, QueryEffectChannelInfoInner_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    string sessionID1 = "123456";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID1, DEFAULT_INFO);
    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    int32_t result = AudioEffectChainManager::GetInstance()->QueryEffectChannelInfoInner(sceneType, channels,
        channelLayout);
    EXPECT_EQ(SUCCESS, result);
}

/**
 * @tc.name   : Test InitAudioEffectChainDynamicInner
 * @tc.number : InitAudioEffectChainDynamicInner_001
 * @tc.desc   : Test InitAudioEffectChainDynamicInner interface.
 */
HWTEST(AudioEffectChainManagerUnitTest, InitAudioEffectChainDynamicInner_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    string sessionID1 = "123456";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID1, DEFAULT_INFO);
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamicInner(sceneType);
    EXPECT_EQ(SUCCESS, result);

    sceneType = "";
    result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamicInner(sceneType);
    EXPECT_EQ(ERROR, result);
}

/**
 * @tc.name   : Test SendEffectApVolume
 * @tc.number : SendEffectApVolume_001
 * @tc.desc   : Test SendEffectApVolume interface.
 */
HWTEST(AudioEffectChainManagerUnitTest, SendEffectApVolume_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    string sessionID1 = "123456";
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID1, DEFAULT_INFO);
    int32_t result = AudioEffectChainManager::GetInstance()->SendEffectApVolume(nullptr);
    EXPECT_EQ(ERROR, result);

    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    audioEffectVolume->SetDspVolume(0.5f);
    result = AudioEffectChainManager::GetInstance()->SendEffectApVolume(audioEffectVolume);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test EffectApVolumeUpdate API
* @tc.number : EffectApVolumeUpdate_002
* @tc.desc   : Test EffectApVolumeUpdate interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, EffectApVolumeUpdate_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->ResetInfo();
    SessionEffectInfo sessionEffectInfo;
    AudioEffectChainManager::GetInstance()->sessionIDSet_.insert("test");
    AudioEffectChainManager::GetInstance()->sessionIDSet_.insert("test1");
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.insert({"test", sessionEffectInfo});
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    auto ret = AudioEffectChainManager::GetInstance()->EffectApVolumeUpdate(audioEffectVolume);
    EXPECT_EQ(ret, SUCCESS);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SendEffectApVolume API
* @tc.number : SendEffectApVolume_002
* @tc.desc   : Test SendEffectApVolume interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SendEffectApVolume_002, TestSize.Level1)
{
    std::string scene = "test";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);
    ASSERT_TRUE(audioEffectChain != nullptr);
    audioEffectChain->SetCurrVolume(0.0f);
    audioEffectChain->SetFinalVolume(0.0f);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({"test", nullptr});
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({"test1", audioEffectChain});
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    auto ret = AudioEffectChainManager::GetInstance()->SendEffectApVolume(audioEffectVolume);
    EXPECT_EQ(ret, SUCCESS);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SendEffectApVolume API
* @tc.number : SendEffectApVolume_003
* @tc.desc   : Test SendEffectApVolume interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SendEffectApVolume_003, TestSize.Level1)
{
    std::string scene = "test";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);
    ASSERT_TRUE(audioEffectChain != nullptr);
    audioEffectChain->SetCurrVolume(0.0f);
    audioEffectChain->SetFinalVolume(0.5f);
    audioEffectChain->SetFinalVolumeState(true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({"test", audioEffectChain});
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    auto ret = AudioEffectChainManager::GetInstance()->SendEffectApVolume(audioEffectVolume);
    EXPECT_EQ(ret, SUCCESS);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test QueryHdiSupportedChannelInfo API
* @tc.number : QueryHdiSupportedChannelInfo_001
* @tc.desc   : Test QueryHdiSupportedChannelInfo interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, QueryHdiSupportedChannelInfo_001, TestSize.Level1)
{
    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    auto ret = AudioEffectChainManager::GetInstance()->QueryHdiSupportedChannelInfo(channels, channelLayout);
    EXPECT_EQ(ret, SUCCESS);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test QueryHdiSupportedChannelInfo API
* @tc.number : QueryHdiSupportedChannelInfo_002
* @tc.desc   : Test QueryHdiSupportedChannelInfo interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, QueryHdiSupportedChannelInfo_002, TestSize.Level1)
{
    uint32_t channels = 0;
    uint64_t channelLayout = 10;
    SessionEffectInfo sessionEffectInfo;
    sessionEffectInfo.channelLayout = 1;
    std::set<std::string> sceneType = {"123"};
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_.insert({"test", sceneType});
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.insert({"123", sessionEffectInfo});

    auto ret = AudioEffectChainManager::GetInstance()->QueryHdiSupportedChannelInfo(channels, channelLayout);
    EXPECT_EQ(ret, ERROR);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test QueryHdiSupportedChannelInfo API
* @tc.number : QueryHdiSupportedChannelInfo_003
* @tc.desc   : Test QueryHdiSupportedChannelInfo interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, QueryHdiSupportedChannelInfo_003, TestSize.Level1)
{
    uint32_t channels = 0;
    uint64_t channelLayout = 10;
    SessionEffectInfo sessionEffectInfo;
    sessionEffectInfo.channelLayout = 1;
    sessionEffectInfo.channels = 5;
    std::set<std::string> sceneType = {"123"};
    AudioEffectChainManager::GetInstance()->initializedLogFlag_ = false;
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_.insert({"test", sceneType});
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.insert({"123", sessionEffectInfo});

    auto ret = AudioEffectChainManager::GetInstance()->QueryHdiSupportedChannelInfo(channels, channelLayout);
    EXPECT_EQ(ret, ERROR);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test QueryHdiSupportedChannelInfo API
* @tc.number : QueryHdiSupportedChannelInfo_004
* @tc.desc   : Test QueryHdiSupportedChannelInfo interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, QueryHdiSupportedChannelInfo_004, TestSize.Level1)
{
    uint32_t channels = 0;
    uint64_t channelLayout = 10;
    SessionEffectInfo sessionEffectInfo;
    sessionEffectInfo.channelLayout = 1;
    sessionEffectInfo.channels = 20;
    std::set<std::string> sceneType = {"123"};
    AudioEffectChainManager::GetInstance()->isInitialized_ = true;
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_.insert({"test", sceneType});
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.insert({"123", sessionEffectInfo});

    auto ret = AudioEffectChainManager::GetInstance()->QueryHdiSupportedChannelInfo(channels, channelLayout);
    EXPECT_EQ(ret, SUCCESS);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test QueryHdiSupportedChannelInfo API
* @tc.number : QueryHdiSupportedChannelInfo_005
* @tc.desc   : Test QueryHdiSupportedChannelInfo interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, QueryHdiSupportedChannelInfo_005, TestSize.Level1)
{
    uint32_t channels = 0;
    uint64_t channelLayout = CH_LAYOUT_STEREO;
    SessionEffectInfo sessionEffectInfo;
    sessionEffectInfo.channels = 0;
    std::set<std::string> sceneType = {"123"};
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_.insert({"test", sceneType});

    auto ret = AudioEffectChainManager::GetInstance()->QueryHdiSupportedChannelInfo(channels, channelLayout);
    EXPECT_EQ(ret, SUCCESS);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SendAudioParamToHDI API
* @tc.number : SendAudioParamToHDI_001
* @tc.desc   : Test SendAudioParamToHDI interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SendAudioParamToHDI_001, TestSize.Level1)
{
    HdiSetParamCommandCode code = HDI_INIT;
    const std::string value = "-0";
    DeviceType device = DEVICE_TYPE_EARPIECE;
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = std::make_shared<AudioEffectHdiParam>();
    ASSERT_TRUE(AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ != nullptr);

    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_->hdiModel_ = nullptr;
    AudioEffectChainManager::GetInstance()->SendAudioParamToHDI(code, value, device);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SendAudioParamToARM API
* @tc.number : SendAudioParamToARM_001
* @tc.desc   : Test SendAudioParamToARM interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SendAudioParamToARM_001, TestSize.Level1)
{
    HdiSetParamCommandCode code = HDI_FOLD_STATE;
    std::string value = "test";
    std::string scene = "123";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);
    ASSERT_TRUE(audioEffectChain != nullptr);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({scene, audioEffectChain});
    AudioEffectChainManager::GetInstance()->SendAudioParamToARM(code, value);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SendAudioParamToARM API
* @tc.number : SendAudioParamToARM_002
* @tc.desc   : Test SendAudioParamToARM interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SendAudioParamToARM_002, TestSize.Level1)
{
    HdiSetParamCommandCode code = HDI_LID_STATE;
    std::string scene = "123";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);
    ASSERT_TRUE(audioEffectChain != nullptr);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({scene, audioEffectChain});
    AudioEffectChainManager::GetInstance()->SendAudioParamToARM(code, scene);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SendAudioParamToARM API
* @tc.number : SendAudioParamToARM_003
* @tc.desc   : Test SendAudioParamToARM interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SendAudioParamToARM_003, TestSize.Level1)
{
    HdiSetParamCommandCode code = HDI_QUERY_CHANNELLAYOUT;
    std::string scene = "123";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);
    ASSERT_TRUE(audioEffectChain != nullptr);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({scene, audioEffectChain});
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({"123", nullptr});
    AudioEffectChainManager::GetInstance()->SendAudioParamToARM(code, scene);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateParamExtra API
* @tc.number : UpdateParamExtra_002
* @tc.desc   : Test UpdateParamExtra interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateParamExtra_002, TestSize.Level1)
{
    std::string mainkey = "device_status";
    std::string subkey = "update_audio_effect_type";
    std::string value = "test";
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, value);

    subkey = "fold_state";
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, value);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->foldState_, value);

    subkey = "lid_state";
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, value);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->lidState_, value);

    mainkey = "test";
    subkey = "test";
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, value);

    subkey = "fold_state";
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, value);

    subkey = "lid_state";
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, value);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test EffectApVolumeUpdate API
* @tc.number : EffectApVolumeUpdate_003
* @tc.desc   : Test EffectApVolumeUpdate interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, EffectApVolumeUpdate_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->ResetInfo();
    SessionEffectInfo sessionEffectInfo;
    sessionEffectInfo.sceneMode = "123";
    AudioEffectChainManager::GetInstance()->sessionIDSet_.insert("test");
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.insert({"test", sessionEffectInfo});
    std::string deviceKey = AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["test"].sceneType +
        "_&_" + AudioEffectChainManager::GetInstance()->GetDeviceTypeName();

    auto headTracker = std::make_shared<HeadTracker>();
    auto audioEffectChain = std::make_shared<AudioEffectChain>("test", headTracker);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[deviceKey] = audioEffectChain;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    auto ret = AudioEffectChainManager::GetInstance()->EffectApVolumeUpdate(audioEffectVolume);
    EXPECT_EQ(ret, SUCCESS);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SendAudioParamToARM API
* @tc.number : SendAudioParamToARM_004
* @tc.desc   : Test SendAudioParamToARM interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SendAudioParamToARM_004, TestSize.Level1)
{
    HdiSetParamCommandCode code = HDI_LID_STATE;
    std::string scene = "123";
    std::shared_ptr<AudioEffectChain> audioEffectChain = nullptr;

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({scene, audioEffectChain});
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.empty());
    AudioEffectChainManager::GetInstance()->SendAudioParamToARM(code, scene);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test LoadEffectProperties API
* @tc.number : LoadEffectProperties_001
* @tc.desc   : Test LoadEffectProperties interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, LoadEffectProperties_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->hasLoadedEffectProperties_ = false;
    AudioEffectChainManager::GetInstance()->LoadEffectProperties();
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->hasLoadedEffectProperties_, true);
}

/**
* @tc.name   : Test SetAudioEffectProperty
* @tc.number : SetAudioEffectProperty_003
* @tc.desc   : Test SetAudioEffectProperty interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetAudioEffectProperty_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->ResetInfo();
    AudioEffectPropertyV3  audioEffectPropertyV31 = {
        .name = "testName1",
        .category = "testCategory1",
    };

    AudioEffectPropertyV3  audioEffectPropertyV32 = {
        .name = "testName2",
        .category = "testCategory2",
    };

    AudioEffectPropertyArrayV3 audioEffectPropertyArrayV3 = {};
    audioEffectPropertyArrayV3.property.push_back(audioEffectPropertyV31);
    audioEffectPropertyArrayV3.property.push_back(audioEffectPropertyV32);

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain = nullptr;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t ret = AudioEffectChainManager::GetInstance()->SetAudioEffectProperty(audioEffectPropertyArrayV3);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name   : Test SetAudioEffectProperty
* @tc.number : SetAudioEffectProperty_004
* @tc.desc   : Test SetAudioEffectProperty interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetAudioEffectProperty_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->ResetInfo();
    AudioEffectProperty  audioEffectProperty1 = {
        .effectClass = "testClass1",
        .effectProp = "testProp1",
    };

    AudioEffectProperty  audioEffectProperty2 = {
        .effectClass = "testClass2",
        .effectProp = "testProp2",
    };

    AudioEffectPropertyArray audioEffectPropertyArray = {};
    audioEffectPropertyArray.property.push_back(audioEffectProperty1);
    audioEffectPropertyArray.property.push_back(audioEffectProperty2);

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain = nullptr;

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    auto ret = AudioEffectChainManager::GetInstance()->SetAudioEffectProperty(audioEffectPropertyArray);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name   : Test GetAudioEffectProperty
* @tc.number : GetAudioEffectProperty_003
* @tc.desc   : Test GetAudioEffectProperty interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetAudioEffectProperty_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->ResetInfo();
    AudioEffectPropertyArrayV3 audioEffectPropertyArrayV3 = {};
    std::string property1 = "123";
    std::string property2 = "";
    AudioEffectChainManager::GetInstance()->effectPropertyMap_.insert({property1, property2});
    int32_t ret = AudioEffectChainManager::GetInstance()->GetAudioEffectProperty(audioEffectPropertyArrayV3);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name   : Test GetAudioEffectProperty
* @tc.number : GetAudioEffectProperty_004
* @tc.desc   : Test GetAudioEffectProperty interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetAudioEffectProperty_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->ResetInfo();
    AudioEffectPropertyArrayV3 audioEffectPropertyArrayV3 = {};
    std::string property1 = "123";
    std::string property2 = "test";
    AudioEffectChainManager::GetInstance()->effectPropertyMap_.insert({property1, property2});
    int32_t ret = AudioEffectChainManager::GetInstance()->GetAudioEffectProperty(audioEffectPropertyArrayV3);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name   : Test GetAudioEffectProperty
* @tc.number : GetAudioEffectProperty_005
* @tc.desc   : Test GetAudioEffectProperty interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetAudioEffectProperty_005, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->ResetInfo();
    AudioEffectPropertyArray audioEffectPropertyArray = {};
    std::string property1 = "123";
    std::string property2 = "test";
    AudioEffectChainManager::GetInstance()->effectPropertyMap_.insert({property1, property2});
    int32_t ret = AudioEffectChainManager::GetInstance()->GetAudioEffectProperty(audioEffectPropertyArray);
    EXPECT_EQ(AUDIO_OK, ret);

    property2 = "";
    AudioEffectChainManager::GetInstance()->effectPropertyMap_.clear();
    AudioEffectChainManager::GetInstance()->effectPropertyMap_.insert({property1, property2});
    ret = AudioEffectChainManager::GetInstance()->GetAudioEffectProperty(audioEffectPropertyArray);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name   : Test UpdateSceneTypeList API
* @tc.number : UpdateSceneTypeList_002
* @tc.desc   : Test UpdateSceneTypeList interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSceneTypeList_002, TestSize.Level1)
{
    std::string sceneType = "test";
    SceneTypeOperation operation = ADD_SCENE_TYPE;
    AudioEffectChainManager::GetInstance()->sceneTypeCountList_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeCountList_.push_back(
        std::make_pair<std::string, int32_t>("test", 10));
    auto ret = AudioEffectChainManager::GetInstance()->UpdateSceneTypeList(sceneType, operation);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name   : Test UpdateSceneTypeList API
* @tc.number : UpdateSceneTypeList_003
* @tc.desc   : Test UpdateSceneTypeList interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSceneTypeList_003, TestSize.Level1)
{
    std::string sceneType = "test";
    SceneTypeOperation operation = REMOVE_SCENE_TYPE;
    AudioEffectChainManager::GetInstance()->sceneTypeCountList_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeCountList_.push_back(
        std::make_pair<std::string, int32_t>("test", 10));
    auto ret = AudioEffectChainManager::GetInstance()->UpdateSceneTypeList(sceneType, operation);
    EXPECT_EQ(ret, SUCCESS);
}

/**
* @tc.name   : Test UpdateSceneTypeList API
* @tc.number : UpdateSceneTypeList_004
* @tc.desc   : Test UpdateSceneTypeList interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSceneTypeList_004, TestSize.Level1)
{
    std::string sceneType = "test";
    int32_t operation = 5;
    SceneTypeOperation sceneTypeOperation = static_cast<SceneTypeOperation>(operation);
    AudioEffectChainManager::GetInstance()->sceneTypeCountList_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeCountList_.push_back(
        std::make_pair<std::string, int32_t>("test", 10));
    auto ret = AudioEffectChainManager::GetInstance()->UpdateSceneTypeList(sceneType, sceneTypeOperation);
    EXPECT_EQ(ret, ERROR);
}

/**
* @tc.name   : Test WaitAndReleaseEffectChain API
* @tc.number : WaitAndReleaseEffectChain_002
* @tc.desc   : Test WaitAndReleaseEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, WaitAndReleaseEffectChain_002, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_.clear();
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, false);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey],
        audioEffectChain);

    AudioEffectChainManager::GetInstance()->defaultEffectChainCount_ = 0;
    AudioEffectChainManager::GetInstance()->WaitAndReleaseEffectChain(sceneType, sceneTypeAndDeviceKey,
        defaultSceneTypeAndDeviceKey, 0);
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.empty());
}

/**
* @tc.name   : Test WaitAndReleaseEffectChain API
* @tc.number : WaitAndReleaseEffectChain_003
* @tc.desc   : Test WaitAndReleaseEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, WaitAndReleaseEffectChain_003, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_.clear();
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, false);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey],
        audioEffectChain);

    AudioEffectChainManager::GetInstance()->defaultEffectChainCount_ = 0;
    AudioEffectChainManager::GetInstance()->WaitAndReleaseEffectChain(sceneType, sceneTypeAndDeviceKey,
        defaultSceneTypeAndDeviceKey, 1);
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.empty());
}

/**
* @tc.name   : Test WaitAndReleaseEffectChain API
* @tc.number : WaitAndReleaseEffectChain_004
* @tc.desc   : Test WaitAndReleaseEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, WaitAndReleaseEffectChain_004, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_.clear();
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, false);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey],
        audioEffectChain);

    AudioEffectChainManager::GetInstance()->defaultEffectChainCount_ = 1;
    AudioEffectChainManager::GetInstance()->WaitAndReleaseEffectChain(sceneType, sceneTypeAndDeviceKey,
        defaultSceneTypeAndDeviceKey, 1);
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.empty());
}

/**
* @tc.name   : Test WaitAndReleaseEffectChain API
* @tc.number : WaitAndReleaseEffectChain_005
* @tc.desc   : Test WaitAndReleaseEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, WaitAndReleaseEffectChain_005, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_.clear();
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, false);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey],
        audioEffectChain);

    AudioEffectChainManager::GetInstance()->defaultEffectChainCount_ = 1;
    AudioEffectChainManager::GetInstance()->WaitAndReleaseEffectChain(sceneType, sceneTypeAndDeviceKey,
        defaultSceneTypeAndDeviceKey, 0);
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.empty());
}

/**
* @tc.name   : Test WaitAndReleaseEffectChain API
* @tc.number : WaitAndReleaseEffectChain_006
* @tc.desc   : Test WaitAndReleaseEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, WaitAndReleaseEffectChain_006, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_.clear();
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, false);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey] = nullptr;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey],
        audioEffectChain);

    AudioEffectChainManager::GetInstance()->defaultEffectChainCount_ = 0;
    AudioEffectChainManager::GetInstance()->WaitAndReleaseEffectChain(sceneType, sceneTypeAndDeviceKey,
        defaultSceneTypeAndDeviceKey, 0);
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.empty());
}

/**
* @tc.name   : Test WaitAndReleaseEffectChain API
* @tc.number : WaitAndReleaseEffectChain_007
* @tc.desc   : Test WaitAndReleaseEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, WaitAndReleaseEffectChain_007, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_.clear();
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, false);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey] = nullptr;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 0;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey],
        audioEffectChain);

    AudioEffectChainManager::GetInstance()->defaultEffectChainCount_ = 1;
    AudioEffectChainManager::GetInstance()->WaitAndReleaseEffectChain(sceneType, sceneTypeAndDeviceKey,
        defaultSceneTypeAndDeviceKey, 1);
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.empty());
}

/**
 * @tc.name   : Test ReleaseAudioEffectChainDynamicInner
 * @tc.number : ReleaseAudioEffectChainDynamicInner_002
 * @tc.desc   : Test ReleaseAudioEffectChainDynamicInner interface.
 */
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamicInner_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->ResetInfo();
    std::string sceneType = "test";
    std::string deviceKey = sceneType + "_&_" + AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[deviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[deviceKey] = 10;

    AudioEffectChainManager::GetInstance()->isInitialized_ = true;
    auto result = AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamicInner(sceneType);
    EXPECT_EQ(SUCCESS, result);
}

/**
 * @tc.name   : Test ReleaseAudioEffectChainDynamicInner
 * @tc.number : ReleaseAudioEffectChainDynamicInner_003
 * @tc.desc   : Test ReleaseAudioEffectChainDynamicInner interface.
 */
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamicInner_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->ResetInfo();
    std::string sceneType = "test";
    std::string deviceKey = sceneType + "_&_" + AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string defaultDeviceKey = DEFAULT_SCENE_TYPE + "_&_" +
        AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[deviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[deviceKey] = 10;

    AudioEffectChainManager::GetInstance()->isInitialized_ = true;
    auto result = AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamicInner(sceneType);
    EXPECT_EQ(SUCCESS, result);
}

/**
 * @tc.name   : Test ReleaseAudioEffectChainDynamicInner
 * @tc.number : ReleaseAudioEffectChainDynamicInner_004
 * @tc.desc   : Test ReleaseAudioEffectChainDynamicInner interface.
 */
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamicInner_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->ResetInfo();
    std::string sceneType = "test";
    std::string deviceKey = sceneType + "_&_" + AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string defaultDeviceKey = DEFAULT_SCENE_TYPE + "_&_" +
        AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[deviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultDeviceKey] = nullptr;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[deviceKey] = 10;

    AudioEffectChainManager::GetInstance()->isInitialized_ = true;
    auto result = AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamicInner(sceneType);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test ExistAudioEffectChainInner
* @tc.number : ExistAudioEffectChainInner_002
* @tc.desc   : Test ExistAudioEffectChainInner interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChainInner_002, TestSize.Level1)
{
    std::string sceneType = "test";
    std::string effectMode = "123";

    AudioEffectChainManager::GetInstance()->ResetInfo();
    AudioEffectChainManager::GetInstance()->isInitialized_ = true;
    auto result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChainInner(sceneType, effectMode);
    EXPECT_EQ(false, result);

    std::string sceneTypeAndMode = sceneType + "_&_" + effectMode + "_&_" +
        AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    AudioEffectChainManager::GetInstance()->sceneTypeAndModeToEffectChainNameMap_[sceneTypeAndMode] = "123456";

    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    ASSERT_TRUE(audioEffectChain != nullptr);
    audioEffectChain->standByEffectHandles_.resize(10);
    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChainInner(sceneType, effectMode);
    EXPECT_EQ(true, result);
}

/**
* @tc.name   : Test ExistAudioEffectChainInner
* @tc.number : ExistAudioEffectChainInner_003
* @tc.desc   : Test ExistAudioEffectChainInner interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChainInner_003, TestSize.Level1)
{
    std::string sceneType = "test";
    std::string effectMode = "123";

    AudioEffectChainManager::GetInstance()->ResetInfo();
    AudioEffectChainManager::GetInstance()->isInitialized_ = true;
    std::string sceneTypeAndMode = sceneType + "_&_" + effectMode + "_&_" +
        AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    AudioEffectChainManager::GetInstance()->sceneTypeAndModeToEffectChainNameMap_[sceneTypeAndMode] = "123456";
    auto result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChainInner(sceneType, effectMode);
    EXPECT_EQ(false, result);

    std::string sceneTypeAndDeviceKey = sceneType + "_&_" + AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = nullptr;
    result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChainInner(sceneType, effectMode);
    EXPECT_EQ(false, result);
}

/**
* @tc.name   : Test SetAbsVolumeStateToEffect API
* @tc.number : SetAbsVolumeStateToEffect_004
* @tc.desc   : Test SetAbsVolumeStateToEffect interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetAbsVolumeStateToEffect_001, TestSize.Level1)
{
    std::string scene = "SCENE_MUSIC";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);
    ASSERT_TRUE(audioEffectChain != nullptr);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({scene, audioEffectChain});
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({"1", nullptr});
    bool absVolumeState = true;
    int32_t ret = AudioEffectChainManager::GetInstance()->SetAbsVolumeStateToEffect(absVolumeState);
    EXPECT_EQ(ret, SUCCESS);
    absVolumeState = false;
    ret = AudioEffectChainManager::GetInstance()->SetAbsVolumeStateToEffect(absVolumeState);
    EXPECT_EQ(ret, SUCCESS);
    ret = AudioEffectChainManager::GetInstance()->EffectDspAbsVolumeStateUpdate(absVolumeState);
    EXPECT_EQ(ret, SUCCESS);
    ret = AudioEffectChainManager::GetInstance()->EffectApAbsVolumeStateUpdate(absVolumeState);
    EXPECT_EQ(ret, SUCCESS);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}
} // namespace AudioStandard
} // namespace OHOS
