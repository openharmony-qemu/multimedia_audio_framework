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
#define LOG_TAG "AudioEnhanceChainUnitTest"
#endif

#include "audio_enhance_chain_unit_test.h"

#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "audio_utils.h"
#include "audio_errors.h"
#include "chain_pool.h"
#include "mock_enhance.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
void AudioEnhanceChainUnitTest::SetUpTestCase(void) {}
void AudioEnhanceChainUnitTest::TearDownTestCase(void) {}
void AudioEnhanceChainUnitTest::SetUp(void) {}
void AudioEnhanceChainUnitTest::TearDown(void) {}

namespace {
std::shared_ptr<AudioEnhanceChain> CreateNewChain()
{
    const uint64_t chainId = 0x1234;
    std::string scene = "SCENE_TEST";
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 0,
        "DEVICE_MIC", "SPEAKER", "SCENE_VOIP_UP", "DEVICE_NAME" };
    const AudioEnhanceDeviceAttr deviceAttr = { 48000, 2, 2, false, 48000, 2, 2, false, 48000, 4, 2 };

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    return chain;
}

void CreateModuleForChain(const std::shared_ptr<AudioEnhanceChain> &chain, AudioEffectLibrary *lib)
{
    if (chain == nullptr) {
        return;
    }
    std::vector<EnhanceModulePara> moduleParas;
    EnhanceModulePara para = { "voip_up", "AIHD", "libvoip_up", lib };
    moduleParas.emplace_back(para);
    chain->CreateAllEnhanceModule(moduleParas);
}
} // namespace

HWTEST(AudioEnhanceChainUnitTest, AddChainRepeat, TestSize.Level1)
{
    EXPECT_EQ(ChainPool::GetInstance().AddChain(nullptr), ERROR);

    const uint64_t chainId = 0x1234;
    std::string scene = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam = {};
    AudioEnhanceDeviceAttr deviceAttr = {};

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, NORMAL_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);

    EXPECT_EQ(ChainPool::GetInstance().AddChain(chain), SUCCESS);
    // add repeat chain
    EXPECT_EQ(ChainPool::GetInstance().AddChain(chain), SUCCESS);

    EXPECT_EQ(ChainPool::GetInstance().DeleteChain(chainId), SUCCESS);
}

HWTEST(AudioEnhanceChainUnitTest, CreateChainSucc, TestSize.Level1)
{
    const uint64_t chainId = 0x1234;
    AudioEnhanceParamAdapter algoParam = {};
    AudioEnhanceDeviceAttr deviceAttr = {};

    std::vector<std::string> sceneArray = { "SCENE_VOIP_UP", "SCENE_RECORD", "SCENE_INVALID" };
    for (const auto &scene : sceneArray) {
        auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, NORMAL_SCENE, algoParam, deviceAttr);
        EXPECT_NE(chain, nullptr);
    }
}

HWTEST(AudioEnhanceChainUnitTest, GetChainIdSucc, TestSize.Level1)
{
    const uint64_t chainId = 0x1234;
    std::string scene = "SCENE_VOIP_UP";
    AudioEnhanceParamAdapter algoParam = {};
    AudioEnhanceDeviceAttr deviceAttr = {};

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, NORMAL_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);
    EXPECT_EQ(chain->GetChainId(), chainId);
    EXPECT_EQ(chain->GetScenePriority(), NORMAL_SCENE);
}

HWTEST(AudioEnhanceChainUnitTest, CreateAllEnhanceModuleFailWithInvalidPara, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    std::vector<EnhanceModulePara> moduleParas;
    EnhanceModulePara para = { "voip_up", "AIHD", "libvoip_up", nullptr };
    moduleParas.emplace_back(para);
    chain->CreateAllEnhanceModule(moduleParas);
    EXPECT_EQ(chain->IsEmptyEnhanceHandles(), true);
}

HWTEST(AudioEnhanceChainUnitTest, CreateAllEnhanceModuleFailWithReturnNullHandle, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    std::vector<EnhanceModulePara> moduleParas;
    AudioEffectLibrary lib = { 0, "name", "implementor", CheckEffectTest, CreateEffectTestFail, ReleaseEffectTest };
    EnhanceModulePara para = { "voip_up", "AIHD", "libvoip_up", &lib };
    moduleParas.emplace_back(para);
    chain->CreateAllEnhanceModule(moduleParas);
    EXPECT_EQ(chain->IsEmptyEnhanceHandles(), true);
}

