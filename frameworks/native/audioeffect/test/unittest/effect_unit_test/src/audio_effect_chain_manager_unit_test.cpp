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
#define LOG_TAG "AudioEffectChainManagerUnitTest"
#endif

#include "audio_effect_chain_manager_unit_test.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_effect.h"
#include "audio_utils.h"
#include "audio_effect_log.h"
#include "audio_effect_chain_manager.h"
#include "audio_errors.h"

using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {

namespace {
constexpr uint32_t INFOCHANNELS = 2;
constexpr uint64_t INFOCHANNELLAYOUT = 0x3;
    
vector<EffectChain> DEFAULT_EFFECT_CHAINS = {{"EFFECTCHAIN_SPK_MUSIC", {}, ""}, {"EFFECTCHAIN_BT_MUSIC", {}, ""}};

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
    "0",
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
* @tc.name   : Test CheckAndAddSessionID API
* @tc.number : CheckAndAddSessionID_001
* @tc.desc   : Test CheckAndAddSessionID interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, CheckAndAddSessionID_001, TestSize.Level1)
{
    string sessionID = "123456";

    bool result = AudioEffectChainManager::GetInstance()->CheckAndAddSessionID(sessionID);
    EXPECT_EQ(true, result);
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
    int32_t result =  AudioEffectChainManager::GetInstance()->ReleaseAudioEffectChainDynamic(sceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
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
    string spatializationEnabled = "0";

    bool result =  AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode,
    spatializationEnabled);
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
    string spatializationEnabled = "0";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    bool result =  AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode,
        spatializationEnabled);
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
    string spatializationEnabled = "";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    bool result =  AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode,
        spatializationEnabled);
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
    string spatializationEnabled = "123";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic(sceneType);
    bool result =  AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode,
        spatializationEnabled);
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
    string spatializationEnabled = "0";

    AudioEffectChainManager::GetInstance()->InitAudioEffectChainManager(DEFAULT_EFFECT_CHAINS,
        DEFAULT_EFFECT_CHAIN_MANAGER_PARAM, DEFAULT_EFFECT_LIBRARY_LIST);
    bool result =  AudioEffectChainManager::GetInstance()->ExistAudioEffectChain(sceneType, effectMode,
        spatializationEnabled);
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
    auto eBufferAttr = make_unique<EffectBufferAttr>(bufIn, bufOut, numChans, frameLen);
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
    auto eBufferAttr = make_unique<EffectBufferAttr>(bufIn, bufOut, numChans, frameLen);
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
    auto eBufferAttr = make_unique<EffectBufferAttr>(bufIn, bufOut, numChans, frameLen);
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
    auto eBufferAttr = make_unique<EffectBufferAttr>(bufIn, bufOut, numChans, frameLen);
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
* @tc.name   : Test UpdateSpatializationState API
* @tc.number : UpdateSpatializationState_001
* @tc.desc   : Test UpdateSpatializationState interface.Test UpdateSensorState,
*              DeleteAllChains and RecoverAllChains interface simultaneously.
*/
HWTEST(AudioEffectChainManagerUnitTest, UpdateSpatializationState_001, TestSize.Level1)
{
    AudioSpatializationState spatializationState = {false, false};

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

    int32_t result = AudioEffectChainManager::GetInstance()->UpdateSpatializationState(spatializationState);
    EXPECT_EQ(SUCCESS, result);
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

/*
 * tc.name   : Test EffectRotationUpdate API
 * tc.number : EffectRotationUpdate_001
 * tc.desc   : Test EffectRotationUpdate interface.
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
}

/*
 * tc.name   : Test EffectVolumeUpdate API
 * tc.number : StreamVolumeUpdate_001
 * tc.desc   : Test EffectVolumeUpdate interface.Test EffectDspVolumeUpdate and EffectApVolumeUpdate simultaneously.
 */
HWTEST(AudioEffectChainManagerUnitTest, StreamVolumeUpdate_001, TestSize.Level1)
{
    string sessionIDString = "123456";
    float streamVolume = 0.666;

    AudioEffectChainManager::GetInstance()->CreateAudioEffectChainDynamic("SCENE_MOVIE");
    int32_t result = AudioEffectChainManager::GetInstance()->StreamVolumeUpdate(sessionIDString, streamVolume);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
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
* @tc.name   : Test SetSpatializationSceneType API
* @tc.number : SetSpatializationSceneType_001
* @tc.desc   : Test SetSpatializationSceneType interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, SetSpatializationSceneType_001, TestSize.Level1)
{
    AudioSpatializationSceneType spatializationSceneType = SPATIALIZATION_SCENE_TYPE_DEFAULT;

    int32_t result = AudioEffectChainManager::GetInstance()->SetSpatializationSceneType(spatializationSceneType);
    EXPECT_EQ(SUCCESS, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test GetCurSpatializationEnabled API
* @tc.number : GetCurSpatializationEnabled_001
* @tc.desc   : Test GetCurSpatializationEnabled interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, GetCurSpatializationEnabled_001, TestSize.Level1)
{
    bool result = AudioEffectChainManager::GetInstance()->GetCurSpatializationEnabled();
    EXPECT_EQ(false, result);
    AudioEffectChainManager::GetInstance()->ResetInfo();
}

/**
* @tc.name   : Test ResetEffectBuffer API
* @tc.number : ResetEffectBuffer_001
* @tc.desc   : Test ResetEffectBuffer interface.
*/
HWTEST(AudioEffectChainManagerUnitTest, ResetEffectBuffer_001, TestSize.Level1)
{
    AudioEffectChainManager::GetInstance()->ResetEffectBuffer();
}
} // namespace AudioStandard
} // namespace OHOS