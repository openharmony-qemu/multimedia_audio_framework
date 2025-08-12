/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioA2dpOffloadManager"
#endif

#include "audio_a2dp_offload_manager.h"
#include <ability_manager_client.h>
#include "iservice_registry.h"
#include "parameter.h"
#include "parameters.h"
#include "audio_utils.h"
#include "audio_policy_log.h"
#include "audio_utils.h"
#include "audio_inner_call.h"
#include "media_monitor_manager.h"
#include "audio_affinity_manager.h"
#include "audio_stream_collector.h"
#include "audio_policy_manager_factory.h"
#include "audio_spatialization_manager.h"
#include "audio_spatialization_service.h"
#include "audio_router_center.h"

#include "audio_policy_utils.h"
#include "audio_server_proxy.h"

namespace OHOS {
namespace AudioStandard {
const int32_t A2DP_PLAYING = 2;
const int32_t A2DP_STOPPED = 1;
const int32_t DATA_LINK_CONNECTING = 10;
const int32_t DATA_LINK_CONNECTED = 11;
const int32_t CONNECTION_TIMEOUT_IN_MS = 1000; // 1000ms

#ifdef BLUETOOTH_ENABLE
const unsigned int BLUETOOTH_TIME_OUT_SECONDS = 8;
#endif

static std::string GetEncryptAddr(const std::string &addr)
{
    const int32_t START_POS = 6;
    const int32_t END_POS = 13;
    const int32_t ADDRESS_STR_LEN = 17;
    if (addr.empty() || addr.length() != ADDRESS_STR_LEN) {
        return std::string("");
    }
    std::string tmp = "**:**:**:**:**:**";
    std::string out = addr;
    for (int i = START_POS; i <= END_POS; i++) {
        out[i] = tmp[i];
    }
    return out;
}

void AudioA2dpOffloadManager::OnA2dpPlayingStateChanged(const std::string &deviceAddress, int32_t playingState)
{
    A2dpOffloadConnectionState state = audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState();
    AUDIO_INFO_LOG("OnA2dpPlayingStateChanged current A2dpOffload MacAddr:%{public}s, incoming MacAddr:%{public}s, "
        "currentStatus:%{public}d, incommingState:%{public}d", GetEncryptAddr(a2dpOffloadDeviceAddress_).c_str(),
        GetEncryptAddr(deviceAddress).c_str(), state, playingState);
    if (deviceAddress != a2dpOffloadDeviceAddress_) {
        if (playingState == A2DP_STOPPED && state == CONNECTION_STATUS_CONNECTED) {
            return;
        }
        // below is A2dp(not offload scenario)
        audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_DISCONNECTED);
        return;
    }

    // deviceAddress matched
    if (playingState == A2DP_PLAYING) {
        if (state == CONNECTION_STATUS_CONNECTING) {
            AUDIO_INFO_LOG("OnA2dpPlayingStateChanged state change "
                "from %{public}d to %{public}d", state, CONNECTION_STATUS_CONNECTED);

            for (int32_t sessionId : connectionTriggerSessionIds_) {
                AudioServerProxy::GetInstance().UpdateSessionConnectionStateProxy(sessionId, DATA_LINK_CONNECTED);
            }
            std::vector<int32_t>().swap(connectionTriggerSessionIds_);
            connectionCV_.notify_all();
        }
        audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    } else if (playingState == A2DP_STOPPED) {
        AUDIO_INFO_LOG("OnA2dpPlayingStateChanged state change "
            "from %{public}d to %{public}d", state, CONNECTION_STATUS_DISCONNECTED);
        audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_DISCONNECTED);
        a2dpOffloadDeviceAddress_ = "";
        std::vector<int32_t>().swap(connectionTriggerSessionIds_);
    } else {
        // at the current moment, we only handle the PLAYING and STOPPED state,
        // will handle other state in the future
        AUDIO_INFO_LOG("OnA2dpPlayingStateChanged state: %{public}d, "
            "received unexpected state:%{public}d", state, playingState);
    }
}

