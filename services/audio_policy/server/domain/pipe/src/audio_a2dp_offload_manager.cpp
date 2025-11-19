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

#include "audio_stream_collector.h"
#include "audio_spatialization_manager.h"
#include "audio_spatialization_service.h"
#include "audio_policy_utils.h"
#include "audio_server_proxy.h"
#include "i_hpae_manager.h"

namespace OHOS {
namespace AudioStandard {
const int32_t A2DP_PLAYING = 2;
const int32_t A2DP_STOPPED = 1;
const int32_t DATA_LINK_CONNECTING = 10;
const int32_t DATA_LINK_CONNECTED = 11;
const int32_t CONNECTION_TIMEOUT_IN_MS = 1000; // 1000ms
const int32_t SESSION_ID_INVALID = 0;

void AudioA2dpOffloadManager::OnA2dpPlayingStateChanged(const std::string &deviceAddress, int32_t playingState)
{
    A2dpOffloadConnectionState state = audioA2dpOffloadFlag_.GetCurrentOffloadConnectedState();
    AUDIO_INFO_LOG("current A2dpOffload MacAddr:%{public}s, incoming MacAddr:%{public}s, "
        "currentStatus:%{public}d, incommingState:%{public}d",
        AudioPolicyUtils::GetInstance().GetEncryptAddr(a2dpOffloadDeviceAddress_).c_str(),
        AudioPolicyUtils::GetInstance().GetEncryptAddr(deviceAddress).c_str(), state, playingState);
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
            AUDIO_INFO_LOG("state change from %{public}d to %{public}d", state, CONNECTION_STATUS_CONNECTED);
            for (int32_t sessionId : connectionTriggerSessionIds_) {
                AudioServerProxy::GetInstance().UpdateSessionConnectionStateProxy(sessionId, DATA_LINK_CONNECTED);
            }
            std::vector<int32_t>().swap(connectionTriggerSessionIds_);
            connectionCV_.notify_all();
        }
        audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_CONNECTED);
    } else if (playingState == A2DP_STOPPED) {
        AUDIO_INFO_LOG("state change from %{public}d to %{public}d", state, CONNECTION_STATUS_DISCONNECTED);
        audioA2dpOffloadFlag_.SetCurrentOffloadConnectedState(CONNECTION_STATUS_DISCONNECTED);
        a2dpOffloadDeviceAddress_ = "";
        std::vector<int32_t>().swap(connectionTriggerSessionIds_);
    } else {
        // Currently we only handle the PLAYING and STOPPED state, may handle other state in the future
        AUDIO_INFO_LOG("state: %{public}d, received unexpected state:%{public}d", state, playingState);
    }
}

