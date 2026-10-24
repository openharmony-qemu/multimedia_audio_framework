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
constexpr int32_t INITIAL_DSP_STREAMUSAGE = -2;
constexpr int32_t DEFAULT_DSP_STREAMUSAGE = 1;
constexpr int32_t DEFAULT_STREAM_OR_VOLUME_TYPE = 1;
constexpr float INITIAL_DSP_VOLUME = -1.0f;
constexpr float DEFAULT_SYSTEM_VOLUME = 1.0f;
constexpr float DEFAULT_STREAM_VOLUME = 1.0f;
const std::string TEST_DEFAULT_SCENE_TYPE = "SCENE_DEFAULT";

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
    DEFAULT_DSP_STREAMUSAGE,
    DEFAULT_STREAM_OR_VOLUME_TYPE,
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
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(sceneType, eBufferAttr,
        outChannels, outChannelLayout);
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
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(sceneType, eBufferAttr,
        outChannels, outChannelLayout);
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
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(sceneType, eBufferAttr,
        outChannels, outChannelLayout);
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
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(sceneType, eBufferAttr,
        outChannels, outChannelLayout);
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
* @tc.name   : Test InitAudioEffectChainDynamic API
* @tc.number : InitAudioEffectChainDynamic_006
* @tc.desc   : Test flag is false
*/
HWTEST(AudioEffectChainManagerUnitTest, InitAudioEffectChainDynamic_006, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->isNeedInitEffectBuffer_ = false;

    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->isNeedInitEffectBuffer_);
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
    std::lock_guard<std::mutex> sensorLock(AudioEffectChainManager::GetInstance()->sensorMutex_);
    auto sensorAction = AudioEffectChainManager::GetInstance()->UpdateSensorStateLocked();
    AudioEffectChainManager::GetInstance()->ApplySensorActionLocked(sensorAction, true);
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
    std::lock_guard<std::mutex> sensorLock(AudioEffectChainManager::GetInstance()->sensorMutex_);
    auto sensorAction = AudioEffectChainManager::GetInstance()->UpdateSensorStateLocked();
    AudioEffectChainManager::GetInstance()->ApplySensorActionLocked(sensorAction, false);
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
    std::lock_guard<std::mutex> sensorLock(AudioEffectChainManager::GetInstance()->sensorMutex_);
    auto sensorAction = AudioEffectChainManager::GetInstance()->UpdateSensorStateLocked();
    AudioEffectChainManager::GetInstance()->ApplySensorActionLocked(sensorAction, false);
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
    AudioEffectChainManager::GetInstance()->currDspSceneType_ = SCENE_INITIAL;
    int32_t result = AudioEffectChainManager::GetInstance()->SetHdiParam(sceneType);
    
    if (result == SUCCESS) {
        EXPECT_EQ(AudioEffectChainManager::GetInstance()->currDspSceneType_, SCENE_MUSIC);
        int32_t resultTemp = AudioEffectChainManager::GetInstance()->SetHdiParam(sceneType);
        EXPECT_EQ(AudioEffectChainManager::GetInstance()->currDspSceneType_, SCENE_MUSIC);
        EXPECT_EQ(resultTemp, SUCCESS);
    } else if (result == ERROR) {
        EXPECT_EQ(AudioEffectChainManager::GetInstance()->currDspSceneType_, SCENE_INITIAL);
    }

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

    AudioEffectChainManager::GetInstance()->currDspSceneType_ = SCENE_INITIAL;
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t result = AudioEffectChainManager::GetInstance()->SetHdiParam(sceneType);

    if (result == SUCCESS) {
        EXPECT_EQ(AudioEffectChainManager::GetInstance()->currDspSceneType_, SCENE_MUSIC);
    } else if (result == ERROR) {
        EXPECT_EQ(AudioEffectChainManager::GetInstance()->currDspSceneType_, SCENE_INITIAL);
    }

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
    
    AudioEffectChainManager::GetInstance()->currDspSceneType_ = SCENE_INITIAL;
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    int32_t result = AudioEffectChainManager::GetInstance()->SetHdiParam(sceneType);

    if (result == SUCCESS) {
        EXPECT_EQ(AudioEffectChainManager::GetInstance()->currDspSceneType_, SCENE_OTHERS);
    } else if (result == ERROR) {
        EXPECT_EQ(AudioEffectChainManager::GetInstance()->currDspSceneType_, SCENE_INITIAL);
    }

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
* @tc.name   : Test NotifyAndCreateAudioEffectChain API
* @tc.number : NotifyAndCreateAudioEffectChain_002
* @tc.desc   : Test NotifyAndCreateAudioEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, NotifyAndCreateAudioEffectChain_002, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->maxEffectChainCount_ = 1;
    int32_t result = AudioEffectChainManager::GetInstance()->NotifyAndCreateAudioEffectChain(sceneType);
    EXPECT_EQ(SUCCESS, result);
    sceneType = "SCENE_VOIP_DOWN";
    AudioEffectChainManager::GetInstance()->priorSceneList_.push_back("SCENE_VOIP_DOWN");
    result = AudioEffectChainManager::GetInstance()->NotifyAndCreateAudioEffectChain(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test NotifyAndCreateAudioEffectChain API
* @tc.number : NotifyAndCreateAudioEffectChain_003
* @tc.desc   : Test NotifyAndCreateAudioEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, NotifyAndCreateAudioEffectChain_003, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->maxEffectChainCount_ = 1;
    int32_t result = AudioEffectChainManager::GetInstance()->NotifyAndCreateAudioEffectChain(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultSceneTypeAndDeviceKey] = 2;
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

    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_EARPIECE;
    result = AudioEffectChainManager::GetInstance()->UpdateDeviceInfo(device, sinkName);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_EARPIECE;
    AudioEffectChainManager::GetInstance()->SetOutputDeviceSink(device, sinkName);
    EXPECT_EQ(SUCCESS, result);
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
    AudioSpatializationState audioSpatializationState(true, false, false);
    AudioEffectChainManager::GetInstance()->UpdateSpatializationEnabled(audioSpatializationState);
    audioSpatializationState.spatializationEnabled = false;
    AudioEffectChainManager::GetInstance()->UpdateSpatializationEnabled(audioSpatializationState);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->UpdateSpatializationEnabled(audioSpatializationState);
    audioSpatializationState.spatializationEnabled = true;
    AudioEffectChainManager::GetInstance()->UpdateSpatializationEnabled(audioSpatializationState);
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = true;
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
    AudioEffectVolume::GetInstance()->SetStreamVolume("12345", 0.0);
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
* @tc.number : SetAudioEffectProperty_002
* @tc.desc   : Test SetAudioEffectProperty interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetAudioEffectProperty_002, TestSize.Level1)
{
    AudioEffectProperty  audioEffectProperty1 = {
        .name = "testName1",
        .category = "testCategory1",
    };

    AudioEffectProperty  audioEffectProperty2 = {
        .name = "testName2",
        .category = "testCategory2",
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
* @tc.name   : Test GetAudioEffectProperty
* @tc.number : GetAudioEffectProperty_002
* @tc.desc   : Test GetAudioEffectProperty interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetAudioEffectProperty_002, TestSize.Level1)
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
* @tc.number : SetSpatializationEnabledToChains_001
* @tc.desc   : Test SetSpatializationEnabledToChains interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetSpatializationEnabledToChains_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->ResetInfo();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({"test", nullptr});
    AudioEffectChainManager::GetInstance()->SetSpatializationEnabledToChains();
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.begin()->second == nullptr);

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(sceneType, headTracker);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->SetSpatializationEnabledToChains();
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->spatializationEnabled_,
        audioEffectChain->spatializationEnabledFading_);

    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = true;
    AudioEffectChainManager::GetInstance()->SetSpatializationEnabledToChains();
    EXPECT_EQ(!AudioEffectChainManager::GetInstance()->spatializationEnabled_,
        audioEffectChain->spatializationEnabledFading_);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetSpatializationEnabledToChains API
* @tc.number : SetSpatializationEnabledToChains_002
* @tc.desc   : Test SetSpatializationEnabledToChains interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetSpatializationEnabledToChains_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(sceneType, headTracker);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.insert({"123456", DEFAULT_INFO});
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->SetSpatializationEnabledToChains();
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->spatializationEnabled_,
        audioEffectChain->spatializationEnabledFading_);

    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = true;
    AudioEffectChainManager::GetInstance()->SetSpatializationEnabledToChains();
    EXPECT_EQ(!AudioEffectChainManager::GetInstance()->spatializationEnabled_,
        audioEffectChain->spatializationEnabledFading_);
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
    std::string sceneTypeMovie = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->NotifyAndCreateAudioEffectChain(sceneTypeMovie);

    std::string sceneType = "";
    std::set<std::string> sessions = {"test1"};
    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_NONE;
    AudioEffectChainManager::GetInstance()->FindMaxEffectChannels(sceneType, sessions, channels, channelLayout);
    EXPECT_EQ(channels, STEREO);
    channels = 10;
    AudioEffectChainManager::GetInstance()->FindMaxEffectChannels(sceneType, sessions, channels, channelLayout);

    SessionEffectInfo infoTemp;
    infoTemp.channels = static_cast<uint32_t>(CHANNEL_6);
    infoTemp.channelLayout = CH_LAYOUT_5POINT1;
    infoTemp.sceneMode = "EFFECT_DEFAULT";
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["test1"] = infoTemp;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectHandle handle = nullptr;
    std::string key = std::string("SCENE_MOVIE") + "_&_" + "DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key]->standByEffectHandles_.push_back(handle);
    sceneType = "SCENE_MOVIE";
    channels = static_cast<uint32_t>(STEREO);
    channelLayout = CH_LAYOUT_STEREO;

    AudioEffectChainManager::GetInstance()->FindMaxEffectChannels(sceneType, sessions, channels, channelLayout);
    EXPECT_EQ(channels, static_cast<uint32_t>(CHANNEL_6));
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
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
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
//
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultSceneTypeAndDeviceKey] = 2;
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
* @tc.name   : Test CheckAndReleaseCommonEffectChain API
* @tc.number : CheckAndReleaseCommonEffectChain_006
* @tc.desc   : Test CheckAndReleaseCommonEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndReleaseCommonEffectChain_006, TestSize.Level1)
{
    std::string sceneType = "test";
    std::string  scene = "SCENE_DEFAULT";

    std::string deviceTypeName = AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string effectChain0 = scene + "_&_" + deviceTypeName;
    std::string effectChain1 = sceneType + "_&_" + deviceTypeName;
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain0, audioEffectChain});
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain1, audioEffectChain});
    AudioEffectChainManager::GetInstance()->isNeedInitEffectBuffer_ = false;

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    auto ret = AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(sceneType);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->isNeedInitEffectBuffer_);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndReleaseCommonEffectChain API
* @tc.number : CheckAndReleaseCommonEffectChain_007
* @tc.desc   : Test CheckAndReleaseCommonEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndReleaseCommonEffectChain_007, TestSize.Level1)
{
    std::string sceneType = "test";
    std::string deviceTypeName = AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string effectChain1 = sceneType + "_&_" + deviceTypeName;
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(sceneType, headTracker);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain1, audioEffectChain});
    AudioEffectChainManager::GetInstance()->sceneTypeToSpecialEffectSet_.insert(sceneType);
    AudioEffectChainManager::GetInstance()->isNeedInitEffectBuffer_ = false;

    auto ret = AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(sceneType);
    EXPECT_EQ(ret, ERROR);
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->isNeedInitEffectBuffer_);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndReleaseCommonEffectChain API
* @tc.number : CheckAndReleaseCommonEffectChain_008
* @tc.desc   : Test CheckAndReleaseCommonEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndReleaseCommonEffectChain_008, TestSize.Level1)
{
    std::string sceneType = "test";
    std::string  scene = "SCENE_DEFAULT";

    std::string deviceTypeName = AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string effectChain0 = scene + "_&_" + deviceTypeName;
    std::string effectChain1 = sceneType + "_&_" + deviceTypeName;
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain0, audioEffectChain});
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain1, audioEffectChain});

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    auto ret = AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(sceneType);
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->isNeedInitEffectBuffer_);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndReleaseCommonEffectChain API
* @tc.number : CheckAndReleaseCommonEffectChain_009
* @tc.desc   : Test CheckAndReleaseCommonEffectChain interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndReleaseCommonEffectChain_009, TestSize.Level1)
{
    std::string sceneType = "test";
    std::string deviceTypeName = AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string effectChain1 = sceneType + "_&_" + deviceTypeName;
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(sceneType, headTracker);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effectChain1, audioEffectChain});
    AudioEffectChainManager::GetInstance()->sceneTypeToSpecialEffectSet_.insert(sceneType);
    auto ret = AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(sceneType);

    EXPECT_EQ(ret, ERROR);
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->isNeedInitEffectBuffer_);
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

