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
const uint8_t TESTSIZE = 9;
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

void RegisterTrackerFuzzTest()
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    int32_t modeCount = static_cast<int32_t>(AudioMode::AUDIO_MODE_RECORD) + 1;
    AudioMode mode = static_cast<AudioMode>(GetData<uint8_t>() % modeCount);
    AudioStreamChangeInfo streamChangeInfo;
    sptr<IRemoteObject> object = nullptr;
    int32_t apiVersion = GetData<int32_t>();
    audioDeviceLock->RegisterTracker(mode, streamChangeInfo, object, apiVersion);
}

void SendA2dpConnectedWhileRunningFuzzTest()
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    int32_t rendererStateCount =
        static_cast<int32_t>(RendererState::RENDERER_PAUSED - RendererState::RENDERER_INVALID) + 1;
    RendererState rendererState = static_cast<RendererState>(GetData<uint8_t>() % rendererStateCount - 1);
    uint32_t sessionId = GetData<uint32_t>();
    audioDeviceLock->audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    audioDeviceLock->SendA2dpConnectedWhileRunning(rendererState, sessionId);
}

void HandleAudioCaptureStateFuzzTest()
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    int32_t modeCount = static_cast<int32_t>(AudioMode::AUDIO_MODE_RECORD) + 1;
    AudioMode mode = static_cast<AudioMode>(GetData<uint8_t>() % modeCount);
    AudioStreamChangeInfo streamChangeInfo;
    int32_t capturerStateCount = static_cast<int32_t>(CapturerState::CAPTURER_PAUSED) + 1;
    streamChangeInfo.audioCapturerChangeInfo.capturerState =
        static_cast<CapturerState>(GetData<uint8_t>() % capturerStateCount);
    uint32_t sourceTypeCount = GetData<uint32_t>() % SourceTypeVec.size();
    streamChangeInfo.audioCapturerChangeInfo.capturerInfo.sourceType = SourceTypeVec[sourceTypeCount];
    audioDeviceLock->HandleAudioCaptureState(mode, streamChangeInfo);
}

void UpdateTrackerFuzzTest()
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    int32_t modeCount = static_cast<int32_t>(AudioMode::AUDIO_MODE_RECORD) + 1;
    AudioMode mode = static_cast<AudioMode>(GetData<uint8_t>() % modeCount);
    AudioStreamChangeInfo streamChangeInfo;
    int32_t rendererStateCount =
        static_cast<int32_t>(RendererState::RENDERER_PAUSED - RendererState::RENDERER_INVALID) + 1;
    streamChangeInfo.audioRendererChangeInfo.rendererState =
        static_cast<RendererState>(GetData<uint8_t>() % rendererStateCount - 1);
    auto ret = audioDeviceLock->UpdateTracker(mode, streamChangeInfo);
}

void RegisteredTrackerClientDiedFuzzTest()
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    int32_t uidCount = static_cast<int32_t>(AudioPipeType::PIPE_TYPE_DIRECT_VOIP) + 1;
    pid_t uid = static_cast<pid_t>(GetData<uint8_t>() % uidCount);
    audioDeviceLock->RegisteredTrackerClientDied(uid);
}

void OnDeviceStatusUpdatedFuzzTest()
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    AudioDeviceDescriptor updatedDesc;
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    updatedDesc.deviceType_ = DeviceTypeVec[deviceTypeCount];
    int32_t connectStateCount = static_cast<int32_t>(ConnectState::DEACTIVE_CONNECTED) + 1;
    updatedDesc.connectState_ = static_cast<ConnectState>(GetData<uint8_t>() % connectStateCount);
    bool isConnected = GetData<uint32_t>() % NUM_2;
    audioDeviceLock->OnDeviceStatusUpdated(updatedDesc, isConnected);
}

void GetCurrentRendererChangeInfosFuzzTest()
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos = {
        std::make_shared<AudioRendererChangeInfo>()
    };
    bool hasBTPermission = GetData<uint32_t>() % NUM_2;
    bool hasSystemPermission = GetData<uint32_t>() % NUM_2;
    audioDeviceLock->GetCurrentRendererChangeInfos(audioRendererChangeInfos, hasBTPermission, hasSystemPermission);
}

void GetVolumeGroupInfosFuzzTest()
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    audioDeviceLock->audioVolumeManager_.isPrimaryMicModuleInfoLoaded_.store(GetData<uint32_t>() % NUM_2);
    audioDeviceLock->GetVolumeGroupInfos();
}

void SetAudioSceneFuzzTest()
{
    auto audioDeviceLock = std::make_shared<AudioDeviceLock>();
    int32_t audioSceneCount = static_cast<int32_t>(AudioScene::AUDIO_SCENE_MAX - AudioScene::AUDIO_SCENE_INVALID) + 1;
    AudioScene audioScene = static_cast<AudioScene>(GetData<uint8_t>() % audioSceneCount - 1);
    audioDeviceLock->SetAudioScene(audioScene);
    audioDeviceLock->SetAudioScene(audioScene);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    RegisterTrackerFuzzTest,
    SendA2dpConnectedWhileRunningFuzzTest,
    HandleAudioCaptureStateFuzzTest,
    UpdateTrackerFuzzTest,
    RegisteredTrackerClientDiedFuzzTest,
    OnDeviceStatusUpdatedFuzzTest,
    GetCurrentRendererChangeInfosFuzzTest,
    GetVolumeGroupInfosFuzzTest,
    SetAudioSceneFuzzTest,
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
