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
#define LOG_TAG "DeviceStatusListener"
#endif

#include "device_status_listener.h"
#include <securec.h>
#include <cstring>
#include "hdf_device_class.h"
#include "v4_0/audio_types.h"

#ifdef BLUETOOTH_ENABLE

#include "audio_bluetooth_manager.h"
#include "bluetooth_def.h"

#endif

#include "audio_errors.h"
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
        case PnpDeviceType::PNP_DEVICE_HDMI_DEVICE:
            internalDeviceType = DEVICE_TYPE_HDMI;
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
    if (internalDevice == DEVICE_TYPE_NONE) {
        AUDIO_DEBUG_LOG("Unsupported device %{public}d", pnpDeviceType);
        return;
    }
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
    CHECK_AND_RETURN_RET_LOG(listener_ != nullptr, ERR_ILLEGAL_STATE,
        "[DeviceStatusListener]: status listener failed");
    listener_->callback = OnServiceStatusReceived;
    listener_->priv = (void *)this;
    int32_t status = hdiServiceManager_->RegisterServiceStatusListener(hdiServiceManager_, listener_,
        DeviceClass::DEVICE_CLASS_AUDIO);
    CHECK_AND_RETURN_RET_LOG(status == HDF_SUCCESS, ERR_OPERATION_FAILED,
        "[DeviceStatusListener]: Register service status listener failed");
    AUDIO_INFO_LOG("Register service status listener finished");

#ifdef AUDIO_WIRED_DETECT
    audioPnpServer_ = &AudioPnpServer::GetAudioPnpServer();
    pnpDeviceCB_ = std::make_shared<AudioPnpStatusCallback>();
    pnpDeviceCB_->SetDeviceStatusListener(this);
    int32_t cbstatus = audioPnpServer_->RegisterPnpStatusListener(pnpDeviceCB_);
    CHECK_AND_RETURN_RET_LOG(cbstatus == SUCCESS, ERR_OPERATION_FAILED,
        "[DeviceStatusListener]: Register Pnp Status Listener failed");
#endif
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

#ifdef AUDIO_WIRED_DETECT
    int32_t cbstatus = audioPnpServer_->UnRegisterPnpStatusListener();
    if (cbstatus != SUCCESS) {
        AUDIO_ERR_LOG("[DeviceStatusListener]: UnRegister Pnp Status Listener failed");
        return ERR_OPERATION_FAILED;
    }
    audioPnpServer_ = nullptr;
    pnpDeviceCB_ = nullptr;
#endif
    return SUCCESS;
}

#ifdef AUDIO_WIRED_DETECT
void DeviceStatusListener::OnPnpDeviceStatusChanged(const std::string &info)
{
    CHECK_AND_RETURN_LOG(!info.empty(), "OnPnpDeviceStatusChange invalid info");

    if (audioDeviceAnahsCb_ != nullptr) {
        std::string anahsName = "";
        auto anahsBegin = info.find("ANAHS_NAME=");
        auto anahsEnd = info.find_first_of(";", anahsBegin);
        anahsName = info.substr(anahsBegin + std::strlen("ANAHS_NAME="),
            anahsEnd - anahsBegin - std::strlen("ANAHS_NAME="));
        if (strncmp(anahsName.c_str(), UEVENT_INSERT, strlen(UEVENT_INSERT)) == 0 ||
            strncmp(anahsName.c_str(), UEVENT_REMOVE, strlen(UEVENT_REMOVE)) == 0) {
            AUDIO_INFO_LOG("parse anahsName = %{public}s", anahsName.c_str());
            audioDeviceAnahsCb_->OnExtPnpDeviceStatusChanged(anahsName, anahsShowType_);
            return;
        }
    }

    PnpDeviceType pnpDeviceType = PNP_DEVICE_UNKNOWN;
    PnpEventType pnpEventType = PNP_EVENT_UNKNOWN;

    AudioDeviceDescriptor desc = {};

    if (sscanf_s(info.c_str(), "EVENT_TYPE=%d;DEVICE_TYPE=%d;", &pnpEventType, &pnpDeviceType) < EVENT_NUM_TYPE) {
        AUDIO_ERR_LOG("Failed to scan info string %{public}s", info.c_str());
        return;
    }

    auto nameBegin = info.find("EVENT_NAME=");
    auto nameEnd = info.find_first_of(";", nameBegin);
    desc.deviceName_ = info.substr(nameBegin + std::strlen("EVENT_NAME="),
        nameEnd - nameBegin - std::strlen("EVENT_NAME="));

    auto addressBegin = info.find("DEVICE_ADDRESS=");
    auto addressEnd = info.find_first_of(";", addressBegin);
    string portId = info.substr(addressBegin + std::strlen("DEVICE_ADDRESS="),
        addressEnd - addressBegin - std::strlen("DEVICE_ADDRESS="));

    desc.deviceType_ = GetInternalDeviceType(pnpDeviceType);
    if (desc.deviceType_ == DEVICE_TYPE_NONE) {
        AUDIO_DEBUG_LOG("Unsupported device %{public}d", pnpDeviceType);
        return;
    }
    bool isConnected = (pnpEventType == PNP_EVENT_DEVICE_ADD) ? true : false;

    if (desc.deviceType_ == DEVICE_TYPE_DP) {
        desc.macAddress_ = DP_ADDRESS + portId;
    }
    AUDIO_INFO_LOG("[device type: %{public}d], [connection state: %{public}d], [name: %{public}s]",
        desc.deviceType_, isConnected, desc.deviceName_.c_str());
    deviceObserver_.OnPnpDeviceStatusUpdated(desc, isConnected);
}
#endif

int32_t DeviceStatusListener::SetAudioDeviceAnahsCallback(const sptr<IRemoteObject> &object)
{
    sptr<IStandardAudioAnahsManagerListener> listener = iface_cast<IStandardAudioAnahsManagerListener>(object);
    if (listener != nullptr) {
        audioDeviceAnahsCb_ = listener;
        return SUCCESS;
    } else {
        return ERROR;
    }
}

int32_t DeviceStatusListener::UnsetAudioDeviceAnahsCallback()
{
    audioDeviceAnahsCb_ = nullptr;
    return SUCCESS;
}

void DeviceStatusListener::UpdateAnahsPlatformType(std::string anahsShowType)
{
    anahsShowType_ = anahsShowType;
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
    if (micBlockedDeviceType == DEVICE_TYPE_NONE) {
        AUDIO_DEBUG_LOG("Unsupported device %{public}d", pnpDeviceType);
        return;
    }

    DeviceBlockStatus status = DEVICE_UNBLOCKED;
    if (pnpEventType == PNP_EVENT_MIC_BLOCKED) {
        status = DEVICE_BLOCKED;
    }
    AUDIO_INFO_LOG("[device type :%{public}d], [status :%{public}d]", micBlockedDeviceType, status);
    deviceObserver_.OnMicrophoneBlockedUpdate(micBlockedDeviceType, status);
}

#ifdef AUDIO_WIRED_DETECT
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
#endif
} // namespace AudioStandard
} // namespace OHOS