/**
* @tc.name   : Test CheckProcessClusterInstances API
* @tc.number : CheckProcessClusterInstances_003
* @tc.desc   : Test CheckProcessClusterInstances interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckProcessClusterInstances_003, TestSize.Level1)
{
    std::string sceneType = "test";
    std::string  scene = "SCENE_DEFAULT";
    std::string effect = sceneType + "_&_" + AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string defaultScene = scene + "_&_" + AudioEffectChainManager::GetInstance()->GetDeviceTypeName();

    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>("123", headTracker);
    ASSERT_TRUE(audioEffectChain != nullptr);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effect, audioEffectChain});
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({defaultScene, audioEffectChain});

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    auto ret = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(sceneType);
    EXPECT_EQ(ret, CREATE_NEW_PROCESSCLUSTER);

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = false;
    ret = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(sceneType);
    EXPECT_EQ(ret, CREATE_NEW_PROCESSCLUSTER);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckProcessClusterInstances API
* @tc.number : CheckProcessClusterInstances_004
* @tc.desc   : Test CheckProcessClusterInstances interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckProcessClusterInstances_004, TestSize.Level1)
{
    std::string sceneType = "test";
    std::string  scene = "SCENE_DEFAULT";
    std::string effect = sceneType + "_&_" + AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string defaultScene = scene + "_&_" + AudioEffectChainManager::GetInstance()->GetDeviceTypeName();

    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>("123", headTracker);
    ASSERT_TRUE(audioEffectChain != nullptr);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effect, audioEffectChain});
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({defaultScene, nullptr});

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    auto ret = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(sceneType);
    EXPECT_EQ(ret, CREATE_NEW_PROCESSCLUSTER);

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = false;
    ret = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(sceneType);
    EXPECT_EQ(ret, CREATE_NEW_PROCESSCLUSTER);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckProcessClusterInstances API
* @tc.number : CheckProcessClusterInstances_005
* @tc.desc   : Test CheckProcessClusterInstances interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckProcessClusterInstances_005, TestSize.Level1)
{
    std::string sceneType = "test";
    std::string  scene = "SCENE_DEFAULT";
    std::string effect = sceneType + "_&_" + AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string defaultScene = scene + "_&_" + AudioEffectChainManager::GetInstance()->GetDeviceTypeName();

    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>("123", headTracker);
    ASSERT_TRUE(audioEffectChain != nullptr);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({effect, audioEffectChain});
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[effect] = 1;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({defaultScene, audioEffectChain});

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    auto ret = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(sceneType);
    EXPECT_EQ(ret, USE_DEFAULT_PROCESSCLUSTER);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultScene] = nullptr;
    ret = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(sceneType);
    EXPECT_EQ(ret, NO_NEED_TO_CREATE_PROCESSCLUSTER);

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = false;
    ret = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(sceneType);
    EXPECT_EQ(ret, NO_NEED_TO_CREATE_PROCESSCLUSTER);
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

    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_NEARLINK;
    result = AudioEffectChainManager::GetInstance()->EffectVolumeUpdateInner(audioEffectVolume);
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
    auto sensorAction = AudioEffectChainManager::SensorAction::SENSOR_ACTION_NONE;
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateSpatializationStateInner(spatializationState,
        sensorAction);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = false;
    result = AudioEffectChainManager::GetInstance()->UpdateSpatializationStateInner(spatializationState,
        sensorAction);
    EXPECT_EQ(SUCCESS, result);
}

/**
* @tc.name   : Test concurrent head tracking flip ordering
* @tc.number : UpdateSpatializationState_Concurrent_001
* @tc.desc   : Two threads flip headTrackingEnabled in opposite directions; the entry-level
*              sensorMutex_ serialization keeps decide+apply ordered and deadlock-free.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSpatializationState_Concurrent_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    auto flipper = [](bool headTracking) {
        AudioSpatializationState spatializationState = {true, headTracking};
        for (int32_t i = 0; i < 50; ++i) {
            AudioEffectChainManager::GetInstance()->UpdateSpatializationState(spatializationState);
        }
    };
    std::thread enableThread(flipper, true);
    std::thread disableThread(flipper, false);
    enableThread.join();
    disableThread.join();
    // Joins returning means no deadlock; a final full-path call still succeeds afterwards.
    AudioSpatializationState finalState = {true, true};
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->UpdateSpatializationState(finalState));
    AudioEffectChainManager::GetInstance()->ResetInfo();
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
* @tc.name   : Test UpdateParamExtra API
* @tc.number : UpdateParamExtra_003
* @tc.desc   : Test UpdateParamExtra interface, key is systemLoad_state.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateParamExtra_003, TestSize.Level1)
{
    std::string mainkey = "audio_effect";
    std::string subkey = SYSTEM_LOAD_SUBKEY;
    std::string value = "0";
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, value);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->systemLoadState_, value);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->effectHdiInput_[0], HDI_SYSTEMLOAD_STATE);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->effectHdiInput_[1], 0);

    value = "1";
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, value);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->systemLoadState_, value);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->effectHdiInput_[0], HDI_SYSTEMLOAD_STATE);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->effectHdiInput_[1], 1);
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
    AudioEffectProperty  audioEffectProperty1 = {
        .name = "testName1",
        .category = "testCategory1",
    };

    AudioEffectProperty  audioEffectProperty2 = {
        .name = "testName2",
        .category = "testCategory2",
    };

    AudioEffectPropertyArray audioEffectPropertyArray = {};
    audioEffectPropertyArray.property.push_back(audioEffectProperty1);
    audioEffectPropertyArray.property.push_back(audioEffectProperty2);

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain = nullptr;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    int32_t ret = AudioEffectChainManager::GetInstance()->SetAudioEffectProperty(audioEffectPropertyArray);
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
    AudioEffectPropertyArray audioEffectPropertyArray = {};
    std::string property1 = "123";
    std::string property2 = "";
    AudioEffectChainManager::GetInstance()->effectPropertyMap_.insert({property1, property2});
    int32_t ret = AudioEffectChainManager::GetInstance()->GetAudioEffectProperty(audioEffectPropertyArray);
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
    AudioEffectPropertyArray audioEffectPropertyArray = {};
    std::string property1 = "123";
    std::string property2 = "test";
    AudioEffectChainManager::GetInstance()->effectPropertyMap_.insert({property1, property2});
    int32_t ret = AudioEffectChainManager::GetInstance()->GetAudioEffectProperty(audioEffectPropertyArray);
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

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultSceneTypeAndDeviceKey] = 0;
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

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultSceneTypeAndDeviceKey] = 0;
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

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultSceneTypeAndDeviceKey] = 1;
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

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultSceneTypeAndDeviceKey] = 1;
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

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultSceneTypeAndDeviceKey] = 0;
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

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultSceneTypeAndDeviceKey] = 1;
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
    std::string defaultDeviceKey = TEST_DEFAULT_SCENE_TYPE + "_&_" +
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
    std::string defaultDeviceKey = TEST_DEFAULT_SCENE_TYPE + "_&_" +
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

/**
* @tc.name   : Test ExistAudioEffectChainArm API
* @tc.number : ExistAudioEffectChainArm_001
* @tc.desc   : Test ExistAudioEffectChainArm interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChainArm_001, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    AudioEffectMode effectMode = EFFECT_NONE;
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(sceneType, headTracker);
    ASSERT_TRUE(audioEffectChain != nullptr);

    int32_t ret = AudioEffectChainManager::GetInstance()->ExistAudioEffectChainArm(sceneType, effectMode);
    EXPECT_EQ(ret, false);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChainArm API
* @tc.number : ExistAudioEffectChainArm_002
* @tc.desc   : Test ExistAudioEffectChainArm interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChainArm_002, TestSize.Level1)
{
    std::string sceneType = "SCENE_UNKNOWN";
    AudioEffectMode effectMode = EFFECT_DEFAULT;
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(sceneType, headTracker);
    ASSERT_TRUE(audioEffectChain != nullptr);

    int32_t ret = AudioEffectChainManager::GetInstance()->ExistAudioEffectChainArm(sceneType, effectMode);
    EXPECT_EQ(ret, false);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChainArm API
* @tc.number : ExistAudioEffectChainArm_003
* @tc.desc   : Test ExistAudioEffectChainArm interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChainArm_003, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    AudioEffectMode effectMode = EFFECT_DEFAULT;
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(sceneType, headTracker);
    ASSERT_TRUE(audioEffectChain != nullptr);

    std::string deviceType = AudioEffectChainManager::GetInstance()->GetDeviceTypeName();
    std::string effectChainKey = sceneType + "_&_EFFECT_DEFAULT_&_" + deviceType;
    int32_t ret = AudioEffectChainManager::GetInstance()->ExistAudioEffectChainArm(sceneType, effectMode);
    
    if (!AudioEffectChainManager::GetInstance()->sceneTypeAndModeToEffectChainNameMap_.count(effectChainKey)) {
        EXPECT_EQ(ret, false);
    } else {
        EXPECT_EQ(ret, true);
    }
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test IsChannelLayoutSupportedForDspEffect API
* @tc.number : IsChannelLayoutSupportedForDspEffect_004
* @tc.desc   : Test IsChannelLayoutSupportedForDspEffect interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, IsChannelLayoutSupportedForDspEffect_001, TestSize.Level1)
{
    std::shared_ptr<AudioEffectHdiParam> audioEffectHdiParam = std::make_shared<AudioEffectHdiParam>();
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = audioEffectHdiParam;
    AudioEffectChainManager::GetInstance()->InitHdiState();
    bool ret = AudioEffectChainManager::GetInstance()->IsChannelLayoutSupportedForDspEffect(CH_LAYOUT_6POINT0_FRONT);
    EXPECT_EQ(ret, false);

    AudioEffectChainManager::GetInstance()->ResetInfo();
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
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->EffectDspVolumeUpdate(audioEffectVolume), SUCCESS);
    EXPECT_EQ(audioEffectVolume->GetDspVolume(), INITIAL_DSP_VOLUME);

    const std::string sessionID = "123456";
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    audioEffectVolume->SetSystemVolume(DEFAULT_STREAM_OR_VOLUME_TYPE, DEFAULT_SYSTEM_VOLUME);
    audioEffectVolume->SetStreamVolume(sessionID, DEFAULT_STREAM_VOLUME);
    EXPECT_EQ(audioEffectVolume->GetSystemVolume(DEFAULT_STREAM_OR_VOLUME_TYPE), DEFAULT_SYSTEM_VOLUME);
    EXPECT_EQ(audioEffectVolume->GetStreamVolume(sessionID), DEFAULT_STREAM_VOLUME);

    int32_t ret = AudioEffectChainManager::GetInstance()->EffectDspVolumeUpdate(audioEffectVolume);
    if (ret == SUCCESS) {
        EXPECT_EQ(audioEffectVolume->GetDspVolume(), DEFAULT_SYSTEM_VOLUME * DEFAULT_STREAM_VOLUME);
    } else if (ret == ERROR) {
        EXPECT_EQ(audioEffectVolume->GetDspVolume(), INITIAL_DSP_VOLUME);
    }

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test EffectDspVolumeUpdate API
* @tc.number : EffectDspVolumeUpdate_002
* @tc.desc   : Test EffectDspVolumeUpdate interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, EffectDspVolumeUpdate_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->EffectDspVolumeUpdate(audioEffectVolume), SUCCESS);
    EXPECT_EQ(audioEffectVolume->GetDspVolume(), INITIAL_DSP_VOLUME);

    const std::string sessionID = "123456";
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    audioEffectVolume->SetSystemVolume(DEFAULT_STREAM_OR_VOLUME_TYPE, DEFAULT_SYSTEM_VOLUME);
    audioEffectVolume->SetStreamVolume(sessionID, DEFAULT_STREAM_VOLUME);
    EXPECT_EQ(audioEffectVolume->GetSystemVolume(DEFAULT_STREAM_OR_VOLUME_TYPE), DEFAULT_SYSTEM_VOLUME);
    EXPECT_EQ(audioEffectVolume->GetStreamVolume(sessionID), DEFAULT_STREAM_VOLUME);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->EffectDspVolumeUpdate(audioEffectVolume);
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd(sessionID, DEFAULT_INFO);
    int32_t ret = AudioEffectChainManager::GetInstance()->EffectDspVolumeUpdate(audioEffectVolume);
    if (ret == SUCCESS) {
        EXPECT_EQ(audioEffectVolume->GetDspVolume(), DEFAULT_SYSTEM_VOLUME * DEFAULT_STREAM_VOLUME);
        int32_t retTemp = AudioEffectChainManager::GetInstance()->EffectDspVolumeUpdate(audioEffectVolume);
        EXPECT_EQ(audioEffectVolume->GetDspVolume(), DEFAULT_SYSTEM_VOLUME * DEFAULT_STREAM_VOLUME);
        EXPECT_EQ(retTemp, SUCCESS);
    } else if (ret == ERROR) {
        EXPECT_EQ(audioEffectVolume->GetDspVolume(), INITIAL_DSP_VOLUME);
    }

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateCurrSceneTypeAndStreamUsageForDsp API
* @tc.number : UpdateCurrSceneTypeAndStreamUsageForDsp_001
* @tc.desc   : Test UpdateCurrSceneTypeAndStreamUsageForDsp interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateCurrSceneTypeAndStreamUsageForDsp_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->maxSessionID_ = static_cast<uint32_t>(std::stoul("123456"));
    AudioEffectChainManager::GetInstance()->maxSessionIDToSceneType_ = "SCENE_MUSIC";
    AudioEffectChainManager::GetInstance()->currDspStreamUsage_ = INITIAL_DSP_STREAMUSAGE;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["123456"] = DEFAULT_INFO;
    int32_t result = AudioEffectChainManager::GetInstance()->UpdateCurrSceneTypeAndStreamUsageForDsp();
    
    if (result == SUCCESS) {
        EXPECT_EQ(AudioEffectChainManager::GetInstance()->currDspStreamUsage_, DEFAULT_DSP_STREAMUSAGE);
        int32_t resultTemp = AudioEffectChainManager::GetInstance()->UpdateCurrSceneTypeAndStreamUsageForDsp();
        EXPECT_EQ(AudioEffectChainManager::GetInstance()->currDspStreamUsage_, DEFAULT_DSP_STREAMUSAGE);
        EXPECT_EQ(resultTemp, SUCCESS);
    } else if (result == ERROR) {
        EXPECT_EQ(AudioEffectChainManager::GetInstance()->currDspStreamUsage_, INITIAL_DSP_STREAMUSAGE);
    }

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateEarphoneProduct API
* @tc.number : UpdateEarphoneProduct_004
* @tc.desc   : Test UpdateEarphoneProduct interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateEarphoneProduct_001, TestSize.Level1)
{
    std::string scene = "SCENE_MUSIC";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);
    ASSERT_TRUE(audioEffectChain != nullptr);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({scene, audioEffectChain});
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({"1", nullptr});
    int32_t earphoneProduct = 1;
    AudioEffectChainManager::GetInstance()->UpdateEarphoneProduct(earphoneProduct);
    EXPECT_EQ(audioEffectChain->earphoneProduct_, earphoneProduct);
    earphoneProduct = 2;
    AudioEffectChainManager::GetInstance()->UpdateEarphoneProduct(earphoneProduct);
    EXPECT_EQ(audioEffectChain->earphoneProduct_, earphoneProduct);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetOutputChannelInfo API
* @tc.number : GetOutputChannelInfo_001
* @tc.desc   : Test GetOutputChannelInfo interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetOutputChannelInfo_001, TestSize.Level1)
{
    std::string scene = "SCENE_MUSIC";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);

    uint32_t channels = 1;
    uint64_t channelLayout = 1;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({scene, audioEffectChain});
    AudioEffectChainManager::GetInstance()->GetOutputChannelInfo(scene, channels, channelLayout);
    ASSERT_TRUE(audioEffectChain != nullptr);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test DeleteAllChains API
* @tc.number : DeleteAllChains_001
* @tc.desc   : Test DeleteAllChains interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, DeleteAllChains_001, TestSize.Level1)
{
    std::string scene = "SCENE_MUSIC";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({scene, audioEffectChain});
    AudioEffectChainManager::GetInstance()->DeleteAllChains();
    ASSERT_TRUE(audioEffectChain != nullptr);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateParamExtra_frontendAppType API
* @tc.number : UpdateParamExtra_frontendAppType_001
* @tc.desc   : Test UpdateParamExtra_frontendAppType interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateParamExtra_frontendAppType_001, TestSize.Level1)
{
    HdiSetParamCommandCode code = HDI_FRONTEND_APP_TYPE;
    std::string value = "test";
    std::string scene = "123";
    std::string mainkey = "audio_effect";
    std::string mainkeyError = "audio_effect_error";
    std::string subkey = "update_audio_effect_hvs_type";
    std::string subkeyError = "update_audio_effect_hvs_type_error";
    auto headTracker = std::make_shared<HeadTracker>();
    std::shared_ptr<AudioEffectChain> audioEffectChain = std::make_shared<AudioEffectChain>(scene, headTracker);

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.insert({scene, audioEffectChain});
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, value);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->frontendAppType_ == value, true);
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkeyError, subkey, scene);
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkeyError, scene);
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkeyError, subkeyError, scene);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->frontendAppType_ == scene, false);
    AudioEffectChainManager::GetInstance()->SendAudioParamToARM(code, value);
    ASSERT_TRUE(audioEffectChain != nullptr);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdatePersonalizedHRTFBinToArm API
* @tc.number : UpdatePersonalizedHRTFBinToArm_001
* @tc.desc   : Test UpdatePersonalizedHRTFBinToArm interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdatePersonalizedHRTFBinToArm_001, TestSize.Level1)
{
    int32_t fd = -1;
    long length = 0;
    int32_t ret = AudioEffectChainManager::GetInstance()->UpdatePersonalizedHRTFBinToArm(fd, length);
    EXPECT_EQ(ret, ERR_SAVE_HRTF_FAIL);
    length = 1;
    ret = AudioEffectChainManager::GetInstance()->UpdatePersonalizedHRTFBinToArm(fd, length);
    EXPECT_EQ(ret, ERR_SAVE_HRTF_FAIL);
    fd = 0;
    length = 0;
    ret = AudioEffectChainManager::GetInstance()->UpdatePersonalizedHRTFBinToArm(fd, length);
    EXPECT_EQ(ret, ERR_SAVE_HRTF_FAIL);
    length = MAX_HRTF_LENGTH + 1;
    ret = AudioEffectChainManager::GetInstance()->UpdatePersonalizedHRTFBinToArm(fd, length);
    EXPECT_EQ(ret, ERR_SAVE_HRTF_FAIL);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// ==================== New UT for per-renderId API overloads ====================

/**
* @tc.name   : Test GenerateEffectChainKey API
* @tc.number : GenerateEffectChainKey_001
* @tc.desc   : Test GenerateEffectChainKey returns correct key format.
*/
HWTEST(AudioEffectChainManagerUnitTest, GenerateEffectChainKey_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    std::string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        "SCENE_MUSIC", DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(key, "DEVICE_TYPE_SPEAKER_&_SCENE_MUSIC_&_100");

    std::string key2 = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        "SCENE_MOVIE", DEVICE_TYPE_BLUETOOTH_A2DP, 200);
    EXPECT_EQ(key2, "DEVICE_TYPE_BLUETOOTH_A2DP_&_SCENE_MOVIE_&_200");

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GenerateEffectChainKey with invalid device type
* @tc.number : GenerateEffectChainKey_002
* @tc.desc   : Test GenerateEffectChainKey with DEVICE_TYPE_INVALID returns empty device name.
*/
HWTEST(AudioEffectChainManagerUnitTest, GenerateEffectChainKey_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    std::string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        "SCENE_MUSIC", DEVICE_TYPE_INVALID, 0);
    EXPECT_EQ(key.find("_&_SCENE_MUSIC_&_0") != string::npos, true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetDeviceTypeName with DeviceType parameter
* @tc.number : GetDeviceTypeName_WithDeviceType_001
* @tc.desc   : Test GetDeviceTypeName(DeviceType) returns correct device name.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetDeviceTypeName_WithDeviceType_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    std::string name1 = AudioEffectChainManager::GetInstance()->GetDeviceTypeName(DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(name1 == "DEVICE_TYPE_SPEAKER", true);

    std::string name2 = AudioEffectChainManager::GetInstance()->GetDeviceTypeName(DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(name2 == "DEVICE_TYPE_BLUETOOTH_A2DP", true);

    std::string name3 = AudioEffectChainManager::GetInstance()->GetDeviceTypeName(
        static_cast<DeviceType>(9999));
    EXPECT_EQ(name3.empty(), true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CreateAudioEffectChainDynamic with renderId - empty scene
* @tc.number : CreateAudioEffectChainDynamic_RenderId_001
* @tc.desc   : Test CreateAudioEffectChainDynamic with renderId returns ERROR for empty scene.
*/
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChainDynamic_RenderId_001, TestSize.Level1)
{
    string sceneType = "";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    int32_t result = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CreateAudioEffectChainDynamic with renderId - success
* @tc.number : CreateAudioEffectChainDynamic_RenderId_002
* @tc.desc   : Test CreateAudioEffectChainDynamic with renderId creates chain successfully.
*/
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChainDynamic_RenderId_002, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    int32_t result = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);

    // Verify chain exists in map
    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.count(key) > 0, true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->renderIdToEffectChainKeysMap_[100].count(key) > 0, true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CreateAudioEffectChainDynamic with renderId - duplicate create increments count
* @tc.number : CreateAudioEffectChainDynamic_RenderId_003
* @tc.desc   : Test CreateAudioEffectChainDynamic with same params increments chain count.
*/
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChainDynamic_RenderId_003, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    int32_t result1 = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result1);

    int32_t result2 = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result2);

    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[key], 2);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReleaseAudioEffectChainDynamic with renderId - empty scene
