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
#include <cstring>
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

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 19;

typedef void (*TestFuncs)();

vector<AudioDeviceUsage> AudioDeviceUsageVec = {
    MEDIA_OUTPUT_DEVICES,
    MEDIA_INPUT_DEVICES,
    ALL_MEDIA_DEVICES,
    CALL_OUTPUT_DEVICES,
    CALL_INPUT_DEVICES,
    ALL_CALL_DEVICES,
    D_ALL_DEVICES,
};

const vector<DeviceType> g_testDeviceTypes = {
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
    DEVICE_TYPE_MAX
};

const vector<StreamUsage> g_testStreamUsages = {
    STREAM_USAGE_INVALID,
    STREAM_USAGE_UNKNOWN,
    STREAM_USAGE_MEDIA,
    STREAM_USAGE_MUSIC,
    STREAM_USAGE_VOICE_COMMUNICATION,
    STREAM_USAGE_VOICE_ASSISTANT,
    STREAM_USAGE_ALARM,
    STREAM_USAGE_VOICE_MESSAGE,
    STREAM_USAGE_NOTIFICATION_RINGTONE,
    STREAM_USAGE_RINGTONE,
    STREAM_USAGE_NOTIFICATION,
    STREAM_USAGE_ACCESSIBILITY,
    STREAM_USAGE_SYSTEM,
    STREAM_USAGE_MOVIE,
    STREAM_USAGE_GAME,
    STREAM_USAGE_AUDIOBOOK,
    STREAM_USAGE_NAVIGATION,
    STREAM_USAGE_DTMF,
    STREAM_USAGE_ENFORCED_TONE,
    STREAM_USAGE_ULTRASONIC,
    STREAM_USAGE_VIDEO_COMMUNICATION,
    STREAM_USAGE_RANGING,
    STREAM_USAGE_VOICE_MODEM_COMMUNICATION,
    STREAM_USAGE_VOICE_RINGTONE,
    STREAM_USAGE_VOICE_CALL_ASSISTANT,
    STREAM_USAGE_MAX,
};

const vector<SourceType> g_testSourceTypes = {
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

const vector<AudioScene> g_testAudioScenes = {
    AUDIO_SCENE_INVALID,
    AUDIO_SCENE_DEFAULT,
    AUDIO_SCENE_RINGING,
    AUDIO_SCENE_PHONE_CALL,
    AUDIO_SCENE_PHONE_CHAT,
    AUDIO_SCENE_CALL_START,
    AUDIO_SCENE_CALL_END,
    AUDIO_SCENE_VOICE_RINGING,
    AUDIO_SCENE_MAX,
};

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void RecoveryPreferredDevicesFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    audioRecoveryDevice->RecoveryPreferredDevices();
}

void RecoverExcludedOutputDevicesFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    audioRecoveryDevice->RecoverExcludedOutputDevices();
}

void HandleExcludedOutputDevicesRecoveryFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    std::vector<std::shared_ptr<Media::MediaMonitor::MonitorDeviceInfo>> excludedDevices;
    auto mediaMonitor = std::make_shared<Media::MediaMonitor::MonitorDeviceInfo>();
    excludedDevices.push_back(mediaMonitor);
    uint32_t audioDevUsageCount = GetData<uint32_t>() % AudioDeviceUsageVec.size();
    AudioDeviceUsage audioDevUsage = AudioDeviceUsageVec[audioDevUsageCount];
    audioRecoveryDevice->HandleExcludedOutputDevicesRecovery(audioDevUsage, excludedDevices);
}

void AudioRecoveryDeviceHandleRecoveryPreferredDevicesFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    if (audioRecoveryDevice == nullptr || g_testDeviceTypes.size() == 0) {
        return;
    }
    std::shared_ptr<AudioA2dpOffloadManager> audioA2dpOffloadManager = std::make_shared<AudioA2dpOffloadManager>();
    audioRecoveryDevice->Init(audioA2dpOffloadManager);

    DeviceType deviceType = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];

    int32_t deviceTypeToInt = static_cast<int32_t>(deviceType);
    int32_t usageOrSourceType = GetData<int32_t>();
    int32_t preferredType = GetData<int32_t>();
    std::shared_ptr<AudioDeviceDescriptor> audioDeviceDescriptor = make_shared<AudioDeviceDescriptor>();
    if (audioDeviceDescriptor == nullptr) {
        return;
    }
    audioDeviceDescriptor->deviceType_ = deviceType;
    audioRecoveryDevice->audioConnectedDevice_.connectedDevices_.push_back(audioDeviceDescriptor);
    audioRecoveryDevice->HandleRecoveryPreferredDevices(preferredType, deviceTypeToInt, usageOrSourceType);
}