HWTEST(AudioEnhanceChainUnitTest, CreateAllEnhanceModuleFailWithNullInterface, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    std::vector<EnhanceModulePara> moduleParas;
    EnhanceModulePara para = { "voip_up", "AIHD", "libvoip_up", nullptr };
    moduleParas.emplace_back(para);
    EXPECT_EQ(chain->CreateAllEnhanceModule(moduleParas), ERROR);

    AudioEffectLibrary lib_1 = { 0, "name", "implementor", CheckEffectTest, nullptr, ReleaseEffectTest };
    moduleParas[0].libHandle = &lib_1;
    EXPECT_EQ(chain->CreateAllEnhanceModule(moduleParas), ERROR);

    AudioEffectLibrary lib_2 = { 0, "name", "implementor", CheckEffectTest, CreateEffectTestSucc, nullptr };
    moduleParas[0].libHandle = &lib_2;
    EXPECT_EQ(chain->CreateAllEnhanceModule(moduleParas), ERROR);
}

HWTEST(AudioEnhanceChainUnitTest, CreateAllEnhanceModuleFailWithCmdFail, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    AudioEffectLibrary lib = { 0, "name", "implementor", CheckEffectTest, CreateEffectTestSucc, ReleaseEffectTest };
    std::vector<EnhanceModulePara> moduleParas;
    EnhanceModulePara para = { "voip_up", "AIHD", "libvoip_up", &lib };
    moduleParas.emplace_back(para);

    const std::vector<uint32_t> cmdArray = {
        static_cast<uint32_t>(EFFECT_CMD_INIT),
        static_cast<uint32_t>(EFFECT_CMD_SET_CONFIG),
        static_cast<uint32_t>(EFFECT_CMD_SET_PARAM),
        static_cast<uint32_t>(EFFECT_CMD_GET_CONFIG),
        static_cast<uint32_t>(EFFECT_CMD_SET_PROPERTY),
        static_cast<uint32_t>(EFFECT_CMD_STOP_STREAM),
    };

    ClearCommandRetMap();
    for (auto cmd : cmdArray) {
        SetCommandRet(cmd, ERROR);
        chain->CreateAllEnhanceModule(moduleParas);
        SetCommandRet(cmd, SUCCESS);
    }
    ClearCommandRetMap();
    chain->ReleaseAllEnhanceModule();
}

HWTEST(AudioEnhanceChainUnitTest, CreateAllEnhanceModuleSucc, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    AudioEffectLibrary lib = { 0, "name", "implementor", CheckEffectTest, CreateEffectTestSucc, ReleaseEffectTest };
    std::vector<EnhanceModulePara> moduleParas;
    EnhanceModulePara para = { "voip_up", "AIHD", "libvoip_up", &lib };
    moduleParas.emplace_back(para);
    EXPECT_EQ(chain->CreateAllEnhanceModule(moduleParas), 0);
    EXPECT_EQ(chain->IsEmptyEnhanceHandles(), false);
    chain->ReleaseAllEnhanceModule();
}

HWTEST(AudioEnhanceChainUnitTest, SetThreadHandler_Fail, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);
    EXPECT_EQ(chain->SetThreadHandler(nullptr), ERROR);
}

HWTEST(AudioEnhanceChainUnitTest, StopStreamSucc, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);
    EXPECT_EQ(chain->StopStream(), 0);

    AudioEffectLibrary lib = { 0, "name", "implementor", CheckEffectTest, CreateEffectTestSucc, ReleaseEffectTest };
    CreateModuleForChain(chain, &lib);
    EXPECT_EQ(chain->StopStream(), 0);
    chain->ReleaseAllEnhanceModule();
}

HWTEST(AudioEnhanceChainUnitTest, SetEnhancePropertySucc, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    std::string enhance = "record";
    std::string property = "record_nr";
    EXPECT_EQ(chain->SetEnhanceProperty(enhance, property), SUCCESS);

    AudioEffectLibrary lib = { 0, "name", "implementor", CheckEffectTest, CreateEffectTestSucc, ReleaseEffectTest };
    CreateModuleForChain(chain, &lib);
    EXPECT_EQ(chain->SetEnhanceProperty(enhance, property), SUCCESS);
    chain->ReleaseAllEnhanceModule();
}

