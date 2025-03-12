/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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
#include <string>

#include "audio_manager_base.h"
#include "audio_policy_manager_listener_stub.h"
#include "audio_server.h"
#include "message_parcel.h"
#include "audio_process_in_client.h"
#include "audio_param_parser.h"
#include "audio_process_config.h"
#include "ipc_stream_in_server.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IStandardAudioService";
const int32_t SYSTEM_ABILITY_ID = 3001;
const bool RUN_ON_CREATE = false;
const int32_t LIMITSIZE = 4;
const uint32_t COMMON_SIZE = 2;
const int32_t COMMON_INT = 2;
const std::u16string COMMONU16STRTEST = u"Test";
const uint32_t IOPERTAION_LENGTH = 13;
const uint32_t ENUM_LENGTH = 15;
const uint32_t ENUM_LENGTH_1 = 6;
const uint32_t ENUM_LENGTH_2 = 1;
const uint32_t APPID_LENGTH = 10;
const uint64_t COMMON_UINT64_NUM = 2;
const uint32_t RES_TYPE_AUDIO_RENDERER_STANDBY = 119;
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

/*
* describe: get data from outside untrusted data(RAW_DATA) which size is according to sizeof(T)
* tips: only support basic type
*/
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

void AudioServerSetSpatializationSceneTypeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    uint32_t sizeMs = *reinterpret_cast<const uint32_t*>(rawData);
    data.WriteUint32(sizeMs);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_SPATIALIZATION_SCENE_TYPE),
        data, reply, option);
}

void AudioServerUpdateSpatialDeviceTypeTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    MessageParcel reply;
    MessageOption option;
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    uint32_t sizeMs = *reinterpret_cast<const uint32_t*>(rawData);
    data.WriteUint32(sizeMs);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);

    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_SPATIAL_DEVICE_TYPE),
        data, reply, option);
}

void AudioServerLoadConfigurationTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }
    std::unordered_map<std::string, std::unordered_map<std::string, std::set<std::string>>> audioParameterKeys = {};
    std::unordered_map<std::string, std::set<std::string>> audioParameterKey = {};
    std::set<std::string> audioParameterValue = {};
    std::string audioParameterKeyStr1 = "key1";
    std::string audioParameterKeyStr2 = "key2";
    std::string audioParameterKeyValueStr1 = "value1";
    std::string audioParameterKeyValueStr2 = "value2";
    audioParameterValue.insert(audioParameterKeyValueStr1);
    audioParameterValue.insert(audioParameterKeyValueStr2);
    audioParameterKey.insert(std::make_pair(audioParameterKeyStr1, audioParameterValue));
    audioParameterKey.insert(std::make_pair(audioParameterKeyStr2, audioParameterValue));
    audioParameterKeys.insert(std::make_pair(audioParameterKeyStr1, audioParameterKey));
    audioParameterKeys.insert(std::make_pair(audioParameterKeyStr2, audioParameterKey));

    std::shared_ptr<AudioParamParser> audioParamParser = std::make_shared<AudioParamParser>();
    audioParamParser->LoadConfiguration(audioParameterKeys);
}

void AudioServerGetExtarAudioParametersTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string mainKey = "mainKey";
    std::string value1 = "value1";
    std::string value2 = "value2";
    std::vector<std::string> subkeys = {};
    subkeys.push_back(value1);
    subkeys.push_back(value2);

    data.WriteString(static_cast<std::string>(mainKey));
    data.WriteInt32(subkeys.size());
    for (std::string subKey : subkeys) {
        data.WriteString(static_cast<std::string>(subKey));
    }

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_EXTRA_AUDIO_PARAMETERS),
        data, reply, option);
}

void AudioServerSetExtraAudioParametersTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::string mainKey = "mainKey";
    std::vector<std::pair<std::string, std::string>> kvpairs;
    for (uint32_t i = 0; i < COMMON_SIZE; i++) {
        std::string subKey = "subKey" + std::to_string(i);
        std::string subValue = "subValue" + std::to_string(i);
        kvpairs.push_back(std::make_pair(subKey, subValue));
    }

    data.WriteString(mainKey);
    data.WriteInt32(static_cast<int32_t>(kvpairs.size()));
    for (auto it = kvpairs.begin(); it != kvpairs.end(); it++) {
        data.WriteString(static_cast<std::string>(it->first));
        data.WriteString(static_cast<std::string>(it->second));
    }

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_EXTRA_AUDIO_PARAMETERS),
        data, reply, option);
}

void AudioServerUpdateRouteReqTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);

    DeviceType type = *reinterpret_cast<const DeviceType*>(rawData);
    DeviceFlag flag = *reinterpret_cast<const DeviceFlag*>(rawData);
    BluetoothOffloadState a2dpOffloadFlag = *reinterpret_cast<const BluetoothOffloadState*>(rawData);
    data.WriteInt32(type);
    data.WriteInt32(flag);
    data.WriteInt32(static_cast<int32_t>(a2dpOffloadFlag));

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_ROUTE_REQ),
        data, reply, option);
}

void AudioServerUpdateActiveDevicesRouteTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
    for (uint32_t i = 0; i < COMMON_SIZE; i++) {
        DeviceType deviceType = *reinterpret_cast<const DeviceType*>(rawData);
        DeviceFlag deviceFlag = *reinterpret_cast<const DeviceFlag*>(rawData);
        activeDevices.push_back(std::make_pair(deviceType, deviceFlag));
    }
    data.WriteInt32(static_cast<int32_t>(activeDevices.size()));
    for (auto it = activeDevices.begin(); it != activeDevices.end(); it++) {
        data.WriteInt32(static_cast<int32_t>(it->first));
        data.WriteInt32(static_cast<int32_t>(it->second));
    }

    BluetoothOffloadState a2dpOffloadFlag = *reinterpret_cast<const BluetoothOffloadState*>(rawData);
    data.WriteInt32(static_cast<int32_t>(a2dpOffloadFlag));

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_ROUTES_REQ),
        data, reply, option);
}

void AudioServerUpdateDualToneStateTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    bool enable = *reinterpret_cast<const bool*>(rawData);
    int32_t sessionId = *reinterpret_cast<const int32_t*>(rawData);

    data.WriteBool(enable);
    data.WriteInt32(sessionId);

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_DUAL_TONE_REQ),
        data, reply, option);
}

void AudioServerGetTransactionIdTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    DeviceType deviceType = *reinterpret_cast<const DeviceType*>(rawData);
    DeviceRole deviceRole = *reinterpret_cast<const DeviceRole*>(rawData);
    data.WriteInt32(deviceType);
    data.WriteInt32(deviceRole);
    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_TRANSACTION_ID),
        data, reply, option);
}

void AudioGetAudioParameterTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(AudioParamKey)) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);

    std::string networkId(reinterpret_cast<const char*>(rawData), size);
    std::string condition(reinterpret_cast<const char*>(rawData), size);
    AudioParamKey key = *reinterpret_cast<const AudioParamKey*>(rawData);
    data.WriteString(networkId);
    data.WriteInt32(static_cast<int32_t>(key));
    data.WriteString(condition);

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::GET_REMOTE_AUDIO_PARAMETER),
        data, reply, option);
}

void AudioSetAudioParameterTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE || size < sizeof(AudioParamKey)) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);

    std::string networkId = "123";
    std::string condition = "123456";
    std::string value = "123456";
    AudioParamKey key = *reinterpret_cast<const AudioParamKey*>(rawData);
    data.WriteString(networkId);
    data.WriteInt32(static_cast<uint32_t>(key));
    data.WriteString(condition);
    data.WriteString(value);

    std::shared_ptr<AudioServer> AudioServerPtr = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    AudioServerPtr->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::SET_REMOTE_AUDIO_PARAMETER),
        data, reply, option);
}

void AudioCreateAudioProcessTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    AudioProcessConfig config;
    config.appInfo.appUid = APPID_LENGTH;
    config.appInfo.appPid = APPID_LENGTH;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_RECORD;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;

    ProcessConfig::WriteConfigToParcel(config, data);

    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    audioServer->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_AUDIOPROCESS),
        data, reply, option);
}

