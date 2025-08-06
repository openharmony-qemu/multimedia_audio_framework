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
#define LOG_TAG "AudioZoneManager"
#endif

#include "audio_zone_manager.h"

#include "audio_errors.h"
#include "audio_log.h"
#include "audio_policy_manager.h"
#include "audio_zone_client.h"

namespace OHOS {
namespace AudioStandard {
class AudioZoneManagerInner : public AudioZoneManager {
public:
    AudioZoneManagerInner() = default;
    ~AudioZoneManagerInner() = default;

    int32_t CreateAudioZone(const std::string &name, const AudioZoneContext &context) override;

    void ReleaseAudioZone(int32_t zoneId) override;

    const std::vector<std::shared_ptr<AudioZoneDescriptor>> GetAllAudioZone() override;

    const std::shared_ptr<AudioZoneDescriptor> GetAudioZone(int32_t zoneId) override;

    int32_t GetAudioZoneByName(std::string name) override;

    int32_t BindDeviceToAudioZone(int32_t zoneId,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices) override;

    int32_t UnBindDeviceToAudioZone(int32_t zoneId,
        std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices) override;

    int32_t RegisterAudioZoneCallback(const std::shared_ptr<AudioZoneCallback> &callback) override;

    int32_t UnRegisterAudioZoneCallback() override;

    int32_t RegisterAudioZoneChangeCallback(int32_t zoneId,
        const std::shared_ptr<AudioZoneChangeCallback> &callback) override;
    
    int32_t UnRegisterAudioZoneChangeCallback(int32_t zoneId) override;

    int32_t AddUidToAudioZone(int32_t zoneId, int32_t uid) override;

    int32_t RemoveUidFromAudioZone(int32_t zoneId, int32_t uid) override;

    int32_t AddStreamToAudioZone(int32_t zoneId, AudioZoneStream stream) override;

    int32_t AddStreamsToAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams) override;

    int32_t RemoveStreamFromAudioZone(int32_t zoneId, AudioZoneStream stream) override;

    int32_t RemoveStreamsFromAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams) override;

    void SetZoneDeviceVisible(bool visible) override;

    int32_t RegisterSystemVolumeProxy(int32_t zoneId,
        const std::shared_ptr<AudioZoneVolumeProxy> &proxy) override;

    int32_t UnRegisterSystemVolumeProxy(int32_t zoneId) override;

    std::list<std::pair<AudioInterrupt, AudioFocuState>> GetAudioInterruptForZone(
        int32_t zoneId) override;
    
    std::list<std::pair<AudioInterrupt, AudioFocuState>> GetAudioInterruptForZone(
        int32_t zoneId, const std::string &deviceTag) override;
    
    int32_t RegisterAudioZoneInterruptCallback(int32_t zoneId,
        const std::shared_ptr<AudioZoneInterruptCallback> &callback) override;
    
    int32_t UnRegisterAudioZoneInterruptCallback(int32_t zoneId) override;

    int32_t RegisterAudioZoneInterruptCallback(int32_t zoneId, const std::string &deviceTag,
        const std::shared_ptr<AudioZoneInterruptCallback> &callback) override;
    
    int32_t UnRegisterAudioZoneInterruptCallback(int32_t zoneId,
        const std::string &deviceTag) override;
    
    int32_t InjectInterruptToAudioZone(int32_t zoneId,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts) override;
    
    int32_t InjectInterruptToAudioZone(int32_t zoneId, const std::string &deviceTag,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts) override;

private:
    sptr<AudioZoneClient> client_;
    std::mutex clientMutex_;

    int32_t RegisterAudioZoneClient();
};

AudioZoneManager *AudioZoneManager::GetInstance()
{
    static AudioZoneManagerInner audioZoneManager;
    return &audioZoneManager;
}