HWTEST(AudioEnhanceChainUnitTest, SetEmptyInputDevice, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    std::string inputDevice = "";
    std::string deviceName = "testDevice";
    EXPECT_EQ(chain->SetInputDevice(inputDevice, deviceName), SUCCESS);
}

HWTEST(AudioEnhanceChainUnitTest, SetValidInputDevice, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    std::string inputDevice = "testDevice";
    std::string deviceName = "testDevice";
    EXPECT_EQ(chain->SetInputDevice(inputDevice, deviceName), SUCCESS);
}

HWTEST(AudioEnhanceChainUnitTest, UpdateInputDeviceSucc, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    AudioEffectLibrary lib = { 0, "name", "implementor", CheckEffectTest, CreateEffectTestSucc, ReleaseEffectTest };
    CreateModuleForChain(chain, &lib);

    std::string inputDevice = "testDevice";
    std::string deviceName = "testDevice";
    EXPECT_EQ(chain->SetInputDevice(inputDevice, deviceName), SUCCESS);

    std::string newDevice = "newDevice";
    std::string newDeviceName = "newDeviceName";
    EXPECT_EQ(chain->SetInputDevice(inputDevice, deviceName), SUCCESS);
    chain->ReleaseAllEnhanceModule();
}

HWTEST(AudioEnhanceChainUnitTest, SetFoldStateSucc, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    const uint32_t state = 0;
    EXPECT_EQ(chain->SetFoldState(state), SUCCESS);
}

HWTEST(AudioEnhanceChainUnitTest, UpdateFoldStateSucc, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    AudioEffectLibrary lib = { 0, "name", "implementor", CheckEffectTest, CreateEffectTestSucc, ReleaseEffectTest };
    CreateModuleForChain(chain, &lib);

    const uint32_t state = 1;
    EXPECT_EQ(chain->SetFoldState(state), SUCCESS);
    const uint32_t newState = 3;
    EXPECT_EQ(chain->SetFoldState(newState), SUCCESS);
    chain->ReleaseAllEnhanceModule();
}

HWTEST(AudioEnhanceChainUnitTest, SetEnhanceParamSucc, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    AudioEffectLibrary lib = { 0, "name", "implementor", CheckEffectTest, CreateEffectTestSucc, ReleaseEffectTest };
    CreateModuleForChain(chain, &lib);

    const uint32_t systemVol = 10;
    EXPECT_EQ(chain->SetEnhanceParam(false, systemVol), SUCCESS);
    EXPECT_EQ(chain->SetEnhanceParam(true, systemVol), SUCCESS);
    chain->ReleaseAllEnhanceModule();
}

HWTEST(AudioEnhanceChainUnitTest, GetAlgoConfigSucc, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    AudioBufferConfig micConfig = {};
    AudioBufferConfig ecConfig = {};
    AudioBufferConfig micRefConfig = {};
    chain->GetAlgoConfig(micConfig, ecConfig, micRefConfig);

    const uint32_t testRate = 48000;
    const uint32_t testChannel = 2;

    EXPECT_EQ(micConfig.samplingRate, testRate);
    EXPECT_EQ(micConfig.channels, testChannel);
}

HWTEST(AudioEnhanceChainUnitTest, ApplyEnhanceChainBypass, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    const uint32_t dataLen = 20 * 48 * 2;
    std::vector<int16_t> input(dataLen);
    for (uint32_t i = 0; i < dataLen; ++i) {
        input[i] = i;
    }

    EnhanceTransBuffer transBuf = {};
    transBuf.micData = input.data();
    transBuf.micDataLen = input.size() * sizeof(int16_t);
    EXPECT_EQ(chain->ApplyEnhanceChain(transBuf), SUCCESS);

    std::vector<int16_t> output(dataLen);
    EXPECT_EQ(chain->GetOutputDataFromChain(output.data(), output.size() * sizeof(int16_t)), SUCCESS);
    EXPECT_THAT(output, ElementsAreArray(input));
}