void AudioA2dpOffloadManager::ConnectA2dpOffload(const std::string &deviceAddress,
    const std::vector<int32_t> &sessionIds)
{
    AUDIO_INFO_LOG("start connecting a2dpOffload for MacAddr:%{public}s.", GetEncryptAddr(deviceAddress).c_str());
    A2dpOffloadConnectionState state = audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState();
    a2dpOffloadDeviceAddress_ = deviceAddress;
    connectionTriggerSessionIds_.assign(sessionIds.begin(), sessionIds.end());

    for (int32_t sessionId : connectionTriggerSessionIds_) {
        AudioServerProxy::GetInstance().UpdateSessionConnectionStateProxy(sessionId, DATA_LINK_CONNECTING);
    }

    if (state == CONNECTION_STATUS_CONNECTED || state == CONNECTION_STATUS_CONNECTING) {
        AUDIO_INFO_LOG("state already in %{public}d, "
            "status, no need to trigger another waiting", state);
        return;
    }

    std::thread switchThread(&AudioA2dpOffloadManager::WaitForConnectionCompleted, this);
    switchThread.detach();
    AUDIO_INFO_LOG("state change from %{public}d to %{public}d",
        state, CONNECTION_STATUS_CONNECTING);
    audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTING);
}

void AudioA2dpOffloadManager::WaitForConnectionCompleted()
{
    std::unique_lock<std::mutex> waitLock(connectionMutex_);
    bool connectionCompleted = connectionCV_.wait_for(waitLock,
        std::chrono::milliseconds(CONNECTION_TIMEOUT_IN_MS), [this] {
            return audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState() == CONNECTION_STATUS_CONNECTED;
        });
    // a2dp connection timeout, anyway we should notify client dataLink OK in order to allow the data flow begin
    AUDIO_INFO_LOG("WaitForConnectionCompleted unblocked, connectionCompleted is %{public}d", connectionCompleted);

    if (!connectionCompleted) {
        AUDIO_INFO_LOG("state change from %{public}d to %{public}d",
            audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState(), CONNECTION_STATUS_TIMEOUT);
        audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
        for (int32_t sessionId : connectionTriggerSessionIds_) {
            AudioServerProxy::GetInstance().UpdateSessionConnectionStateProxy(sessionId, DATA_LINK_CONNECTED);
        }
        std::vector<int32_t>().swap(connectionTriggerSessionIds_);
    }
    waitLock.unlock();
    FetchStreamForA2dpOffload(false);
    return;
}

bool AudioA2dpOffloadManager::IsA2dpOffloadConnecting(int32_t sessionId)
{
    if (audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState() == CONNECTION_STATUS_CONNECTING) {
        if (std::find(connectionTriggerSessionIds_.begin(), connectionTriggerSessionIds_.end(), sessionId) !=
            connectionTriggerSessionIds_.end()) {
            return true;
        }
    }
    return false;
}

bool AudioA2dpOffloadManager::IsA2dpOffloadConnected()
{
    return audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState() == CONNECTION_STATUS_CONNECTED;
}

int32_t AudioA2dpOffloadManager::OffloadStartPlaying(const std::vector<int32_t> &sessionIds)
{
#ifdef BLUETOOTH_ENABLE
    if (GetA2dpOffloadFlag() != A2DP_OFFLOAD || sessionIds.size() == 0) {
        return SUCCESS;
    }
    AUDIO_INFO_LOG("a2dpOffloadFlag: %{public}d, sessionCnt: %{public}zu",
        GetA2dpOffloadFlag(), sessionIds.size());
    int32_t ret = audioA2dpOffloadFlag_.OffloadStartPlaying(sessionIds);
    A2dpOffloadConnectionState state = audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState();
    if (ret == SUCCESS && (state != CONNECTION_STATUS_CONNECTED)) {
        ConnectA2dpOffload(Bluetooth::AudioA2dpManager::GetActiveA2dpDevice(), sessionIds);
    }
    return ret;
#else
    return SUCCESS;
#endif
}

int32_t AudioA2dpOffloadManager::OffloadStopPlaying(const std::vector<int32_t> &sessionIds)
{
#ifdef BLUETOOTH_ENABLE
    if (GetA2dpOffloadFlag() != A2DP_OFFLOAD || sessionIds.size() == 0) {
        return SUCCESS;
    }
    AUDIO_PRERELEASE_LOGI("a2dpOffloadFlag: %{public}d, sessionCnt: %{public}zu",
        GetA2dpOffloadFlag(), sessionIds.size());
    return audioA2dpOffloadFlag_.OffloadStopPlaying(sessionIds);
#else
    return SUCCESS;
#endif
}