int32_t AudioZoneManagerInner::RegisterAudioZoneClient()
{
    CHECK_AND_RETURN_RET_LOG(client_ == nullptr, SUCCESS, "client_ has registered!");
    sptr<AudioZoneClient> temp = new(std::nothrow) AudioZoneClient();
    CHECK_AND_RETURN_RET_LOG(temp != nullptr, ERROR, "temp client is nullptr!");
    sptr<IRemoteObject> object = temp->AsObject();
    CHECK_AND_RETURN_RET_LOG(object != nullptr, ERROR, "client asobject is nullptr!");

    int32_t ret = AudioPolicyManager::GetInstance().RegisterAudioZoneClient(object);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ERROR, "register audio zone client failed.");
    client_ = temp;
    AudioPolicyManager::RegisterServerDiedCallBack([this]() {
        CHECK_AND_RETURN_LOG(this->client_ != nullptr, "client_ is nullptr!");
        this->client_->Restore();
    });
    return SUCCESS;
}
int32_t AudioZoneManagerInner::CreateAudioZone(const std::string &name, const AudioZoneContext &context)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(!name.empty(), ERR_INVALID_PARAM, "name is empty!");

    int32_t zoneId = AudioPolicyManager::GetInstance().CreateAudioZone(name, context);
    
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_OPERATION_FAILED, "CreateAudioZone result:%{public}d", zoneId);
    return zoneId;
}

int32_t AudioZoneManagerInner::GetAudioZoneByName(std::string name)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(!name.empty(), ERR_INVALID_PARAM, "name is empty!");

    int32_t zoneId = AudioPolicyManager::GetInstance().GetAudioZoneByName(name);
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_OPERATION_FAILED, "GetAudioZoneByName result:%{public}d", zoneId);
    return zoneId;
}

int32_t AudioZoneManagerInner::BindDeviceToAudioZone(int32_t zoneId,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");
    CHECK_AND_RETURN_RET_LOG(devices.size() > 0, ERR_INVALID_PARAM, "devices is empty");

    int32_t result = AudioPolicyManager::GetInstance().BindDeviceToAudioZone(zoneId, devices);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "BindDeviceToAudioZone result:%{public}d", result);
    return result;
}

int32_t AudioZoneManagerInner::UnBindDeviceToAudioZone(int32_t zoneId,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> devices)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");
    CHECK_AND_RETURN_RET_LOG(devices.size() > 0, ERR_INVALID_PARAM, "devices is empty");

    int32_t result = AudioPolicyManager::GetInstance().UnBindDeviceToAudioZone(zoneId, devices);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "UnBindDeviceToAudioZone result:%{public}d", result);
    return result;
}

void AudioZoneManagerInner::ReleaseAudioZone(int32_t zoneId)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_LOG(zoneId > 0, "zoneId is invalid");
    AudioPolicyManager::GetInstance().ReleaseAudioZone(zoneId);
}

const std::vector<std::shared_ptr<AudioZoneDescriptor>> AudioZoneManagerInner::GetAllAudioZone()
{
    AUDIO_INFO_LOG("in");
    return AudioPolicyManager::GetInstance().GetAllAudioZone();
}

const std::shared_ptr<AudioZoneDescriptor> AudioZoneManagerInner::GetAudioZone(int32_t zoneId)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, nullptr, "zoneId is invalid");
    return AudioPolicyManager::GetInstance().GetAudioZone(zoneId);
}

int32_t AudioZoneManagerInner::RegisterAudioZoneCallback(const std::shared_ptr<AudioZoneCallback> &callback)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    std::unique_lock<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_RET_LOG(RegisterAudioZoneClient() == SUCCESS, ERROR,
        "RegisterAudioZoneClient failed!");
    CHECK_AND_RETURN_RET_LOG(client_ != nullptr, ERROR, "client_ is nullptr!");

    client_->AddAudioZoneCallback(callback);
    return SUCCESS;
}

int32_t AudioZoneManagerInner::UnRegisterAudioZoneCallback()
{
    AUDIO_INFO_LOG("in");
    std::unique_lock<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_RET_LOG(client_ != nullptr, ERROR, "client_ is nullptr!");

    client_->RemoveAudioZoneCallback();
    return SUCCESS;
}