void AudioRecoveryDeviceSelectOutputDeviceFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioRecoveryDevice == nullptr || g_testDeviceTypes.size() == 0
        || audioRendererFilter == nullptr || deviceDescriptor == nullptr) {
        return;
    }
    audioRendererFilter->uid = GetData<int32_t>();
    audioRendererFilter->rendererInfo.rendererFlags = GetData<int32_t>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    deviceDescriptor->deviceRole_ = DeviceRole::OUTPUT_DEVICE;
    deviceDescriptor->deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    selectedDesc.push_back(deviceDescriptor);
    audioRecoveryDevice->audioA2dpOffloadManager_ = make_shared<AudioA2dpOffloadManager>();
    audioRecoveryDevice->SelectOutputDevice(audioRendererFilter, selectedDesc);
}

void AudioRecoveryDeviceHandleFetchDeviceChangeFuzzTest()
{
    static const vector<AudioStreamDeviceChangeReason> testAudioStreamDeviceChangeReason = {
        AudioStreamDeviceChangeReason::UNKNOWN,
        AudioStreamDeviceChangeReason::NEW_DEVICE_AVAILABLE,
        AudioStreamDeviceChangeReason::OLD_DEVICE_UNAVALIABLE,
        AudioStreamDeviceChangeReason::OVERRODE,
        AudioStreamDeviceChangeReason::AUDIO_SESSION_ACTIVATE,
        AudioStreamDeviceChangeReason::STREAM_PRIORITY_CHANGED,
    };
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    if (audioRecoveryDevice == nullptr || g_testDeviceTypes.size() == 0
        || testAudioStreamDeviceChangeReason.size() == 0) {
        return;
    }
    audioRecoveryDevice->audioActiveDevice_.currentActiveDevice_.deviceType_ =
        g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    audioRecoveryDevice->audioActiveDevice_.currentActiveDevice_.networkId_ = "testNetworkId";
    AudioStreamDeviceChangeReason reason =
        testAudioStreamDeviceChangeReason[GetData<uint32_t>() % testAudioStreamDeviceChangeReason.size()];
    std::string caller{};
    audioRecoveryDevice->audioA2dpOffloadManager_ = make_shared<AudioA2dpOffloadManager>();
    audioRecoveryDevice->HandleFetchDeviceChange(reason, caller);
}

void AudioRecoveryDeviceSelectOutputDeviceForFastInnerFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    if (audioRecoveryDevice == nullptr) {
        return;
    }
    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    selectedDesc.push_back(deviceDescriptor);

    audioRecoveryDevice->SelectOutputDeviceForFastInner(audioRendererFilter, selectedDesc);
}

void AudioRecoveryDeviceSetRenderDeviceForUsageFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    std::shared_ptr<AudioDeviceDescriptor> desc = std::make_shared<AudioDeviceDescriptor>();
    if (audioRecoveryDevice == nullptr || g_testStreamUsages.size() == 0 || desc == nullptr
        || g_testDeviceTypes.size() == 0) {
        return;
    }
    StreamUsage streamUsage = g_testStreamUsages[GetData<uint32_t>() % g_testStreamUsages.size()];
    desc->deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];

    audioRecoveryDevice->SetRenderDeviceForUsage(streamUsage, desc);
}

void AudioRecoveryDeviceConnectVirtualDeviceFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    std::shared_ptr<AudioDeviceDescriptor> selectedDesc = std::make_shared<AudioDeviceDescriptor>();
    if (audioRecoveryDevice == nullptr || selectedDesc == nullptr || g_testDeviceTypes.size() == 0) {
        return;
    }
    selectedDesc->deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];

    audioRecoveryDevice->ConnectVirtualDevice(selectedDesc);
}

void AudioRecoveryDeviceWriteSelectOutputSysEventsFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDescs;
    std::shared_ptr<AudioDeviceDescriptor> selectedDesc = std::make_shared<AudioDeviceDescriptor>();
    if (audioRecoveryDevice == nullptr || selectedDesc == nullptr ||
        g_testDeviceTypes.size() == 0 || g_testStreamUsages.size() == 0) {
        return;
    }
    selectedDesc->deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    selectedDescs.push_back(selectedDesc);
    StreamUsage strUsage = g_testStreamUsages[GetData<uint32_t>() % g_testStreamUsages.size()];

    audioRecoveryDevice->WriteSelectOutputSysEvents(selectedDescs, strUsage);
}