* @tc.number : ReleaseAudioEffectChainDynamic_RenderId_001
* @tc.desc   : Test ReleaseAudioEffectChainDynamic with renderId returns ERROR for empty scene.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamic_RenderId_001, TestSize.Level1)
{
    string sceneType = "";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    int32_t result = AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReleaseAudioEffectChainDynamic with renderId - release after create
* @tc.number : ReleaseAudioEffectChainDynamic_RenderId_002
* @tc.desc   : Test ReleaseAudioEffectChainDynamic decrements chain count correctly.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamic_RenderId_002, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[key], 2);

    int32_t result = AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[key], 1);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReleaseAudioEffectChainDynamic with renderId - release unknown chain
* @tc.number : ReleaseAudioEffectChainDynamic_RenderId_003
* @tc.desc   : Test ReleaseAudioEffectChainDynamic for non-existent chain returns SUCCESS.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamic_RenderId_003, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    int32_t result = AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 999);
    EXPECT_EQ(SUCCESS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChain with renderId - chain not exists
* @tc.number : ExistAudioEffectChain_RenderId_001
* @tc.desc   : Test ExistAudioEffectChain with renderId returns false for non-existent chain.
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChain_RenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    bool result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(
        "SCENE_MOVIE", "EFFECT_DEFAULT", DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(result, false);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChain with renderId - chain exists
* @tc.number : ExistAudioEffectChain_RenderId_002
* @tc.desc   : Test ExistAudioEffectChain with renderId returns true after chain creation.
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChain_RenderId_002, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    bool result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(
        sceneType, "EFFECT_DEFAULT", DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(result, true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test InitAudioEffectChainDynamic with renderId
* @tc.number : InitAudioEffectChainDynamic_RenderId_001
* @tc.desc   : Test InitAudioEffectChainDynamic with renderId succeeds after creating chain.
*/
HWTEST(AudioEffectChainManagerUnitTest, InitAudioEffectChainDynamic_RenderId_001, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test InitAudioEffectChainDynamic with renderId - empty scene
* @tc.number : InitAudioEffectChainDynamic_RenderId_002
* @tc.desc   : Test InitAudioEffectChainDynamic with renderId returns ERROR for empty scene.
*/
HWTEST(AudioEffectChainManagerUnitTest, InitAudioEffectChainDynamic_RenderId_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(
        "", DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(ERROR, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test InitAudioEffectChainDynamic with renderId - empty scene
* @tc.number : InitAudioEffectChainDynamic_RenderId_003
* @tc.desc   : Test flag is false
*/
HWTEST(AudioEffectChainManagerUnitTest, InitAudioEffectChainDynamic_RenderId_003, TestSize.Level1)
{
    string sceneType = "SCENE_MUSIC";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->isNeedInitEffectBuffer_ = false;

    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 0);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->isNeedInitEffectBuffer_);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateMultichannelConfig with renderId - chain not found
* @tc.number : UpdateMultichannelConfig_RenderId_001
* @tc.desc   : Test UpdateMultichannelConfig with renderId returns ERROR for non-existent chain.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateMultichannelConfig_RenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    int32_t result = AudioEffectChainManager::GetInstance()->UpdateMultichannelConfig(
        "SCENE_MOVIE", DEVICE_TYPE_SPEAKER, 999);
    EXPECT_EQ(ERROR, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateMultichannelConfig with renderId - success
* @tc.number : UpdateMultichannelConfig_RenderId_002
* @tc.desc   : Test UpdateMultichannelConfig with renderId succeeds after chain creation.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateMultichannelConfig_RenderId_002, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    int32_t result = AudioEffectChainManager::GetInstance()->UpdateMultichannelConfig(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckProcessClusterInstances with renderId - SCENE_EXTRA
* @tc.number : CheckProcessClusterInstances_RenderId_001
* @tc.desc   : Test CheckProcessClusterInstances with renderId returns CREATE_EXTRA for SCENE_EXTRA.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckProcessClusterInstances_RenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    int32_t result = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(
        "SCENE_EXTRA", DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(result, CREATE_EXTRA_PROCESSCLUSTER);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckProcessClusterInstances with renderId - new scene
* @tc.number : CheckProcessClusterInstances_RenderId_002
* @tc.desc   : Test CheckProcessClusterInstances with renderId returns CREATE_NEW for unknown scene.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckProcessClusterInstances_RenderId_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    int32_t result = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(
        "SCENE_MUSIC", DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(result, CREATE_NEW_PROCESSCLUSTER);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckProcessClusterInstances with renderId - existing chain
* @tc.number : CheckProcessClusterInstances_RenderId_003
* @tc.desc   : Test CheckProcessClusterInstances with renderId for existing non-default chain.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckProcessClusterInstances_RenderId_003, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    int32_t result = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(result, NO_NEED_TO_CREATE_PROCESSCLUSTER);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test QueryEffectChannelInfo with renderId - chain not found
* @tc.number : QueryEffectChannelInfo_RenderId_001
* @tc.desc   : Test QueryEffectChannelInfo with renderId returns ERROR for non-existent chain.
*/
HWTEST(AudioEffectChainManagerUnitTest, QueryEffectChannelInfo_RenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    int32_t result = AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(
        "SCENE_MOVIE", DEVICE_TYPE_SPEAKER, 999, channels, channelLayout);
    EXPECT_EQ(ERROR, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChainArm with renderId - EFFECT_NONE mode
* @tc.number : ExistAudioEffectChainArm_RenderId_001
* @tc.desc   : Test ExistAudioEffectChainArm with renderId returns false for EFFECT_NONE mode.
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChainArm_RenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    bool result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChainArm(
        "SCENE_MOVIE", EFFECT_NONE, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(result, false);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChainArm with renderId - no chain config
* @tc.number : ExistAudioEffectChainArm_RenderId_002
* @tc.desc   : Test ExistAudioEffectChainArm with renderId returns false when no chain mapping exists.
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChainArm_RenderId_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    bool result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChainArm(
        "SCENE_MUSIC", EFFECT_DEFAULT, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(result, false);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetOutputChannelInfo with renderId - chain not found
* @tc.number : GetOutputChannelInfo_RenderId_001
* @tc.desc   : Test GetOutputChannelInfo with renderId returns ERROR for non-existent chain.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetOutputChannelInfo_RenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    int32_t result = AudioEffectChainManager::GetInstance()->GetOutputChannelInfo(
        "SCENE_MOVIE", DEVICE_TYPE_SPEAKER, 999, channels, channelLayout);
    EXPECT_EQ(ERROR, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetOutputChannelInfo with renderId - success
* @tc.number : GetOutputChannelInfo_RenderId_002
* @tc.desc   : Test GetOutputChannelInfo with renderId succeeds after chain creation.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetOutputChannelInfo_RenderId_002, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    int32_t result = AudioEffectChainManager::GetInstance()->GetOutputChannelInfo(
        sceneType, DEVICE_TYPE_SPEAKER, 100, channels, channelLayout);
    EXPECT_EQ(SUCCESS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SessionEffectInfo new fields renderId and deviceType
* @tc.number : SessionEffectInfo_NewFields_001
* @tc.desc   : Test SessionEffectInfo stores renderId and deviceType correctly.
*/
HWTEST(AudioEffectChainManagerUnitTest, SessionEffectInfo_NewFields_001, TestSize.Level1)
{
    SessionEffectInfo info;
    info.sceneMode = "EFFECT_DEFAULT";
    info.sceneType = "SCENE_MOVIE";
    info.channels = INFOCHANNELS;
    info.channelLayout = INFOCHANNELLAYOUT;
    info.streamUsage = DEFAULT_DSP_STREAMUSAGE;
    info.systemVolumeType = DEFAULT_STREAM_OR_VOLUME_TYPE;
    info.renderId = 100;
    info.deviceType = DEVICE_TYPE_SPEAKER;

    EXPECT_EQ(info.renderId, 100u);
    EXPECT_EQ(info.deviceType, DEVICE_TYPE_SPEAKER);

    // Test default values
    SessionEffectInfo defaultInfo;
    EXPECT_EQ(defaultInfo.renderId, 0u);
    EXPECT_EQ(defaultInfo.deviceType, DEVICE_TYPE_INVALID);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test multiple renderId chains are independent
* @tc.number : MultiRenderId_Independent_001
* @tc.desc   : Test chains with different renderIds are independent in the map.
*/
HWTEST(AudioEffectChainManagerUnitTest, MultiRenderId_Independent_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";

    // Create chains for two different renderIds on the same device
    int32_t result1 = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result1);

    int32_t result2 = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 200);
    EXPECT_EQ(SUCCESS, result2);

    // Verify both chains exist independently
    string key1 = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    string key2 = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 200);

    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.count(key1) > 0, true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.count(key2) > 0, true);
    EXPECT_NE(key1, key2);

    // Verify renderIdToEffectChainKeysMap_ tracks correctly
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->renderIdToEffectChainKeysMap_[100].count(key1) > 0, true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->renderIdToEffectChainKeysMap_[200].count(key2) > 0, true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test same scene different device types are independent
* @tc.number : MultiRenderId_Independent_002
* @tc.desc   : Test chains with same renderId but different device types are independent.
*/
HWTEST(AudioEffectChainManagerUnitTest, MultiRenderId_Independent_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";

    int32_t result1 = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result1);

    int32_t result2 = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_BLUETOOTH_A2DP, 100);
    EXPECT_EQ(SUCCESS, result2);

    string key1 = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    string key2 = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_BLUETOOTH_A2DP, 100);

    EXPECT_NE(key1, key2);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.count(key1) > 0, true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.count(key2) > 0, true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test renderIdToEffectChainKeysMap_ cleanup on release
* @tc.number : MultiRenderId_Cleanup_001
* @tc.desc   : Test renderIdToEffectChainKeysMap_ is cleaned up when chain is released.
*/
HWTEST(AudioEffectChainManagerUnitTest, MultiRenderId_Cleanup_001, TestSize.Level1)
{
    // Clear stale entries left by previous tests (ResetInfo doesn't clear this map)
    AudioEffectChainManager::GetInstance()->renderIdToEffectChainKeysMap_.clear();

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->renderIdToEffectChainKeysMap_.count(100) > 0, true);

    AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    // After full release, renderId entry should be cleaned up
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->renderIdToEffectChainKeysMap_.count(100), 0);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

// ==================== Additional UTs for uncovered branches ====================

/**
* @tc.name   : Test ApplyAudioEffectChain with renderId - chain not found
* @tc.number : ApplyAudioEffectChain_RenderId_001
* @tc.desc   : Test ApplyAudioEffectChain with renderId returns ERROR when chain not found.
*/
HWTEST(AudioEffectChainManagerUnitTest, ApplyAudioEffectChain_RenderId_001, TestSize.Level1)
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
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;

    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(
        sceneType, DEVICE_TYPE_SPEAKER, 100, eBufferAttr, outChannels, outChannelLayout);
    EXPECT_EQ(ERROR, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ApplyAudioEffectChain with renderId - success
* @tc.number : ApplyAudioEffectChain_RenderId_002
* @tc.desc   : Test ApplyAudioEffectChain with renderId succeeds after chain creation.
*/
HWTEST(AudioEffectChainManagerUnitTest, ApplyAudioEffectChain_RenderId_002, TestSize.Level1)
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
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;

    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(
        sceneType, DEVICE_TYPE_SPEAKER, 100, eBufferAttr, outChannels, outChannelLayout);
    EXPECT_EQ(SUCCESS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ApplyAudioEffectChain API with offload enabled
* @tc.number : ApplyAudioEffectChain_OffloadBypass_001
* @tc.desc   : Test ApplyAudioEffectChain returns EFFECT_OFFLOAD_BYPASS when spkOffloadEnabled_ is true.
*/
HWTEST(AudioEffectChainManagerUnitTest, ApplyAudioEffectChain_OffloadBypass_001, TestSize.Level1)
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
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;

    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(sceneType, eBufferAttr,
        outChannels, outChannelLayout);
    EXPECT_EQ(EFFECT_OFFLOAD_BYPASS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ApplyAudioEffectChain with renderId and offload enabled
* @tc.number : ApplyAudioEffectChain_OffloadBypass_RenderId_001
* @tc.desc   : Test ApplyAudioEffectChain with renderId returns EFFECT_OFFLOAD_BYPASS when spkOffloadEnabled_ is true.
*/
HWTEST(AudioEffectChainManagerUnitTest, ApplyAudioEffectChain_OffloadBypass_RenderId_001, TestSize.Level1)
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
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;

    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(
        sceneType, DEVICE_TYPE_SPEAKER, 100, eBufferAttr, outChannels, outChannelLayout);
    EXPECT_EQ(EFFECT_OFFLOAD_BYPASS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ApplyAudioEffectChain API with channel change
* @tc.number : ApplyAudioEffectChain_NeedReconfig_001
* @tc.desc   : Test ApplyAudioEffectChain returns EFFECT_NEED_RECONFIG when output channel differs from chain config.
*/
HWTEST(AudioEffectChainManagerUnitTest, ApplyAudioEffectChain_NeedReconfig_001, TestSize.Level1)
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
    uint32_t outChannels = 1;
    uint64_t outChannelLayout = 0x4;
    auto eBufferAttr = make_unique<EffectBufferAttr>(bufIn, bufOut, numChans, frameLen, outChannels, outChannelLayout);
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;

    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(sceneType, eBufferAttr,
        outChannels, outChannelLayout);
    EXPECT_EQ(EFFECT_NEED_RECONFIG, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ApplyAudioEffectChain with renderId and channel change
* @tc.number : ApplyAudioEffectChain_NeedReconfig_RenderId_001
* @tc.desc   : Test ApplyAudioEffectChain with renderId returns EFFECT_NEED_RECONFIG when output channel differs.
*/
HWTEST(AudioEffectChainManagerUnitTest, ApplyAudioEffectChain_NeedReconfig_RenderId_001, TestSize.Level1)
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
    uint32_t outChannels = 1;
    uint64_t outChannelLayout = 0x4;
    auto eBufferAttr = make_unique<EffectBufferAttr>(bufIn, bufOut, numChans, frameLen, outChannels, outChannelLayout);
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;

    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(
        sceneType, DEVICE_TYPE_SPEAKER, 100, eBufferAttr, outChannels, outChannelLayout);
    EXPECT_EQ(EFFECT_NEED_RECONFIG, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ApplyAudioEffectChain API with channelLayout-only change
* @tc.number : ApplyAudioEffectChain_NeedReconfig__LayoutOnly_001
* @tc.desc   : Test ApplyAudioEffectChain returns EFFECT_NEED_RECONFIG when only channelLayout differs.
*/
HWTEST(AudioEffectChainManagerUnitTest, ApplyAudioEffectChain_NeedReconfig__LayoutOnly_001, TestSize.Level1)
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
    uint64_t outChannelLayout = 0x1;
    auto eBufferAttr = make_unique<EffectBufferAttr>(bufIn, bufOut, numChans, frameLen, outChannels, outChannelLayout);
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;

    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(sceneType, eBufferAttr,
        outChannels, outChannelLayout);
    EXPECT_EQ(EFFECT_NEED_RECONFIG, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ApplyAudioEffectChain with renderId and channelLayout-only change
* @tc.number : ApplyAudioEffectChain_NeedReconfig_LayoutOnly_RenderId_001
* @tc.desc   : Test ApplyAudioEffectChain with renderId returns EFFECT_NEED_RECONFIG when only channelLayout differs.
*/
HWTEST(AudioEffectChainManagerUnitTest, ApplyAudioEffectChain_NeedReconfig_LayoutOnly_RenderId_001, TestSize.Level1)
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
    uint64_t outChannelLayout = 0x1;
    auto eBufferAttr = make_unique<EffectBufferAttr>(bufIn, bufOut, numChans, frameLen, outChannels, outChannelLayout);
    string sceneType = "SCENE_MOVIE";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;

    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(
        sceneType, DEVICE_TYPE_SPEAKER, 100, eBufferAttr, outChannels, outChannelLayout);
    EXPECT_EQ(EFFECT_NEED_RECONFIG, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetAudioEffectChainDynamic with renderId - chain not found
* @tc.number : SetAudioEffectChainDynamic_RenderId_001
* @tc.desc   : Test SetAudioEffectChainDynamic with renderId returns ERROR when chain not found.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetAudioEffectChainDynamic_RenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    string effectMode = "EFFECT_DEFAULT";
    int32_t result = AudioEffectChainManager::GetInstance()->SetAudioEffectChainDynamic(
        sceneType, effectMode, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(ERROR, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetAudioEffectChainDynamic with renderId - success
* @tc.number : SetAudioEffectChainDynamic_RenderId_002
* @tc.desc   : Test SetAudioEffectChainDynamic with renderId succeeds after chain creation.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetAudioEffectChainDynamic_RenderId_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    string effectMode = "EFFECT_DEFAULT";
    int32_t result = AudioEffectChainManager::GetInstance()->SetAudioEffectChainDynamic(
        sceneType, effectMode, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test NotifyAndCreateAudioEffectChain with renderId - new chain
* @tc.number : NotifyAndCreateAudioEffectChain_RenderId_001
* @tc.desc   : Test NotifyAndCreateAudioEffectChain with renderId creates new chain.
*/
HWTEST(AudioEffectChainManagerUnitTest, NotifyAndCreateAudioEffectChain_RenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    int32_t result = AudioEffectChainManager::GetInstance()->NotifyAndCreateAudioEffectChain(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);

    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.count(key) > 0, true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[key], 1);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test NotifyAndCreateAudioEffectChain with renderId - reuse existing
* @tc.number : NotifyAndCreateAudioEffectChain_RenderId_002
* @tc.desc   : Test NotifyAndCreateAudioEffectChain with renderId reuses existing chain with count 0.
*/
HWTEST(AudioEffectChainManagerUnitTest, NotifyAndCreateAudioEffectChain_RenderId_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    // First create
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    // Manually set count to 0 to simulate a released but not erased chain
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[key] = 0;

    // Should reuse existing chain (count==0 branch)
    int32_t result = AudioEffectChainManager::GetInstance()->NotifyAndCreateAudioEffectChain(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[key], 1);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CreateAudioEffectChainDynamicInner with renderId - not initialized
* @tc.number : CreateAudioEffectChainDynamicInner_RenderId_001
* @tc.desc   : Test CreateAudioEffectChainDynamicInner with renderId returns ERROR when not initialized.
*/
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChainDynamicInner_RenderId_001, TestSize.Level1)
{
    // Do not call InitAudioEffectChainManager
    string sceneType = "SCENE_MOVIE";
    int32_t result = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamicInner(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CreateAudioEffectChainDynamicInner with renderId - null chain cleanup
* @tc.number : CreateAudioEffectChainDynamicInner_RenderId_002
* @tc.desc   : Test CreateAudioEffectChainDynamicInner cleans up null chain entry.
*/
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChainDynamicInner_RenderId_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    // Insert a null chain with positive count to trigger null cleanup branch
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key] = nullptr;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[key] = 1;

    // Should clean up null chain and proceed to create a new one
    int32_t result = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamicInner(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReleaseAudioEffectChainDynamicInner with renderId - null chain
* @tc.number : ReleaseAudioEffectChainDynamicInner_RenderId_001
* @tc.desc   : Test ReleaseAudioEffectChainDynamicInner returns SUCCESS when chain is null.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamicInner_RenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    // Insert null chain
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key] = nullptr;

    int32_t result = AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamicInner(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReleaseAudioEffectChainDynamicInner with renderId - not initialized
* @tc.number : ReleaseAudioEffectChainDynamicInner_RenderId_002
* @tc.desc   : Test ReleaseAudioEffectChainDynamicInner returns ERROR when not initialized.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamicInner_RenderId_002, TestSize.Level1)
{
    string sceneType = "SCENE_MOVIE";
    int32_t result = AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamicInner(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReleaseAudioEffectChainDynamicInner with renderId - shared default decrement
* @tc.number : ReleaseAudioEffectChainDynamicInner_RenderId_003
* @tc.desc   : Test ReleaseAudioEffectChainDynamicInner decrements default chain count when shared.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamicInner_RenderId_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    string defaultKey = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        TEST_DEFAULT_SCENE_TYPE, DEVICE_TYPE_SPEAKER, 100);

    // Create a chain and set up shared default scenario
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    // Create 2 default references so count > 1 for default decrement path
    auto chain = AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key];
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultKey] = chain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultKey] = 3;
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;

    // Release one count - should go through > 1 branch and decrement default
    int32_t result = AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamicInner(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultKey], 2);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndReleaseCommonEffectChain with renderId - not default existed
* @tc.number : CheckAndReleaseCommonEffectChain_RenderId_001
* @tc.desc   : Test CheckAndReleaseCommonEffectChain returns ERROR when isDefaultEffectChainExisted_ is false.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndReleaseCommonEffectChain_RenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = false;
    int32_t result = AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(ERROR, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndReleaseCommonEffectChain with renderId - shared default release
* @tc.number : CheckAndReleaseCommonEffectChain_RenderId_002
* @tc.desc   : Test CheckAndReleaseCommonEffectChain releases default chain when count <= 1.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndReleaseCommonEffectChain_RenderId_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    string defaultKey = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        TEST_DEFAULT_SCENE_TYPE, DEVICE_TYPE_SPEAKER, 100);

    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    auto chain = AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key];
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultKey] = chain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultKey] = 1;
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;

    int32_t result = AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_, false);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckAndReleaseCommonEffectChain with renderId - shared default decrement
* @tc.number : CheckAndReleaseCommonEffectChain_RenderId_003
* @tc.desc   : Test CheckAndReleaseCommonEffectChain decrements default count when count > 1.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndReleaseCommonEffectChain_RenderId_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    string defaultKey = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        TEST_DEFAULT_SCENE_TYPE, DEVICE_TYPE_SPEAKER, 100);

    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    auto chain = AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key];
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultKey] = chain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultKey] = 3;
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;

    int32_t result = AudioEffectChainManager::GetInstance()->CheckAndReleaseCommonEffectChain(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    // count > 1, so just decrement
    EXPECT_EQ(ERROR, result);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultKey], 2);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test InitAudioEffectChainDynamicInner with renderId - chain not in map
* @tc.number : InitAudioEffectChainDynamicInner_RenderId_001
* @tc.desc   : Test InitAudioEffectChainDynamicInner returns SUCCESS when chain not in map.
*/
HWTEST(AudioEffectChainManagerUnitTest, InitAudioEffectChainDynamicInner_RenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    // No chain created for this key - returns SUCCESS (early return branch)
    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamicInner(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test InitAudioEffectChainDynamicInner with renderId - success with chain
* @tc.number : InitAudioEffectChainDynamicInner_RenderId_002
* @tc.desc   : Test InitAudioEffectChainDynamicInner initializes existing chain.
*/
HWTEST(AudioEffectChainManagerUnitTest, InitAudioEffectChainDynamicInner_RenderId_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    int32_t result = AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamicInner(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateMultichannelConfigInner with renderId - null chain
* @tc.number : UpdateMultichannelConfigInner_RenderId_001
* @tc.desc   : Test UpdateMultichannelConfigInner returns ERROR when chain is null.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateMultichannelConfigInner_RenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    // Insert null chain to trigger null check
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key] = nullptr;

    int32_t result = AudioEffectChainManager::GetInstance()->UpdateMultichannelConfigInner(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(ERROR, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateMultichannelConfigInner with renderId - success
* @tc.number : UpdateMultichannelConfigInner_RenderId_002
* @tc.desc   : Test UpdateMultichannelConfigInner succeeds with valid chain.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateMultichannelConfigInner_RenderId_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    int32_t result = AudioEffectChainManager::GetInstance()->UpdateMultichannelConfigInner(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReturnEffectChannelInfoInner with renderId - chain not found
* @tc.number : ReturnEffectChannelInfoInner_RenderId_001
* @tc.desc   : Test ReturnEffectChannelInfoInner returns ERROR when chain not found.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnEffectChannelInfoInner_RenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    int32_t result = AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfoInner(
        "SCENE_MOVIE", DEVICE_TYPE_SPEAKER, 999, channels, channelLayout);
    EXPECT_EQ(ERROR, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReturnEffectChannelInfoInner with renderId - success
* @tc.number : ReturnEffectChannelInfoInner_RenderId_002
* @tc.desc   : Test ReturnEffectChannelInfoInner succeeds with valid chain.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnEffectChannelInfoInner_RenderId_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    int32_t result = AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfoInner(
        sceneType, DEVICE_TYPE_SPEAKER, 100, channels, channelLayout);
    EXPECT_EQ(SUCCESS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReturnEffectChannelInfoInner different chain skipped
* @tc.number : ReturnEffectChannelInfoInner_002
* @tc.desc   : The for loop skips a session whose scene type maps to a different chain instance, so
*              FindMaxEffectChannels is not called for it and the output channel count stays zero.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnEffectChannelInfoInner_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::shared_ptr<AudioEffectChain> targetChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_MOVIE", true);
    std::shared_ptr<AudioEffectChain> otherChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_OTHER", true);
    ASSERT_NE(targetChain, nullptr);
    ASSERT_NE(otherChain, nullptr);
    ASSERT_NE(targetChain, otherChain);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_.clear();
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER"] =
        targetChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["SCENE_OTHER_&_DEVICE_TYPE_SPEAKER"] =
        otherChain;

    SessionEffectInfo info = DEFAULT_INFO;
    info.sceneType = "SCENE_OTHER";
    info.channels = DEFAULT_MCH_NUM_CHANNEL;
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->SessionInfoMapAdd("session_other", info));

    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    int32_t result = AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfoInner("SCENE_MOVIE",
        channels, channelLayout);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(0u, channels);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReturnEffectChannelInfoInner unmapped scene mode
* @tc.number : ReturnEffectChannelInfoInner_003
* @tc.desc   : When a session scene mode has no effect chain, FindMaxEffectChannels falls back to the
*              default channel count instead of the session channel count.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnEffectChannelInfoInner_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    ASSERT_NE(audioEffectChain, nullptr);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_.clear();
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;

    SessionEffectInfo info = DEFAULT_INFO;
    info.sceneMode = "EFFECT_NONE";
    info.channels = DEFAULT_MCH_NUM_CHANNEL;
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->SessionInfoMapAdd("session_default", info));

    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    int32_t result = AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfoInner(sceneType,
        channels, channelLayout);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(DEFAULT_NUM_CHANNEL, channels);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReturnEffectChannelInfoInner null pair chain skipped
* @tc.number : ReturnEffectChannelInfoInner_004
* @tc.desc   : The for loop skips a session whose scene type maps to a null chain instance,
*              exercising the non-null guard so the output channel count stays zero.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnEffectChannelInfoInner_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    ASSERT_NE(audioEffectChain, nullptr);

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_.clear();
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["SCENE_OTHER_&_DEVICE_TYPE_SPEAKER"] = nullptr;

    SessionEffectInfo info = DEFAULT_INFO;
    info.sceneType = "SCENE_OTHER";
    info.channels = DEFAULT_MCH_NUM_CHANNEL;
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->SessionInfoMapAdd("session_other", info));

    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    int32_t result = AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfoInner(sceneType,
        channels, channelLayout);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(0u, channels);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReturnEffectChannelInfoInner renderId shared chain
* @tc.number : ReturnEffectChannelInfoInner_RenderId_003
* @tc.desc   : The nested for loop finds a session sharing the renderId keyed chain and invokes
*              FindMaxEffectChannels to report its channel count.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnEffectChannelInfoInner_RenderId_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    uint32_t renderId = 100;
    AudioEffectChainManager::GetInstance()->deviceType_ = deviceType;
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, deviceType, renderId));

    std::string renderIdKey = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, deviceType, renderId);
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[renderIdKey];
    ASSERT_NE(audioEffectChain, nullptr);
    std::string legacyKey = sceneType + "_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[legacyKey] = audioEffectChain;

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_.clear();
    SessionEffectInfo info = DEFAULT_INFO;
    info.channels = DEFAULT_MCH_NUM_CHANNEL;
    info.channelLayout = DEFAULT_MCH_NUM_CHANNELLAYOUT;
    info.deviceType = deviceType;
    info.renderId = renderId;
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->SessionInfoMapAdd("session_render", info));

    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    int32_t result = AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfoInner(
        sceneType, deviceType, renderId, channels, channelLayout);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(DEFAULT_NUM_CHANNEL, channels);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReturnEffectChannelInfoInner renderId dangling session
* @tc.number : ReturnEffectChannelInfoInner_RenderId_004
* @tc.desc   : A session id present in the scene map but absent from the effect info map is skipped by
*              the continue branch, while the valid session is still processed.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReturnEffectChannelInfoInner_RenderId_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MOVIE";
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    uint32_t renderId = 100;
    AudioEffectChainManager::GetInstance()->deviceType_ = deviceType;
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, deviceType, renderId));

    std::string renderIdKey = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, deviceType, renderId);
    std::shared_ptr<AudioEffectChain> audioEffectChain =
        AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[renderIdKey];
    ASSERT_NE(audioEffectChain, nullptr);
    std::string legacyKey = sceneType + "_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[legacyKey] = audioEffectChain;

    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_.clear();
    SessionEffectInfo info = DEFAULT_INFO;
    info.channels = DEFAULT_MCH_NUM_CHANNEL;
    info.channelLayout = DEFAULT_MCH_NUM_CHANNELLAYOUT;
    info.deviceType = deviceType;
    info.renderId = renderId;
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->SessionInfoMapAdd("session_real", info));
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_[sceneType].insert("session_dangling");

    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    int32_t result = AudioEffectChainManager::GetInstance()->ReturnEffectChannelInfoInner(
        sceneType, deviceType, renderId, channels, channelLayout);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(DEFAULT_NUM_CHANNEL, channels);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckProcessClusterInstances with renderId - null chain
* @tc.number : CheckProcessClusterInstances_RenderId_004
* @tc.desc   : Test CheckProcessClusterInstances with renderId when chain exists but is null.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckProcessClusterInstances_RenderId_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    // Insert null chain with positive count to trigger null warning branch
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key] = nullptr;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[key] = 1;

    // null chain falls through to the else branch at the bottom
    int32_t result = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    // Falls through since null chain does not match any existing case, goes to capacity check
    EXPECT_TRUE(result == CREATE_NEW_PROCESSCLUSTER || result == CREATE_DEFAULT_PROCESSCLUSTER ||
        result == USE_DEFAULT_PROCESSCLUSTER);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckProcessClusterInstances with renderId - USE_DEFAULT
* @tc.number : CheckProcessClusterInstances_RenderId_005
* @tc.desc   : Test CheckProcessClusterInstances returns USE_DEFAULT when chain shares default.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckProcessClusterInstances_RenderId_005, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    string defaultKey = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        TEST_DEFAULT_SCENE_TYPE, DEVICE_TYPE_SPEAKER, 100);

    // Create a chain for default scene
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        TEST_DEFAULT_SCENE_TYPE, DEVICE_TYPE_SPEAKER, 100);

    auto defaultChain = AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultKey];

    // Make the scene type point to the default chain
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key] = defaultChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[key] = 1;
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;

    int32_t result = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(result, USE_DEFAULT_PROCESSCLUSTER);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckProcessClusterInstances with renderId - CREATE_DEFAULT
* @tc.number : CheckProcessClusterInstances_RenderId_006
* @tc.desc   : Test CheckProcessClusterInstances returns CREATE_DEFAULT when capacity is limited.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckProcessClusterInstances_RenderId_006, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    // Set maxEffectChainCount_ to minimum and no special effects to force CREATE_DEFAULT path
    AudioEffectChainManager::GetInstance()->maxEffectChainCount_ = 1;
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = false;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;

    int32_t result = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(
        "SCENE_MUSIC", DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(result, CREATE_DEFAULT_PROCESSCLUSTER);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckProcessClusterInstances with renderId - USE_DEFAULT when at capacity
* @tc.number : CheckProcessClusterInstances_RenderId_007
* @tc.desc   : Test CheckProcessClusterInstances returns USE_DEFAULT when at capacity and default exists.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckProcessClusterInstances_RenderId_007, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    // Set maxEffectChainCount_ to minimum, default exists
    AudioEffectChainManager::GetInstance()->maxEffectChainCount_ = 1;
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = false;

    int32_t result = AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances(
        "SCENE_MUSIC", DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(result, USE_DEFAULT_PROCESSCLUSTER);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test QueryEffectChannelInfo with renderId - success
* @tc.number : QueryEffectChannelInfo_RenderId_002
* @tc.desc   : Test QueryEffectChannelInfo with renderId succeeds after chain creation.
*/
HWTEST(AudioEffectChainManagerUnitTest, QueryEffectChannelInfo_RenderId_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    uint32_t channels = 0;
    uint64_t channelLayout = 0;
    int32_t result = AudioEffectChainManager::GetInstance()->QueryEffectChannelInfo(
        sceneType, DEVICE_TYPE_SPEAKER, 100, channels, channelLayout);
    EXPECT_EQ(SUCCESS, result);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChainArm with renderId - invalid effect mode
* @tc.number : ExistAudioEffectChainArm_RenderId_003
* @tc.desc   : Test ExistAudioEffectChainArm with renderId returns false for invalid effect mode.
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChainArm_RenderId_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    // Pass an invalid effect mode value
    bool result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChainArm(
        "SCENE_MUSIC", static_cast<AudioEffectMode>(9999), DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(result, false);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChain with renderId - EFFECT_NONE mode
* @tc.number : ExistAudioEffectChain_RenderId_003
* @tc.desc   : Test ExistAudioEffectChain with renderId returns false for EFFECT_NONE mode.
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChain_RenderId_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    bool result = AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(
        "SCENE_MUSIC", "EFFECT_NONE", DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(result, false);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CreateAudioEffectChainDynamicInner with renderId - shared default increment
* @tc.number : CreateAudioEffectChainDynamicInner_RenderId_003
* @tc.desc   : Test CreateAudioEffectChainDynamicInner increments default count for shared chain.
*/
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChainDynamicInner_RenderId_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    string key = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    string defaultKey = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        TEST_DEFAULT_SCENE_TYPE, DEVICE_TYPE_SPEAKER, 100);

    // Create chain
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    // Set up shared default chain scenario
    auto chain = AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key];
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultKey] = chain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultKey] = 1;
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;

    // Create again - should increment default count
    int32_t result = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamicInner(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultKey], 2);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetBypassSpatializationForStereo API
* @tc.number : SetBypassSpatializationForStereo_001
* @tc.desc   : Test SetBypassSpatializationForStereo with same value (no update needed).
*/
HWTEST(AudioEffectChainManagerUnitTest, SetBypassSpatializationForStereo_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = false;

    // bypass == bypassSpatializationForStereo_(false), should early return without update
    AudioEffectChainManager::GetInstance()->SetBypassSpatializationForStereo(false);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_, false);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetBypassSpatializationForStereo API
* @tc.number : SetBypassSpatializationForStereo_002
* @tc.desc   : Test SetBypassSpatializationForStereo changes value, device is SPEAKER
*              (not BT/NearLink), spatializationEnabled_ is false.
*              Should update bypassSpatializationForStereo_ but skip UpdateSpatializationEnabled.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetBypassSpatializationForStereo_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = false;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = false;

    AudioEffectChainManager::GetInstance()->SetBypassSpatializationForStereo(true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_, true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetBypassSpatializationForStereo API
* @tc.number : SetBypassSpatializationForStereo_003
* @tc.desc   : Test SetBypassSpatializationForStereo with BLUETOOTH_A2DP device,
*              spatializationEnabled_ is false. Should update value but skip UpdateSpatializationEnabled.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetBypassSpatializationForStereo_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = false;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = false;

    AudioEffectChainManager::GetInstance()->SetBypassSpatializationForStereo(true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_, true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetBypassSpatializationForStereo API
* @tc.number : SetBypassSpatializationForStereo_004
* @tc.desc   : Test SetBypassSpatializationForStereo with BLUETOOTH_A2DP device,
*              spatializationEnabled_ is true. Should update value and trigger UpdateSpatializationEnabled.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetBypassSpatializationForStereo_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = false;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;
    AudioEffectChainManager::GetInstance()->headTrackingEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadSupported_ = false;

    AudioEffectChainManager::GetInstance()->SetBypassSpatializationForStereo(true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_, true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetBypassSpatializationForStereo API
* @tc.number : SetBypassSpatializationForStereo_005
* @tc.desc   : Test SetBypassSpatializationForStereo with BLUETOOTH_SCO device,
*              spatializationEnabled_ is true. Should update value and trigger UpdateSpatializationEnabled.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetBypassSpatializationForStereo_005, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = false;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;
    AudioEffectChainManager::GetInstance()->headTrackingEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadSupported_ = false;

    AudioEffectChainManager::GetInstance()->SetBypassSpatializationForStereo(true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_, true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetBypassSpatializationForStereo API
* @tc.number : SetBypassSpatializationForStereo_006
* @tc.desc   : Test SetBypassSpatializationForStereo with NEARLINK device,
*              spatializationEnabled_ is true. Should update value and trigger UpdateSpatializationEnabled.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetBypassSpatializationForStereo_006, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = false;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_NEARLINK;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;
    AudioEffectChainManager::GetInstance()->headTrackingEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->btOffloadSupported_ = false;

    AudioEffectChainManager::GetInstance()->SetBypassSpatializationForStereo(true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_, true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetBypassSpatializationForStereo API
* @tc.number : SetBypassSpatializationForStereo_007
* @tc.desc   : Test SetBypassSpatializationForStereo with headTrackingEnabled_ true,
*              btOffloadEnabled_ changes from true to false, should trigger UpdateSensorState.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetBypassSpatializationForStereo_007, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = false;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;
    AudioEffectChainManager::GetInstance()->headTrackingEnabled_ = true;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = true;
    AudioEffectChainManager::GetInstance()->btOffloadSupported_ = false;

    // bypass=true, headTracking=true, previousOffloadState=true
    // UpdateSpatializationEnabled may set btOffloadEnabled_=false (ARM processing path)
    // bypassSpatializationForStereo_ is now true, so !bypass == false
    // If btOffloadEnabled_ changed and headTracking enabled, UpdateSensorState will be called
    AudioEffectChainManager::GetInstance()->SetBypassSpatializationForStereo(true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_, true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetBypassSpatializationForStereo API
* @tc.number : SetBypassSpatializationForStereo_008
* @tc.desc   : Test SetBypassSpatializationForStereo toggling: false→true→false.
*              Verify value updates correctly on each call.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetBypassSpatializationForStereo_008, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = false;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = false;

    AudioEffectChainManager::GetInstance()->SetBypassSpatializationForStereo(true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_, true);

    AudioEffectChainManager::GetInstance()->SetBypassSpatializationForStereo(false);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_, false);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetBypassSpatializationForStereo API
* @tc.number : SetBypassSpatializationForStereo_009
* @tc.desc   : Test SetBypassSpatializationForStereo with WIRED_HEADPHONES (non-BT/NearLink device),
*              spatializationEnabled_ is true. Should update value but skip UpdateSpatializationEnabled
*              due to device type check.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetBypassSpatializationForStereo_009, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = false;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_WIRED_HEADPHONES;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;

    AudioEffectChainManager::GetInstance()->SetBypassSpatializationForStereo(true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_, true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetBypassSpatializationForStereo API
* @tc.number : SetBypassSpatializationForStereo_010
* @tc.desc   : Test SetBypassSpatializationForStereo with EARPIECE (non-BT/NearLink device),
*              spatializationEnabled_ is true. Should update value but skip UpdateSpatializationEnabled.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetBypassSpatializationForStereo_010, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_ = false;
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_EARPIECE;
    AudioEffectChainManager::GetInstance()->spatializationEnabled_ = true;

    AudioEffectChainManager::GetInstance()->SetBypassSpatializationForStereo(true);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->bypassSpatializationForStereo_, true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdatePersonalizedSpatializationStateInner API
* @tc.number : UpdatePersonalizedSpatializationStateInner_001
* @tc.desc   : Test UpdatePersonalizedSpatializationStateInner with btOffloadSupported_=false
*              (chains path) and personalizedSpatializationEnabled=true. With the HRTF file absent,
*              !personalizedSpatializationEnabled_ is false, so the file-existence guard in
*              UpdatePersonalizedSpatializationEnabledToChains returns ERROR; true + fail triggers
*              rollback to false.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdatePersonalizedSpatializationStateInner_001, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->btOffloadSupported_ = false;
    mgr->btOffloadEnabled_ = false;
    mgr->personalizedSpatializationEnabled_ = false;

    int32_t ret = mgr->UpdatePersonalizedSpatializationStateInner(true);
    // btOffloadSupported_=false → chains path; enabled=true + no HRTF file → ERROR; rollback to false
    EXPECT_NE(ret, SUCCESS);
    EXPECT_EQ(mgr->personalizedSpatializationEnabled_, false);

    mgr->ResetInfo();
}

/**
* @tc.name   : Test UpdatePersonalizedSpatializationStateInner API
* @tc.number : UpdatePersonalizedSpatializationStateInner_002
* @tc.desc   : Test UpdatePersonalizedSpatializationStateInner with btOffloadSupported_=true,
*              btOffloadEnabled_=false (chains path) and personalizedSpatializationEnabled=false.
*              !personalizedSpatializationEnabled_ short-circuits the file-existence guard so chains
*              returns SUCCESS (the empty map makes the loop a no-op, so SUCCESS is deterministic);
*              passing false means no rollback.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdatePersonalizedSpatializationStateInner_002, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->btOffloadSupported_ = true;
    mgr->btOffloadEnabled_ = false;
    mgr->personalizedSpatializationEnabled_ = true;

    int32_t ret = mgr->UpdatePersonalizedSpatializationStateInner(false);
    // btOffloadSupported_=true && btOffloadEnabled_=false → chains path; enabled=false → SUCCESS
    EXPECT_EQ(ret, SUCCESS);
    EXPECT_EQ(mgr->personalizedSpatializationEnabled_, false);

    mgr->ResetInfo();
}

/**
* @tc.name   : Test UpdatePersonalizedSpatializationStateInner API
* @tc.number : UpdatePersonalizedSpatializationStateInner_003
* @tc.desc   : Test UpdatePersonalizedSpatializationStateInner with btOffloadSupported_=true,
*              btOffloadEnabled_=true. Should take UpdatePersonalizedSpatializationEnabledToHdi path.
*              With audioEffectHdiParam_=nullptr, returns ERROR; personalizedSpatializationEnabled=true
*              triggers rollback to false.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdatePersonalizedSpatializationStateInner_003, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->btOffloadSupported_ = true;
    mgr->btOffloadEnabled_ = true;
    mgr->audioEffectHdiParam_ = nullptr;
    mgr->personalizedSpatializationEnabled_ = false;

    int32_t ret = mgr->UpdatePersonalizedSpatializationStateInner(true);
    // btOffloadSupported_=true && btOffloadEnabled_=true → HDI path
    // audioEffectHdiParam_=nullptr → ERROR; true + fail → rollback
    EXPECT_NE(ret, SUCCESS);
    EXPECT_EQ(mgr->personalizedSpatializationEnabled_, false);

    mgr->ResetInfo();
}

/**
* @tc.name   : Test UpdatePersonalizedSpatializationStateInner API
* @tc.number : UpdatePersonalizedSpatializationStateInner_004
* @tc.desc   : Test UpdatePersonalizedSpatializationStateInner rollback behavior.
*              Pass true with HDI failure; personalizedSpatializationEnabled_ should be rolled back to false.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdatePersonalizedSpatializationStateInner_004, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->btOffloadSupported_ = true;
    mgr->btOffloadEnabled_ = true;
    mgr->audioEffectHdiParam_ = nullptr;
    mgr->personalizedSpatializationEnabled_ = false;

    int32_t ret = mgr->UpdatePersonalizedSpatializationStateInner(true);
    EXPECT_NE(ret, SUCCESS);
    // Key assertion: true + fail → rollback to false
    EXPECT_EQ(mgr->personalizedSpatializationEnabled_, false);

    mgr->ResetInfo();
}

/**
* @tc.name   : Test UpdatePersonalizedSpatializationStateInner API
* @tc.number : UpdatePersonalizedSpatializationStateInner_005
* @tc.desc   : Test UpdatePersonalizedSpatializationStateInner no-rollback behavior.
*              Pass false with HDI failure; personalizedSpatializationEnabled_ should stay false
*              without rollback logic triggered.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdatePersonalizedSpatializationStateInner_005, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->btOffloadSupported_ = true;
    mgr->btOffloadEnabled_ = true;
    mgr->audioEffectHdiParam_ = nullptr;

    int32_t ret = mgr->UpdatePersonalizedSpatializationStateInner(false);
    EXPECT_NE(ret, SUCCESS);
    // false + fail → no rollback, stays false
    EXPECT_EQ(mgr->personalizedSpatializationEnabled_, false);

    mgr->ResetInfo();
}

/**
* @tc.name   : Test UpdatePersonalizedSpatializationStateInner API
* @tc.number : UpdatePersonalizedSpatializationStateInner_006
* @tc.desc   : Test UpdatePersonalizedSpatializationStateInner success path.
*              With initialized chain and btOffloadSupported_=false, pass false.
*              Should succeed or fail gracefully without rollback (false passed).
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdatePersonalizedSpatializationStateInner_006, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    mgr->btOffloadSupported_ = false;
    mgr->personalizedSpatializationEnabled_ = true;

    mgr->UpdatePersonalizedSpatializationStateInner(false);
    // Pass false, no rollback even on failure
    EXPECT_EQ(mgr->personalizedSpatializationEnabled_, false);

    mgr->ResetInfo();
}

/**
* @tc.name   : Test UpdateEffectBtOffloadSupported callback invocation
* @tc.number : UpdateEffectBtOffloadSupported_Callback_001
* @tc.desc   : Test UpdateEffectBtOffloadSupported with spatializationEnabled_=true,
*              personalizedSpatializationEnabled_=true, and a non-null callback set.
*              When UpdatePersonalizedSpatializationStateInner fails (no chains/HDI),
*              the callback should be invoked with the error code.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateEffectBtOffloadSupported_Callback_001, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    bool callbackCalled = false;
    int32_t callbackRet = SUCCESS;
    mgr->SetPersonalizedSpatializationStateResultCallback([&](int32_t ret) {
        callbackCalled = true;
        callbackRet = ret;
    });
    mgr->btOffloadSupported_ = false;
    mgr->spatializationEnabled_ = true;
    mgr->personalizedSpatializationEnabled_ = true;

    mgr->UpdateEffectBtOffloadSupported(true);
    // If UpdatePersonalizedSpatializationStateInner failed, callback should fire
    if (callbackCalled) {
        EXPECT_NE(callbackRet, SUCCESS);
    }

    mgr->personalizedSpatializationStateResultCallback_ = nullptr;
    mgr->ResetInfo();
}

/**
* @tc.name   : Test UpdateEffectBtOffloadSupported with null callback
* @tc.number : UpdateEffectBtOffloadSupported_Callback_002
* @tc.desc   : Test UpdateEffectBtOffloadSupported with spatializationEnabled_=true,
*              personalizedSpatializationEnabled_=true, but callback is nullptr.
*              When UpdatePersonalizedSpatializationStateInner fails,
*              should only log warning without crash.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateEffectBtOffloadSupported_Callback_002, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    mgr->personalizedSpatializationStateResultCallback_ = nullptr;
    mgr->btOffloadSupported_ = false;
    mgr->spatializationEnabled_ = true;
    mgr->personalizedSpatializationEnabled_ = true;

    // Should not crash when callback is null and inner fails
    mgr->UpdateEffectBtOffloadSupported(true);

    mgr->ResetInfo();
}

/**
* @tc.name   : Test SetChannelOverride / ClearChannelOverride keyed by chain scene
* @tc.number : ChannelOverride_SetClearPlain
* @tc.desc   : SetChannelOverride resolves routeKey -> chain -> chain.sceneType_ as the override key,
*              so the chain must exist first (a prior chain stores sceneType_ == sceneType). Clear erases
*              by the same scene key.
*/
HWTEST(AudioEffectChainManagerUnitTest, ChannelOverride_SetClearPlain, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->channelOverrideMap_.clear();

    std::string sceneType = "SCENE_MOVIE";
    std::string routeKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    auto chain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    ASSERT_NE(chain, nullptr);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[routeKey] = chain;
    std::string sceneKey = sceneType; // prior chain: GetSceneType() == sceneType
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.count(sceneKey), 0u);

    AudioEffectChainManager::GetInstance()->SetChannelOverride(sceneType, STEREO, CH_LAYOUT_STEREO);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.count(sceneKey), 1u);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_[sceneKey].inputChannels,
        static_cast<uint32_t>(STEREO));
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_[sceneKey].inputChannelLayout,
        static_cast<uint64_t>(CH_LAYOUT_STEREO));

    AudioEffectChainManager::GetInstance()->ClearChannelOverride(sceneType);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.count(sceneKey), 0u);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateMultichannelConfig consult override (scenes 1, 2)
* @tc.number : ChannelOverride_ConsultForces2ch
* @tc.desc   : Scene1: set override(2ch) -> consult hits -> ioBufferConfig_.inputCfg.channels==2.
*              Scene2: clear override -> recompute via FindMaxEffectChannels (DEFAULT_INFO).
*/
HWTEST(AudioEffectChainManagerUnitTest, ChannelOverride_ConsultForces2ch, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->channelOverrideMap_.clear();
    std::string sceneType = "SCENE_MOVIE";
    std::string key = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    auto chain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    ASSERT_NE(chain, nullptr);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[key] = chain;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd("s8ch", DEFAULT_INFO); // DEFAULT_INFO.channels=2

    // 场景1: set override(2ch) -> consult 命中 -> ioBufferConfig_.inputCfg.channels==2
    AudioEffectChainManager::GetInstance()->SetChannelOverride(sceneType, STEREO, CH_LAYOUT_STEREO);
    AudioEffectChainManager::GetInstance()->UpdateMultichannelConfig(sceneType);
    uint32_t inCh = 0;
    uint64_t inLayout = 0;
    chain->GetInputChannelInfo(inCh, inLayout);
    EXPECT_EQ(inCh, static_cast<uint32_t>(STEREO));
    EXPECT_EQ(inLayout, static_cast<uint64_t>(CH_LAYOUT_STEREO));

    // 场景2: clear override -> 回到 FindMaxEffectChannels(DEFAULT_INFO)
    AudioEffectChainManager::GetInstance()->ClearChannelOverride(sceneType);
    AudioEffectChainManager::GetInstance()->UpdateMultichannelConfig(sceneType);
    chain->GetInputChannelInfo(inCh, inLayout);
    EXPECT_EQ(inCh, DEFAULT_INFO.channels);
    EXPECT_EQ(inLayout, DEFAULT_INFO.channelLayout);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test per-scene override isolation (scene 5)
* @tc.number : ChannelOverride_IsolatedPerScene
* @tc.desc   : Override set on sceneA must not affect sceneB's recompute (FindMaxEffectChannels path).
*/
HWTEST(AudioEffectChainManagerUnitTest, ChannelOverride_IsolatedPerScene, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->channelOverrideMap_.clear();
    std::string sceneA = "SCENE_MOVIE";
    std::string sceneB = "SCENE_MUSIC";
    auto chainA = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneA, true);
    auto chainB = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneB, true);
    ASSERT_NE(chainA, nullptr);
    ASSERT_NE(chainB, nullptr);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneA + "_&_DEVICE_TYPE_SPEAKER"] = chainA;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneB + "_&_DEVICE_TYPE_SPEAKER"] = chainB;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd("sA", DEFAULT_INFO);
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd("sB", DEFAULT_INFO);

    // 只对 sceneA 设 override，sceneB 不应被影响
    AudioEffectChainManager::GetInstance()->SetChannelOverride(sceneA, STEREO, CH_LAYOUT_STEREO);
    AudioEffectChainManager::GetInstance()->UpdateMultichannelConfig(sceneB);
    uint32_t inCh = 0;
    uint64_t inLayout = 0;
    chainB->GetInputChannelInfo(inCh, inLayout);
    EXPECT_EQ(inCh, DEFAULT_INFO.channels); // 未 override，走 FindMaxEffectChannels

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ResetInfo clears channelOverrideMap_ (scene 9)
* @tc.number : ChannelOverride_ResetInfoClearsMap
* @tc.desc   : ResetInfo must clear all entries in channelOverrideMap_ (held under dynamicMutex_).
*              SetChannelOverride needs the chain to exist first to store by chain scene.
*/
HWTEST(AudioEffectChainManagerUnitTest, ChannelOverride_ResetInfoClearsMap, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->channelOverrideMap_.clear();
    std::string sceneType = "SCENE_MOVIE";
    std::string routeKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    auto chain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    ASSERT_NE(chain, nullptr);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[routeKey] = chain;
    AudioEffectChainManager::GetInstance()->SetChannelOverride(sceneType, STEREO, CH_LAYOUT_STEREO);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.size(), 1u);
    AudioEffectChainManager::GetInstance()->ResetInfo();
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->channelOverrideMap_.size(), 0u);
}
/**
* @tc.name   : Test enhanced UpdateMultichannelConfig consult override (line 2451 if/else)
* @tc.number : ChannelOverride_EnhancedConsultForces2ch
* @tc.desc   : Enhanced route (sceneType+deviceType+renderId): set override(2ch) -> consult hits
*              (if-branch) -> chain.inputCfg.channels==2; clear override -> recompute via
*              FindMaxEffectChannels (else-branch).
*/
HWTEST(AudioEffectChainManagerUnitTest, ChannelOverride_EnhancedConsultForces2ch, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DeviceType::DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->channelOverrideMap_.clear();
    string sceneType = "SCENE_MOVIE";
    DeviceType deviceType = DeviceType::DEVICE_TYPE_SPEAKER;
    uint32_t renderId = 100;
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType, deviceType, renderId);
    string routeKey = AudioEffectChainManager::GetInstance()->GenerateEffectChainKey(
        sceneType, deviceType, renderId);
    auto chain = AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[routeKey];
    ASSERT_NE(chain, nullptr);
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_.clear();
    AudioEffectChainManager::GetInstance()->SessionInfoMapAdd("sEnh", DEFAULT_INFO);

    // if-branch: enhanced override(2ch) set -> consult hits -> chain forced to STEREO.
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->SetChannelOverride(
        sceneType, deviceType, renderId, STEREO, CH_LAYOUT_STEREO), SUCCESS);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->UpdateMultichannelConfig(
        sceneType, deviceType, renderId), SUCCESS);
    uint32_t inCh = 0;
    uint64_t inLayout = 0;
    chain->GetInputChannelInfo(inCh, inLayout);
    EXPECT_EQ(inCh, static_cast<uint32_t>(STEREO));

    // else-branch: clear override -> recompute via FindMaxEffectChannels(DEFAULT_INFO).
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->ClearChannelOverride(
        sceneType, deviceType, renderId), SUCCESS);
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->UpdateMultichannelConfig(
        sceneType, deviceType, renderId), SUCCESS);
    chain->GetInputChannelInfo(inCh, inLayout);
    EXPECT_EQ(inCh, DEFAULT_INFO.channels);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}

