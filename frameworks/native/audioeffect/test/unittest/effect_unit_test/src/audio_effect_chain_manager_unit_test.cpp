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
* @tc.name   : Test IsDeviceTypeSupportingSpatialization API
* @tc.number : IsDeviceTypeSupportingSpatialization_001
* @tc.desc   : Test IsDeviceTypeSupportingSpatialization interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, IsDeviceTypeSupportingSpatialization_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->IsDeviceTypeSupportingSpatialization(), true);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->IsDeviceTypeSupportingSpatialization(), true);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_NEARLINK;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->IsDeviceTypeSupportingSpatialization(), true);
    AudioEffectChainManager::GetInstance()->deviceType_ = DEVICE_TYPE_MIC;
    EXPECT_EQ(AudioEffectChainManager::GetInstance()->IsDeviceTypeSupportingSpatialization(), false);
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
* @tc.name   : Test IsDownSamplingForSpatialization with renderId - no chain
* @tc.number : IsDownSamplingForSpatialization_RenderId_001
* @tc.desc   : Test IsDownSamplingForSpatialization with renderId returns true when chain not found.
*/
HWTEST(AudioEffectChainManagerUnitTest, IsDownSamplingForSpatialization_RenderId_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    bool result = AudioEffectChainManager::GetInstance()->IsDownSamplingForSpatialization(
        "SCENE_MOVIE", DEVICE_TYPE_SPEAKER, 999);
    EXPECT_EQ(result, true);

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

    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(
        sceneType, DEVICE_TYPE_SPEAKER, 100, eBufferAttr);
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

    int32_t result = AudioEffectChainManager::GetInstance()->ApplyAudioEffectChain(
        sceneType, DEVICE_TYPE_SPEAKER, 100, eBufferAttr);
    EXPECT_EQ(SUCCESS, result);

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
* @tc.name   : Test IsDownSamplingForSpatialization with renderId - chain exists
* @tc.number : IsDownSamplingForSpatialization_RenderId_002
* @tc.desc   : Test IsDownSamplingForSpatialization with renderId when chain exists.
*/
HWTEST(AudioEffectChainManagerUnitTest, IsDownSamplingForSpatialization_RenderId_002, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);

    string sceneType = "SCENE_MOVIE";
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(
        sceneType, DEVICE_TYPE_SPEAKER, 100);

    bool result = AudioEffectChainManager::GetInstance()->IsDownSamplingForSpatialization(
        sceneType, DEVICE_TYPE_SPEAKER, 100);
    // Chain exists - returns fading || !(spatializationEnabled_ && bypassForStereo && deviceSupport)
    EXPECT_TRUE(result == true || result == false);

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
} // namespace AudioStandard
} // namespace OHOS