void AudioRecoveryDeviceSelectFastOutputDeviceFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioRecoveryDevice == nullptr || audioRendererFilter == nullptr || deviceDescriptor == nullptr) {
        return;
    }
    audioRendererFilter->uid = GetData<int32_t>();
    deviceDescriptor->networkId_ = "testNetworkId";

    audioRecoveryDevice->SelectFastOutputDevice(audioRendererFilter, deviceDescriptor);
}

void AudioRecoveryDeviceSelectOutputDeviceByFilterInnerFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    sptr<AudioRendererFilter> audioRendererFilter = new AudioRendererFilter();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioRecoveryDevice == nullptr || audioRendererFilter == nullptr ||
        deviceDescriptor == nullptr || g_testDeviceTypes.size() == 0) {
        return;
    }
    audioRendererFilter->uid = GetData<int32_t>();
    deviceDescriptor->deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    deviceDescriptor->networkId_ = "testNetworkId";
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    selectedDesc.push_back(deviceDescriptor);
    std::shared_ptr<AudioRendererChangeInfo> audioRendererChangeInfo = make_shared<AudioRendererChangeInfo>();
    audioRecoveryDevice->streamCollector_.audioRendererChangeInfos_.push_back(audioRendererChangeInfo);

    audioRecoveryDevice->SelectOutputDeviceByFilterInner(audioRendererFilter, selectedDesc);
}

void AudioRecoveryDeviceSelectInputDeviceFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    sptr<AudioCapturerFilter> audioCapturerFilter = new AudioCapturerFilter();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioRecoveryDevice == nullptr || audioCapturerFilter == nullptr ||
        deviceDescriptor == nullptr || g_testSourceTypes.size() == 0 || g_testDeviceTypes.size() == 0) {
        return;
    }

    audioCapturerFilter->uid = GetData<int32_t>();
    audioCapturerFilter->capturerInfo.sourceType = g_testSourceTypes[GetData<uint32_t>() % g_testSourceTypes.size()];
    audioCapturerFilter->capturerInfo.capturerFlags = GetData<int32_t>();
    deviceDescriptor->networkId_ = "testNetworkId";
    deviceDescriptor->deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    deviceDescriptor->macAddress_ = "testMacAddress";
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    selectedDesc.push_back(deviceDescriptor);

    audioRecoveryDevice->SelectInputDevice(audioCapturerFilter, selectedDesc);
}

void AudioRecoveryDeviceExcludeOutputDevicesFuzzTest()
{
    static const vector<AudioDeviceUsage> testAudioDeviceUsage = {
        MEDIA_OUTPUT_DEVICES,
        MEDIA_INPUT_DEVICES,
        ALL_MEDIA_DEVICES,
        CALL_OUTPUT_DEVICES,
        CALL_INPUT_DEVICES,
        ALL_CALL_DEVICES,
        D_ALL_DEVICES,
    };
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioRecoveryDevice == nullptr || testAudioDeviceUsage.size() == 0
        || deviceDescriptor == nullptr || g_testDeviceTypes.size() == 0) {
        return;
    }
    AudioDeviceUsage audioDevUsage =
        testAudioDeviceUsage[GetData<uint32_t>() % testAudioDeviceUsage.size()];
    deviceDescriptor->networkId_ = "testNetworkId";
    deviceDescriptor->deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    selectedDesc.push_back(deviceDescriptor);
    audioRecoveryDevice->audioA2dpOffloadManager_ = make_shared<AudioA2dpOffloadManager>();

    audioRecoveryDevice->ExcludeOutputDevices(audioDevUsage, selectedDesc);
}

void AudioRecoveryDeviceUnexcludeOutputDevicesFuzzTest()
{
    static const vector<AudioDeviceUsage> testAudioDeviceUsage = {
        MEDIA_OUTPUT_DEVICES,
        MEDIA_INPUT_DEVICES,
        ALL_MEDIA_DEVICES,
        CALL_OUTPUT_DEVICES,
        CALL_INPUT_DEVICES,
        ALL_CALL_DEVICES,
        D_ALL_DEVICES,
    };
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioRecoveryDevice == nullptr || testAudioDeviceUsage.size() == 0
        || deviceDescriptor == nullptr || g_testDeviceTypes.size() == 0) {
        return;
    }
    AudioDeviceUsage audioDevUsage =
        testAudioDeviceUsage[GetData<uint32_t>() % testAudioDeviceUsage.size()];
    deviceDescriptor->networkId_ = "testNetworkId";
    deviceDescriptor->deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    selectedDesc.push_back(deviceDescriptor);
    audioRecoveryDevice->audioA2dpOffloadManager_ = make_shared<AudioA2dpOffloadManager>();

    audioRecoveryDevice->UnexcludeOutputDevices(audioDevUsage, selectedDesc);
}

