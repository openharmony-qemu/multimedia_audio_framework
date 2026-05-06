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
#include <set>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include "audio_router_infra.h"

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

const std::array<DeviceType, 9> DEVICE_TYPES = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
};

const std::array<DeviceRole, 3> DEVICE_ROLES = {
    DEVICE_ROLE_NONE,
    INPUT_DEVICE,
    OUTPUT_DEVICE,
};

const std::array<SelectDeviceType, 5> SELECT_TYPES = {
    SelectDeviceType::MEDIA_INPUT,
    SelectDeviceType::MEDIA_OUTPUT,
    SelectDeviceType::CALL_INPUT,
    SelectDeviceType::CALL_OUTPUT,
    SelectDeviceType::INVALID_TYPE,
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

const std::array<CapturerState, 5> CAPTURER_STATES = {
    CAPTURER_NEW,
    CAPTURER_PREPARED,
    CAPTURER_RUNNING,
    CAPTURER_STOPPED,
    CAPTURER_RELEASED,
};

const std::array<RendererState, 5> RENDERER_STATES = {
    RENDERER_NEW,
    RENDERER_PREPARED,
    RENDERER_RUNNING,
    RENDERER_STOPPED,
    RENDERER_RELEASED,
};

const std::array<BluetoothAndNearlinkPreferredRecordCategory, 4> PREFERRED_CATEGORIES = {
    PREFERRED_NONE,
    PREFERRED_DEFAULT,
    PREFERRED_LOW_LATENCY,
    PREFERRED_HIGH_QUALITY,
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

std::shared_ptr<AudioDeviceSimpleDescriptor> MakeSimpleDevice(FuzzedDataProvider &fdp)
{
    if (fdp.ConsumeBool()) {
        return nullptr;
    }
    return std::make_shared<AudioDeviceSimpleDescriptor>(
        Pick(fdp, DEVICE_TYPES),
        Pick(fdp, DEVICE_ROLES),
        fdp.ConsumeIntegral<int32_t>(),
        fdp.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH),
        fdp.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH));
}

std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> MakeSimpleDeviceList(FuzzedDataProvider &fdp)
{
    std::vector<std::shared_ptr<AudioDeviceSimpleDescriptor>> devices;
    size_t count = fdp.ConsumeIntegralInRange<size_t>(0, MAX_VECTOR_SIZE);
    for (size_t i = 0; i < count; ++i) {
        devices.push_back(MakeSimpleDevice(fdp));
    }
    return devices;
}

std::set<DeviceType> MakeDeviceTypeSet(FuzzedDataProvider &fdp)
{
    std::set<DeviceType> types;
    size_t count = fdp.ConsumeIntegralInRange<size_t>(0, MAX_VECTOR_SIZE);
    for (size_t i = 0; i < count; ++i) {
        types.insert(Pick(fdp, DEVICE_TYPES));
    }
    return types;
}

void FuzzSelectDevice(FuzzedDataProvider &fdp)
{
    auto &infra = AudioRouterInfra::GetInstance();
    SelectDeviceType type = Pick(fdp, SELECT_TYPES);
    int32_t uid = PickUid(fdp);
    auto device = MakeSimpleDevice(fdp);

    infra.RefreshSelectDevice(type, uid, device);
    infra.GetSystemSelectDevice(type);
    infra.GetAppSelectDevice(type, uid);
    infra.GetLatestAppSelectDevice(type);
    infra.HasValidSelectDevice(AudioSelectDeviceInfo(device, fdp.ConsumeIntegral<int64_t>()));
    if (fdp.ConsumeBool()) {
        infra.ClearAllSelectSelectDevice(type, device);
    }
    infra.OnAppDied(uid);
}

void FuzzRecognitionDevice(FuzzedDataProvider &fdp)
{
    auto &infra = AudioRouterInfra::GetInstance();
    infra.SetRecognitionCaptureDevice(MakeSimpleDevice(fdp));
    infra.GetRecognitionCaptureDevice();
}

void FuzzStreamSelectDevice(FuzzedDataProvider &fdp)
{
    auto &infra = AudioRouterInfra::GetInstance();
    int32_t uid = PickUid(fdp);
    uint32_t streamId = PickStreamId(fdp);
    SelectDeviceType type = Pick(fdp, SELECT_TYPES);
    auto device = MakeSimpleDevice(fdp);

    infra.UpdateStreamSelectDevice(uid, streamId, device);
    infra.UpdateStreamDefaultDevice(uid, streamId, type, device);
    infra.GetStreamSelectDevice(uid, streamId);
    infra.GetStreamDefaultDevice(uid, streamId, type);
    infra.GetFirstRunningStreamDefaultDevice(uid, type);
    infra.GetLatestStreamSelectDevice(type);
    infra.ClearStreamSelectDevice(fdp.ConsumeBool() ? device : nullptr);
    infra.OnAppDied(uid);
}

