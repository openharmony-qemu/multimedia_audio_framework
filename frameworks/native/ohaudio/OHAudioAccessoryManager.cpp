/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "OHAudioAccessoryManager.h"
#include "audio_accessory.h"
#include "audio_errors.h"
#include "audio_common_log.h"
#include "iservice_registry.h"
#include "istandard_audio_service.h"
#include "audio_service_proxy.h"
#include "va_device_controller_stub_impl.h"
#include "va_device_broker_wrapper_impl.h"
#include "audio_device_info.h"

namespace {

OHOS::sptr<OHOS::AudioStandard::IStandardAudioService> GetAudioServerProxy()
{
    OHOS::sptr<OHOS::AudioStandard::IStandardAudioService> audioServer =
        OHOS::AudioStandard::AudioServiceProxy::GetAudioSystemManagerProxy();
    CHECK_AND_RETURN_RET_LOG(audioServer != nullptr, nullptr, "audio server proxy is null");
    return audioServer;
}

int32_t RegisterControllerToAudioServer(const std::string &address,
    const std::shared_ptr<OHOS::AudioStandard::VADeviceControllerCallback> &controller)
{
    AUDIO_INFO_LOG("register controller to audio server begin, addressLen=%{public}zu, "
        "controllerNull=%{public}d", address.size(), controller == nullptr);
    CHECK_AND_RETURN_RET_LOG(controller != nullptr, OHOS::AudioStandard::ERROR, "controller is null");
    OHOS::sptr<OHOS::AudioStandard::VADeviceControllerStubImpl> controllerStub =
        OHOS::sptr<OHOS::AudioStandard::VADeviceControllerStubImpl>::MakeSptr();
    CHECK_AND_RETURN_RET_LOG(controllerStub != nullptr, OHOS::AudioStandard::ERROR, "create controller stub failed");
    CHECK_AND_RETURN_RET_LOG(controllerStub->SetVADeviceControllerCallback(controller) == OHOS::AudioStandard::SUCCESS,
        OHOS::AudioStandard::ERROR,
        "set controller callback failed");
    OHOS::sptr<OHOS::AudioStandard::IStandardAudioService> audioServer = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(audioServer != nullptr, OHOS::AudioStandard::ERROR, "audio server is null");
    return audioServer->RegisterVADeviceController(address, controllerStub->AsObject());
}

int32_t UnregisterControllerFromAudioServer(const std::string &address)
{
    OHOS::sptr<OHOS::AudioStandard::IStandardAudioService> audioServer = GetAudioServerProxy();
    CHECK_AND_RETURN_RET_LOG(audioServer != nullptr, OHOS::AudioStandard::ERROR, "audio server is null");
    return audioServer->UnregisterVADeviceController(address);
}
} // namespace

