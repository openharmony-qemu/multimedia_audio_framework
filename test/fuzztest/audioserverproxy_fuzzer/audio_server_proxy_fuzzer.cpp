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
#include "audio_server_proxy.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
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
    DEVICE_TYPE_MAX
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

void AudioServerProxyGetEffectOffloadEnabledProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    audioServerProxy.GetEffectOffloadEnabledProxy();
}

void AudioServerProxyUpdateDualToneStateProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    bool enable = GetData<bool>();
    int32_t sessionId = GetData<int32_t>();
    audioServerProxy.UpdateDualToneStateProxy(enable, sessionId);
}

void AudioServerProxyUpdateSessionConnectionStateProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    int32_t sessionID = GetData<int32_t>();
    int32_t state = GetData<int32_t>();
    audioServerProxy.UpdateSessionConnectionStateProxy(sessionID, state);
}

void AudioServerProxyCheckRemoteDeviceStateProxyFuzzTest()
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
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    string networkId = "testNetworkId";
    bool isStartDevice = GetData<bool>();
    DeviceRole deviceRole = testDeviceRoles[GetData<uint32_t>() % testDeviceRoles.size()];
    audioServerProxy.CheckRemoteDeviceStateProxy(networkId, deviceRole, isStartDevice);
}

void AudioServerProxyResetAudioEndpointProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    audioServerProxy.ResetAudioEndpointProxy();
}

void AudioServerProxyOffloadSetVolumeProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    float volume = GetData<float>();
    std::string deviceClass = "testDeviceClass";
    std::string networkId = "testNetworkId";
    audioServerProxy.OffloadSetVolumeProxy(volume, deviceClass, networkId);
}

void AudioServerProxyUnsetOffloadModeProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    uint32_t sessionId = GetData<uint32_t>();
    audioServerProxy.UnsetOffloadModeProxy(sessionId);
}

void AudioServerProxySetOffloadModeProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    uint32_t sessionId = GetData<uint32_t>();
    int32_t state = GetData<int32_t>();
    bool isAppBack = GetData<bool>();
    audioServerProxy.SetOffloadModeProxy(sessionId, state, isAppBack);
}

void AudioServerProxyCheckHibernateStateProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    bool hibernate = GetData<bool>();
    audioServerProxy.CheckHibernateStateProxy(hibernate);
}

void AudioServerProxySetAudioEnhancePropertyProxyFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) {
        return;
    }
    DeviceType deviceType = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    AudioEnhancePropertyArray propertyArray;
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    audioServerProxy.SetAudioEnhancePropertyProxy(propertyArray, deviceType);
}

void AudioServerProxySetSinkMuteForSwitchDeviceProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    std::string devceClass = "testDeviceClass";
    int32_t durationUs = GetData<int32_t>();
    bool mute = GetData<bool>();
    audioServerProxy.SetSinkMuteForSwitchDeviceProxy(devceClass, durationUs, mute);
}

void AudioServerProxySuspendRenderSinkProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    std::string sinkName = "testSinkName";
    audioServerProxy.SuspendRenderSinkProxy(sinkName);
}

void AudioServerProxyNotifyDeviceInfoProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    std::string networkId = "testNetworkId";
    bool connected = GetData<bool>();
    audioServerProxy.NotifyDeviceInfoProxy(networkId, connected);
}

void AudioServerProxyCreatePlaybackCapturerManagerProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    audioServerProxy.CreatePlaybackCapturerManagerProxy();
}

void AudioServerProxySetAudioEffectPropertyProxyFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) {
        return;
    }
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    DeviceType deviceType = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    AudioEffectPropertyArrayV3 propertyArrayV3;
    audioServerProxy.SetAudioEffectPropertyProxy(propertyArrayV3, deviceType);
    AudioEffectPropertyArray propertyArray;
    audioServerProxy.SetAudioEffectPropertyProxy(propertyArray);
}

void AudioServerProxySetRotationToEffectProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    uint32_t rotate = GetData<uint32_t>();
    audioServerProxy.SetRotationToEffectProxy(rotate);
}

void AudioServerProxySetAudioMonoStateProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    bool audioMono = GetData<bool>();
    audioServerProxy.SetAudioMonoStateProxy(audioMono);
}

void AudioServerProxySetAudioBalanceValueProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    float audioBalance = GetData<float>();
    audioServerProxy.SetAudioBalanceValueProxy(audioBalance);
}

void AudioServerProxyNotifyAccountsChangedFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    audioServerProxy.NotifyAccountsChanged();
    audioServerProxy.NotifySettingsDataReady();
}

void AudioServerProxyCreateHdiSinkPortProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    std::string deviceClass = "testDeviceClass";
    std::string idInfo = "testIdInfo";
    IAudioSinkAttr attr;
    audioServerProxy.CreateHdiSinkPortProxy(deviceClass, idInfo, attr);
}

void AudioServerProxyCreateHdiSourcePortProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    std::string deviceClass = "testDeviceClass";
    std::string idInfo = "testIdInfo";
    IAudioSourceAttr attr;
    audioServerProxy.CreateHdiSourcePortProxy(deviceClass, idInfo, attr);
}

void AudioServerProxyDestroyHdiPortProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    uint32_t id = GetData<uint32_t>();
    audioServerProxy.DestroyHdiPortProxy(id);
}

void AudioServerProxySetDeviceConnectedFlagFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    bool flag = GetData<bool>();
    audioServerProxy.SetDeviceConnectedFlag(flag);
}

void AudioServerProxySetLatestMuteStateFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    uint32_t sessionId = GetData<uint32_t>();
    bool muteFlag = GetData<bool>();
    audioServerProxy.SetLatestMuteState(sessionId, muteFlag);
}

void AudioServerProxyGetAudioEnhancePropertyProxyFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) {
        return;
    }
    DeviceType deviceType = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    AudioEnhancePropertyArray propertyArray;
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    audioServerProxy.GetAudioEnhancePropertyProxy(propertyArray, deviceType);
}

void AudioServerProxyGetAudioEffectPropertyProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    AudioEffectPropertyArrayV3 propertyArrayV3;
    audioServerProxy.GetAudioEffectPropertyProxy(propertyArrayV3);
}
 
void AudioServerProxyIsAcousticEchoCancelerSupportedFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    SourceType sourceType = SOURCE_TYPE_VOICE_RECOGNITION;
    audioServerProxy.IsAcousticEchoCancelerSupported(sourceType);
}
 
void AudioServerProxySetKaraokeParametersFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    std::string parameters = "testParameters";
    audioServerProxy.SetKaraokeParameters(parameters);
}
 
void AudioServerProxyIsAudioLoopbackSupportedFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    AudioLoopbackMode mode = LOOPBACK_HARDWARE;
    audioServerProxy.IsAudioLoopbackSupported(mode);
}
 
void AudioServerProxySetSessionMuteStateFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    uint32_t sessionId = GetData<uint32_t>();
    bool insert = GetData<bool>();
    bool muteFlag = GetData<bool>();
    audioServerProxy.SetSessionMuteState(sessionId, insert, muteFlag);
}
 
void AudioServerProxySetActiveOutputDeviceProxyFuzzTest()
{
    if (g_testDeviceTypes.size() == 0) {
        return;
    }
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    DeviceType deviceType = g_testDeviceTypes[GetData<uint32_t>() % g_testDeviceTypes.size()];
    audioServerProxy.SetActiveOutputDeviceProxy(deviceType);
}
 
void AudioServerProxyForceStopAudioStreamProxyFuzzTest()
{
    AudioServerProxy &audioServerProxy = AudioServerProxy::GetInstance();
    StopAudioType audioType = STOP_RENDER;
    audioServerProxy.ForceStopAudioStreamProxy(audioType);
}

TestPtr g_testPtrs[] = {
    AudioServerProxyGetEffectOffloadEnabledProxyFuzzTest,
    AudioServerProxyUpdateDualToneStateProxyFuzzTest,
    AudioServerProxyUpdateSessionConnectionStateProxyFuzzTest,
    AudioServerProxyCheckRemoteDeviceStateProxyFuzzTest,
    AudioServerProxyResetAudioEndpointProxyFuzzTest,
    AudioServerProxyOffloadSetVolumeProxyFuzzTest,
    AudioServerProxyUnsetOffloadModeProxyFuzzTest,
    AudioServerProxySetOffloadModeProxyFuzzTest,
    AudioServerProxyCheckHibernateStateProxyFuzzTest,
    AudioServerProxySetAudioEnhancePropertyProxyFuzzTest,
    AudioServerProxySetSinkMuteForSwitchDeviceProxyFuzzTest,
    AudioServerProxySuspendRenderSinkProxyFuzzTest,
    AudioServerProxyNotifyDeviceInfoProxyFuzzTest,
    AudioServerProxyCreatePlaybackCapturerManagerProxyFuzzTest,
    AudioServerProxySetAudioEffectPropertyProxyFuzzTest,
    AudioServerProxySetRotationToEffectProxyFuzzTest,
    AudioServerProxySetAudioMonoStateProxyFuzzTest,
    AudioServerProxySetAudioBalanceValueProxyFuzzTest,
    AudioServerProxyNotifyAccountsChangedFuzzTest,
    AudioServerProxyCreateHdiSinkPortProxyFuzzTest,
    AudioServerProxyCreateHdiSourcePortProxyFuzzTest,
    AudioServerProxyDestroyHdiPortProxyFuzzTest,
    AudioServerProxySetDeviceConnectedFlagFuzzTest,
    AudioServerProxySetLatestMuteStateFuzzTest,
    AudioServerProxyGetAudioEnhancePropertyProxyFuzzTest,
    AudioServerProxyGetAudioEffectPropertyProxyFuzzTest,
    AudioServerProxyIsAcousticEchoCancelerSupportedFuzzTest,
    AudioServerProxySetKaraokeParametersFuzzTest,
    AudioServerProxyIsAudioLoopbackSupportedFuzzTest,
    AudioServerProxySetSessionMuteStateFuzzTest,
    AudioServerProxySetActiveOutputDeviceProxyFuzzTest,
    AudioServerProxyForceStopAudioStreamProxyFuzzTest,
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