HWTEST(AudioEnhanceChainUnitTest, ApplyEmptyEnhanceChain, TestSize.Level1)
{
    const uint64_t chainId = 0x3456;
    std::string scene = "SCENE_VOIP_UP";
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 0,
        "DEVICE_MIC", "SPEAKER", "SCENE_VOIP_UP", "DEVICE_NAME" };
    const AudioEnhanceDeviceAttr deviceAttr = { 48000, 2, 2, true, 48000, 2, 2, true, 48000, 4, 2 };
    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);
    std::vector<EnhanceModulePara> moduleParas;
    chain->CreateAllEnhanceModule(moduleParas);

    const uint32_t ecDataLen = 20 * 48 * 2;
    const uint32_t micDataLen = 20 * 48 * 2;
    const uint32_t micRefDataLen = 20 * 48 * 4;
    std::vector<int16_t> micInput(micDataLen);
    std::vector<int16_t> ecInput(ecDataLen);
    std::vector<int16_t> micRefInput(micRefDataLen);
    for (uint32_t i = 0; i < micDataLen; ++i) {
        micInput[i] = i + 1;
    }

    EnhanceTransBuffer transBuf = { ecInput.data(), micInput.data(), micRefInput.data(),
        ecDataLen * sizeof(int16_t), micDataLen * sizeof(int16_t), micRefDataLen * sizeof(int16_t) };
    EXPECT_EQ(chain->ApplyEnhanceChain(transBuf), SUCCESS);

    std::vector<int16_t> output(micDataLen);
    EXPECT_EQ(chain->GetOutputDataFromChain(output.data(), output.size() * sizeof(int16_t)), SUCCESS);
    EXPECT_THAT(output, ElementsAreArray(micInput));
    chain->ReleaseAllEnhanceModule();
}

HWTEST(AudioEnhanceChainUnitTest, ApplyEnhanceChainSucc, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    AudioEffectLibrary lib = { 0, "name", "implementor", CheckEffectTest, CreateEffectTestSucc, ReleaseEffectTest };
    CreateModuleForChain(chain, &lib);

    const uint32_t chlNum = 2;
    const uint32_t factor = 10;
    const uint32_t dataLen = 20 * 48 * chlNum;
    std::vector<int16_t> input(dataLen);
    std::vector<int16_t> targetOut(dataLen);
    for (uint32_t i = 0; i < dataLen / chlNum; ++i) {
        for (uint32_t j = 0; j < chlNum; ++j) {
            input[i * chlNum + j] = i;
            targetOut[j * dataLen / chlNum + i] = i * factor;
        }
    }

    EnhanceTransBuffer transBuf = {};
    transBuf.micData = input.data();
    transBuf.micDataLen = input.size() * sizeof(int16_t);
    EXPECT_EQ(chain->ApplyEnhanceChain(transBuf), SUCCESS);

    std::vector<int16_t> output(dataLen);
    EXPECT_EQ(chain->GetOutputDataFromChain(output.data(), output.size() * sizeof(int16_t)), SUCCESS);
    EXPECT_THAT(output, ElementsAreArray(targetOut));
    chain->ReleaseAllEnhanceModule();
}

HWTEST(AudioEnhanceChainUnitTest, InitAudioEnhanceChain_MicChannelsLessThanDefault, TestSize.Level1)
{
    const uint64_t chainId = 0x5678; // 0x5678 for test chainid
    std::string scene = "SCENE_TEST";
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 0, // 20 for volume, 3 for state
        "DEVICE_TYPE_MIC", "SPEAKER", "SCENE_VOIP_UP", "DEVICE_NAME" };
    const AudioEnhanceDeviceAttr deviceAttr = { 48000, 2, 2, false, 48000, 2, 2, false, 48000, 2, 2 };

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);
    EXPECT_EQ(chain->GetChainId(), chainId);
}

HWTEST(AudioEnhanceChainUnitTest, InitAudioEnhanceChain_MicChannelsEqualDefault, TestSize.Level1)
{
    const uint64_t chainId = 0x5679; // 0x5679 for test chainid
    std::string scene = "SCENE_TEST";
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 0, // 20 for volume, 3 for state
        "DEVICE_TYPE_MIC", "SPEAKER", "SCENE_VOIP_UP", "DEVICE_NAME" };
    const AudioEnhanceDeviceAttr deviceAttr =
        { 48000, 4, 4, false, 48000, 2, 2, false, 48000, 4, 4 }; // 48000 for rate, 4 for ch and format

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);
    EXPECT_EQ(chain->GetChainId(), chainId);
}

