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

/* Combined fuzz test suite for all source files under:
 * services/audio_policy/server/infra/datashare/src/
 *
 * Covers 2 source files:
 *   audio_policy_datashare_listener.cpp (4 test functions)
 *   audio_policy_utils.cpp (43 test functions)
 */

#include "securec.h"
#include "audio_log.h"
#include "audio_info.h"
#include "audio_device_info.h"
#include "audio_policy_utils.h"
#include "audio_policy_datashare_listener.h"
#include "audio_router_select_strategy.h"
#include "istandard_audio_policy_manager_listener.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)();

const vector<DeviceType> g_testDeviceTypes = {
    DEVICE_TYPE_NONE, DEVICE_TYPE_INVALID, DEVICE_TYPE_EARPIECE, DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET, DEVICE_TYPE_WIRED_HEADPHONES, DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP, DEVICE_TYPE_BLUETOOTH_A2DP_IN, DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP, DEVICE_TYPE_USB_HEADSET, DEVICE_TYPE_DP, DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_USB_DEVICE, DEVICE_TYPE_ACCESSORY, DEVICE_TYPE_REMOTE_DAUDIO, DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL, DEVICE_TYPE_NEARLINK, DEVICE_TYPE_NEARLINK_IN, DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE, DEVICE_TYPE_EXTERN_CABLE, DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_TYPE_BT_SPP, DEVICE_TYPE_WIRED_MICIN, DEVICE_TYPE_MAX
};

const vector<PreferredType> g_testPreferredTypes = {
    AUDIO_MEDIA_RENDER, AUDIO_CALL_RENDER, AUDIO_CALL_CAPTURE, AUDIO_RING_RENDER,
    AUDIO_RECORD_CAPTURE, AUDIO_TONE_RENDER, AUDIO_RECOGNITION_CAPTURE,
};

const vector<AudioSampleFormat> g_testSampleFormats = {
    SAMPLE_U8, SAMPLE_S16LE, SAMPLE_S24LE, SAMPLE_S32LE, SAMPLE_F32LE,
};

const vector<StreamUsage> g_testStreamUsages = {
    STREAM_USAGE_VOICE_COMMUNICATION, STREAM_USAGE_NOTIFICATION_RINGTONE,
    STREAM_USAGE_VIDEO_COMMUNICATION, STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
    STREAM_USAGE_VOICE_RINGTONE, STREAM_USAGE_VOICE_TRANSFER_COMMUNICATION,
    STREAM_USAGE_INTERPHONE, STREAM_USAGE_MEDIA,
};

const vector<SourceType> g_testSourceTypes = {
    SOURCE_TYPE_VOICE_CALL, SOURCE_TYPE_VOICE_COMMUNICATION, SOURCE_TYPE_INTERPHONE,
    SOURCE_TYPE_MIC, SOURCE_TYPE_INVALID,
};

const vector<AudioPipeType> g_testPipeTypes = {
    PIPE_TYPE_UNKNOWN, PIPE_TYPE_OUT_NORMAL, PIPE_TYPE_OUT_OFFLOAD,
    PIPE_TYPE_OUT_MULTICHANNEL, PIPE_TYPE_OUT_DIRECT_NORMAL,
};

const vector<DeviceInfoUpdateCommand> g_testUpdateCommands = {
    CATEGORY_UPDATE, CONNECTSTATE_UPDATE, ENABLE_UPDATE,
    USAGE_UPDATE, EXCEPTION_FLAG_UPDATE,
};

class PolicyManagerListenerMock : public IStandardAudioPolicyManagerListener {
public:
    sptr<IRemoteObject> AsObject() override { return nullptr; }
    ErrCode OnInterrupt(const InterruptEventInternal &) override { return SUCCESS; }
    ErrCode OnRouteUpdate(uint32_t, const std::string &) override { return SUCCESS; }
    ErrCode OnAvailableDeviceChange(uint32_t, const DeviceChangeAction &) override { return SUCCESS; }
    ErrCode OnQueryClientType(const std::string &, uint32_t, bool &ret) override
    {
        ret = false;
        return SUCCESS;
    }

