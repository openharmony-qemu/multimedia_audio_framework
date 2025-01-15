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
#ifndef LOG_TAG
#define LOG_TAG "OfflineStreamInClient"
#endif

#include "offline_stream_in_client.h"

#include <mutex>

#include "ipc_offline_stream.h"
#include "ipc_skeleton.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "audio_manager_base.h"
#include "audio_service_log.h"
#include "audio_errors.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {
namespace {
mutex g_audioServerProxyMutex;
sptr<IStandardAudioService> gAudioServerProxy = nullptr;
static const sptr<IStandardAudioService> GetAudioServerProxy()
{
    lock_guard<mutex> lock(g_audioServerProxyMutex);
    if (gAudioServerProxy == nullptr) {
        auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        CHECK_AND_RETURN_RET_LOG(samgr != nullptr, nullptr, "get sa manager failed");
        sptr<IRemoteObject> object = samgr->GetSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID);
        CHECK_AND_RETURN_RET_LOG(object != nullptr, nullptr, "get audio service remote object failed");
        gAudioServerProxy = iface_cast<IStandardAudioService>(object);
        CHECK_AND_RETURN_RET_LOG(gAudioServerProxy != nullptr, nullptr, "get audio service proxy failed");
    }
    sptr<IStandardAudioService> gasp = gAudioServerProxy;
    return gasp;
}
}

shared_ptr<OfflineStreamInClient> OfflineStreamInClient::Create()
{
    sptr<IStandardAudioService> gasp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, nullptr, "Create failed, can not get service.");
    int32_t errCode = 0;
    sptr<IRemoteObject> ipcProxy = gasp->CreateIpcOfflineStream(errCode);
    CHECK_AND_RETURN_RET_LOG(errCode == 0, nullptr, "create audio stream fail, errcode is %{public}d.", errCode);
    CHECK_AND_RETURN_RET_LOG(ipcProxy != nullptr, nullptr, "Create failed with null ipcProxy.");
    sptr<IpcOfflineStream> iOfflineStreamProxy = iface_cast<IpcOfflineStream>(ipcProxy);
    CHECK_AND_RETURN_RET_LOG(iOfflineStreamProxy != nullptr, nullptr, "Create failed when iface_cast.");
    shared_ptr<OfflineStreamInClient> stream = make_shared<OfflineStreamInClient>(iOfflineStreamProxy);
    return stream;
}

int32_t OfflineStreamInClient::GetOfflineAudioEffectChains(std::vector<std::string> &effectChains)
{
    sptr<IStandardAudioService> gasp = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(gasp != nullptr, ERR_OPERATION_FAILED, "Create failed, can not get service.");
    return gasp->GetOfflineAudioEffectChains(effectChains);
}

OfflineStreamInClient::OfflineStreamInClient(const sptr<IpcOfflineStream> &ipcProxy) : streamProxy_(ipcProxy) {}

#ifdef FEATURE_OFFLINE_EFFECT
int32_t OfflineStreamInClient::CreateOfflineEffectChain(const std::string &effectName)
{
    CHECK_AND_RETURN_RET_LOG(streamProxy_ != nullptr, ERR_OPERATION_FAILED, "Create failed with null ipcProxy.");
    return streamProxy_->CreateOfflineEffectChain(effectName);
}

int32_t OfflineStreamInClient::ConfigureOfflineEffectChain(const AudioStreamInfo &inInfo,
    const AudioStreamInfo &outInfo)
{
    CHECK_AND_RETURN_RET_LOG(streamProxy_ != nullptr, ERR_OPERATION_FAILED, "Configure failed with null ipcProxy.");
    return streamProxy_->ConfigureOfflineEffectChain(inInfo, outInfo);
}

int32_t OfflineStreamInClient::PrepareOfflineEffectChain(std::shared_ptr<AudioSharedMemory> &clientInBuffer,
    std::shared_ptr<AudioSharedMemory> &clientOutBuffer)
{
    CHECK_AND_RETURN_RET_LOG(streamProxy_ != nullptr, ERR_OPERATION_FAILED, "Prepare failed with null ipcProxy.");
    return streamProxy_->PrepareOfflineEffectChain(clientInBuffer, clientOutBuffer);
}

int32_t OfflineStreamInClient::ProcessOfflineEffectChain(uint32_t inputSize, uint32_t outputSize)
{
    CHECK_AND_RETURN_RET_LOG(streamProxy_ != nullptr, ERR_OPERATION_FAILED, "Process failed with null ipcProxy.");
    return streamProxy_->ProcessOfflineEffectChain(inputSize, outputSize);
}

void OfflineStreamInClient::ReleaseOfflineEffectChain()
{
    CHECK_AND_RETURN_LOG(streamProxy_ != nullptr, "Release failed with null ipcProxy.");
    streamProxy_->ReleaseOfflineEffectChain();
}
#endif
} // namespace AudioStandard
} // namespace OHOS
