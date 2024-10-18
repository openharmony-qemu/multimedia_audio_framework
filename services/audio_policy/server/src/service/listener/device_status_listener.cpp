/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "DeviceStatusListener"
#endif

#include "device_status_listener.h"
#include <securec.h>
#include "hdf_device_class.h"
#include "v4_0/audio_types.h"
#ifdef BLUETOOTH_ENABLE

#include "audio_bluetooth_manager.h"
#include "bluetooth_def.h"

#endif

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
const std::string AUDIO_HDI_SERVICE_NAME = "audio_manager_service";
const std::string AUDIO_HDI_PNP_SERVICE_NAME = "audio_hdi_pnp_service";
const std::string AUDIO_BLUETOOTH_HDI_SERVICE_NAME = "audio_bluetooth_hdi_service";
const std::string DAUDIO_HDI_SERVICE_NAME = "daudio_primary_service";
const std::string DP_ADDRESS = "card=0;port=";
const uint8_t EVENT_NUM_TYPE = 2;
const uint8_t EVENT_PARAMS = 4;
const uint8_t D_EVENT_PARAMS = 5;

static DeviceType GetInternalDeviceType(PnpDeviceType pnpDeviceType)
{
    DeviceType internalDeviceType = DEVICE_TYPE_NONE;

    switch (pnpDeviceType) {
        case PnpDeviceType::PNP_DEVICE_HEADSET:
            internalDeviceType = DEVICE_TYPE_WIRED_HEADSET;
            break;
        case PnpDeviceType::PNP_DEVICE_HEADPHONE:
            internalDeviceType = DEVICE_TYPE_WIRED_HEADPHONES;
            break;
        case PnpDeviceType::PNP_DEVICE_USB_HEADSET:
            internalDeviceType = DEVICE_TYPE_USB_HEADSET;
            break;
        case PnpDeviceType::PNP_DEVICE_ADAPTER_DEVICE:
            internalDeviceType = DEVICE_TYPE_EXTERN_CABLE;
            break;
        case PnpDeviceType::PNP_DEVICE_DP_DEVICE:
            internalDeviceType = DEVICE_TYPE_DP;
            break;
        case PnpDeviceType::PNP_DEVICE_MIC:
            internalDeviceType = DEVICE_TYPE_MIC;
            break;
        default:
            internalDeviceType = DEVICE_TYPE_NONE;
            break;
    }

    return internalDeviceType;
}

static void ReceviceDistributedInfo(struct ServiceStatus* serviceStatus, std::string & info,
    DeviceStatusListener * devListener)
{
    if (serviceStatus->status == SERVIE_STATUS_START) {
        AUDIO_DEBUG_LOG("distributed service online");
    } else if (serviceStatus->status == SERVIE_STATUS_CHANGE && !info.empty()) {
        DStatusInfo statusInfo;
        statusInfo.connectType = ConnectType::CONNECT_TYPE_DISTRIBUTED;
        PnpEventType pnpEventType = PNP_EVENT_UNKNOWN;
        if (sscanf_s(info.c_str(), "EVENT_TYPE=%d;NID=%[^;];PIN=%d;VID=%d;IID=%d", &pnpEventType,
            statusInfo.networkId, sizeof(statusInfo.networkId), &(statusInfo.hdiPin), &(statusInfo.mappingVolumeId),
            &(statusInfo.mappingInterruptId)) < D_EVENT_PARAMS) {
            AUDIO_ERR_LOG("[DeviceStatusListener]: Failed to scan info string");
            return;
        }

        statusInfo.isConnected = (pnpEventType == PNP_EVENT_DEVICE_ADD) ? true : false;
        devListener->deviceObserver_.OnDeviceStatusUpdated(statusInfo);
    } else if (serviceStatus->status == SERVIE_STATUS_STOP) {
        AUDIO_DEBUG_LOG("distributed service offline");
        DStatusInfo statusInfo;
        devListener->deviceObserver_.OnDeviceStatusUpdated(statusInfo, true);
    }
}

static void OnDeviceStatusChange(const std::string &info, DeviceStatusListener *devListener)
{
    CHECK_AND_RETURN_LOG(!info.empty(), "OnDeviceStatusChange invalid info");
    PnpDeviceType pnpDeviceType = PNP_DEVICE_UNKNOWN;
    PnpEventType pnpEventType = PNP_EVENT_UNKNOWN;
    if (sscanf_s(info.c_str(), "EVENT_TYPE=%d;DEVICE_TYPE=%d", &pnpEventType, &pnpDeviceType) < EVENT_PARAMS) {
        AUDIO_WARNING_LOG("[DeviceStatusListener]: Failed to scan info string %{public}s", info.c_str());
        return;
    }

    DeviceType internalDevice = GetInternalDeviceType(pnpDeviceType);
    AUDIO_DEBUG_LOG("internalDevice = %{public}d, pnpDeviceType = %{public}d", internalDevice, pnpDeviceType);
    CHECK_AND_RETURN_LOG(internalDevice != DEVICE_TYPE_NONE, "Unsupported device %{public}d", pnpDeviceType);

    bool isConnected = (pnpEventType == PNP_EVENT_DEVICE_ADD) ? true : false;
    AudioStreamInfo streamInfo = {};
    devListener->deviceObserver_.OnDeviceStatusUpdated(internalDevice, isConnected, "", "", streamInfo);
}

static void OnServiceStatusReceived(struct ServiceStatusListener *listener, struct ServiceStatus *serviceStatus)
{
    CHECK_AND_RETURN_LOG(serviceStatus != nullptr, "Invalid ServiceStatus");
    std::string info = serviceStatus->info;
    AUDIO_INFO_LOG("OnServiceStatusReceived: [service name:%{public}s] [status:%{public}d]",
        serviceStatus->serviceName, serviceStatus->status);

    DeviceStatusListener *devListener = reinterpret_cast<DeviceStatusListener *>(listener->priv);
    CHECK_AND_RETURN_LOG(devListener != nullptr, "Invalid deviceStatusListener");
    if (serviceStatus->serviceName == AUDIO_HDI_SERVICE_NAME) {
        if (serviceStatus->status == SERVIE_STATUS_START) {
            devListener->deviceObserver_.OnServiceConnected(AudioServiceIndex::HDI_SERVICE_INDEX);
        } else if (serviceStatus->status == SERVIE_STATUS_STOP) {
            devListener->deviceObserver_.OnServiceDisconnected(AudioServiceIndex::HDI_SERVICE_INDEX);
        } else if (serviceStatus->status == SERVIE_STATUS_CHANGE) {
            OnDeviceStatusChange(info, devListener);
        }
    } else if (serviceStatus->serviceName == AUDIO_BLUETOOTH_HDI_SERVICE_NAME) {
#ifdef BLUETOOTH_ENABLE
        if (serviceStatus->status == SERVIE_STATUS_START) {
            AUDIO_INFO_LOG("Bluetooth hdi service started");
        } else if (serviceStatus->status == SERVIE_STATUS_STOP) {
            AUDIO_INFO_LOG("Bluetooth hdi service stopped");
            if (Bluetooth::AudioA2dpManager::HasA2dpDeviceConnected()) {
                AUDIO_ERR_LOG("Auto exit audio policy service for bluetooth hdi service crashed!");
                _Exit(0);
            }
        }
#endif
    } else if (serviceStatus->serviceName == DAUDIO_HDI_SERVICE_NAME) {
        ReceviceDistributedInfo(serviceStatus, info, devListener);
    } else {
        AUDIO_DEBUG_LOG("unkown service name.");
    }
}

DeviceStatusListener::DeviceStatusListener(IDeviceStatusObserver &observer)
    : deviceObserver_(observer), hdiServiceManager_(nullptr), listener_(nullptr) {}

DeviceStatusListener::~DeviceStatusListener() = default;

int32_t DeviceStatusListener::RegisterDeviceStatusListener()
{
    AUDIO_INFO_LOG("Enter");
    hdiServiceManager_ = HDIServiceManagerGet();
    CHECK_AND_RETURN_RET_LOG(hdiServiceManager_ != nullptr, ERR_OPERATION_FAILED,
        "[DeviceStatusListener]: Get HDI service manager failed");

    listener_ = HdiServiceStatusListenerNewInstance();
    if (listener_ == nullptr) {
        return ERR_ILLEGAL_STATE;
    }
    listener_->callback = OnServiceStatusReceived;
    listener_->priv = (void *)this;
    int32_t status = hdiServiceManager_->RegisterServiceStatusListener(hdiServiceManager_, listener_,
        DeviceClass::DEVICE_CLASS_AUDIO);
    CHECK_AND_RETURN_RET_LOG(status == HDF_SUCCESS, ERR_OPERATION_FAILED,
        "[DeviceStatusListener]: Register service status listener failed");
    AUDIO_INFO_LOG("Register service status listener finished");

    audioPnpServer_ = &AudioPnpServer::GetAudioPnpServer();
    pnpDeviceCB_ = std::make_shared<AudioPnpStatusCallback>();
    pnpDeviceCB_->SetDeviceStatusListener(this);
    int32_t cbstatus = audioPnpServer_->RegisterPnpStatusListener(pnpDeviceCB_);
    CHECK_AND_RETURN_RET_LOG(cbstatus == SUCCESS, ERR_OPERATION_FAILED,
        "[DeviceStatusListener]: Register Pnp Status Listener failed");
    AUDIO_INFO_LOG("Done");
    return SUCCESS;
}