void AudioA2dpOffloadManager::SetA2dpOffloadFlag(BluetoothOffloadState state)
{
    audioA2dpOffloadFlag_.SetA2dpOffloadFlag(state);
}

BluetoothOffloadState AudioA2dpOffloadManager::GetA2dpOffloadFlag()
{
    return audioA2dpOffloadFlag_.GetA2dpOffloadFlag();
}

void AudioA2dpOffloadManager::UpdateA2dpOffloadFlagForAllStream(
    std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnableMap, DeviceType deviceType)
{
#ifdef BLUETOOTH_ENABLE
    vector<Bluetooth::A2dpStreamInfo> allSessionInfos;
    Bluetooth::A2dpStreamInfo a2dpStreamInfo;
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    streamCollector_.GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    std::vector<int32_t> stopPlayingStream(0);
    for (auto &changeInfo : audioRendererChangeInfos) {
        if (changeInfo->rendererState != RENDERER_RUNNING) {
            stopPlayingStream.emplace_back(changeInfo->sessionId);
            continue;
        }
        a2dpStreamInfo.sessionId = changeInfo->sessionId;
        a2dpStreamInfo.streamType = streamCollector_.GetStreamType(changeInfo->sessionId);
        if (sessionIDToSpatializationEnableMap.count(static_cast<uint32_t>(a2dpStreamInfo.sessionId))) {
            a2dpStreamInfo.isSpatialAudio =
                sessionIDToSpatializationEnableMap[static_cast<uint32_t>(a2dpStreamInfo.sessionId)];
        } else {
            a2dpStreamInfo.isSpatialAudio = 0;
        }
        allSessionInfos.push_back(a2dpStreamInfo);
    }
    if (stopPlayingStream.size() > 0) {
        OffloadStopPlaying(stopPlayingStream);
    }
    UpdateA2dpOffloadFlag(allSessionInfos, deviceType);
#endif
    AUDIO_DEBUG_LOG("deviceType %{public}d", deviceType);
}

