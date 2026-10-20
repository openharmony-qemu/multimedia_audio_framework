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

#include <gtest/gtest.h>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "audio_core_config_manager.h"
#include "audio_definition_adapter_info.h"
#include "audio_device_descriptor.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_stream_descriptor.h"
#include "audio_utils.h"

#include "aiut_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
const char *TEST_MODULE_NAME = "test_module";
const char *TEST_GROUP_NAME = "group1";
const char *TEST_INTERRUPT_GROUP = "interrupt1";
const uint64_t TEST_LATENCY = 100;
const uint32_t TEST_SINK_LATENCY = 200;

// Local test constants mirroring file-scope constants in audio_core_config_manager.cpp
const int32_t TEST_DEFAULT_MAX_OUTPUT_NORMAL_INSTANCES = 128;
const int32_t TEST_DEFAULT_MAX_INPUT_NORMAL_INSTANCES = 16;
const int32_t TEST_DEFAULT_MAX_FAST_NORMAL_INSTANCES = 6;
} // namespace

class AudioCoreConfigManagerAiutTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp(void) {}
    void TearDown(void) {}
};

/**
 * @tc.name: AudioCoreConfigManager_SetEcEnableState_001
 * @tc.number: AudioCoreConfigManager_SetEcEnableState_001
 * @tc.desc: Test SetEcEnableState sets ecEnableState_ correctly
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, AudioCoreConfigManager_SetEcEnableState_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.SetEcEnableState(true);
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.ecEnableState_, "ecEnableState_ should be true");
    configManager.SetEcEnableState(false);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.ecEnableState_, "ecEnableState_ should be false");
}

/**
 * @tc.name: AudioCoreConfigManager_OnXmlParsingCompleted_Empty_001
 * @tc.number: AudioCoreConfigManager_OnXmlParsingCompleted_Empty_001
 * @tc.desc: Test OnAudioPolicyConfigXmlParsingCompleted with empty adapterInfoMap
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnXmlParsingCompleted_Empty_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.isAdapterInfoMap_.store(false);
    configManager.GetAudioPolicyConfigData().adapterInfoMap.clear();
    configManager.OnAudioPolicyConfigXmlParsingCompleted();
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.isAdapterInfoMap_.load(),
        "isAdapterInfoMap_ should remain false for empty map");
}

/**
 * @tc.name: AudioCoreConfigManager_OnXmlParsingCompleted_001
 * @tc.number: AudioCoreConfigManager_OnXmlParsingCompleted_001
 * @tc.desc: Test OnXmlParsingCompleted with empty data
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnXmlParsingCompleted_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> emptyData;
    configManager.OnXmlParsingCompleted(emptyData);
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.deviceClassInfo_.empty(),
        "deviceClassInfo_ should remain empty for empty data");
}

/**
 * @tc.name: AudioCoreConfigManager_OnXmlParsingCompleted_002
 * @tc.number: AudioCoreConfigManager_OnXmlParsingCompleted_002
 * @tc.desc: Test OnXmlParsingCompleted with non-empty data
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnXmlParsingCompleted_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> xmlData;
    AudioModuleInfo moduleInfo;
    moduleInfo.name = TEST_MODULE_NAME;
    xmlData[ClassType::TYPE_PRIMARY] = {moduleInfo};
    configManager.OnXmlParsingCompleted(xmlData);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.deviceClassInfo_.empty(),
        "deviceClassInfo_ should not be empty for non-empty data");
}

/**
 * @tc.name: AudioCoreConfigManager_OnAudioLatencyParsed_001
 * @tc.number: AudioCoreConfigManager_OnAudioLatencyParsed_001
 * @tc.desc: Test OnAudioLatencyParsed sets audioLatencyInMsec_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnAudioLatencyParsed_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.OnAudioLatencyParsed(TEST_LATENCY);
    AIUT_EXPECT_EQ_INFO_LOG(configManager.audioLatencyInMsec_, TEST_LATENCY,
        "audioLatencyInMsec_ should be 100");
}

/**
 * @tc.name: AudioCoreConfigManager_OnFastFormatParsed_001
 * @tc.number: AudioCoreConfigManager_OnFastFormatParsed_001
 * @tc.desc: Test OnFastFormatParsed sets fastFormat_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnFastFormatParsed_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioSampleFormat format = SAMPLE_S32LE;
    configManager.OnFastFormatParsed(format);
    AIUT_EXPECT_EQ_INFO_LOG(configManager.fastFormat_, format, "fastFormat_ should be SAMPLE_S32LE");
}

/**
 * @tc.name: AudioCoreConfigManager_GetFastFormat_001
 * @tc.number: AudioCoreConfigManager_GetFastFormat_001
 * @tc.desc: Test GetFastFormat returns fastFormat_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetFastFormat_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.fastFormat_ = SAMPLE_F32LE;
    AIUT_EXPECT_EQ_INFO_LOG(configManager.GetFastFormat(), SAMPLE_F32LE, "should return SAMPLE_F32LE");
}

/**
 * @tc.name: AudioCoreConfigManager_OnSinkLatencyParsed_001
 * @tc.number: AudioCoreConfigManager_OnSinkLatencyParsed_001
 * @tc.desc: Test OnSinkLatencyParsed sets sinkLatencyInMsec_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnSinkLatencyParsed_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.OnSinkLatencyParsed(TEST_SINK_LATENCY);
    AIUT_EXPECT_EQ_INFO_LOG(configManager.sinkLatencyInMsec_, TEST_SINK_LATENCY,
        "sinkLatencyInMsec_ should be 200");
}

/**
 * @tc.name: AudioCoreConfigManager_OnVolumeGroupParsed_001
 * @tc.number: AudioCoreConfigManager_OnVolumeGroupParsed_001
 * @tc.desc: Test OnVolumeGroupParsed with empty data
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnVolumeGroupParsed_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    std::unordered_map<std::string, std::string> emptyData;
    configManager.volumeGroupData_ = {{"key", "val"}};
    configManager.OnVolumeGroupParsed(emptyData);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.volumeGroupData_.empty(),
        "volumeGroupData_ should remain unchanged for empty input");
}

/**
 * @tc.name: AudioCoreConfigManager_OnVolumeGroupParsed_002
 * @tc.number: AudioCoreConfigManager_OnVolumeGroupParsed_002
 * @tc.desc: Test OnVolumeGroupParsed with non-empty data
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnVolumeGroupParsed_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    std::unordered_map<std::string, std::string> groupData = {{"speaker", TEST_GROUP_NAME}};
    configManager.OnVolumeGroupParsed(groupData);
    AIUT_EXPECT_EQ_INFO_LOG(configManager.volumeGroupData_["speaker"], TEST_GROUP_NAME,
        "volumeGroupData_ should contain speaker->group1");
}

/**
 * @tc.name: AudioCoreConfigManager_OnInterruptGroupParsed_001
 * @tc.number: AudioCoreConfigManager_OnInterruptGroupParsed_001
 * @tc.desc: Test OnInterruptGroupParsed with empty data
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnInterruptGroupParsed_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    std::unordered_map<std::string, std::string> emptyData;
    configManager.interruptGroupData_ = {{"key", "val"}};
    configManager.OnInterruptGroupParsed(emptyData);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.interruptGroupData_.empty(),
        "interruptGroupData_ should remain unchanged for empty input");
}

/**
 * @tc.name: AudioCoreConfigManager_OnInterruptGroupParsed_002
 * @tc.number: AudioCoreConfigManager_OnInterruptGroupParsed_002
 * @tc.desc: Test OnInterruptGroupParsed with non-empty data
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnInterruptGroupParsed_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    std::unordered_map<std::string, std::string> groupData = {{"speaker", TEST_INTERRUPT_GROUP}};
    configManager.OnInterruptGroupParsed(groupData);
    AIUT_EXPECT_EQ_INFO_LOG(configManager.interruptGroupData_["speaker"], TEST_INTERRUPT_GROUP,
        "interruptGroupData_ should contain speaker->interrupt1");
}

/**
 * @tc.name: AudioCoreConfigManager_OnUpdateRouteSupport_001
 * @tc.number: AudioCoreConfigManager_OnUpdateRouteSupport_001
 * @tc.desc: Test OnUpdateRouteSupport sets isUpdateRouteSupported_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnUpdateRouteSupport_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.OnUpdateRouteSupport(false);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.isUpdateRouteSupported_, "should be false");
    configManager.OnUpdateRouteSupport(true);
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.isUpdateRouteSupported_, "should be true");
}

/**
 * @tc.name: AudioCoreConfigManager_OnGlobalConfigsParsed_001
 * @tc.number: AudioCoreConfigManager_OnGlobalConfigsParsed_001
 * @tc.desc: Test OnGlobalConfigsParsed sets globalConfigs_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnGlobalConfigsParsed_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    PolicyGlobalConfigs configs;
    PolicyConfigInfo commonConfig;
    commonConfig.name_ = "test";
    commonConfig.value_ = "val";
    configs.commonConfigs_.push_back(commonConfig);
    configManager.OnGlobalConfigsParsed(configs);
    AIUT_EXPECT_EQ_INFO_LOG(configManager.globalConfigs_.commonConfigs_.size(), 1,
        "globalConfigs_ should have 1 common config");
}

/**
 * @tc.name: AudioCoreConfigManager_OnVoipConfigParsed_001
 * @tc.number: AudioCoreConfigManager_OnVoipConfigParsed_001
 * @tc.desc: Test OnVoipConfigParsed sets enableFastVoip_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnVoipConfigParsed_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.OnVoipConfigParsed(true);
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.enableFastVoip_, "enableFastVoip_ should be true");
    configManager.OnVoipConfigParsed(false);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.enableFastVoip_, "enableFastVoip_ should be false");
}

/**
 * @tc.name: AudioCoreConfigManager_OnMultSupMode_001
 * @tc.number: AudioCoreConfigManager_OnMultSupMode_001
 * @tc.desc: Test OnMultSupMode sets supportMultSupMode_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnMultSupMode_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.OnMultSupMode(true);
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.supportMultSupMode_, "supportMultSupMode_ should be true");
    configManager.OnMultSupMode(false);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.supportMultSupMode_, "supportMultSupMode_ should be false");
}

/**
 * @tc.name: AudioCoreConfigManager_OnUpdateEac3Support_001
 * @tc.number: AudioCoreConfigManager_OnUpdateEac3Support_001
 * @tc.desc: Test OnUpdateEac3Support sets isSupportEac3_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnUpdateEac3Support_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.OnUpdateEac3Support(true);
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.isSupportEac3_, "isSupportEac3_ should be true");
    configManager.OnUpdateEac3Support(false);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.isSupportEac3_, "isSupportEac3_ should be false");
}

/**
 * @tc.name: AudioCoreConfigManager_OnSupportUltraFast_001
 * @tc.number: AudioCoreConfigManager_OnSupportUltraFast_001
 * @tc.desc: Test OnSupportUltraFast sets supportUltraFast_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnSupportUltraFast_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.OnSupportUltraFast(true);
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.supportUltraFast_, "supportUltraFast_ should be true");
    configManager.OnSupportUltraFast(false);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.supportUltraFast_, "supportUltraFast_ should be false");
}

/**
 * @tc.name: AudioCoreConfigManager_OnIsSupportIndependentPipe_001
 * @tc.number: AudioCoreConfigManager_OnIsSupportIndependentPipe_001
 * @tc.desc: Test OnIsSupportIndependentPipe sets isSupportIndependentPipe_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, OnIsSupportIndependentPipe_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.OnIsSupportIndependentPipe(true);
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.IsSupportIndependentPipe(),
        "IsSupportIndependentPipe should return true");
    configManager.OnIsSupportIndependentPipe(false);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsSupportIndependentPipe(),
        "IsSupportIndependentPipe should return false");
}

/**
 * @tc.name: AudioCoreConfigManager_SetNormalVoipFlag_001
 * @tc.number: AudioCoreConfigManager_SetNormalVoipFlag_001
 * @tc.desc: Test SetNormalVoipFlag and GetNormalVoipFlag
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, SetNormalVoipFlag_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.SetNormalVoipFlag(true);
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.GetNormalVoipFlag(), "should return true");
    configManager.SetNormalVoipFlag(false);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.GetNormalVoipFlag(), "should return false");
}

/**
 * @tc.name: AudioCoreConfigManager_GetModuleListByType_001
 * @tc.number: AudioCoreConfigManager_GetModuleListByType_001
 * @tc.desc: Test GetModuleListByType when type is found
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetModuleListByType_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioModuleInfo moduleInfo;
    moduleInfo.name = TEST_MODULE_NAME;
    configManager.deviceClassInfo_[ClassType::TYPE_PRIMARY] = {moduleInfo};
    std::list<AudioModuleInfo> moduleList;
    bool result = configManager.GetModuleListByType(ClassType::TYPE_PRIMARY, moduleList);
    AIUT_EXPECT_TRUE_INFO_LOG(result, "should return true for existing type");
    AIUT_EXPECT_FALSE_INFO_LOG(moduleList.empty(), "moduleList should not be empty");
}

/**
 * @tc.name: AudioCoreConfigManager_GetModuleListByType_002
 * @tc.number: AudioCoreConfigManager_GetModuleListByType_002
 * @tc.desc: Test GetModuleListByType when type is not found
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetModuleListByType_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.deviceClassInfo_.clear();
    std::list<AudioModuleInfo> moduleList;
    bool result = configManager.GetModuleListByType(ClassType::TYPE_PRIMARY, moduleList);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "should return false for non-existing type");
}

/**
 * @tc.name: AudioCoreConfigManager_UpdateDynamicCapturerConfig_001
 * @tc.number: AudioCoreConfigManager_UpdateDynamicCapturerConfig_001
 * @tc.desc: Test UpdateDynamicCapturerConfig stores config
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, UpdateDynamicCapturerConfig_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "dynamic_test";
    configManager.UpdateDynamicCapturerConfig(ClassType::TYPE_USB, moduleInfo);
    AIUT_EXPECT_EQ_INFO_LOG(configManager.dynamicCapturerConfig_[ClassType::TYPE_USB].name,
        "dynamic_test", "dynamicCapturerConfig should contain TYPE_USB entry");
}

/**
 * @tc.name: AudioCoreConfigManager_GetDeviceClassInfo_001
 * @tc.number: AudioCoreConfigManager_GetDeviceClassInfo_001
 * @tc.desc: Test GetDeviceClassInfo returns deviceClassInfo_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetDeviceClassInfo_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioModuleInfo moduleInfo;
    moduleInfo.name = "test_info";
    configManager.deviceClassInfo_[ClassType::TYPE_PRIMARY] = {moduleInfo};
    std::unordered_map<ClassType, std::list<AudioModuleInfo>> deviceClassInfo;
    configManager.GetDeviceClassInfo(deviceClassInfo);
    AIUT_EXPECT_FALSE_INFO_LOG(deviceClassInfo.empty(), "deviceClassInfo should not be empty");
}

/**
 * @tc.name: AudioCoreConfigManager_GetGroupName_001
 * @tc.number: AudioCoreConfigManager_GetGroupName_001
 * @tc.desc: Test GetGroupName with VOLUME_TYPE and found device
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetGroupName_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.volumeGroupData_ = {{"speaker", TEST_GROUP_NAME}};
    std::string result = configManager.GetGroupName("speaker", VOLUME_TYPE);
    AIUT_EXPECT_EQ_INFO_LOG(result, TEST_GROUP_NAME, "GetGroupName should return group1");
}

/**
 * @tc.name: AudioCoreConfigManager_GetGroupName_002
 * @tc.number: AudioCoreConfigManager_GetGroupName_002
 * @tc.desc: Test GetGroupName with VOLUME_TYPE and not found device
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetGroupName_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.volumeGroupData_.clear();
    std::string result = configManager.GetGroupName("unknown", VOLUME_TYPE);
    AIUT_EXPECT_EQ_INFO_LOG(result, GROUP_NAME_NONE, "GetGroupName should return GROUP_NAME_NONE");
}

/**
 * @tc.name: AudioCoreConfigManager_GetGroupName_003
 * @tc.number: AudioCoreConfigManager_GetGroupName_003
 * @tc.desc: Test GetGroupName with non-VOLUME_TYPE and found device
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetGroupName_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.interruptGroupData_ = {{"speaker", TEST_INTERRUPT_GROUP}};
    std::string result = configManager.GetGroupName("speaker", INTERRUPT_TYPE);
    AIUT_EXPECT_EQ_INFO_LOG(result, TEST_INTERRUPT_GROUP, "GetGroupName should return interrupt1");
}

/**
 * @tc.name: AudioCoreConfigManager_GetGroupName_004
 * @tc.number: AudioCoreConfigManager_GetGroupName_004
 * @tc.desc: Test GetGroupName with non-VOLUME_TYPE and not found device
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetGroupName_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.interruptGroupData_.clear();
    std::string result = configManager.GetGroupName("unknown", INTERRUPT_TYPE);
    AIUT_EXPECT_EQ_INFO_LOG(result, GROUP_NAME_NONE, "GetGroupName should return GROUP_NAME_NONE");
}

/**
 * @tc.name: AudioCoreConfigManager_GetMaxRendererInstances_001
 * @tc.number: AudioCoreConfigManager_GetMaxRendererInstances_001
 * @tc.desc: Test GetMaxRendererInstances with matching config and valid value
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetMaxRendererInstances_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    PolicyConfigInfo config;
    config.name_ = "maxRenderers";
    config.value_ = "64";
    configManager.globalConfigs_.commonConfigs_ = {config};
    int32_t result = configManager.GetMaxRendererInstances();
    AIUT_EXPECT_EQ_INFO_LOG(result, 64, "GetMaxRendererInstances should return 64");
}

/**
 * @tc.name: AudioCoreConfigManager_GetMaxRendererInstances_002
 * @tc.number: AudioCoreConfigManager_GetMaxRendererInstances_002
 * @tc.desc: Test GetMaxRendererInstances with no matching config
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetMaxRendererInstances_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    PolicyConfigInfo config;
    config.name_ = "other";
    config.value_ = "64";
    configManager.globalConfigs_.commonConfigs_ = {config};
    int32_t result = configManager.GetMaxRendererInstances();
    AIUT_EXPECT_EQ_INFO_LOG(result, TEST_DEFAULT_MAX_OUTPUT_NORMAL_INSTANCES,
        "GetMaxRendererInstances should return default");
}

/**
 * @tc.name: AudioCoreConfigManager_GetMaxRendererInstances_003
 * @tc.number: AudioCoreConfigManager_GetMaxRendererInstances_003
 * @tc.desc: Test GetMaxRendererInstances with invalid value string
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetMaxRendererInstances_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    PolicyConfigInfo config;
    config.name_ = "maxRenderers";
    config.value_ = "invalid";
    configManager.globalConfigs_.commonConfigs_ = {config};
    int32_t result = configManager.GetMaxRendererInstances();
    AIUT_EXPECT_EQ_INFO_LOG(result, TEST_DEFAULT_MAX_OUTPUT_NORMAL_INSTANCES,
        "GetMaxRendererInstances should return default for invalid value");
}

/**
 * @tc.name: AudioCoreConfigManager_GetMaxCapturersInstances_001
 * @tc.number: AudioCoreConfigManager_GetMaxCapturersInstances_001
 * @tc.desc: Test GetMaxCapturersInstances with matching config and valid value
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetMaxCapturersInstances_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    PolicyConfigInfo config;
    config.name_ = "maxCapturers";
    config.value_ = "8";
    configManager.globalConfigs_.commonConfigs_ = {config};
    int32_t result = configManager.GetMaxCapturersInstances();
    AIUT_EXPECT_EQ_INFO_LOG(result, 8, "GetMaxCapturersInstances should return 8");
}

/**
 * @tc.name: AudioCoreConfigManager_GetMaxCapturersInstances_002
 * @tc.number: AudioCoreConfigManager_GetMaxCapturersInstances_002
 * @tc.desc: Test GetMaxCapturersInstances with no matching config
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetMaxCapturersInstances_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.globalConfigs_.commonConfigs_.clear();
    int32_t result = configManager.GetMaxCapturersInstances();
    AIUT_EXPECT_EQ_INFO_LOG(result, TEST_DEFAULT_MAX_INPUT_NORMAL_INSTANCES,
        "GetMaxCapturersInstances should return default");
}

/**
 * @tc.name: AudioCoreConfigManager_GetMaxFastRenderersInstances_001
 * @tc.number: AudioCoreConfigManager_GetMaxFastRenderersInstances_001
 * @tc.desc: Test GetMaxFastRenderersInstances with matching config and valid value
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetMaxFastRenderersInstances_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    PolicyConfigInfo config;
    config.name_ = "maxFastRenderers";
    config.value_ = "4";
    configManager.globalConfigs_.commonConfigs_ = {config};
    int32_t result = configManager.GetMaxFastRenderersInstances();
    AIUT_EXPECT_EQ_INFO_LOG(result, 4, "GetMaxFastRenderersInstances should return 4");
}

/**
 * @tc.name: AudioCoreConfigManager_GetMaxFastRenderersInstances_002
 * @tc.number: AudioCoreConfigManager_GetMaxFastRenderersInstances_002
 * @tc.desc: Test GetMaxFastRenderersInstances with no matching config
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetMaxFastRenderersInstances_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.globalConfigs_.commonConfigs_.clear();
    int32_t result = configManager.GetMaxFastRenderersInstances();
    AIUT_EXPECT_EQ_INFO_LOG(result, TEST_DEFAULT_MAX_FAST_NORMAL_INSTANCES,
        "GetMaxFastRenderersInstances should return default");
}

/**
 * @tc.name: AudioCoreConfigManager_IsSupportInnerCaptureOffload_001
 * @tc.number: AudioCoreConfigManager_IsSupportInnerCaptureOffload_001
 * @tc.desc: Test IsSupportInnerCaptureOffload when already has value
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, IsSupportInnerCaptureOffload_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.isSupportInnerCaptureOffload_ = true;
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.IsSupportInnerCaptureOffload(),
        "IsSupportInnerCaptureOffload should return true when already set");
    configManager.isSupportInnerCaptureOffload_ = false;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsSupportInnerCaptureOffload(),
        "IsSupportInnerCaptureOffload should return false when already set");
}

/**
 * @tc.name: AudioCoreConfigManager_IsSupportInnerCaptureOffload_002
 * @tc.number: AudioCoreConfigManager_IsSupportInnerCaptureOffload_002
 * @tc.desc: Test IsSupportInnerCaptureOffload when no value and config has true
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, IsSupportInnerCaptureOffload_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.isSupportInnerCaptureOffload_ = std::nullopt;
    PolicyConfigInfo config;
    config.name_ = "offloadInnerCaptureSupport";
    config.value_ = "true";
    configManager.globalConfigs_.commonConfigs_ = {config};
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.IsSupportInnerCaptureOffload(),
        "IsSupportInnerCaptureOffload should return true for config true");
}

/**
 * @tc.name: AudioCoreConfigManager_IsSupportInnerCaptureOffload_003
 * @tc.number: AudioCoreConfigManager_IsSupportInnerCaptureOffload_003
 * @tc.desc: Test IsSupportInnerCaptureOffload when no value and config has false
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, IsSupportInnerCaptureOffload_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.isSupportInnerCaptureOffload_ = std::nullopt;
    PolicyConfigInfo config;
    config.name_ = "offloadInnerCaptureSupport";
    config.value_ = "false";
    configManager.globalConfigs_.commonConfigs_ = {config};
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsSupportInnerCaptureOffload(),
        "IsSupportInnerCaptureOffload should return false for config false");
}

/**
 * @tc.name: AudioCoreConfigManager_IsSupportInnerCaptureOffload_004
 * @tc.number: AudioCoreConfigManager_IsSupportInnerCaptureOffload_004
 * @tc.desc: Test IsSupportInnerCaptureOffload when no value and no matching config
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, IsSupportInnerCaptureOffload_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.isSupportInnerCaptureOffload_ = std::nullopt;
    configManager.globalConfigs_.commonConfigs_.clear();
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsSupportInnerCaptureOffload(),
        "IsSupportInnerCaptureOffload should return false when no config");
}

/**
 * @tc.name: AudioCoreConfigManager_GetAudioLatencyFromXml_001
 * @tc.number: AudioCoreConfigManager_GetAudioLatencyFromXml_001
 * @tc.desc: Test GetAudioLatencyFromXml returns audioLatencyInMsec_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetAudioLatencyFromXml_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.audioLatencyInMsec_ = 75;
    AIUT_EXPECT_EQ_INFO_LOG(configManager.GetAudioLatencyFromXml(), 75,
        "GetAudioLatencyFromXml should return 75");
}

/**
 * @tc.name: AudioCoreConfigManager_GetSinkLatencyFromXml_001
 * @tc.number: AudioCoreConfigManager_GetSinkLatencyFromXml_001
 * @tc.desc: Test GetSinkLatencyFromXml returns sinkLatencyInMsec_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetSinkLatencyFromXml_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.sinkLatencyInMsec_ = 300;
    AIUT_EXPECT_EQ_INFO_LOG(configManager.GetSinkLatencyFromXml(), 300,
        "GetSinkLatencyFromXml should return 300");
}

/**
 * @tc.name: AudioCoreConfigManager_GetVolumeGroupData_001
 * @tc.number: AudioCoreConfigManager_GetVolumeGroupData_001
 * @tc.desc: Test GetVolumeGroupData returns volumeGroupData_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetVolumeGroupData_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.volumeGroupData_ = {{"speaker", TEST_GROUP_NAME}};
    std::unordered_map<std::string, std::string> data;
    configManager.GetVolumeGroupData(data);
    AIUT_EXPECT_EQ_INFO_LOG(data["speaker"], TEST_GROUP_NAME,
        "GetVolumeGroupData should return speaker->group1");
}

/**
 * @tc.name: AudioCoreConfigManager_GetInterruptGroupData_001
 * @tc.number: AudioCoreConfigManager_GetInterruptGroupData_001
 * @tc.desc: Test GetInterruptGroupData returns interruptGroupData_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetInterruptGroupData_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.interruptGroupData_ = {{"speaker", TEST_INTERRUPT_GROUP}};
    std::unordered_map<std::string, std::string> data;
    configManager.GetInterruptGroupData(data);
    AIUT_EXPECT_EQ_INFO_LOG(data["speaker"], TEST_INTERRUPT_GROUP,
        "GetInterruptGroupData should return speaker->interrupt1");
}

/**
 * @tc.name: AudioCoreConfigManager_GetGlobalConfigs_001
 * @tc.number: AudioCoreConfigManager_GetGlobalConfigs_001
 * @tc.desc: Test GetGlobalConfigs returns globalConfigs_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetGlobalConfigs_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    PolicyConfigInfo config;
    config.name_ = "test_global";
    configManager.globalConfigs_.commonConfigs_ = {config};
    PolicyGlobalConfigs configs;
    configManager.GetGlobalConfigs(configs);
    AIUT_EXPECT_EQ_INFO_LOG(configs.commonConfigs_.size(), 1,
        "GetGlobalConfigs should return 1 common config");
}

/**
 * @tc.name: AudioCoreConfigManager_GetVoipConfig_001
 * @tc.number: AudioCoreConfigManager_GetVoipConfig_001
 * @tc.desc: Test GetVoipConfig returns enableFastVoip_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetVoipConfig_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.enableFastVoip_ = true;
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.GetVoipConfig(), "GetVoipConfig should return true");
    configManager.enableFastVoip_ = false;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.GetVoipConfig(), "GetVoipConfig should return false");
}

/**
 * @tc.name: AudioCoreConfigManager_GetUpdateRouteSupport_001
 * @tc.number: AudioCoreConfigManager_GetUpdateRouteSupport_001
 * @tc.desc: Test GetUpdateRouteSupport returns isUpdateRouteSupported_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetUpdateRouteSupport_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.isUpdateRouteSupported_ = true;
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.GetUpdateRouteSupport(),
        "GetUpdateRouteSupport should return true");
    configManager.isUpdateRouteSupported_ = false;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.GetUpdateRouteSupport(),
        "GetUpdateRouteSupport should return false");
}

/**
 * @tc.name: AudioCoreConfigManager_GetAdapterInfoFlag_001
 * @tc.number: AudioCoreConfigManager_GetAdapterInfoFlag_001
 * @tc.desc: Test GetAdapterInfoFlag returns isAdapterInfoMap_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetAdapterInfoFlag_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.isAdapterInfoMap_.store(true);
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.GetAdapterInfoFlag(),
        "GetAdapterInfoFlag should return true");
    configManager.isAdapterInfoMap_.store(false);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.GetAdapterInfoFlag(),
        "GetAdapterInfoFlag should return false");
}

/**
 * @tc.name: AudioCoreConfigManager_GetHasEarpiece_001
 * @tc.number: AudioCoreConfigManager_GetHasEarpiece_001
 * @tc.desc: Test GetHasEarpiece returns hasEarpiece_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetHasEarpiece_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.hasEarpiece_ = true;
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.GetHasEarpiece(), "GetHasEarpiece should return true");
    configManager.hasEarpiece_ = false;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.GetHasEarpiece(), "GetHasEarpiece should return false");
}

/**
 * @tc.name: AudioCoreConfigManager_GetUltraFastFlag_001
 * @tc.number: AudioCoreConfigManager_GetUltraFastFlag_001
 * @tc.desc: Test GetUltraFastFlag returns supportUltraFast_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetUltraFastFlag_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.supportUltraFast_ = true;
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.GetUltraFastFlag(),
        "GetUltraFastFlag should return true");
    configManager.supportUltraFast_ = false;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.GetUltraFastFlag(),
        "GetUltraFastFlag should return false");
}

/**
 * @tc.name: AudioCoreConfigManager_GetSupportMultSupMode_001
 * @tc.number: AudioCoreConfigManager_GetSupportMultSupMode_001
 * @tc.desc: Test GetSupportMultSupMode returns supportMultSupMode_
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetSupportMultSupMode_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.supportMultSupMode_ = true;
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.GetSupportMultSupMode(),
        "GetSupportMultSupMode should return true");
    configManager.supportMultSupMode_ = false;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.GetSupportMultSupMode(),
        "GetSupportMultSupMode should return false");
}

/**
 * @tc.name: AudioCoreConfigManager_GetTargetSourceType_001
 * @tc.number: AudioCoreConfigManager_GetTargetSourceType_001
 * @tc.desc: Test GetTargetSourceTypeAndMatchingFlag for VOICE_RECOGNITION
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetTargetSourceType_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    bool useMatchingPropInfo = false;
    configManager.GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_VOICE_RECOGNITION, useMatchingPropInfo);
    AIUT_EXPECT_TRUE_INFO_LOG(useMatchingPropInfo,
        "useMatchingPropInfo should be true for VOICE_RECOGNITION");
}

/**
 * @tc.name: AudioCoreConfigManager_GetTargetSourceType_002
 * @tc.number: AudioCoreConfigManager_GetTargetSourceType_002
 * @tc.desc: Test GetTargetSourceTypeAndMatchingFlag for VOICE_COMMUNICATION with ec disabled
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetTargetSourceType_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.ecEnableState_ = false;
    bool useMatchingPropInfo = false;
    configManager.GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_VOICE_COMMUNICATION, useMatchingPropInfo);
    AIUT_EXPECT_TRUE_INFO_LOG(useMatchingPropInfo,
        "useMatchingPropInfo should be true for VOICE_COMMUNICATION with ec disabled");
}

/**
 * @tc.name: AudioCoreConfigManager_GetTargetSourceType_003
 * @tc.number: AudioCoreConfigManager_GetTargetSourceType_003
 * @tc.desc: Test GetTargetSourceTypeAndMatchingFlag for VOICE_COMMUNICATION with ec enabled
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetTargetSourceType_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.ecEnableState_ = true;
    bool useMatchingPropInfo = true;
    configManager.GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_VOICE_COMMUNICATION, useMatchingPropInfo);
    AIUT_EXPECT_FALSE_INFO_LOG(useMatchingPropInfo,
        "useMatchingPropInfo should be false for VOICE_COMMUNICATION with ec enabled");
}

/**
 * @tc.name: AudioCoreConfigManager_GetTargetSourceType_004
 * @tc.number: AudioCoreConfigManager_GetTargetSourceType_004
 * @tc.desc: Test GetTargetSourceTypeAndMatchingFlag for VOICE_TRANSCRIPTION with ec disabled
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetTargetSourceType_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.ecEnableState_ = false;
    bool useMatchingPropInfo = false;
    configManager.GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_VOICE_TRANSCRIPTION, useMatchingPropInfo);
    AIUT_EXPECT_TRUE_INFO_LOG(useMatchingPropInfo,
        "useMatchingPropInfo should be true for VOICE_TRANSCRIPTION with ec disabled");
}

/**
 * @tc.name: AudioCoreConfigManager_GetTargetSourceType_005
 * @tc.number: AudioCoreConfigManager_GetTargetSourceType_005
 * @tc.desc: Test GetTargetSourceTypeAndMatchingFlag for UNPROCESSED with ec disabled
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetTargetSourceType_005, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.ecEnableState_ = false;
    bool useMatchingPropInfo = false;
    configManager.GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_UNPROCESSED, useMatchingPropInfo);
    AIUT_EXPECT_TRUE_INFO_LOG(useMatchingPropInfo,
        "useMatchingPropInfo should be true for UNPROCESSED with ec disabled");
}

/**
 * @tc.name: AudioCoreConfigManager_GetTargetSourceType_006
 * @tc.number: AudioCoreConfigManager_GetTargetSourceType_006
 * @tc.desc: Test GetTargetSourceTypeAndMatchingFlag for VOICE_CALL (no change)
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetTargetSourceType_006, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    bool useMatchingPropInfo = false;
    configManager.GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_VOICE_CALL, useMatchingPropInfo);
    AIUT_EXPECT_FALSE_INFO_LOG(useMatchingPropInfo,
        "useMatchingPropInfo should remain false for VOICE_CALL");
}

/**
 * @tc.name: AudioCoreConfigManager_GetTargetSourceType_007
 * @tc.number: AudioCoreConfigManager_GetTargetSourceType_007
 * @tc.desc: Test GetTargetSourceTypeAndMatchingFlag for CAMCORDER (no change)
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetTargetSourceType_007, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    bool useMatchingPropInfo = false;
    configManager.GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_CAMCORDER, useMatchingPropInfo);
    AIUT_EXPECT_FALSE_INFO_LOG(useMatchingPropInfo,
        "useMatchingPropInfo should remain false for CAMCORDER");
}

/**
 * @tc.name: AudioCoreConfigManager_GetTargetSourceType_008
 * @tc.number: AudioCoreConfigManager_GetTargetSourceType_008
 * @tc.desc: Test GetTargetSourceTypeAndMatchingFlag for default case
 */
