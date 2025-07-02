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

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 12;
static int32_t NUM_2 = 2;

typedef void (*TestFuncs)();

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

void OffloadStartPlayingFuzzTest()
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    constexpr int32_t stateCount = static_cast<int32_t>(BluetoothOffloadState::A2DP_OFFLOAD) + 1;
    BluetoothOffloadState state = static_cast<BluetoothOffloadState>(GetData<int32_t>() % stateCount);
    manager->SetA2dpOffloadFlag(state);
    std::vector<int32_t> sessionIds = {1, 2, 3};
    constexpr int32_t a2dpOffloadConnectionStateCount =
        static_cast<int32_t>(A2dpOffloadConnectionState::CONNECTION_STATUS_TIMEOUT) + 1;
    A2dpOffloadConnectionState currentOffloadConnectionState =
        static_cast<A2dpOffloadConnectionState>(GetData<uint8_t>() % a2dpOffloadConnectionStateCount);
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(currentOffloadConnectionState);
    manager->OffloadStartPlaying(sessionIds);
}

void OffloadStopPlayingFuzzTest()
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    constexpr int32_t stateCount = static_cast<int32_t>(BluetoothOffloadState::A2DP_OFFLOAD) + 1;
    BluetoothOffloadState state = static_cast<BluetoothOffloadState>(GetData<int32_t>() % stateCount);
    manager->SetA2dpOffloadFlag(state);
    std::vector<int32_t> sessionIds = {1, 2, 3};
    manager->OffloadStopPlaying(sessionIds);
}

void UpdateA2dpOffloadFlagForAllStreamFuzzTest()
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::unordered_map<uint32_t, bool> sessionIDToSpatializationEnableMap = {
        {1, true},
        {2, false}
    };

    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    auto changeInfo1 = std::make_shared<AudioRendererChangeInfo>();
    changeInfo1->sessionId = GetData<uint32_t>();
    constexpr int32_t rendererStateCount =
        static_cast<int32_t>(RendererState::RENDERER_PAUSED - RendererState::RENDERER_INVALID) + 1;
    changeInfo1->rendererState = static_cast<RendererState>(GetData<int32_t>() % rendererStateCount- 1);
    audioRendererChangeInfos.push_back(changeInfo1);

    auto changeInfo2 = std::make_shared<AudioRendererChangeInfo>();
    changeInfo2->sessionId = GetData<uint32_t>();
    changeInfo2->rendererState = static_cast<RendererState>(GetData<int32_t>() % rendererStateCount - 1);
    audioRendererChangeInfos.push_back(changeInfo2);
    manager->streamCollector_.audioRendererChangeInfos_ = audioRendererChangeInfos;
    manager->UpdateA2dpOffloadFlagForAllStream(sessionIDToSpatializationEnableMap, deviceType);
}

void UpdateA2dpOffloadFlagFuzzTest()
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    DeviceType deviceType = DEVICE_TYPE_BLUETOOTH_A2DP;
    std::vector<Bluetooth::A2dpStreamInfo> allActiveSessions;
    Bluetooth::A2dpStreamInfo a2dpStreamInfo;
    a2dpStreamInfo.sessionId = 1;
    a2dpStreamInfo.streamType = 1;
    a2dpStreamInfo.isSpatialAudio = false;
    allActiveSessions.push_back(a2dpStreamInfo);
    constexpr int32_t stateCount = static_cast<int32_t>(BluetoothOffloadState::A2DP_OFFLOAD) + 1;
    BluetoothOffloadState state = static_cast<BluetoothOffloadState>(GetData<int32_t>() % stateCount);
    manager->SetA2dpOffloadFlag(state);
    manager->UpdateA2dpOffloadFlag(allActiveSessions, deviceType);
}

void HandleA2dpDeviceOutOffloadFuzzTest()
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    AudioDeviceDescriptor descriptor;
    descriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    manager->audioActiveDevice_.SetCurrentOutputDevice(descriptor);
    constexpr int32_t stateCount = static_cast<int32_t>(BluetoothOffloadState::A2DP_OFFLOAD) + 1;
    BluetoothOffloadState a2dpOffloadFlag = static_cast<BluetoothOffloadState>(GetData<int32_t>() % stateCount);
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    manager->HandleA2dpDeviceOutOffload(a2dpOffloadFlag);
}