int32_t AudioZoneManagerInner::RegisterAudioZoneChangeCallback(int32_t zoneId,
    const std::shared_ptr<AudioZoneChangeCallback> &callback)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    std::unique_lock<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_RET_LOG(RegisterAudioZoneClient() == SUCCESS, ERROR,
        "RegisterAudioZoneClient failed!");
    CHECK_AND_RETURN_RET_LOG(client_ != nullptr, ERROR, "client_ is nullptr!");

    client_->AddAudioZoneChangeCallback(zoneId, callback);
    return SUCCESS;
}

int32_t AudioZoneManagerInner::UnRegisterAudioZoneChangeCallback(int32_t zoneId)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");

    std::unique_lock<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_RET_LOG(client_ != nullptr, ERROR, "client_ is nullptr!");

    client_->RemoveAudioZoneChangeCallback(zoneId);
    return SUCCESS;
}

int32_t AudioZoneManagerInner::AddUidToAudioZone(int32_t zoneId, int32_t uid)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");
    
    int32_t result = AudioPolicyManager::GetInstance().AddUidToAudioZone(zoneId, uid);

    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "AddUidToAudioZone result:%{public}d", result);
    return result;
}

int32_t AudioZoneManagerInner::RemoveUidFromAudioZone(int32_t zoneId, int32_t uid)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");

    int32_t result = AudioPolicyManager::GetInstance().RemoveUidFromAudioZone(zoneId, uid);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "RemoveUidFromAudioZone result:%{public}d", result);
    return result;
}

int32_t AudioZoneManagerInner::AddStreamToAudioZone(int32_t zoneId, AudioZoneStream stream)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");

    int32_t result = AudioPolicyManager::GetInstance().AddStreamToAudioZone(zoneId, stream);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "AddStreamToAudioZone result:%{public}d", result);
    return result;
}

int32_t AudioZoneManagerInner::AddStreamsToAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");
    CHECK_AND_RETURN_RET_LOG(streams.size() > 0, ERR_INVALID_PARAM, "streams is empty");

    int32_t result = AudioPolicyManager::GetInstance().AddStreamsToAudioZone(zoneId, streams);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "AddStreamsToAudioZone result:%{public}d", result);
    return result;
}

void AudioZoneManagerInner::SetZoneDeviceVisible(bool visible)
{
    AUDIO_INFO_LOG("in");
    AudioPolicyManager::GetInstance().SetZoneDeviceVisible(visible);
}

int32_t AudioZoneManagerInner::RemoveStreamFromAudioZone(int32_t zoneId, AudioZoneStream stream)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");

    int32_t result = AudioPolicyManager::GetInstance().RemoveStreamFromAudioZone(zoneId, stream);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "RemoveStreamFromAudioZone result:%{public}d", result);
    return result;
}

int32_t AudioZoneManagerInner::RemoveStreamsFromAudioZone(int32_t zoneId, std::vector<AudioZoneStream> streams)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");
    CHECK_AND_RETURN_RET_LOG(streams.size() > 0, ERR_INVALID_PARAM, "streams is empty");

    int32_t result = AudioPolicyManager::GetInstance().RemoveStreamsFromAudioZone(zoneId, streams);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "RemoveStreamsFromAudioZone result:%{public}d", result);
    return result;
}

int32_t AudioZoneManagerInner::RegisterSystemVolumeProxy(int32_t zoneId,
    const std::shared_ptr<AudioZoneVolumeProxy> &proxy)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");
    CHECK_AND_RETURN_RET_LOG(proxy != nullptr, ERR_INVALID_PARAM, "proxy is nullptr");
    
    std::unique_lock<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_RET_LOG(RegisterAudioZoneClient() == SUCCESS, ERROR,
        "RegisterAudioZoneClient failed!");
    CHECK_AND_RETURN_RET_LOG(client_ != nullptr, ERROR, "client_ is nullptr!");

    client_->AddAudioZoneVolumeProxy(zoneId, proxy);
    return SUCCESS;
}

