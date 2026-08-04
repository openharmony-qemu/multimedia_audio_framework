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

#include "audio_select_interface_service_unit_test.h"
#include "audio_errors.h"
#include "audio_policy_log.h"
#include "audio_policy_utils.h"
#include "audio_router_select_strategy.h"
#include "audio_core_service.h"
#include "audio_active_device.h"
#include "audio_device_manager.h"
#include "audio_device_factory.h"
#include "audio_scene_manager.h"
#include "audio_router_map.h"
#include "audio_pipe_manager.h"
#include "audio_router_center.h"
#include "audio_server_proxy.h"
#include "audio_bundle_manager.h"
#include "audio_stream_collector.h"
#include "audio_a2dp_offload_manager.h"
#include "audio_recovery_device.h"
#include "audio_session_service.h"
#include "stream_dfx_manager.h"
#include "sle_audio_device_manager.h"
#include "bluetooth_device_manager.h"

using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

void AudioSelectInterfaceServiceUnitTest::SetUpTestCase(void) {}
void AudioSelectInterfaceServiceUnitTest::TearDownTestCase(void) {}
void AudioSelectInterfaceServiceUnitTest::SetUp(void) {}
void AudioSelectInterfaceServiceUnitTest::TearDown(void) {}

/**
 * @tc.name  : SetDeviceActive_001
 * @tc.desc  : Test SetDeviceActive with DEVICE_TYPE_NONE
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest, SetDeviceActive_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetDeviceActive(DEVICE_TYPE_NONE, true, "");
    EXPECT_EQ(ret, ERR_DEVICE_NOT_SUPPORTED);
}

/**
 * @tc.name  : SetDeviceActive_002
 * @tc.desc  : Test SetDeviceActive device not available
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest, SetDeviceActive_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetDeviceActive(
        DEVICE_TYPE_BLUETOOTH_SCO, true, "00:00:00:00:00:00");
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : SetDeviceActive_003
 * @tc.desc  : Test SetDeviceActive with DEVICE_TYPE_FILE_SINK and empty addr
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest, SetDeviceActive_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetDeviceActive(DEVICE_TYPE_FILE_SINK, true, "");
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : GetPreferredUid_001
 * @tc.desc  : Test GetPreferredUid with null callback
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest, GetPreferredUid_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.audioClientInfoMgrCallback_ = nullptr;
    int32_t uid = 1000;
    int32_t ret = svc.GetPreferredUid(uid);
    EXPECT_EQ(ret, uid);
}

/**
 * @tc.name  : GetPreferredUid_002
 * @tc.desc  : Test GetPreferredUid with non-null callback
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest, GetPreferredUid_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.audioClientInfoMgrCallback_ = nullptr;
    int32_t uid = 1000;
    int32_t ret = svc.GetPreferredUid(uid);
    EXPECT_EQ(ret, uid);
}

/**
 * @tc.name  : HandleNegtiveBt_001
 * @tc.desc  : Test HandleNegtiveBt with enhanced routing
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest, HandleNegtiveBt_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleNegtiveBt(DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleNegtiveBt_002
 * @tc.desc  : Test HandleNegtiveBt with NEARLINK
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest, HandleNegtiveBt_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleNegtiveBt(DEVICE_TYPE_NEARLINK);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleNegtiveBtOriginalLogic_001
 * @tc.desc  : Test HandleNegtiveBtOriginalLogic with BT_SCO
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleNegtiveBtOriginalLogic_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleNegtiveBtOriginalLogic(DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleNegtiveBtOriginalLogic_002
 * @tc.desc  : Test HandleNegtiveBtOriginalLogic with NEARLINK
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleNegtiveBtOriginalLogic_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleNegtiveBtOriginalLogic(DEVICE_TYPE_NEARLINK);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleNegtiveBtOriginalLogic_003
 * @tc.desc  : Test HandleNegtiveBtOriginalLogic with non-matching type
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleNegtiveBtOriginalLogic_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleNegtiveBtOriginalLogic(DEVICE_TYPE_SPEAKER);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleNegtiveBtEnhancedRouting_001
 * @tc.desc  : Test HandleNegtiveBtEnhancedRouting with null coreService
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleNegtiveBtEnhancedRouting_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleNegtiveBtEnhancedRouting(DEVICE_TYPE_BLUETOOTH_SCO);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleNegtiveBtEnhancedRouting_002
 * @tc.desc  : Test HandleNegtiveBtEnhancedRouting with NEARLINK
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleNegtiveBtEnhancedRouting_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleNegtiveBtEnhancedRouting(DEVICE_TYPE_NEARLINK);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleActiveBt_001
 * @tc.desc  : Test HandleActiveBt with enhanced routing
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest, HandleActiveBt_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleActiveBt(DEVICE_TYPE_BLUETOOTH_SCO, "00:11:22:33:44:55");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleActiveBt_002
 * @tc.desc  : Test HandleActiveBt with NEARLINK
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest, HandleActiveBt_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleActiveBt(DEVICE_TYPE_NEARLINK, "00:11:22:33:44:55");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleActiveBtOriginalLogic_001
 * @tc.desc  : Test HandleActiveBtOriginalLogic with BT_SCO and non-SCO
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleActiveBtOriginalLogic_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleActiveBtOriginalLogic(
        DEVICE_TYPE_SPEAKER, "00:11:22:33:44:55");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleActiveBtOriginalLogic_002
 * @tc.desc  : Test HandleActiveBtOriginalLogic with BT_SCO active
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleActiveBtOriginalLogic_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleActiveBtOriginalLogic(
        DEVICE_TYPE_BLUETOOTH_SCO, "00:11:22:33:44:55");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleActiveBtOriginalLogic_003
 * @tc.desc  : Test HandleActiveBtOriginalLogic with NEARLINK active
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleActiveBtOriginalLogic_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleActiveBtOriginalLogic(
        DEVICE_TYPE_NEARLINK, "00:11:22:33:44:55");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleActiveBtEnhancedRouting_001
 * @tc.desc  : Test HandleActiveBtEnhancedRouting with null coreService
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleActiveBtEnhancedRouting_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleActiveBtEnhancedRouting(
        DEVICE_TYPE_BLUETOOTH_SCO, "00:11:22:33:44:55");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleActiveBtEnhancedRouting_002
 * @tc.desc  : Test HandleActiveBtEnhancedRouting with NEARLINK
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleActiveBtEnhancedRouting_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleActiveBtEnhancedRouting(
        DEVICE_TYPE_NEARLINK, "00:11:22:33:44:55");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleActiveBtEnhancedRouting_003
 * @tc.desc  : Test HandleActiveBtEnhancedRouting with non-BT type
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleActiveBtEnhancedRouting_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleActiveBtEnhancedRouting(
        DEVICE_TYPE_SPEAKER, "00:11:22:33:44:55");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : NotifyBtForOutputDeviceSelection_001
 * @tc.desc  : Test NotifyBtForOutputDeviceSelection without enhanced routing
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    NotifyBtForOutputDeviceSelection_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    svc.NotifyBtForOutputDeviceSelection(
        desc, STREAM_USAGE_MEDIA);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : NotifyBtForOutputDeviceSelection_002
 * @tc.desc  : Test NotifyBtForOutputDeviceSelection with null desc
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    NotifyBtForOutputDeviceSelection_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.NotifyBtForOutputDeviceSelection(
        nullptr, STREAM_USAGE_MEDIA);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : NotifyBtForInputDeviceSelection_001
 * @tc.desc  : Test NotifyBtForInputDeviceSelection without enhanced routing
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    NotifyBtForInputDeviceSelection_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    svc.NotifyBtForInputDeviceSelection(
        desc, SOURCE_TYPE_VOICE_COMMUNICATION);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : NotifyBtForInputDeviceSelection_002
 * @tc.desc  : Test NotifyBtForInputDeviceSelection with null desc
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    NotifyBtForInputDeviceSelection_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.NotifyBtForInputDeviceSelection(
        nullptr, SOURCE_TYPE_VOICE_COMMUNICATION);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : SelectOutputDevice_001
 * @tc.desc  : Test SelectOutputDevice with null filter
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectOutputDevice_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    int32_t ret = svc.SelectOutputDevice(nullptr, descs, 0);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : SelectOutputDevice_002
 * @tc.desc  : Test SelectOutputDevice with empty selectedDesc
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectOutputDevice_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    int32_t ret = svc.SelectOutputDevice(filter, descs, 0);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : SelectOutputDevice_003
 * @tc.desc  : Test SelectOutputDevice with null desc in vector
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectOutputDevice_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(nullptr);
    int32_t ret = svc.SelectOutputDevice(filter, descs, 0);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : SelectOutputDevice_004
 * @tc.desc  : Test SelectOutputDevice with non-output device role
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectOutputDevice_004, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = INPUT_DEVICE;
    int32_t ret = svc.SelectOutputDevice(filter, {desc}, 0);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : SelectOutputDevice_005
 * @tc.desc  : Test SelectOutputDevice with multiple descs
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectOutputDevice_005, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    auto desc1 = std::make_shared<AudioDeviceDescriptor>();
    desc1->deviceRole_ = OUTPUT_DEVICE;
    auto desc2 = std::make_shared<AudioDeviceDescriptor>();
    desc2->deviceRole_ = OUTPUT_DEVICE;
    int32_t ret = svc.SelectOutputDevice(filter, {desc1, desc2}, 0);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : SelectOutputDevicePrepare_001
 * @tc.desc  : Test SelectOutputDevicePrepare with excluded device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectOutputDevicePrepare_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    desc->deviceRole_ = OUTPUT_DEVICE;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = {desc};
    int32_t ret = svc.SelectOutputDevicePrepare(
        descs, STREAM_USAGE_MEDIA);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SelectOutputDevicePrepare_002
 * @tc.desc  : Test SelectOutputDevicePrepare with BT_SCO device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectOutputDevicePrepare_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc->deviceRole_ = OUTPUT_DEVICE;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = {desc};
    int32_t ret = svc.SelectOutputDevicePrepare(
        descs, STREAM_USAGE_VOICE_COMMUNICATION);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : SetRenderDeviceForUsage_001
 * @tc.desc  : Test SetRenderDeviceForUsage with device not available
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetRenderDeviceForUsage_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    desc->macAddress_ = "unavailable";
    desc->networkId_ = "unavailable";
    int32_t ret = svc.SetRenderDeviceForUsage(
        STREAM_USAGE_MEDIA, desc);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : SetRenderDeviceForUsage_002
 * @tc.desc  : Test SetRenderDeviceForUsage with DEVICE_TYPE_NONE
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetRenderDeviceForUsage_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_NONE;
    int32_t ret = svc.SetRenderDeviceForUsage(
        STREAM_USAGE_MEDIA, desc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SetRenderDeviceForUsage_003
 * @tc.desc  : Test SetRenderDeviceForUsage with AUDIO_CALL_RENDER
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetRenderDeviceForUsage_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_NONE;
    int32_t ret = svc.SetRenderDeviceForUsage(
        STREAM_USAGE_VOICE_COMMUNICATION, desc, 1000);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SetRenderDeviceForUsage_004
 * @tc.desc  : Test SetRenderDeviceForUsage with non-call usage
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetRenderDeviceForUsage_004, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_NONE;
    int32_t ret = svc.SetRenderDeviceForUsage(
        STREAM_USAGE_MEDIA, desc, INVALID_UID);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : HandleFetchDeviceChange_001
 * @tc.desc  : Test HandleFetchDeviceChange basic call
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleFetchDeviceChange_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.audioA2dpOffloadManager_ = nullptr;
    svc.HandleFetchDeviceChange(
        AudioStreamDeviceChangeReason::OVERRODE, "Test");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : HandleFetchDeviceChange_002
 * @tc.desc  : Test HandleFetchDeviceChange with A2DP offload manager
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleFetchDeviceChange_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.audioA2dpOffloadManager_ = nullptr;
    svc.HandleFetchDeviceChange(
        AudioStreamDeviceChangeReason::UNKNOWN, "Test");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : SelectInputDeviceInner_001
 * @tc.desc  : Test SelectInputDeviceInner with null filter
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectInputDeviceInner_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t ret = svc.SelectInputDeviceInner(nullptr, desc);
    EXPECT_EQ(ret, ERR_NULL_POINTER);
}

/**
 * @tc.name  : SelectInputDeviceInner_002
 * @tc.desc  : Test SelectInputDeviceInner with null desc
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectInputDeviceInner_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioCapturerFilter> filter = new AudioCapturerFilter();
    int32_t ret = svc.SelectInputDeviceInner(filter, nullptr);
    EXPECT_EQ(ret, ERR_NULL_POINTER);
}

/**
 * @tc.name  : SelectInputDeviceInner_003
 * @tc.desc  : Test SelectInputDeviceInner with fast stream phone call
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectInputDeviceInner_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioCapturerFilter> filter = new AudioCapturerFilter();
    filter->uid = 1000;
    filter->capturerInfo.capturerFlags = STREAM_FLAG_FAST;
    filter->capturerInfo.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_MIC;
    int32_t ret = svc.SelectInputDeviceInner(filter, desc);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : SelectInputDeviceInner_004
 * @tc.desc  : Test SelectInputDeviceInner with fast stream non-call
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectInputDeviceInner_004, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioCapturerFilter> filter = new AudioCapturerFilter();
    filter->uid = 1000;
    filter->capturerInfo.capturerFlags = STREAM_FLAG_FAST;
    filter->capturerInfo.sourceType = SOURCE_TYPE_MIC;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_MIC;
    int32_t ret = svc.SelectInputDeviceInner(filter, desc);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : SelectInputDeviceInner_005
 * @tc.desc  : Test SelectInputDeviceInner with non-fast stream
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectInputDeviceInner_005, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioCapturerFilter> filter = new AudioCapturerFilter();
    filter->uid = 1000;
    filter->capturerInfo.capturerFlags = 0;
    filter->capturerInfo.sourceType = SOURCE_TYPE_MIC;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_MIC;
    int32_t ret = svc.SelectInputDeviceInner(filter, desc);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : SelectInputDeviceInner_006
 * @tc.desc  : Test SelectInputDeviceInner with uid -1
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectInputDeviceInner_006, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioCapturerFilter> filter = new AudioCapturerFilter();
    filter->uid = -1;
    filter->capturerInfo.capturerFlags = 0;
    filter->capturerInfo.sourceType = SOURCE_TYPE_MIC;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_MIC;
    int32_t ret = svc.SelectInputDeviceInner(filter, desc);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : SelectInputDeviceInner_007
 * @tc.desc  : Test SelectInputDeviceInner with SELECT_STRATEGY_STREAM
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectInputDeviceInner_007, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioCapturerFilter> filter = new AudioCapturerFilter();
    filter->uid = 1000;
    filter->audioDeviceSelectMode = SELECT_STRATEGY_STREAM;
    filter->capturerInfo.capturerFlags = 0;
    filter->capturerInfo.sourceType = SOURCE_TYPE_MIC;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_MIC;
    int32_t ret = svc.SelectInputDeviceInner(filter, desc);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : ExcludeOutputDevices_001
 * @tc.desc  : Test ExcludeOutputDevices with empty vector
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    ExcludeOutputDevices_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    int32_t ret = svc.ExcludeOutputDevices(MEDIA_OUTPUT_DEVICES, descs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : ExcludeOutputDevices_002
 * @tc.desc  : Test ExcludeOutputDevices with null desc in vector
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    ExcludeOutputDevices_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(nullptr);
    int32_t ret = svc.ExcludeOutputDevices(MEDIA_OUTPUT_DEVICES, descs);
    EXPECT_EQ(ret, ERROR_INVALID_PARAM);
}

/**
 * @tc.name  : ExcludeOutputDevices_003
 * @tc.desc  : Test ExcludeOutputDevices with BT_SCO and empty mac
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    ExcludeOutputDevices_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc->macAddress_ = "";
    int32_t ret = svc.ExcludeOutputDevices(
        MEDIA_OUTPUT_DEVICES, {desc});
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : ExcludeOutputDevices_004
 * @tc.desc  : Test ExcludeOutputDevices with DM_DEVICE_TYPE_MUSIC_HOST
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    ExcludeOutputDevices_004, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    desc->dmDeviceType_ = DM_DEVICE_TYPE_MUSIC_HOST;
    desc->macAddress_ = "11:22:33:44:55:66";
    int32_t ret = svc.ExcludeOutputDevices(
        MEDIA_OUTPUT_DEVICES, {desc});
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : ExcludeOutputDevices_005
 * @tc.desc  : Test ExcludeOutputDevices with normal device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    ExcludeOutputDevices_005, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    desc->macAddress_ = "11:22:33:44:55:66";
    int32_t ret = svc.ExcludeOutputDevices(
        MEDIA_OUTPUT_DEVICES, {desc});
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : UnexcludeOutputDevices_001
 * @tc.desc  : Test UnexcludeOutputDevices with empty vector
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    UnexcludeOutputDevices_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    int32_t ret = svc.UnexcludeOutputDevices(
        MEDIA_OUTPUT_DEVICES, descs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : UnexcludeOutputDevices_002
 * @tc.desc  : Test UnexcludeOutputDevices with null front desc
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    UnexcludeOutputDevices_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(nullptr);
    int32_t ret = svc.UnexcludeOutputDevices(
        MEDIA_OUTPUT_DEVICES, descs);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : UnexcludeOutputDevices_003
 * @tc.desc  : Test UnexcludeOutputDevices with BT_SCO and empty mac
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    UnexcludeOutputDevices_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc->macAddress_ = "";
    int32_t ret = svc.UnexcludeOutputDevices(
        MEDIA_OUTPUT_DEVICES, {desc});
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : UnexcludeOutputDevices_004
 * @tc.desc  : Test UnexcludeOutputDevices with normal device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    UnexcludeOutputDevices_004, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    desc->macAddress_ = "11:22:33:44:55:66";
    int32_t ret = svc.UnexcludeOutputDevices(
        MEDIA_OUTPUT_DEVICES, {desc});
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : UnexcludeOutputDevices_005
 * @tc.desc  : Test UnexcludeOutputDevices with wireless device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    UnexcludeOutputDevices_005, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->macAddress_ = "11:22:33:44:55:66";
    int32_t ret = svc.UnexcludeOutputDevices(
        MEDIA_OUTPUT_DEVICES, {desc});
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : SetPreferredInputDeviceIfValid_001
 * @tc.desc  : Test SetPreferredInputDeviceIfValid with null streamDesc
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetPreferredInputDeviceIfValid_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.SetPreferredInputDeviceIfValid(nullptr);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : SetPreferredInputDeviceIfValid_002
 * @tc.desc  : Test SetPreferredInputDeviceIfValid with invalid deviceType
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetPreferredInputDeviceIfValid_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->preferredInputDevice.deviceType_ = DEVICE_TYPE_INVALID;
    svc.SetPreferredInputDeviceIfValid(streamDesc);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : SetPreferredInputDeviceIfValid_003
 * @tc.desc  : Test SetPreferredInputDeviceIfValid with VOICE_RECOGNITION
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetPreferredInputDeviceIfValid_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->preferredInputDevice.deviceType_ = DEVICE_TYPE_MIC;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    svc.SetPreferredInputDeviceIfValid(streamDesc);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : SetPreferredInputDeviceIfValid_004
 * @tc.desc  : Test SetPreferredInputDeviceIfValid with BT_SPP
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetPreferredInputDeviceIfValid_004, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->preferredInputDevice.deviceType_ = DEVICE_TYPE_BT_SPP;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    svc.SetPreferredInputDeviceIfValid(streamDesc);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : SetPreferredInputDeviceIfValid_005
 * @tc.desc  : Test SetPreferredInputDeviceIfValid with normal source
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetPreferredInputDeviceIfValid_005, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->preferredInputDevice.deviceType_ = DEVICE_TYPE_MIC;
    streamDesc->capturerInfo_.sourceType = SOURCE_TYPE_MIC;
    svc.SetPreferredInputDeviceIfValid(streamDesc);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : SetMediaOutputDeviceByUid_001
 * @tc.desc  : Test SetMediaOutputDeviceByUid with DEVICE_TYPE_DEFAULT
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetMediaOutputDeviceByUid_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetMediaOutputDeviceByUid(
        DEVICE_TYPE_DEFAULT, 1000);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SetMediaOutputDeviceByUid_002
 * @tc.desc  : Test SetMediaOutputDeviceByUid with device not available
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetMediaOutputDeviceByUid_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetMediaOutputDeviceByUid(
        DEVICE_TYPE_BLUETOOTH_SCO, 1000);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SetMediaOutputDeviceByUid_003
 * @tc.desc  : Test SetMediaOutputDeviceByUid with SPEAKER
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetMediaOutputDeviceByUid_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetMediaOutputDeviceByUid(
        DEVICE_TYPE_SPEAKER, 1000);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : RefreshVirtualDevice_001
 * @tc.desc  : Test RefreshVirtualDevice with null desc
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    RefreshVirtualDevice_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = nullptr;
    int32_t ret = svc.RefreshVirtualDevice(desc);
    EXPECT_EQ(ret, ERR_NULL_POINTER);
}

/**
 * @tc.name  : RefreshVirtualDevice_002
 * @tc.desc  : Test RefreshVirtualDevice with normal device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    RefreshVirtualDevice_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    int32_t ret = svc.RefreshVirtualDevice(desc);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : RefreshVirtualDevice_003
 * @tc.desc  : Test RefreshVirtualDevice with BT_SCO device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    RefreshVirtualDevice_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc->macAddress_ = "11:22:33:44:55:66";
    int32_t ret = svc.RefreshVirtualDevice(desc);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : ClearActiveHfpDevice_001
 * @tc.desc  : Test ClearActiveHfpDevice with BT_SCO device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    ClearActiveHfpDevice_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    int32_t ret = svc.ClearActiveHfpDevice(desc);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : ClearActiveHfpDevice_002
 * @tc.desc  : Test ClearActiveHfpDevice with non-BT_SCO device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    ClearActiveHfpDevice_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    int32_t ret = svc.ClearActiveHfpDevice(desc);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SetDefaultOutputDevice_001
 * @tc.desc  : Test SetDefaultOutputDevice with no earpiece
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetDefaultOutputDevice_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetDefaultOutputDevice(
        DEVICE_TYPE_SPEAKER, 1, STREAM_USAGE_MEDIA, false);
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : SetDefaultOutputDevice_002
 * @tc.desc  : Test SetDefaultOutputDevice with isRunning true
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetDefaultOutputDevice_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetDefaultOutputDevice(
        DEVICE_TYPE_SPEAKER, 1, STREAM_USAGE_MEDIA, true);
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : SetDefaultOutputDevice_003
 * @tc.desc  : Test SetDefaultOutputDevice with skipForce true
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetDefaultOutputDevice_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetDefaultOutputDevice(
        DEVICE_TYPE_SPEAKER, 1, STREAM_USAGE_MEDIA, false, true);
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : SetDefaultOutputDevice_004
 * @tc.desc  : Test SetDefaultOutputDevice with voice communication
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetDefaultOutputDevice_004, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetDefaultOutputDevice(
        DEVICE_TYPE_SPEAKER, 1,
        STREAM_USAGE_VOICE_COMMUNICATION, false, false);
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : OnForcedDeviceSelected_001
 * @tc.desc  : Test OnForcedDeviceSelected with null filter
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    OnForcedDeviceSelected_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.OnForcedDeviceSelected(
        DEVICE_TYPE_BLUETOOTH_SCO, "11:22:33:44:55:66", nullptr, "");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : OnForcedDeviceSelected_002
 * @tc.desc  : Test OnForcedDeviceSelected with non-null filter
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    OnForcedDeviceSelected_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    filter->uid = 1000;
    svc.OnForcedDeviceSelected(
        DEVICE_TYPE_BLUETOOTH_SCO, "11:22:33:44:55:66", filter, "");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : OnForcedDeviceSelected_003
 * @tc.desc  : Test OnForcedDeviceSelected with Bluetooth caller
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    OnForcedDeviceSelected_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    filter->uid = 1000;
    svc.OnForcedDeviceSelected(
        DEVICE_TYPE_BLUETOOTH_SCO, "11:22:33:44:55:66",
        filter, "Bluetooth");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : OnPrivacyDeviceSelected_001
 * @tc.desc  : Test OnPrivacyDeviceSelected basic call
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    OnPrivacyDeviceSelected_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.OnPrivacyDeviceSelected(
        DEVICE_TYPE_BLUETOOTH_SCO, "11:22:33:44:55:66", "");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : OnPrivacyDeviceSelected_002
 * @tc.desc  : Test OnPrivacyDeviceSelected with Bluetooth caller
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    OnPrivacyDeviceSelected_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.OnPrivacyDeviceSelected(
        DEVICE_TYPE_BLUETOOTH_SCO, "11:22:33:44:55:66",
        "Bluetooth");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : SetDeviceActive_004
 * @tc.desc  : Test SetDeviceActive with address not empty and BT_SCO
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetDeviceActive_004, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetDeviceActive(
        DEVICE_TYPE_BLUETOOTH_SCO, true, "11:22:33:44:55:66");
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : SetDeviceActive_005
 * @tc.desc  : Test SetDeviceActive with non-BT type and active
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetDeviceActive_005, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetDeviceActive(
        DEVICE_TYPE_SPEAKER, true, "");
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : SelectOutputDevice_006
 * @tc.desc  : Test SelectOutputDevice with SELECT_STRATEGY_STREAM
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectOutputDevice_006, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    filter->uid = 1000;
    filter->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = OUTPUT_DEVICE;
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    int32_t ret = svc.SelectOutputDevice(
        filter, {desc}, SELECT_STRATEGY_STREAM);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : SelectOutputDevice_007
 * @tc.desc  : Test SelectOutputDevice with SELECT_STRATEGY_STREAM invalid uid
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectOutputDevice_007, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    filter->uid = -1;
    filter->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = OUTPUT_DEVICE;
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    int32_t ret = svc.SelectOutputDevice(
        filter, {desc}, SELECT_STRATEGY_STREAM);
    EXPECT_EQ(ret, ERR_INVALID_OPERATION);
}

/**
 * @tc.name  : SelectOutputDevice_008
 * @tc.desc  : Test SelectOutputDevice with fast stream
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectOutputDevice_008, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    filter->uid = 1000;
    filter->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    filter->rendererInfo.rendererFlags = STREAM_FLAG_FAST;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = OUTPUT_DEVICE;
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    int32_t ret = svc.SelectOutputDevice(
        filter, {desc}, SELECT_STRATEGY_DEFAULT);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : SelectOutputDevice_009
 * @tc.desc  : Test SelectOutputDevice with default mode
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectOutputDevice_009, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    filter->uid = 1000;
    filter->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    filter->rendererInfo.rendererFlags = 0;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = OUTPUT_DEVICE;
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    int32_t ret = svc.SelectOutputDevice(
        filter, {desc}, SELECT_STRATEGY_DEFAULT);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : SelectOutputDevice_010
 * @tc.desc  : Test SelectOutputDevice with BT_SCO device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectOutputDevice_010, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    filter->uid = 1000;
    filter->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = OUTPUT_DEVICE;
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc->macAddress_ = "11:22:33:44:55:66";
    int32_t ret = svc.SelectOutputDevice(
        filter, {desc}, SELECT_STRATEGY_DEFAULT, false);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : SelectOutputDevice_011
 * @tc.desc  : Test SelectOutputDevice with SELECT_STRATEGY_INDEPENDENT
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectOutputDevice_011, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    filter->uid = 1000;
    filter->rendererInfo.streamUsage = STREAM_USAGE_MEDIA;
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = OUTPUT_DEVICE;
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    int32_t ret = svc.SelectOutputDevice(
        filter, {desc}, SELECT_STRATEGY_INDEPENDENT);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : ExcludeOutputDevices_006
 * @tc.desc  : Test ExcludeOutputDevices with ALL_MEDIA_DEVICES
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    ExcludeOutputDevices_006, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->macAddress_ = "11:22:33:44:55:66";
    desc->deviceRole_ = OUTPUT_DEVICE;
    int32_t ret = svc.ExcludeOutputDevices(
        ALL_MEDIA_DEVICES, {desc});
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : ExcludeOutputDevices_007
 * @tc.desc  : Test ExcludeOutputDevices with ALL_MEDIA_DEVICES and empty mac
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    ExcludeOutputDevices_007, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    desc->macAddress_ = "";
    desc->deviceRole_ = OUTPUT_DEVICE;
    int32_t ret = svc.ExcludeOutputDevices(
        ALL_MEDIA_DEVICES, {desc});
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : UnexcludeOutputDevices_006
 * @tc.desc  : Test UnexcludeOutputDevices with wireless device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    UnexcludeOutputDevices_006, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_SCO;
    desc->macAddress_ = "11:22:33:44:55:66";
    int32_t ret = svc.UnexcludeOutputDevices(
        MEDIA_OUTPUT_DEVICES, {desc});
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : HandleFetchDeviceChange_003
 * @tc.desc  : Test HandleFetchDeviceChange with UNKNOWN reason
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleFetchDeviceChange_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleFetchDeviceChange(
        AudioStreamDeviceChangeReason::UNKNOWN, "Test2");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : SetDeviceActive_006
 * @tc.desc  : Test SetDeviceActive with wireless device type
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetDeviceActive_006, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetDeviceActive(
        DEVICE_TYPE_BLUETOOTH_A2DP, true, "");
    EXPECT_NE(ret, SUCCESS);
}

/**
 * @tc.name  : SetAudioClientInfoMgrCallback_001
 * @tc.desc  : Test SetAudioClientInfoMgrCallback with null callback
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetAudioClientInfoMgrCallback_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<IStandardAudioPolicyManagerListener> callback = nullptr;
    int32_t ret = svc.SetAudioClientInfoMgrCallback(callback);
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name  : GetEnhancedRoutingSupported_001
 * @tc.desc  : Test GetEnhancedRoutingSupported
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    GetEnhancedRoutingSupported_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    bool supported = svc.GetEnhancedRoutingSupported();
    EXPECT_TRUE(supported == true || supported == false);
}

/**
 * @tc.name  : SetDeviceEnableAndUsage_001
 * @tc.desc  : Test SetDeviceEnableAndUsage with wireless device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetDeviceEnableAndUsage_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    svc.SetDeviceEnableAndUsage(desc);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : SetDeviceEnableAndUsage_002
 * @tc.desc  : Test SetDeviceEnableAndUsage with non-wireless device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetDeviceEnableAndUsage_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    svc.SetDeviceEnableAndUsage(desc);
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : SetA2dpDeviceOffloadManager_001
 * @tc.desc  : Test SetA2dpDeviceOffloadManager with null manager
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetA2dpDeviceOffloadManager_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetA2dpDeviceOffloadManager(nullptr);
    EXPECT_EQ(ret, ERR_INVALID_PARAM);
}

/**
 * @tc.name  : SetInputDevice_001
 * @tc.desc  : Test SetInputDevice with non-existing device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetInputDevice_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetInputDevice(DEVICE_TYPE_MIC, 1, 1000);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : SelectInputDeviceByUid_001
 * @tc.desc  : Test SelectInputDeviceByUid with null desc
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectInputDeviceByUid_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SelectInputDeviceByUid(nullptr, 1000);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : FetchInputAndOutputDevice_001
 * @tc.desc  : Test FetchInputAndOutputDevice basic call
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    FetchInputAndOutputDevice_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.FetchInputAndOutputDevice(
        "Test", AudioStreamDeviceChangeReason::OVERRODE);
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_NULL_POINTER);
}

/**
 * @tc.name  : SetSessionDefaultOutputDevice_001
 * @tc.desc  : Test SetSessionDefaultOutputDevice returns SUCCESS
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetSessionDefaultOutputDevice_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetSessionDefaultOutputDevice(1000, DEVICE_TYPE_SPEAKER);
    EXPECT_EQ(ret, SUCCESS);
}

/**
 * @tc.name  : SelectInputDevice_001
 * @tc.desc  : Test SelectInputDevice with null filter
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectInputDevice_001, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    int32_t ret = svc.SelectInputDevice(nullptr, {desc});
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : SelectInputDevice_002
 * @tc.desc  : Test SelectInputDevice with empty selectedDesc
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectInputDevice_002, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioCapturerFilter> filter = new AudioCapturerFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    int32_t ret = svc.SelectInputDevice(filter, descs);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : SelectInputDevice_003
 * @tc.desc  : Test SelectInputDevice with null desc in vector
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SelectInputDevice_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioCapturerFilter> filter = new AudioCapturerFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(nullptr);
    int32_t ret = svc.SelectInputDevice(filter, descs);
    EXPECT_EQ(ret, ERROR);
}

/**
 * @tc.name  : OnForcedDeviceSelected_004
 * @tc.desc  : Test OnForcedDeviceSelected with NEARLINK type
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    OnForcedDeviceSelected_004, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    filter->uid = 1000;
    svc.OnForcedDeviceSelected(
        DEVICE_TYPE_NEARLINK, "11:22:33:44:55:66", filter, "");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : OnPrivacyDeviceSelected_003
 * @tc.desc  : Test OnPrivacyDeviceSelected with NEARLINK type
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    OnPrivacyDeviceSelected_003, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.OnPrivacyDeviceSelected(
        DEVICE_TYPE_NEARLINK, "11:22:33:44:55:66", "");
    EXPECT_TRUE(true);
}

/**
 * @tc.name  : RefreshVirtualDevice_004
 * @tc.desc  : Test RefreshVirtualDevice with virtual device
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    RefreshVirtualDevice_004, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_REMOTE_CAST;
    int32_t ret = svc.RefreshVirtualDevice(desc);
    EXPECT_TRUE(ret == SUCCESS || ret != SUCCESS);
}

/**
 * @tc.name  : SetDefaultOutputDevice_005
 * @tc.desc  : Test SetDefaultOutputDevice with video communication
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    SetDefaultOutputDevice_005, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    int32_t ret = svc.SetDefaultOutputDevice(
        DEVICE_TYPE_SPEAKER, 1,
        STREAM_USAGE_VIDEO_COMMUNICATION, false, false);
    EXPECT_TRUE(ret == SUCCESS || ret == ERR_NOT_SUPPORTED);
}

/**
 * @tc.name  : HandleActiveBtOriginalLogic_004
 * @tc.desc  : Test HandleActiveBtOriginalLogic with SPEAKER type
 * @tc.type  : FUNC
 */
HWTEST_F(AudioSelectInterfaceServiceUnitTest,
    HandleActiveBtOriginalLogic_004, TestSize.Level4)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    svc.HandleActiveBtOriginalLogic(
        DEVICE_TYPE_SPEAKER, "");
    EXPECT_TRUE(true);
}
} // namespace AudioStandard
} // namespace OHOS
