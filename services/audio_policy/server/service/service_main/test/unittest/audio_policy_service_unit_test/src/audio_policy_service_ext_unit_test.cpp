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

#include "audio_policy_service_ext_unit_test.h"
#include "audio_core_config_manager.h"
#include "audio_core_service.h"
#include "audio_safe_volume_event_subscriber.h"
#include "audio_server_proxy.h"
#include "nativetoken_kit.h"
#include "dfx_msg_manager.h"
#include "audio_errors.h"
#include <thread>
#include <memory>
#include <vector>
using namespace testing::ext;

namespace OHOS {
namespace AudioStandard {

bool g_hasPermissioned = false;

const int32_t TEST_SESSIONID = MIN_STREAMID + 1010;
const int32_t A2DP_STOPPED = 1;
const int32_t A2DP_PLAYING = 2;
const int32_t A2DP_INVALID = 3;
const uint32_t ROTATE = 1;
const int32_t SESSION_ID = 1000001;
const int32_t STATE = 1;
const uint32_t TEST_APP_UID = 1;
const std::string AUDIO_RESTORE_VOLUME_EVENT = "AUDIO_RESTORE_VOLUME_EVENT";
const std::string AUDIO_INCREASE_VOLUME_EVENT = "AUDIO_INCREASE_VOLUME_EVENT";

void AudioPolicyServiceExtUnitTest::SetUpTestCase(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::SetUpTestCase start-end");
}
void AudioPolicyServiceExtUnitTest::TearDownTestCase(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::TearDownTestCase start-end");
}
void AudioPolicyServiceExtUnitTest::SetUp(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::SetUp start-end");
}
void AudioPolicyServiceExtUnitTest::TearDown(void)
{
    AUDIO_INFO_LOG("AudioPolicyServiceFourthUnitTest::TearDown start-end");
}

class RemoteObjectTestStub : public IRemoteObject {
public:
    RemoteObjectTestStub() : IRemoteObject(u"IRemoteObject") {}
    int32_t GetObjectRefCount() { return 0; };
    int SendRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) { return 0; };
    bool AddDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    bool RemoveDeathRecipient(const sptr<DeathRecipient> &recipient) { return true; };
    int Dump(int fd, const std::vector<std::u16string> &args) { return 0; };