HWTEST(AudioEnhanceChainUnitTest, InitAudioEnhanceChain_MicChannelsGreaterThanDefault, TestSize.Level1)
{
    const uint64_t chainId = 0x5680; // 0x5680 for test chainid
    std::string scene = "SCENE_TEST";
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 0, // 20 for volume, 3 for state
        "DEVICE_TYPE_MIC", "SPEAKER", "SCENE_VOIP_UP", "DEVICE_NAME" };
    const AudioEnhanceDeviceAttr deviceAttr =
        { 48000, 6, 6, false, 48000, 2, 2, false, 48000, 6, 6 }; // 48000 for rate, 6 and 2 for ch and format

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);
    EXPECT_EQ(chain->GetChainId(), chainId);
}

HWTEST(AudioEnhanceChainUnitTest, InitAudioEnhanceChain_PreDeviceNotDefault, TestSize.Level1)
{
    const uint64_t chainId = 0x5681; // 0x5681 for test chainid
    std::string scene = "SCENE_TEST";
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 0, // 20 for volume, 3 for state
        "DEVICE_OTHER", "SPEAKER", "SCENE_VOIP_UP", "DEVICE_NAME" };
    const AudioEnhanceDeviceAttr deviceAttr =
        { 48000, 2, 2, false, 48000, 2, 2, false, 48000, 2, 2 }; // 48000 for rate, 2 for ch and format

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);
    EXPECT_EQ(chain->GetChainId(), chainId);
}

HWTEST(AudioEnhanceChainUnitTest, InitAudioEnhanceChain_NeedNotEc, TestSize.Level1)
{
    const uint64_t chainId = 0x5681; // 0x5681 for test chainid
    std::string scene = "SCENE_VOIP_UP";
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 0, // 20 for volume, 3 for state
        "DEVICE_OTHER", "SPEAKER", "SCENE_VOIP_UP", "DEVICE_NAME" };
    const AudioEnhanceDeviceAttr deviceAttr =
        { 48000, 2, 2, false, 48000, 2, 2, false, 48000, 2, 2 }; // 48000 for rate, 2 for ch and format, needEc is false

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);
    EXPECT_NE(chain->algoSupportedConfig_.ecNum, 2); // 2 for DEFAULT_ECON_CH
}

HWTEST(AudioEnhanceChainUnitTest, InitAudioEnhanceChain_NeedEc, TestSize.Level1)
{
    const uint64_t chainId = 0x5681; // 0x5681 for test chainid
    std::string scene = "SCENE_VOIP_UP";
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 0, // 20 for volume, 3 for state
        "DEVICE_OTHER", "SPEAKER", "SCENE_VOIP_UP", "DEVICE_NAME" };
    const AudioEnhanceDeviceAttr deviceAttr =
        { 48000, 2, 2, true, 48000, 2, 2, false, 48000, 2, 2 }; // 48000 for rate, 2 for ch and format, needEc is true

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);
    EXPECT_EQ(chain->algoSupportedConfig_.ecNum, 2); // 2 for DEFAULT_ECON_CH
}

HWTEST(AudioEnhanceChainUnitTest, InitAudioEnhanceChain_NeedNotMicRef, TestSize.Level1)
{
    const uint64_t chainId = 0x5681; // 0x5681 for test chainid
    std::string scene = "SCENE_VOIP_UP";
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 0, // 20 for volume, 3 for state
        "DEVICE_OTHER", "SPEAKER", "SCENE_VOIP_UP", "DEVICE_NAME" };
    // needMicRef is false
    const AudioEnhanceDeviceAttr deviceAttr =
        { 48000, 2, 2, false, 48000, 2, 2, false, 48000, 2, 2 }; // 48000 for rate, 2 for ch and format

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);
    EXPECT_NE(chain->algoSupportedConfig_.micRefNum, 4); // 4 for DEFAULT_MICREFON_CH
}

HWTEST(AudioEnhanceChainUnitTest, InitAudioEnhanceChain_NeedMicRef, TestSize.Level1)
{
    const uint64_t chainId = 0x5681; // 0x5681 for test chainid
    std::string scene = "SCENE_VOIP_UP";
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 0, // 20 for volume, 3 for state
        "DEVICE_OTHER", "SPEAKER", "SCENE_VOIP_UP", "DEVICE_NAME" };
    // needMicRef is true
    const AudioEnhanceDeviceAttr deviceAttr =
        { 48000, 2, 2, false, 48000, 2, 2, true, 48000, 4, 2 }; // 48000 for rate, 2 for ch and format

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);
    EXPECT_EQ(chain->algoSupportedConfig_.micRefNum, 4); // 4 for DEFAULT_MICREFON_CH
}