namespace {
bool CheckEffectLibStub(const AudioEffectDescriptor descriptor)
{
    return true;
}

int32_t EffectProcessStub(AudioEffectHandle self, AudioBuffer *inBuffer, AudioBuffer *outBuffer)
{
    return SUCCESS;
}

int32_t EffectCommandStub(AudioEffectHandle self, uint32_t cmdCode, AudioEffectTransInfo *cmdInfo,
    AudioEffectTransInfo *replyInfo)
{
    return SUCCESS;
}

int32_t CreateEffectLibSuccStub(const AudioEffectDescriptor descriptor, AudioEffectHandle *handle)
{
    static AudioEffectInterface effectInterface = {EffectProcessStub, EffectCommandStub};
    static AudioEffectInterface *effectInterfacePtr = &effectInterface;
    if (handle == nullptr) {
        return ERROR;
    }
    *handle = &effectInterfacePtr;
    return SUCCESS;
}

int32_t ReleaseEffectLibStub(AudioEffectHandle handle)
{
    return SUCCESS;
}

AudioEffectLibrary g_effectLibSucc = {0, "lib_test_succ", "impl", CheckEffectLibStub,
    CreateEffectLibSuccStub, ReleaseEffectLibStub, nullptr};

std::shared_ptr<AudioEffectLibEntry> BuildLibEntry(AudioEffectLibrary *libHandle, const std::string &libName,
    const std::vector<std::string> &effectNames)
{
    auto libEntry = std::make_shared<AudioEffectLibEntry>();
    libEntry->audioEffectLibHandle = libHandle;
    libEntry->libraryName = libName;
    libEntry->effectName = effectNames;
    return libEntry;
}
} // namespace

