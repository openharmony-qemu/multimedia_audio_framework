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

#undef LOG_TAG
#define LOG_TAG "AudioEnhanceChainManagerUnitTest"

#include "audio_enhance_chain_manager_unit_test.h"
#include "audio_enhance_chain_manager.h"
#include <chrono>
#include <thread>
#include <fstream>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "audio_effect.h"
#include "audio_effect_log.h"
#include "audio_errors.h"
#include "mock_enhance.h"


using namespace std;
using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr uint32_t SCENE_TYPE_OFFSET = 32;
constexpr uint32_t CAPTURER_ID_OFFSET = 16;
constexpr uint32_t TEST_CAPTURER_ID = 4096;
constexpr uint32_t TEST_RENDER_ID = 128;
constexpr uint32_t MAX_EXTRA_NUM = 2;
const AudioEnhanceDeviceAttr TEST_DEVICE_ATTR = { 48000, 4, 2, false, 48000, 2, 2, false, 48000, 4, 2 };

uint64_t GetValidSceneKeyCode(AudioEnhanceScene sceneType, uint32_t captureId)
{
    uint64_t sceneCode = static_cast<uint64_t>(sceneType);
    uint64_t sceneKeyCode = (sceneCode << SCENE_TYPE_OFFSET) + (captureId << CAPTURER_ID_OFFSET) + TEST_RENDER_ID;
    return sceneKeyCode;
}
} // namespace

AudioEnhanceChainManager* AudioEnhanceChainManagerUnitTest::manager_ = nullptr;
std::vector<EffectChain> AudioEnhanceChainManagerUnitTest::enhanceChains_;
EffectChainManagerParam AudioEnhanceChainManagerUnitTest::managerParam_;
std::vector<std::shared_ptr<AudioEffectLibEntry>> AudioEnhanceChainManagerUnitTest::enhanceLibraryList_;

void AudioEnhanceChainManagerUnitTest::SetUpTestSuite(void)
{
    enhanceChains_ = {
        { "EFFECTCHAIN_VOIP_UP", { "voip_up_test" }, "label" },
        { "EFFECTCHAIN_RECORD", { "record_test" }, "label" },
        { "EFFECTCHAIN_TRANS", { "trans_test" }, "label" },
        { "EFFECTCHAIN_VOICE_MESSAGE", { "voice_message_test" }, "label" },
    };

    managerParam_.maxExtraNum = MAX_EXTRA_NUM;
    managerParam_.defaultSceneName = "SCENE_RECORD";
    managerParam_.priorSceneList = { "SCENE_VOIP_UP" };
    managerParam_.sceneTypeToChainNameMap = {
        { "SCENE_VOIP_UP_&_ENHANCE_DEFAULT", "EFFECTCHAIN_VOIP_UP" },
        { "SCENE_RECORD_&_ENHANCE_DEFAULT", "EFFECTCHAIN_RECORD" },
        { "SCENE_PRE_ENHANCE_&_ENHANCE_DEFAULT", "EFFECTCHAIN_TRANS" },
        { "SCENE_VOICE_MESSAGE_&_ENHANCE_DEFAULT", "EFFECTCHAIN_VOICE_MESSAGE" },
    };
    managerParam_.effectDefaultProperty = {
        { "voip_up_test", "AIHD" },
        { "record_test", "NROFF" },
        { "voice_message_test", "NROFF" },
    };

    static AudioEffectLibrary lib = { 0, "name", "impl", CheckEffectTest, CreateEffectTestSucc, ReleaseEffectTest };
    std::vector<std::string> effectArray = { "voip_up_test", "record_test", "trans_test", "voice_message_test" };
    auto libEntry = std::make_shared<AudioEffectLibEntry>();
    if (libEntry != nullptr) {
        libEntry->audioEffectLibHandle = &lib;
        libEntry->libraryName = "enhance_test_lib";
        libEntry->effectName = effectArray;
    }
    enhanceLibraryList_ = { nullptr, libEntry };

    manager_ = AudioEnhanceChainManager::GetInstance();
}

