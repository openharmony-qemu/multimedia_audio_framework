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

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 2;
bool g_isInit = false;
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
static AudioPolicyServer g_server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

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

const vector<StreamUsage> StreamUsageVec = {
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

const vector<SourceType> SourceTypeVec = {
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

AudioPolicyServer *GetServerPtr()
{
    if (!g_isInit) {
        g_server.OnStart();
        g_server.OnAddSystemAbility(DISTRIBUTED_KV_DATA_SERVICE_ABILITY_ID, "");
        g_server.OnAddSystemAbility(DISTRIBUTED_HARDWARE_DEVICEMANAGER_SA_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        g_server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        g_server.OnAddSystemAbility(COMMON_EVENT_SERVICE_ID, "");
        g_server.OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
        g_server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
        g_server.OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        g_server.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        g_server.OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        g_server.OnAddSystemAbility(ACCESSIBILITY_MANAGER_SERVICE_ID, "");
#ifdef USB_ENABLE
        g_server.OnAddSystemAbility(USB_SYSTEM_ABILITY_ID, "");
#endif
        g_server.audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_isInit = true;
    }
    return &g_server;
}

void GetMediaRenderDeviceFuzzTest()
{
    auto streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    uint32_t streamUsageCount = GetData<uint32_t>() % StreamUsageVec.size();
    StreamUsage streamUsage = StreamUsageVec[streamUsageCount];
    int32_t clientId = 1;
    int32_t typeCount = static_cast<int32_t>(CastType::CAST_TYPE_COOPERATION) + 1;
    CastType type = static_cast<CastType>(GetData<uint8_t>() % typeCount);
    auto server = GetServerPtr();
    server->audioPolicyService_.StoreDistributedRoutingRoleInfo(deviceDescriptor, type);
    streamFilterRouter_->GetCallRenderDevice(streamUsage, clientId);
    streamFilterRouter_->GetMediaRenderDevice(streamUsage, clientId);
    g_server.OnStop();
    g_isInit = false;
}

void GetRecordCaptureDeviceFuzzTest()
{
    auto streamFilterRouter_ = std::make_shared<StreamFilterRouter>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDescriptor = std::make_shared<AudioDeviceDescriptor>();
    uint32_t sourceTypeCount = GetData<uint32_t>() % SourceTypeVec.size();
    SourceType sourceType = SourceTypeVec[sourceTypeCount];
    int32_t clientId = 1;
    int32_t typeCount = static_cast<int32_t>(CastType::CAST_TYPE_COOPERATION) + 1;
    CastType type = static_cast<CastType>(GetData<uint8_t>() % typeCount);
    auto server = GetServerPtr();
    server->audioPolicyService_.StoreDistributedRoutingRoleInfo(deviceDescriptor, type);
    streamFilterRouter_->GetCallCaptureDevice(sourceType, clientId);
    streamFilterRouter_->GetRecordCaptureDevice(sourceType, clientId);
    g_server.OnStop();
    g_isInit = false;
}

TestFuncs g_testFuncs[] = {
    GetMediaRenderDeviceFuzzTest,
    GetRecordCaptureDeviceFuzzTest,
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