namespace OHOS {
namespace AudioStandard {

OHAudioAccessoryManager& OHAudioAccessoryManager::GetInstance()
{
    static OHAudioAccessoryManager instance;
    return instance;
}

OH_AudioCommon_Result OHAudioAccessoryManager::Create(
    const OH_AudioAccessoryInfo* info,
    const OH_AudioAccessoryCapabilities* capabilities,
    OH_AudioAccessory_OpenInputStreamCallback onOpenInputStream,
    OH_AudioAccessory** accessory)
{
    CHECK_AND_RETURN_RET_LOG(info != nullptr && capabilities != nullptr &&
        onOpenInputStream != nullptr && accessory != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "Create: invalid param");

    if (info->structSize < sizeof(OH_AudioAccessoryInfo)) {
        AUDIO_ERR_LOG("Invalid info structSize: %{public}u, expected %{public}zu",
            info->structSize, sizeof(OH_AudioAccessoryInfo));
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }
    if (capabilities->structSize < sizeof(OH_AudioAccessoryCapabilities)) {
        AUDIO_ERR_LOG("Invalid capabilities structSize: %{public}u, expected %{public}zu",
            capabilities->structSize, sizeof(OH_AudioAccessoryCapabilities));
        return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    for (const auto& [ptr, acc] : accessoryMap_) {
        if (acc->GetMacAddress() == info->macAddress) {
            return AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM;
        }
    }

    auto newAccessory = std::make_shared<AudioAccessory>(*info, nullptr, onOpenInputStream);
    newAccessory->SetCapabilities(*capabilities);
    *accessory = reinterpret_cast<OH_AudioAccessory*>(newAccessory.get());
    accessoryMap_[*accessory] = newAccessory;
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioAccessoryManager::RegisterNoiseReductionCapability(
    OH_AudioAccessory* accessory,
    const OH_AudioAccessoryNoiseReductionCapability* capability,
    OH_AudioAccessory_SetNoiseReductionCallback onNoiseReduction)
{
    CHECK_AND_RETURN_RET_LOG(accessory != nullptr && capability != nullptr &&
        capability->supportedModes != nullptr && capability->supportedModeCount != 0,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "RegisterNoiseReductionCapability: invalid param");

    std::lock_guard<std::mutex> lock(mutex_);

    auto it = accessoryMap_.find(accessory);
    CHECK_AND_RETURN_RET_LOG(it != accessoryMap_.end(),
        AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE, "RegisterNoiseReductionCapability: accessory not found");

    std::vector<OH_AudioNoiseReductionMode> modes(
        capability->supportedModes, capability->supportedModes + capability->supportedModeCount);
    it->second->SetNoiseReductionCapability(modes, capability->currentMode, onNoiseReduction);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioAccessoryManager::SetNoiseReductionMode(
    OH_AudioAccessory* accessory,
    OH_AudioNoiseReductionMode mode)
{
    CHECK_AND_RETURN_RET_LOG(accessory != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "SetNoiseReductionMode: invalid param");

    std::shared_ptr<AudioAccessory> acc;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = accessoryMap_.find(accessory);
        CHECK_AND_RETURN_RET_LOG(it != accessoryMap_.end(),
            AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE, "SetNoiseReductionMode: accessory not found");
        acc = it->second;
    }

    int32_t ret = acc->SetNoiseReductionMode(mode);
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("SetNoiseReductionMode failed: %{public}d", ret);
        if (ret == ERR_ILLEGAL_STATE) {
            return AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE;
        }
        return AUDIOCOMMON_RESULT_ERROR_UNSUPPORTED;
    }

    VADeviceBrokerWrapperImpl brokerWrapper;
    ret = brokerWrapper.SetNoiseReductionMode(acc->MakeVADevice(), static_cast<int32_t>(mode));
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "SetNoiseReductionMode to broker failed");
    AUDIO_INFO_LOG("SetNoiseReductionMode: mode=%{public}d", static_cast<int>(mode));
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioAccessoryManager::SetAssociatedMacAddresses(
    OH_AudioAccessory* accessory,
    const char** macAddresses,
    uint32_t count)
{
    CHECK_AND_RETURN_RET_LOG(accessory != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "invalid param");

    CHECK_AND_RETURN_RET_LOG(macAddresses != nullptr || count == 0,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "invalid macAddresses");

    std::shared_ptr<AudioAccessory> acc;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = accessoryMap_.find(accessory);
        CHECK_AND_RETURN_RET_LOG(it != accessoryMap_.end(),
            AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE, "accessory not created");
        acc = it->second;
    }

    for (uint32_t i = 0; i < count; i++) {
        CHECK_AND_RETURN_RET_LOG(macAddresses[i] != nullptr,
            AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM,
            "macAddresses[%{public}u] is null", i);
    }

    VADeviceBrokerWrapperImpl brokerWrapper;

    for (const auto& macAddr : associatedMacAddresses_) {
        int32_t ret = brokerWrapper.SetAssociatedMacAddresses(macAddr, true);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, AUDIOCOMMON_RESULT_SUCCESS,
            "failed to enable device");
    }

    associatedMacAddresses_.clear();
    associatedMacAddresses_.reserve(count);
    for (uint32_t i = 0; i < count; i++) {
        associatedMacAddresses_.emplace_back(macAddresses[i]);
    }

    for (const auto& macAddr : associatedMacAddresses_) {
        int32_t ret = brokerWrapper.SetAssociatedMacAddresses(macAddr, false);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, AUDIOCOMMON_RESULT_SUCCESS,
            "failed to disable device");
    }