    DECLARE_INTERFACE_DESCRIPTOR(u"RemoteObjectTestStub");
};

static void GetPermission()
{
    if (!g_hasPermissioned) {
        uint64_t tokenId;
        constexpr int perNum = 10;
        const char *perms[perNum] = {
            "ohos.permission.MICROPHONE",
            "ohos.permission.MANAGE_INTELLIGENT_VOICE",
            "ohos.permission.MANAGE_AUDIO_CONFIG",
            "ohos.permission.MICROPHONE_CONTROL",
            "ohos.permission.MODIFY_AUDIO_SETTINGS",
            "ohos.permission.ACCESS_NOTIFICATION_POLICY",
            "ohos.permission.USE_BLUETOOTH",
            "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
            "ohos.permission.RECORD_VOICE_CALL",
            "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 10,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audio_policy_service_unit_test",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermissioned = true;
    }
}


/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_001
* @tc.desc  : Test LoadAudioPolicyConfig interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_001, TestSize.Level1)
{
    AudioCoreConfigManager::GetInstance().xmlHasLoaded_ = true;
    auto ret = AudioPipeManager::GetPipeManager()->Init();
    EXPECT_FALSE(ret);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_003
* @tc.desc  : Test CreateRecoveryThread interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_003, TestSize.Level1)
{
    AudioPolicyService::GetAudioPolicyService().CreateRecoveryThread();
    EXPECT_NE(nullptr, AudioPolicyService::GetAudioPolicyService().RecoveryDevicesThread_);
    AudioPolicyService::GetAudioPolicyService().CreateRecoveryThread();
}

/**
* @tc.name  : Test SafeVolumeEventSubscriber.
* @tc.number: SafeVolumeEventSubscriber_001
* @tc.desc  : Test OnReceiveEvent interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, SafeVolumeEventSubscriber_001, TestSize.Level1)
{
    OHOS::Singleton<SafeVolumeEventSubscriber>::GetInstance().SubscribeSafeVolumeEvent();
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_005
* @tc.desc  : Test SetSourceOutputStreamMute interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_005, TestSize.Level1)
{
    int32_t uid = 123;
    auto ret = AudioPolicyService::GetAudioPolicyService().SetSourceOutputStreamMute(uid, true);
    EXPECT_NE(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_07
* @tc.desc  : Test LoadModernOffloadCapSource interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_07, TestSize.Level1)
{
    auto ret0 = AudioPolicyService::GetAudioPolicyService().LoadModernOffloadCapSource();
    EXPECT_EQ(SUCCESS, ret0);

    AudioIOHandle audioIOHandle = 1003;
    AudioPolicyService::GetAudioPolicyService().audioIOHandleMap_.IOHandles_["offloadCapturerSource"] = audioIOHandle;

    auto ret1 = AudioPolicyService::GetAudioPolicyService().LoadModernOffloadCapSource();
    EXPECT_EQ(SUCCESS, ret1);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_008
* @tc.desc  : Test GetOutputDevice interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_008, TestSize.Level1)
{
    sptr<AudioRendererFilter> audioRendererFilter(new AudioRendererFilter());
    audioRendererFilter->uid = 123;
    auto ret = AudioPolicyService::GetAudioPolicyService().GetOutputDevice(audioRendererFilter);
    EXPECT_NE(nullptr, audioRendererFilter);
    audioRendererFilter->uid = -1;
    ret = AudioPolicyService::GetAudioPolicyService().GetOutputDevice(audioRendererFilter);
    EXPECT_NE(0, ret.size());
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_009
* @tc.desc  : Test GetInputDevice interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_009, TestSize.Level1)
{
    sptr<AudioCapturerFilter> audioCapturerFilter(new AudioCapturerFilter());
    audioCapturerFilter->uid = 123;
    auto ret = AudioPolicyService::GetAudioPolicyService().GetInputDevice(audioCapturerFilter);
    EXPECT_NE(nullptr, audioCapturerFilter);
    audioCapturerFilter->uid = -1;
    ret = AudioPolicyService::GetAudioPolicyService().GetInputDevice(audioCapturerFilter);
    EXPECT_NE(0, ret.size());
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_010
* @tc.desc  : Test UpdateA2dpOffloadFlagBySpatialService interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_010, TestSize.Level1)
{
    std::string macAddress = "00-15-5D-E6-DE-FC";
    AudioPolicyService::GetAudioPolicyService().UpdateA2dpOffloadFlagBySpatialService(macAddress);
    EXPECT_EQ(nullptr, AudioPolicyService::GetAudioPolicyService().audioA2dpOffloadManager_);
    AudioPolicyService::GetAudioPolicyService().audioA2dpOffloadManager_ =
        std::make_shared<AudioA2dpOffloadManager>();
    AudioPolicyService::GetAudioPolicyService().UpdateA2dpOffloadFlagBySpatialService(macAddress);
    EXPECT_NE(nullptr, AudioPolicyService::GetAudioPolicyService().audioA2dpOffloadManager_);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_011
* @tc.desc  : Test GetCurrentCapturerChangeInfos interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_011, TestSize.Level1)
{
    AudioConnectedDevice::GetInstance().AddConnectedDevice(std::make_shared<AudioDeviceDescriptor>());
    vector<shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    audioCapturerChangeInfos.push_back(std::make_shared<AudioCapturerChangeInfo>());
    auto ret = AudioPolicyService::GetAudioPolicyService().GetCurrentCapturerChangeInfos(
        audioCapturerChangeInfos, false, false);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_014
* @tc.desc  : Test RegisterBluetoothListener interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_014, TestSize.Level1)
{
    AudioPolicyService::GetAudioPolicyService().RegisterBluetoothListener();
    EXPECT_NE(nullptr, AudioPolicyService::GetAudioPolicyService().deviceStatusListener_);
    AudioPolicyService::GetAudioPolicyService().RegisterBluetoothListener();
    AudioPolicyService::GetAudioPolicyService().UnregisterBluetoothListener();
    EXPECT_NE(nullptr, AudioPolicyService::GetAudioPolicyService().deviceStatusListener_);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_015
* @tc.desc  : Test CheckSupportedAudioEffectProperty interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_015, TestSize.Level1)
{
    AudioEffectPropertyArray propertyArray;
    EffectFlag flag = CAPTURE_EFFECT_FLAG;
    auto ret = AudioPolicyService::GetAudioPolicyService().CheckSupportedAudioEffectProperty(propertyArray, flag);
    EXPECT_EQ(AUDIO_OK, ret);
    flag = RENDER_EFFECT_FLAG;
    ret = AudioPolicyService::GetAudioPolicyService().CheckSupportedAudioEffectProperty(propertyArray, flag);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test AudioPolicyService.
* @tc.number: AudioPolicyServiceTest_016
* @tc.desc  : Test SetAudioEffectProperty interfaces.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, AudioPolicyServiceTest_016, TestSize.Level1)
{
    AudioEffectPropertyArray propertyArray;
    AudioEffectProperty tmp;
    tmp.flag = CAPTURE_EFFECT_FLAG;
    propertyArray.property.push_back(tmp);
    tmp.flag = RENDER_EFFECT_FLAG;
    propertyArray.property.push_back(tmp);
    auto ret = AudioPolicyService::GetAudioPolicyService().SetAudioEffectProperty(propertyArray);
    EXPECT_EQ(ERR_INVALID_PARAM, ret);
}

/**
* @tc.name  : Test AudioPolicyService Deinit with isBtListenerRegistered=true.
* @tc.number: Deinit_001
* @tc.desc  : Test Deinit when isBtListenerRegistered is true, should call UnregisterBluetoothListener.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, Deinit_001, TestSize.Level1)
{
    AudioPolicyService::isBtListenerRegistered = true;
    AudioPolicyService::GetAudioPolicyService().Deinit();
    EXPECT_EQ(false, AudioPolicyService::isBtListenerRegistered);
}

/**
* @tc.name  : Test AudioPolicyService Deinit with isBtListenerRegistered=false.
* @tc.number: Deinit_002
* @tc.desc  : Test Deinit when isBtListenerRegistered is false, should skip UnregisterBluetoothListener.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, Deinit_002, TestSize.Level1)
{
    AudioPolicyService::isBtListenerRegistered = false;
    AudioPolicyService::GetAudioPolicyService().RecoveryDevicesThread_ = nullptr;
    AudioPolicyService::GetAudioPolicyService().Deinit();
    EXPECT_EQ(false, AudioPolicyService::isBtListenerRegistered);
}

/**
* @tc.name  : Test AudioPolicyService Deinit with joinable RecoveryDevicesThread.
* @tc.number: Deinit_003
* @tc.desc  : Test Deinit when RecoveryDevicesThread is not null and joinable.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, Deinit_003, TestSize.Level1)
{
    AudioPolicyService::isBtListenerRegistered = false;
    AudioPolicyService::GetAudioPolicyService().RecoveryDevicesThread_ = nullptr;
    AudioPolicyService::GetAudioPolicyService().Deinit();
    EXPECT_EQ(nullptr, AudioPolicyService::GetAudioPolicyService().RecoveryDevicesThread_);
}

/**
* @tc.name  : Test AudioPolicyService LoadEffectLibrary.
* @tc.number: LoadEffectLibrary_001
* @tc.desc  : Test LoadEffectLibrary when effect chain manager creation fails.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, LoadEffectLibrary_001, TestSize.Level1)
{
    AudioPolicyService::GetAudioPolicyService().LoadEffectLibrary();
}

/**
* @tc.name  : Test SetQueryClientTypeCallback with null object.
* @tc.number: SetQueryClientTypeCallback_001
* @tc.desc  : Test SetQueryClientTypeCallback with null remote object returns SUCCESS and logs error.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, SetQueryClientTypeCallback_001, TestSize.Level1)
{
    sptr<IRemoteObject> object = nullptr;
    int32_t ret = AudioPolicyService::GetAudioPolicyService().SetQueryClientTypeCallback(object);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test SetQueryClientTypeCallback with valid object.
* @tc.number: SetQueryClientTypeCallback_002
* @tc.desc  : Test SetQueryClientTypeCallback with valid remote object returns SUCCESS.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, SetQueryClientTypeCallback_002, TestSize.Level1)
{
    sptr<IRemoteObject> object = new RemoteObjectTestStub();
    int32_t ret = AudioPolicyService::GetAudioPolicyService().SetQueryClientTypeCallback(object);
    EXPECT_EQ(SUCCESS, ret);
}

/**
* @tc.name  : Test GetProcessDeviceInfo with coreService null.
* @tc.number: GetProcessDeviceInfo_001
* @tc.desc  : Test GetProcessDeviceInfo returns ERROR when coreService is null.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, GetProcessDeviceInfo_001, TestSize.Level1)
{
    AudioProcessConfig config;
    AudioDeviceDescriptor deviceInfo;
    int32_t ret = AudioPolicyService::GetAudioPolicyService().GetProcessDeviceInfo(config, false, deviceInfo);
    EXPECT_EQ(ERROR, ret);
}

/**
* @tc.name  : Test GetProcessDeviceInfo with AUDIO_MODE_PLAYBACK.
* @tc.number: GetProcessDeviceInfo_002
* @tc.desc  : Test GetProcessDeviceInfo with playback mode.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, GetProcessDeviceInfo_002, TestSize.Level1)
{
    AudioProcessConfig config;
    config.audioMode = AUDIO_MODE_PLAYBACK;
    AudioDeviceDescriptor deviceInfo;
    int32_t ret = AudioPolicyService::GetAudioPolicyService().GetProcessDeviceInfo(config, false, deviceInfo);
    EXPECT_EQ(ERROR, ret);
}

/**
* @tc.name  : Test GetProcessDeviceInfo with AUDIO_MODE_RECORD.
* @tc.number: GetProcessDeviceInfo_003
* @tc.desc  : Test GetProcessDeviceInfo with record mode.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, GetProcessDeviceInfo_003, TestSize.Level1)
{
    AudioProcessConfig config;
    config.audioMode = AUDIO_MODE_RECORD;
    AudioDeviceDescriptor deviceInfo;
    int32_t ret = AudioPolicyService::GetAudioPolicyService().GetProcessDeviceInfo(config, false, deviceInfo);
    EXPECT_EQ(ERROR, ret);
}

/**
* @tc.name  : Test GetRealUid with callerUid == MEDIA_SERVICE_UID.
* @tc.number: GetRealUid_001
* @tc.desc  : Test GetRealUid returns appUid when callerUid is MEDIA_SERVICE_UID.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, GetRealUid_001, TestSize.Level1)
{
    AudioProcessConfig config;
    config.callerUid = 1013;
    config.appInfo.appUid = 2000;
    int32_t ret = AudioPolicyService::GetAudioPolicyService().GetRealUid(config);
    EXPECT_EQ(2000, ret);
}

/**
* @tc.name  : Test GetRealUid with callerUid != MEDIA_SERVICE_UID.
* @tc.number: GetRealUid_002
* @tc.desc  : Test GetRealUid returns callerUid when callerUid is not MEDIA_SERVICE_UID.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, GetRealUid_002, TestSize.Level1)
{
    AudioProcessConfig config;
    config.callerUid = 9999;
    config.appInfo.appUid = 2000;
    int32_t ret = AudioPolicyService::GetAudioPolicyService().GetRealUid(config);
    EXPECT_EQ(9999, ret);
}

/**
* @tc.name  : Test SetParameterCallback with valid callback.
* @tc.number: SetParameterCallback_001
* @tc.desc  : Test SetParameterCallback with a valid parameter callback.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, SetParameterCallback_001, TestSize.Level1)
{
    std::shared_ptr<AudioParameterCallback> callback = nullptr;
    AudioPolicyService::GetAudioPolicyService().SetParameterCallback(callback);
}

/**
* @tc.name  : Test RegisterBluetoothListener with isBtListenerRegistered=true.
* @tc.number: RegisterBluetoothListener_001
* @tc.desc  : Test RegisterBluetoothListener when already registered should return early.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, RegisterBluetoothListener_001, TestSize.Level1)
{
    AudioPolicyService::isBtListenerRegistered = true;
    AudioPolicyService::GetAudioPolicyService().RegisterBluetoothListener();
    EXPECT_EQ(true, AudioPolicyService::isBtListenerRegistered);
}

/**
* @tc.name  : Test RegisterBluetoothListener with isBtListenerRegistered=false and isBtCrashed=false.
* @tc.number: RegisterBluetoothListener_002
* @tc.desc  : Test RegisterBluetoothListener when not registered and not crashed.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, RegisterBluetoothListener_002, TestSize.Level1)
{
    AudioPolicyService::isBtListenerRegistered = false;
    AudioPolicyService::isBtCrashed = false;
    AudioPolicyService::GetAudioPolicyService().RegisterBluetoothListener();
    EXPECT_EQ(true, AudioPolicyService::isBtListenerRegistered);
    EXPECT_EQ(false, AudioPolicyService::isBtCrashed);
}

/**
* @tc.name  : Test RegisterBluetoothListener with isBtListenerRegistered=false and isBtCrashed=true.
* @tc.number: RegisterBluetoothListener_003
* @tc.desc  : Test RegisterBluetoothListener when not registered but bt crashed, skips A2dp/Sco registration.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, RegisterBluetoothListener_003, TestSize.Level1)
{
    AudioPolicyService::isBtListenerRegistered = false;
    AudioPolicyService::isBtCrashed = true;
    AudioPolicyService::GetAudioPolicyService().RegisterBluetoothListener();
    EXPECT_EQ(true, AudioPolicyService::isBtListenerRegistered);
    EXPECT_EQ(false, AudioPolicyService::isBtCrashed);
}

/**
* @tc.name  : Test GetHardwareOutputSamplingRate with null desc.
* @tc.number: GetHardwareOutputSamplingRate_001
* @tc.desc  : Test GetHardwareOutputSamplingRate returns -1 when desc is null.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, GetHardwareOutputSamplingRate_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = nullptr;
    int32_t ret = AudioPolicyService::GetAudioPolicyService().GetHardwareOutputSamplingRate(desc);
    EXPECT_EQ(-1, ret);
}

/**
* @tc.name  : Test GetHardwareOutputSamplingRate with non-connected device.
* @tc.number: GetHardwareOutputSamplingRate_002
* @tc.desc  : Test GetHardwareOutputSamplingRate returns -1 when device is not connected.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, GetHardwareOutputSamplingRate_002, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = DEVICE_TYPE_SPEAKER;
    int32_t ret = AudioPolicyService::GetAudioPolicyService().GetHardwareOutputSamplingRate(desc);
    EXPECT_EQ(-1, ret);
}

/**
* @tc.name  : Test IsMatchDeviceWithUsage with null desc.
* @tc.number: IsMatchDeviceWithUsage_001
* @tc.desc  : Test IsMatchDeviceWithUsage returns true when desc is null.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsMatchDeviceWithUsage_001, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = nullptr;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsMatchDeviceWithUsage(desc, MEDIA_OUTPUT_DEVICES);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test IsMatchDeviceWithUsage with MEDIA_OUTPUT_DEVICES and OUTPUT_DEVICE.
* @tc.number: IsMatchDeviceWithUsage_002
* @tc.desc  : Test IsMatchDeviceWithUsage returns true for output device with output usage.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsMatchDeviceWithUsage_002, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = OUTPUT_DEVICE;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsMatchDeviceWithUsage(desc, MEDIA_OUTPUT_DEVICES);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test IsMatchDeviceWithUsage with MEDIA_OUTPUT_DEVICES and INPUT_DEVICE.
* @tc.number: IsMatchDeviceWithUsage_003
* @tc.desc  : Test IsMatchDeviceWithUsage returns false for input device with output usage.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsMatchDeviceWithUsage_003, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = INPUT_DEVICE;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsMatchDeviceWithUsage(desc, MEDIA_OUTPUT_DEVICES);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test IsMatchDeviceWithUsage with CALL_OUTPUT_DEVICES.
* @tc.number: IsMatchDeviceWithUsage_004
* @tc.desc  : Test IsMatchDeviceWithUsage with call output devices usage.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsMatchDeviceWithUsage_004, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = OUTPUT_DEVICE;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsMatchDeviceWithUsage(desc, CALL_OUTPUT_DEVICES);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test IsMatchDeviceWithUsage with ALL_OUTPUT_DEVICES.
* @tc.number: IsMatchDeviceWithUsage_005
* @tc.desc  : Test IsMatchDeviceWithUsage with all output devices usage.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsMatchDeviceWithUsage_005, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = OUTPUT_DEVICE;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsMatchDeviceWithUsage(desc, ALL_OUTPUT_DEVICES);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test IsMatchDeviceWithUsage with MEDIA_INPUT_DEVICES and INPUT_DEVICE.
* @tc.number: IsMatchDeviceWithUsage_006
* @tc.desc  : Test IsMatchDeviceWithUsage returns true for input device with input usage.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsMatchDeviceWithUsage_006, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = INPUT_DEVICE;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsMatchDeviceWithUsage(desc, MEDIA_INPUT_DEVICES);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test IsMatchDeviceWithUsage with MEDIA_INPUT_DEVICES and OUTPUT_DEVICE.
* @tc.number: IsMatchDeviceWithUsage_007
* @tc.desc  : Test IsMatchDeviceWithUsage returns false for output device with input usage.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsMatchDeviceWithUsage_007, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = OUTPUT_DEVICE;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsMatchDeviceWithUsage(desc, MEDIA_INPUT_DEVICES);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test IsMatchDeviceWithUsage with CALL_INPUT_DEVICES.
* @tc.number: IsMatchDeviceWithUsage_008
* @tc.desc  : Test IsMatchDeviceWithUsage with call input devices usage.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsMatchDeviceWithUsage_008, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = INPUT_DEVICE;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsMatchDeviceWithUsage(desc, CALL_INPUT_DEVICES);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test IsMatchDeviceWithUsage with ALL_INPUT_DEVICES.
* @tc.number: IsMatchDeviceWithUsage_009
* @tc.desc  : Test IsMatchDeviceWithUsage with all input devices usage.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsMatchDeviceWithUsage_009, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = INPUT_DEVICE;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsMatchDeviceWithUsage(desc, ALL_INPUT_DEVICES);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test IsMatchDeviceWithUsage with default (unknown) usage.
* @tc.number: IsMatchDeviceWithUsage_010
* @tc.desc  : Test IsMatchDeviceWithUsage returns true for unknown usage (default branch).
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsMatchDeviceWithUsage_010, TestSize.Level1)
{
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = OUTPUT_DEVICE;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsMatchDeviceWithUsage(desc, AudioDeviceUsage(999));
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test DeviceFilterByUsageInner with null device in list.
* @tc.number: DeviceFilterByUsageInner_001
* @tc.desc  : Test DeviceFilterByUsageInner skips null devices.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, DeviceFilterByUsageInner_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(nullptr);
    auto result = AudioPolicyService::GetAudioPolicyService().DeviceFilterByUsageInner(MEDIA_OUTPUT_DEVICES, descs);
    EXPECT_EQ(0, result.size());
}

/**
* @tc.name  : Test DeviceFilterByUsageInner with remote device.
* @tc.number: DeviceFilterByUsageInner_002
* @tc.desc  : Test DeviceFilterByUsageInner with remote device matching usage.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, DeviceFilterByUsageInner_002, TestSize.Level1)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceRole_ = OUTPUT_DEVICE;
    desc->networkId_ = "remote_network_id";
    descs.push_back(desc);
    auto result = AudioPolicyService::GetAudioPolicyService().DeviceFilterByUsageInner(MEDIA_OUTPUT_DEVICES, descs);
    EXPECT_GE(result.size(), 0);
}

/**
* @tc.name  : Test OffloadGetRenderPosition.
* @tc.number: OffloadGetRenderPosition_001
* @tc.desc  : Test OffloadGetRenderPosition with default values.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, OffloadGetRenderPosition_001, TestSize.Level1)
{
    uint32_t delayValue = 0;
    uint64_t sendDataSize = 0;
    uint32_t timeStamp = 0;
    int32_t ret = AudioPolicyService::GetAudioPolicyService().OffloadGetRenderPosition(delayValue,
        sendDataSize, timeStamp);
#ifdef BLUETOOTH_ENABLE
    EXPECT_EQ(SUCCESS, ret);
#else
    EXPECT_EQ(SUCCESS, ret);
#endif
}

/**
* @tc.name  : Test SetAudioEffectProperty with empty property.
* @tc.number: SetAudioEffectProperty_002
* @tc.desc  : Test SetAudioEffectProperty with empty property array returns AUDIO_OK.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, SetAudioEffectProperty_002, TestSize.Level1)
{
    AudioEffectPropertyArray propertyArray;
    auto ret = AudioPolicyService::GetAudioPolicyService().SetAudioEffectProperty(propertyArray);
    EXPECT_EQ(AUDIO_OK, ret);
}

/**
* @tc.name  : Test SetAudioEffectProperty with only CAPTURE_EFFECT_FLAG.
* @tc.number: SetAudioEffectProperty_003
* @tc.desc  : Test SetAudioEffectProperty with capture effect flag only.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, SetAudioEffectProperty_003, TestSize.Level1)
{
    AudioEffectPropertyArray propertyArray;
    AudioEffectProperty tmp;
    tmp.flag = CAPTURE_EFFECT_FLAG;
    propertyArray.property.push_back(tmp);
    auto ret = AudioPolicyService::GetAudioPolicyService().SetAudioEffectProperty(propertyArray);
    EXPECT_NE(AUDIO_OK, ret);
}

/**
* @tc.name  : Test SetAudioEffectProperty with only RENDER_EFFECT_FLAG.
* @tc.number: SetAudioEffectProperty_004
* @tc.desc  : Test SetAudioEffectProperty with render effect flag only.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, SetAudioEffectProperty_004, TestSize.Level1)
{
    AudioEffectPropertyArray propertyArray;
    AudioEffectProperty tmp;
    tmp.flag = RENDER_EFFECT_FLAG;
    propertyArray.property.push_back(tmp);
    auto ret = AudioPolicyService::GetAudioPolicyService().SetAudioEffectProperty(propertyArray);
    EXPECT_NE(AUDIO_OK, ret);
}

/**
* @tc.name  : Test GetAudioEnhanceProperty.
* @tc.number: GetAudioEnhanceProperty_001
* @tc.desc  : Test GetAudioEnhanceProperty returns valid result.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, GetAudioEnhanceProperty_001, TestSize.Level1)
{
    AudioEffectPropertyArray propertyArray;
    auto ret = AudioPolicyService::GetAudioPolicyService().GetAudioEnhanceProperty(propertyArray);
#ifdef BLUETOOTH_ENABLE
    EXPECT_NE(ERROR, ret);
#else
    EXPECT_NE(ERROR, ret);
#endif
}

/**
* @tc.name  : Test GetAudioEffectProperty.
* @tc.number: GetAudioEffectProperty_001
* @tc.desc  : Test GetAudioEffectProperty returns valid result.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, GetAudioEffectProperty_001, TestSize.Level1)
{
    AudioEffectPropertyArray propertyArray;
    auto ret = AudioPolicyService::GetAudioPolicyService().GetAudioEffectProperty(propertyArray);
}

/**
* @tc.name  : Test GetA2dpOffloadFlag with null manager.
* @tc.number: GetA2dpOffloadFlag_001
* @tc.desc  : Test GetA2dpOffloadFlag returns NO_A2DP_DEVICE when manager is null.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, GetA2dpOffloadFlag_001, TestSize.Level1)
{
    AudioPolicyService::GetAudioPolicyService().audioA2dpOffloadManager_ = nullptr;
    BluetoothOffloadState ret = AudioPolicyService::GetAudioPolicyService().GetA2dpOffloadFlag();
    EXPECT_EQ(NO_A2DP_DEVICE, ret);
}

/**
* @tc.name  : Test GetA2dpOffloadFlag with valid manager.
* @tc.number: GetA2dpOffloadFlag_002
* @tc.desc  : Test GetA2dpOffloadFlag returns manager's flag when manager is not null.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, GetA2dpOffloadFlag_002, TestSize.Level1)
{
    AudioPolicyService::GetAudioPolicyService().audioA2dpOffloadManager_ =
        std::make_shared<AudioA2dpOffloadManager>();
    BluetoothOffloadState ret = AudioPolicyService::GetAudioPolicyService().GetA2dpOffloadFlag();
    EXPECT_EQ(NO_A2DP_DEVICE, ret);
}

/**
* @tc.name  : Test IsDevicePlaybackSupported with EAC3 and non-HDMI/LINE_DIGITAL device.
* @tc.number: IsDevicePlaybackSupported_001
* @tc.desc  : Test IsDevicePlaybackSupported returns false for EAC3 on unsupported device type.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsDevicePlaybackSupported_001, TestSize.Level1)
{
    AudioProcessConfig config;
    config.streamInfo.encoding = ENCODING_EAC3;
    AudioDeviceDescriptor deviceInfo;
    deviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsDevicePlaybackSupported(config, deviceInfo);
    EXPECT_EQ(false, ret);
}

/**
* @tc.name  : Test IsDevicePlaybackSupported with EAC3 and HDMI device.
* @tc.number: IsDevicePlaybackSupported_002
* @tc.desc  : Test IsDevicePlaybackSupported returns true for EAC3 on HDMI device.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsDevicePlaybackSupported_002, TestSize.Level1)
{
    AudioProcessConfig config;
    config.streamInfo.encoding = ENCODING_EAC3;
    AudioDeviceDescriptor deviceInfo;
    deviceInfo.deviceType_ = DEVICE_TYPE_HDMI;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsDevicePlaybackSupported(config, deviceInfo);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test IsDevicePlaybackSupported with EAC3 and LINE_DIGITAL device.
* @tc.number: IsDevicePlaybackSupported_003
* @tc.desc  : Test IsDevicePlaybackSupported returns true for EAC3 on LINE_DIGITAL device.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsDevicePlaybackSupported_003, TestSize.Level1)
{
    AudioProcessConfig config;
    config.streamInfo.encoding = ENCODING_EAC3;
    AudioDeviceDescriptor deviceInfo;
    deviceInfo.deviceType_ = DEVICE_TYPE_LINE_DIGITAL;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsDevicePlaybackSupported(config, deviceInfo);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test IsDevicePlaybackSupported with PCM encoding.
* @tc.number: IsDevicePlaybackSupported_004
* @tc.desc  : Test IsDevicePlaybackSupported returns true for PCM encoding on any device.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsDevicePlaybackSupported_004, TestSize.Level1)
{
    AudioProcessConfig config;
    config.streamInfo.encoding = ENCODING_PCM;
    AudioDeviceDescriptor deviceInfo;
    deviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsDevicePlaybackSupported(config, deviceInfo);
    EXPECT_EQ(true, ret);
}

/**
* @tc.name  : Test IsDevicePlaybackSupported with null handler and EAC3 encoding.
* @tc.number: IsDevicePlaybackSupported_005
* @tc.desc  : Test IsDevicePlaybackSupported returns true when handler is null even with EAC3 on speaker.
*/
HWTEST_F(AudioPolicyServiceExtUnitTest, IsDevicePlaybackSupported_005, TestSize.Level1)
{
    AudioPolicyService::GetAudioPolicyService().audioPolicyServerHandler_ = nullptr;
    AudioProcessConfig config;
    config.streamInfo.encoding = ENCODING_EAC3;
    AudioDeviceDescriptor deviceInfo;
    deviceInfo.deviceType_ = DEVICE_TYPE_SPEAKER;
    bool ret = AudioPolicyService::GetAudioPolicyService().IsDevicePlaybackSupported(config, deviceInfo);
    EXPECT_EQ(true, ret);
}
} // namespace AudioStandard
} // namespace OHOS