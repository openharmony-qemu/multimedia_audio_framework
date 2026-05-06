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

#include "audio_select_interface_service.h"
#include "audio_stream_types.h"

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

const std::array<DeviceType, 10> DEVICE_TYPES = {
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
    SOURCE_TYPE_MAX,
};

const std::array<StreamUsage, 9> STREAM_USAGES = {
    STREAM_USAGE_INVALID,
    STREAM_USAGE_MEDIA,
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_NOTIFICATION,
    STREAM_USAGE_GAME,
    STREAM_USAGE_VOICE_MESSAGE,
    STREAM_USAGE_VIDEO_COMMUNICATION,
    STREAM_USAGE_MAX,
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

std::shared_ptr<AudioDeviceDescriptor> MakeDevice(FuzzedDataProvider &fdp)
{
    auto desc = std::make_shared<AudioDeviceDescriptor>();
    desc->deviceType_ = Pick(fdp, DEVICE_TYPES);
    desc->deviceRole_ = Pick(fdp, DEVICE_ROLES);
    desc->deviceId_ = fdp.ConsumeIntegral<int32_t>();
    desc->networkId_ = fdp.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH);
    desc->macAddress_ = fdp.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH);
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

void FuzzSetDeviceActive(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    DeviceType deviceType = Pick(fdp, DEVICE_TYPES);
    bool active = fdp.ConsumeBool();
    std::string address = fdp.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH);
    int32_t uid = PickUid(fdp);

    service.SetDeviceActive(deviceType, active, address, uid);
}

void FuzzSelectOutputDevice(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    sptr<AudioRendererFilter> filter = MakeRendererFilter(fdp);
    auto devices = MakeDeviceList(fdp);
    int32_t selectMode = Pick(fdp, AUDIO_DEVICE_SELECT_MODES);
    bool isNeedNotifyBt = fdp.ConsumeBool();

    service.SelectOutputDevice(filter, devices, selectMode, isNeedNotifyBt);
}

void FuzzSetMediaOutputDeviceByUid(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    DeviceType deviceType = Pick(fdp, DEVICE_TYPES);
    int32_t uid = PickUid(fdp);

    service.SetMediaOutputDeviceByUid(deviceType, uid);
}

void FuzzSelectInputDevice(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    sptr<AudioCapturerFilter> filter = MakeCapturerFilter(fdp);
    auto devices = MakeDeviceList(fdp);

    service.SelectInputDevice(filter, devices);
}

void FuzzSelectInputDeviceByUid(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    auto device = MakeDevice(fdp);
    int32_t uid = PickUid(fdp);

    service.SelectInputDeviceByUid(device, uid);
}

void FuzzExcludeOutputDevices(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    AudioDeviceUsage usage = Pick(fdp, DEVICE_USAGES);
    auto devices = MakeDeviceList(fdp);

    service.ExcludeOutputDevices(usage, devices);
}

void FuzzUnexcludeOutputDevices(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    AudioDeviceUsage usage = Pick(fdp, DEVICE_USAGES);
    auto devices = MakeDeviceList(fdp);

    service.UnexcludeOutputDevices(usage, devices);
}

void FuzzSetSessionDefaultOutputDevice(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    int32_t callerPid = fdp.ConsumeIntegral<int32_t>();
    DeviceType deviceType = Pick(fdp, DEVICE_TYPES);

    service.SetSessionDefaultOutputDevice(callerPid, deviceType);
}

void FuzzSetDefaultOutputDevice(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    DeviceType deviceType = Pick(fdp, DEVICE_TYPES);
    uint32_t sessionId = PickStreamId(fdp);
    StreamUsage streamUsage = Pick(fdp, STREAM_USAGES);
    bool isRunning = fdp.ConsumeBool();
    bool skipForce = fdp.ConsumeBool();

    service.SetDefaultOutputDevice(deviceType, sessionId, streamUsage, isRunning, skipForce);
}

void FuzzSetInputDevice(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    DeviceType deviceType = Pick(fdp, DEVICE_TYPES);
    uint32_t sessionId = PickStreamId(fdp);
    int32_t uid = PickUid(fdp);

    service.SetInputDevice(deviceType, sessionId, uid);
}

void FuzzOnForcedDeviceSelected(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    DeviceType deviceType = Pick(fdp, DEVICE_TYPES);
    std::string macAddress = fdp.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH);
    sptr<AudioRendererFilter> filter = fdp.ConsumeBool() ? MakeRendererFilter(fdp) : nullptr;
    std::string caller = fdp.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH);

    service.OnForcedDeviceSelected(deviceType, macAddress, filter, caller);
}

void FuzzOnPrivacyDeviceSelected(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    DeviceType deviceType = Pick(fdp, DEVICE_TYPES);
    std::string macAddress = fdp.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH);
    std::string caller = fdp.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH);

    service.OnPrivacyDeviceSelected(deviceType, macAddress, caller);
}

void FuzzGetEnhancedRoutingSupported(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    service.GetEnhancedRoutingSupported();
}

void FuzzGetPreferredUid(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    int32_t uid = PickUid(fdp);
    service.GetPreferredUid(uid);
}

void FuzzSetDeviceEnableAndUsage(FuzzedDataProvider &fdp)
{
    auto &service = AudioSelectInterfaceService::GetInstance();
    auto device = MakeDevice(fdp);
    service.SetDeviceEnableAndUsage(device);
}

void FuzzTest(FuzzedDataProvider &fdp)
{
    auto func = fdp.PickValueInArray({
        FuzzSetDeviceActive,
        FuzzSelectOutputDevice,
        FuzzSetMediaOutputDeviceByUid,
        FuzzSelectInputDevice,
        FuzzSelectInputDeviceByUid,
        FuzzExcludeOutputDevices,
        FuzzUnexcludeOutputDevices,
        FuzzSetSessionDefaultOutputDevice,
        FuzzSetDefaultOutputDevice,
        FuzzSetInputDevice,
        FuzzOnForcedDeviceSelected,
        FuzzOnPrivacyDeviceSelected,
        FuzzGetEnhancedRoutingSupported,
        FuzzGetPreferredUid,
        FuzzSetDeviceEnableAndUsage,
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