int32_t AudioA2dpOffloadManager::UpdateA2dpOffloadFlagForAllStream(DeviceType deviceType)
{
    int32_t activeSessionsSize = 0;
#ifdef BLUETOOTH_ENABLE
    vector<Bluetooth::A2dpStreamInfo> allSessionInfos;
    Bluetooth::A2dpStreamInfo a2dpStreamInfo;
    vector<shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
    streamCollector_.GetCurrentRendererChangeInfos(audioRendererChangeInfos);
    {
        AudioXCollie audioXCollie("AudioA2dpOffloadManager::UpdateA2dpOffloadFlagForAllStream",
            BLUETOOTH_TIME_OUT_SECONDS, nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
        std::vector<int32_t> stopPlayingStream(0);
        for (auto &changeInfo : audioRendererChangeInfos) {
            if (changeInfo->rendererState != RENDERER_RUNNING) {
                stopPlayingStream.emplace_back(changeInfo->sessionId);
                continue;
            }
            a2dpStreamInfo.sessionId = changeInfo->sessionId;
            a2dpStreamInfo.streamType = streamCollector_.GetStreamType(changeInfo->sessionId);
            StreamUsage tempStreamUsage = changeInfo->rendererInfo.streamUsage;
            AudioSpatializationState spatialState =
                AudioSpatializationService::GetAudioSpatializationService().GetSpatializationState(tempStreamUsage);
            a2dpStreamInfo.isSpatialAudio = spatialState.spatializationEnabled;
            allSessionInfos.push_back(a2dpStreamInfo);
        }
        if (stopPlayingStream.size() > 0) {
            OffloadStopPlaying(stopPlayingStream);
        }
    }
    UpdateA2dpOffloadFlag(allSessionInfos, deviceType);
    activeSessionsSize = static_cast<int32_t>(allSessionInfos.size());
#endif
    AUDIO_DEBUG_LOG("deviceType %{public}d", deviceType);
    return activeSessionsSize;
}

void AudioA2dpOffloadManager::UpdateOffloadWhenActiveDeviceSwitchFromA2dp()
{
    AUDIO_PRERELEASE_LOGI("a2dpOffloadFlag change from %{public}d to %{public}d", GetA2dpOffloadFlag(),
        NO_A2DP_DEVICE);
    std::vector<int32_t> allSessions;
    GetAllRunningStreamSession(allSessions);
    OffloadStopPlaying(allSessions);
    SetA2dpOffloadFlag(NO_A2DP_DEVICE);
    for (auto it = allSessions.begin(); it != allSessions.end(); ++it) {
        audioOffloadStream_.ResetOffloadMode(*it);
    }
}

#ifdef BLUETOOTH_ENABLE
void AudioA2dpOffloadManager::UpdateA2dpOffloadFlag(const std::vector<Bluetooth::A2dpStreamInfo> &allActiveSessions,
    DeviceType deviceType)
{
    if (allActiveSessions.size() == 0) {
        return;
    }
    auto receiveOffloadFlag = NO_A2DP_DEVICE;
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        receiveOffloadFlag = static_cast<BluetoothOffloadState>(Bluetooth::AudioA2dpManager::A2dpOffloadSessionRequest(
            allActiveSessions));
    } else if (audioActiveDevice_.GetCurrentOutputDeviceType() == DEVICE_TYPE_BLUETOOTH_A2DP &&
        audioActiveDevice_.GetCurrentOutputDeviceNetworkId() == LOCAL_NETWORK_ID && deviceType == DEVICE_TYPE_NONE) {
        receiveOffloadFlag = static_cast<BluetoothOffloadState>(Bluetooth::AudioA2dpManager::A2dpOffloadSessionRequest(
            allActiveSessions));
    }

    std::lock_guard<std::mutex> lock(switchA2dpOffloadMutex_);
    AUDIO_PRERELEASE_LOGI("deviceType: %{public}d, currentActiveDevice_: %{public}d, allActiveSessions: %{public}zu, "
        "a2dpOffloadFlag: %{public}d, receiveOffloadFlag: %{public}d",
        deviceType, audioActiveDevice_.GetCurrentOutputDeviceType(), allActiveSessions.size(), GetA2dpOffloadFlag(),
        receiveOffloadFlag);

    if (receiveOffloadFlag == NO_A2DP_DEVICE) {
        UpdateOffloadWhenActiveDeviceSwitchFromA2dp();
    } else if (receiveOffloadFlag != GetA2dpOffloadFlag()) {
        if (GetA2dpOffloadFlag() == A2DP_OFFLOAD) {
            HandleA2dpDeviceOutOffload(receiveOffloadFlag);
        } else if (receiveOffloadFlag == A2DP_OFFLOAD) {
            HandleA2dpDeviceInOffload(receiveOffloadFlag);
        } else {
            AUDIO_INFO_LOG("a2dpOffloadFlag change from %{public}d to %{public}d", GetA2dpOffloadFlag(),
                receiveOffloadFlag);
            SetA2dpOffloadFlag(receiveOffloadFlag);
        }
    } else if (GetA2dpOffloadFlag() == A2DP_OFFLOAD) {
        std::vector<int32_t> allSessions;
        GetAllRunningStreamSession(allSessions);
        OffloadStartPlaying(allSessions);
        AudioServerProxy::GetInstance().UpdateEffectBtOffloadSupportedProxy(true);
        audioOffloadStream_.ResetOffloadModeOnSpatializationChanged(allSessions);
        GetA2dpOffloadCodecAndSendToDsp();
        std::string activePort = BLUETOOTH_SPEAKER;
        audioPolicyManager_.SuspendAudioDevice(activePort, true);
    }
}

#endif

