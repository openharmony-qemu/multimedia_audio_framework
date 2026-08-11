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

#include <array>
#include <memory>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include "audio_device_info.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "audio_select_interface_service.h"
#include "audio_stream_descriptor.h"
#include "audio_stream_types.h"
#include "standard_audio_policy_manager_listener_stub.h"

namespace OHOS {
namespace AudioStandard {
constexpr size_t THRESHOLD = 10;

namespace {
constexpr size_t MAX_VECTOR_SIZE = 3;
constexpr size_t MAX_RANDOM_STRING_LENGTH = 16;
constexpr int32_t MIN_NORMAL_UID = 2;
constexpr int32_t MAX_NORMAL_UID = 10000;
constexpr uint32_t MIN_NORMAL_STREAM_ID = 1;
constexpr uint32_t MAX_NORMAL_STREAM_ID = 1024;

const std::array<DeviceType, 13> DEVICE_TYPES = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_BT_SPP,
};

const std::array<DeviceRole, 3> DEVICE_ROLES = {
    DEVICE_ROLE_NONE,
    INPUT_DEVICE,
    OUTPUT_DEVICE,
};

const std::array<AudioDeviceUsage, 7> DEVICE_USAGES = {
    MEDIA_OUTPUT_DEVICES,
    MEDIA_INPUT_DEVICES,
    ALL_MEDIA_DEVICES,
    CALL_OUTPUT_DEVICES,
    CALL_INPUT_DEVICES,
    ALL_CALL_DEVICES,
    D_ALL_DEVICES,
};

const std::array<SourceType, 8> SOURCE_TYPES = {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_LIVE,
    SOURCE_TYPE_VOICE_COMMUNICATION,
};

const std::array<StreamUsage, 14> STREAM_USAGES = {
    STREAM_USAGE_INVALID,
    STREAM_USAGE_MEDIA,
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_NOTIFICATION,
    STREAM_USAGE_GAME,
    STREAM_USAGE_VOICE_MESSAGE,
    STREAM_USAGE_VIDEO_COMMUNICATION,
    STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
    STREAM_USAGE_VOICE_CALL_ASSISTANT,
    STREAM_USAGE_INTERPHONE,
    STREAM_USAGE_MAX,
    static_cast<StreamUsage>(-1),
    static_cast<StreamUsage>(999),
};

const std::array<int32_t, 4> AUDIO_DEVICE_SELECT_MODES = {
    0,
    SELECT_STRATEGY_STREAM,
    SELECT_STRATEGY_INDEPENDENT,
    -1,
};

const std::array<int32_t, 3> STREAM_FLAGS = {
    0,
    STREAM_FLAG_FAST,
    -1,
};

const std::array<int32_t, 4> DM_DEVICE_TYPES = {
    0,
    DM_DEVICE_TYPE_MUSIC_HOST,
    static_cast<int32_t>(0xA16),
    static_cast<int32_t>(0x001),
};

class FuzzPolicyManagerListener : public StandardAudioPolicyManagerListenerStub {
public:
    int32_t OnInterrupt(const InterruptEventInternal &interruptEvent) override { return SUCCESS; }
    int32_t OnRouteUpdate(uint32_t routeFlag, const std::string &networkId) override { return SUCCESS; }
    int32_t OnAvailableDeviceChange(uint32_t usage, const DeviceChangeAction &deviceChangeAction) override
    {
        return SUCCESS;
    }
    int32_t OnQueryClientType(const std::string &bundleName, uint32_t uid, bool &ret) override { return SUCCESS; }
    int32_t OnCheckClientInfo(const std::string &bundleName, int32_t &uid, int32_t pid, bool &ret) override
    {
        return SUCCESS;
    }
    int32_t OnCheckMediaControllerBundle(const std::string &bundleName, bool &ret) override { return SUCCESS; }
    int32_t OnQueryIsForceGetZoneDevice(const std::string &bundleName, bool &ret) override { return SUCCESS; }
    int32_t OnCheckVKBInfo(const std::string &bundleName, bool &isValid) override { return SUCCESS; }
    int32_t OnQueryAllowedPlayback(int32_t uid, int32_t pid, bool &ret) override { return SUCCESS; }
    int32_t OnBackgroundMute(const int32_t uid) override { return SUCCESS; }
    int32_t OnQueryBundleNameIsInList(const std::string &bundleName, const std::string &listType, bool &ret) override
    {
        return SUCCESS;
    }
    int32_t OnQueryDeviceVolumeBehavior(VolumeBehavior &volumeBehavior) override { return SUCCESS; }
    int32_t OnQueryIsForceGetDevByVolumeType(const std::string &bundleName, bool &ret) override { return SUCCESS; }
    int32_t OnQueryUseSystemPreferredDeviceForVolume(const std::string &bundleName, bool &ret) override
    {
        return SUCCESS;
    }
    int32_t StartVoiceBroadcast(uint32_t sessionId, const std::string &cellularRecordPhoneNum) override
    {
        return SUCCESS;
    }
};

template<typename T, size_t N>
T Pick(FuzzedDataProvider &fdp, const std::array<T, N> &values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, N - 1)];
}