void AudioA2dpOffloadManager::ConnectA2dpOffload(const std::string &deviceAddress,
    const std::vector<int32_t> &sessionIds)
{
    AUDIO_INFO_LOG("start connecting a2dpOffload for MacAddr:%{public}s.",
        AudioPolicyUtils::GetInstance().GetEncryptAddr(deviceAddress).c_str());
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
    AUDIO_INFO_LOG("unblocked, connectionCompleted is %{public}d", connectionCompleted);

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

void AudioA2dpOffloadManager::UpdateA2dpOffloadFlagForSpatializationChanged(
    std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnabledMap, DeviceType deviceType)
{
    UpdateA2dpOffloadFlagCommon(SESSION_ID_INVALID, deviceType, false, sessionIDToSpatializationEnabledMap);
}

void AudioA2dpOffloadManager::UpdateA2dpOffloadFlagForStartStream(int32_t startSessionId)
{
    std::unordered_map<uint32_t, bool> emptySpatializationMap;
    UpdateA2dpOffloadFlagCommon(startSessionId, DEVICE_TYPE_NONE, true, emptySpatializationMap);
}

uint32_t AudioA2dpOffloadManager::UpdateA2dpOffloadFlagForAllStream(DeviceType deviceType)
{
    std::unordered_map<uint32_t, bool> emptySpatializationMap;
    uint32_t runningSessionsSize =
        UpdateA2dpOffloadFlagCommon(SESSION_ID_INVALID, deviceType, true, emptySpatializationMap);
    return runningSessionsSize;
}

uint32_t AudioA2dpOffloadManager::UpdateA2dpOffloadFlagCommon(
    int32_t startSessionId, DeviceType deviceType, bool getSpatialFromService,
    std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnabledMap)
{
#ifdef BLUETOOTH_ENABLE
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    streamCollector_.GetCurrentRendererChangeInfos(rendererChangeInfos);

    std::vector<int32_t> allRunningSessions;
    std::vector<int32_t> allStopSessions;
    std::vector<Bluetooth::A2dpStreamInfo> allRunningA2dpInfos;
    Bluetooth::A2dpStreamInfo tmpA2dpStreamInfo;
    for (auto &changeInfo : rendererChangeInfos) {
        if (changeInfo->sessionId != startSessionId && changeInfo->rendererState != RENDERER_RUNNING) {
            allStopSessions.push_back(changeInfo->sessionId);
            continue;
        }
        allRunningSessions.push_back(changeInfo->sessionId);

        // Generate a2dp info
        tmpA2dpStreamInfo.sessionId = changeInfo->sessionId;
        tmpA2dpStreamInfo.streamType = streamCollector_.GetStreamType(changeInfo->sessionId);
        tmpA2dpStreamInfo.isSpatialAudio = GetSpatialAudio(getSpatialFromService,
            changeInfo->sessionId, changeInfo->rendererInfo.streamUsage,
            sessionIDToSpatializationEnabledMap);

        allRunningA2dpInfos.push_back(tmpA2dpStreamInfo);
    }
    if (allStopSessions.size() > 0) {
        OffloadStopPlaying(allStopSessions);
    }

    UpdateA2dpOffloadFlagInternal(allRunningA2dpInfos, allRunningSessions, deviceType);

    return allRunningSessions.size();
#endif
    return 0;
}

bool AudioA2dpOffloadManager::GetSpatialAudio(bool getSpatialFromService,
    int32_t sessionId, StreamUsage usage,
    std::unordered_map<uint32_t, bool> &sessionIDToSpatializationEnabledMap)
{
    bool isSpatial = false;
    if (getSpatialFromService) {
        // Get spatial from service may meet dead lock, use this case carefully
        AudioSpatializationState spatialState =
                AudioSpatializationService::GetAudioSpatializationService().GetSpatializationState(usage);
        isSpatial = spatialState.spatializationEnabled;
    } else {
        if (sessionIDToSpatializationEnabledMap.count(static_cast<uint32_t>(sessionId))) {
            isSpatial = sessionIDToSpatializationEnabledMap[static_cast<uint32_t>(sessionId)];
        } else {
            isSpatial = false;
        }
    }
    return isSpatial;
}

void AudioA2dpOffloadManager::UpdateA2dpOffloadFlagForA2dpDeviceOut()
{
    JUDGE_AND_INFO_LOG(GetA2dpOffloadFlag() != NO_A2DP_DEVICE,
        "a2dpOffloadFlag change from %{public}d to %{public}d", GetA2dpOffloadFlag(), NO_A2DP_DEVICE);

    // Get current running stream sessions to stop
    std::vector<int32_t> allRunningSessions;
    std::vector<std::shared_ptr<AudioRendererChangeInfo>> rendererChangeInfos;
    streamCollector_.GetCurrentRendererChangeInfos(rendererChangeInfos);
    for (auto &changeInfo : rendererChangeInfos) {
        if (changeInfo->rendererState != RENDERER_RUNNING) {
            continue;
        }
        allRunningSessions.push_back(changeInfo->sessionId);
    }
    OffloadStopPlaying(allRunningSessions);
    SetA2dpOffloadFlag(NO_A2DP_DEVICE);
}

#ifdef BLUETOOTH_ENABLE
void AudioA2dpOffloadManager::UpdateA2dpOffloadFlagInternal(
    const std::vector<Bluetooth::A2dpStreamInfo> &allRunningA2dpInfos,
    std::vector<int32_t> &allRunningSessions, DeviceType deviceType)
{
    if (allRunningA2dpInfos.size() == 0) {
        return;
    }
    BluetoothOffloadState newA2dpOffloadFlag = NO_A2DP_DEVICE;
    BluetoothOffloadState oldA2dpOffloadFlag = GetA2dpOffloadFlag();
    if (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP) {
        newA2dpOffloadFlag = static_cast<BluetoothOffloadState>(
                Bluetooth::AudioA2dpManager::A2dpOffloadSessionRequest(allRunningA2dpInfos));
    } else if (audioActiveDevice_.GetCurrentOutputDeviceType() == DEVICE_TYPE_BLUETOOTH_A2DP &&
        audioActiveDevice_.GetCurrentOutputDeviceNetworkId() == LOCAL_NETWORK_ID && deviceType == DEVICE_TYPE_NONE) {
        newA2dpOffloadFlag = static_cast<BluetoothOffloadState>(
                Bluetooth::AudioA2dpManager::A2dpOffloadSessionRequest(allRunningA2dpInfos));
    }

    std::lock_guard<std::mutex> lock(switchA2dpOffloadMutex_);
    AUDIO_PRERELEASE_LOGI("device: %{public}d, currentDevice: %{public}d, runStreamSize: %{public}zu, "
        "oldFlag: %{public}d, newFlag: %{public}d",
        deviceType, audioActiveDevice_.GetCurrentOutputDeviceType(), allRunningA2dpInfos.size(),
        oldA2dpOffloadFlag, newA2dpOffloadFlag);

    if (newA2dpOffloadFlag == NO_A2DP_DEVICE) {
        UpdateA2dpOffloadFlagForA2dpDeviceOut();
    } else if (newA2dpOffloadFlag != oldA2dpOffloadFlag) {
        if (oldA2dpOffloadFlag == A2DP_OFFLOAD) {
            HandleA2dpDeviceOutOffload(newA2dpOffloadFlag, allRunningSessions);
        } else if (newA2dpOffloadFlag == A2DP_OFFLOAD) {
            HandleA2dpDeviceInOffload(newA2dpOffloadFlag, allRunningSessions);
        } else {
            // Only NO_A2DP_DEVICE to A2DP_NOT_OFFLOAD case
            AUDIO_INFO_LOG("a2dpOffloadFlag change from %{public}d to %{public}d",
                oldA2dpOffloadFlag, newA2dpOffloadFlag);
            SetA2dpOffloadFlag(newA2dpOffloadFlag);
        }
    } else if (oldA2dpOffloadFlag == A2DP_OFFLOAD) {
        OffloadStartPlaying(allRunningSessions);
        AudioServerProxy::GetInstance().UpdateEffectBtOffloadSupportedProxy(true);
        GetA2dpOffloadCodecAndSendToDsp();
    }
}
#endif

int32_t AudioA2dpOffloadManager::HandleA2dpDeviceOutOffload(BluetoothOffloadState a2dpOffloadFlag,
    std::vector<int32_t> &allRunningSessions)
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("a2dpOffloadFlag change from %{public}d to %{public}d", GetA2dpOffloadFlag(), a2dpOffloadFlag);

    OffloadStopPlaying(allRunningSessions);
    SetA2dpOffloadFlag(a2dpOffloadFlag);
    DeviceType dev = audioActiveDevice_.GetCurrentOutputDeviceType();
    AudioPolicyUtils::GetInstance().UpdateEffectDefaultSink(dev);

    return SUCCESS;
#else
    return ERROR;
#endif
}

int32_t AudioA2dpOffloadManager::HandleA2dpDeviceInOffload(BluetoothOffloadState a2dpOffloadFlag,
    std::vector<int32_t> &allRunningSessions)
{
#ifdef BLUETOOTH_ENABLE
    AUDIO_INFO_LOG("a2dpOffloadFlag change from %{public}d to %{public}d", GetA2dpOffloadFlag(), a2dpOffloadFlag);

    SetA2dpOffloadFlag(a2dpOffloadFlag);
    GetA2dpOffloadCodecAndSendToDsp();

    OffloadStartPlaying(allRunningSessions);
    DeviceType dev = audioActiveDevice_.GetCurrentOutputDeviceType();
    AudioPolicyUtils::GetInstance().UpdateEffectDefaultSink(dev);

    AudioServerProxy::GetInstance().UpdateEffectBtOffloadSupportedProxy(true);

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
} // namespace AudioStandard
} // namespace OHOS
