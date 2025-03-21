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
#define LOG_TAG "AudioCoreServiceEntry"
#endif

#include "audio_core_service.h"
#include "audio_server_proxy.h"
#include "audio_usb_manager.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static constexpr int64_t WAIT_LOAD_DEFAULT_DEVICE_TIME_MS = 200; // 200ms
static constexpr int32_t RETRY_TIMES = 25;
}
AudioCoreService::EventEntry::EventEntry(std::shared_ptr<AudioCoreService> coreService) : coreService_(coreService)
{
    AUDIO_INFO_LOG("Ctor");
}

void AudioCoreService::EventEntry::RegistCoreService()
{
    AUDIO_INFO_LOG("In");
    coreService_->SetAudioServerProxy();
    sptr<CoreServiceProviderWrapper> wrapper = new(std::nothrow) CoreServiceProviderWrapper(this);
    CHECK_AND_RETURN_LOG(wrapper != nullptr, "Get null CoreServiceProviderWrapper");
    sptr<IRemoteObject> object = wrapper->AsObject();
    CHECK_AND_RETURN_LOG(object != nullptr, "AsObject is nullptr");

    int32_t ret = AudioServerProxy::GetInstance().RegistCoreServiceProviderProxy(object);
    AUDIO_INFO_LOG("Result:%{public}d", ret);
}

int32_t AudioCoreService::EventEntry::CreateRendererClient(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &flag, uint32_t &sessionId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->CreateRendererClient(streamDesc, flag, sessionId);
    return SUCCESS;
}

int32_t AudioCoreService::EventEntry::CreateCapturerClient(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &flag, uint32_t &sessionId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->CreateCapturerClient(streamDesc, flag, sessionId);
    return SUCCESS;
}

int32_t AudioCoreService::EventEntry::UpdateSessionOperation(uint32_t sessionId, SessionOperation operation)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    switch (operation) {
        case SESSION_OPERATION_START:
            return coreService_->StartClient(sessionId);
        case SESSION_OPERATION_PAUSE:
            return coreService_->PauseClient(sessionId);
        case SESSION_OPERATION_STOP:
            return coreService_->StopClient(sessionId);
        case SESSION_OPERATION_RELEASE:
            return coreService_->ReleaseClient(sessionId);
        default:
            return SUCCESS;
    }
}

std::string AudioCoreService::EventEntry::GetAdapterNameBySessionId(uint32_t sessionId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetAdapterNameBySessionId(sessionId);
}

int32_t AudioCoreService::EventEntry::GetProcessDeviceInfoBySessionId(
    uint32_t sessionId, AudioDeviceDescriptor &deviceInfo)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetProcessDeviceInfoBySessionId(sessionId, deviceInfo);
}

uint32_t AudioCoreService::EventEntry::GenerateSessionId()
{
    return coreService_->GenerateSessionId();
}

int32_t AudioCoreService::EventEntry::SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionID,
    const StreamUsage streamUsage, bool isRunning)
{
    int32_t ret = coreService_->SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning);
    return ret;
}

// device status listener
void AudioCoreService::EventEntry::OnDeviceStatusUpdated(
    DeviceType devType, bool isConnected, const std::string& macAddress,
    const std::string& deviceName, const AudioStreamInfo& streamInfo, DeviceRole role, bool hasPair)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo, role, hasPair);
}

void AudioCoreService::EventEntry::OnDeviceStatusUpdated(AudioDeviceDescriptor &updatedDesc, bool isConnected)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnDeviceStatusUpdated(updatedDesc, isConnected);
}

void AudioCoreService::EventEntry::OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnDeviceStatusUpdated(statusInfo, isStop);
}

void AudioCoreService::EventEntry::OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status)
{
    CHECK_AND_RETURN_LOG(devType != DEVICE_TYPE_NONE, "devType is none type");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnMicrophoneBlockedUpdate(devType, status);
}

void AudioCoreService::EventEntry::OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnPnpDeviceStatusUpdated(desc, isConnected);
}