int32_t PickUid(FuzzedDataProvider &fdp)
{
    const std::array<int32_t, 5> uids = {
        INVALID_UID,
        SYSTEM_UID,
        fdp.ConsumeIntegralInRange<int32_t>(MIN_NORMAL_UID, MAX_NORMAL_UID),
        fdp.ConsumeIntegral<int32_t>(),
        CLEAR_UID,
    };
    return Pick(fdp, uids);
}

uint32_t PickStreamId(FuzzedDataProvider &fdp)
{
    const std::array<uint32_t, 4> streamIds = {
        0,
        INVALID_STREAM_ID,
        fdp.ConsumeIntegralInRange<uint32_t>(MIN_NORMAL_STREAM_ID, MAX_NORMAL_STREAM_ID),
        fdp.ConsumeIntegral<uint32_t>(),
    };
    return Pick(fdp, streamIds);
}

std::string PickString(FuzzedDataProvider &fdp)
{
    return fdp.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH);
}

std::shared_ptr<AudioDeviceDescriptor> MakeDevice(FuzzedDataProvider &fdp)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = Pick(fdp, DEVICE_TYPES);
    desc->deviceRole_ = Pick(fdp, DEVICE_ROLES);
    desc->deviceId_ = fdp.ConsumeIntegral<int32_t>();
    desc->networkId_ = PickString(fdp);
    desc->macAddress_ = PickString(fdp);
    desc->dmDeviceType_ = Pick(fdp, DM_DEVICE_TYPES);
    desc->deviceUsage_ = static_cast<DeviceUsage>(fdp.ConsumeIntegral<int32_t>());
    return desc;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> MakeDeviceList(FuzzedDataProvider &fdp)
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    size_t count = fdp.ConsumeIntegralInRange<size_t>(0, MAX_VECTOR_SIZE);
    for (size_t i = 0; i < count; ++i) {
        devices.push_back(fdp.ConsumeBool() ? nullptr : MakeDevice(fdp));
    }
    return devices;
}

sptr<AudioRendererFilter> MakeRendererFilter(FuzzedDataProvider &fdp)
{
    sptr<AudioRendererFilter> filter = new AudioRendererFilter();
    filter->uid = PickUid(fdp);
    filter->streamId = static_cast<int32_t>(PickStreamId(fdp));
    filter->rendererInfo.streamUsage = Pick(fdp, STREAM_USAGES);
    filter->rendererInfo.rendererFlags = Pick(fdp, STREAM_FLAGS);
    filter->streamType = static_cast<AudioStreamType>(fdp.ConsumeIntegral<int32_t>());
    return filter;
}

sptr<AudioCapturerFilter> MakeCapturerFilter(FuzzedDataProvider &fdp)
{
    sptr<AudioCapturerFilter> filter = new AudioCapturerFilter();
    filter->uid = PickUid(fdp);
    filter->streamId = static_cast<int32_t>(PickStreamId(fdp));
    filter->capturerInfo.sourceType = Pick(fdp, SOURCE_TYPES);
    filter->capturerInfo.capturerFlags = Pick(fdp, STREAM_FLAGS);
    filter->audioDeviceSelectMode = Pick(fdp, AUDIO_DEVICE_SELECT_MODES);
    return filter;
}

