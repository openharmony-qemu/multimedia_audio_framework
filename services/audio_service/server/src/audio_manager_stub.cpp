/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#ifndef LOG_TAG
#define LOG_TAG "AudioManagerStub"
#endif

#include "audio_manager_base.h"
#include "audio_system_manager.h"
#include "audio_service_log.h"
#include "i_audio_process.h"
#include "audio_effect_server.h"
#include "audio_asr.h"
#include "audio_utils.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t AUDIO_EXTRA_PARAMETERS_COUNT_UPPER_LIMIT = 40;
constexpr int32_t AUDIO_EFFECT_CHAIN_CONFIG_UPPER_LIMIT = 64;
constexpr int32_t AUDIO_EFFECT_CHAIN_COUNT_UPPER_LIMIT = 32;
constexpr int32_t AUDIO_EFFECT_COUNT_PER_CHAIN_UPPER_LIMIT = 16;
constexpr int32_t AUDIO_EFFECT_PRIOR_SCENE_UPPER_LIMIT = 7;
constexpr int32_t AUDIO_EFFECT_COUNT_PROPERTY_UPPER_LIMIT = 20;
#ifndef HAS_FEATURE_INNERCAPTURER
constexpr int32_t ERROR = -1;
#endif

const char *g_audioServerCodeStrs[] = {
    "GET_AUDIO_PARAMETER",
    "SET_AUDIO_PARAMETER",
    "GET_EXTRA_AUDIO_PARAMETERS",
    "SET_EXTRA_AUDIO_PARAMETERS",
    "SET_MICROPHONE_MUTE",
    "SET_AUDIO_SCENE",
    "UPDATE_ROUTE_REQ",
    "UPDATE_ROUTES_REQ",
    "UPDATE_DUAL_TONE_REQ",
    "GET_TRANSACTION_ID",
    "SET_PARAMETER_CALLBACK",
    "GET_REMOTE_AUDIO_PARAMETER",
    "SET_REMOTE_AUDIO_PARAMETER",
    "NOTIFY_DEVICE_INFO",
    "CHECK_REMOTE_DEVICE_STATE",
    "SET_VOICE_VOLUME",
    "SET_AUDIO_MONO_STATE",
    "SET_AUDIO_BALANCE_VALUE",
    "CREATE_AUDIOPROCESS",
    "LOAD_AUDIO_EFFECT_LIBRARIES",
    "CREATE_AUDIO_EFFECT_CHAIN_MANAGER",
    "SET_OUTPUT_DEVICE_SINK",
    "SET_ACTIVE_OUTPUT_DEVICE",
    "CREATE_PLAYBACK_CAPTURER_MANAGER",
    "REGISET_POLICY_PROVIDER",
    "SET_WAKEUP_CLOSE_CALLBACK",
    "SET_CAPTURE_SILENT_STATE",
    "UPDATE_SPATIALIZATION_STATE",
    "UPDATE_SPATIAL_DEVICE_TYPE",
    "OFFLOAD_SET_VOLUME",
    "NOTIFY_STREAM_VOLUME_CHANGED",
    "SET_SPATIALIZATION_SCENE_TYPE",
    "GET_MAX_AMPLITUDE",
    "RESET_AUDIO_ENDPOINT",
    "RESET_ROUTE_FOR_DISCONNECT",
    "GET_EFFECT_LATENCY",
    "UPDATE_LATENCY_TIMESTAMP",
    "SET_ASR_AEC_MODE",
    "GET_ASR_AEC_MODE",
    "SET_ASR_NOISE_SUPPRESSION_MODE",
    "SET_OFFLOAD_MODE",
    "UNSET_OFFLOAD_MODE",
    "CHECK_HIBERNATE_STATE",
    "GET_ASR_NOISE_SUPPRESSION_MODE",
    "SET_ASR_WHISPER_DETECTION_MODE",
    "GET_ASR_WHISPER_DETECTION_MODE",
    "SET_ASR_VOICE_CONTROL_MODE",
    "SET_ASR_VOICE_MUTE_MODE",
    "IS_WHISPERING",
    "GET_EFFECT_OFFLOAD_ENABLED",
    "SUSPEND_RENDERSINK",
    "RESTORE_RENDERSINK",
    "LOAD_HDI_EFFECT_MODEL",
    "GET_AUDIO_EFFECT_PROPERTY_V3",
    "SET_AUDIO_EFFECT_PROPERTY_V3",
    "GET_AUDIO_ENHANCE_PROPERTY",
    "GET_AUDIO_EFFECT_PROPERTY",
    "SET_AUDIO_ENHANCE_PROPERTY",
    "SET_AUDIO_EFFECT_PROPERTY",
    "UPDATE_EFFECT_BT_OFFLOAD_SUPPORTED",
    "SET_SINK_MUTE_FOR_SWITCH_DEVICE",
    "SET_ROTATION_TO_EFFECT",
    "UPDATE_SESSION_CONNECTION_STATE",
    "SET_SINGLE_STREAM_MUTE",
    "RESTORE_SESSION",
    "GET_ALL_SINK_INPUTS",
    "SET_DEFAULT_ADAPTER_ENABLE",
    "CREATE_IPC_OFFLINE_STREAM",
    "GET_OFFLINE_AUDIO_EFFECT_CHAINS",
    "GET_STANDBY_STATUS",
    "GENERATE_SESSION_ID",
    "NOTIFY_ACCOUNTS_CHANGED",
    "NOTIFY_AUDIO_POLICY_READY",
    "SET_CAPTURE_LIMIT",
    "LOAD_HDI_ADAPTER",
    "UNLOAD_HDI_ADAPTER",
    "CHECK_CAPTURE_LIMIT",
    "RELEASE_CAPTURE_LIMIT",
    "DEVICE_CONNECTED_FLAG",
    "NOTIFY_SETTINGS_DATA_READY",
};
constexpr size_t codeNums = sizeof(g_audioServerCodeStrs) / sizeof(const char *);
static_assert(codeNums == (static_cast<size_t> (AudioServerInterfaceCode::AUDIO_SERVER_CODE_MAX) + 1),
    "keep same with AudioServerInterfaceCode");
}
static void LoadEffectLibrariesReadData(vector<Library>& libList, vector<Effect>& effectList, MessageParcel &data,
    int32_t countLib, int32_t countEff)
{
    int32_t i;
    for (i = 0; i < countLib; i++) {
        string libName = data.ReadString();
        string libPath = data.ReadString();
        libList.push_back({libName, libPath});
    }
    for (i = 0; i < countEff; i++) {
        string effectName = data.ReadString();
        string libName = data.ReadString();
        effectList.push_back({effectName, libName});
    }
}