void AudioEnhanceChainManagerUnitTest::TearDownTestSuite(void)
{
}

void AudioEnhanceChainManagerUnitTest::SetUp(void)
{
}

void AudioEnhanceChainManagerUnitTest::TearDown(void)
{
    AudioEnhanceChainManager::GetInstance()->ResetInfo();
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamicFailWithNoEnhance, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_VOIP_UP, TEST_CAPTURER_ID);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), ERROR);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamicFailWithNoDevice, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_VOIP_UP, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), ERROR);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamicFailWithNoConfig, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_ASR, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), ERROR);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamicWithErrorDeviceOrScene, TestSize.Level1)
{
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);

    const uint32_t testCaseNum = 4;
    const std::vector<AudioEnhanceScene> sceneArray = { static_cast<AudioEnhanceScene>(0x78), SCENE_RECORD,
        SCENE_RECORD, SCENE_RECORD };
    const std::vector<uint32_t> captureIdArray = { 0x700, 0x800, 0x900, 0x110 };
    const std::vector<DeviceType> deviceArray = { DEVICE_TYPE_MIC, DEVICE_TYPE_INVALID, DEVICE_TYPE_NONE,
        static_cast<DeviceType>(0x4567) };
    const std::vector<int32_t> expectRetArray = { ERROR, SUCCESS, SUCCESS, ERROR };

    for (uint32_t i = 0; i < testCaseNum; ++i) {
        auto sceneKeyCode = GetValidSceneKeyCode(sceneArray[i], captureIdArray[i]);
        EXPECT_EQ(manager_->SetInputDevice(captureIdArray[i], deviceArray[i]), SUCCESS);
        EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), expectRetArray[i]);
        EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
    }
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateMultiAudioEnhanceChainSucc, TestSize.Level1)
{
    const uint32_t tempCaptureId = 0x800;
    std::vector<uint64_t> sceneKeyCodeArray = {
        GetValidSceneKeyCode(SCENE_VOIP_UP, TEST_CAPTURER_ID),
        GetValidSceneKeyCode(SCENE_VOIP_UP, tempCaptureId),
        GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID),
        GetValidSceneKeyCode(SCENE_PRE_ENHANCE, TEST_CAPTURER_ID),
        GetValidSceneKeyCode(SCENE_PRE_ENHANCE, tempCaptureId),
        GetValidSceneKeyCode(SCENE_VOICE_MESSAGE, TEST_CAPTURER_ID),
        GetValidSceneKeyCode(SCENE_VOICE_MESSAGE, tempCaptureId),
    };

    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->SetInputDevice(tempCaptureId, DEVICE_TYPE_USB_HEADSET), SUCCESS);
    for (const auto sceneKeyCode : sceneKeyCodeArray) {
        int32_t result = manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR);
        EXPECT_EQ(result, SUCCESS);
    }

    for (const auto sceneKeyCode : sceneKeyCodeArray) {
        EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
    }
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainDynamicRepeat, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, ReleaseNotExistAudioEnhanceChain, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, AudioEnhanceChainGetAlgoConfigSucc, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    AudioBufferConfig micConfig = {};
    AudioBufferConfig ecConfig = {};
    AudioBufferConfig micRefConfig = {};
    // use not exist sceneKeyCode
    const uint64_t invalidKeyCode = 0x123456;
    manager_->AudioEnhanceChainGetAlgoConfig(invalidKeyCode, micConfig, ecConfig, micRefConfig);
    manager_->AudioEnhanceChainGetAlgoConfig(sceneKeyCode, micConfig, ecConfig, micRefConfig);
    EXPECT_EQ(micConfig.channels, TEST_DEVICE_ATTR.micChannels);
    EXPECT_EQ(micConfig.samplingRate, TEST_DEVICE_ATTR.micRate);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, SetInvalidInputDevice, TestSize.Level1)
{
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);

    DeviceType invalidDevice = static_cast<DeviceType>(0x1234);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, invalidDevice), ERROR);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, SetInputAndOutputDeviceSucc, TestSize.Level1)
{
    const uint32_t tempRenderId = 0x400;
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    // set repeat device
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->SetOutputDevice(tempRenderId, DEVICE_TYPE_SPEAKER), SUCCESS);

    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    // update device
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_USB_HEADSET), SUCCESS);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, SetVolumeInfoSucc, TestSize.Level1)
{
    const float systemVol = 0.2;
    EXPECT_EQ(manager_->SetVolumeInfo(STREAM_SYSTEM, systemVol), SUCCESS);

    const uint32_t sessionId = 0x6789;
    const float streamVol = 0.5;
    EXPECT_EQ(manager_->SetStreamVolumeInfo(sessionId, streamVol), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, SetMicrophoneMuteInfoSucc, TestSize.Level1)
{
    EXPECT_EQ(manager_->SetMicrophoneMuteInfo(true), SUCCESS);

    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    EXPECT_EQ(manager_->SetMicrophoneMuteInfo(true), SUCCESS);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, NotifyChainToStopStreamSucc, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    EXPECT_EQ(manager_->NotifyChainToStopStream(), SUCCESS);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, UpdateExtraSceneTypeFoldState, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    std::string validMainKey = "device_status";
    std::string invalidMainKey = "device_status_test";
    std::string validSubKey = "fold_state";
    std::string invalidSubKey = "fold_state_test";
    std::string validExtraSceneType = "3";
    std::string invalidExtraSceneType = "@#^#%^@#()";
    manager_->UpdateExtraSceneType(invalidMainKey, invalidSubKey, invalidExtraSceneType);
    manager_->UpdateExtraSceneType(invalidMainKey, validSubKey, validExtraSceneType);
    manager_->UpdateExtraSceneType(validMainKey, invalidSubKey, invalidExtraSceneType);
    manager_->UpdateExtraSceneType(validMainKey, validSubKey, invalidExtraSceneType);
    manager_->UpdateExtraSceneType(validMainKey, validSubKey, validExtraSceneType);
    manager_->UpdateExtraSceneType(validMainKey, validSubKey, validExtraSceneType);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, UpdateExtraSceneTypePowerState, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    std::string validMainKey = "device_status";
    std::string invalidMainKey = "device_status_test";
    std::string validSubKey = "power_state";
    std::string invalidSubKey = "power_state_test";
    std::string validExtraSceneType = "20";
    std::string invalidExtraSceneType = "@#^#%^@#()";
    manager_->UpdateExtraSceneType(invalidMainKey, invalidSubKey, invalidExtraSceneType);
    manager_->UpdateExtraSceneType(invalidMainKey, validSubKey, validExtraSceneType);
    manager_->UpdateExtraSceneType(validMainKey, invalidSubKey, invalidExtraSceneType);
    manager_->UpdateExtraSceneType(validMainKey, validSubKey, invalidExtraSceneType);
    manager_->UpdateExtraSceneType(validMainKey, validSubKey, validExtraSceneType);
    manager_->UpdateExtraSceneType(validMainKey, validSubKey, validExtraSceneType);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, ApplyEnhanceChainNotExist, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    EnhanceTransBuffer transBuf = {};
    EXPECT_EQ(manager_->ApplyEnhanceChainById(sceneKeyCode, transBuf), ERROR);
    EXPECT_EQ(manager_->GetChainOutputDataById(sceneKeyCode, nullptr, 0), ERROR);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, ApplyEnhanceChainSucc, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    const uint32_t waitTime = 10;
    const uint32_t chlNum = 4;
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
    std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
    EXPECT_EQ(manager_->ApplyEnhanceChainById(sceneKeyCode, transBuf), SUCCESS);

    std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
    std::vector<int16_t> output(dataLen);
    EXPECT_EQ(manager_->GetChainOutputDataById(sceneKeyCode, output.data(), output.size() * sizeof(int16_t)), SUCCESS);
    EXPECT_THAT(output, ElementsAreArray(targetOut));

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAppsMuteStateToChainByIdSucc, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    // Test set apps mute state to true
    EXPECT_EQ(manager_->SetAppsMuteStateToChainById(sceneKeyCode, true), SUCCESS);

    // Test set apps mute state to false
    EXPECT_EQ(manager_->SetAppsMuteStateToChainById(sceneKeyCode, false), SUCCESS);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAppsMuteStateToChainByIdWithInvalidCode, TestSize.Level1)
{
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);

    // Test with invalid scene key code
    const uint64_t invalidKeyCode = 0x123456;
    EXPECT_EQ(manager_->SetAppsMuteStateToChainById(invalidKeyCode, true), ERROR);
    EXPECT_EQ(manager_->SetAppsMuteStateToChainById(invalidKeyCode, false), ERROR);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAppsMuteStateToChainByIdMultiScene, TestSize.Level1)
{
    const uint32_t tempCaptureId = 0x800;
    std::vector<uint64_t> sceneKeyCodeArray = {
        GetValidSceneKeyCode(SCENE_VOIP_UP, TEST_CAPTURER_ID),
        GetValidSceneKeyCode(SCENE_RECORD, tempCaptureId),
    };

    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->SetInputDevice(tempCaptureId, DEVICE_TYPE_USB_HEADSET), SUCCESS);

    for (const auto sceneKeyCode : sceneKeyCodeArray) {
        EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);
    }

    // Test set apps mute state for multiple chains
    for (const auto sceneKeyCode : sceneKeyCodeArray) {
        EXPECT_EQ(manager_->SetAppsMuteStateToChainById(sceneKeyCode, true), SUCCESS);
        EXPECT_EQ(manager_->SetAppsMuteStateToChainById(sceneKeyCode, false), SUCCESS);
    }

    for (const auto sceneKeyCode : sceneKeyCodeArray) {
        EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
    }
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAndGetAudioEnhanceProperty, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_VOIP_UP, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    AudioEffectPropertyArray propertyArray = {};
    // voip_up_test is relate with device, key will append device type name
    propertyArray.property.push_back(AudioEffectProperty{ "voip_up_test", "AIHD_ON", CAPTURE_EFFECT_FLAG });
    // record_test is not relate with device
    propertyArray.property.push_back(AudioEffectProperty{ "record_test", "NROFF_ON", CAPTURE_EFFECT_FLAG });
    EXPECT_EQ(manager_->SetAudioEnhanceProperty(propertyArray, DEVICE_TYPE_MIC), SUCCESS);

    // deviceType is valid, refresh enhance property map from db before get
    AudioEffectPropertyArray getPropertyArray = {};
    EXPECT_EQ(manager_->GetAudioEnhanceProperty(getPropertyArray, DEVICE_TYPE_MIC), SUCCESS);
    bool foundVoipProp = false;
    bool foundRecordProp = false;
    for (const auto &property : getPropertyArray.property) {
        EXPECT_FALSE(property.category.empty());
        foundVoipProp = foundVoipProp || property.name == "voip_up_test";
        foundRecordProp = foundRecordProp || property.name == "record_test";
    }
    EXPECT_TRUE(foundVoipProp);
    EXPECT_TRUE(foundRecordProp);

    // deviceType is none, get property map directly
    AudioEffectPropertyArray noneTypeArray = {};
    EXPECT_EQ(manager_->GetAudioEnhanceProperty(noneTypeArray, DEVICE_TYPE_NONE), SUCCESS);
    // invalid device type, UpdateEnhancePropertyMapFromDb return early
    AudioEffectPropertyArray invalidTypeArray = {};
    EXPECT_EQ(manager_->GetAudioEnhanceProperty(invalidTypeArray, static_cast<DeviceType>(0x4321)), SUCCESS);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, SetAudioEnhancePropertyWithEmptyCategory, TestSize.Level1)
{
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);

    AudioEffectPropertyArray propertyArray = {};
    propertyArray.property.push_back(AudioEffectProperty{ "record_test", "", CAPTURE_EFFECT_FLAG });
    EXPECT_EQ(manager_->SetAudioEnhanceProperty(propertyArray, DEVICE_TYPE_NONE), SUCCESS);

    // empty category property will not be return
    AudioEffectPropertyArray getPropertyArray = {};
    EXPECT_EQ(manager_->GetAudioEnhanceProperty(getPropertyArray, DEVICE_TYPE_NONE), SUCCESS);
    bool hasEmptyCategory = false;
    bool hasRecordTest = false;
    for (const auto &property : getPropertyArray.property) {
        hasEmptyCategory = hasEmptyCategory || property.category.empty();
        hasRecordTest = hasRecordTest || property.name == "record_test";
    }
    EXPECT_FALSE(hasEmptyCategory);
    EXPECT_FALSE(hasRecordTest);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateAudioEnhanceChainFailWithLibNotExist, TestSize.Level1)
{
    std::vector<EffectChain> enhanceChains = { { "EFFECTCHAIN_NO_LIB", { "enhance_no_lib" }, "label" } };
    EffectChainManagerParam managerParam = {};
    managerParam.maxExtraNum = MAX_EXTRA_NUM;
    managerParam.defaultSceneName = "SCENE_RECORD";
    managerParam.sceneTypeToChainNameMap = { { "SCENE_RECORD_&_ENHANCE_DEFAULT", "EFFECTCHAIN_NO_LIB" } };
    managerParam.effectDefaultProperty = { { "enhance_no_lib", "ON" } };

    manager_->InitAudioEnhanceChainManager(enhanceChains, managerParam, enhanceLibraryList_);
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    // enhance lib not found, CreateAllEnhanceModule fail
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), ERROR);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, InitAudioEnhanceChainManagerWithUnknownChainName, TestSize.Level1)
{
    std::vector<EffectChain> enhanceChains = { { "EFFECTCHAIN_RECORD", { "record_test" }, "label" } };
    EffectChainManagerParam managerParam = {};
    managerParam.maxExtraNum = MAX_EXTRA_NUM;
    managerParam.defaultSceneName = "SCENE_RECORD";
    // chain name not in enhanceChains, enhanceNames will be empty
    managerParam.sceneTypeToChainNameMap = { { "SCENE_RECORD_&_ENHANCE_DEFAULT", "EFFECTCHAIN_NOT_EXIST" } };

    manager_->InitAudioEnhanceChainManager(enhanceChains, managerParam, enhanceLibraryList_);
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), ERROR);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, CreateRecognitionSceneEnhanceChainSucc, TestSize.Level1)
{
    std::vector<EffectChain> enhanceChains = { { "EFFECTCHAIN_RECOGNITION", { "record_test" }, "label" } };
    EffectChainManagerParam managerParam = {};
    managerParam.maxExtraNum = MAX_EXTRA_NUM;
    managerParam.defaultSceneName = "SCENE_RECORD";
    managerParam.sceneTypeToChainNameMap = { { "SCENE_RECOGNITION_&_ENHANCE_DEFAULT", "EFFECTCHAIN_RECOGNITION" } };
    managerParam.effectDefaultProperty = { { "record_test", "NROFF" } };

    manager_->InitAudioEnhanceChainManager(enhanceChains, managerParam, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    // SCENE_RECOGNITION has no dedicated thread id, use default thread id
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECOGNITION, TEST_CAPTURER_ID);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, GetChainOutputDataFailWithInvalidBuffer, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    const uint32_t waitTime = 10;
    std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
    std::vector<int16_t> output(10);
    // null buffer
    EXPECT_EQ(manager_->GetChainOutputDataById(sceneKeyCode, nullptr, output.size() * sizeof(int16_t)), ERROR);
    // zero buffer size
    EXPECT_EQ(manager_->GetChainOutputDataById(sceneKeyCode, output.data(), 0), ERROR);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, ApplyEnhanceChainWithEmptyTransBuffer, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    // apply without any input data, both ready and not ready chain return success
    EnhanceTransBuffer transBuf = {};
    EXPECT_EQ(manager_->ApplyEnhanceChainById(sceneKeyCode, transBuf), SUCCESS);

    const uint32_t waitTime = 10;
    std::this_thread::sleep_for(std::chrono::milliseconds(waitTime));
    // full frame size buffer: byteLenPerFrame(20ms * 48 * 2) * micChannels(4)
    const uint32_t chlNum = 4;
    std::vector<int16_t> output(20 * 48 * chlNum);
    EXPECT_EQ(manager_->GetChainOutputDataById(sceneKeyCode, output.data(), output.size() * sizeof(int16_t)),
        SUCCESS);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, UpdateExtraSceneTypeWithEdgeValue, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    std::string mainKey = "device_status";
    // empty extraSceneType
    manager_->UpdateExtraSceneType(mainKey, "fold_state", "");
    // fold state out of range [FOLD_STATE_EXPAND, FOLD_STATE_MIDDLE]
    manager_->UpdateExtraSceneType(mainKey, "fold_state", "0");
    manager_->UpdateExtraSceneType(mainKey, "fold_state", "99");
    // valid fold state
    manager_->UpdateExtraSceneType(mainKey, "fold_state", "1");
    // power state out of range [0, 100]
    manager_->UpdateExtraSceneType(mainKey, "power_state", "101");
    // valid power state
    manager_->UpdateExtraSceneType(mainKey, "power_state", "0");
    manager_->UpdateExtraSceneType(mainKey, "power_state", "100");

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}