    ErrCode OnCheckClientInfo(const std::string &, int32_t &, int32_t, bool &ret) override
    {
        ret = false;
        return SUCCESS;
    }

    ErrCode OnCheckMediaControllerBundle(const std::string &, bool &ret) override
    {
        ret = false;
        return SUCCESS;
    }

    ErrCode OnQueryIsForceGetZoneDevice(const std::string &, bool &ret) override
    {
        ret = false;
        return SUCCESS;
    }

    ErrCode OnCheckVKBInfo(const std::string &, bool &isValid) override
    {
        isValid = false;
        return SUCCESS;
    }

    ErrCode OnQueryAllowedPlayback(int32_t, int32_t, bool &ret) override
    {
        ret = false;
        return SUCCESS;
    }

    ErrCode OnBackgroundMute(int32_t) override { return SUCCESS; }
    ErrCode OnQueryBundleNameIsInList(const std::string &, const std::string &, bool &ret) override
    {
        ret = false;
        return SUCCESS;
    }

    ErrCode OnQueryDeviceVolumeBehavior(VolumeBehavior &) override { return SUCCESS; }
    ErrCode OnQueryIsForceGetDevByVolumeType(const std::string &, bool &ret) override
    {
        ret = false;
        return SUCCESS;
    }

    ErrCode OnQueryUseSystemPreferredDeviceForVolume(const std::string &, bool &ret) override
    {
        ret = false;
        return SUCCESS;
    }

    ErrCode StartVoiceBroadcast(uint32_t, const std::string &) override { return SUCCESS; }
};

template <class T>
uint32_t GetArrLength(T &arr)
{
    if (arr == nullptr) { return 0; }
    return sizeof(arr) / sizeof(arr[0]);
}

template <class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (g_dataSize < g_pos) { return object; }
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) { return object; }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) { return {}; }
    g_pos += objectSize;
    return object;
}

uint32_t GetIndex(uint32_t count)
{
    if (count == 0) {
        return 0;
    }
    return GetData<uint32_t>() % count;
}

shared_ptr<AudioDeviceDescriptor> MakeTestDescriptor(DeviceType type, DeviceRole role)
{
    auto desc = make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = type;
    desc->deviceRole_ = role;
    desc->networkId_ = LOCAL_NETWORK_ID;
    desc->macAddress_ = "00:11:22:33:44:55";
    desc->deviceId_ = GetData<int32_t>();
    return desc;
}

void SetMockCallback(AudioPolicyUtils &utils)
{
    sptr<PolicyManagerListenerMock> mock = sptr<PolicyManagerListenerMock>::MakeSptr();
    if (mock != nullptr) {
        utils.audioClientInfoMgrCallback_ = mock;
        utils.queryBundleNameListCallback_ = mock;
    }
}

// ==================== audio_policy_datashare_listener.cpp ====================

void DslRegisterAccessiblilityBalanceFuzzTest()
{
    AudioPolicyDataShareListener::RegisterAccessiblilityBalance();
}

void DslRegisterAccessiblilityMonoFuzzTest()
{
    AudioPolicyDataShareListener::RegisterAccessiblilityMono();
}

void DslRegisterBroadcastFuzzTest()
{
    AudioPolicyDataShareListener::RegisterBroadcast();
}

void DslRegisterVehiclePriorityFuzzTest()
{
    AudioPolicyDataShareListener::RegisterVehiclePriority();
}

// ==================== audio_policy_utils.cpp ====================

void UtilsGetSourcePortNameFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    if (g_testDeviceTypes.size() == 0) { return; }
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    uint32_t routeFlag = GetData<uint32_t>();
    utils.GetSourcePortName(dt, routeFlag);
}

void UtilsWriteServiceStartupErrorFuzzTest()
{
    AudioPolicyUtils::GetInstance().WriteServiceStartupError("fuzz_test_reason");
}

void UtilsGetRemoteNetworkIdFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    std::string sinkName = "testNetwork_out";
    DeviceRole role = GetIndex(2) == 0 ? DeviceRole::OUTPUT_DEVICE : DeviceRole::INPUT_DEVICE;
    utils.GetRemoteNetworkId(sinkName, role);
}