/**
* @tc.name   : Test FindEffectLib and CheckValidEffectLibEntry
* @tc.number : FindEffectLib_001
* @tc.desc   : Init with valid effect library list, effect maps are constructed.
*/
HWTEST(AudioEffectChainManagerUnitTest, FindEffectLib_001, TestSize.Level1)
{
    vector<shared_ptr<AudioEffectLibEntry>> effectLibraryList = {
        BuildLibEntry(&g_effectLibSucc, "lib_test_succ", {"apply1", "apply2", "apply3"})
    };
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, effectLibraryList);
    EXPECT_EQ(static_cast<size_t>(3), AudioEffectChainManager::GetInstance()->effectToLibraryEntryMap_.size());
    EXPECT_EQ("lib_test_succ", AudioEffectChainManager::GetInstance()->effectToLibraryNameMap_["apply1"]);
    EXPECT_EQ(static_cast<size_t>(3),
        AudioEffectChainManager::GetInstance()->effectChainToEffectsMap_["EFFECTCHAIN_SPK_MUSIC"].size());
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckValidEffectLibEntry with invalid entries
* @tc.number : FindEffectLib_002
* @tc.desc   : Invalid lib entries (null handle / null createEffect / null releaseEffect) are skipped.
*/
HWTEST(AudioEffectChainManagerUnitTest, FindEffectLib_002, TestSize.Level1)
{
    AudioEffectLibrary libWithNullCreate = {0, "lib_null_create", "impl", CheckEffectLibStub,
        nullptr, ReleaseEffectLibStub, nullptr};
    AudioEffectLibrary libWithNullRelease = {0, "lib_null_release", "impl", CheckEffectLibStub,
        CreateEffectLibSuccStub, nullptr, nullptr};
    vector<shared_ptr<AudioEffectLibEntry>> effectLibraryList = {
        BuildLibEntry(nullptr, "lib_null_handle", {"apply1"}),
        BuildLibEntry(&libWithNullCreate, "lib_null_create", {"apply2"}),
        BuildLibEntry(&libWithNullRelease, "lib_null_release", {"apply3"}),
    };
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, effectLibraryList);
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->effectToLibraryEntryMap_.empty());
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ConstructEffectChainMgrMaps default scene alias
* @tc.number : ConstructEffectChainMgrMaps_001
* @tc.desc   : Scene key starting with defaultSceneName generates SCENE_DEFAULT alias entry.
*/
HWTEST(AudioEffectChainManagerUnitTest, ConstructEffectChainMgrMaps_001, TestSize.Level1)
{
    EffectChainManagerParam param = DEFAULT_EFFECT_CHAIN_MANAGER_PARAM;
    param.sceneTypeToChainNameMap = {
        {"SCENE_DEFAULT_&_EFFECT_NONE_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_SPK_MUSIC"}};
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        param, DEFAULT_EFFECT_LIBRARY_LIST);
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->sceneTypeAndModeToEffectChainNameMap_.count(
        "SCENE_DEFAULT_&_EFFECT_NONE_&_DEVICE_TYPE_SPEAKER") > 0);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetEffectChainByMode fallback