    AUDIO_INFO_LOG("macCount=%{public}u", count);
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioAccessoryManager::NotifyConnected(OH_AudioAccessory* accessory)
{
    CHECK_AND_RETURN_RET_LOG(accessory != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "NotifyConnected: invalid param");

    std::shared_ptr<AudioAccessory> acc;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = accessoryMap_.find(accessory);
        CHECK_AND_RETURN_RET_LOG(it != accessoryMap_.end(),
            AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE, "NotifyConnected: accessory not found");
        acc = it->second;
    }

    int32_t ret = acc->NotifyConnected();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("NotifyConnected failed: %{public}d", ret);
        CHECK_AND_RETURN_RET(ret != ERR_ILLEGAL_STATE, AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE);
        CHECK_AND_RETURN_RET(ret != ERR_PERMISSION_DENIED && ret != ERR_SYSTEM_PERMISSION_DENIED,
            AUDIOCOMMON_RESULT_ERROR_PERMISSION_DENIED);
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }

    VADeviceBrokerWrapperImpl brokerWrapper;
    ret = RegisterControllerToAudioServer(acc->GetMacAddress(), acc);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, AUDIOCOMMON_RESULT_ERROR_SYSTEM,
        "register va device controller failed");
    ret = brokerWrapper.OnDevicesConnected(acc->MakeVADevice(), acc);
    if (ret != SUCCESS) {
        (void)UnregisterControllerFromAudioServer(acc->GetMacAddress());
        AUDIO_ERR_LOG("notify va device connected failed: %{public}d", ret);
        CHECK_AND_RETURN_RET(ret != ERR_PERMISSION_DENIED && ret != ERR_SYSTEM_PERMISSION_DENIED,
            AUDIOCOMMON_RESULT_ERROR_PERMISSION_DENIED);
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }

    AUDIO_INFO_LOG("NotifyConnected: Exit");
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioAccessoryManager::NotifyDisconnected(OH_AudioAccessory* accessory)
{
    CHECK_AND_RETURN_RET_LOG(accessory != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "NotifyDisconnected: invalid param");

    std::shared_ptr<AudioAccessory> acc;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = accessoryMap_.find(accessory);
        if (it == accessoryMap_.end()) {
            AUDIO_INFO_LOG("Accessory already removed");
            return AUDIOCOMMON_RESULT_SUCCESS;
        }
        acc = it->second;
    }

    int32_t ret = acc->NotifyDisconnected();
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("NotifyDisconnected failed: %{public}d", ret);
        CHECK_AND_RETURN_RET(ret != ERR_ILLEGAL_STATE, AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE);
        CHECK_AND_RETURN_RET(ret != ERR_PERMISSION_DENIED && ret != ERR_SYSTEM_PERMISSION_DENIED,
            AUDIOCOMMON_RESULT_ERROR_PERMISSION_DENIED);
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }

    VADeviceBrokerWrapperImpl brokerWrapper;
    ret = brokerWrapper.OnDevicesDisconnected(acc->MakeVADevice());
    if (ret != SUCCESS) {
        AUDIO_ERR_LOG("notify va device disconnected failed: %{public}d", ret);
        CHECK_AND_RETURN_RET(ret != ERR_PERMISSION_DENIED && ret != ERR_SYSTEM_PERMISSION_DENIED,
            AUDIOCOMMON_RESULT_ERROR_PERMISSION_DENIED);
        return AUDIOCOMMON_RESULT_ERROR_SYSTEM;
    }

    int32_t unregisterRet = UnregisterControllerFromAudioServer(acc->GetMacAddress());
    if (unregisterRet != SUCCESS) {
        AUDIO_WARNING_LOG("unregister va device controller failed");
    }
    AUDIO_INFO_LOG("NotifyDisconnected");
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OHAudioAccessoryManager::Release(OH_AudioAccessory* accessory)
{
    CHECK_AND_RETURN_RET_LOG(accessory != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM, "Release: invalid param");

    std::shared_ptr<AudioAccessory> acc;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = accessoryMap_.find(accessory);
        CHECK_AND_RETURN_RET_LOG(it != accessoryMap_.end(),
            AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE, "Release: accessory not found");
        acc = it->second;
        CHECK_AND_RETURN_RET_LOG(!acc->IsConnected(), AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE,
            "Release: accessory is still connected");
        accessoryMap_.erase(it);
    }
    AUDIO_INFO_LOG("Release");
    return AUDIOCOMMON_RESULT_SUCCESS;
}
} // namespace AudioStandard
} // namespace OHOS