HWTEST_F(AudioEnhanceChainManagerUnitTest, SetInputDeviceSkipOtherCaptureChain, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    EXPECT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    EXPECT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);

    // new captureId has no chain, chain of TEST_CAPTURER_ID will be skipped
    const uint32_t newCaptureId = 0x900;
    EXPECT_EQ(manager_->SetInputDevice(newCaptureId, DEVICE_TYPE_USB_HEADSET), SUCCESS);

    EXPECT_EQ(manager_->ReleaseAudioEnhanceChainDynamic(sceneKeyCode), SUCCESS);
}
/**
 * @tc.name  : Test ApplyEnhanceChainById chain-missing log gate
 * @tc.type  : FUNC
 * @tc.number: ApplyEnhanceChainGateSequence_001
 * @tc.desc  : First missing call logs, repeats are suppressed, an existing chain
 *             prints recovery and re-arms the gate
 */
HWTEST_F(AudioEnhanceChainManagerUnitTest, ApplyEnhanceChainGateSequence_001, TestSize.Level1)
{
    auto sceneKeyCode = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID);
    manager_->InitAudioEnhanceChainManager(enhanceChains_, managerParam_, enhanceLibraryList_);
    ASSERT_EQ(manager_->SetInputDevice(TEST_CAPTURER_ID, DEVICE_TYPE_MIC), SUCCESS);
    ASSERT_EQ(manager_->CreateAudioEnhanceChainDynamic(sceneKeyCode, TEST_DEVICE_ATTR), SUCCESS);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    uint64_t missingKey = GetValidSceneKeyCode(SCENE_RECORD, TEST_CAPTURER_ID + 7);
    EnhanceTransBuffer transBuf = {};
    // missing chain: first-hit logs, repeats are suppressed by the gate
    EXPECT_EQ(manager_->ApplyEnhanceChainById(missingKey, transBuf), ERROR);
    EXPECT_EQ(manager_->ApplyEnhanceChainById(missingKey, transBuf), ERROR);
    EXPECT_EQ(manager_->GetChainOutputDataById(missingKey, nullptr, 0), ERROR);
    // existing chain: recovery line plus the normal passthrough path
    EXPECT_EQ(manager_->ApplyEnhanceChainById(sceneKeyCode, transBuf), SUCCESS);
    EXPECT_EQ(manager_->ApplyEnhanceChainById(sceneKeyCode, transBuf), SUCCESS);
    // missing again: gate re-armed
    EXPECT_EQ(manager_->ApplyEnhanceChainById(missingKey, transBuf), ERROR);
}

} // namespace AudioStandard
} // namespace OHOS