* @tc.number : GetEffectChainByMode_001
* @tc.desc   : Chain name not in effectChainToEffectsMap_ falls back to EFFECT_NONE.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetEffectChainByMode_001, TestSize.Level1)
{
    EffectChainManagerParam param = DEFAULT_EFFECT_CHAIN_MANAGER_PARAM;
    param.sceneTypeToChainNameMap = {
        {"SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER", "EFFECTCHAIN_NOT_EXIST"}};
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        param, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string result = AudioEffectChainManager::GetInstance()->GetEffectChainByMode(
        "SCENE_MOVIE_&_EFFECT_DEFAULT_&_DEVICE_TYPE_SPEAKER");
    EXPECT_EQ("EFFECT_NONE", result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetAudioEffectChainDynamic without chain
* @tc.number : SetAudioEffectChainDynamic_001
* @tc.desc   : Set effect chain dynamic when chain does not exist, return ERROR.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetAudioEffectChainDynamic_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneType = "SCENE_MUSIC";
    std::string effectMode = "EFFECT_DEFAULT";
    int32_t result = AudioEffectChainManager::GetInstance()->SetAudioEffectChainDynamic(sceneType, effectMode);
    EXPECT_EQ(ERROR, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckProcessClusterInstances offload branch
* @tc.number : CheckProcessClusterInstances_006
* @tc.desc   : Offload enabled returns USE_NONE_PROCESSCLUSTER for both overloads.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckProcessClusterInstances_006, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->spkOffloadEnabled_ = true;
    EXPECT_EQ(USE_NONE_PROCESSCLUSTER,
        AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances("SCENE_MUSIC"));
    EXPECT_EQ(USE_NONE_PROCESSCLUSTER,
        AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances("SCENE_MUSIC",
        DEVICE_TYPE_SPEAKER, 1));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckProcessClusterInstances SCENE_EXTRA branch
* @tc.number : CheckProcessClusterInstances_007
* @tc.desc   : SCENE_EXTRA returns CREATE_EXTRA_PROCESSCLUSTER.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckProcessClusterInstances_007, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    EXPECT_EQ(CREATE_EXTRA_PROCESSCLUSTER,
        AudioEffectChainManager::GetInstance()->CheckProcessClusterInstances("SCENE_EXTRA"));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test NotifyAndCreateAudioEffectChain reuse with new default chain
* @tc.number : NotifyAndCreateAudioEffectChain_004
* @tc.desc   : Reuse path backfills default chain when defaultEffectChainCreated_ is true.
*/
HWTEST(AudioEffectChainManagerUnitTest, NotifyAndCreateAudioEffectChain_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->sceneTypeCountList_.clear();
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->maxEffectChainCount_ = 1;
    int32_t result = AudioEffectChainManager::GetInstance()->NotifyAndCreateAudioEffectChain(sceneType);
    EXPECT_EQ(SUCCESS, result);

    // make the second call reuse the scene key while a new default chain is created
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 0;
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = false;
    result = AudioEffectChainManager::GetInstance()->NotifyAndCreateAudioEffectChain(sceneType);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(1, AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey]);
    EXPECT_NE(nullptr, AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey]);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test NotifyAndCreateAudioEffectChain reuse with existing default chain
* @tc.number : NotifyAndCreateAudioEffectChain_005
* @tc.desc   : Reuse path rebinds new chain to scene key when default chain already exists.
*/
HWTEST(AudioEffectChainManagerUnitTest, NotifyAndCreateAudioEffectChain_005, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->sceneTypeCountList_.clear();
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->maxEffectChainCount_ = 1;
    int32_t result = AudioEffectChainManager::GetInstance()->NotifyAndCreateAudioEffectChain(sceneType);
    EXPECT_EQ(SUCCESS, result);
    auto defaultChain = AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey];

    // reuse path: default chain exists, new chain is bound to the scene key
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 0;
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;
    result = AudioEffectChainManager::GetInstance()->NotifyAndCreateAudioEffectChain(sceneType);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(1, AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey]);
    // scene key is rebound to the reused default chain
    EXPECT_EQ(defaultChain, AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey]);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test WaitAndReleaseEffectChain predicate satisfied
* @tc.number : WaitAndReleaseEffectChain_008
* @tc.desc   : Chain count greater than 0 cancels the release.
*/
HWTEST(AudioEffectChainManagerUnitTest, WaitAndReleaseEffectChain_008, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_.clear();
    auto audioEffectChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 1;

    // predicate is satisfied immediately, release is cancelled
    AudioEffectChainManager::GetInstance()->WaitAndReleaseEffectChain(sceneType, sceneTypeAndDeviceKey,
        defaultSceneTypeAndDeviceKey, 0);
    EXPECT_EQ(audioEffectChain,
        AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey]);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test WaitAndReleaseEffectChain clears channel override
* @tc.number : WaitAndReleaseEffectChain_009
* @tc.desc   : Releasing a chain distinct from default chain clears its channel override.
*/
HWTEST(AudioEffectChainManagerUnitTest, WaitAndReleaseEffectChain_009, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_.clear();
    auto sceneChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    auto defaultChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_DEFAULT", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = sceneChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey] = defaultChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 0;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultSceneTypeAndDeviceKey] = 1;
    AudioEffectChainManager::GetInstance()->channelOverrideMap_[sceneChain->GetSceneType()] = {2, 0x3};

    AudioEffectChainManager::GetInstance()->WaitAndReleaseEffectChain(sceneType, sceneTypeAndDeviceKey,
        defaultSceneTypeAndDeviceKey, 0);
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->channelOverrideMap_.empty());
    EXPECT_EQ(0, AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.count(sceneTypeAndDeviceKey));
    EXPECT_NE(nullptr, AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey]);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetChannelOverride without chain