void HandleA2dpDeviceInOffloadFuzzTest()
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    constexpr int32_t stateCount = static_cast<int32_t>(BluetoothOffloadState::A2DP_OFFLOAD) + 1;
    BluetoothOffloadState state = static_cast<BluetoothOffloadState>(GetData<int32_t>() % stateCount);
    manager->SetA2dpOffloadFlag(state);
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_BLUETOOTH_A2DP;
    manager->audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
    constexpr int32_t currentOffloadConnectionStateCount =
        static_cast<int32_t>(A2dpOffloadConnectionState::CONNECTION_STATUS_TIMEOUT) + 1;
    A2dpOffloadConnectionState currentOffloadConnectionState =
        static_cast<A2dpOffloadConnectionState>(GetData<int32_t>() % currentOffloadConnectionStateCount);
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(currentOffloadConnectionState);
    constexpr int32_t a2dpOffloadFlagCount = static_cast<int32_t>(BluetoothOffloadState::A2DP_OFFLOAD) + 1;
    BluetoothOffloadState a2dpOffloadFlag =
        static_cast<BluetoothOffloadState>(GetData<int32_t>() % a2dpOffloadFlagCount);
    manager->HandleA2dpDeviceInOffload(a2dpOffloadFlag);
}

void GetA2dpOffloadCodecAndSendToDspFuzzTest()
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    AudioDeviceDescriptor deviceDescriptor;
    deviceDescriptor.deviceType_ = DEVICE_TYPE_SPEAKER;
    manager->audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
    manager->GetA2dpOffloadCodecAndSendToDsp();
}

void FetchStreamForA2dpOffloadFuzzTest()
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    auto changeInfo = std::make_shared<AudioRendererChangeInfo>();
    constexpr int32_t streamUsageCount = static_cast<int32_t>(StreamUsage::STREAM_USAGE_VOICE_CALL_ASSISTANT) + 1;
    StreamUsage streamUsage = static_cast<StreamUsage>(GetData<int32_t>() % streamUsageCount - 1);
    changeInfo->rendererInfo.streamUsage = streamUsage;
    changeInfo->clientUID = GetData<uint32_t>();
    changeInfo->rendererInfo.rendererFlags = STREAM_USAGE_UNKNOWN;
    rendererChangeInfos.push_back(changeInfo);
    manager->streamCollector_.audioRendererChangeInfos_ = rendererChangeInfos;
    bool requireReset = GetData<uint32_t>() % NUM_2;
    manager->FetchStreamForA2dpOffload(requireReset);
}

void GetVolumeGroupTypeFuzzTest()
{
    std::shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    manager->GetVolumeGroupType(deviceType);
}

void OnA2dpPlayingStateChangedFuzzTest()
{
    shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    const std::string deviceAddress = manager->a2dpOffloadDeviceAddress_;
    int32_t playingState = GetData<uint32_t>();
    constexpr int32_t currentOffloadConnectionStateCount =
        static_cast<int32_t>(A2dpOffloadConnectionState::CONNECTION_STATUS_TIMEOUT) + 1;
    A2dpOffloadConnectionState currentOffloadConnectionState =
        static_cast<A2dpOffloadConnectionState>(GetData<uint8_t>() % currentOffloadConnectionStateCount);
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(currentOffloadConnectionState);
    manager->OnA2dpPlayingStateChanged(deviceAddress, playingState);
}

void IsA2dpOffloadConnectingFuzzTest()
{
    shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    manager->connectionTriggerSessionIds_ = {123};
    constexpr int32_t currentOffloadConnectionStateCount =
        static_cast<int32_t>(A2dpOffloadConnectionState::CONNECTION_STATUS_TIMEOUT) + 1;
    A2dpOffloadConnectionState currentOffloadConnectionState =
        static_cast<A2dpOffloadConnectionState>(GetData<uint8_t>() % currentOffloadConnectionStateCount);
    manager->audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(currentOffloadConnectionState);
    manager->IsA2dpOffloadConnecting(GetData<uint32_t>());
}

void HandleActiveDeviceFuzzTest()
{
    shared_ptr<AudioA2dpOffloadManager> manager = std::make_shared<AudioA2dpOffloadManager>();
    manager->Init();
    AudioDeviceDescriptor deviceDescriptor;
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    deviceDescriptor.deviceType_ = DeviceTypeVec[deviceTypeCount];
    deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    manager->audioActiveDevice_.SetCurrentOutputDevice(deviceDescriptor);
    manager->audioConfigManager_.isUpdateRouteSupported_ = GetData<uint32_t>() % NUM_2;
    manager->HandleActiveDevice(deviceDescriptor);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    OffloadStartPlayingFuzzTest,
    OffloadStopPlayingFuzzTest,
    UpdateA2dpOffloadFlagForAllStreamFuzzTest,
    UpdateA2dpOffloadFlagFuzzTest,
    HandleA2dpDeviceOutOffloadFuzzTest,
    HandleA2dpDeviceInOffloadFuzzTest,
    GetA2dpOffloadCodecAndSendToDspFuzzTest,
    FetchStreamForA2dpOffloadFuzzTest,
    GetVolumeGroupTypeFuzzTest,
    OnA2dpPlayingStateChangedFuzzTest,
    IsA2dpOffloadConnectingFuzzTest,
    HandleActiveDeviceFuzzTest,
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
