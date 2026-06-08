/*
* Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "audio_log.h"
#include "audio_zone.h"
#include "audio_zone_client_manager.h"
#include "audio_zone_service.h"
#include <fuzzer/FuzzedDataProvider.h>
using namespace std;

namespace OHOS {
namespace AudioStandard {

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
typedef void (*TestPtr)(FuzzedDataProvider&);

template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

template<typename T>
T PickValue(FuzzedDataProvider& fdp, const vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& fallback)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return value.empty() ? fallback : value;
}

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

const vector<RouterType> g_testRouterTypes = {
    ROUTER_TYPE_NONE,
    ROUTER_TYPE_DEFAULT,
    ROUTER_TYPE_STREAM_FILTER,
    ROUTER_TYPE_PACKAGE_FILTER,
    ROUTER_TYPE_COCKPIT_PHONE,
    ROUTER_TYPE_PRIVACY_PRIORITY,
    ROUTER_TYPE_PUBLIC_PRIORITY,
    ROUTER_TYPE_PAIR_DEVICE,
    ROUTER_TYPE_USER_SELECT,
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
    DEVICE_TYPE_MAX,
};

const vector<AudioStreamType> g_testAudioStreamTypes = {
    STREAM_DEFAULT,
    STREAM_VOICE_CALL,
    STREAM_MUSIC,
    STREAM_RING,
    STREAM_MEDIA,
    STREAM_VOICE_ASSISTANT,
    STREAM_SYSTEM,
    STREAM_ALARM,
    STREAM_NOTIFICATION,
    STREAM_BLUETOOTH_SCO,
    STREAM_ENFORCED_AUDIBLE,
    STREAM_DTMF,
    STREAM_TTS,
    STREAM_RECORDING,
    STREAM_MOVIE,
    STREAM_GAME,
    STREAM_SPEECH,
    STREAM_SYSTEM_ENFORCED,
    STREAM_ULTRASONIC,
    STREAM_WAKEUP,
    STREAM_VOICE_MESSAGE,
    STREAM_NAVIGATION,
    STREAM_INTERNAL_FORCE_STOP,
    STREAM_SOURCE_VOICE_CALL,
    STREAM_VOICE_COMMUNICATION,
    STREAM_VOICE_RING,
    STREAM_VOICE_CALL_ASSISTANT,
    STREAM_CAMCORDER,
    STREAM_APP,
    STREAM_TYPE_MAX,
    STREAM_ALL,
};

void AudioZoneBindKeyAudioZoneBindKeyFuzzTest(FuzzedDataProvider& fdp)
{
    std::string deviceTag = ConsumeString(fdp, "testDeviceTag");
    uint32_t streamId = fdp.ConsumeIntegral<uint32_t>();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    AudioZoneBindKey audioZoneBindKey_1(uid);
    AudioZoneBindKey audioZoneBindKey_2(uid, deviceTag);
    AudioZoneBindKey audioZoneBindKey_3(streamId);
    AudioZoneBindKey audioZoneBindKey_4(audioZoneBindKey_1);
    AudioZoneBindKey audioZoneBindKey_5(std::move(audioZoneBindKey_1));
}

void AudioZoneBindKeyOperatorFuzzTest(FuzzedDataProvider& fdp)
{
    std::string deviceTag = ConsumeString(fdp, "testDeviceTag");
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    AudioZoneBindKey audioZoneBindKey_1(uid, deviceTag);
    AudioZoneBindKey audioZoneBindKey_2 = audioZoneBindKey_1;
}

void AudioZoneBindKeyGetUidFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    AudioZoneBindKey audioZoneBindKey(uid);
    audioZoneBindKey.GetUid();
}

void AudioZoneBindKeyGetStringFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    AudioZoneBindKey audioZoneBindKey(uid);
    audioZoneBindKey.GetString();
}

void AudioZoneBindKeyIsContainFuzzTest(FuzzedDataProvider& fdp)
{
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    AudioZoneBindKey audioZoneBindKey(uid);
    AudioZoneBindKey audioZoneBindKey1(uid);
    audioZoneBindKey.IsContain(audioZoneBindKey1);
}

void AudioZoneBindKeyGetSupportKeysFuzzTest(FuzzedDataProvider& fdp)
{
    std::string deviceTag = ConsumeString(fdp, "testDeviceTag");
    uint32_t streamId = fdp.ConsumeIntegral<uint32_t>();
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    StreamUsage usage = PickValue(fdp, g_testStreamUsages);
    AudioZoneBindKey audioZoneBindKey(uid, deviceTag);
    audioZoneBindKey.GetSupportKeys(audioZoneBindKey);
    audioZoneBindKey.GetSupportKeys(uid, deviceTag, streamId, usage);
}

void AudioZoneGetDescriptorFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    zone->GetDescriptor();
}

void AudioZoneGetStringDescriptorFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    zone->GetStringDescriptor();
}

void AudioZoneGetDescriptorNoLockFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    zone->GetDescriptorNoLock();
}

void AudioZoneBindByKeyFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    AudioZoneBindKey audioZoneBindKey(uid);
    zone->BindByKey(audioZoneBindKey);
}

void AudioZoneRemoveKeyFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    AudioZoneBindKey audioZoneBindKey(uid);
    zone->RemoveKey(audioZoneBindKey);
}

void AudioZoneIsContainKeyFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    int32_t uid = fdp.ConsumeIntegral<int32_t>();
    AudioZoneBindKey audioZoneBindKey(uid);
    zone->IsContainKey(audioZoneBindKey);
}

void AudioZoneRemoveDeviceDescriptorFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    devices.push_back(device);
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    zone->RemoveDeviceDescriptor(devices);
}

void AudioZoneAddDeviceDescriptorFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    devices.push_back(device);
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    zone->AddDeviceDescriptor(devices);
}

void AudioZoneUpdateDeviceDescriptorFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    devices.push_back(device);
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    zone->AddDeviceDescriptor(devices);
    zone->UpdateDeviceDescriptor(device);
}

void AudioZoneEnableDeviceDescriptorFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    devices.push_back(device);
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    zone->AddDeviceDescriptor(devices);
    zone->EnableDeviceDescriptor(device);
}

void AudioZoneDisableDeviceDescriptorFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    devices.push_back(device);
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    zone->AddDeviceDescriptor(devices);
    zone->DisableDeviceDescriptor(device);
}

void AudioZoneIsDeviceConnectFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices;
    auto device = std::make_shared<AudioDeviceDescriptor>();
    devices.push_back(device);
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    zone->AddDeviceDescriptor(devices);
    zone->IsDeviceConnect(device);
}

void AudioZoneFetchOutputDevicesFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    StreamUsage streamUsage = PickValue(fdp, g_testStreamUsages);
    int32_t clientUid = fdp.ConsumeIntegral<int32_t>();
    RouterType bypassType = PickValue(fdp, g_testRouterTypes);
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    zone->FetchOutputDevices(streamUsage, clientUid, bypassType);
}

void AudioZoneFetchInputDeviceFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    SourceType sourceType = PickValue(fdp, g_testSourceTypes);
    int32_t clientUid = fdp.ConsumeIntegral<int32_t>();
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    zone->FetchInputDevice(sourceType, clientUid);
}

void AudioZoneEnableChangeReportFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    bool enable = fdp.ConsumeBool();
    pid_t clientPid = fdp.ConsumeIntegral<pid_t>();
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    zone->EnableChangeReport(clientPid, enable);
}

void AudioZoneEnableSystemVolumeProxyFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    bool enable = fdp.ConsumeBool();
    pid_t clientPid = fdp.ConsumeIntegral<pid_t>();
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    zone->EnableSystemVolumeProxy(clientPid, deviceType, enable);
}

void AudioZoneSetSystemVolumeLevelFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    AudioVolumeType volumeProxyClientPid =
        static_cast<AudioVolumeType>(PickValue(fdp, g_testAudioStreamTypes));
    int32_t volumeLevel = fdp.ConsumeIntegral<int32_t>();
    int32_t volumeFlag = fdp.ConsumeIntegral<int32_t>();
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    zone->SetSystemVolumeLevel(deviceType, volumeProxyClientPid, volumeLevel, volumeFlag);
}

void AudioZoneGetSystemVolumeLevelFuzzTest(FuzzedDataProvider& fdp)
{
    std::string name = ConsumeString(fdp, "testAudioZone");
    AudioVolumeType volumeProxyClientPid =
        static_cast<AudioVolumeType>(PickValue(fdp, g_testAudioStreamTypes));
    AudioZoneContext context;
    std::shared_ptr<AudioZoneClientManager> zoneClientManager;
    std::shared_ptr<AudioZone> zone =
        std::make_shared<AudioZone>(zoneClientManager, name, context);
    DeviceType deviceType = PickValue(fdp, g_testDeviceTypes);
    zone->GetSystemVolumeLevel(deviceType, volumeProxyClientPid);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    AudioZoneBindKeyAudioZoneBindKeyFuzzTest,
    AudioZoneBindKeyOperatorFuzzTest,
    AudioZoneBindKeyGetUidFuzzTest,
    AudioZoneBindKeyGetStringFuzzTest,
    AudioZoneBindKeyIsContainFuzzTest,
    AudioZoneBindKeyGetSupportKeysFuzzTest,
    AudioZoneGetDescriptorFuzzTest,
    AudioZoneGetStringDescriptorFuzzTest,
    AudioZoneGetDescriptorNoLockFuzzTest,
    AudioZoneBindByKeyFuzzTest,
    AudioZoneRemoveKeyFuzzTest,
    AudioZoneIsContainKeyFuzzTest,
    AudioZoneRemoveDeviceDescriptorFuzzTest,
    AudioZoneAddDeviceDescriptorFuzzTest,
    AudioZoneEnableDeviceDescriptorFuzzTest,
    AudioZoneDisableDeviceDescriptorFuzzTest,
    AudioZoneIsDeviceConnectFuzzTest,
    AudioZoneFetchOutputDevicesFuzzTest,
    AudioZoneFetchInputDeviceFuzzTest,
    AudioZoneEnableChangeReportFuzzTest,
    AudioZoneEnableSystemVolumeProxyFuzzTest,
    AudioZoneSetSystemVolumeLevelFuzzTest,
    AudioZoneGetSystemVolumeLevelFuzzTest,
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
extern "C" int LLVMFuzzerInitialize(int* argc, char*** argv)
{
    OHOS::AudioStandard::Init();
    return 0;
}