HWTEST(AudioEnhanceChainUnitTest, SetAppsMuteStateSucc, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    // Test set apps mute state without effect module
    uint32_t appsMuteState = 1;
    EXPECT_EQ(chain->SetAppsMuteState(appsMuteState), SUCCESS);

    AudioEffectLibrary lib = { 0, "name", "implementor", CheckEffectTest, CreateEffectTestSucc, ReleaseEffectTest };
    CreateModuleForChain(chain, &lib);

    // Test set apps mute state with effect module
    EXPECT_EQ(chain->SetAppsMuteState(appsMuteState), SUCCESS);

    // Test set same state again (no update)
    EXPECT_EQ(chain->SetAppsMuteState(appsMuteState), SUCCESS);

    // Test set different state
    uint32_t newMuteState = 0;
    EXPECT_EQ(chain->SetAppsMuteState(newMuteState), SUCCESS);

    chain->ReleaseAllEnhanceModule();
}

HWTEST(AudioEnhanceChainUnitTest, SetAppsMuteStateWithDifferentValues, TestSize.Level1)
{
    auto chain = CreateNewChain();
    EXPECT_NE(chain, nullptr);

    AudioEffectLibrary lib = { 0, "name", "implementor", CheckEffectTest, CreateEffectTestSucc, ReleaseEffectTest };
    CreateModuleForChain(chain, &lib);

    // Test various mute state values
    std::vector<uint32_t> muteStates = { 0, 1, 2, 10, 100, UINT32_MAX };
    for (auto state : muteStates) {
        EXPECT_EQ(chain->SetAppsMuteState(state), SUCCESS);
    }

    chain->ReleaseAllEnhanceModule();
}

HWTEST(AudioEnhanceChainUnitTest, SetAppsMuteStateWithAlgoParam, TestSize.Level1)
{
    const uint64_t chainId = 0x1234;
    std::string scene = "SCENE_TEST";
    // algoParam includes appsMuteState field
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 1,
        "DEVICE_MIC", "SPEAKER", "SCENE_VOIP_UP", "DEVICE_NAME" };
    const AudioEnhanceDeviceAttr deviceAttr = { 48000, 2, 2, false, 48000, 2, 2, false, 48000, 4, 2 };

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);

    // Test set same state as initial state (should not update)
    EXPECT_EQ(chain->SetAppsMuteState(1), SUCCESS);

    // Test set different state
    EXPECT_EQ(chain->SetAppsMuteState(0), SUCCESS);
}

HWTEST(AudioEnhanceChainUnitTest, CreateCollaborativeRecordChain, TestSize.Level1)
{
    const uint64_t chainId = 0x5678;
    std::string scene = "SCENE_COLLABORATIVE_RECORD";
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 0,
        "DEVICE_MIC", "SPEAKER", "SCENE_COLLABORATIVE_RECORD", "DEVICE_NAME" };
    const AudioEnhanceDeviceAttr deviceAttr = { 48000, 2, 2, false, 48000, 2, 2, false, 48000, 2, 2 };

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);
    EXPECT_EQ(chain->GetChainId(), chainId);

    AudioBufferConfig micConfig = {};
    AudioBufferConfig ecConfig = {};
    AudioBufferConfig micRefConfig = {};
    chain->GetAlgoConfig(micConfig, ecConfig, micRefConfig);

    EXPECT_EQ(micConfig.samplingRate, 48000);
    EXPECT_EQ(micConfig.channels, 2);
    EXPECT_EQ(micConfig.format, 32);
    EXPECT_EQ(ecConfig.samplingRate, 0);
    EXPECT_EQ(ecConfig.channels, 0);
    EXPECT_EQ(micRefConfig.samplingRate, 48000);
    EXPECT_EQ(micRefConfig.channels, 2);
    EXPECT_EQ(micRefConfig.format, 32);
}

HWTEST(AudioEnhanceChainUnitTest, CreateCollaborativeRecordChainWithModule, TestSize.Level1)
{
    const uint64_t chainId = 0x5679;
    std::string scene = "SCENE_COLLABORATIVE_RECORD";
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 0,
        "DEVICE_MIC", "SPEAKER", "SCENE_COLLABORATIVE_RECORD", "DEVICE_NAME" };
    const AudioEnhanceDeviceAttr deviceAttr = { 48000, 2, 2, false, 48000, 2, 2, false, 48000, 2, 2 };

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);

    AudioEffectLibrary lib = { 0, "name", "implementor", CheckEffectTest, CreateEffectTestSucc, ReleaseEffectTest };
    std::vector<EnhanceModulePara> moduleParas;
    EnhanceModulePara para = { "collab_record", "AIHD", "libcollab_record", &lib };
    moduleParas.emplace_back(para);
    EXPECT_EQ(chain->CreateAllEnhanceModule(moduleParas), 0);
    EXPECT_EQ(chain->IsEmptyEnhanceHandles(), false);
    chain->ReleaseAllEnhanceModule();
}

