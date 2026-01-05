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

#ifndef LOG_TAG
#define LOG_TAG "AudioStreamClientManager"
#endif

#include "audio_log.h"
#include "audio_errors.h"
#include "audio_stream_client_manager.h"
#include "audio_service_proxy.h"
#include "audio_manager_listener_stub_impl.h"
#include "ipc_skeleton.h"
#include "audio_server_death_recipient.h"

namespace OHOS {
namespace AudioStandard {

std::mutex g_audioListenerMutex;
sptr<AudioManagerListenerStubImpl> g_audioListener = nullptr;

void AudioServerDied(pid_t pid, pid_t uid)
{
    AUDIO_INFO_LOG("audio server died, will restore proxy in next call");
    std::lock_guard<std::mutex> lock(g_audioListenerMutex);
    g_audioListener = nullptr;
}

AudioStreamClientManager &AudioStreamClientManager::GetInstance()
{
    static AudioStreamClientManager instance;
    return instance;
}

int32_t AudioStreamClientManager::GetVolumeBySessionId(const uint32_t &sessionId, float &volume)
{
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_INVALID_PARAM, "Audio service unavailable.");
    std::string identity = IPCSkeleton::ResetCallingIdentity();
    int32_t ret = gasp->GetVolumeBySessionId(sessionId, volume);
    IPCSkeleton::SetCallingIdentity(identity);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "failed: %{public}d", ret);
    return ret;
}

int32_t AudioStreamClientManager::RegisterRendererDataTransferCallback(const DataTransferMonitorParam &param,
    const std::shared_ptr<AudioRendererDataTransferStateChangeCallback> &callback)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERROR_INVALID_PARAM, "callback is null");
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERROR, "Audio service unavailable.");

    int32_t ret = SUCCESS;

    std::lock_guard<std::mutex> lock(g_audioListenerMutex);
    if (g_audioListener == nullptr) {
        g_audioListener = new(std::nothrow) AudioManagerListenerStubImpl();
        if (g_audioListener == nullptr) {
            AUDIO_ERR_LOG("g_audioListener is null");
            return ERROR;
        }

        sptr<IRemoteObject> object = g_audioListener->AsObject();
        if (object == nullptr) {
            AUDIO_ERR_LOG("as object result is null");
            g_audioListener = nullptr;
            return ERROR;
        }

        ret = gasp->RegisterDataTransferCallback(object);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR_INVALID_PARAM, "ret: %{public}d", ret);

        // register death recipent to restore proxy
        sptr<AudioServerDeathRecipient> asDeathRecipient =
            new(std::nothrow) AudioServerDeathRecipient(getpid(), getuid());
        if (asDeathRecipient != nullptr) {
            asDeathRecipient->SetNotifyCb([] (pid_t pid, pid_t uid) {
                AudioServerDied(pid, uid);
            });
            bool result = object->AddDeathRecipient(asDeathRecipient);
            if (!result) {
                AUDIO_ERR_LOG("failed to add deathRecipient");
            }
        }
    }

    auto callbackId = g_audioListener->AddDataTransferStateChangeCallback(param, callback);
    CHECK_AND_RETURN_RET_LOG(callbackId != -1, ERROR_SYSTEM, "out of max register count");
    ret = gasp->RegisterDataTransferMonitorParam(callbackId, param);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR_INVALID_PARAM, "ret: %{public}d", ret);
    return ret;
}


int32_t AudioStreamClientManager::UnregisterRendererDataTransferCallback(
    const std::shared_ptr<AudioRendererDataTransferStateChangeCallback> &callback)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERROR_INVALID_PARAM, "callback is null");
    const sptr<IStandardAudioService> gasp = AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERROR, "Audio service unavailable.");

    std::lock_guard<std::mutex> lock(g_audioListenerMutex);
    CHECK_AND_RETURN_RET_LOG(g_audioListener != nullptr, ERROR_INVALID_PARAM, "audio listener is null");
    auto callbackIds = g_audioListener->RemoveDataTransferStateChangeCallback(callback);

    for (auto callbackId : callbackIds) {
        gasp->UnregisterDataTransferMonitorParam(callbackId);
    }

    return SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS
