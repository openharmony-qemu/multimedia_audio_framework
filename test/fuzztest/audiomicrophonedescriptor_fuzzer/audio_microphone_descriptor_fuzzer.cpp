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
#include "audio_microphone_descriptor.h"
#include "audio_router_map.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const int32_t NUM_3 = 3;
typedef void (*TestPtr)();

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

template<class T>
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (g_dataSize < g_pos) {
        return object;
    }
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

void AudioMicrophoneDescriptorSetMicrophoneMuteFuzzTest()
{
    AudioMicrophoneDescriptor &audioMicrophoneDescriptor = AudioMicrophoneDescriptor::GetInstance();
    bool isMute = GetData<bool>();
    audioMicrophoneDescriptor.SetMicrophoneMute(isMute);
}

void AudioMicrophoneDescriptorSetMicrophoneMutePersistentFuzzTest()
{
    AudioMicrophoneDescriptor &audioMicrophoneDescriptor = AudioMicrophoneDescriptor::GetInstance();
    bool isMute = GetData<bool>();
    audioMicrophoneDescriptor.SetMicrophoneMutePersistent(isMute);
}

void AudioMicrophoneDescriptorGetPersistentMicMuteStateFuzzTest()
{
    AudioMicrophoneDescriptor &audioMicrophoneDescriptor = AudioMicrophoneDescriptor::GetInstance();

    audioMicrophoneDescriptor.GetPersistentMicMuteState();
}

void AudioMicrophoneDescriptorInitPersistentMicrophoneMuteStateFuzzTest()
{
    AudioMicrophoneDescriptor &audioMicrophoneDescriptor = AudioMicrophoneDescriptor::GetInstance();
    bool isMute = GetData<bool>();
    audioMicrophoneDescriptor.isMicrophoneMutePersistent_ = GetData<bool>();
    audioMicrophoneDescriptor.InitPersistentMicrophoneMuteState(isMute);
}

void AudioMicrophoneDescriptorIsMicrophoneMuteFuzzTest()
{
    AudioMicrophoneDescriptor &audioMicrophoneDescriptor = AudioMicrophoneDescriptor::GetInstance();
    audioMicrophoneDescriptor.IsMicrophoneMute();
}

void AudioMicrophoneDescriptorGetMicrophoneMuteTemporaryFuzzTest()
{
    AudioMicrophoneDescriptor &audioMicrophoneDescriptor = AudioMicrophoneDescriptor::GetInstance();
    audioMicrophoneDescriptor.GetMicrophoneMuteTemporary();
    audioMicrophoneDescriptor.GetMicrophoneMutePersistent();
}

void AudioMicrophoneDescriptorAddAudioCapturerMicrophoneDescriptorFuzzTest()
{
    AudioMicrophoneDescriptor &audioMicrophoneDescriptor = AudioMicrophoneDescriptor::GetInstance();
    if (g_testDeviceTypes.size() == 0) {
        return;
    }
    DeviceType devType = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    int32_t sessionId = GetData<int32_t>();
    sptr<MicrophoneDescriptor> microphoneDescriptor = new MicrophoneDescriptor();
    audioMicrophoneDescriptor.connectedMicrophones_.push_back(microphoneDescriptor);
    audioMicrophoneDescriptor.AddAudioCapturerMicrophoneDescriptor(sessionId, devType);
}

void AudioMicrophoneDescriptorGetAudioCapturerMicrophoneDescriptorsFuzzTest()
{
    AudioMicrophoneDescriptor &audioMicrophoneDescriptor = AudioMicrophoneDescriptor::GetInstance();
    int32_t sessionId = GetData<int32_t>();
    sptr<MicrophoneDescriptor> microphoneDescriptor = new MicrophoneDescriptor();
    audioMicrophoneDescriptor.audioCaptureMicrophoneDescriptor_.clear();
    audioMicrophoneDescriptor.audioCaptureMicrophoneDescriptor_.insert({sessionId, microphoneDescriptor});
    audioMicrophoneDescriptor.GetAudioCapturerMicrophoneDescriptors(sessionId);
}

void AudioMicrophoneDescriptorUpdateAudioCapturerMicrophoneDescriptorFuzzTest()
{
    AudioMicrophoneDescriptor &audioMicrophoneDescriptor = AudioMicrophoneDescriptor::GetInstance();
    if (g_testDeviceTypes.size() == 0) {
        return;
    }
    DeviceType devType = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    sptr<MicrophoneDescriptor> microphoneDescriptor = new MicrophoneDescriptor();
    audioMicrophoneDescriptor.connectedMicrophones_.push_back(microphoneDescriptor);
    microphoneDescriptor->deviceType_ = devType;
    int32_t sessionId = GetData<int32_t>();
    audioMicrophoneDescriptor.audioCaptureMicrophoneDescriptor_.clear();
    audioMicrophoneDescriptor.audioCaptureMicrophoneDescriptor_.insert({sessionId, microphoneDescriptor});
    audioMicrophoneDescriptor.UpdateAudioCapturerMicrophoneDescriptor(devType);
    audioMicrophoneDescriptor.GetAvailableMicrophones();
}

void AudioMicrophoneDescriptorRemoveAudioCapturerMicrophoneDescriptorFuzzTest()
{
    AudioMicrophoneDescriptor &audioMicrophoneDescriptor = AudioMicrophoneDescriptor::GetInstance();
    int32_t uid = GetData<int32_t>();
    audioMicrophoneDescriptor.RemoveAudioCapturerMicrophoneDescriptor(uid);
}

void AudioRouteMapGetDeviceInfoByUidAndPidFuzzTest()
{
    AudioRouteMap &audioRouteMap = AudioRouteMap::GetInstance();
    int32_t uid = GetData<int32_t>();
    int32_t pid = GetData<int32_t>();
    uint32_t index = GetData<uint32_t>() % NUM_3;
    audioRouteMap.routerMap_.clear();
    if (index == 0) {
        audioRouteMap.routerMap_.insert({uid, {"testNetworkId", pid}});
    } else if (index == 1) {
        audioRouteMap.routerMap_.insert({uid, {"testNetworkId", -1}});
    } else {
        audioRouteMap.routerMap_.insert({uid, {"testNetworkId", GetData<int32_t>()}});
    }
    audioRouteMap.GetDeviceInfoByUidAndPid(uid, pid);
}

void AudioRouteMapDelRouteMapInfoByKeyFuzzTest()
{
    AudioRouteMap &audioRouteMap = AudioRouteMap::GetInstance();
    int32_t uid = GetData<int32_t>();
    audioRouteMap.routerMap_.clear();
    audioRouteMap.routerMap_.insert({uid, {"testNetworkId", GetData<int32_t>()}});

    audioRouteMap.DelRouteMapInfoByKey(uid);
}

void AudioRouteMapAddRouteMapInfoFuzzTest()
{
    AudioRouteMap &audioRouteMap = AudioRouteMap::GetInstance();
    int32_t uid = GetData<int32_t>();
    int32_t pid = GetData<int32_t>();
    std::string device = "testDevice";

    audioRouteMap.AddRouteMapInfo(uid, device, pid);
}

void AudioRouteMapAddFastRouteMapInfoFuzzTest()
{
    static const vector<DeviceRole> testDeviceRoles = {
        DEVICE_ROLE_NONE,
        INPUT_DEVICE,
        OUTPUT_DEVICE,
        DEVICE_ROLE_MAX,
    };
    if (testDeviceRoles.size() == 0) {
        return;
    }
    AudioRouteMap &audioRouteMap = AudioRouteMap::GetInstance();
    int32_t uid = GetData<int32_t>();
    std::string device = "testDevice";
    DeviceRole role = testDeviceRoles[GetData<uint32_t>() % testDeviceRoles.size()];

    audioRouteMap.AddFastRouteMapInfo(uid, device, role);
}

