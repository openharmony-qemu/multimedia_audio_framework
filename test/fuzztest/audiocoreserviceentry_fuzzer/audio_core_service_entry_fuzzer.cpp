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
const uint8_t TESTSIZE = 14;
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

vector<StreamUsage> StreamUsageVec = {
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

void UpdateSessionOperationFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    uint32_t sessionId = 0;
    constexpr int32_t operationCount = static_cast<int32_t>(SessionOperation::SESSION_OPERATION_RELEASE) + 1;
    SessionOperation operation = static_cast<SessionOperation>(GetData<uint8_t>() % operationCount);
    eventEntry->UpdateSessionOperation(sessionId, operation);
}

void OnServiceConnectedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    int32_t serviceIndexCount = static_cast<int32_t>(AudioServiceIndex::AUDIO_SERVICE_INDEX) + 1;
    AudioServiceIndex serviceIndex = static_cast<AudioServiceIndex>(GetData<uint8_t>() % serviceIndexCount);
    eventEntry->OnServiceConnected(serviceIndex);
}

void OnServiceDisconnectedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    int32_t serviceIndexCount = static_cast<int32_t>(AudioServiceIndex::AUDIO_SERVICE_INDEX) + 1;
    AudioServiceIndex serviceIndex = static_cast<AudioServiceIndex>(GetData<uint8_t>() % serviceIndexCount);
    eventEntry->OnServiceDisconnected(serviceIndex);
}

void CreateRendererClientFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    uint32_t audioFlag = 0;
    uint32_t sessionId = 0;
    std::string networkId = "";
    eventEntry->CreateRendererClient(streamDesc, audioFlag, sessionId, networkId);
}

void CreateCapturerClientFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    uint32_t audioFlag = 0;
    uint32_t sessionId = 0;
    eventEntry->CreateCapturerClient(streamDesc, audioFlag, sessionId);
}

void SetDefaultOutputDeviceFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    uint32_t sessionID = 0;
    uint32_t streamUsageCount = GetData<uint32_t>() % StreamUsageVec.size();
    StreamUsage streamUsage = StreamUsageVec[streamUsageCount];
    bool isRunning = true;
    eventEntry->SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning);
}

void GetAdapterNameBySessionIdFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    uint32_t sessionId = 0;
    eventEntry->GetAdapterNameBySessionId(sessionId);
}

void GetProcessDeviceInfoBySessionIdFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    uint32_t sessionId = 0;
    AudioDeviceDescriptor deviceInfo;
    auto ret = eventEntry->GetProcessDeviceInfoBySessionId(sessionId, deviceInfo);
}

void GenerateSessionIdFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    auto ret = eventEntry->GenerateSessionId();
}

void OnDeviceInfoUpdatedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioDeviceDescriptor desc;
    desc.isEnable_ = true;
    int32_t commandCount = static_cast<int32_t>(DeviceInfoUpdateCommand::EXCEPTION_FLAG_UPDATE
                                                - DeviceInfoUpdateCommand::CATEGORY_UPDATE) + 1;
    DeviceInfoUpdateCommand command = static_cast<DeviceInfoUpdateCommand>(GetData<uint8_t>() % commandCount + 1);
    eventEntry->OnDeviceInfoUpdated(desc, command);
}

void SetAudioSceneFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    int32_t audioSceneCount = static_cast<int32_t>(AudioScene::AUDIO_SCENE_MAX - AudioScene::AUDIO_SCENE_INVALID) + 1;
    AudioScene audioScene = static_cast<AudioScene>(GetData<uint8_t>() % audioSceneCount - 1);
    eventEntry->SetAudioScene(audioScene);
}

void OnDeviceStatusUpdatedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioDeviceDescriptor desc;
    bool isConnected = GetData<uint32_t>() % NUM_2;
    eventEntry->OnDeviceStatusUpdated(desc, isConnected);
}

void OnMicrophoneBlockedUpdateFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    DeviceType deviceType = DeviceTypeVec[deviceTypeCount];
    int32_t statusCount = static_cast<int32_t>(DeviceBlockStatus::DEVICE_BLOCKED) + 1;
    DeviceBlockStatus status = static_cast<DeviceBlockStatus>(GetData<uint8_t>() % statusCount);
    eventEntry->OnMicrophoneBlockedUpdate(deviceType, status);
}

void OnPnpDeviceStatusUpdatedFuzzTest()
{
    auto audioCoreService = std::make_shared<AudioCoreService>();
    auto eventEntry = std::make_shared<AudioCoreService::EventEntry>(audioCoreService);
    AudioDeviceDescriptor desc;
    uint32_t deviceTypeCount = GetData<uint32_t>() % DeviceTypeVec.size();
    desc.deviceType_ = DeviceTypeVec[deviceTypeCount];
    bool isConnected = GetData<uint32_t>() % NUM_2;
    eventEntry->OnPnpDeviceStatusUpdated(desc, isConnected);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    UpdateSessionOperationFuzzTest,
    OnServiceConnectedFuzzTest,
    OnServiceDisconnectedFuzzTest,
    CreateRendererClientFuzzTest,
    CreateCapturerClientFuzzTest,
    SetDefaultOutputDeviceFuzzTest,
    GetAdapterNameBySessionIdFuzzTest,
    GetProcessDeviceInfoBySessionIdFuzzTest,
    GenerateSessionIdFuzzTest,
    OnDeviceInfoUpdatedFuzzTest,
    SetAudioSceneFuzzTest,
    OnDeviceStatusUpdatedFuzzTest,
    OnMicrophoneBlockedUpdateFuzzTest,
    OnPnpDeviceStatusUpdatedFuzzTest,
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