void UtilsSetPreferredDeviceFuzzTest()
{
    if (g_testPreferredTypes.size() == 0 || g_testDeviceTypes.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    SetMockCallback(utils);
    PreferredType pt = g_testPreferredTypes[GetIndex(g_testPreferredTypes.size())];
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    auto desc = MakeTestDescriptor(dt, DeviceRole::OUTPUT_DEVICE);
    int32_t uid = GetData<int32_t>();
    utils.SetPreferredDevice(pt, desc, uid, "fuzz");
}

void UtilsIsSelectedByMediaControllerFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    SetMockCallback(utils);
    utils.IsSelectedByMediaController();
}

void UtilsShouldUseSystemPreferredDeviceForVolumeFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    SetMockCallback(utils);
    utils.ShouldUseSystemPreferredDeviceForVolume("fuzzBundle");
}

void UtilsClearScoDeviceSuspendStateFuzzTest()
{
    AudioPolicyUtils::GetInstance().ClearScoDeviceSuspendState("00:11:22:33:44:55");
}

void UtilsGetCurrentTimeMSFuzzTest()
{
    AudioPolicyUtils::GetInstance().GetCurrentTimeMS();
}

void UtilsPcmFormatToBytesFuzzTest()
{
    if (g_testSampleFormats.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    AudioSampleFormat fmt = g_testSampleFormats[GetIndex(g_testSampleFormats.size())];
    utils.PcmFormatToBytes(fmt);
}

void UtilsIsVoiceStreamTypeFuzzTest()
{
    if (g_testStreamUsages.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    StreamUsage su = g_testStreamUsages[GetIndex(g_testStreamUsages.size())];
    utils.IsVoiceStreamType(su);
}

void UtilsGetNewSinkPortNameFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    utils.GetNewSinkPortName(dt);
}

void UtilsGetSinkPortNameFuzzTest()
{
    if (g_testDeviceTypes.size() == 0 || g_testPipeTypes.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    AudioPipeType pt = g_testPipeTypes[GetIndex(g_testPipeTypes.size())];
    utils.GetSinkPortName(dt, pt);
}

void UtilsConvertToHDIAudioFormatFuzzTest()
{
    if (g_testSampleFormats.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    AudioSampleFormat fmt = g_testSampleFormats[GetIndex(g_testSampleFormats.size())];
    utils.ConvertToHDIAudioFormat(fmt);
}

void UtilsGetSinkNameFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    auto desc = MakeTestDescriptor(DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    int32_t sessionId = GetData<int32_t>();
    utils.GetSinkName(desc, sessionId);
    auto remoteDesc = make_shared<AudioDeviceDescriptor>();
    remoteDesc->networkId_ = "remoteNetworkId";
    remoteDesc->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    utils.GetSinkName(remoteDesc, sessionId);
}

void UtilsGetDeviceClassByPortNameFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    utils.GetOutputDeviceClassBySinkPortName(PRIMARY_SPEAKER);
    utils.GetOutputDeviceClassBySinkPortName(BLUETOOTH_SPEAKER);
    utils.GetOutputDeviceClassBySinkPortName("unknown_port");
    utils.GetInputDeviceClassBySourcePortName(PRIMARY_MIC);
    utils.GetInputDeviceClassBySourcePortName(USB_MIC);
    utils.GetInputDeviceClassBySourcePortName("unknown_source");
}

void UtilsGetDeviceRoleByTypeFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    utils.GetDeviceRole(dt);
}

void UtilsGetDeviceRoleByStringFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    utils.GetDeviceRole(std::string("sink"));
    utils.GetDeviceRole(std::string("source"));
    utils.GetDeviceRole(std::string("unknown"));
}

void UtilsGetDeviceTypeByNameFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    utils.GetDeviceType(std::string("Speaker"));
    utils.GetDeviceType(std::string("Built_in_mic"));
    utils.GetDeviceType(std::string("Built_in_wakeup"));
    utils.GetDeviceType(std::string("fifo_output"));
    utils.GetDeviceType(std::string("file_sink"));
    utils.GetDeviceType(std::string("file_source"));
    utils.GetDeviceType(std::string("unknown_name"));
}