void AudioRouteMapRemoveDeviceInRouterMapFuzzTest()
{
    AudioRouteMap &audioRouteMap = AudioRouteMap::GetInstance();
    audioRouteMap.routerMap_.clear();
    int32_t uid = GetData<int32_t>();
    std::string networkId = "testNetworkId";
    audioRouteMap.routerMap_.insert({uid, {"testNetworkId1", GetData<int32_t>()}});
    audioRouteMap.routerMap_.insert({uid + 1, {"testNetworkId", GetData<int32_t>()}});

    audioRouteMap.RemoveDeviceInRouterMap(networkId);
}

void AudioRouteMapRemoveDeviceInFastRouterMapFuzzTest()
{
    AudioRouteMap &audioRouteMap = AudioRouteMap::GetInstance();
    audioRouteMap.fastRouterMap_.clear();
    int32_t uid = GetData<int32_t>();
    std::string networkId = "testNetworkId";
    audioRouteMap.fastRouterMap_.insert({uid, {networkId, DEVICE_ROLE_NONE}});
    audioRouteMap.fastRouterMap_.insert({uid + 1, {"testNetworkId1", DEVICE_ROLE_NONE}});

    audioRouteMap.RemoveDeviceInFastRouterMap(networkId);
}

void AudioRouteMapGetNetworkIDInFastRouterMapFuzzTest()
{
    AudioRouteMap &audioRouteMap = AudioRouteMap::GetInstance();
    audioRouteMap.fastRouterMap_.clear();
    int32_t uid = GetData<int32_t>();
    std::string networkId = "testNetworkId";
    audioRouteMap.fastRouterMap_.insert({uid, {networkId, DEVICE_ROLE_NONE}});
    audioRouteMap.fastRouterMap_.insert({uid + 1, {"testNetworkId1", DEVICE_ROLE_NONE}});

    audioRouteMap.GetNetworkIDInFastRouterMap(uid, DEVICE_ROLE_NONE, networkId);
}

TestPtr g_testPtrs[] = {
    AudioMicrophoneDescriptorSetMicrophoneMuteFuzzTest,
    AudioMicrophoneDescriptorSetMicrophoneMutePersistentFuzzTest,
    AudioMicrophoneDescriptorGetPersistentMicMuteStateFuzzTest,
    AudioMicrophoneDescriptorInitPersistentMicrophoneMuteStateFuzzTest,
    AudioMicrophoneDescriptorIsMicrophoneMuteFuzzTest,
    AudioMicrophoneDescriptorGetMicrophoneMuteTemporaryFuzzTest,
    AudioMicrophoneDescriptorAddAudioCapturerMicrophoneDescriptorFuzzTest,
    AudioMicrophoneDescriptorGetAudioCapturerMicrophoneDescriptorsFuzzTest,
    AudioMicrophoneDescriptorUpdateAudioCapturerMicrophoneDescriptorFuzzTest,
    AudioMicrophoneDescriptorRemoveAudioCapturerMicrophoneDescriptorFuzzTest,
    AudioRouteMapGetDeviceInfoByUidAndPidFuzzTest,
    AudioRouteMapDelRouteMapInfoByKeyFuzzTest,
    AudioRouteMapAddRouteMapInfoFuzzTest,
    AudioRouteMapAddFastRouteMapInfoFuzzTest,
    AudioRouteMapRemoveDeviceInRouterMapFuzzTest,
    AudioRouteMapRemoveDeviceInFastRouterMapFuzzTest,
    AudioRouteMapGetNetworkIDInFastRouterMapFuzzTest,
};

void FuzzTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr) {
        return;
    }

    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testPtrs);
    if (len > 0) {
        g_testPtrs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }
    return;
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