static void LoadEffectLibrariesWriteReply(const vector<Effect>& successEffectList, MessageParcel &reply)
{
    reply.WriteInt32(successEffectList.size());
    for (Effect effect: successEffectList) {
        reply.WriteString(effect.name);
        reply.WriteString(effect.libraryName);
    }
}

int AudioManagerStub::HandleGetAudioParameter(MessageParcel &data, MessageParcel &reply)
{
    const std::string key = data.ReadString();
    const std::string value = GetAudioParameter(key);
    reply.WriteString(value);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetAudioParameter(MessageParcel &data, MessageParcel &reply)
{
    const std::string key = data.ReadString();
    const std::string value = data.ReadString();
    SetAudioParameter(key, value);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetAsrAecMode(MessageParcel &data, MessageParcel &reply)
{
    AsrAecMode asrAecMode = (static_cast<AsrAecMode>(data.ReadInt32()));
    int32_t result = SetAsrAecMode(asrAecMode);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleGetAsrAecMode(MessageParcel &data, MessageParcel &reply)
{
    AsrAecMode asrAecMode = (static_cast<AsrAecMode>(data.ReadInt32()));
    int32_t ret = GetAsrAecMode(asrAecMode);
    CHECK_AND_RETURN_RET_LOG(ret == 0, AUDIO_ERR, "Get AsrAec Mode audio parameters failed");
    reply.WriteInt32(int32_t(asrAecMode));
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetOffloadMode(MessageParcel &data, MessageParcel &reply)
{
    uint32_t sessionId = data.ReadUint32();
    int32_t state = data.ReadInt32();
    bool isAppBack = data.ReadBool();
    int32_t result = SetOffloadMode(sessionId, state, isAppBack);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleUnsetOffloadMode(MessageParcel &data, MessageParcel &reply)
{
    uint32_t sessionId = data.ReadUint32();
    int32_t result = UnsetOffloadMode(sessionId);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleCheckHibernateState(MessageParcel &data, MessageParcel &reply)
{
    bool onHibernate = data.ReadBool();
    CheckHibernateState(onHibernate);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetAsrNoiseSuppressionMode(MessageParcel &data, MessageParcel &reply)
{
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(data.ReadInt32()));
    int32_t result = SetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleGetAsrNoiseSuppressionMode(MessageParcel &data, MessageParcel &reply)
{
    AsrNoiseSuppressionMode asrNoiseSuppressionMode = (static_cast<AsrNoiseSuppressionMode>(data.ReadInt32()));
    int32_t ret = GetAsrNoiseSuppressionMode(asrNoiseSuppressionMode);
    CHECK_AND_RETURN_RET_LOG(ret == 0, AUDIO_ERR, "Get AsrNoiseSuppression Mode audio parameters failed");
    reply.WriteInt32(int32_t(asrNoiseSuppressionMode));
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetAsrWhisperDetectionMode(MessageParcel &data, MessageParcel &reply)
{
    AsrWhisperDetectionMode asrWhisperDetectionMode = (static_cast<AsrWhisperDetectionMode>(data.ReadInt32()));
    int32_t result = SetAsrWhisperDetectionMode(asrWhisperDetectionMode);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleGetAsrWhisperDetectionMode(MessageParcel &data, MessageParcel &reply)
{
    AsrWhisperDetectionMode asrWhisperDetectionMode = (static_cast<AsrWhisperDetectionMode>(data.ReadInt32()));
    int32_t ret = GetAsrWhisperDetectionMode(asrWhisperDetectionMode);
    CHECK_AND_RETURN_RET_LOG(ret == 0, AUDIO_ERR, "Get AsrWhisperDetection Mode audio parameters failed");
    reply.WriteInt32(int32_t(asrWhisperDetectionMode));
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetAsrVoiceControlMode(MessageParcel &data, MessageParcel &reply)
{
    AsrVoiceControlMode asrVoiceControlMode = (static_cast<AsrVoiceControlMode>(data.ReadInt32()));
    bool on = data.ReadBool();
    int32_t result = SetAsrVoiceControlMode(asrVoiceControlMode, on);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetAsrVoiceMuteMode(MessageParcel &data, MessageParcel &reply)
{
    AsrVoiceMuteMode asrVoiceMuteMode = (static_cast<AsrVoiceMuteMode>(data.ReadInt32()));
    bool on = data.ReadBool();
    int32_t result = SetAsrVoiceMuteMode(asrVoiceMuteMode, on);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleIsWhispering(MessageParcel &data, MessageParcel &reply)
{
    const std::string key = data.ReadString();
    const std::string value = data.ReadString();
    int32_t result = IsWhispering();
    return result;
}

int AudioManagerStub::HandleGetEffectOffloadEnabled(MessageParcel &data, MessageParcel &reply)
{
    bool result = GetEffectOffloadEnabled();
    return result;
}

int AudioManagerStub::HandleGetExtraAudioParameters(MessageParcel &data, MessageParcel &reply)
{
    const std::string mainKey = data.ReadString();
    int32_t num = data.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(num >= 0 && num <= AUDIO_EXTRA_PARAMETERS_COUNT_UPPER_LIMIT,
        AUDIO_ERR, "Get extra audio parameters failed");
    std::vector<std::string> subKeys = {};
    for (int32_t i = 0; i < num; i++) {
        std::string subKey = data.ReadString();
        subKeys.push_back(subKey);
    }

    std::vector<std::pair<std::string, std::string>> values;
    int32_t result = GetExtraParameters(mainKey, subKeys, values);
    reply.WriteInt32(static_cast<int32_t>(values.size()));
    for (auto it = values.begin(); it != values.end(); it++) {
        reply.WriteString(static_cast<std::string>(it->first));
        reply.WriteString(static_cast<std::string>(it->second));
    }
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetExtraAudioParameters(MessageParcel &data, MessageParcel &reply)
{
    const std::string mainKey = data.ReadString();
    std::vector<std::pair<std::string, std::string>> audioParametersSubKVPairs;
    int32_t mapSize = data.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(mapSize >= 0 && mapSize <= AUDIO_EXTRA_PARAMETERS_COUNT_UPPER_LIMIT,
        AUDIO_ERR, "Set extra audio parameters failed");
    for (int32_t i = 0; i < mapSize; i++) {
        std::string subKey = data.ReadString();
        std::string value = data.ReadString();
        audioParametersSubKVPairs.push_back(std::make_pair(subKey, value));
    }
    int32_t result = SetExtraParameters(mainKey, audioParametersSubKVPairs);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetMicrophoneMute(MessageParcel &data, MessageParcel &reply)
{
    bool isMute = data.ReadBool();
    int32_t result = SetMicrophoneMute(isMute);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetAudioScene(MessageParcel &data, MessageParcel &reply)
{
    AudioScene audioScene = (static_cast<AudioScene>(data.ReadInt32()));
    std::vector<DeviceType> activeOutputDevices;
    int32_t vecSize = data.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(vecSize > 0 && static_cast<size_t>(vecSize) <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        AUDIO_ERR, "HandleSetAudioScene failed");
    for (int32_t i = 0; i < vecSize; i++) {
        DeviceType deviceType = (static_cast<DeviceType>(data.ReadInt32()));
        activeOutputDevices.push_back(deviceType);
    }
    DeviceType activeInputDevice = (static_cast<DeviceType>(data.ReadInt32()));
    BluetoothOffloadState a2dpOffloadFlag =  static_cast<BluetoothOffloadState>(data.ReadInt32());
    int32_t result = SetAudioScene(audioScene, activeOutputDevices, activeInputDevice, a2dpOffloadFlag);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleUpdateActiveDeviceRoute(MessageParcel &data, MessageParcel &reply)
{
    DeviceType type = static_cast<DeviceType>(data.ReadInt32());
    DeviceFlag flag = static_cast<DeviceFlag>(data.ReadInt32());
    BluetoothOffloadState a2dpOffloadFlag =  static_cast<BluetoothOffloadState>(data.ReadInt32());
    int32_t ret = UpdateActiveDeviceRoute(type, flag, a2dpOffloadFlag);
    reply.WriteInt32(ret);
    return AUDIO_OK;
}

int AudioManagerStub::HandleUpdateActiveDevicesRoute(MessageParcel &data, MessageParcel &reply)
{
    std::vector<std::pair<DeviceType, DeviceFlag>> activeDevices;
    int32_t vecSize = data.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(vecSize > 0 && static_cast<size_t>(vecSize) <= AUDIO_CONCURRENT_ACTIVE_DEVICES_LIMIT,
        AUDIO_ERR, "HandleUpdateActiveDevicesRoute failed");
    for (int32_t i = 0; i < vecSize; i++) {
        DeviceType deviceType = (static_cast<DeviceType>(data.ReadInt32()));
        DeviceFlag deviceFlag = (static_cast<DeviceFlag>(data.ReadInt32()));
        activeDevices.push_back(std::make_pair(deviceType, deviceFlag));
    }
    BluetoothOffloadState a2dpOffloadFlag =  static_cast<BluetoothOffloadState>(data.ReadInt32());
    std::string deviceName = data.ReadString();
    int32_t ret = UpdateActiveDevicesRoute(activeDevices, a2dpOffloadFlag, deviceName);
    reply.WriteInt32(ret);
    return AUDIO_OK;
}

int AudioManagerStub::HandleDualToneState(MessageParcel &data, MessageParcel &reply)
{
    bool enable = data.ReadBool();
    int32_t sessionId = data.ReadInt32();

    int32_t ret = UpdateDualToneState(enable, sessionId);
    reply.WriteInt32(ret);
    return AUDIO_OK;
}

int AudioManagerStub::HandleGetTransactionId(MessageParcel &data, MessageParcel &reply)
{
    DeviceType deviceType = (static_cast<DeviceType>(data.ReadInt32()));
    DeviceRole deviceRole = (static_cast<DeviceRole>(data.ReadInt32()));
    uint64_t transactionId = GetTransactionId(deviceType, deviceRole);
    reply.WriteUint64(transactionId);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetParameterCallback(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, AUDIO_ERR, "SET_PARAMETER_CALLBACK obj is null");
    int32_t result = SetParameterCallback(object);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleGetRemoteAudioParameter(MessageParcel &data, MessageParcel &reply)
{
    const std::string networkId = data.ReadString();
    AudioParamKey key = static_cast<AudioParamKey>(data.ReadInt32());
    const std::string condition = data.ReadString();
    const std::string value = GetAudioParameter(networkId, key, condition);
    reply.WriteString(value);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetRemoteAudioParameter(MessageParcel &data, MessageParcel &reply)
{
    const std::string networkId = data.ReadString();
    AudioParamKey key = static_cast<AudioParamKey>(data.ReadInt32());
    const std::string condtion = data.ReadString();
    const std::string value = data.ReadString();
    SetAudioParameter(networkId, key, condtion, value);
    return AUDIO_OK;
}

int AudioManagerStub::HandleNotifyDeviceInfo(MessageParcel &data, MessageParcel &reply)
{
    const std::string networkId = data.ReadString();
    const bool connected = data.ReadBool();
    NotifyDeviceInfo(networkId, connected);
    return AUDIO_OK;
}

int AudioManagerStub::HandleCheckRemoteDeviceState(MessageParcel &data, MessageParcel &reply)
{
    std::string networkId = data.ReadString();
    DeviceRole deviceRole = static_cast<DeviceRole>(data.ReadInt32());
    bool isStartDevice = data.ReadBool();
    int32_t result = CheckRemoteDeviceState(networkId, deviceRole, isStartDevice);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetVoiceVolume(MessageParcel &data, MessageParcel &reply)
{
    const float volume = data.ReadFloat();
    int32_t result = SetVoiceVolume(volume);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetAudioMonoState(MessageParcel &data, MessageParcel &reply)
{
    bool audioMonoState = data.ReadBool();
    SetAudioMonoState(audioMonoState);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetAudioBalanceValue(MessageParcel &data, MessageParcel &reply)
{
    float audioBalanceValue = data.ReadFloat();
    SetAudioBalanceValue(audioBalanceValue);
    return AUDIO_OK;
}

int AudioManagerStub::HandleCreateAudioProcess(MessageParcel &data, MessageParcel &reply)
{
    AudioProcessConfig config;
    ProcessConfig::ReadConfigFromParcel(config, data);
    int32_t errorCode = 0;
    AudioPlaybackCaptureConfig filterConfig;
    ProcessConfig::ReadInnerCapConfigFromParcel(filterConfig, data);
    sptr<IRemoteObject> process = CreateAudioProcess(config, errorCode, filterConfig);
    if (process == nullptr) {
        AUDIO_ERR_LOG("CREATE_AUDIOPROCESS AudioManagerStub CreateAudioProcess failed");
        if (errorCode == 0) {
            errorCode = AUDIO_ERR;
        }
        reply.WriteInt32(errorCode);
        // Only ipc failed need return err. Here using errorCode send information.
        return AUDIO_OK;
    }

    reply.WriteInt32(errorCode);
    reply.WriteRemoteObject(process);
    return AUDIO_OK;
}

int AudioManagerStub::HandleLoadAudioEffectLibraries(MessageParcel &data, MessageParcel &reply)
{
    vector<Library> libList = {};
    vector<Effect> effectList = {};
    int32_t countLib = data.ReadInt32();
    int32_t countEff = data.ReadInt32();
    CHECK_AND_RETURN_RET_LOG((countLib >= 0) && (countLib <= AUDIO_EFFECT_COUNT_UPPER_LIMIT) &&
        (countEff >= 0) && (countEff <= AUDIO_EFFECT_COUNT_UPPER_LIMIT), AUDIO_ERR,
        "LOAD_AUDIO_EFFECT_LIBRARIES read data failed");
    LoadEffectLibrariesReadData(libList, effectList, data, countLib, countEff);
    if (countLib > 0) {
        // load lib and reply success list
        vector<Effect> successEffectList = {};
        bool loadSuccess = LoadAudioEffectLibraries(libList, effectList, successEffectList);
        CHECK_AND_RETURN_RET_LOG(loadSuccess, AUDIO_ERR, "Load audio effect libraries failed, please check log");
        LoadEffectLibrariesWriteReply(successEffectList, reply);
    }
    return AUDIO_OK;
}

static bool UnmarshallEffectChainMgrParam(EffectChainManagerParam &effectChainMgrParam, MessageParcel &data)
{
    effectChainMgrParam.maxExtraNum = static_cast<uint32_t>(data.ReadInt32());
    effectChainMgrParam.defaultSceneName = data.ReadString();

    int32_t containSize = data.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(containSize >= 0 && containSize <= AUDIO_EFFECT_PRIOR_SCENE_UPPER_LIMIT,
        false, "Create audio effect priorscene failed, please check log");
    while (containSize--) {
        effectChainMgrParam.priorSceneList.emplace_back(data.ReadString());
    }

    containSize = data.ReadInt32();

    CHECK_AND_RETURN_RET_LOG(containSize >= 0 && containSize <= AUDIO_EFFECT_CHAIN_CONFIG_UPPER_LIMIT,
        false, "Create audio effect chain name map failed, please check log");
    while (containSize--) {
        string key = data.ReadString();
        string value = data.ReadString();
        effectChainMgrParam.sceneTypeToChainNameMap[key] = value;
    }

    containSize = data.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(containSize >= 0 && containSize <= AUDIO_EFFECT_COUNT_PROPERTY_UPPER_LIMIT,
        false, "Create audio effect default property failed, please check log");
    while (containSize--) {
        string key = data.ReadString();
        string value = data.ReadString();
        effectChainMgrParam.effectDefaultProperty[key] = value;
    }
    return true;
}

int AudioManagerStub::HandleCreateAudioEffectChainManager(MessageParcel &data, MessageParcel &reply)
{
    int32_t i;
    vector<EffectChain> effectChains = {};
    vector<int32_t> countEffect = {};
    int32_t countChains = data.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(countChains >= 0 && countChains <= AUDIO_EFFECT_CHAIN_COUNT_UPPER_LIMIT,
        AUDIO_ERR, "Create audio effect chains failed, invalid countChains");
    for (i = 0; i < countChains; i++) {
        int32_t count = data.ReadInt32();
        CHECK_AND_RETURN_RET_LOG(count >= 0 && count <= AUDIO_EFFECT_COUNT_PER_CHAIN_UPPER_LIMIT,
            AUDIO_ERR, "Create audio effect chains failed, effect countChains");
        countEffect.emplace_back(count);
    }

    for (int32_t count: countEffect) {
        EffectChain effectChain;
        effectChain.name = data.ReadString();
        for (i = 0; i < count; i++) {
            effectChain.apply.emplace_back(data.ReadString());
        }
        effectChains.emplace_back(effectChain);
    }

    EffectChainManagerParam effectParam;
    EffectChainManagerParam enhanceParam;
    if (!UnmarshallEffectChainMgrParam(effectParam, data) || !UnmarshallEffectChainMgrParam(enhanceParam, data)) {
        return AUDIO_ERR;
    }
    bool createSuccess = CreateEffectChainManager(effectChains, effectParam, enhanceParam);
    CHECK_AND_RETURN_RET_LOG(createSuccess, AUDIO_ERR,
        "Create audio effect chain manager failed, please check log");
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetOutputDeviceSink(MessageParcel &data, MessageParcel &reply)
{
    int32_t deviceType = data.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(deviceType >= DEVICE_TYPE_NONE && deviceType <= DEVICE_TYPE_MAX, AUDIO_ERR,
        "Set output device sink failed, please check log");
    std::string sinkName = data.ReadString();
    SetOutputDeviceSink(deviceType, sinkName);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetActiveOutputDevice(MessageParcel &data, MessageParcel &reply)
{
    DeviceType deviceType = static_cast<DeviceType>(data.ReadInt32());
    CHECK_AND_RETURN_RET_LOG(deviceType >= DEVICE_TYPE_NONE && deviceType <= DEVICE_TYPE_MAX, AUDIO_ERR,
        "Set active output device failed, please check log");
    SetActiveOutputDevice(deviceType);
    return AUDIO_OK;
}

int AudioManagerStub::HandleCreatePlaybackCapturerManager(MessageParcel &data, MessageParcel &reply)
{
#ifdef HAS_FEATURE_INNERCAPTURER
    bool ret = CreatePlaybackCapturerManager();
    reply.WriteBool(ret);
    return AUDIO_OK;
#else
    return ERROR;
#endif
}

int AudioManagerStub::HandleRegiestPolicyProvider(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, AUDIO_ERR, "obj is null");
    int32_t result = RegiestPolicyProvider(object);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetWakeupSourceCallback(MessageParcel &data, MessageParcel &reply)
{
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, AUDIO_ERR,
        "SET_WAKEUP_CLOSE_CALLBACK obj is null");
    int32_t result = SetWakeupSourceCallback(object);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleUpdateSpatializationState(MessageParcel &data, MessageParcel &reply)
{
    AudioSpatializationState spatializationState;
    spatializationState.spatializationEnabled = data.ReadBool();
    spatializationState.headTrackingEnabled = data.ReadBool();
    int32_t ret = UpdateSpatializationState(spatializationState);
    reply.WriteInt32(ret);
    return AUDIO_OK;
}

int AudioManagerStub::HandleUpdateSpatialDeviceType(MessageParcel& data, MessageParcel& reply)
{
    AudioSpatialDeviceType spatialDeviceType = static_cast<AudioSpatialDeviceType>(data.ReadInt32());
    int32_t ret = UpdateSpatialDeviceType(spatialDeviceType);
    reply.WriteInt32(ret);
    return AUDIO_OK;
}

int AudioManagerStub::HandleOffloadSetVolume(MessageParcel &data, MessageParcel &reply)
{
    const float volume = data.ReadFloat();
    int32_t result = OffloadSetVolume(volume);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleNotifyStreamVolumeChanged(MessageParcel &data, MessageParcel &reply)
{
    AudioStreamType type = static_cast<AudioStreamType>(data.ReadInt32());
    float volume = data.ReadFloat();
    int32_t ret = NotifyStreamVolumeChanged(type, volume);
    reply.WriteInt32(ret);

    return AUDIO_OK;
}

int AudioManagerStub::HandleSetSpatializationSceneType(MessageParcel &data, MessageParcel &reply)
{
    AudioSpatializationSceneType spatializationSceneType = static_cast<AudioSpatializationSceneType>(data.ReadInt32());
    int32_t ret = SetSpatializationSceneType(spatializationSceneType);
    reply.WriteInt32(ret);
    return AUDIO_OK;
}

int AudioManagerStub::HandleResetRouteForDisconnect(MessageParcel &data, MessageParcel &reply)
{
    DeviceType deviceType = static_cast<DeviceType>(data.ReadInt32());
    int32_t ret = ResetRouteForDisconnect(deviceType);
    reply.WriteInt32(ret);
    return AUDIO_OK;
}

int AudioManagerStub::HandleGetEffectLatency(MessageParcel &data, MessageParcel &reply)
{
    string sessionId = data.ReadString();
    uint32_t ret = GetEffectLatency(sessionId);
    reply.WriteUint32(ret);
    return AUDIO_OK;
}

int AudioManagerStub::HandleGetMaxAmplitude(MessageParcel &data, MessageParcel &reply)
{
    bool isOutputDevice = data.ReadBool();
    std::string deviceClass = data.ReadString();
    SourceType sourceType = static_cast<SourceType>(data.ReadInt32());
    float result = GetMaxAmplitude(isOutputDevice, deviceClass, sourceType);
    reply.WriteFloat(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleResetAudioEndpoint(MessageParcel &data, MessageParcel &reply)
{
    ResetAudioEndpoint();
    return AUDIO_OK;
}

int AudioManagerStub::HandleSuspendRenderSink(MessageParcel &data, MessageParcel &reply)
{
    std::string sinkName = data.ReadString();
    int32_t ret = SuspendRenderSink(sinkName);
    reply.WriteInt32(ret);
    return AUDIO_OK;
}

int AudioManagerStub::HandleRestoreRenderSink(MessageParcel &data, MessageParcel &reply)
{
    std::string sinkName = data.ReadString();
    int32_t ret = RestoreRenderSink(sinkName);
    reply.WriteInt32(ret);
    return AUDIO_OK;
}

int AudioManagerStub::HandleUpdateLatencyTimestamp(MessageParcel &data, MessageParcel &reply)
{
    std::string timestamp = data.ReadString();
    bool isRenderer = data.ReadBool();
    UpdateLatencyTimestamp(timestamp, isRenderer);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetRotationToEffect(MessageParcel &data, MessageParcel &reply)
{
    SetRotationToEffect(data.ReadUint32());
    return AUDIO_OK;
}

int AudioManagerStub::HandleRestoreSession(MessageParcel &data, MessageParcel &reply)
{
    RestoreInfo restoreInfo;
    uint32_t sessionID = data.ReadUint32();
    restoreInfo.restoreReason = static_cast<RestoreReason>(data.ReadInt32());
    restoreInfo.deviceChangeReason = data.ReadInt32();
    restoreInfo.targetStreamFlag = data.ReadInt32();
    RestoreSession(sessionID, restoreInfo);
    return AUDIO_OK;
}

int AudioManagerStub::HandleCreateIpcOfflineStream(MessageParcel &data, MessageParcel &reply)
{
    int32_t errorCode = 0;
    sptr<IRemoteObject> process = CreateIpcOfflineStream(errorCode);
    CHECK_AND_RETURN_RET_LOG(process != nullptr, AUDIO_ERR,
        "CREATE_IPC_OFFLINE_STREAM AudioManagerStub CreateIpcOfflineStream failed");
    reply.WriteRemoteObject(process);
    reply.WriteInt32(errorCode);
    return AUDIO_OK;
}

int AudioManagerStub::HandleGetOfflineAudioEffectChains(MessageParcel &data, MessageParcel &reply)
{
    vector<string> effectChains{};
    int32_t errCode = GetOfflineAudioEffectChains(effectChains);
    reply.WriteInt32(effectChains.size());
    for (auto &chainName : effectChains) {
        reply.WriteString(chainName);
    }
    reply.WriteInt32(errCode);
    return AUDIO_OK;
}

int AudioManagerStub::HandleFourthPartCode(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_NOISE_SUPPRESSION_MODE):
            return HandleGetAsrNoiseSuppressionMode(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_WHISPER_DETECTION_MODE):
            return HandleSetAsrWhisperDetectionMode(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_WHISPER_DETECTION_MODE):
            return HandleGetAsrWhisperDetectionMode(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_VOICE_CONTROL_MODE):
            return HandleSetAsrVoiceControlMode(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_VOICE_MUTE_MODE):
            return HandleSetAsrVoiceMuteMode(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::IS_WHISPERING):
            return HandleIsWhispering(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::GET_EFFECT_OFFLOAD_ENABLED):
            return HandleGetEffectOffloadEnabled(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SUSPEND_RENDERSINK):
            return HandleSuspendRenderSink(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::RESTORE_RENDERSINK):
            return HandleRestoreRenderSink(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::LOAD_HDI_EFFECT_MODEL):
            return HandleLoadHdiEffectModel(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_EFFECT_BT_OFFLOAD_SUPPORTED):
            return HandleUpdateEffectBtOffloadSupported(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_ACCOUNTS_CHANGED):
            return HandleNotifyAccountsChanged(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_AUDIO_POLICY_READY):
            return HandleNotifyAudioPolicyReady(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::DEVICE_CONNECTED_FLAG):
            return HandleDeviceConnectedFlag(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_SETTINGS_DATA_READY):
            return HandleNotifySettingsDataReady(data, reply);
        default:
            return HandleFifthPartCode(code, data, reply, option);
    }
}

int AudioManagerStub::HandleFifthPartCode(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_SINK_MUTE_FOR_SWITCH_DEVICE):
            return HandleSetSinkMuteForSwitchDevice(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_ROTATION_TO_EFFECT):
            return HandleSetRotationToEffect(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_SESSION_CONNECTION_STATE):
            return HandleUpdateSessionConnectionState(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_SINGLE_STREAM_MUTE):
            return HandleSetNonInterruptMute(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::RESTORE_SESSION):
            return HandleRestoreSession(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_IPC_OFFLINE_STREAM):
            return HandleCreateIpcOfflineStream(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::GET_OFFLINE_AUDIO_EFFECT_CHAINS):
            return HandleGetOfflineAudioEffectChains(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::GET_STANDBY_STATUS):
            return HandleGetStandbyStatus(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::GENERATE_SESSION_ID):
            return HandleGenerateSessionId(data, reply);
#ifdef HAS_FEATURE_INNERCAPTURER
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_CAPTURE_LIMIT):
            return HandleSetInnerCapLimit(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::CHECK_CAPTURE_LIMIT):
            return HandleCheckCaptureLimit(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::RELEASE_CAPTURE_LIMIT):
            return HandleReleaseCaptureLimit(data, reply);
#endif
        case static_cast<uint32_t>(AudioServerInterfaceCode::LOAD_HDI_ADAPTER):
            return HandleLoadHdiAdapter(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::UNLOAD_HDI_ADAPTER):
            return HandleUnloadHdiAdapter(data, reply);
        default:
            AUDIO_ERR_LOG("default case, need check AudioManagerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
}

int AudioManagerStub::HandleThirdPartCode(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_STREAM_VOLUME_CHANGED):
            return HandleNotifyStreamVolumeChanged(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_SPATIALIZATION_SCENE_TYPE):
            return HandleSetSpatializationSceneType(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::GET_MAX_AMPLITUDE):
            return HandleGetMaxAmplitude(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::RESET_AUDIO_ENDPOINT):
            return HandleResetAudioEndpoint(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::RESET_ROUTE_FOR_DISCONNECT):
            return HandleResetRouteForDisconnect(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::GET_EFFECT_LATENCY):
            return HandleGetEffectLatency(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_LATENCY_TIMESTAMP):
            return HandleUpdateLatencyTimestamp(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_AEC_MODE):
            return HandleSetAsrAecMode(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::GET_ASR_AEC_MODE):
            return HandleGetAsrAecMode(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_ASR_NOISE_SUPPRESSION_MODE):
            return HandleSetAsrNoiseSuppressionMode(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_OFFLOAD_MODE):
            return HandleSetOffloadMode(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::UNSET_OFFLOAD_MODE):
            return HandleUnsetOffloadMode(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_EFFECT_PROPERTY_V3):
            return HandleGetAudioEffectPropertyV3(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_EFFECT_PROPERTY_V3):
            return HandleSetAudioEffectPropertyV3(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_ENHANCE_PROPERTY):
            return HandleGetAudioEnhanceProperty(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_EFFECT_PROPERTY):
            return HandleGetAudioEffectProperty(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_ENHANCE_PROPERTY):
            return HandleSetAudioEnhanceProperty(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_EFFECT_PROPERTY):
            return HandleSetAudioEffectProperty(data, reply);
        default:
            return HandleFourthPartCode(code, data, reply, option);
    }
}

int AudioManagerStub::HandleSecondPartCode(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    switch (code) {
        case static_cast<uint32_t>(AudioServerInterfaceCode::CHECK_REMOTE_DEVICE_STATE):
            return HandleCheckRemoteDeviceState(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_VOICE_VOLUME):
            return HandleSetVoiceVolume(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_MONO_STATE):
            return HandleSetAudioMonoState(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_BALANCE_VALUE):
            return HandleSetAudioBalanceValue(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_AUDIOPROCESS):
            return HandleCreateAudioProcess(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::LOAD_AUDIO_EFFECT_LIBRARIES):
            return HandleLoadAudioEffectLibraries(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_AUDIO_EFFECT_CHAIN_MANAGER):
            return HandleCreateAudioEffectChainManager(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_OUTPUT_DEVICE_SINK):
            return HandleSetOutputDeviceSink(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_ACTIVE_OUTPUT_DEVICE):
            return HandleSetActiveOutputDevice(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::CREATE_PLAYBACK_CAPTURER_MANAGER):
            return HandleCreatePlaybackCapturerManager(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::REGISET_POLICY_PROVIDER):
            return HandleRegiestPolicyProvider(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::SET_WAKEUP_CLOSE_CALLBACK):
            return HandleSetWakeupSourceCallback(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_SPATIALIZATION_STATE):
            return HandleUpdateSpatializationState(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_SPATIAL_DEVICE_TYPE):
            return HandleUpdateSpatialDeviceType(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::OFFLOAD_SET_VOLUME):
            return HandleOffloadSetVolume(data, reply);
        case static_cast<uint32_t>(AudioServerInterfaceCode::CHECK_HIBERNATE_STATE):
            return HandleCheckHibernateState(data, reply);
        default:
            return HandleThirdPartCode(code, data, reply, option);
    }
}

int AudioManagerStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    CHECK_AND_RETURN_RET_LOG(data.ReadInterfaceToken() == GetDescriptor(),
        -1, "ReadInterfaceToken failed");
    Trace trace(code >= codeNums ? "invalid audio server code!" : g_audioServerCodeStrs[code]);
    if (code <= static_cast<uint32_t>(AudioServerInterfaceCode::AUDIO_SERVER_CODE_MAX)) {
        switch (code) {
            case static_cast<uint32_t>(AudioServerInterfaceCode::GET_AUDIO_PARAMETER):
                return HandleGetAudioParameter(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_PARAMETER):
                return HandleSetAudioParameter(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::GET_EXTRA_AUDIO_PARAMETERS):
                return HandleGetExtraAudioParameters(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::SET_EXTRA_AUDIO_PARAMETERS):
                return HandleSetExtraAudioParameters(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::SET_MICROPHONE_MUTE):
                return HandleSetMicrophoneMute(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::SET_AUDIO_SCENE):
                return HandleSetAudioScene(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_ROUTE_REQ):
                return HandleUpdateActiveDeviceRoute(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_ROUTES_REQ):
                return HandleUpdateActiveDevicesRoute(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::UPDATE_DUAL_TONE_REQ):
                return HandleDualToneState(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::GET_TRANSACTION_ID):
                return HandleGetTransactionId(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::SET_PARAMETER_CALLBACK):
                return HandleSetParameterCallback(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::GET_REMOTE_AUDIO_PARAMETER):
                return HandleGetRemoteAudioParameter(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::SET_REMOTE_AUDIO_PARAMETER):
                return HandleSetRemoteAudioParameter(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::NOTIFY_DEVICE_INFO):
                return HandleNotifyDeviceInfo(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::GET_ALL_SINK_INPUTS):
                return HandleGetAllSinkInputs(data, reply);
            case static_cast<uint32_t>(AudioServerInterfaceCode::SET_DEFAULT_ADAPTER_ENABLE):
                return HandleSetDefaultAdapterEnable(data, reply);
            default:
                return HandleSecondPartCode(code, data, reply, option);
        }
    }
    AUDIO_ERR_LOG("default case, need check AudioManagerStub");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int AudioManagerStub::HandleLoadHdiEffectModel(MessageParcel &data, MessageParcel &reply)
{
    LoadHdiEffectModel();
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetAudioEffectPropertyV3(MessageParcel &data, MessageParcel &reply)
{
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(size > 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "audio enhance property array size invalid");
    AudioEffectPropertyArrayV3 propertyArray = {};
    for (int32_t i = 0; i < size; i++) {
        AudioEffectPropertyV3 prop = {};
        prop.Unmarshalling(data);
        propertyArray.property.push_back(prop);
    }
    int32_t result = SetAudioEffectProperty(propertyArray);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleGetAudioEffectPropertyV3(MessageParcel &data, MessageParcel &reply)
{
    AudioEffectPropertyArrayV3 propertyArray = {};
    int32_t result = GetAudioEffectProperty(propertyArray);
    int32_t size = static_cast<int32_t>(propertyArray.property.size());
    CHECK_AND_RETURN_RET_LOG(size >= 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "audio enhance property array size invalid");
    reply.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        propertyArray.property[i].Marshalling(reply);
    }
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetAudioEffectProperty(MessageParcel &data, MessageParcel &reply)
{
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(size > 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "Audio enhance property array size invalid");
    AudioEffectPropertyArray propertyArray = {};
    for (int32_t i = 0; i < size; i++) {
        AudioEffectProperty prop = {};
        prop.Unmarshalling(data);
        propertyArray.property.push_back(prop);
    }
    int32_t result = SetAudioEffectProperty(propertyArray);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleGetAudioEffectProperty(MessageParcel &data, MessageParcel &reply)
{
    AudioEffectPropertyArray propertyArray = {};
    int32_t result = GetAudioEffectProperty(propertyArray);
    int32_t size = static_cast<int32_t>(propertyArray.property.size());
    CHECK_AND_RETURN_RET_LOG(size >= 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "Audio enhance property array size invalid");
    reply.WriteInt32(size);
    for (int32_t i = 0; i < size; i++)    {
        propertyArray.property[i].Marshalling(reply);
    }
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetAudioEnhanceProperty(MessageParcel &data, MessageParcel &reply)
{
    int32_t size = data.ReadInt32();
    CHECK_AND_RETURN_RET_LOG(size > 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "Audio enhance property array size invalid");
    AudioEnhancePropertyArray propertyArray = {};
    for (int32_t i = 0; i < size; i++) {
        AudioEnhanceProperty prop = {};
        prop.Unmarshalling(data);
        propertyArray.property.push_back(prop);
    }
    DeviceType deviceType = static_cast<DeviceType>(data.ReadInt32());
    int32_t result = SetAudioEnhanceProperty(propertyArray, deviceType);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleGetAudioEnhanceProperty(MessageParcel &data, MessageParcel &reply)
{
    AudioEnhancePropertyArray propertyArray = {};
    DeviceType deviceType = static_cast<DeviceType>(data.ReadInt32());
    int32_t result = GetAudioEnhanceProperty(propertyArray, deviceType);
    int32_t size = static_cast<int32_t>(propertyArray.property.size());
    CHECK_AND_RETURN_RET_LOG(size >= 0 && size <= AUDIO_EFFECT_COUNT_UPPER_LIMIT,
        ERROR_INVALID_PARAM, "Audio enhance property array size invalid");
    reply.WriteInt32(size);
    for (int32_t i = 0; i < size; i++) {
        propertyArray.property[i].Marshalling(reply);
    }
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleUpdateEffectBtOffloadSupported(MessageParcel &data, MessageParcel &reply)
{
    UpdateEffectBtOffloadSupported(data.ReadBool());
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetSinkMuteForSwitchDevice(MessageParcel &data, MessageParcel &reply)
{
    const std::string deviceClass = data.ReadString();
    int32_t duration = data.ReadInt32();
    int32_t mute = data.ReadBool();
    int32_t result = SetSinkMuteForSwitchDevice(deviceClass, duration, mute);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleUpdateSessionConnectionState(MessageParcel &data, MessageParcel &reply)
{
    int32_t sessionID = data.ReadInt32();
    int32_t state = data.ReadInt32();
    UpdateSessionConnectionState(sessionID, state);
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetNonInterruptMute(MessageParcel &data, MessageParcel &reply)
{
    uint32_t sessionId = data.ReadUint32();
    bool muteFlag = data.ReadBool();
    SetNonInterruptMute(sessionId, muteFlag);
    return AUDIO_OK;
}

int AudioManagerStub::HandleGetStandbyStatus(MessageParcel &data, MessageParcel &reply)
{
    uint32_t sessionId = data.ReadUint32();
    bool isStandby = false;
    int64_t enterStandbyTime = 0;
    int32_t result = GetStandbyStatus(sessionId, isStandby, enterStandbyTime);

    reply.WriteInt32(result);
    reply.WriteBool(isStandby);
    reply.WriteInt64(enterStandbyTime);
    return AUDIO_OK;
}

int AudioManagerStub::HandleGenerateSessionId(MessageParcel &data, MessageParcel &reply)
{
    uint32_t sessionId = data.ReadUint32();
    int32_t ret = GenerateSessionId(sessionId);
    CHECK_AND_RETURN_RET_LOG(ret == 0, AUDIO_ERR, "generate session id failed");
    reply.WriteUint32(sessionId);
    return AUDIO_OK;
}

int AudioManagerStub::HandleNotifyAccountsChanged(MessageParcel &data, MessageParcel &reply)
{
    NotifyAccountsChanged();
    return AUDIO_OK;
}

int AudioManagerStub::HandleGetAllSinkInputs(MessageParcel &data, MessageParcel &reply)
{
    std::vector<SinkInput> sinkInputs;
    GetAllSinkInputs(sinkInputs);
    size_t size = sinkInputs.size();
    reply.WriteUint64(size);
    for (auto &sinkInput : sinkInputs) {
        sinkInput.Marshalling(reply);
    }
    return AUDIO_OK;
}

int AudioManagerStub::HandleSetDefaultAdapterEnable(MessageParcel &data, MessageParcel &reply)
{
    bool isEnable = data.ReadBool();
    SetDefaultAdapterEnable(isEnable);
    return AUDIO_OK;
}

int AudioManagerStub::HandleNotifyAudioPolicyReady(MessageParcel &data, MessageParcel &reply)
{
    NotifyAudioPolicyReady();
    return AUDIO_OK;
}

#ifdef HAS_FEATURE_INNERCAPTURER
int AudioManagerStub::HandleSetInnerCapLimit(MessageParcel &data, MessageParcel &reply)
{
    uint32_t innerCapId = data.ReadUint32();
    reply.WriteInt32(SetInnerCapLimit(innerCapId));
    return AUDIO_OK;
}

int AudioManagerStub::HandleCheckCaptureLimit(MessageParcel &data, MessageParcel &reply)
{
    AudioPlaybackCaptureConfig filterConfig;
    ProcessConfig::ReadInnerCapConfigFromParcel(filterConfig, data);
    int32_t innerCapId = 0;
    reply.WriteInt32(CheckCaptureLimit(filterConfig, innerCapId));
    return AUDIO_OK;
}

int AudioManagerStub::HandleReleaseCaptureLimit(MessageParcel &data, MessageParcel &reply)
{
    int32_t innerCapId = data.ReadInt32();
    reply.WriteInt32(ReleaseCaptureLimit(innerCapId));
    return AUDIO_OK;
}
#endif

int AudioManagerStub::HandleLoadHdiAdapter(MessageParcel &data, MessageParcel &reply)
{
    uint32_t devMgrType = data.ReadUint32();
    const std::string adapterName = data.ReadString();
    int32_t result = LoadHdiAdapter(devMgrType, adapterName);
    reply.WriteInt32(result);
    return AUDIO_OK;
}

int AudioManagerStub::HandleUnloadHdiAdapter(MessageParcel &data, MessageParcel &reply)
{
    uint32_t devMgrType = data.ReadUint32();
    const std::string adapterName = data.ReadString();
    bool force = data.ReadBool();
    UnloadHdiAdapter(devMgrType, adapterName, force);
    return AUDIO_OK;
}

int AudioManagerStub::HandleDeviceConnectedFlag(MessageParcel &data, MessageParcel &reply)
{
    bool flag = data.ReadBool();
    SetDeviceConnectedFlag(flag);
    return AUDIO_OK;
}

int AudioManagerStub::HandleNotifySettingsDataReady(MessageParcel &data, MessageParcel &reply)
{
    NotifySettingsDataReady();
    return AUDIO_OK;
}
} // namespace AudioStandard
} // namespace OHOS