void FuzzPreferredInputCategory(FuzzedDataProvider &fdp)
{
    auto &infra = AudioRouterInfra::GetInstance();
    int32_t uid = PickUid(fdp);
    infra.UpdatePreferredInputCategory(uid, Pick(fdp, PREFERRED_CATEGORIES));
    infra.GetPreferredInputCategory(uid);
    infra.GetHighestPriorityPreferredInputCategory();
    infra.OnAppDied(uid);
}

void FuzzExcludedDevice(FuzzedDataProvider &fdp)
{
    auto &infra = AudioRouterInfra::GetInstance();
    auto device = MakeSimpleDevice(fdp);
    AudioDeviceUsage usage = Pick(fdp, DEVICE_USAGES);

    infra.ExcludeDevice(device, usage);
    infra.GetExcludedDevices(usage);
    infra.IsDeviceExcluded(device, usage);
    if (fdp.ConsumeBool()) {
        infra.UnexcludeDevice(device, usage);
    }
    infra.SetScoExcluded(fdp.ConsumeBool());
    infra.GetScoExcluded();
    if (fdp.ConsumeBool()) {
        infra.UnexcludeAllDevice();
    }
}

void FuzzCurrentDevice(FuzzedDataProvider &fdp)
{
    auto &infra = AudioRouterInfra::GetInstance();
    int32_t uid = PickUid(fdp);
    auto devices = MakeSimpleDeviceList(fdp);

    infra.UpdateCurrentInputDevice(uid, devices);
    infra.GetCurrentInputDevice(uid);
    infra.FindCurrentInputDevice(MakeDeviceTypeSet(fdp));
    infra.UpdateCurrentOutputDevice(uid, devices);
    infra.GetCurrentOutputDevice(uid);
    infra.FindCurrentOutputDevice(MakeDeviceTypeSet(fdp));
    infra.OnAppDied(uid);
}

void FuzzStreamState(FuzzedDataProvider &fdp)
{
    auto &infra = AudioRouterInfra::GetInstance();
    int32_t uid = PickUid(fdp);
    uint32_t streamId = PickStreamId(fdp);

    infra.UpdateInputStreamState(uid, streamId, Pick(fdp, SOURCE_TYPES), Pick(fdp, CAPTURER_STATES));
    infra.UpdateOutputStreamState(uid, streamId, Pick(fdp, STREAM_USAGES), Pick(fdp, RENDERER_STATES),
        fdp.ConsumeBool());
    infra.GetHighestInputPriorityApp();
    infra.GetHighestOutputPriorityApp();
    infra.HasHighestPriorityRunningSourceType({SOURCE_TYPE_MIC, SOURCE_TYPE_CAMCORDER, SOURCE_TYPE_LIVE});
    infra.GetAppRunningSourceTypes(uid);
    infra.GetAppRunningStreamUsages(uid);
    infra.GetStreamSourceType(uid, streamId);
    infra.GetStreamStreamUsage(uid, streamId);
    infra.HasRunningInputStream(uid);
    infra.HasRunningOutputStream(uid);
    infra.UpdateAppForegroundState(uid, fdp.ConsumeBool());
    infra.OnAppDied(uid);
}

void FuzzSimpleDescriptor(FuzzedDataProvider &fdp)
{
    auto desc = MakeSimpleDevice(fdp);
    auto other = MakeSimpleDevice(fdp);
    if (desc == nullptr) {
        return;
    }

    if (other != nullptr) {
        (void)(*desc == *other);
        (void)(*desc != *other);
        (void)(*desc < *other);
    }
    AudioDeviceDescriptor deviceDesc;
    deviceDesc.deviceId_ = fdp.ConsumeIntegral<int32_t>();
    (void)(*desc == deviceDesc);
    (void)(*desc != deviceDesc);
    desc->GetOnlineDeviceDescriptor();
    AudioRouterInfra::GetInstance().IsPreferredDevice(desc);
}

void FuzzTest(FuzzedDataProvider &fdp)
{
    auto func = fdp.PickValueInArray({
        FuzzSelectDevice,
        FuzzRecognitionDevice,
        FuzzStreamSelectDevice,
        FuzzPreferredInputCategory,
        FuzzExcludedDevice,
        FuzzCurrentDevice,
        FuzzStreamState,
        FuzzSimpleDescriptor,
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
