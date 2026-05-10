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

#include "audio_router_follow_strategy.h"
#include "audio_router_independent_strategy.h"
#include "audio_router_infra.h"
#include "audio_router_select_strategy.h"

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

const std::array<DeviceType, 8> DEVICE_TYPES = {
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
};

const std::array<DeviceRole, 2> DEVICE_ROLES = {
    INPUT_DEVICE,
    OUTPUT_DEVICE,
};

const std::array<AudioDeviceUsage, 6> DEVICE_USAGES = {
    MEDIA_OUTPUT_DEVICES,
    MEDIA_INPUT_DEVICES,
    ALL_MEDIA_DEVICES,
    CALL_OUTPUT_DEVICES,
    CALL_INPUT_DEVICES,
    D_ALL_DEVICES,
};

const std::array<SourceType, 7> SOURCE_TYPES = {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_LIVE,
};

const std::array<StreamUsage, 8> STREAM_USAGES = {
    STREAM_USAGE_INVALID,
    STREAM_USAGE_MEDIA,
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_VOICE_MESSAGE,
    STREAM_USAGE_NOTIFICATION,
    STREAM_USAGE_GAME,
    STREAM_USAGE_VIDEO_COMMUNICATION,
};

template<typename T, size_t N>
T Pick(FuzzedDataProvider &fdp, const std::array<T, N> &values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, N - 1)];
}

int32_t PickUid(FuzzedDataProvider &fdp)
{
    const std::array<int32_t, 4> uids = {
        INVALID_UID,
        SYSTEM_UID,
        fdp.ConsumeIntegralInRange<int32_t>(MIN_NORMAL_UID, MAX_NORMAL_UID),
        fdp.ConsumeIntegral<int32_t>(),
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

std::set<DeviceType> MakeDeviceTypeSet(FuzzedDataProvider &fdp)
{
    std::set<DeviceType> types;
    size_t count = fdp.ConsumeIntegralInRange<size_t>(0, MAX_VECTOR_SIZE);
    for (size_t i = 0; i < count; ++i) {
        types.insert(Pick(fdp, DEVICE_TYPES));
    }
    return types;
}

AudioRouterSelectStrategy &PickStrategy(FuzzedDataProvider &fdp)
{
    static AudioRouterFollowStrategy followStrategy;
    static AudioRouterIndependentStrategy independentStrategy;
    if (fdp.ConsumeBool()) {
        return followStrategy;
    }
    return independentStrategy;
}

void FuzzMediaInput(FuzzedDataProvider &fdp)
{
    auto &strategy = PickStrategy(fdp);
    int32_t uid = PickUid(fdp);
    uint32_t streamId = PickStreamId(fdp);
    SourceType sourceType = Pick(fdp, SOURCE_TYPES);

    strategy.SetMediaInputDevice(uid, streamId, MakeDevice(fdp));
    strategy.GetMediaInputDevice(uid, streamId, sourceType);
    strategy.SetRecognitionInputDevice(MakeDevice(fdp));
    strategy.GetRecognitionInputDevice();
    AudioRouterInfra::GetInstance().OnAppDied(uid);
}

void FuzzMediaOutput(FuzzedDataProvider &fdp)
{
    auto &strategy = PickStrategy(fdp);
    int32_t uid = PickUid(fdp);
    uint32_t streamId = PickStreamId(fdp);

    strategy.SetMediaOutputDevice(uid, streamId, MakeDevice(fdp));
    strategy.GetMediaOutputDevice(uid, streamId);
    strategy.UpdateMediaDefaultOutputDevice(uid, streamId, MakeDevice(fdp));
    strategy.GetMediaDefaultOutputDevice(uid, streamId);
    strategy.UpdateDefaultOutputDevice(Pick(fdp, DEVICE_TYPES), uid, streamId, Pick(fdp, STREAM_USAGES));
    strategy.IsStreamSetDefaultOutputDevice(uid, streamId);
    AudioRouterInfra::GetInstance().OnAppDied(uid);
}

void FuzzCallDevice(FuzzedDataProvider &fdp)
{
    auto &strategy = PickStrategy(fdp);
    int32_t uid = PickUid(fdp);
    uint32_t streamId = PickStreamId(fdp);

    strategy.SetCallInputDevice(uid, streamId, MakeDevice(fdp));
    strategy.GetCallInputDevice(uid, streamId);
    strategy.SetCallOutputDevice(uid, streamId, MakeDevice(fdp),
        fdp.ConsumeRandomLengthString(MAX_RANDOM_STRING_LENGTH));
    strategy.GetCallOutputDevice(uid, streamId);
    strategy.UpdateCallDefaultOutputDevice(uid, streamId, MakeDevice(fdp));
    strategy.GetCallDefaultOutputDevice(uid, streamId);
    AudioRouterInfra::GetInstance().OnAppDied(uid);
}

void FuzzCurrentDevice(FuzzedDataProvider &fdp)
{
    auto &strategy = PickStrategy(fdp);
    int32_t uid = PickUid(fdp);
    auto devices = MakeDeviceList(fdp);
    int32_t deviceId = fdp.ConsumeIntegral<int32_t>();

    strategy.UpdateCurrentInputDevice(uid, devices);
    strategy.GetCurrentInputDevice(uid);
    strategy.Get1stCurrentInputDevice(uid);
    strategy.IsCurrentInputDevice(deviceId, uid);
    strategy.FindCurrentInputDevice(MakeDeviceTypeSet(fdp));
    strategy.UpdateCurrentOutputDevice(uid, devices);
    strategy.GetCurrentOutputDevice(uid);
    strategy.Get1stCurrentOutputDevice(uid);
    strategy.IsCurrentOutputDevice(deviceId, uid);
    strategy.FindCurrentOutputDevice(MakeDeviceTypeSet(fdp));
    AudioRouterInfra::GetInstance().OnAppDied(uid);
}

void FuzzExcludeDevice(FuzzedDataProvider &fdp)
{
    auto &strategy = PickStrategy(fdp);
    auto devices = MakeDeviceList(fdp);
    auto device = MakeDevice(fdp);
    AudioDeviceUsage usage = Pick(fdp, DEVICE_USAGES);

    strategy.ExcludeDevices(devices, usage);
    strategy.GetExcludedDevices(usage);
    strategy.IsDeviceExcluded(device, usage);
    strategy.UnexcludeDevices(devices, usage);
    strategy.SetScoExcluded(fdp.ConsumeBool());
    strategy.GetScoExcluded();
    strategy.IsPreferredDevice(*device);
    AudioRouterInfra::GetInstance().UnexcludeAllDevice();
}

void FuzzInstance(FuzzedDataProvider &fdp)
{
    auto &strategy = AudioRouterSelectStrategy::GetInstance();
    int32_t uid = PickUid(fdp);
    uint32_t streamId = PickStreamId(fdp);

    strategy.GetEnhancedRoutingSupported();
    strategy.GetMediaOutputDevice(uid, streamId);
    strategy.GetCallOutputDevice(uid, streamId);
    strategy.GetCurrentInputDevice(uid);
    strategy.GetCurrentOutputDevice(uid);
    AudioRouterInfra::GetInstance().OnAppDied(uid);
}

void FuzzTest(FuzzedDataProvider &fdp)
{
    auto func = fdp.PickValueInArray({
        FuzzMediaInput,
        FuzzMediaOutput,
        FuzzCallDevice,
        FuzzCurrentDevice,
        FuzzExcludeDevice,
        FuzzInstance,
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