int32_t AudioA2dpOffloadManager::HandleA2dpDeviceOutOffload(BluetoothOffloadState a2dpOffloadFlag)
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("a2dpOffloadFlag change from %{public}d to %{public}d", GetA2dpOffloadFlag(), a2dpOffloadFlag);
    std::vector<int32_t> allSessions;
    GetAllRunningStreamSession(allSessions);
    OffloadStopPlaying(allSessions);
    SetA2dpOffloadFlag(a2dpOffloadFlag);

    DeviceType dev = audioActiveDevice_.GetCurrentOutputDeviceType();
    AUDIO_INFO_LOG("Handle A2dpDevice Out Offload");

    FetchStreamForA2dpOffload(true);

    AudioDeviceDescriptor deviceDescriptor = audioActiveDevice_.GetCurrentOutputDevice();
    if (deviceDescriptor.deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        return HandleActiveDevice(deviceDescriptor);
    } else {
        return SUCCESS;
    }
#else
    return ERROR;
#endif
}

int32_t AudioA2dpOffloadManager::HandleA2dpDeviceInOffload(BluetoothOffloadState a2dpOffloadFlag)
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("a2dpOffloadFlag change from %{public}d to %{public}d", GetA2dpOffloadFlag(), a2dpOffloadFlag);
    SetA2dpOffloadFlag(a2dpOffloadFlag);
    GetA2dpOffloadCodecAndSendToDsp();
    std::vector<int32_t> allSessions;
    GetAllRunningStreamSession(allSessions);
    OffloadStartPlaying(allSessions);

    DeviceType dev = audioActiveDevice_.GetCurrentOutputDeviceType();
    AUDIO_INFO_LOG("Handle A2dpDevice In Offload");
    AudioServerProxy::GetInstance().UpdateEffectBtOffloadSupportedProxy(true);

    if (IsA2dpOffloadConnected()) {
        AUDIO_INFO_LOG("A2dpOffload has been connected, Fetch stream");
        FetchStreamForA2dpOffload(true);
    }
    return SUCCESS;
#else
    return ERROR;
#endif
}

void AudioA2dpOffloadManager::GetA2dpOffloadCodecAndSendToDsp()
{
#ifdef BLUETOOTH_ENABLE
    if (audioActiveDevice_.GetCurrentOutputDeviceType() != DEVICE_TYPE_BLUETOOTH_A2DP) {
        return;
    }
    Bluetooth::BluetoothRemoteDevice bluetoothRemoteDevice_
        = Bluetooth::AudioA2dpManager::GetCurrentActiveA2dpDevice();
    Bluetooth::A2dpOffloadCodecStatus offloadCodeStatus = Bluetooth::A2dpSource::GetProfile()->
        GetOffloadCodecStatus(bluetoothRemoteDevice_);
    std::string key = "AUDIO_EXT_PARAM_KEY_A2DP_OFFLOAD_CONFIG";
    std::string value = std::to_string(offloadCodeStatus.offloadInfo.mediaPacketHeader) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.mPt) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.ssrc) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.boundaryFlag) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.broadcastFlag) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.codecType) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.maxLatency) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.scmsTEnable) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.sampleRate) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.encodedAudioBitrate) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.bitsPerSample) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.chMode) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.aclHdl) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.l2cRcid) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.mtu) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.codecSpecific0) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.codecSpecific1) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.codecSpecific2) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.codecSpecific3) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.codecSpecific4) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.codecSpecific5) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.codecSpecific6) + ","
        + std::to_string(offloadCodeStatus.offloadInfo.codecSpecific7) + ";";

    AudioServerProxy::GetInstance().SetAudioParameterProxy(key, value);
    AUDIO_INFO_LOG("update offloadcodec[%{public}s]", value.c_str());
#endif
}

int32_t AudioA2dpOffloadManager::HandleActiveDevice(AudioDeviceDescriptor deviceDescriptor)
{
    AudioDeviceDescriptor curOutputDevice = audioActiveDevice_.GetCurrentOutputDevice();
    if (GetVolumeGroupType(curOutputDevice.deviceType_) != GetVolumeGroupType(deviceDescriptor.deviceType_)) {
        audioVolumeManager_.SetVolumeForSwitchDevice(deviceDescriptor);
    }
    if (audioConfigManager_.GetUpdateRouteSupport()) {
        audioActiveDevice_.UpdateActiveDeviceRoute(deviceDescriptor.deviceType_, DeviceFlag::OUTPUT_DEVICES_FLAG,
            deviceDescriptor.deviceName_, deviceDescriptor.networkId_);
    }
    std::string sinkPortName = AudioPolicyUtils::GetInstance().GetSinkPortName(deviceDescriptor.deviceType_);
    std::string sourcePortName = AudioPolicyUtils::GetInstance().GetSourcePortName(deviceDescriptor.deviceType_);
    if (sinkPortName == PORT_NONE && sourcePortName == PORT_NONE) {
        AUDIO_ERR_LOG("failed for sinkPortName and sourcePortName are none");
        return ERR_OPERATION_FAILED;
    }
    if (sinkPortName != PORT_NONE) {
        audioIOHandleMap_.GetSinkIOHandle(deviceDescriptor.deviceType_);
        audioPolicyManager_.SuspendAudioDevice(sinkPortName, false);
    }
    if (sourcePortName != PORT_NONE) {
        audioIOHandleMap_.GetSourceIOHandle(deviceDescriptor.deviceType_);
        audioPolicyManager_.SuspendAudioDevice(sourcePortName, false);
    }

    return SUCCESS;
}