// Lines 48-52, 58-90: SetDeviceActive (isPresent lambda + body)
void FuzzSetDeviceActive(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    DeviceType deviceType = Pick(fdp, DEVICE_TYPES);
    bool active = fdp.ConsumeBool();
    std::string address = PickString(fdp);
    int32_t uid = PickUid(fdp);
    svc.SetDeviceActive(deviceType, active, address, uid);
}

// Lines 92-96, 102-106: SetAudioClientInfoMgrCallback + GetPreferredUid with callback
void FuzzSetAudioClientInfoMgrCallback(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    bool useNullCallback = fdp.ConsumeBool();
    if (useNullCallback) {
        sptr<IStandardAudioPolicyManagerListener> callback = nullptr;
        svc.SetAudioClientInfoMgrCallback(callback);
    } else {
        sptr<IStandardAudioPolicyManagerListener> callback = new FuzzPolicyManagerListener();
        svc.SetAudioClientInfoMgrCallback(callback);
    }
    int32_t uid = PickUid(fdp);
    svc.GetPreferredUid(uid);
}

// Lines 115-165: HandleNegtiveBt + HandleNegtiveBtOriginalLogic + HandleNegtiveBtEnhancedRouting
void FuzzHandleNegtiveBt(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    DeviceType deviceType = Pick(fdp, DEVICE_TYPES);
    svc.HandleNegtiveBt(deviceType);
    svc.HandleNegtiveBtOriginalLogic(deviceType);
    svc.HandleNegtiveBtEnhancedRouting(deviceType);
}

// Lines 167-238: HandleActiveBt + HandleActiveBtOriginalLogic + HandleActiveBtEnhancedRouting
void FuzzHandleActiveBt(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    DeviceType deviceType = Pick(fdp, DEVICE_TYPES);
    std::string macAddress = PickString(fdp);
    svc.HandleActiveBt(deviceType, macAddress);
    svc.HandleActiveBtOriginalLogic(deviceType, macAddress);
    svc.HandleActiveBtEnhancedRouting(deviceType, macAddress);
}

// Lines 247-253: NotifyBtForOutputDeviceSelection
void FuzzNotifyBtForOutput(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = fdp.ConsumeBool() ? nullptr : MakeDevice(fdp);
    StreamUsage streamUsage = Pick(fdp, STREAM_USAGES);
    svc.NotifyBtForOutputDeviceSelection(desc, streamUsage);
}

// Lines 255-268: NotifyBtForInputDeviceSelection
void FuzzNotifyBtForInput(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = fdp.ConsumeBool() ? nullptr : MakeDevice(fdp);
    SourceType sourceType = Pick(fdp, SOURCE_TYPES);
    svc.NotifyBtForInputDeviceSelection(desc, sourceType);
}

// Lines 335-342: ConnectVirtualDeviceIfNeeded
void FuzzConnectVirtualDeviceIfNeeded(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = fdp.ConsumeBool() ? nullptr : MakeDevice(fdp);
    svc.ConnectVirtualDeviceIfNeeded(desc);
}

// Lines 486-535: SelectInputDeviceInner
void FuzzSelectInputDeviceInner(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioCapturerFilter> filter = fdp.ConsumeBool() ? nullptr : MakeCapturerFilter(fdp);
    std::shared_ptr<AudioDeviceDescriptor> desc = fdp.ConsumeBool() ? nullptr : MakeDevice(fdp);
    svc.SelectInputDeviceInner(filter, desc);
}

// Lines 544-546, 554-572: SelectInputDevice (log) + RefreshVirtualDevice
void FuzzSelectInputDevice(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    sptr<AudioCapturerFilter> filter = fdp.ConsumeBool() ? nullptr : MakeCapturerFilter(fdp);
    auto devices = MakeDeviceList(fdp);
    svc.SelectInputDevice(filter, devices);

    std::shared_ptr<AudioDeviceDescriptor> desc = fdp.ConsumeBool() ? nullptr : MakeDevice(fdp);
    svc.RefreshVirtualDevice(desc);
}

// Lines 580-587: SelectInputDeviceByUid
void FuzzSelectInputDeviceByUid(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = fdp.ConsumeBool() ? nullptr : MakeDevice(fdp);
    int32_t uid = PickUid(fdp);
    svc.SelectInputDeviceByUid(desc, uid);
}