void AudioRecoveryDeviceSetCaptureDeviceForUsageFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioRecoveryDevice == nullptr || g_testSourceTypes.size() == 0
        || deviceDescriptor == nullptr || g_testAudioScenes.size() == 0) {
        return;
    }
    AudioScene scene = g_testAudioScenes[GetData<uint32_t>() % g_testAudioScenes.size()];
    SourceType srcType = g_testSourceTypes[GetData<uint32_t>() % g_testSourceTypes.size()];
    audioRecoveryDevice->audioA2dpOffloadManager_ = make_shared<AudioA2dpOffloadManager>();

    audioRecoveryDevice->SetCaptureDeviceForUsage(scene, srcType, deviceDescriptor);
}

void AudioRecoveryDeviceSelectFastInputDeviceFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioRecoveryDevice == nullptr || deviceDescriptor == nullptr) {
        return;
    }
    sptr<AudioCapturerFilter> audioCapturerFilter = new AudioCapturerFilter();
    audioRecoveryDevice->audioA2dpOffloadManager_ = make_shared<AudioA2dpOffloadManager>();

    audioRecoveryDevice->SelectFastInputDevice(audioCapturerFilter, deviceDescriptor);
}

void AudioRecoveryDeviceWriteSelectInputSysEventsFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioRecoveryDevice == nullptr || g_testDeviceTypes.size() == 0
        || deviceDescriptor == nullptr || g_testSourceTypes.size() == 0 || g_testAudioScenes.size() == 0) {
        return;
    }

    SourceType srcType = g_testSourceTypes[GetData<uint32_t>() % g_testSourceTypes.size()];
    AudioScene scene = g_testAudioScenes[GetData<uint32_t>() % g_testAudioScenes.size()];
    deviceDescriptor->deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc;
    selectedDesc.push_back(deviceDescriptor);
    audioRecoveryDevice->audioA2dpOffloadManager_ = make_shared<AudioA2dpOffloadManager>();

    audioRecoveryDevice->WriteSelectInputSysEvents(selectedDesc, srcType, scene);
}

void AudioRecoveryDeviceWriteExcludeOutputSysEventsFuzzTest()
{
    auto audioRecoveryDevice = std::make_shared<AudioRecoveryDevice>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    if (audioRecoveryDevice == nullptr || g_testDeviceTypes.size() == 0
        || deviceDescriptor == nullptr ||AudioDeviceUsageVec.size() == 0) {
        return;
    }

    deviceDescriptor->deviceType_ = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    AudioDeviceUsage audioDevUsage = AudioDeviceUsageVec[GetData<uint32_t>() % AudioDeviceUsageVec.size()];
    audioRecoveryDevice->audioA2dpOffloadManager_ = make_shared<AudioA2dpOffloadManager>();

    audioRecoveryDevice->WriteExcludeOutputSysEvents(audioDevUsage, deviceDescriptor);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    RecoveryPreferredDevicesFuzzTest,
    RecoverExcludedOutputDevicesFuzzTest,
    HandleExcludedOutputDevicesRecoveryFuzzTest,
    AudioRecoveryDeviceHandleRecoveryPreferredDevicesFuzzTest,
    AudioRecoveryDeviceSelectOutputDeviceFuzzTest,
    AudioRecoveryDeviceHandleFetchDeviceChangeFuzzTest,
    AudioRecoveryDeviceSelectOutputDeviceForFastInnerFuzzTest,
    AudioRecoveryDeviceSetRenderDeviceForUsageFuzzTest,
    AudioRecoveryDeviceConnectVirtualDeviceFuzzTest,
    AudioRecoveryDeviceWriteSelectOutputSysEventsFuzzTest,
    AudioRecoveryDeviceSelectFastOutputDeviceFuzzTest,
    AudioRecoveryDeviceSelectOutputDeviceByFilterInnerFuzzTest,
    AudioRecoveryDeviceSelectInputDeviceFuzzTest,
    AudioRecoveryDeviceExcludeOutputDevicesFuzzTest,
    AudioRecoveryDeviceUnexcludeOutputDevicesFuzzTest,
    AudioRecoveryDeviceSetCaptureDeviceForUsageFuzzTest,
    AudioRecoveryDeviceSelectFastInputDeviceFuzzTest,
    AudioRecoveryDeviceWriteSelectInputSysEventsFuzzTest,
    AudioRecoveryDeviceWriteExcludeOutputSysEventsFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