int32_t AudioZoneManagerInner::UnRegisterSystemVolumeProxy(int32_t zoneId)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");

    std::unique_lock<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_RET_LOG(client_ != nullptr, ERROR, "client_ is nullptr!");

    client_->RemoveAudioZoneVolumeProxy(zoneId);
    return SUCCESS;
}

std::list<std::pair<AudioInterrupt, AudioFocuState>> AudioZoneManagerInner::GetAudioInterruptForZone(
    int32_t zoneId)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, {}, "zoneId is invalid");

    return AudioPolicyManager::GetInstance().GetAudioInterruptForZone(zoneId);
}

std::list<std::pair<AudioInterrupt, AudioFocuState>> AudioZoneManagerInner::GetAudioInterruptForZone(
    int32_t zoneId, const std::string &deviceTag)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, {}, "zoneId is invalid");
    return AudioPolicyManager::GetInstance().GetAudioInterruptForZone(zoneId, deviceTag);
}

int32_t AudioZoneManagerInner::RegisterAudioZoneInterruptCallback(int32_t zoneId,
    const std::shared_ptr<AudioZoneInterruptCallback> &callback)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");

    std::unique_lock<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_RET_LOG(RegisterAudioZoneClient() == SUCCESS, ERROR,
        "RegisterAudioZoneClient failed!");
    CHECK_AND_RETURN_RET_LOG(client_ != nullptr, ERROR, "client_ is nullptr!");

    client_->AddAudioInterruptCallback(zoneId, callback);
    return SUCCESS;
}

int32_t AudioZoneManagerInner::UnRegisterAudioZoneInterruptCallback(int32_t zoneId)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");

    std::unique_lock<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_RET_LOG(client_ != nullptr, ERROR, "client_ is nullptr!");

    client_->RemoveAudioInterruptCallback(zoneId);
    return SUCCESS;
}

int32_t AudioZoneManagerInner::RegisterAudioZoneInterruptCallback(int32_t zoneId, const std::string &deviceTag,
    const std::shared_ptr<AudioZoneInterruptCallback> &callback)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    
    std::unique_lock<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_RET_LOG(RegisterAudioZoneClient() == SUCCESS, ERROR,
        "RegisterAudioZoneClient failed!");
    CHECK_AND_RETURN_RET_LOG(client_ != nullptr, ERROR, "client_ is nullptr!");

    client_->AddAudioInterruptCallback(zoneId, deviceTag, callback);
    return SUCCESS;
}

int32_t AudioZoneManagerInner::UnRegisterAudioZoneInterruptCallback(int32_t zoneId, const std::string &deviceTag)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");

    std::unique_lock<std::mutex> lock(clientMutex_);
    CHECK_AND_RETURN_RET_LOG(client_ != nullptr, ERROR, "client_ is nullptr!");

    client_->RemoveAudioInterruptCallback(zoneId, deviceTag);
    return SUCCESS;
}

int32_t AudioZoneManagerInner::InjectInterruptToAudioZone(int32_t zoneId,
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");
    
    int32_t result = AudioPolicyManager::GetInstance().InjectInterruptToAudioZone(zoneId, interrupts);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "InjectInterruptToAudioZone result:%{public}d", result);
    return result;
}

int32_t AudioZoneManagerInner::InjectInterruptToAudioZone(int32_t zoneId, const std::string &deviceTag,
    const std::list<std::pair<AudioInterrupt, AudioFocuState>> &interrupts)
{
    AUDIO_INFO_LOG("in");
    CHECK_AND_RETURN_RET_LOG(zoneId > 0, ERR_INVALID_PARAM, "zoneId is invalid");

    int32_t result = AudioPolicyManager::GetInstance().InjectInterruptToAudioZone(zoneId, deviceTag, interrupts);
    CHECK_AND_RETURN_RET_LOG(result == SUCCESS, ERR_OPERATION_FAILED,
        "InjectInterruptToAudioZone result:%{public}d", result);
    return result;
}
} // namespace AudioStandard
} // namespace OHOS