void AudioLoadAudioEffectLibrariesTest(const uint8_t* rawData, size_t size)
{
    if (rawData == nullptr || size < LIMITSIZE) {
        return;
    }

    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteInt32(COMMON_INT);
    data.WriteInt32(COMMON_INT);
    for (int32_t i = 0; i < COMMON_INT; i++) {
        std::string libName(reinterpret_cast<const char*>(rawData), size - 1);
        std::string libPath(reinterpret_cast<const char*>(rawData), size - 1);
        data.WriteString(libName);
        data.WriteString(libPath);
    }

    for (int32_t i = 0; i < COMMON_INT; i++) {
        std::string effectName = "effectName" + std::to_string(i);
        std::string libName = "libName" + std::to_string(i);
        data.WriteString(effectName);
        data.WriteString(libName);
    }

    std::shared_ptr<AudioServer> audioServer = std::make_shared<AudioServer>(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    MessageParcel reply;
    MessageOption option;
    audioServer->OnRemoteRequest(static_cast<uint32_t>(AudioServerInterfaceCode::LOAD_AUDIO_EFFECT_LIBRARIES),
        data, reply, option);
}

void AudioCapturerInServerFuzzTest()
{
    std::shared_ptr<CapturerInServer> capturerInServer = nullptr;
    AudioProcessConfig config;
    config.appInfo.appUid = APPID_LENGTH;
    config.appInfo.appPid = APPID_LENGTH;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    std::weak_ptr<IStreamListener> innerListener;
    capturerInServer = std::make_shared<CapturerInServer>(config, innerListener);
    if (capturerInServer == nullptr) {
        return;
    }
    uint32_t operationCode = GetData<uint32_t>();
    operationCode = (operationCode % IOPERTAION_LENGTH) - 1;
    IOperation operation = static_cast<IOperation>(operationCode);
    capturerInServer->OnStatusUpdate(operation);
    std::shared_ptr<OHAudioBuffer> buffer = nullptr;
    capturerInServer->ResolveBuffer(buffer);
    uint32_t sessionId = GetData<uint32_t>();
    capturerInServer->GetSessionId(sessionId);
}

void AudioRendererInServerTestFirst(std::shared_ptr<RendererInServer> renderer)
{
    uint32_t operationCode = GetData<uint32_t>();
    operationCode = (operationCode % IOPERTAION_LENGTH) - 1;
    IOperation operation = static_cast<IOperation>(operationCode);
    renderer->OnStatusUpdate(operation);
    renderer->HandleOperationFlushed();
    std::shared_ptr<OHAudioBuffer> buffer = nullptr;
    renderer->ResolveBuffer(buffer);
    uint32_t sessionId = GetData<uint32_t>();
    renderer->GetSessionId(sessionId);
    uint64_t framePos = COMMON_UINT64_NUM;
    uint64_t timeStamp = COMMON_UINT64_NUM;
    uint64_t latency = COMMON_UINT64_NUM;
    renderer->GetAudioTime(framePos, timeStamp);
    renderer->GetAudioPosition(framePos, timeStamp, latency);
    renderer->GetLatency(latency);
    int32_t rate = GetData<int32_t>();
    renderer->SetRate(rate);
    float volume = GetData<float>();
    renderer->SetLowPowerVolume(volume);
    renderer->GetLowPowerVolume(volume);
    int32_t effectMode = GetData<int32_t>();
    renderer->SetAudioEffectMode(effectMode);
    renderer->GetAudioEffectMode(effectMode);
    int32_t privacyType = GetData<int32_t>();
    renderer->SetPrivacyType(privacyType);
    renderer->GetPrivacyType(privacyType);
    int32_t state = GetData<int32_t>();
    bool isAppBack = GetData<bool>();
    renderer->SetOffloadMode(state, isAppBack);
    renderer->UnsetOffloadMode();
}

void AudioRendererInServerTestSecond(std::shared_ptr<RendererInServer> renderer)
{
    bool isAppBack = GetData<bool>();
    bool headTrackingEnabled = GetData<bool>();
    RestoreInfo restoreInfo;
    restoreInfo.restoreReason = static_cast<RestoreReason>(GetData<int32_t>());
    restoreInfo.targetStreamFlag = GetData<int32_t>();
    renderer->UpdateSpatializationState(isAppBack, headTrackingEnabled);
    renderer->CheckAndWriterRenderStreamStandbySysEvent(GetData<bool>());
    uint64_t timeStamp = COMMON_UINT64_NUM;
    renderer->GetOffloadApproximatelyCacheTime(timeStamp, timeStamp, timeStamp, timeStamp);
    BufferDesc desc;
    desc.buffer = nullptr;
    desc.bufLength = 0;
    desc.dataLength =0;
    renderer->VolumeHandle(desc);
    renderer->WriteData();
    renderer->WriteEmptyData();
    renderer->DrainAudioBuffer();
    renderer->EnableInnerCap(1);
    renderer->DisableInnerCap(1);
    renderer->InitDupStream(1);
    renderer->EnableDualTone();
    renderer->DisableDualTone();
    renderer->InitDualToneStream();
    renderer->GetStreamManagerType();
    renderer->SetSilentModeAndMixWithOthers(isAppBack);
    renderer->SetClientVolume();
    uint32_t operationCode = GetData<uint32_t>();
    operationCode = (operationCode % IOPERTAION_LENGTH) - 1;
    IOperation operation = static_cast<IOperation>(operationCode);
    renderer->OnDataLinkConnectionUpdate(operation);
    std::string dumpString = "";
    renderer->managerType_ = DIRECT_PLAYBACK;
    renderer->Dump(dumpString);
    bool muteFlag = false;
    renderer->SetNonInterruptMute(muteFlag);
    renderer->RestoreSession(restoreInfo);
    renderer->Pause();
    renderer->Flush();
    renderer->Drain(headTrackingEnabled);
    renderer->Stop();
    renderer->Release();
}

void AudioRendererInServerTest()
{
    AudioProcessConfig config;
    config.appInfo.appUid = APPID_LENGTH;
    config.appInfo.appPid = APPID_LENGTH;
    config.streamInfo.format = SAMPLE_S32LE;
    config.streamInfo.samplingRate = SAMPLE_RATE_48000;
    config.streamInfo.channels = STEREO;
    config.streamInfo.channelLayout = AudioChannelLayout::CH_LAYOUT_STEREO;
    config.audioMode = AudioMode::AUDIO_MODE_PLAYBACK;
    config.streamType = AudioStreamType::STREAM_MUSIC;
    config.deviceType = DEVICE_TYPE_USB_HEADSET;
    int32_t ret = 0;
    sptr<IpcStreamInServer> ipcStreamInServer = IpcStreamInServer::Create(config, ret);
    if (ipcStreamInServer == nullptr) {
        return;
    }
    std::shared_ptr<RendererInServer> renderer = ipcStreamInServer->GetRenderer();
    renderer->Init();
    renderer->Start();
    renderer->ConfigServerBuffer();
    renderer->InitBufferStatus();
    renderer->UpdateWriteIndex();
    uint32_t statusInt = GetData<uint32_t>();
    statusInt = (statusInt % ENUM_LENGTH) -ENUM_LENGTH_2;
    IStatus status = static_cast<IStatus>(statusInt);
    uint32_t typeInt = GetData<uint32_t>();
    typeInt = typeInt % ENUM_LENGTH_1;
    ManagerType type = static_cast<ManagerType>(typeInt);
    renderer->managerType_ = type;
    renderer->status_ = status;
    std::string dumpString = "";
    renderer->Dump(dumpString);
    renderer->SetStreamVolumeInfoForEnhanceChain();
    std::unordered_map<std::string, std::string> payload;
    renderer->ReportDataToResSched(payload, RES_TYPE_AUDIO_RENDERER_STANDBY);
    AudioRendererInServerTestFirst(renderer);
    AudioRendererInServerTestSecond(renderer);
}

void AudioMicroPhoneFuzzTest()
{
    sptr<MicrophoneDescriptor> micDesc = new (std::nothrow) MicrophoneDescriptor();
    MicrophoneDescriptor micDescs;
    Vector3D vector3d;
    vector3d.x = 0.0f;
    vector3d.y = 0.0f;
    vector3d.z = 0.0f;
    micDesc->SetMicPositionInfo(vector3d);
    micDesc->SetMicOrientationInfo(vector3d);
}

typedef void (*TestFuncs[3])();

TestFuncs g_testFuncs = {
    AudioCapturerInServerFuzzTest,
    AudioRendererInServerTest,
    AudioMicroPhoneFuzzTest,
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
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    OHOS::AudioStandard::AudioServerSetSpatializationSceneTypeTest(data, size);
    OHOS::AudioStandard::AudioServerUpdateSpatialDeviceTypeTest(data, size);
    OHOS::AudioStandard::AudioServerLoadConfigurationTest(data, size);
    OHOS::AudioStandard::AudioServerGetExtarAudioParametersTest(data, size);
    OHOS::AudioStandard::AudioServerSetExtraAudioParametersTest(data, size);
    OHOS::AudioStandard::AudioServerUpdateRouteReqTest(data, size);
    OHOS::AudioStandard::AudioServerUpdateActiveDevicesRouteTest(data, size);
    OHOS::AudioStandard::AudioServerUpdateDualToneStateTest(data, size);
    OHOS::AudioStandard::AudioServerGetTransactionIdTest(data, size);
    OHOS::AudioStandard::AudioSetAudioParameterTest(data, size);
    OHOS::AudioStandard::AudioGetAudioParameterTest(data, size);
    OHOS::AudioStandard::AudioCreateAudioProcessTest(data, size);
    OHOS::AudioStandard::AudioLoadAudioEffectLibrariesTest(data, size);
    return 0;
}
