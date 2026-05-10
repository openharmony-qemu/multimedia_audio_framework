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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <vector>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"
#include "dfx_msg_manager.h"
#include "hpae_manager.h"
#include "audio_info.h"
#include "device_status_listener.h"
#include "audio_policy_service.h"
#include "bluetooth_host.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;

typedef void (*TestFuncs)();

template<typename T>
T ConsumeEnumInRange(FuzzedDataProvider& fdp, T minValue, T maxValue)
{
    return static_cast<T>(fdp.ConsumeIntegralInRange<int32_t>(
        static_cast<int32_t>(minValue), static_cast<int32_t>(maxValue)));
}

template<typename T>
const T& PickValue(FuzzedDataProvider& fdp, const vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

vector<DeviceType> DeviceTypeVec = {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_INVALID,
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WAKEUP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_DP,
    DEVICE_TYPE_REMOTE_CAST,
    DEVICE_TYPE_USB_DEVICE,
    DEVICE_TYPE_ACCESSORY,
    DEVICE_TYPE_REMOTE_DAUDIO,
    DEVICE_TYPE_HDMI,
    DEVICE_TYPE_LINE_DIGITAL,
    DEVICE_TYPE_NEARLINK,
    DEVICE_TYPE_NEARLINK_IN,
    DEVICE_TYPE_FILE_SINK,
    DEVICE_TYPE_FILE_SOURCE,
    DEVICE_TYPE_EXTERN_CABLE,
    DEVICE_TYPE_DEFAULT,
    DEVICE_TYPE_USB_ARM_HEADSET,
    DEVICE_TYPE_MAX,
};

vector<SourceType> SourceTypeVec = {
    SOURCE_TYPE_INVALID,
    SOURCE_TYPE_MIC,
    SOURCE_TYPE_VOICE_RECOGNITION,
    SOURCE_TYPE_PLAYBACK_CAPTURE,
    SOURCE_TYPE_WAKEUP,
    SOURCE_TYPE_VOICE_CALL,
    SOURCE_TYPE_VOICE_COMMUNICATION,
    SOURCE_TYPE_ULTRASONIC,
    SOURCE_TYPE_VIRTUAL_CAPTURE,
    SOURCE_TYPE_VOICE_MESSAGE,
    SOURCE_TYPE_REMOTE_CAST,
    SOURCE_TYPE_VOICE_TRANSCRIPTION,
    SOURCE_TYPE_CAMCORDER,
    SOURCE_TYPE_UNPROCESSED,
    SOURCE_TYPE_EC,
    SOURCE_TYPE_MIC_REF,
    SOURCE_TYPE_LIVE,
    SOURCE_TYPE_MAX,
};

const vector<AudioDeviceUsage> AudioDeviceUsageVec = {
    MEDIA_OUTPUT_DEVICES,
    MEDIA_INPUT_DEVICES,
    ALL_MEDIA_DEVICES,
    CALL_OUTPUT_DEVICES,
    CALL_INPUT_DEVICES,
    ALL_CALL_DEVICES,
    D_ALL_DEVICES,
};

void OnStop()
{
    Bluetooth::BluetoothHost::GetDefaultHost().Close();
}

void RegisterTrackerFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    AudioMode mode = ConsumeEnumInRange(fdp, AudioMode::AUDIO_MODE_PLAYBACK, AudioMode::AUDIO_MODE_RECORD);
    AudioStreamChangeInfo streamChangeInfo;
    sptr<IRemoteObject> object = nullptr;
    int32_t apiVersion = fdp.ConsumeIntegral<int32_t>();
    OnStop();
}

void SendA2dpConnectedWhileRunningFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    RendererState rendererState =
        ConsumeEnumInRange(fdp, RendererState::RENDERER_INVALID, RendererState::RENDERER_PAUSED);
    uint32_t sessionId = fdp.ConsumeIntegral<uint32_t>();
    audioDeviceLock->audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    OnStop();
}

void RegisteredTrackerClientDiedFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    pid_t uid = static_cast<pid_t>(
        fdp.ConsumeIntegralInRange<int32_t>(0, static_cast<int32_t>(AudioPipeType::PIPE_TYPE_OUT_VOIP)));
    OnStop();
}

void UpdateTrackerFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    AudioMode mode = ConsumeEnumInRange(fdp, AudioMode::AUDIO_MODE_PLAYBACK, AudioMode::AUDIO_MODE_RECORD);
    AudioStreamChangeInfo streamChangeInfo;
    streamChangeInfo.audioRendererChangeInfo.rendererState =
        ConsumeEnumInRange(fdp, RendererState::RENDERER_INVALID, RendererState::RENDERER_PAUSED);
    OnStop();
}

void GetCurrentRendererChangeInfosFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos = {
        std::make_shared<AudioRendererChangeInfo>()
    };
    bool hasBTPermission = fdp.ConsumeBool();
    bool hasSystemPermission = fdp.ConsumeBool();
    OnStop();
}

void OnDeviceStatusUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    AudioDeviceDescriptor updatedDesc;
    updatedDesc.deviceType_ = PickValue(fdp, DeviceTypeVec);
    updatedDesc.connectState_ = ConsumeEnumInRange(fdp, ConnectState::CONNECTED, ConnectState::DEACTIVE_CONNECTED);
    bool isConnected = fdp.ConsumeBool();
    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
    OnStop();
}

void GetVolumeGroupInfosFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    audioDeviceLock->audioVolumeManager_.isPrimaryMicModuleInfoLoaded_.store(fdp.ConsumeBool());
    OnStop();
}

void SetAudioSceneFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    AudioScene audioScene = ConsumeEnumInRange(fdp, AudioScene::AUDIO_SCENE_INVALID, AudioScene::AUDIO_SCENE_MAX);
    OnStop();
}

void AudioDeviceLockGetDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    vector<DeviceFlag> testDeviceFlags = {
        NONE_DEVICES_FLAG,
        OUTPUT_DEVICES_FLAG,
        INPUT_DEVICES_FLAG,
        ALL_DEVICES_FLAG,
        DISTRIBUTED_OUTPUT_DEVICES_FLAG,
        DISTRIBUTED_INPUT_DEVICES_FLAG,
        ALL_DISTRIBUTED_DEVICES_FLAG,
        ALL_L_D_DEVICES_FLAG,
        DEVICE_FLAG_MAX,
    };
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    if (audioDeviceLock == nullptr || testDeviceFlags.size() == 0) {
        return;
    }
    DeviceFlag deviceFlag = PickValue(fdp, testDeviceFlags);
    audioDeviceLock->GetDevices(deviceFlag);
    OnStop();
}

void AudioDeviceLockOnDeviceInfoUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    static const vector<DeviceInfoUpdateCommand> testDeviceInfoUpdateCommands = {
        CATEGORY_UPDATE,
        CONNECTSTATE_UPDATE,
        ENABLE_UPDATE,
        EXCEPTION_FLAG_UPDATE,
    };
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    if (audioDeviceLock == nullptr || testDeviceInfoUpdateCommands.size() == 0) {
        return;
    }
    AudioDeviceDescriptor desc;
    DeviceInfoUpdateCommand command = PickValue(fdp, testDeviceInfoUpdateCommands);
    audioDeviceLock->OnDeviceInfoUpdated(desc, command);
    OnStop();
}

void AudioDeviceLockUpdateAppVolumeFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    if (audioDeviceLock == nullptr) {
        return;
    }
    int32_t appUid = fdp.ConsumeIntegral<int32_t>();
    int32_t volume = fdp.ConsumeIntegral<int32_t>();
    audioDeviceLock->UpdateAppVolume(appUid, volume);
    OnStop();
}

void AudioDeviceLockOnDeviceStatusUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    if (audioDeviceLock == nullptr) {
        return;
    }

    DStatusInfo statusInfo;
    bool isStop = fdp.ConsumeBool();
    audioDeviceLock->OnDeviceStatusUpdated(statusInfo, isStop);
    OnStop();
}

void AudioDeviceLockOnPnpDeviceStatusUpdatedFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    if (audioDeviceLock == nullptr) {
        return;
    }

    AudioDeviceDescriptor desc;
    bool isConnected = fdp.ConsumeBool();
    audioDeviceLock->OnPnpDeviceStatusUpdated(desc, isConnected);
    OnStop();
}

void AudioDeviceLockUpdateSpatializationSupportedFuzzTest(FuzzedDataProvider& fdp)
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    if (audioDeviceLock == nullptr) {
        return;
    }

    std::string macAddress = "test_mac_address";
    bool support = fdp.ConsumeBool();
    audioDeviceLock->UpdateSpatializationSupported(macAddress, support);
    OnStop();
}

void AudioDeviceDescriptorMarshallingToDeviceInfoFuzzTest(FuzzedDataProvider& fdp)
{
    Parcel parcel;
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    audioDeviceDescriptor->deviceType_ = PickValue(fdp, DeviceTypeVec);
    bool hasBTPermission = fdp.ConsumeBool();
    bool hasSystemPermission = fdp.ConsumeBool();
    int32_t apiVersion = fdp.ConsumeIntegral<int32_t>();
    audioDeviceDescriptor->MarshallingToDeviceInfo(parcel, hasBTPermission, hasSystemPermission, apiVersion);
    OnStop();
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    RegisterTrackerFuzzTest,
    SendA2dpConnectedWhileRunningFuzzTest,
    UpdateTrackerFuzzTest,
    RegisteredTrackerClientDiedFuzzTest,
    OnDeviceStatusUpdatedFuzzTest,
    GetCurrentRendererChangeInfosFuzzTest,
    GetVolumeGroupInfosFuzzTest,
    SetAudioSceneFuzzTest,
    AudioDeviceLockGetDevicesFuzzTest,
    AudioDeviceLockUpdateAppVolumeFuzzTest,
    AudioDeviceLockOnDeviceInfoUpdatedFuzzTest,
    AudioDeviceLockOnDeviceStatusUpdatedFuzzTest,
    AudioDeviceLockOnPnpDeviceStatusUpdatedFuzzTest,
    AudioDeviceLockUpdateSpatializationSupportedFuzzTest,
    AudioDeviceDescriptorMarshallingToDeviceInfoFuzzTest
    });
    func(fdp);
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