extern "C" {
OH_AudioCommon_Result OH_AudioManager_GetAccessoryManager(OH_AudioAccessoryManager** outManager)
{
    CHECK_AND_RETURN_RET(outManager != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
    *outManager = reinterpret_cast<OH_AudioAccessoryManager*>(
        &OHOS::AudioStandard::OHAudioAccessoryManager::GetInstance());
    return AUDIOCOMMON_RESULT_SUCCESS;
}

OH_AudioCommon_Result OH_AudioAccessoryManager_CreateInput(
    OH_AudioAccessoryManager* manager,
    const OH_AudioAccessoryInfo* info,
    const OH_AudioAccessoryCapabilities* capabilities,
    OH_AudioAccessory_OpenInputStreamCallback onOpenInputStream,
    OH_AudioAccessory** outOwnedAccessory)
{
    CHECK_AND_RETURN_RET(manager != nullptr, AUDIOCOMMON_RESULT_ERROR_ILLEGAL_STATE);
    CHECK_AND_RETURN_RET(info != nullptr && capabilities != nullptr &&
        onOpenInputStream != nullptr && outOwnedAccessory != nullptr, AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
    return OHOS::AudioStandard::OHAudioAccessoryManager::GetInstance().Create(
        info, capabilities, onOpenInputStream, outOwnedAccessory);
}

OH_AudioCommon_Result OH_AudioAccessoryManager_SetAssociatedMacAddresses(
    OH_AudioAccessoryManager *manager,
    OH_AudioAccessory *accessory,
    const char **macAddresses,
    uint32_t count)
{
    CHECK_AND_RETURN_RET(manager != nullptr && accessory != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
    return OHOS::AudioStandard::OHAudioAccessoryManager::GetInstance().SetAssociatedMacAddresses(
        accessory, macAddresses, count);
}

OH_AudioCommon_Result OH_AudioAccessoryManager_RegisterNoiseReductionCapability(
    OH_AudioAccessoryManager* manager,
    OH_AudioAccessory* accessory,
    const OH_AudioAccessoryNoiseReductionCapability* capability,
    OH_AudioAccessory_SetNoiseReductionCallback onNoiseReduction)
{
    CHECK_AND_RETURN_RET(manager != nullptr && accessory != nullptr && capability != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
    return OHOS::AudioStandard::OHAudioAccessoryManager::GetInstance().RegisterNoiseReductionCapability(
        accessory, capability, onNoiseReduction);
}

OH_AudioCommon_Result OH_AudioAccessoryManager_SetNoiseReductionMode(
    OH_AudioAccessoryManager* manager,
    OH_AudioAccessory* accessory,
    OH_AudioNoiseReductionMode mode)
{
    CHECK_AND_RETURN_RET(manager != nullptr && accessory != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
    return OHOS::AudioStandard::OHAudioAccessoryManager::GetInstance().SetNoiseReductionMode(accessory, mode);
}

OH_AudioCommon_Result OH_AudioAccessoryManager_Connected(
    OH_AudioAccessoryManager* manager,
    OH_AudioAccessory* accessory)
{
    CHECK_AND_RETURN_RET(manager != nullptr && accessory != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
    return OHOS::AudioStandard::OHAudioAccessoryManager::GetInstance().NotifyConnected(accessory);
}

OH_AudioCommon_Result OH_AudioAccessoryManager_Disconnected(
    OH_AudioAccessoryManager* manager,
    OH_AudioAccessory* accessory)
{
    CHECK_AND_RETURN_RET(manager != nullptr && accessory != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
    return OHOS::AudioStandard::OHAudioAccessoryManager::GetInstance().NotifyDisconnected(accessory);
}

OH_AudioCommon_Result OH_AudioAccessoryManager_Destroy(
    OH_AudioAccessoryManager* manager,
    OH_AudioAccessory* accessory)
{
    CHECK_AND_RETURN_RET(manager != nullptr && accessory != nullptr,
        AUDIOCOMMON_RESULT_ERROR_INVALID_PARAM);
    return OHOS::AudioStandard::OHAudioAccessoryManager::GetInstance().Release(accessory);
}

} // extern "C"