* @tc.number : SetChannelOverrideNoChain_001
* @tc.desc   : Set/Clear channel override when chain does not exist, return ERROR.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetChannelOverrideNoChain_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->ResetInfo();
    EXPECT_EQ(ERROR, AudioEffectChainManager::GetInstance()->SetChannelOverride("SCENE_MUSIC", 2, 0x3));
    EXPECT_EQ(ERROR, AudioEffectChainManager::GetInstance()->SetChannelOverride("SCENE_MUSIC",
        DEVICE_TYPE_SPEAKER, 1, 2, 0x3));
    EXPECT_EQ(ERROR, AudioEffectChainManager::GetInstance()->ClearChannelOverride("SCENE_MUSIC"));
    EXPECT_EQ(ERROR, AudioEffectChainManager::GetInstance()->ClearChannelOverride("SCENE_MUSIC",
        DEVICE_TYPE_SPEAKER, 1));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CreateAudioEffectChainDynamicInner null chain cleanup
* @tc.number : CreateAudioEffectChainDynamicInner_002
* @tc.desc   : Null chain entry is erased and a new chain is created.
*/
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChainDynamicInner_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->sceneTypeCountList_.clear();
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = nullptr;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 1;
    int32_t result = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamicInner(sceneType);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_NE(nullptr, AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey]);
    EXPECT_EQ(1, AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey]);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CreateAudioEffectChainDynamicInner shared default chain count
* @tc.number : CreateAudioEffectChainDynamicInner_003
* @tc.desc   : Existing chain shared with default chain increases both counts.
*/
HWTEST(AudioEffectChainManagerUnitTest, CreateAudioEffectChainDynamicInner_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->sceneTypeCountList_.clear();
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    auto audioEffectChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain(sceneType, true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey] =
        audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 1;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultSceneTypeAndDeviceKey] = 1;
    AudioEffectChainManager::GetInstance()->isDefaultEffectChainExisted_ = true;

    int32_t result = AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamicInner(sceneType);
    EXPECT_EQ(SUCCESS, result);
    EXPECT_EQ(2, AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey]);
    EXPECT_EQ(2, AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultSceneTypeAndDeviceKey]);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ReleaseAudioEffectChainDynamicInner null chain
* @tc.number : ReleaseAudioEffectChainDynamicInner_005
* @tc.desc   : Null chain or zero count entry is erased and returns SUCCESS.
*/
HWTEST(AudioEffectChainManagerUnitTest, ReleaseAudioEffectChainDynamicInner_005, TestSize.Level1)
{
    std::string sceneType = "SCENE_MUSIC";
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = nullptr;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 0;
    EXPECT_EQ(SUCCESS,
        AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamicInner(sceneType));
    EXPECT_EQ(0, AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_.count(
        sceneTypeAndDeviceKey));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetLatency offload and chain path
* @tc.number : GetLatency_009
* @tc.desc   : Nearlink offload returns 0, normal path returns chain latency.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetLatency_009, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_NEARLINK;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = true;
    EXPECT_EQ(0, AudioEffectChainManager::GetInstance()->GetLatency("123456"));

    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_SPEAKER;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = false;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["123456"] = DEFAULT_INFO;
    auto audioEffectChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_MOVIE", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER"] =
        audioEffectChain;
    EXPECT_EQ(audioEffectChain->GetLatency(), AudioEffectChainManager::GetInstance()->GetLatency("123456"));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test EffectDspVolumeUpdate branches
* @tc.number : EffectDspVolumeUpdate_003
* @tc.desc   : Null volume returns ERROR, EFFECT_NONE session is skipped, null hdiParam returns ERROR.
*/
HWTEST(AudioEffectChainManagerUnitTest, EffectDspVolumeUpdate_003, TestSize.Level1)
{
    auto hdiParamBak = AudioEffectChainManager::GetInstance()->audioEffectHdiParam_;
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    EXPECT_EQ(ERROR, AudioEffectChainManager::GetInstance()->EffectDspVolumeUpdate(nullptr));

    // session with EFFECT_NONE mode is skipped
    SessionEffectInfo noneInfo = DEFAULT_INFO;
    noneInfo.sceneMode = "EFFECT_NONE";
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["111"] = noneInfo;
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_["SCENE_MOVIE"].insert("111");
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->EffectDspVolumeUpdate(audioEffectVolume));

    // valid session with volume diff but null hdi param
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["222"] = DEFAULT_INFO;
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_["SCENE_MOVIE"].insert("222");
    audioEffectVolume->SetStreamVolume("222", 1.0f);
    audioEffectVolume->SetSystemVolume(DEFAULT_STREAM_OR_VOLUME_TYPE, 1.0f);
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = nullptr;
    EXPECT_EQ(ERROR, AudioEffectChainManager::GetInstance()->EffectDspVolumeUpdate(audioEffectVolume));
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = hdiParamBak;
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test EffectApVolumeUpdate branches
* @tc.number : EffectApVolumeUpdate_004
* @tc.desc   : Null volume returns ERROR, EFFECT_NONE session is skipped, volume increases chain curr volume.
*/
HWTEST(AudioEffectChainManagerUnitTest, EffectApVolumeUpdate_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    EXPECT_EQ(ERROR, AudioEffectChainManager::GetInstance()->EffectApVolumeUpdate(nullptr));

    SessionEffectInfo noneInfo = DEFAULT_INFO;
    noneInfo.sceneMode = "EFFECT_NONE";
    AudioEffectChainManager::GetInstance()->sessionIDSet_.insert("111");
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["111"] = noneInfo;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->EffectApVolumeUpdate(audioEffectVolume));

    // volume greater than chain curr volume updates final volume
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    auto audioEffectChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_MOVIE", true);
    audioEffectChain->SetCurrVolume(0.3f);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sessionIDSet_.insert("222");
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["222"] = DEFAULT_INFO;
    audioEffectVolume->SetStreamVolume("222", 1.0f);
    audioEffectVolume->SetSystemVolume(DEFAULT_STREAM_OR_VOLUME_TYPE, 1.0f);
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->EffectApVolumeUpdate(audioEffectVolume));
    EXPECT_EQ(1.0f, audioEffectChain->GetFinalVolume());
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SendEffectApVolume final state
* @tc.number : SendEffectApVolume_004
* @tc.desc   : Final volume state false skips updating final volume.
*/
HWTEST(AudioEffectChainManagerUnitTest, SendEffectApVolume_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    std::string sceneTypeAndDeviceKey = "SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER";
    auto audioEffectChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_MUSIC", true);
    audioEffectChain->SetCurrVolume(0.3f);
    audioEffectChain->SetFinalVolume(0.5f);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->SendEffectApVolume(audioEffectVolume));
    EXPECT_EQ(0.5f, audioEffectChain->GetFinalVolume());
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test EffectVolumeUpdateInner dsp path
* @tc.number : EffectVolumeUpdateInner_002
* @tc.desc   : Nearlink with bt offload takes DSP volume update path.
*/
HWTEST(AudioEffectChainManagerUnitTest, EffectVolumeUpdateInner_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_NEARLINK;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = true;
    std::shared_ptr<AudioEffectVolume> audioEffectVolume = std::make_shared<AudioEffectVolume>();
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->EffectVolumeUpdateInner(audioEffectVolume));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetEffectSystemVolume API
* @tc.number : SetEffectSystemVolume_001
* @tc.desc   : Set system volume returns SUCCESS.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetEffectSystemVolume_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->SetEffectSystemVolume(
        DEFAULT_STREAM_OR_VOLUME_TYPE, 0.5f));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetAbsVolumeStateToEffect unchanged branch
* @tc.number : SetAbsVolumeStateToEffect_002
* @tc.desc   : Setting the same abs volume state twice skips updating.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetAbsVolumeStateToEffect_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->SetAbsVolumeStateToEffect(true));
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->SetAbsVolumeStateToEffect(true));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test EffectDspAbsVolumeStateUpdate null hdi param
* @tc.number : EffectDspAbsVolumeStateUpdate_001
* @tc.desc   : Null hdi param returns ERROR.
*/
HWTEST(AudioEffectChainManagerUnitTest, EffectDspAbsVolumeStateUpdate_001, TestSize.Level1)
{
    auto hdiParamBak = AudioEffectChainManager::GetInstance()->audioEffectHdiParam_;
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = nullptr;
    EXPECT_EQ(ERROR, AudioEffectChainManager::GetInstance()->EffectDspAbsVolumeStateUpdate(true));
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = hdiParamBak;
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateParamExtra outdoor mode
* @tc.number : UpdateParamExtra_007
* @tc.desc   : Update outdoor mode param.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateParamExtra_007, TestSize.Level1)
{
    std::string mainkey = "audio_effect";
    std::string subkey = "outdoor_mode";
    std::string value = "1";
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, value);
    EXPECT_EQ(value, AudioEffectChainManager::GetInstance()->outdoorMode_);
    EXPECT_EQ(HDI_OUTDOOR_MODE, AudioEffectChainManager::GetInstance()->effectHdiInput_[0]);
    EXPECT_EQ(1, AudioEffectChainManager::GetInstance()->effectHdiInput_[1]);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateParamExtra super loudness mode
* @tc.number : UpdateParamExtra_005
* @tc.desc   : Update super loudness mode param.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateParamExtra_005, TestSize.Level1)
{
    std::string mainkey = "LOUD_VOLUME_MODE";
    std::string subkey = "super_loudness_mode";
    std::string value = "1";
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, value);
    EXPECT_EQ(value, AudioEffectChainManager::GetInstance()->superLoudnessMode_);
    EXPECT_EQ(HDI_SUPER_LOUDNESS_MODE, AudioEffectChainManager::GetInstance()->effectHdiInput_[0]);
    EXPECT_EQ(1, AudioEffectChainManager::GetInstance()->effectHdiInput_[1]);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateParamExtra tablet premium
* @tc.number : UpdateParamExtra_006
* @tc.desc   : Update tablet premium param (stereo enhancement).
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateParamExtra_006, TestSize.Level1)
{
    std::string mainkey = "audio_effect";
    std::string subkey = "tablet_premium";
    std::string value = "1";
    AudioEffectChainManager::GetInstance()->UpdateParamExtra(mainkey, subkey, value);
    EXPECT_EQ(value, AudioEffectChainManager::GetInstance()->stereoEnhancementMode_);
    EXPECT_EQ(HDI_STEREO_ENHANCEMENT, AudioEffectChainManager::GetInstance()->effectHdiInput_[0]);
    EXPECT_EQ(1, AudioEffectChainManager::GetInstance()->effectHdiInput_[1]);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SendAudioParamToARM missing cases
* @tc.number : SendAudioParamToARM_007
* @tc.desc   : Outdoor mode / super loudness mode / stereo enhancement are sent to chains.
*/
HWTEST(AudioEffectChainManagerUnitTest, SendAudioParamToARM_007, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.clear();
    auto audioEffectChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_MUSIC", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER"] =
        audioEffectChain;
    AudioEffectChainManager::GetInstance()->SendAudioParamToARM(HDI_OUTDOOR_MODE, "1");
    AudioEffectChainManager::GetInstance()->SendAudioParamToARM(HDI_SUPER_LOUDNESS_MODE, "1");
    AudioEffectChainManager::GetInstance()->SendAudioParamToARM(HDI_STEREO_ENHANCEMENT, "1");
    AudioEffectChainManager::GetInstance()->SendAudioParamToARM(
        static_cast<HdiSetParamCommandCode>(9999), "1");
    // null chain entry is skipped
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["SCENE_NULL_&_DEVICE_TYPE_SPEAKER"] =
        nullptr;
    AudioEffectChainManager::GetInstance()->SendAudioParamToARM(HDI_OUTDOOR_MODE, "1");
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetHdiParam null hdi param
* @tc.number : SetHdiParam_004
* @tc.desc   : Null hdi param returns ERROR when scene type changes.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetHdiParam_004, TestSize.Level1)
{
    auto hdiParamBak = AudioEffectChainManager::GetInstance()->audioEffectHdiParam_;
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = nullptr;
    AudioEffectChainManager::GetInstance()->currDspSceneType_ = SCENE_INITIAL;
    EXPECT_EQ(ERROR, AudioEffectChainManager::GetInstance()->SetHdiParam(SCENE_MUSIC));
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = hdiParamBak;
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetHdiParam log flag branch
* @tc.number : SetHdiParam_005
* @tc.desc   : Second call before init skips the error log.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetHdiParam_005, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->isInitialized_ = false;
    AudioEffectChainManager::GetInstance()->initializedLogFlag_ = true;
    EXPECT_EQ(ERROR, AudioEffectChainManager::GetInstance()->SetHdiParam(SCENE_MUSIC));
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->initializedLogFlag_);
    EXPECT_EQ(ERROR, AudioEffectChainManager::GetInstance()->SetHdiParam(SCENE_MUSIC));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdateSpatialDeviceType branches
* @tc.number : UpdateSpatialDeviceType_002
* @tc.desc   : Null hdi param returns ERROR, null chain entry is skipped.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSpatialDeviceType_002, TestSize.Level1)
{
    auto hdiParamBak = AudioEffectChainManager::GetInstance()->audioEffectHdiParam_;
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = nullptr;
    EXPECT_EQ(ERROR, AudioEffectChainManager::GetInstance()->UpdateSpatialDeviceType(EARPHONE_TYPE_OTHERS));
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = hdiParamBak;

    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["SCENE_NULL_&_DEVICE_TYPE_SPEAKER"] = nullptr;
    auto audioEffectChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_MUSIC", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER"] =
        audioEffectChain;
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->UpdateSpatialDeviceType(EARPHONE_TYPE_OTHERS));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetSpatializationSceneType null hdi param
* @tc.number : SetSpatializationSceneType_003
* @tc.desc   : Null hdi param returns ERROR.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetSpatializationSceneType_003, TestSize.Level1)
{
    auto hdiParamBak = AudioEffectChainManager::GetInstance()->audioEffectHdiParam_;
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = nullptr;
    EXPECT_EQ(ERROR, AudioEffectChainManager::GetInstance()->SetSpatializationSceneType(
        SPATIALIZATION_SCENE_TYPE_MUSIC));
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = hdiParamBak;
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
 * @tc.name   : Test IsChannelLayoutSupportedForDspEffect null hdi param
 * @tc.number : IsChannelLayoutSupportedForDspEffect_002
 * @tc.desc   : Null hdi param returns false.
 */
HWTEST(AudioEffectChainManagerUnitTest, IsChannelLayoutSupportedForDspEffect_002, TestSize.Level1)
{
    auto hdiParamBak = AudioEffectChainManager::GetInstance()->audioEffectHdiParam_;
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = nullptr;
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->IsChannelLayoutSupportedForDspEffect(
        static_cast<AudioChannelLayout>(0x3)));
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = hdiParamBak;
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetInitEffectBufferFlag
* @tc.number : SetInitEffectBufferFlag_001
* @tc.desc   : When flag is false InitAudioEffectChainDynamic returns SUCCESS without locking.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetInitEffectBufferFlag_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->SetInitEffectBufferFlag(false);
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic("SCENE_MUSIC"));
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->InitAudioEffectChainDynamic("SCENE_MUSIC",
        DEVICE_TYPE_SPEAKER, 1));
    AudioEffectChainManager::GetInstance()->SetInitEffectBufferFlag(true);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdatePersonalizedHRTFBin invalid fd
* @tc.number : UpdatePersonalizedHRTFBin_001
* @tc.desc   : Invalid fd returns ERR_SAVE_HRTF_FAIL.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdatePersonalizedHRTFBin_001, TestSize.Level1)
{
    EXPECT_EQ(ERR_SAVE_HRTF_FAIL, AudioEffectChainManager::GetInstance()->UpdatePersonalizedHRTFBin(-1, 10));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test UpdatePersonalizedHRTFBinToHdi null hdi param
* @tc.number : UpdatePersonalizedHRTFBinToHdi_001
* @tc.desc   : Null hdi param returns ERROR.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdatePersonalizedHRTFBinToHdi_001, TestSize.Level1)
{
    auto hdiParamBak = AudioEffectChainManager::GetInstance()->audioEffectHdiParam_;
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = nullptr;
    EXPECT_EQ(ERROR, AudioEffectChainManager::GetInstance()->UpdatePersonalizedHRTFBinToHdi(1, 10));
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = hdiParamBak;
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test IsEffectChainStop branches
* @tc.number : IsEffectChainStop_002
* @tc.desc   : Missing chain of self or other session returns false, single session returns true.
*/
HWTEST(AudioEffectChainManagerUnitTest, IsEffectChainStop_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    // self chain missing
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->IsEffectChainStop("SCENE_MOVIE", "111"));

    auto audioEffectChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_MOVIE", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER"] =
        audioEffectChain;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["111"] = DEFAULT_INFO;
    // only self session uses the chain
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->IsEffectChainStop("SCENE_MOVIE", "111"));

    // other session's chain missing returns false
    SessionEffectInfo otherInfo = DEFAULT_INFO;
    otherInfo.sceneType = "SCENE_GAME";
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["222"] = otherInfo;
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->IsEffectChainStop("SCENE_MOVIE", "111"));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test HasRunningSessionForEffectChain
* @tc.number : HasRunningSessionForEffectChain_001
* @tc.desc   : EFFECT_NONE session is skipped, matching chain returns true.
*/
HWTEST(AudioEffectChainManagerUnitTest, HasRunningSessionForEffectChain_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    auto audioEffectChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_MOVIE", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER"] =
        audioEffectChain;
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["111"] = DEFAULT_INFO;

    SessionEffectInfo noneInfo = DEFAULT_INFO;
    noneInfo.sceneMode = "EFFECT_NONE";
    AudioEffectChainManager::GetInstance()->sessionIDToEffectInfoMap_["222"] = noneInfo;

    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->HasRunningSessionForEffectChain(audioEffectChain));
    auto otherChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_GAME", true);
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->HasRunningSessionForEffectChain(otherChain));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test CheckSceneTypeMatch branches
* @tc.number : CheckSceneTypeMatch_005
* @tc.desc   : Prior scene matches itself, same chain matches only when scene is default.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckSceneTypeMatch_005, TestSize.Level1)
{
    std::string sceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    std::string gameSceneTypeAndDeviceKey = "SCENE_GAME_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    auto audioEffectChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_DEFAULT", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = audioEffectChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[gameSceneTypeAndDeviceKey] =
        audioEffectChain;
    AudioEffectChainManager::GetInstance()->priorSceneList_.push_back("SCENE_MUSIC");
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER"] =
        audioEffectChain;

    // prior scene matches itself
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch("SCENE_MUSIC", "SCENE_MUSIC"));
    // same chain: only the default scene key matches
    EXPECT_TRUE(AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch("SCENE_GAME", "SCENE_DEFAULT"));
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->CheckSceneTypeMatch("SCENE_DEFAULT", "SCENE_GAME"));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChainArm invalid mode
* @tc.number : ExistAudioEffectChainArm_004
* @tc.desc   : Invalid effect mode returns false.
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChainArm_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->ExistAudioEffectChainArm("SCENE_MOVIE",
        static_cast<AudioEffectMode>(9999)));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ExistAudioEffectChainInner branches
* @tc.number : ExistAudioEffectChainInner_004
* @tc.desc   : Nearlink offload returns false, log flag is not reset before init.
*/
HWTEST(AudioEffectChainManagerUnitTest, ExistAudioEffectChainInner_004, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_NEARLINK;
    AudioEffectChainManager::GetInstance()->btOffloadEnabled_ = true;
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->ExistAudioEffectChainInner("SCENE_MOVIE",
        "EFFECT_DEFAULT"));

    AudioEffectChainManager::GetInstance()->isInitialized_ = false;
    AudioEffectChainManager::GetInstance()->initializedLogFlag_ = false;
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->ExistAudioEffectChainInner("SCENE_MOVIE",
        "EFFECT_DEFAULT"));
    EXPECT_FALSE(AudioEffectChainManager::GetInstance()->initializedLogFlag_);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test EffectRotationUpdate with null chain
* @tc.number : EffectRotationUpdate_003
* @tc.desc   : Null chain of scene with session returns ERROR inside ap rotation update.
*/
HWTEST(AudioEffectChainManagerUnitTest, EffectRotationUpdate_003, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToSessionIDMap_["SCENE_MUSIC"].insert("111");
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_.clear();
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_["SCENE_MUSIC_&_DEVICE_TYPE_SPEAKER"] =
        nullptr;
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->EffectRotationUpdate(99));
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test DeleteAllChains keeps default chains
* @tc.number : DeleteAllChains_002
* @tc.desc   : Default scene chains are kept, other scenes are released.
*/
HWTEST(AudioEffectChainManagerUnitTest, DeleteAllChains_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->sceneTypeCountList_.clear();
    std::string sceneTypeAndDeviceKey = "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER";
    std::string defaultSceneTypeAndDeviceKey = "SCENE_DEFAULT_&_DEVICE_TYPE_SPEAKER";
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    auto sceneChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_MOVIE", true);
    auto defaultChain = AudioEffectChainManager::GetInstance()->CreateAudioEffectChain("SCENE_DEFAULT", true);
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[sceneTypeAndDeviceKey] = sceneChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[sceneTypeAndDeviceKey] = 2;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey] = defaultChain;
    AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultSceneTypeAndDeviceKey] = 1;

    AudioEffectChainManager::GetInstance()->DeleteAllChains();
    EXPECT_NE(nullptr,
        AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[defaultSceneTypeAndDeviceKey]);
    EXPECT_EQ(1, AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainCountMap_[defaultSceneTypeAndDeviceKey]);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test RecoverAllChains with scene count list
* @tc.number : RecoverAllChains_001
* @tc.desc   : Chains recorded in sceneTypeCountList_ are recovered.
*/
HWTEST(AudioEffectChainManagerUnitTest, RecoverAllChains_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->sceneTypeCountList_.clear();
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->UpdateSceneTypeList("SCENE_MOVIE", ADD_SCENE_TYPE);
    EXPECT_EQ(SUCCESS, AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic("SCENE_MOVIE"));
    AudioEffectChainManager::GetInstance()->RecoverAllChains();
    EXPECT_NE(nullptr, AudioEffectChainManager::GetInstance()->sceneTypeToEffectChainMap_[
        "SCENE_MOVIE_&_DEVICE_TYPE_SPEAKER"]);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test SetEffectSystemVolumeWithDeviceType API
* @tc.number : SetEffectSystemVolumeWithDeviceType_001
* @tc.desc   : Test SetEffectSystemVolumeWithDeviceType interface for SPEAKER device.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetEffectSystemVolumeWithDeviceType_001, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 0.6f;
    const DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    int32_t result = mgr->SetEffectSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType);
    EXPECT_EQ(SUCCESS, result);

    mgr->ResetInfo();
}

/**
* @tc.name   : Test SetEffectSystemVolumeWithDeviceType API
* @tc.number : SetEffectSystemVolumeWithDeviceType_002
* @tc.desc   : Test SetEffectSystemVolumeWithDeviceType interface for BLUETOOTH_A2DP device.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetEffectSystemVolumeWithDeviceType_002, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 0.7f;
    const DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;

    int32_t result = mgr->SetEffectSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType);
    EXPECT_EQ(SUCCESS, result);

    mgr->ResetInfo();
}

/**
* @tc.name   : Test SetEffectSystemVolumeWithDeviceType API
* @tc.number : SetEffectSystemVolumeWithDeviceType_003
* @tc.desc   : Test SetEffectSystemVolumeWithDeviceType interface for WIRED_HEADSET device.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetEffectSystemVolumeWithDeviceType_003, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    const int32_t systemVolumeType = STREAM_RING;
    const float systemVolume = 0.8f;
    const DeviceType deviceType = DEVICE_TYPE_WIRED_HEADSET;

    int32_t result = mgr->SetEffectSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType);
    EXPECT_EQ(SUCCESS, result);

    mgr->ResetInfo();
}

/**
* @tc.name   : Test SetEffectSystemVolumeWithDeviceType with max volume
* @tc.number : SetEffectSystemVolumeWithDeviceType_MaxVolume_001
* @tc.desc   : Test SetEffectSystemVolumeWithDeviceType with maximum volume (1.0f).
*/
HWTEST(AudioEffectChainManagerUnitTest, SetEffectSystemVolumeWithDeviceType_MaxVolume_001, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 1.0f;
    const DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    int32_t result = mgr->SetEffectSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType);
    EXPECT_EQ(SUCCESS, result);

    mgr->ResetInfo();
}

/**
* @tc.name   : Test SetEffectSystemVolumeWithDeviceType with min volume
* @tc.number : SetEffectSystemVolumeWithDeviceType_MinVolume_001
* @tc.desc   : Test SetEffectSystemVolumeWithDeviceType with minimum volume (0.0f).
*/
HWTEST(AudioEffectChainManagerUnitTest, SetEffectSystemVolumeWithDeviceType_MinVolume_001, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    const int32_t systemVolumeType = STREAM_MUSIC;
    const float systemVolume = 0.0f;
    const DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;

    int32_t result = mgr->SetEffectSystemVolumeWithDeviceType(systemVolumeType, systemVolume, deviceType);
    EXPECT_EQ(SUCCESS, result);

    mgr->ResetInfo();
}

/**
* @tc.name   : Test SetEffectSystemVolumeWithDeviceType for multiple devices
* @tc.number : SetEffectSystemVolumeWithDeviceType_Multiple_001
* @tc.desc   : Test SetEffectSystemVolumeWithDeviceType for multiple device types sequentially.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetEffectSystemVolumeWithDeviceType_Multiple_001, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    const int32_t systemVolumeType = STREAM_MUSIC;

    // Set different volumes for different devices
    int32_t result1 = mgr->SetEffectSystemVolumeWithDeviceType(systemVolumeType, 0.5f, DEVICE_TYPE_SPEAKER);
    int32_t result2 = mgr->SetEffectSystemVolumeWithDeviceType(systemVolumeType, 0.6f, DEVICE_TYPE_BLUETOOTH_A2DP);
    int32_t result3 = mgr->SetEffectSystemVolumeWithDeviceType(systemVolumeType, 0.7f, DEVICE_TYPE_WIRED_HEADSET);

    EXPECT_EQ(SUCCESS, result1);
    EXPECT_EQ(SUCCESS, result2);
    EXPECT_EQ(SUCCESS, result3);

    mgr->ResetInfo();
}

/**
* @tc.name   : Test SetEffectSystemVolumeWithDeviceType for different stream types
* @tc.number : SetEffectSystemVolumeWithDeviceType_DifferentStreams_001
* @tc.desc   : Test SetEffectSystemVolumeWithDeviceType for different stream types.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetEffectSystemVolumeWithDeviceType_DifferentStreams_001, TestSize.Level1)
{
    auto* mgr = AudioEffectChainManager::GetInstance();
    mgr->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    const DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    // Set different volumes for different stream types
    int32_t result1 = mgr->SetEffectSystemVolumeWithDeviceType(STREAM_MUSIC, 0.5f, deviceType);
    int32_t result2 = mgr->SetEffectSystemVolumeWithDeviceType(STREAM_RING, 0.6f, deviceType);
    int32_t result3 = mgr->SetEffectSystemVolumeWithDeviceType(STREAM_NOTIFICATION, 0.7f, deviceType);

    EXPECT_EQ(SUCCESS, result1);
    EXPECT_EQ(SUCCESS, result2);
    EXPECT_EQ(SUCCESS, result3);

    mgr->ResetInfo();
}

/**
* @tc.name   : Test IsChannelLayoutSupportedForDspEffect with DeviceType API
* @tc.number : IsChannelLayoutSupportedForDspEffectWithDeviceType_001
* @tc.desc   : Test IsChannelLayoutSupportedForDspEffect with DeviceType for SPEAKER device.
*/
HWTEST(AudioEffectChainManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_001, TestSize.Level1)
{
    std::shared_ptr<AudioEffectHdiParam> audioEffectHdiParam = std::make_shared<AudioEffectHdiParam>();
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = audioEffectHdiParam;
    AudioEffectChainManager::GetInstance()->InitHdiState();
    bool ret = AudioEffectChainManager::GetInstance()->IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_6POINT0_FRONT, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, false);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test IsChannelLayoutSupportedForDspEffect with DeviceType API
* @tc.number : IsChannelLayoutSupportedForDspEffectWithDeviceType_002
* @tc.desc   : Test IsChannelLayoutSupportedForDspEffect with DeviceType for BLUETOOTH_A2DP device.
*/
HWTEST(AudioEffectChainManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_002, TestSize.Level1)
{
    std::shared_ptr<AudioEffectHdiParam> audioEffectHdiParam = std::make_shared<AudioEffectHdiParam>();
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = audioEffectHdiParam;
    AudioEffectChainManager::GetInstance()->InitHdiState();
    bool ret = AudioEffectChainManager::GetInstance()->IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_STEREO, DEVICE_TYPE_BLUETOOTH_A2DP);
    EXPECT_EQ(ret, true);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test IsChannelLayoutSupportedForDspEffect with DeviceType API
* @tc.number : IsChannelLayoutSupportedForDspEffectWithDeviceType_003
* @tc.desc   : Test IsChannelLayoutSupportedForDspEffect with DeviceType for NEARLINK device
*              (should be converted to BLUETOOTH_A2DP).
*/
HWTEST(AudioEffectChainManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_003, TestSize.Level1)
{
    std::shared_ptr<AudioEffectHdiParam> audioEffectHdiParam = std::make_shared<AudioEffectHdiParam>();
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = audioEffectHdiParam;
    AudioEffectChainManager::GetInstance()->InitHdiState();
    // DEVICE_TYPE_NEARLINK should be converted to DEVICE_TYPE_BLUETOOTH_A2DP internally
    bool ret = AudioEffectChainManager::GetInstance()->IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_2POINT1, DEVICE_TYPE_NEARLINK);
    EXPECT_EQ(ret, false);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test IsChannelLayoutSupportedForDspEffect with DeviceType API
* @tc.number : IsChannelLayoutSupportedForDspEffectWithDeviceType_004
* @tc.desc   : Test IsChannelLayoutSupportedForDspEffect with DeviceType for WIRED_HEADSET device.
*/
HWTEST(AudioEffectChainManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_004, TestSize.Level1)
{
    std::shared_ptr<AudioEffectHdiParam> audioEffectHdiParam = std::make_shared<AudioEffectHdiParam>();
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = audioEffectHdiParam;
    AudioEffectChainManager::GetInstance()->InitHdiState();
    bool ret = AudioEffectChainManager::GetInstance()->IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_MONO, DEVICE_TYPE_WIRED_HEADSET);
    EXPECT_EQ(ret, false);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test IsChannelLayoutSupportedForDspEffect with DeviceType API - nullptr case
* @tc.number : IsChannelLayoutSupportedForDspEffectWithDeviceType_005
* @tc.desc   : Test IsChannelLayoutSupportedForDspEffect with DeviceType when audioEffectHdiParam is nullptr.
*/
HWTEST(AudioEffectChainManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_005, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = nullptr;
    bool ret = AudioEffectChainManager::GetInstance()->IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_5POINT1, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, false);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test IsChannelLayoutSupportedForDspEffect with DeviceType API
* @tc.number : IsChannelLayoutSupportedForDspEffectWithDeviceType_006
* @tc.desc   : Test IsChannelLayoutSupportedForDspEffect with DeviceType for USB_HEADSET device.
*/
HWTEST(AudioEffectChainManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_006, TestSize.Level1)
{
    std::shared_ptr<AudioEffectHdiParam> audioEffectHdiParam = std::make_shared<AudioEffectHdiParam>();
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = audioEffectHdiParam;
    AudioEffectChainManager::GetInstance()->InitHdiState();
    bool ret = AudioEffectChainManager::GetInstance()->IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_7POINT1, DEVICE_TYPE_USB_HEADSET);
    EXPECT_EQ(ret, false);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test IsChannelLayoutSupportedForDspEffect with DeviceType API
* @tc.number : IsChannelLayoutSupportedForDspEffectWithDeviceType_007
* @tc.desc   : Test IsChannelLayoutSupportedForDspEffect with DeviceType for different channel layouts.
*/
HWTEST(AudioEffectChainManagerUnitTest, IsChannelLayoutSupportedForDspEffectWithDeviceType_007, TestSize.Level1)
{
    std::shared_ptr<AudioEffectHdiParam> audioEffectHdiParam = std::make_shared<AudioEffectHdiParam>();
    AudioEffectChainManager::GetInstance()->audioEffectHdiParam_ = audioEffectHdiParam;
    AudioEffectChainManager::GetInstance()->InitHdiState();
    DeviceType deviceType = DEVICE_TYPE_SPEAKER;

    // Test different channel layouts
    bool ret1 = AudioEffectChainManager::GetInstance()->IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_MONO, deviceType);
    bool ret2 = AudioEffectChainManager::GetInstance()->IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_STEREO, deviceType);
    bool ret3 = AudioEffectChainManager::GetInstance()->IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_2POINT1, deviceType);
    bool ret4 = AudioEffectChainManager::GetInstance()->IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_5POINT1, deviceType);
    bool ret5 = AudioEffectChainManager::GetInstance()->IsChannelLayoutSupportedForDspEffect(
        CH_LAYOUT_7POINT1, deviceType);

    EXPECT_EQ(ret1, false);
    EXPECT_EQ(ret2, true);
    EXPECT_EQ(ret3, false);
    EXPECT_EQ(ret4, true);
    EXPECT_EQ(ret5, true);

    AudioEffectChainManager::GetInstance()->ResetInfo();
}
} // namespace AudioStandard
} // namespace OHOS