void UtilsIsDataShareReadyFuzzTest()
{
    AudioPolicyUtils::GetInstance().IsDataShareReady();
}

void UtilsSetQueryBundleNameListCallbackFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    sptr<IRemoteObject> obj = nullptr;
    utils.SetQueryBundleNameListCallback(obj);
}

void UtilsIsBundleNameInListFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    SetMockCallback(utils);
    utils.IsBundleNameInList("fuzzBundle", DEVICE_TYPE_NEARLINK);
    utils.IsBundleNameInList("fuzzBundle", DEVICE_TYPE_SPEAKER);
}

void UtilsSetDeclaredDeviceTypesFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    std::string bundleName = "fuzzBundle";
    std::vector<DeviceType> types;
    size_t count = GetData<uint8_t>() % g_testDeviceTypes.size() + 1;
    for (size_t i = 0; i < count; ++i) {
        types.push_back(g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())]);
    }
    utils.SetDeclaredDeviceTypes(bundleName, types);
}

void UtilsIsDeclaredByAppFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    utils.IsDeclaredByApp("fuzzBundle", dt);
}

void UtilsIsSupportedDeviceFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    SetMockCallback(utils);
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    bool hasSysPerm = GetData<bool>();
    utils.IsSupportedDevice("fuzzBundle", dt, hasSysPerm);
}

void UtilsCheckInterphoneFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) { return; }
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    auto desc = MakeTestDescriptor(dt, DeviceRole::OUTPUT_DEVICE);
    desc->deviceCategory_ = static_cast<DeviceCategory>(GetData<int32_t>());
    AudioPolicyUtils::CheckInterphone(STREAM_USAGE_INTERPHONE, desc);
    AudioPolicyUtils::CheckInterphone(STREAM_USAGE_MEDIA, nullptr);
}

void UtilsSetAudioClientInfoMgrCallbackFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    sptr<IStandardAudioPolicyManagerListener> mock = sptr<PolicyManagerListenerMock>::MakeSptr();
    if (mock != nullptr) {
        utils.SetAudioClientInfoMgrCallback(mock);
    }
}

void UtilsEncUsbAddrFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    utils.EncUsbAddr("card=1;2");
    utils.EncUsbAddr("invalid");
}

void UtilsFilterDevicesByTokenIdFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto desc1 = MakeTestDescriptor(DEVICE_TYPE_SPEAKER, DeviceRole::OUTPUT_DEVICE);
    desc1->deviceTokenId_ = GetData<uint32_t>();
    devices.push_back(desc1);
    devices.push_back(nullptr);
    utils.FilterDevicesByTokenId(devices);
}

void UtilsUpdateDevicesListInfoFuzzTest()
{
    if (g_testUpdateCommands.size() == 0 || g_testDeviceTypes.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    auto devDesc = MakeTestDescriptor(dt, DeviceRole::OUTPUT_DEVICE);
    devices.push_back(MakeTestDescriptor(dt, DeviceRole::OUTPUT_DEVICE));
    DeviceInfoUpdateCommand cmd = g_testUpdateCommands[GetIndex(g_testUpdateCommands.size())];
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    utils.UpdateDevicesListInfo(devices, devDesc, cmd, reason);
}

void UtilsUpdateDeviceCategoryFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto existing = MakeTestDescriptor(dt, DeviceRole::OUTPUT_DEVICE);
    existing->deviceCategory_ = BT_UNWEAR_HEADPHONE;
    devices.push_back(existing);
    auto devDesc = MakeTestDescriptor(dt, DeviceRole::OUTPUT_DEVICE);
    devDesc->deviceCategory_ = static_cast<DeviceCategory>(GetData<int32_t>());
    utils.UpdateDeviceCategory(devices, devDesc);
}

void UtilsUpdateConnectStateFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    devices.push_back(MakeTestDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE));
    auto devDesc = MakeTestDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    devDesc->connectState_ = static_cast<ConnectState>(GetData<int32_t>());
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    utils.UpdateConnectState(devices, devDesc, reason);
}

void UtilsUpdateConnectStateScoSuspendFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto a2dpDesc = MakeTestDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE);
    a2dpDesc->connectState_ = CONNECTED;
    devices.push_back(a2dpDesc);
    auto scoDesc = MakeTestDescriptor(DEVICE_TYPE_BLUETOOTH_SCO, DeviceRole::OUTPUT_DEVICE);
    scoDesc->connectState_ = SUSPEND_CONNECTED;
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    utils.UpdateConnectState(devices, scoDesc, reason);
}

void UtilsUpdateEnableStateFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto existing = MakeTestDescriptor(dt, DeviceRole::OUTPUT_DEVICE);
    existing->isEnable_ = false;
    devices.push_back(existing);
    auto devDesc = MakeTestDescriptor(dt, DeviceRole::OUTPUT_DEVICE);
    devDesc->isEnable_ = GetData<bool>();
    utils.UpdateEnableState(devices, devDesc);
}

void UtilsUpdateExceptionFlagFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto existing = MakeTestDescriptor(dt, DeviceRole::OUTPUT_DEVICE);
    existing->exceptionFlag_ = false;
    devices.push_back(existing);
    auto devDesc = MakeTestDescriptor(dt, DeviceRole::OUTPUT_DEVICE);
    devDesc->exceptionFlag_ = GetData<bool>();
    utils.UpdateExceptionFlag(devices, devDesc);
}

void UtilsUpdateDeviceUsageFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto existing = MakeTestDescriptor(dt, DeviceRole::OUTPUT_DEVICE);
    existing->deviceUsage_ = static_cast<DeviceUsage>(0);
    devices.push_back(existing);
    auto devDesc = MakeTestDescriptor(dt, DeviceRole::OUTPUT_DEVICE);
    devDesc->deviceUsage_ = static_cast<DeviceUsage>(GetData<int32_t>());
    AudioStreamDeviceChangeReasonExt reason = AudioStreamDeviceChangeReason::UNKNOWN;
    utils.UpdateDeviceUsage(devices, devDesc, reason);
}

void UtilsTriggerDeviceInfoUpdatedCallbackFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    AudioDeviceDescriptor desc;
    desc.glassesWearCount_ = AudioDeviceDescriptor::WEAR_FIRST;
    desc.deviceCategory_ = BT_UNWEAR_HEADPHONE;
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descForCb;
    utils.TriggerDeviceInfoUpdatedCallback(desc, descForCb);
    desc.glassesWearCount_ = 0;
    desc.deviceCategory_ = CATEGORY_DEFAULT;
    utils.TriggerDeviceInfoUpdatedCallback(desc, descForCb);
}

void UtilsUpdateDeviceConnectTimestampFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    AudioDeviceDescriptor desc;
    desc.deviceCategory_ = BT_GLASSES;
    desc.glassesWearCount_ = AudioDeviceDescriptor::WEAR_FIRST;
    utils.UpdateDeviceConnectTimestamp(desc);
    desc.glassesWearCount_ = AudioDeviceDescriptor::WEAR_AGAIN;
    utils.UpdateDeviceConnectTimestamp(desc);
    desc.deviceCategory_ = CATEGORY_DEFAULT;
    utils.UpdateDeviceConnectTimestamp(desc);
}

void UtilsIsMultiBusDeviceFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) { return; }
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    utils.IsMultiBusDevice(dt);
}

void UtilsConstructRemoteAudioModuleInfoFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    DeviceRole role = GetIndex(2) == 0 ? DeviceRole::OUTPUT_DEVICE : DeviceRole::INPUT_DEVICE;
    DeviceType dt = g_testDeviceTypes[GetIndex(g_testDeviceTypes.size())];
    utils.ConstructRemoteAudioModuleInfo("remoteNetworkId", role, dt);
}

void UtilsGetDeviceNameFromDataShareHelperFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    std::string deviceName;
    utils.GetDeviceNameFromDataShareHelper(deviceName);
}

void UtilsUpdateDisplayNameForRemoteFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    auto desc = make_shared<AudioDeviceDescriptor>();
    desc->networkId_ = "remoteNetworkId";
    utils.UpdateDisplayNameForRemote(desc);
}

void UtilsGetRemoteDeviceNameFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    std::string deviceName;
    utils.GetRemoteDeviceName("remoteDeviceId", deviceName);
}

void UtilsGetDevicesStrFuzzTest()
{
    AudioPolicyUtils &utils = AudioPolicyUtils::GetInstance();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(MakeTestDescriptor(DEVICE_TYPE_BLUETOOTH_A2DP, DeviceRole::OUTPUT_DEVICE));
    descs.push_back(MakeTestDescriptor(DEVICE_TYPE_USB_HEADSET, DeviceRole::OUTPUT_DEVICE));
    descs.push_back(nullptr);
    utils.GetDevicesStr(descs);
}

// ==================== Dispatch Table ====================

TestPtr g_testPtrs[] = {
    // audio_policy_datashare_listener.cpp
    DslRegisterAccessiblilityBalanceFuzzTest,
    DslRegisterAccessiblilityMonoFuzzTest,
    DslRegisterBroadcastFuzzTest,
    DslRegisterVehiclePriorityFuzzTest,
    // audio_policy_utils.cpp
    UtilsGetSourcePortNameFuzzTest,
    UtilsWriteServiceStartupErrorFuzzTest,
    UtilsGetRemoteNetworkIdFuzzTest,
    UtilsSetPreferredDeviceFuzzTest,
    UtilsIsSelectedByMediaControllerFuzzTest,
    UtilsShouldUseSystemPreferredDeviceForVolumeFuzzTest,
    UtilsClearScoDeviceSuspendStateFuzzTest,
    UtilsGetCurrentTimeMSFuzzTest,
    UtilsPcmFormatToBytesFuzzTest,
    UtilsIsVoiceStreamTypeFuzzTest,
    UtilsGetNewSinkPortNameFuzzTest,
    UtilsGetSinkPortNameFuzzTest,
    UtilsConvertToHDIAudioFormatFuzzTest,
    UtilsGetSinkNameFuzzTest,
    UtilsGetDeviceClassByPortNameFuzzTest,
    UtilsGetDeviceRoleByTypeFuzzTest,
    UtilsGetDeviceRoleByStringFuzzTest,
    UtilsGetDeviceTypeByNameFuzzTest,
    UtilsIsDataShareReadyFuzzTest,
    UtilsSetQueryBundleNameListCallbackFuzzTest,
    UtilsIsBundleNameInListFuzzTest,
    UtilsSetDeclaredDeviceTypesFuzzTest,
    UtilsIsDeclaredByAppFuzzTest,
    UtilsIsSupportedDeviceFuzzTest,
    UtilsCheckInterphoneFuzzTest,
    UtilsSetAudioClientInfoMgrCallbackFuzzTest,
    UtilsEncUsbAddrFuzzTest,
    UtilsFilterDevicesByTokenIdFuzzTest,
    UtilsUpdateDevicesListInfoFuzzTest,
    UtilsUpdateDeviceCategoryFuzzTest,
    UtilsUpdateConnectStateFuzzTest,
    UtilsUpdateConnectStateScoSuspendFuzzTest,
    UtilsUpdateEnableStateFuzzTest,
    UtilsUpdateExceptionFlagFuzzTest,
    UtilsUpdateDeviceUsageFuzzTest,
    UtilsTriggerDeviceInfoUpdatedCallbackFuzzTest,
    UtilsUpdateDeviceConnectTimestampFuzzTest,
    UtilsIsMultiBusDeviceFuzzTest,
    UtilsConstructRemoteAudioModuleInfoFuzzTest,
    UtilsGetDeviceNameFromDataShareHelperFuzzTest,
    UtilsUpdateDisplayNameForRemoteFuzzTest,
    UtilsGetRemoteDeviceNameFuzzTest,
    UtilsGetDevicesStrFuzzTest,
};

void FuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr) { return; }
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;
    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testPtrs);
    if (len > 0) {
        g_testPtrs[code % len]();
    }
}

} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) { return 0; }
    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