HWTEST(AudioEnhanceChainUnitTest, ApplyCollaborativeRecordChain, TestSize.Level1)
{
    const uint64_t chainId = 0x5680;
    std::string scene = "SCENE_COLLABORATIVE_RECORD";
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 0,
        "DEVICE_MIC", "SPEAKER", "SCENE_COLLABORATIVE_RECORD", "DEVICE_NAME" };
    const AudioEnhanceDeviceAttr deviceAttr = { 48000, 2, 2, false, 48000, 2, 2, false, 48000, 2, 2 };

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);

    const uint32_t dataLen = 20 * 48 * 2 * 4;
    std::vector<float> micInput(dataLen / sizeof(float));
    std::vector<float> micRefInput(dataLen / sizeof(float));
    for (uint32_t i = 0; i < micInput.size(); ++i) {
        micInput[i] = static_cast<float>(i) * 0.01f;
        micRefInput[i] = static_cast<float>(i) * 0.02f;
    }

    EnhanceTransBuffer transBuf = {};
    transBuf.micData = micInput.data();
    transBuf.micDataLen = micInput.size() * sizeof(float);
    transBuf.micRefData = micRefInput.data();
    transBuf.micRefDataLen = micRefInput.size() * sizeof(float);
    EXPECT_EQ(chain->ApplyEnhanceChain(transBuf), SUCCESS);

    std::vector<float> output(micInput.size());
    EXPECT_EQ(chain->GetOutputDataFromChain(output.data(), output.size() * sizeof(float)), SUCCESS);
}

HWTEST(AudioEnhanceChainUnitTest, ApplyCollaborativeRecordChainWithModule, TestSize.Level1)
{
    const uint64_t chainId = 0x5681;
    std::string scene = "SCENE_COLLABORATIVE_RECORD";
    const AudioEnhanceParamAdapter algoParam = { 0, 20, 3, 0, 0,
        "DEVICE_MIC", "SPEAKER", "SCENE_COLLABORATIVE_RECORD", "DEVICE_NAME" };
    const AudioEnhanceDeviceAttr deviceAttr = { 48000, 2, 2, false, 48000, 2, 2, false, 48000, 2, 2 };

    auto chain = std::make_shared<AudioEnhanceChain>(chainId, scene, PRIOR_SCENE, algoParam, deviceAttr);
    EXPECT_NE(chain, nullptr);

    AudioEffectLibrary lib = { 0, "name", "implementor", CheckEffectTest, CreateEffectTestSucc, ReleaseEffectTest };
    std::vector<EnhanceModulePara> moduleParas;
    EnhanceModulePara para = { "collab_record", "AIHD", "libcollab_record", &lib };
    moduleParas.emplace_back(para);
    EXPECT_EQ(chain->CreateAllEnhanceModule(moduleParas), 0);

    const uint32_t dataLen = 20 * 48 * 2 * 4;
    std::vector<float> micInput(dataLen / sizeof(float));
    std::vector<float> micRefInput(dataLen / sizeof(float));
    for (uint32_t i = 0; i < micInput.size(); ++i) {
        micInput[i] = static_cast<float>(i) * 0.01f;
        micRefInput[i] = static_cast<float>(i) * 0.02f;
    }

    EnhanceTransBuffer transBuf = {};
    transBuf.micData = micInput.data();
    transBuf.micDataLen = micInput.size() * sizeof(float);
    transBuf.micRefData = micRefInput.data();
    transBuf.micRefDataLen = micRefInput.size() * sizeof(float);
    EXPECT_EQ(chain->ApplyEnhanceChain(transBuf), SUCCESS);

    std::vector<float> output(micInput.size());
    EXPECT_EQ(chain->GetOutputDataFromChain(output.data(), output.size() * sizeof(float)), SUCCESS);

    chain->ReleaseAllEnhanceModule();
}
} // AudioStandard
} // OHOS