void AudioCoreService::EventEntry::OnDeviceConfigurationChanged(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    coreService_->OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

void AudioCoreService::EventEntry::OnServiceConnected(AudioServiceIndex serviceIndex)
{
    AUDIO_INFO_LOG("[module_load]::OnServiceConnected for [%{public}d]", serviceIndex);
    CHECK_AND_RETURN_LOG(serviceIndex >= HDI_SERVICE_INDEX && serviceIndex <= AUDIO_SERVICE_INDEX, "invalid index");

    // If audio service or hdi service is not ready, donot load default modules
    std::lock_guard<std::mutex> lock(coreService_->serviceFlagMutex_);
    coreService_->serviceFlag_.set(serviceIndex, true);
    if (coreService_->serviceFlag_.count() != MIN_SERVICE_COUNT) {
        AUDIO_INFO_LOG("[module_load]::hdi service or audio service not up. Cannot load default module now");
        return;
    }

    std::unique_lock<std::shared_mutex> serviceLock(eventMutex_);
    int32_t ret = coreService_->OnServiceConnected(serviceIndex);
    serviceLock.unlock();
    if (ret == SUCCESS) {
#ifdef USB_ENABLE
        AudioUsbManager::GetInstance().Init();
#endif
        coreService_->audioEffectService_.SetMasterSinkAvailable();
    }
    // RegisterBluetoothListener() will be called when bluetooth_host is online
    // load hdi-effect-model
    AudioServerProxy::GetInstance().LoadHdiEffectModelProxy();
    AudioServerProxy::GetInstance().NotifyAudioPolicyReady();
}

void AudioCoreService::EventEntry::OnServiceDisconnected(AudioServiceIndex serviceIndex)
{
    AUDIO_WARNING_LOG("Service index [%{public}d]", serviceIndex);
}

void AudioCoreService::EventEntry::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnForcedDeviceSelected(devType, macAddress);
}

int32_t AudioCoreService::EventEntry::SetAudioScene(AudioScene audioScene)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->SetAudioScene(audioScene);
    return SUCCESS;
}

bool AudioCoreService::EventEntry::IsArmUsbDevice(const AudioDeviceDescriptor &deviceDesc)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->IsArmUsbDevice(deviceDesc);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::EventEntry::GetDevices(DeviceFlag deviceFlag)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetDevices(deviceFlag);
}

int32_t AudioCoreService::EventEntry::SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t pid)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->SetDeviceActive(deviceType, active, pid);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::EventEntry::GetPreferredOutputDeviceDescriptors(
    AudioRendererInfo &rendererInfo, std::string networkId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetPreferredOutputDeviceDescInner(rendererInfo, networkId);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::EventEntry::GetPreferredInputDeviceDescriptors(
    AudioCapturerInfo &captureInfo, std::string networkId)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetPreferredInputDeviceDescInner(captureInfo, networkId);
}

std::shared_ptr<AudioDeviceDescriptor> AudioCoreService::EventEntry::GetActiveBluetoothDevice()
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetActiveBluetoothDevice();
}

void AudioCoreService::EventEntry::OnDeviceInfoUpdated(
    AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnDeviceInfoUpdated(desc, command);
}

int32_t AudioCoreService::EventEntry::SetCallDeviceActive(
    InternalDeviceType deviceType, bool active, std::string address)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->SetCallDeviceActive(deviceType, active, address);
    return SUCCESS;
}

std::vector<shared_ptr<AudioDeviceDescriptor>> AudioCoreService::EventEntry::GetAvailableDevices(AudioDeviceUsage usage)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetAvailableDevices(usage);
}

int32_t AudioCoreService::EventEntry::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    const sptr<IRemoteObject> &object, const int32_t apiVersion)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->RegisterTracker(mode, streamChangeInfo, object, apiVersion);
}

int32_t AudioCoreService::EventEntry::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->UpdateTracker(mode, streamChangeInfo);
}

void AudioCoreService::EventEntry::RegisteredTrackerClientDied(pid_t uid)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->RegisteredTrackerClientDied(uid);
}

bool AudioCoreService::EventEntry::ConnectServiceAdapter()
{
    bool ret = coreService_->ConnectServiceAdapter();
    CHECK_AND_RETURN_RET_LOG(ret, false, "Error in connecting to audio service adapter");

    OnServiceConnected(AudioServiceIndex::AUDIO_SERVICE_INDEX);

    return true;
}