void AudioA2dpOffloadManager::FetchStreamForA2dpOffload(const bool &requireReset)
{
    vector<shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    streamCollector_.GetCurrentRendererChangeInfos(rendererChangeInfos);
    AUDIO_INFO_LOG("FetchStreamForA2dpOffload start for %{public}zu stream", rendererChangeInfos.size());
    for (auto &rendererChangeInfo : rendererChangeInfos) {
        if (!audioDeviceCommon_.IsRendererStreamRunning(rendererChangeInfo)) {
            continue;
        }
        vector<std::shared_ptr<AudioDeviceDescriptor>> descs =
            audioRouterCenter_.FetchOutputDevices(rendererChangeInfo->rendererInfo.streamUsage,
            rendererChangeInfo->clientUID, "FetchStreamForA2dpOffload");

        if (descs.front()->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
            if (requireReset) {
                int32_t ret = audioDeviceCommon_.ActivateA2dpDevice(descs.front(), rendererChangeInfos);
                CHECK_AND_RETURN_LOG(ret == SUCCESS, "activate a2dp [%{public}s] failed",
                    GetEncryptAddr(descs.front()->macAddress_).c_str());
                std::string activePort = BLUETOOTH_SPEAKER;
                audioPolicyManager_.SuspendAudioDevice(activePort, true);
            }
            if (rendererChangeInfo->rendererInfo.rendererFlags == AUDIO_FLAG_MMAP) {
                AudioServerProxy::GetInstance().ResetAudioEndpointProxy();
            }
            audioDeviceCommon_.FetchStreamForA2dpMchStream(rendererChangeInfo, descs);
        }
    }
}

void AudioA2dpOffloadManager::GetAllRunningStreamSession(std::vector<int32_t> &allSessions, bool doStop)
{
#ifdef BLUETOOTH_ENABLE
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    streamCollector_.GetCurrentRendererChangeInfos(rendererChangeInfos);
    std::vector<int32_t> stopPlayingStream(0);
    for (auto &changeInfo : rendererChangeInfos) {
        if (changeInfo->rendererState != RENDERER_RUNNING) {
            if (doStop) {
                stopPlayingStream.push_back(changeInfo->sessionId);
            }
            continue;
        }
        allSessions.push_back(changeInfo->sessionId);
    }
    if (doStop && stopPlayingStream.size() > 0) {
        OffloadStopPlaying(stopPlayingStream);
    }
#endif
}

std::string AudioA2dpOffloadManager::GetVolumeGroupType(DeviceType deviceType)
{
    std::string volumeGroupType = "";
    switch (deviceType) {
        case DEVICE_TYPE_EARPIECE:
        case DEVICE_TYPE_SPEAKER:
        case DEVICE_TYPE_DP:
        case DEVICE_TYPE_HDMI:
            volumeGroupType = "build-in";
            break;
        case DEVICE_TYPE_BLUETOOTH_A2DP:
        case DEVICE_TYPE_BLUETOOTH_SCO:
        case DEVICE_TYPE_ACCESSORY:
            volumeGroupType = "wireless";
            break;
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_USB_ARM_HEADSET:
            volumeGroupType = "wired";
            break;
        default:
            AUDIO_ERR_LOG("device %{public}d is not supported", deviceType);
            break;
    }
    return volumeGroupType;
}

}
}