HWTEST_F(AudioCoreConfigManagerAiutTest, GetTargetSourceType_008, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    bool useMatchingPropInfo = false;
    configManager.GetTargetSourceTypeAndMatchingFlag(SOURCE_TYPE_INVALID, useMatchingPropInfo);
    AIUT_EXPECT_FALSE_INFO_LOG(useMatchingPropInfo,
        "useMatchingPropInfo should remain false for default case");
}

/** @tc.desc: Routing checks the real local device and independent input pipe, regardless of stream profiles. */
HWTEST_F(AudioCoreConfigManagerAiutTest, UltraFastRecordingCapability_001, TestSize.Level1)
{
    auto &manager = AudioCoreConfigManager::GetInstance();
    auto &config = AudioPolicyConfigData::GetInstance();
    auto savedAdapters = config.adapterInfoMap;
    auto savedDevices = config.deviceInfoMap;
    config.adapterInfoMap.clear();
    config.deviceInfoMap.clear();
    EXPECT_FALSE(manager.GetUltraFastFlag("primary", PIPE_ROLE_INPUT));
    auto adapter = std::make_shared<PolicyAdapterInfo>();
    adapter->adapterName = "primary";
    auto device = std::make_shared<AdapterDeviceInfo>();
    device->type_ = DEVICE_TYPE_USB_HEADSET;
    device->role_ = INPUT_DEVICE;
    device->adapterInfo_ = adapter;
    auto pipe = std::make_shared<AdapterPipeInfo>();
    pipe->name_ = "primary_ultra_fast_input";
    pipe->role_ = PIPE_ROLE_INPUT;
    pipe->supportFlags_ = AUDIO_INPUT_FLAG_ULTRA_FAST;
    auto prop = std::make_shared<PipeStreamPropInfo>();
    prop->sampleRate_ = SAMPLE_RATE_48000;
    prop->format_ = SAMPLE_S16LE;
    prop->channels_ = STEREO;
    prop->channelLayout_ = CH_LAYOUT_STEREO;
    prop->bufferSize_ = 480;
    prop->supportDeviceMap_[DEVICE_TYPE_USB_HEADSET] = device;
    pipe->streamPropInfos_ = {nullptr, prop};
    pipe->adapterInfo_ = adapter;
    device->supportPipeMap_[AUDIO_INPUT_FLAG_ULTRA_FAST] = pipe;
    adapter->pipeInfos = {nullptr, pipe};
    adapter->deviceInfos = {device};
    config.adapterInfoMap[AudioAdapterType::TYPE_PRIMARY] = adapter;
    config.deviceInfoMap[{DEVICE_TYPE_USB_HEADSET, INPUT_DEVICE}] = {device};
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    desc->deviceRole_ = INPUT_DEVICE;
    desc->networkId_ = LOCAL_NETWORK_ID;
    EXPECT_TRUE(manager.GetUltraFastFlag("primary", PIPE_ROLE_INPUT));
    EXPECT_FALSE(manager.GetUltraFastFlag("primary", PIPE_ROLE_OUTPUT));
    EXPECT_FALSE(manager.GetUltraFastFlag("remote", PIPE_ROLE_INPUT));
    EXPECT_TRUE(manager.IsUltraFastRecordingSupported({nullptr, desc}));
    EXPECT_FALSE(manager.IsUltraFastRecordingSupported({}));
    EXPECT_FALSE(manager.IsUltraFastRecordingSupported({nullptr}));
    prop->bufferSize_ = 960;
    EXPECT_TRUE(manager.IsUltraFastRecordingSupported({desc}));
    prop->sampleRate_ = SAMPLE_RATE_44100;
    prop->format_ = SAMPLE_S32LE;
    prop->channelLayout_ = CH_LAYOUT_MONO;
    prop->supportDeviceMap_.clear();
    EXPECT_TRUE(manager.IsUltraFastRecordingSupported({desc}));
    pipe->streamPropInfos_.clear();
    EXPECT_TRUE(manager.IsUltraFastRecordingSupported({desc}));
    EXPECT_TRUE(manager.GetUltraFastFlag("primary", PIPE_ROLE_INPUT));
    pipe->name_ = "primary_fast_input";
    EXPECT_FALSE(manager.GetUltraFastFlag("primary", PIPE_ROLE_INPUT));
    EXPECT_FALSE(manager.IsUltraFastRecordingSupported({desc}));
    pipe->name_ = "primary_ultra_fast_input";
    device->isSimulated_ = true;
    EXPECT_FALSE(manager.IsUltraFastRecordingSupported({desc}));
    device->isSimulated_ = false;
    desc->networkId_ = "remote";
    EXPECT_FALSE(manager.IsUltraFastRecordingSupported({desc}));
    desc->networkId_ = LOCAL_NETWORK_ID;
    desc->deviceRole_ = OUTPUT_DEVICE;
    EXPECT_FALSE(manager.IsUltraFastRecordingSupported({desc}));
    desc->deviceRole_ = INPUT_DEVICE;
    pipe->role_ = PIPE_ROLE_OUTPUT;
    EXPECT_FALSE(manager.IsUltraFastRecordingSupported({desc}));
    pipe->role_ = PIPE_ROLE_INPUT;
    auto wrongPipe = std::make_shared<AdapterPipeInfo>();
    wrongPipe->name_ = "primary_fast_input";
    wrongPipe->role_ = PIPE_ROLE_INPUT;
    device->supportPipeMap_[AUDIO_INPUT_FLAG_ULTRA_FAST] = wrongPipe;
    EXPECT_FALSE(manager.IsUltraFastRecordingSupported({desc}));
    device->supportPipeMap_[AUDIO_INPUT_FLAG_ULTRA_FAST] = nullptr;
    EXPECT_FALSE(manager.IsUltraFastRecordingSupported({desc}));
    device->supportPipeMap_[AUDIO_INPUT_FLAG_ULTRA_FAST] = pipe;
    device->adapterInfo_.reset();
    EXPECT_FALSE(manager.IsUltraFastRecordingSupported({desc}));
    device->adapterInfo_ = adapter;
    adapter->adapterName = "remote";
    EXPECT_FALSE(manager.IsUltraFastRecordingSupported({desc}));
    adapter->adapterName = "primary";
    config.adapterInfoMap.clear();
    EXPECT_FALSE(manager.IsUltraFastRecordingSupported({desc}));
    config.adapterInfoMap[AudioAdapterType::TYPE_PRIMARY] = adapter;
    device->supportPipeMap_.clear();
    EXPECT_FALSE(manager.IsUltraFastRecordingSupported({desc}));
    device->supportPipeMap_[AUDIO_INPUT_FLAG_ULTRA_FAST] = pipe;
    adapter->adapterName = "usb";
    pipe->name_ = "usb_ultra_fast_input";
    config.adapterInfoMap[AudioAdapterType::TYPE_USB] = adapter;
    EXPECT_TRUE(manager.IsUltraFastRecordingSupported({desc}));
    config.deviceInfoMap.clear();
    EXPECT_FALSE(manager.IsUltraFastRecordingSupported({desc}));
    config.adapterInfoMap = std::move(savedAdapters);
    config.deviceInfoMap = std::move(savedDevices);
}

