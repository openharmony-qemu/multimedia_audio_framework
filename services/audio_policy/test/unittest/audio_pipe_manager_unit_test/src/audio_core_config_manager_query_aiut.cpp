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
#include "audio_device_stream_info.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_stream_descriptor.h"
#include "audio_utils.h"

#include "aiut_log.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {
namespace {
const char *TEST_ADAPTER_NAME = "test_adapter";
const char *TEST_PIPE_NAME = "test_pipe";
} // namespace

class AudioCoreConfigManagerQueryAiutTest : public testing::Test {
public:
    static void SetUpTestCase(void) {}
    static void TearDownTestCase(void) {}
    void SetUp(void) {}
    void TearDown(void) {}
};

/**
 * @tc.name: AudioCoreConfigManager_SupportImplicitConv_001
 * @tc.number: AudioCoreConfigManager_SupportImplicitConv_001
 * @tc.desc: Test SupportImplicitConversion with AUDIO_OUTPUT_FLAG_NORMAL
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, SupportImplicitConv_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.SupportImplicitConversion(AUDIO_OUTPUT_FLAG_NORMAL),
        "should return true for AUDIO_OUTPUT_FLAG_NORMAL");
}

/**
 * @tc.name: AudioCoreConfigManager_SupportImplicitConv_002
 * @tc.number: AudioCoreConfigManager_SupportImplicitConv_002
 * @tc.desc: Test SupportImplicitConversion with AUDIO_INPUT_FLAG_NORMAL
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, SupportImplicitConv_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.SupportImplicitConversion(AUDIO_INPUT_FLAG_NORMAL),
        "should return true for AUDIO_INPUT_FLAG_NORMAL");
}

/**
 * @tc.name: AudioCoreConfigManager_SupportImplicitConv_003
 * @tc.number: AudioCoreConfigManager_SupportImplicitConv_003
 * @tc.desc: Test SupportImplicitConversion with AUDIO_OUTPUT_FLAG_LOWPOWER
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, SupportImplicitConv_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.SupportImplicitConversion(AUDIO_OUTPUT_FLAG_LOWPOWER),
        "should return true for AUDIO_OUTPUT_FLAG_LOWPOWER");
}

/**
 * @tc.name: AudioCoreConfigManager_SupportImplicitConv_004
 * @tc.number: AudioCoreConfigManager_SupportImplicitConv_004
 * @tc.desc: Test SupportImplicitConversion with AUDIO_INPUT_FLAG_WAKEUP
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, SupportImplicitConv_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.SupportImplicitConversion(AUDIO_INPUT_FLAG_WAKEUP),
        "should return true for AUDIO_INPUT_FLAG_WAKEUP");
}

/**
 * @tc.name: AudioCoreConfigManager_SupportImplicitConv_005
 * @tc.number: AudioCoreConfigManager_SupportImplicitConv_005
 * @tc.desc: Test SupportImplicitConversion with unsupported flag
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, SupportImplicitConv_005, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.SupportImplicitConversion(AUDIO_FLAG_NONE),
        "should return false for AUDIO_FLAG_NONE");
}

/**
 * @tc.name: AudioCoreConfigManager_SupportImplicitConv_006
 * @tc.number: AudioCoreConfigManager_SupportImplicitConv_006
 * @tc.desc: Test SupportImplicitConversion with AUDIO_OUTPUT_FLAG_MULTICHANNEL
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, SupportImplicitConv_006, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.SupportImplicitConversion(AUDIO_OUTPUT_FLAG_MULTICHANNEL),
        "should return true for AUDIO_OUTPUT_FLAG_MULTICHANNEL");
}

/**
 * @tc.name: AudioCoreConfigManager_SupportImplicitConv_007
 * @tc.number: AudioCoreConfigManager_SupportImplicitConv_007
 * @tc.desc: Test SupportImplicitConversion with AUDIO_OUTPUT_FLAG_VOICE_CALL_ASSISTANT
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, SupportImplicitConv_007, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.SupportImplicitConversion(AUDIO_OUTPUT_FLAG_VOICE_CALL_ASSISTANT),
        "should return true for AUDIO_OUTPUT_FLAG_VOICE_CALL_ASSISTANT");
}

/**
 * @tc.name: AudioCoreConfigManager_SupportImplicitConv_008
 * @tc.number: AudioCoreConfigManager_SupportImplicitConv_008
 * @tc.desc: Test SupportImplicitConversion with DIRECT+HD flags
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, SupportImplicitConv_008, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    uint32_t flag = AUDIO_OUTPUT_FLAG_DIRECT | AUDIO_OUTPUT_FLAG_HD;
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.SupportImplicitConversion(flag),
        "should return true for DIRECT|HD");
}

/**
 * @tc.name: AudioCoreConfigManager_IsStreamPropMatch_001
 * @tc.number: AudioCoreConfigManager_IsStreamPropMatch_001
 * @tc.desc: Test IsStreamPropMatch when properties match
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsStreamPropMatch_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.channels = STEREO;
    auto propInfo = std::make_shared<PipeStreamPropInfo>();
    propInfo->format_ = SAMPLE_S16LE;
    propInfo->sampleRate_ = SAMPLE_RATE_48000;
    propInfo->channels_ = STEREO;
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos = {propInfo};
    AIUT_EXPECT_TRUE_INFO_LOG(configManager.IsStreamPropMatch(streamInfo, infos),
        "should return true for matching props");
}

/**
 * @tc.name: AudioCoreConfigManager_IsStreamPropMatch_002
 * @tc.number: AudioCoreConfigManager_IsStreamPropMatch_002
 * @tc.desc: Test IsStreamPropMatch when properties do not match
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsStreamPropMatch_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    streamInfo.format = SAMPLE_S32LE;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.channels = STEREO;
    auto propInfo = std::make_shared<PipeStreamPropInfo>();
    propInfo->format_ = SAMPLE_S16LE;
    propInfo->sampleRate_ = SAMPLE_RATE_48000;
    propInfo->channels_ = STEREO;
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos = {propInfo};
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsStreamPropMatch(streamInfo, infos),
        "should return false for non-matching props");
}

/**
 * @tc.name: AudioCoreConfigManager_IsStreamPropMatch_003
 * @tc.number: AudioCoreConfigManager_IsStreamPropMatch_003
 * @tc.desc: Test IsStreamPropMatch with nullptr in list
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsStreamPropMatch_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    streamInfo.format = SAMPLE_S16LE;
    streamInfo.samplingRate = SAMPLE_RATE_48000;
    streamInfo.channels = STEREO;
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos = {nullptr};
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsStreamPropMatch(streamInfo, infos),
        "should return false for nullptr in list");
}

/**
 * @tc.name: AudioCoreConfigManager_GetAvailStreamProp_001
 * @tc.number: AudioCoreConfigManager_GetAvailStreamProp_001
 * @tc.desc: Test GetAvailableStreamPropInfoForDevice with empty list
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetAvailStreamProp_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    std::list<std::shared_ptr<PipeStreamPropInfo>> emptyList;
    auto result = configManager.GetAvailableStreamPropInfoForDevice(emptyList, DEVICE_TYPE_SPEAKER);
    AIUT_EXPECT_EQ_INFO_LOG(result, nullptr, "result should be nullptr for empty list");
}

/**
 * @tc.name: AudioCoreConfigManager_GetAvailStreamProp_002
 * @tc.number: AudioCoreConfigManager_GetAvailStreamProp_002
 * @tc.desc: Test GetAvailableStreamPropInfoForDevice with matching device
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetAvailStreamProp_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto propInfo = std::make_shared<PipeStreamPropInfo>();
    propInfo->supportDeviceMap_[DEVICE_TYPE_SPEAKER] = nullptr;
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos = {propInfo};
    auto result = configManager.GetAvailableStreamPropInfoForDevice(infos, DEVICE_TYPE_SPEAKER);
    AIUT_ASSERT_NE_INFO_LOG(result, nullptr, "result should not be nullptr for matching device");
}

/**
 * @tc.name: AudioCoreConfigManager_GetAvailStreamProp_003
 * @tc.number: AudioCoreConfigManager_GetAvailStreamProp_003
 * @tc.desc: Test GetAvailableStreamPropInfoForDevice with nullptr in list
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetAvailStreamProp_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto propInfo = std::make_shared<PipeStreamPropInfo>();
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos = {nullptr, propInfo};
    auto result = configManager.GetAvailableStreamPropInfoForDevice(infos, DEVICE_TYPE_SPEAKER);
    AIUT_EXPECT_EQ_INFO_LOG(result, nullptr, "result should be nullptr when front is nullptr");
}

/**
 * @tc.name: AudioCoreConfigManager_GetAvailStreamProp_004
 * @tc.number: AudioCoreConfigManager_GetAvailStreamProp_004
 * @tc.desc: Test GetAvailableStreamPropInfoForDevice with no matching device returns front
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetAvailStreamProp_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto propInfo = std::make_shared<PipeStreamPropInfo>();
    propInfo->format_ = SAMPLE_S16LE;
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos = {propInfo};
    auto result = configManager.GetAvailableStreamPropInfoForDevice(infos, DEVICE_TYPE_MIC);
    AIUT_ASSERT_NE_INFO_LOG(result, nullptr, "result should not be nullptr");
    AIUT_EXPECT_EQ_INFO_LOG(result->format_, SAMPLE_S16LE, "result should be front element");
}

/**
 * @tc.name: AudioCoreConfigManager_AddRequiredPipe_001
 * @tc.number: AudioCoreConfigManager_AddRequiredPipe_001
 * @tc.desc: Test AddRequiredPipe adds pipe info
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, AddRequiredPipe_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.requiredPipes_.clear();
    RequiredPipeInfo pipeInfo;
    pipeInfo.adapterName = TEST_ADAPTER_NAME;
    pipeInfo.routeFlag = AUDIO_OUTPUT_FLAG_NORMAL;
    configManager.AddRequiredPipe(pipeInfo);
    AIUT_EXPECT_EQ_INFO_LOG(configManager.requiredPipes_.size(), 1,
        "requiredPipes_ should have 1 entry");
}

/**
 * @tc.name: AudioCoreConfigManager_GetRequiredPipes_001
 * @tc.number: AudioCoreConfigManager_GetRequiredPipes_001
 * @tc.desc: Test GetRequiredPipes returns requiredPipes_
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetRequiredPipes_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.requiredPipes_.clear();
    RequiredPipeInfo pipeInfo;
    pipeInfo.adapterName = TEST_ADAPTER_NAME;
    pipeInfo.routeFlag = AUDIO_INPUT_FLAG_NORMAL;
    configManager.AddRequiredPipe(pipeInfo);
    auto pipes = configManager.GetRequiredPipes();
    AIUT_EXPECT_EQ_INFO_LOG(pipes.size(), 1, "GetRequiredPipes should return 1 pipe");
    AIUT_EXPECT_EQ_INFO_LOG(pipes[0].adapterName, TEST_ADAPTER_NAME,
        "pipe adapterName should be test_adapter");
}

/**
 * @tc.name: AudioCoreConfigManager_NeedUidCheck_001
 * @tc.number: AudioCoreConfigManager_NeedUidCheck_001
 * @tc.desc: Test NeedUidCheckForOffloadPipe with nullptr desc
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, NeedUidCheck_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    std::shared_ptr<AudioStreamDescriptor> desc = nullptr;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.NeedUidCheckForOffloadPipe(desc),
        "should return false for nullptr desc");
}

/**
 * @tc.name: AudioCoreConfigManager_NeedUidCheck_002
 * @tc.number: AudioCoreConfigManager_NeedUidCheck_002
 * @tc.desc: Test NeedUidCheckForOffloadPipe with empty newDeviceDescs
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, NeedUidCheck_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto desc = std::make_shared<AudioStreamDescriptor>();
    desc->newDeviceDescs_.clear();
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.NeedUidCheckForOffloadPipe(desc),
        "should return false for empty newDeviceDescs");
}

/**
 * @tc.name: AudioCoreConfigManager_NeedUidCheck_003
 * @tc.number: AudioCoreConfigManager_NeedUidCheck_003
 * @tc.desc: Test NeedUidCheckForOffloadPipe with nullptr front device desc
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, NeedUidCheck_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto desc = std::make_shared<AudioStreamDescriptor>();
    desc->newDeviceDescs_.push_back(nullptr);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.NeedUidCheckForOffloadPipe(desc),
        "should return false for nullptr front device");
}

/**
 * @tc.name: AudioCoreConfigManager_IsDeviceSimulated_001
 * @tc.number: AudioCoreConfigManager_IsDeviceSimulated_001
 * @tc.desc: Test IsDeviceSimulated with nullptr desc
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsDeviceSimulated_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = nullptr;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsDeviceSimulated(desc),
        "should return false for nullptr desc");
}

/**
 * @tc.name: AudioCoreConfigManager_IsDeviceSimulated_002
 * @tc.number: AudioCoreConfigManager_IsDeviceSimulated_002
 * @tc.desc: Test IsDeviceSimulated with remote cast device
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsDeviceSimulated_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsDeviceSimulated(desc),
        "should return false for remote cast");
}

/**
 * @tc.name: AudioCoreConfigManager_GetStreamPropInfoSize_001
 * @tc.number: AudioCoreConfigManager_GetStreamPropInfoSize_001
 * @tc.desc: Test GetStreamPropInfoSize returns 0 for non-existent adapter/pipe
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetStreamPropInfoSize_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    uint32_t result = configManager.GetStreamPropInfoSize("non_existent", "non_existent", LOCAL_NETWORK_ID);
    AIUT_EXPECT_EQ_INFO_LOG(result, 0,
        "should return 0 for non-existent adapter/pipe");
}

/**
 * @tc.name: AudioCoreConfigManager_IsFastStreamSupported_001
 * @tc.number: AudioCoreConfigManager_IsFastStreamSupported_001
 * @tc.desc: Test IsFastStreamSupported with empty desc list
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsFastStreamSupported_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsFastStreamSupported(streamInfo, desc),
        "should return false for empty desc");
}

/**
 * @tc.name: AudioCoreConfigManager_IsFastStreamSupported_002
 * @tc.number: AudioCoreConfigManager_IsFastStreamSupported_002
 * @tc.desc: Test IsFastStreamSupported with nullptr in desc list
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsFastStreamSupported_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc = {nullptr};
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsFastStreamSupported(streamInfo, desc),
        "should return false for nullptr desc");
}

/**
 * @tc.name: AudioCoreConfigManager_IsFastStreamSupported_003
 * @tc.number: AudioCoreConfigManager_IsFastStreamSupported_003
 * @tc.desc: Test IsFastStreamSupported with non-input/output device role
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsFastStreamSupported_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    auto deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    deviceDesc->deviceRole_ = DEVICE_ROLE_NONE;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc = {deviceDesc};
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsFastStreamSupported(streamInfo, desc),
        "should return false for non-input/output role");
}

/**
 * @tc.name: AudioCoreConfigManager_GetFastStreamSupport_001
 * @tc.number: AudioCoreConfigManager_GetFastStreamSupport_001
 * @tc.desc: Test GetFastStreamSupport with nullptr deviceInfo
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetFastStreamSupport_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    std::shared_ptr<AdapterDeviceInfo> deviceInfo = nullptr;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.GetFastStreamSupport(streamInfo, deviceInfo),
        "should return false for nullptr deviceInfo");
}

/**
 * @tc.name: AudioCoreConfigManager_GetFastStreamSupport_002
 * @tc.number: AudioCoreConfigManager_GetFastStreamSupport_002
 * @tc.desc: Test GetFastStreamSupport with invalid device role
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetFastStreamSupport_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    auto deviceInfo = std::make_shared<AdapterDeviceInfo>();
    deviceInfo->role_ = DEVICE_ROLE_NONE;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.GetFastStreamSupport(streamInfo, deviceInfo),
        "should return false for invalid role");
}

/**
 * @tc.name: AudioCoreConfigManager_GetFastStreamSupport_003
 * @tc.number: AudioCoreConfigManager_GetFastStreamSupport_003
 * @tc.desc: Test GetFastStreamSupport with empty supportPipeMap
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetFastStreamSupport_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    auto deviceInfo = std::make_shared<AdapterDeviceInfo>();
    deviceInfo->role_ = OUTPUT_DEVICE;
    deviceInfo->supportPipeMap_.clear();
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.GetFastStreamSupport(streamInfo, deviceInfo),
        "should return false for empty pipe map");
}

/**
 * @tc.name: AudioCoreConfigManager_GetFastStreamSupport_004
 * @tc.number: AudioCoreConfigManager_GetFastStreamSupport_004
 * @tc.desc: Test GetFastStreamSupport with nullptr pipe in map
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetFastStreamSupport_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    auto deviceInfo = std::make_shared<AdapterDeviceInfo>();
    deviceInfo->role_ = OUTPUT_DEVICE;
    deviceInfo->supportPipeMap_[AUDIO_OUTPUT_FLAG_FAST] = nullptr;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.GetFastStreamSupport(streamInfo, deviceInfo),
        "should return false for nullptr pipe");
}

/**
 * @tc.name: AudioCoreConfigManager_PreferMultiChannelPipe_001
 * @tc.number: AudioCoreConfigManager_PreferMultiChannelPipe_001
 * @tc.desc: Test PreferMultiChannelPipe with nullptr desc
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, PreferMultiChannelPipe_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    std::shared_ptr<AudioStreamDescriptor> desc = nullptr;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.PreferMultiChannelPipe(desc),
        "should return false for nullptr desc");
}

/**
 * @tc.name: AudioCoreConfigManager_PreferMultiChannelPipe_002
 * @tc.number: AudioCoreConfigManager_PreferMultiChannelPipe_002
 * @tc.desc: Test PreferMultiChannelPipe with empty newDeviceDescs
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, PreferMultiChannelPipe_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto desc = std::make_shared<AudioStreamDescriptor>();
    desc->newDeviceDescs_.clear();
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.PreferMultiChannelPipe(desc),
        "should return false for empty newDeviceDescs");
}

/**
 * @tc.name: AudioCoreConfigManager_PreferMultiChannelPipe_003
 * @tc.number: AudioCoreConfigManager_PreferMultiChannelPipe_003
 * @tc.desc: Test PreferMultiChannelPipe with nullptr front device
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, PreferMultiChannelPipe_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto desc = std::make_shared<AudioStreamDescriptor>();
    desc->newDeviceDescs_.push_back(nullptr);
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.PreferMultiChannelPipe(desc),
        "should return false for nullptr front device");
}

/**
 * @tc.name: AudioCoreConfigManager_SelectStreamPropInfo_001
 * @tc.number: AudioCoreConfigManager_SelectStreamPropInfo_001
 * @tc.desc: Test SelectStreamPropInfo with matching limitFunc
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, SelectStreamPropInfo_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto prop1 = std::make_shared<PipeStreamPropInfo>();
    prop1->sampleRate_ = 48000;
    auto prop2 = std::make_shared<PipeStreamPropInfo>();
    prop2->sampleRate_ = 44100;
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos = {prop1, prop2};
    auto limitFunc = [](std::shared_ptr<PipeStreamPropInfo> prop) {
        return prop != nullptr && prop->sampleRate_ == 48000;
    };
    auto result = configManager.SelectStreamPropInfo(infos, limitFunc);
    AIUT_EXPECT_EQ_INFO_LOG(result.size(), 1, "should return 1 matching prop");
}

/**
 * @tc.name: AudioCoreConfigManager_SelectStreamPropInfo_002
 * @tc.number: AudioCoreConfigManager_SelectStreamPropInfo_002
 * @tc.desc: Test SelectStreamPropInfo with no matching limitFunc
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, SelectStreamPropInfo_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto prop1 = std::make_shared<PipeStreamPropInfo>();
    prop1->sampleRate_ = 48000;
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos = {prop1};
    auto limitFunc = [](std::shared_ptr<PipeStreamPropInfo> prop) {
        return prop != nullptr && prop->sampleRate_ == 44100;
    };
    auto result = configManager.SelectStreamPropInfo(infos, limitFunc);
    AIUT_EXPECT_TRUE_INFO_LOG(result.empty(), "should return empty for no match");
}

/**
 * @tc.name: AudioCoreConfigManager_SortStreamPropInfos_001
 * @tc.number: AudioCoreConfigManager_SortStreamPropInfos_001
 * @tc.desc: Test SortStreamPropInfosBySampleRate with null elements
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, SortStreamPropInfos_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto prop1 = std::make_shared<PipeStreamPropInfo>();
    prop1->sampleRate_ = 48000;
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos = {nullptr, prop1, nullptr};
    configManager.SortStreamPropInfosBySampleRate(infos);
    AIUT_EXPECT_EQ_INFO_LOG(infos.size(), 3, "Sort should preserve all elements");
}

/**
 * @tc.name: AudioCoreConfigManager_SortStreamPropInfos_002
 * @tc.number: AudioCoreConfigManager_SortStreamPropInfos_002
 * @tc.desc: Test SortStreamPropInfosBySampleRate with same rate different format
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, SortStreamPropInfos_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto prop1 = std::make_shared<PipeStreamPropInfo>();
    prop1->sampleRate_ = 48000;
    prop1->format_ = SAMPLE_S32LE;
    auto prop2 = std::make_shared<PipeStreamPropInfo>();
    prop2->sampleRate_ = 48000;
    prop2->format_ = SAMPLE_S16LE;
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos = {prop1, prop2};
    configManager.SortStreamPropInfosBySampleRate(infos);
    AIUT_EXPECT_EQ_INFO_LOG(infos.size(), 2, "Sort should preserve all elements");
}

/**
 * @tc.name: AudioCoreConfigManager_SortStreamPropInfos_003
 * @tc.number: AudioCoreConfigManager_SortStreamPropInfos_003
 * @tc.desc: Test SortStreamPropInfosBySampleRate with different rates
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, SortStreamPropInfos_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto prop1 = std::make_shared<PipeStreamPropInfo>();
    prop1->sampleRate_ = 48000;
    auto prop2 = std::make_shared<PipeStreamPropInfo>();
    prop2->sampleRate_ = 44100;
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos = {prop1, prop2};
    configManager.SortStreamPropInfosBySampleRate(infos);
    AIUT_EXPECT_EQ_INFO_LOG(infos.size(), 2, "Sort should preserve all elements");
}

/**
 * @tc.name: AudioCoreConfigManager_GetSuitableStreamProp_001
 * @tc.number: AudioCoreConfigManager_GetSuitableStreamProp_001
 * @tc.desc: Test GetSuitableStreamPropInfo with matching prop
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetSuitableStreamProp_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto prop1 = std::make_shared<PipeStreamPropInfo>();
    prop1->sampleRate_ = 48000;
    prop1->format_ = SAMPLE_S16LE;
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos = {prop1};
    auto result = configManager.GetSuitableStreamPropInfo(SAMPLE_S16LE, 48000, infos);
    AIUT_ASSERT_NE_INFO_LOG(result, nullptr, "result should not be nullptr");
}

/**
 * @tc.name: AudioCoreConfigManager_GetSuitableStreamProp_002
 * @tc.number: AudioCoreConfigManager_GetSuitableStreamProp_002
 * @tc.desc: Test GetSuitableStreamPropInfo with nullptr in list
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetSuitableStreamProp_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto prop1 = std::make_shared<PipeStreamPropInfo>();
    prop1->sampleRate_ = 48000;
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos = {nullptr, prop1};
    auto result = configManager.GetSuitableStreamPropInfo(SAMPLE_S16LE, 48000, infos);
    AIUT_ASSERT_NE_INFO_LOG(result, nullptr, "result should not be nullptr");
}

/**
 * @tc.name: AudioCoreConfigManager_GetSuitableStreamProp_003
 * @tc.number: AudioCoreConfigManager_GetSuitableStreamProp_003
 * @tc.desc: Test GetSuitableStreamPropInfo returns back when no match
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetSuitableStreamProp_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto prop1 = std::make_shared<PipeStreamPropInfo>();
    prop1->sampleRate_ = 8000;
    prop1->format_ = SAMPLE_U8;
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos = {prop1};
    auto result = configManager.GetSuitableStreamPropInfo(SAMPLE_S32LE, 48000, infos);
    AIUT_ASSERT_NE_INFO_LOG(result, nullptr, "result should not be nullptr");
}

/**
 * @tc.name: AudioCoreConfigManager_MatchStreamPropInfo_001
 * @tc.number: AudioCoreConfigManager_MatchStreamPropInfo_001
 * @tc.desc: Test MatchStreamPropInfo with nullptr adapterPipeInfo
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, MatchStreamPropInfo_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto info = std::make_shared<PipeStreamPropInfo>();
    std::shared_ptr<AdapterPipeInfo> adapterPipeInfo = nullptr;
    AudioStreamInfo streamInfo;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.MatchStreamPropInfo(info, adapterPipeInfo, streamInfo),
        "should return false for nullptr adapterPipeInfo");
}

/**
 * @tc.name: AudioCoreConfigManager_GetStreamPropInfoFromPipe_001
 * @tc.number: AudioCoreConfigManager_GetStreamPropInfoFromPipe_001
 * @tc.desc: Test GetStreamPropInfoFromPipe with nullptr info
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetStreamPropInfoFromPipe_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    std::shared_ptr<AdapterPipeInfo> info = nullptr;
    AudioStreamInfo streamInfo;
    auto result = configManager.GetStreamPropInfoFromPipe(info, streamInfo);
    AIUT_EXPECT_EQ_INFO_LOG(result, nullptr, "should return nullptr for nullptr info");
}

/**
 * @tc.name: AudioCoreConfigManager_GetStreamPropInfoFromPipe_002
 * @tc.number: AudioCoreConfigManager_GetStreamPropInfoFromPipe_002
 * @tc.desc: Test GetStreamPropInfoFromPipe with empty streamPropInfos
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetStreamPropInfoFromPipe_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto info = std::make_shared<AdapterPipeInfo>();
    info->streamPropInfos_.clear();
    info->dynamicStreamPropInfosMap_.clear();
    AudioStreamInfo streamInfo;
    auto result = configManager.GetStreamPropInfoFromPipe(info, streamInfo);
    AIUT_EXPECT_EQ_INFO_LOG(result, nullptr, "should return nullptr for empty props");
}

/**
 * @tc.name: AudioCoreConfigManager_GetStreamPropInfoFromPipe_003
 * @tc.number: AudioCoreConfigManager_GetStreamPropInfoFromPipe_003
 * @tc.desc: Test GetStreamPropInfoFromPipe with matching xml config
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetStreamPropInfoFromPipe_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto info = std::make_shared<AdapterPipeInfo>();
    info->dynamicStreamPropInfosMap_.clear();
    auto propInfo = std::make_shared<PipeStreamPropInfo>();
    propInfo->format_ = SAMPLE_S16LE;
    propInfo->sampleRate_ = SAMPLE_RATE_48000;
    propInfo->channels_ = STEREO;
    info->streamPropInfos_ = {propInfo};
    AudioStreamInfo streamInfo(SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, STEREO);
    auto result = configManager.GetStreamPropInfoFromPipe(info, streamInfo);
    AIUT_ASSERT_NE_INFO_LOG(result, nullptr, "result should not be nullptr for matching props");
}

/**
 * @tc.name: AudioCoreConfigManager_GetStreamPropInfoFromPipe_004
 * @tc.number: AudioCoreConfigManager_GetStreamPropInfoFromPipe_004
 * @tc.desc: Test GetStreamPropInfoFromPipe with no matching xml config
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetStreamPropInfoFromPipe_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto info = std::make_shared<AdapterPipeInfo>();
    info->dynamicStreamPropInfosMap_.clear();
    auto propInfo = std::make_shared<PipeStreamPropInfo>();
    propInfo->format_ = SAMPLE_S16LE;
    propInfo->sampleRate_ = SAMPLE_RATE_48000;
    propInfo->channels_ = STEREO;
    info->streamPropInfos_ = {propInfo};
    AudioStreamInfo streamInfo(SAMPLE_RATE_44100, ENCODING_PCM, SAMPLE_S16LE, STEREO);
    auto result = configManager.GetStreamPropInfoFromPipe(info, streamInfo);
    AIUT_EXPECT_EQ_INFO_LOG(result, nullptr, "should return nullptr for no match");
}

/**
 * @tc.name: AudioCoreConfigManager_ClearStreamPropInfo_001
 * @tc.number: AudioCoreConfigManager_ClearStreamPropInfo_001
 * @tc.desc: Test ClearStreamPropInfo calls config data clear
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, ClearStreamPropInfo_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.ClearStreamPropInfo(TEST_ADAPTER_NAME, TEST_PIPE_NAME, LOCAL_NETWORK_ID);
    AIUT_EXPECT_EQ_INFO_LOG(configManager.GetStreamPropInfoSize(TEST_ADAPTER_NAME, TEST_PIPE_NAME, LOCAL_NETWORK_ID), 0,
        "GetStreamPropInfoSize should return 0 after clear");
}

/**
 * @tc.name: AudioCoreConfigManager_UpdateStreamPropInfo_001
 * @tc.number: AudioCoreConfigManager_UpdateStreamPropInfo_001
 * @tc.desc: Test UpdateStreamPropInfo with empty deviceStreamInfo
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, UpdateStreamPropInfo_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    std::list<DeviceStreamInfo> emptyStreamInfo;
    std::list<std::string> supportDevices = {"speaker"};
    configManager.UpdateStreamPropInfo(TEST_ADAPTER_NAME, TEST_PIPE_NAME,
        emptyStreamInfo, supportDevices, LOCAL_NETWORK_ID);
    AIUT_EXPECT_EQ_INFO_LOG(configManager.GetStreamPropInfoSize(TEST_ADAPTER_NAME, TEST_PIPE_NAME, LOCAL_NETWORK_ID), 0,
        "GetStreamPropInfoSize should return 0 for empty stream info");
}

/**
 * @tc.name: AudioCoreConfigManager_UpdateStreamPropInfo_002
 * @tc.number: AudioCoreConfigManager_UpdateStreamPropInfo_002
 * @tc.desc: Test UpdateStreamPropInfo with non-empty deviceStreamInfo
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, UpdateStreamPropInfo_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    DeviceStreamInfo streamInfo;
    streamInfo.samplingRate = {SAMPLE_RATE_48000};
    streamInfo.channelLayout = {CH_LAYOUT_STEREO};
    streamInfo.format = SAMPLE_S16LE;
    std::list<DeviceStreamInfo> deviceStreamInfo = {streamInfo};
    std::list<std::string> supportDevices = {"speaker"};
    configManager.UpdateStreamPropInfo(TEST_ADAPTER_NAME, TEST_PIPE_NAME,
        deviceStreamInfo, supportDevices, LOCAL_NETWORK_ID);
    AIUT_EXPECT_TRUE_INFO_LOG(
        configManager.GetStreamPropInfoSize(TEST_ADAPTER_NAME, TEST_PIPE_NAME, LOCAL_NETWORK_ID) >= 0,
        "GetStreamPropInfoSize should return non-negative after update");
}

/**
 * @tc.name: AudioCoreConfigManager_GetAdapterInfoByType_001
 * @tc.number: AudioCoreConfigManager_GetAdapterInfoByType_001
 * @tc.desc: Test GetAdapterInfoByType with non-existent type
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetAdapterInfoByType_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.GetAudioPolicyConfigData().adapterInfoMap.clear();
    std::shared_ptr<PolicyAdapterInfo> info;
    bool result = configManager.GetAdapterInfoByType(AudioAdapterType::TYPE_PRIMARY, info);
    AIUT_EXPECT_FALSE_INFO_LOG(result, "should return false for non-existent type");
}

/**
 * @tc.name: AudioCoreConfigManager_GetAudioAdapterInfos_001
 * @tc.number: AudioCoreConfigManager_GetAudioAdapterInfos_001
 * @tc.desc: Test GetAudioAdapterInfos delegates to config data
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetAudioAdapterInfos_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    std::unordered_map<AudioAdapterType, std::shared_ptr<PolicyAdapterInfo>> adapterInfoMap;
    configManager.GetAudioAdapterInfos(adapterInfoMap);
    AIUT_EXPECT_EQ_INFO_LOG(configManager.GetAdapterInfoFlag(), configManager.GetAdapterInfoFlag(),
        "GetAudioAdapterInfos should not crash, adapter flag accessible");
}

/**
 * @tc.name: AudioCoreConfigManager_GetVoipRendererFlag_001
 * @tc.number: AudioCoreConfigManager_GetVoipRendererFlag_001
 * @tc.desc: Test GetVoipRendererFlag with fast voip enabled and 48k rate
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetVoipRendererFlag_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.enableFastVoip_ = true;
    int32_t result = configManager.GetVoipRendererFlag(PRIMARY_SPEAKER, LOCAL_NETWORK_ID,
        SAMPLE_RATE_48000);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_FLAG_VOIP_FAST, "should return AUDIO_FLAG_VOIP_FAST");
}

/**
 * @tc.name: AudioCoreConfigManager_GetVoipRendererFlag_002
 * @tc.number: AudioCoreConfigManager_GetVoipRendererFlag_002
 * @tc.desc: Test GetVoipRendererFlag with fast voip enabled and 16k rate
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetVoipRendererFlag_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.enableFastVoip_ = true;
    int32_t result = configManager.GetVoipRendererFlag(PRIMARY_SPEAKER, LOCAL_NETWORK_ID,
        SAMPLE_RATE_16000);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_FLAG_VOIP_FAST, "should return AUDIO_FLAG_VOIP_FAST for 16k");
}

/**
 * @tc.name: AudioCoreConfigManager_GetVoipRendererFlag_003
 * @tc.number: AudioCoreConfigManager_GetVoipRendererFlag_003
 * @tc.desc: Test GetVoipRendererFlag with fast voip enabled but unsupported rate
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetVoipRendererFlag_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.enableFastVoip_ = true;
    int32_t result = configManager.GetVoipRendererFlag(PRIMARY_SPEAKER, LOCAL_NETWORK_ID,
        SAMPLE_RATE_8000);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_FLAG_NORMAL,
        "should return AUDIO_FLAG_NORMAL for unsupported rate");
}

/**
 * @tc.name: AudioCoreConfigManager_GetVoipRendererFlag_004
 * @tc.number: AudioCoreConfigManager_GetVoipRendererFlag_004
 * @tc.desc: Test GetVoipRendererFlag with direct voip mode
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetVoipRendererFlag_004, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.enableFastVoip_ = false;
    configManager.normalVoipFlag_ = false;
    int32_t result = configManager.GetVoipRendererFlag(PRIMARY_SPEAKER, LOCAL_NETWORK_ID,
        SAMPLE_RATE_48000);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_FLAG_VOIP_DIRECT, "should return AUDIO_FLAG_VOIP_DIRECT");
}

/**
 * @tc.name: AudioCoreConfigManager_GetVoipRendererFlag_005
 * @tc.number: AudioCoreConfigManager_GetVoipRendererFlag_005
 * @tc.desc: Test GetVoipRendererFlag with normal voip flag set
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetVoipRendererFlag_005, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.enableFastVoip_ = false;
    configManager.normalVoipFlag_ = true;
    int32_t result = configManager.GetVoipRendererFlag(PRIMARY_SPEAKER, LOCAL_NETWORK_ID,
        SAMPLE_RATE_48000);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_FLAG_NORMAL,
        "should return AUDIO_FLAG_NORMAL when normalVoipFlag set");
}

/**
 * @tc.name: AudioCoreConfigManager_GetVoipRendererFlag_006
 * @tc.number: AudioCoreConfigManager_GetVoipRendererFlag_006
 * @tc.desc: Test GetVoipRendererFlag with non-primary speaker
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetVoipRendererFlag_006, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.enableFastVoip_ = true;
    int32_t result = configManager.GetVoipRendererFlag("other_speaker", LOCAL_NETWORK_ID,
        SAMPLE_RATE_48000);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_FLAG_NORMAL,
        "should return AUDIO_FLAG_NORMAL for non-primary speaker");
}

/**
 * @tc.name: AudioCoreConfigManager_GetVoipRendererFlag_007
 * @tc.number: AudioCoreConfigManager_GetVoipRendererFlag_007
 * @tc.desc: Test GetVoipRendererFlag with non-local network id
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetVoipRendererFlag_007, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    configManager.enableFastVoip_ = true;
    int32_t result = configManager.GetVoipRendererFlag(PRIMARY_SPEAKER, "remote_network",
        SAMPLE_RATE_48000);
    AIUT_EXPECT_EQ_INFO_LOG(result, AUDIO_FLAG_NORMAL,
        "should return AUDIO_FLAG_NORMAL for remote network");
}

/**
 * @tc.name: AudioCoreConfigManager_IsOffloadPlaybackSupported_001
 * @tc.number: AudioCoreConfigManager_IsOffloadPlaybackSupported_001
 * @tc.desc: Test IsOffloadPlaybackSupported with non-music usage
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsOffloadPlaybackSupported_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsOffloadPlaybackSupported(streamInfo, desc,
        STREAM_USAGE_VOICE_COMMUNICATION), "should return false for non-music usage");
}

/**
 * @tc.name: AudioCoreConfigManager_IsOffloadPlaybackSupported_002
 * @tc.number: AudioCoreConfigManager_IsOffloadPlaybackSupported_002
 * @tc.desc: Test IsOffloadPlaybackSupported with music usage and empty desc
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsOffloadPlaybackSupported_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsOffloadPlaybackSupported(streamInfo, desc,
        STREAM_USAGE_MUSIC), "should return false for empty desc");
}

/**
 * @tc.name: AudioCoreConfigManager_IsOffloadPlaybackSupported_003
 * @tc.number: AudioCoreConfigManager_IsOffloadPlaybackSupported_003
 * @tc.desc: Test IsOffloadPlaybackSupported with audiobook usage
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsOffloadPlaybackSupported_003, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsOffloadPlaybackSupported(streamInfo, desc,
        STREAM_USAGE_AUDIOBOOK), "should return false for empty desc with audiobook");
}

/**
 * @tc.name: AudioCoreConfigManager_IsMultichannelPlaybackSupported_001
 * @tc.number: AudioCoreConfigManager_IsMultichannelPlaybackSupported_001
 * @tc.desc: Test IsMultichannelPlaybackSupported with empty desc
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsMultichannelPlaybackSupported_001,
    TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsMultichannelPlaybackSupported(streamInfo, desc),
        "should return false for empty desc");
}

/**
 * @tc.name: AudioCoreConfigManager_IsMultichannelPlaybackSupported_002
 * @tc.number: AudioCoreConfigManager_IsMultichannelPlaybackSupported_002
 * @tc.desc: Test IsMultichannelPlaybackSupported with nullptr in desc
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsMultichannelPlaybackSupported_002,
    TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc = {nullptr};
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsMultichannelPlaybackSupported(streamInfo, desc),
        "should return false for nullptr desc");
}

/**
 * @tc.name: AudioCoreConfigManager_IsDirectPlaybackSupported_001
 * @tc.number: AudioCoreConfigManager_IsDirectPlaybackSupported_001
 * @tc.desc: Test IsDirectPlaybackSupported with empty desc
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsDirectPlaybackSupported_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc;
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsDirectPlaybackSupported(streamInfo, desc,
        STREAM_USAGE_MUSIC), "should return false for empty desc");
}

/**
 * @tc.name: AudioCoreConfigManager_IsDirectPlaybackSupported_002
 * @tc.number: AudioCoreConfigManager_IsDirectPlaybackSupported_002
 * @tc.desc: Test IsDirectPlaybackSupported with nullptr in desc
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, IsDirectPlaybackSupported_002, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    AudioStreamInfo streamInfo;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> desc = {nullptr};
    AIUT_EXPECT_FALSE_INFO_LOG(configManager.IsDirectPlaybackSupported(streamInfo, desc,
        STREAM_USAGE_MUSIC), "should return false for nullptr desc");
}

/**
 * @tc.name: AudioCoreConfigManager_GetDirectPlaybackSupport_001
 * @tc.number: AudioCoreConfigManager_GetDirectPlaybackSupport_001
 * @tc.desc: Test GetDirectPlaybackSupport with invalid device type
 */
HWTEST_F(AudioCoreConfigManagerQueryAiutTest, GetDirectPlaybackSupport_001, TestSize.Level1)
{
    AIUT_INFO_LOG("entry");
    auto &configManager = AudioCoreConfigManager::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_INVALID;
    AudioStreamInfo streamInfo;
    AIUT_EXPECT_EQ_INFO_LOG(configManager.GetDirectPlaybackSupport(desc, streamInfo),
        DIRECT_PLAYBACK_NOT_SUPPORTED,
        "should return NOT_SUPPORTED for invalid device type");
}

} // namespace AudioStandard
} // namespace OHOS