int32_t DeviceStatusListener::UnRegisterDeviceStatusListener()
{
    if ((hdiServiceManager_ == nullptr) || (listener_ == nullptr)) {
        return ERR_ILLEGAL_STATE;
    }
    int32_t status = hdiServiceManager_->UnregisterServiceStatusListener(hdiServiceManager_, listener_);
    CHECK_AND_RETURN_RET_LOG(status == HDF_SUCCESS, ERR_OPERATION_FAILED,
        "[DeviceStatusListener]: UnRegister service status listener failed");

    hdiServiceManager_ = nullptr;
    listener_ = nullptr;

    int32_t cbstatus = audioPnpServer_->UnRegisterPnpStatusListener();
    if (cbstatus != SUCCESS) {
        AUDIO_ERR_LOG("[DeviceStatusListener]: UnRegister Pnp Status Listener failed");
        return ERR_OPERATION_FAILED;
    }
    audioPnpServer_ = nullptr;
    pnpDeviceCB_ = nullptr;
    return SUCCESS;
}

void DeviceStatusListener::OnPnpDeviceStatusChanged(const std::string &info)
{
    CHECK_AND_RETURN_LOG(!info.empty(), "OnPnpDeviceStatusChange invalid info");
    PnpDeviceType pnpDeviceType = PNP_DEVICE_UNKNOWN;
    PnpEventType pnpEventType = PNP_EVENT_UNKNOWN;

    std::string name = "";
    std::string address = "";
 
    if (sscanf_s(info.c_str(), "EVENT_TYPE=%d;DEVICE_TYPE=%d;", &pnpEventType, &pnpDeviceType) < EVENT_NUM_TYPE) {
        AUDIO_ERR_LOG("Failed to scan info string %{public}s", info.c_str());
        return;
    }
 
    auto nameBegin = info.find("EVENT_NAME=");
    auto nameEnd = info.find_first_of(";", nameBegin);
    name = info.substr(nameBegin + std::strlen("EVENT_NAME="),
        nameEnd - nameBegin - std::strlen("EVENT_NAME="));

    auto addressBegin = info.find("DEVICE_ADDRESS=");
    auto addressEnd = info.find_first_of(";", addressBegin);
    string portId = info.substr(addressBegin + std::strlen("DEVICE_ADDRESS="),
        addressEnd - addressBegin - std::strlen("DEVICE_ADDRESS="));

    DeviceType internalDevice = GetInternalDeviceType(pnpDeviceType);
    CHECK_AND_RETURN_LOG(internalDevice != DEVICE_TYPE_NONE, "Unsupported device %{public}d", pnpDeviceType);
    bool isConnected = (pnpEventType == PNP_EVENT_DEVICE_ADD) ? true : false;

    if (internalDevice == DEVICE_TYPE_DP) {
        address = DP_ADDRESS + portId;
    }
    AUDIO_INFO_LOG("[device type :%{public}d], [connection state: %{public}d] "
        "[name: %{public}s]", internalDevice, isConnected, name.c_str());
    deviceObserver_.OnPnpDeviceStatusUpdated(internalDevice, isConnected, name, address);
}

void DeviceStatusListener::OnMicrophoneBlocked(const std::string &info)
{
    CHECK_AND_RETURN_LOG(!info.empty(), "OnMicrophoneBlocked invalid info");

    PnpDeviceType pnpDeviceType = PNP_DEVICE_UNKNOWN;
    PnpEventType pnpEventType = PNP_EVENT_UNKNOWN;
 
    if (sscanf_s(info.c_str(), "EVENT_TYPE=%d;DEVICE_TYPE=%d;", &pnpEventType, &pnpDeviceType) < EVENT_NUM_TYPE) {
        AUDIO_ERR_LOG("Failed to scan info string %{public}s", info.c_str());
        return;
    }
 
    DeviceType micBlockedDeviceType = GetInternalDeviceType(pnpDeviceType);
    CHECK_AND_RETURN_LOG(micBlockedDeviceType != DEVICE_TYPE_NONE, "Unsupported device %{public}d", pnpDeviceType);

    DeviceBlockStatus status = DEVICE_UNBLOCKED;
    if (pnpEventType == PNP_EVENT_MIC_BLOCKED) {
        status = DEVICE_BLOCKED;
    }
    AUDIO_INFO_LOG("[device type :%{public}d], [status :%{public}d]", micBlockedDeviceType, status);
    deviceObserver_.OnMicrophoneBlockedUpdate(micBlockedDeviceType, status);
}

AudioPnpStatusCallback::AudioPnpStatusCallback()
{
    AUDIO_INFO_LOG("ctor");
}

AudioPnpStatusCallback::~AudioPnpStatusCallback() {}

void AudioPnpStatusCallback::SetDeviceStatusListener(DeviceStatusListener *listener)
{
    listener_ = listener;
}

void AudioPnpStatusCallback::OnPnpDeviceStatusChanged(const std::string &info)
{
    listener_->OnPnpDeviceStatusChanged(info);
}

void AudioPnpStatusCallback::OnMicrophoneBlocked(const std::string &info)
{
    listener_->OnMicrophoneBlocked(info);
}
} // namespace AudioStandard
} // namespace OHOS