/** @tc.desc: Device-only query applies the Primary caller allowlist without checking stream profiles. */
HWTEST_F(AudioCoreConfigManagerAiutTest, UltraFastRecordingDeviceQuery_001, TestSize.Level1)
{
    auto &manager = AudioCoreConfigManager::GetInstance();
    auto &config = AudioPolicyConfigData::GetInstance();
    auto savedAdapters = config.adapterInfoMap;
    auto savedDevices = config.deviceInfoMap;
    config.adapterInfoMap.clear();
    config.deviceInfoMap.clear();

    auto adapter = std::make_shared<PolicyAdapterInfo>();
    adapter->adapterName = "primary";
    auto pipe = std::make_shared<AdapterPipeInfo>();
    pipe->name_ = "primary_ultra_fast_input";
    pipe->role_ = PIPE_ROLE_INPUT;
    pipe->supportFlags_ = AUDIO_INPUT_FLAG_ULTRA_FAST;
    pipe->adapterInfo_ = adapter;
    adapter->pipeInfos = {nullptr, pipe};
    config.adapterInfoMap[AudioAdapterType::TYPE_PRIMARY] = adapter;

    auto device = std::make_shared<AdapterDeviceInfo>();
    device->role_ = INPUT_DEVICE;
    device->adapterInfo_ = adapter;
    device->supportPipeMap_[AUDIO_INPUT_FLAG_ULTRA_FAST] = pipe;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = INPUT_DEVICE;
    desc->networkId_ = LOCAL_NETWORK_ID;
    const std::vector<std::pair<std::string, DeviceType>> supportedSystemDevices = {
        {"Builtin_Mic_In", DEVICE_TYPE_MIC},
        {"Hifi_Headset_In", DEVICE_TYPE_USB_HEADSET}
    };
    const std::vector<std::pair<std::string, DeviceType>> unsupportedSystemDevices = {
        {"Wired_Headset_In", DEVICE_TYPE_WIRED_HEADSET},
        {"Bt_SCO_In", DEVICE_TYPE_BLUETOOTH_SCO},
        {"Nearlink_In", DEVICE_TYPE_NEARLINK_IN}
    };
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({}, true));
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({nullptr}, true));
    for (const auto &[name, type] : supportedSystemDevices) {
        device->name_ = name;
        device->type_ = type;
        desc->deviceType_ = type;
        config.deviceInfoMap.clear();
        config.deviceInfoMap[{type, INPUT_DEVICE}] = {device};
        // No profile is required for the device-only query.
        EXPECT_TRUE(manager.IsUltraFastRecordingDeviceSupported({nullptr, desc}, true)) << name;
        EXPECT_EQ(manager.IsUltraFastRecordingDeviceSupported({desc}, false), name == "Hifi_Headset_In");
    }
    for (const auto &[name, type] : unsupportedSystemDevices) {
        device->name_ = name;
        device->type_ = type;
        desc->deviceType_ = type;
        config.deviceInfoMap.clear();
        config.deviceInfoMap[{type, INPUT_DEVICE}] = {device};
        EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, true)) << name;
        EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, false)) << name;
    }

    device->name_ = "Unlisted_In";
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, true));
    device->name_ = "Hifi_Headset_In";
    device->type_ = DEVICE_TYPE_USB_HEADSET;
    desc->deviceType_ = DEVICE_TYPE_USB_HEADSET;
    config.deviceInfoMap.clear();
    config.deviceInfoMap[{DEVICE_TYPE_USB_HEADSET, INPUT_DEVICE}] = {device};
    EXPECT_TRUE(manager.IsUltraFastRecordingDeviceSupported({desc}, false));
    auto prop = std::make_shared<PipeStreamPropInfo>();
    prop->format_ = SAMPLE_S32LE;
    prop->sampleRate_ = SAMPLE_RATE_44100;
    prop->bufferSize_ = 0;
    pipe->streamPropInfos_ = {prop};
    EXPECT_TRUE(manager.IsUltraFastRecordingDeviceSupported({desc}, false));
    device->isSimulated_ = true;
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, true));
    device->isSimulated_ = false;
    desc->networkId_ = "remote";
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, true));
    desc->networkId_ = LOCAL_NETWORK_ID;
    desc->deviceRole_ = OUTPUT_DEVICE;
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, true));
    desc->deviceRole_ = INPUT_DEVICE;
    pipe->role_ = PIPE_ROLE_OUTPUT;
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, true));
    pipe->role_ = PIPE_ROLE_INPUT;
    pipe->name_ = "fast_input";
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, true));
    pipe->name_ = "primary_ultra_fast_input";
    device->supportPipeMap_.clear();
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, true));
    device->supportPipeMap_[AUDIO_INPUT_FLAG_ULTRA_FAST] = nullptr;
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, true));
    device->supportPipeMap_[AUDIO_INPUT_FLAG_ULTRA_FAST] = pipe;
    adapter->pipeInfos.clear();
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, true));
    adapter->pipeInfos = {pipe};
    device->adapterInfo_.reset();
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, true));
    device->adapterInfo_ = adapter;

    // USB ARM support uses the independent USB pipe for both application types.
    config.adapterInfoMap.clear();
    adapter->adapterName = "usb";
    config.adapterInfoMap[PolicyAdapterInfo::GetAdapterType("usb")] = adapter;
    pipe->name_ = "usb_ultra_fast_input";
    device->name_ = "Arm_Usb_Headset_In";
    device->type_ = DEVICE_TYPE_USB_ARM_HEADSET;
    desc->deviceType_ = DEVICE_TYPE_USB_ARM_HEADSET;
    config.deviceInfoMap.clear();
    config.deviceInfoMap[{DEVICE_TYPE_USB_ARM_HEADSET, INPUT_DEVICE}] = {device};
    EXPECT_TRUE(manager.IsUltraFastRecordingDeviceSupported({desc}, false));
    EXPECT_TRUE(manager.IsUltraFastRecordingDeviceSupported({desc}, true));
    pipe->name_ = "primary_ultra_fast_input";
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, true));
    pipe->name_ = "usb_ultra_fast_input";
    device->type_ = DEVICE_TYPE_MIC;
    desc->deviceType_ = DEVICE_TYPE_MIC;
    config.deviceInfoMap[{DEVICE_TYPE_MIC, INPUT_DEVICE}] = {device};
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, true));
    config.deviceInfoMap.clear();
    EXPECT_FALSE(manager.IsUltraFastRecordingDeviceSupported({desc}, true));
    config.adapterInfoMap = std::move(savedAdapters);
    config.deviceInfoMap = std::move(savedDevices);
}

} // namespace AudioStandard
} // namespace OHOS