vector<sptr<MicrophoneDescriptor>> AudioCoreService::EventEntry::GetAvailableMicrophones()
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetAvailableMicrophones();
}

vector<sptr<MicrophoneDescriptor>> AudioCoreService::EventEntry::GetAudioCapturerMicrophoneDescriptors(
    int32_t sessionId)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetAudioCapturerMicrophoneDescriptors(sessionId);
}

void AudioCoreService::EventEntry::OnReceiveBluetoothEvent(const std::string macAddress, const std::string deviceName)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnReceiveBluetoothEvent(macAddress, deviceName);
}

int32_t AudioCoreService::EventEntry::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    Trace trace("AudioCoreService::SelectOutputDevice");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->SelectOutputDevice(audioRendererFilter, selectedDesc);
}

int32_t AudioCoreService::EventEntry::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    Trace trace("AudioCoreService::SelectInputDevice");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->SelectInputDevice(audioCapturerFilter, selectedDesc);
}

int32_t AudioCoreService::EventEntry::GetCurrentRendererChangeInfos(vector<shared_ptr<AudioRendererChangeInfo>>
    &audioRendererChangeInfos, bool hasBTPermission, bool hasSystemPermission)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetCurrentRendererChangeInfos(audioRendererChangeInfos, hasBTPermission,
        hasSystemPermission);
}

int32_t AudioCoreService::EventEntry::GetCurrentCapturerChangeInfos(vector<shared_ptr<AudioCapturerChangeInfo>>
    &audioCapturerChangeInfos, bool hasBTPermission, bool hasSystemPermission)
{
    return coreService_->GetCurrentCapturerChangeInfos(audioCapturerChangeInfos, hasBTPermission,
        hasSystemPermission);
}

void AudioCoreService::EventEntry::NotifyRemoteRenderState(
    std::string networkId, std::string condition, std::string value)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->NotifyRemoteRenderState(networkId, condition, value);
}

int32_t AudioCoreService::EventEntry::OnCapturerSessionAdded(uint64_t sessionID, SessionInfo sessionInfo,
    AudioStreamInfo streamInfo)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->OnCapturerSessionAdded(sessionID, sessionInfo, streamInfo);
}

void AudioCoreService::EventEntry::OnCapturerSessionRemoved(uint64_t sessionID)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnCapturerSessionRemoved(sessionID);
}

void AudioCoreService::EventEntry::SetDisplayName(const std::string &deviceName, bool isLocalDevice)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->SetDisplayName(deviceName, isLocalDevice);
}

int32_t AudioCoreService::EventEntry::TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->TriggerFetchDevice(reason);
}

std::vector<sptr<VolumeGroupInfo>> AudioCoreService::EventEntry::GetVolumeGroupInfos()
{
    std::vector<sptr<VolumeGroupInfo>> infos = {};
    for (int32_t i = 0; i < RETRY_TIMES; i++) {
        std::shared_lock<std::shared_mutex> lock(eventMutex_);
        if (coreService_->GetVolumeGroupInfos(infos)) {
            return infos;
        } else {
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(WAIT_LOAD_DEFAULT_DEVICE_TIME_MS));
        }
    }
    AUDIO_ERR_LOG("timeout");
    return infos;
}

void AudioCoreService::EventEntry::FetchOutputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo,
    const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("Not support");
}

void AudioCoreService::EventEntry::FetchInputDeviceForTrack(AudioStreamChangeInfo &streamChangeInfo)
{
    AUDIO_INFO_LOG("Not support");
}

int32_t AudioCoreService::EventEntry::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    Trace trace("AudioCoreService::EventEntry::ExcludeOutputDevices");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioCoreService::EventEntry::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::EventEntry::GetExcludedDevices(
    AudioDeviceUsage audioDevUsage)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetExcludedDevices(audioDevUsage);
}

int32_t AudioCoreService::EventEntry::GetPreferredOutputStreamType(AudioRendererInfo &rendererInfo,
    const std::string &bundleName)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetPreferredOutputStreamType(rendererInfo, bundleName);
}

int32_t AudioCoreService::EventEntry::GetPreferredInputStreamType(AudioCapturerInfo &capturerInfo)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetPreferredInputStreamType(capturerInfo);
}
}
}