// Lines 678-686: ClearActiveHfpDevice
void FuzzClearActiveHfpDevice(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    std::shared_ptr<AudioDeviceDescriptor> desc = fdp.ConsumeBool() ? nullptr : MakeDevice(fdp);
    svc.ClearActiveHfpDevice(desc);
}

// Lines 754-794: SetDefaultOutputDevice
void FuzzSetDefaultOutputDevice(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    DeviceType deviceType = Pick(fdp, DEVICE_TYPES);
    uint32_t sessionId = PickStreamId(fdp);
    StreamUsage streamUsage = Pick(fdp, STREAM_USAGES);
    bool isRunning = fdp.ConsumeBool();
    bool skipForce = fdp.ConsumeBool();
    svc.SetDefaultOutputDevice(deviceType, sessionId, streamUsage, isRunning, skipForce);
}

// Lines 802-807: SetInputDevice
void FuzzSetInputDevice(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    DeviceType deviceType = Pick(fdp, DEVICE_TYPES);
    uint32_t sessionId = PickStreamId(fdp);
    int32_t uid = PickUid(fdp);
    svc.SetInputDevice(deviceType, sessionId, uid);
}

// Lines 825-831: SetPreferredInputDeviceIfValid
void FuzzSetPreferredInputDeviceIfValid(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    if (fdp.ConsumeBool()) {
        svc.SetPreferredInputDeviceIfValid(nullptr);
        return;
    }
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->preferredInputDevice.deviceType_ = Pick(fdp, DEVICE_TYPES);
    streamDesc->preferredInputDevice.deviceRole_ = Pick(fdp, DEVICE_ROLES);
    streamDesc->preferredInputDevice.macAddress_ = PickString(fdp);
    streamDesc->capturerInfo_.sourceType = Pick(fdp, SOURCE_TYPES);
    streamDesc->appInfo_.appUid = PickUid(fdp);
    streamDesc->sessionId_ = PickStreamId(fdp);
    svc.SetPreferredInputDeviceIfValid(streamDesc);
}

// Lines 847-849: OnForcedDeviceSelected
void FuzzOnForcedDeviceSelected(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    DeviceType deviceType = Pick(fdp, DEVICE_TYPES);
    std::string macAddress = PickString(fdp);
    sptr<AudioRendererFilter> filter = fdp.ConsumeBool() ? nullptr : MakeRendererFilter(fdp);
    std::string caller = fdp.ConsumeBool() ? "Bluetooth" : PickString(fdp);
    svc.OnForcedDeviceSelected(deviceType, macAddress, filter, caller);
}

// Lines 867-872, 881-883: OnPrivacyDeviceSelected
void FuzzOnPrivacyDeviceSelected(FuzzedDataProvider &fdp)
{
    auto &svc = AudioSelectInterfaceService::GetInstance();
    DeviceType deviceType = Pick(fdp, DEVICE_TYPES);
    std::string macAddress = PickString(fdp);
    std::string caller = fdp.ConsumeBool() ? "Bluetooth" : PickString(fdp);
    svc.OnPrivacyDeviceSelected(deviceType, macAddress, caller);
}

void FuzzTest(FuzzedDataProvider &fdp)
{
    auto func = fdp.PickValueInArray({
        FuzzSetDeviceActive,
        FuzzSetAudioClientInfoMgrCallback,
        FuzzHandleNegtiveBt,
        FuzzHandleActiveBt,
        FuzzNotifyBtForOutput,
        FuzzNotifyBtForInput,
        FuzzConnectVirtualDeviceIfNeeded,
        FuzzSelectInputDeviceInner,
        FuzzSelectInputDevice,
        FuzzSelectInputDeviceByUid,
        //FuzzExcludeOutputDevices,
        FuzzClearActiveHfpDevice,
        FuzzSetDefaultOutputDevice,
        FuzzSetInputDevice,
        FuzzSetPreferredInputDeviceIfValid,
        FuzzOnForcedDeviceSelected,
        FuzzOnPrivacyDeviceSelected,
    });
    func(fdp);
}
} // namespace
} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::FuzzTest(fdp);
    return 0;
}
