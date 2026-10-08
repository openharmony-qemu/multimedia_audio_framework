/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
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

#include "audio_errors.h"
#include "audio_utils.h"
#include "audio_server_proxy.h"
#include "audio_select_interface_service.h"
#ifdef USB_ENABLE
#include "audio_usb_manager.h"
#endif

#include <utility>

namespace OHOS {
namespace AudioStandard {
namespace {
static constexpr int64_t WAIT_LOAD_DEFAULT_DEVICE_TIME_MS = 200; // 200ms
static constexpr int32_t RETRY_TIMES = 25;
static constexpr uint32_t TIMEOUT_CORE_ENTRY_S = 20;
}

static const char *SessionOperationToString(SessionOperation operation)
{
    switch (operation) {
        case SESSION_OPERATION_START:
            return "OPERATION_START";
        case SESSION_OPERATION_PAUSE:
            return "OPERATION_PAUSE";
        case SESSION_OPERATION_STOP:
            return "OPERATION_STOP";
        case SESSION_OPERATION_RELEASE:
            return "OPERATION_RELEASE";
        case SESSION_OPERATION_STANDBY:
            return "OPERATION_STANDBY";
        case SESSION_OPERATION_CREATE:
            return "OPERATION_CREATE";
        default:
            return "OPERATION_UNKNOWN";
    }
}

static const char *SessionOperationMsgToString(SessionOperationMsg opMsg)
{
    switch (opMsg) {
        case SESSION_OP_MSG_DEFAULT:
            return "MSG_DEFAULT";
        case SESSION_OP_MSG_REMOVE_PIPE:
            return "MSG_REMOVE_REC_PIPE";
        default:
            return "MSG_UNKNOWN";
    }
}

static int32_t WaitResultToAudioError(MultiConditionWaiter::WaitResult result)
{
    switch (result) {
        case MultiConditionWaiter::WaitResult::SUCCESS:
            return SUCCESS;
        case MultiConditionWaiter::WaitResult::WAIT_LIMIT_EXCEEDED:
            return ERR_RETRY_IN_CLIENT;
        case MultiConditionWaiter::WaitResult::INVALID_STATE:
            return ERR_OPERATION_FAILED;
        case MultiConditionWaiter::WaitResult::TIMEOUT:
            return ERR_OPERATION_FAILED;
        default:
            return ERR_OPERATION_FAILED;
    }
}

AudioCoreService::EventEntry::EventEntry(std::shared_ptr<AudioCoreService> coreService,
    const MultiConditionNotifier &readyConditionNotifier) : coreService_(coreService),
    readyConditionWaiter_(readyConditionNotifier.CreateWaiter()),
    audioPolicyDump_(AudioPolicyDump::GetInstance())
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

int32_t AudioCoreService::EventEntry::WaitForReadyConditions()
{
    auto result = readyConditionWaiter_.WaitFor(
        READY_CONDITION_MASK, std::chrono::milliseconds(READY_CONDITION_WAIT_TIMEOUT_MS),
        READY_CONDITION_WAIT_LIMIT);
    return WaitResultToAudioError(result);
}

void AudioCoreService::EventEntry::NotifyServiceReady()
{
    coreService_->NotifyServiceReady();
}

int32_t AudioCoreService::EventEntry::CreateRendererClient(std::shared_ptr<AudioStreamDescriptor> &streamDesc,
    StreamFlagInfo &flagInfo, uint32_t &sessionId, std::string &networkId)
{
    int32_t waitRet = WaitForReadyConditions();
    RecordBootStateTime(static_cast<int32_t>(BootAnimationState::WAIT_FOR_SERVICE_READY), streamDesc->GetRealUid(),
        ClockTime::GetCurMilli());
    CHECK_AND_RETURN_RET(waitRet != ERR_RETRY_IN_CLIENT, ERR_RETRY_IN_CLIENT);
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->CreateRendererClient(streamDesc, flagInfo, sessionId, networkId);
    return SUCCESS;
}

int32_t AudioCoreService::EventEntry::CreateCapturerClient(
    std::shared_ptr<AudioStreamDescriptor> streamDesc, uint32_t &flag, uint32_t &sessionId)
{
    int32_t waitRet = WaitForReadyConditions();
    CHECK_AND_RETURN_RET(waitRet != ERR_RETRY_IN_CLIENT, ERR_RETRY_IN_CLIENT);
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->CreateCapturerClient(streamDesc, flag, sessionId);
    return SUCCESS;
}

int32_t AudioCoreService::EventEntry::UpdateSessionOperation(uint32_t sessionId, SessionOperation operation,
    SessionOperationMsg opMsg, bool needRemoveFromMap)
{
    // This function is called frequently, add xcollie here to avoid freezing for a very long time
    AudioXCollie audioXCollie("EventEntry::UpdateSessionOperation", TIMEOUT_CORE_ENTRY_S,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);

    bool isInterruptActive = false;
    if (operation == SESSION_OPERATION_START) {
        isInterruptActive = coreService_->IsStreamActive(sessionId);
    }
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    HILOG_COMM_INFO("[UpdateSessionOperation]withlock sessionId %{public}u, operation %{public}s, msg %{public}s",
        sessionId, SessionOperationToString(operation), SessionOperationMsgToString(opMsg));
    switch (operation) {
        case SESSION_OPERATION_START:
            return coreService_->StartClient(sessionId, opMsg, isInterruptActive);
        case SESSION_OPERATION_PAUSE:
            return coreService_->PauseClient(sessionId);
        case SESSION_OPERATION_STANDBY:
            return coreService_->StandbyClient(sessionId);
        case SESSION_OPERATION_STOP:
            return coreService_->StopClient(sessionId);
        case SESSION_OPERATION_RELEASE:
            return coreService_->ReleaseClient(sessionId, opMsg, needRemoveFromMap);
        case SESSION_OPERATION_CREATE:
            coreService_->MarkServerStreamCreated(sessionId);
            return SUCCESS;
        default:
            return SUCCESS;
    }
}

int32_t AudioCoreService::EventEntry::ReloadCaptureSession(uint32_t sessionId, SessionOperation operation)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->ReloadCaptureSession(sessionId, operation);
}

int32_t AudioCoreService::EventEntry::RecordBootStateTime(int32_t state, int32_t uid, int64_t curTime)
{
    CHECK_AND_RETURN_RET(uid == BOOT_ANIMATION_UID, SUCCESS);
    audioPolicyDump_.RecordBootStateTime(static_cast<BootAnimationState>(state), uid, curTime);
    return SUCCESS;
}

std::string AudioCoreService::EventEntry::GetAdapterNameBySessionId(uint32_t sessionId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetAdapterNameBySessionId(sessionId);
}

std::string AudioCoreService::EventEntry::GetModuleNameBySessionId(uint32_t sessionId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetModuleNameBySessionId(sessionId);
}

uint32_t AudioCoreService::EventEntry::QueryPipeIdBySessionId(uint32_t sessionId)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERR_UNKNOWN, "coreService_ is nullptr");
    return coreService_->QueryPipeIdBySessionId(sessionId);
}

int32_t AudioCoreService::EventEntry::GetProcessDeviceInfoBySessionId(uint32_t sessionId,
    AudioDeviceDescriptor &deviceInfo, AudioStreamInfo &streamInfo, bool &isUltraFast, bool isReloadProcess)
{
    if (isReloadProcess) {
        // Get process from reload does not require lock
        return coreService_->GetProcessDeviceInfoBySessionId(sessionId, deviceInfo, streamInfo, isUltraFast);
    }
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetProcessDeviceInfoBySessionId(sessionId, deviceInfo, streamInfo, isUltraFast);
}

int32_t AudioCoreService::EventEntry::GetAudioDebugInfoBySessionId(uint32_t sessionId,
    AudioDebugInfo &audioDebugInfo)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetAudioDebugInfoBySessionId(sessionId, audioDebugInfo);
}

uint32_t AudioCoreService::EventEntry::GenerateSessionId()
{
    return coreService_->GenerateSessionId();
}

int32_t AudioCoreService::EventEntry::SetDefaultOutputDevice(const DeviceType deviceType, const uint32_t sessionID,
    const StreamUsage streamUsage, bool isRunning, bool skipForce)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("withlock device %{public}d, sessionId %{public}u, streamUsage %{public}d, running %{public}d",
        deviceType, sessionID, streamUsage, isRunning);
    auto &selector = AudioSelectInterfaceService::GetInstance();
    int32_t ret = selector.SetDefaultOutputDevice(deviceType, sessionID, streamUsage, isRunning, skipForce);
    return ret;
}

int32_t AudioCoreService::EventEntry::SetOffloadAllowedForUid(uint32_t uid, bool allowed)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERR_UNKNOWN, "coreService_ is nullptr");
    return coreService_->SetOffloadAllowedForUid(uid, allowed);
}

int32_t AudioCoreService::EventEntry::SetSplitModeReady()
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERR_UNKNOWN, "coreService_ is nullptr");
    return coreService_->SetSplitModeReady();
}

int32_t AudioCoreService::EventEntry::LoadSplitModule(const std::string &splitArgs, const std::string &networkId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->LoadSplitModule(splitArgs, networkId);
}

int32_t AudioCoreService::EventEntry::SetAudioRouteSelectorCallback(
    const AudioPipeSelector::AudioRouteSelectorCallback &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM, "callback is nullptr");
    return HandleSetAudioRouteSelectorCallback(callback);
}

int32_t AudioCoreService::EventEntry::UnsetAudioRouteSelectorCallback()
{
    return HandleUnsetAudioRouteSelectorCallback();
}

int32_t AudioCoreService::EventEntry::HandleSetAudioRouteSelectorCallback(
    const AudioPipeSelector::AudioRouteSelectorCallback &callback)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERR_NULL_POINTER, "coreService_ is nullptr");
    return coreService_->SetAudioRouteSelectorCallback(callback);
}

int32_t AudioCoreService::EventEntry::HandleUnsetAudioRouteSelectorCallback()
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERR_NULL_POINTER, "coreService_ is nullptr");
    return coreService_->UnsetAudioRouteSelectorCallback();
}

// device status listener
void AudioCoreService::EventEntry::OnDeviceStatusUpdated(
    DeviceType devType, bool isConnected, const std::string& macAddress,
    const std::string& deviceName, const AudioStreamInfo& streamInfo, DeviceRole role, bool hasPair)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("withlock legacy devicechange for common");
    coreService_->OnDeviceStatusUpdated(devType, isConnected, macAddress, deviceName, streamInfo, role, hasPair);
}

void AudioCoreService::EventEntry::OnDeviceStatusUpdated(AudioDeviceDescriptor &updatedDesc, bool isConnected)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("withlock legacy devicechange for bt");
    coreService_->OnDeviceStatusUpdated(updatedDesc, isConnected);
}

void AudioCoreService::EventEntry::OnDeviceStatusUpdated(DStatusInfo statusInfo, bool isStop)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("withlock legacy devicechange for remote");
    coreService_->OnDeviceStatusUpdated(statusInfo, isStop);
}

void AudioCoreService::EventEntry::OnMicrophoneBlockedUpdate(DeviceType devType, DeviceBlockStatus status)
{
    CHECK_AND_RETURN_LOG(devType != DEVICE_TYPE_NONE, "devType is none type");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("withlock device %{public}d, blockstatus %{public}d", devType, status);
    coreService_->OnMicrophoneBlockedUpdate(devType, status);
}

void AudioCoreService::EventEntry::OnPnpDeviceStatusUpdated(AudioDeviceDescriptor &desc, bool isConnected)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("withlock pnp devicechange");
    coreService_->OnPnpDeviceStatusUpdated(desc, isConnected);
}

void AudioCoreService::EventEntry::OnDeviceConfigurationChanged(DeviceType deviceType, const std::string &macAddress,
    const std::string &deviceName, const AudioStreamInfo &streamInfo)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("withlock configure change");
    coreService_->OnDeviceConfigurationChanged(deviceType, macAddress, deviceName, streamInfo);
}

void AudioCoreService::EventEntry::OnServiceConnected(AudioServiceIndex serviceIndex)
{
    CHECK_AND_RETURN_LOG(serviceIndex >= HDI_SERVICE_INDEX && serviceIndex <= AUDIO_SERVICE_INDEX, "invalid index");

    AUDIO_INFO_LOG("load serviceIndex [%{public}d]", serviceIndex);

    // If audio service or hdi service is not ready, donot load default modules
    std::lock_guard<std::mutex> lock(coreService_->serviceFlagMutex_);
    coreService_->serviceFlag_.set(serviceIndex, true);
    if (coreService_->serviceFlag_.count() != MIN_SERVICE_COUNT) {
        AUDIO_INFO_LOG("audiohost or audioserver not both boot, waiting for next time");
        return;
    }

    std::unique_lock<std::shared_mutex> serviceLock(eventMutex_);
    int32_t ret = coreService_->OnServiceConnected(serviceIndex);
    serviceLock.unlock();
    if (ret == SUCCESS) {
        coreService_->audioEffectService_.SetMasterSinkAvailable();
    }
    // RegisterBluetoothListener() will be called when bluetooth_host is online
    // load hdi-effect-model
    AudioServerProxy::GetInstance().LoadHdiEffectModelProxy();
    AudioServerProxy::GetInstance().NotifyAudioPolicyReady();
    NotifyServiceReady();
#ifdef AUDIO_WIRED_DETECT
    AudioPnpServer::GetAudioPnpServer().init();
#endif
#ifdef USB_ENABLE
    AudioUsbManager::GetInstance().Init(coreService_->eventEntry_, FLAG_AUD_SVC_UP);
    AudioUsbManager::GetInstance().SubscribeEvent(FLAG_AUD_SVC_UP);
#endif
}

void AudioCoreService::EventEntry::OnServiceDisconnected(AudioServiceIndex serviceIndex)
{
    AUDIO_WARNING_LOG("unload serviceIndex [%{public}d], should not be here", serviceIndex);
}

void AudioCoreService::EventEntry::OnForcedDeviceSelected(DeviceType devType, const std::string &macAddress,
    sptr<AudioRendererFilter> filter, const std::string &caller)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("withlock");
    coreService_->OnForcedDeviceSelected(devType, macAddress, filter, caller);
}

void AudioCoreService::EventEntry::OnPrivacyDeviceSelected(DeviceType devType, const std::string &macAddress,
    const std::string &caller)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnPrivacyDeviceSelected(devType, macAddress, caller);
}

void AudioCoreService::EventEntry::OnConnectFailed(AudioDeviceDescriptor &desc)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnConnectFailed(desc);
}

void AudioCoreService::EventEntry::OnVehiclePriorityChanged(bool enable)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnVehiclePriorityChanged(enable);
}

int32_t AudioCoreService::EventEntry::SetAudioScene(AudioScene audioScene, const int32_t uid, const int32_t pid)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->SetAudioScene(audioScene, uid, pid);
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

int32_t AudioCoreService::EventEntry::SetDeviceActive(InternalDeviceType deviceType, bool active, const int32_t uid)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("withlock");
    return coreService_->SetDeviceActive(deviceType, active, uid);
}

int32_t AudioCoreService::EventEntry::SetInputDevice(const DeviceType deviceType, const uint32_t sessionID, int32_t uid)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("withlock");
    return coreService_->SetInputDevice(deviceType, sessionID, uid);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::EventEntry::GetPreferredOutputDeviceDescriptors(
    AudioRendererInfo &rendererInfo, int32_t uid, std::string networkId, const uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    if (coreService_ == nullptr) {
        AUDIO_INFO_LOG("coreService_ is nullptr");
        return std::vector<std::shared_ptr<AudioDeviceDescriptor>>();
    }
    return coreService_->GetPreferredOutputDeviceDescInner(rendererInfo, networkId, uid, streamId);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::EventEntry::GetPreferredInputDeviceDescriptors(
    AudioCapturerInfo &captureInfo, int32_t uid, std::string networkId, const uint32_t streamId)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetPreferredInputDeviceDescInner(captureInfo, networkId, uid, streamId);
}

int32_t AudioCoreService::EventEntry::FetchOutputDeviceAndRoute(std::string caller,
    const AudioStreamDeviceChangeReasonExt reason)
{
    if (coreService_ == nullptr) {
        return ERR_UNKNOWN;
    }

    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->FetchOutputDeviceAndRoute(caller, reason);
}

int32_t AudioCoreService::EventEntry::FetchInputDeviceAndRoute(std::string caller,
    const AudioStreamDeviceChangeReasonExt reason)
{
    CHECK_AND_RETURN_RET(coreService_ != nullptr, ERR_UNKNOWN);
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->FetchInputDeviceAndRoute(caller, reason);
}

std::shared_ptr<AudioDeviceDescriptor> AudioCoreService::EventEntry::GetActiveBluetoothDevice()
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetActiveBluetoothDevice();
}

int32_t AudioCoreService::EventEntry::SkipDeviceTypes(bool skipped, const std::vector<DeviceType> &deviceTypes)
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    return coreService_->SkipDeviceTypes(skipped, deviceTypes);
}

void AudioCoreService::EventEntry::OnDeviceInfoUpdated(
    AudioDeviceDescriptor &desc, const DeviceInfoUpdateCommand command)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    HILOG_COMM_WARN("[OnDeviceInfoUpdated]withlock mac[%{public}s] type[%{public}d] command: %{public}d "
        "category[%{public}d] connectState[%{public}d] isEnable[%{public}d]",
        GetEncryptAddr(desc.macAddress_).c_str(),
        desc.deviceType_, command, desc.deviceCategory_, desc.connectState_, desc.isEnable_);
    coreService_->OnDeviceInfoUpdated(desc, command);
}

int32_t AudioCoreService::EventEntry::SetCallDeviceActive(
    InternalDeviceType deviceType, bool active, std::string address, const int32_t uid)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("[ADeviceEvent] withlock device %{public}d, active %{public}d, uid %{public}d",
        deviceType, active, uid);
    coreService_->SetCallDeviceActive(deviceType, active, address, uid);
    return SUCCESS;
}

int32_t AudioCoreService::EventEntry::RegisterTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo,
    const sptr<IRemoteObject> &object, const int32_t apiVersion)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->RegisterTracker(mode, streamChangeInfo, object, apiVersion);
}

int32_t AudioCoreService::EventEntry::UpdateTracker(AudioMode &mode, AudioStreamChangeInfo &streamChangeInfo)
{
    // This function is called frequently, add xcollie here to avoid freezing for a very long time
    AudioXCollie audioXCollie("EventEntry::UpdateSessionOperation", TIMEOUT_CORE_ENTRY_S,
        nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);

    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->UpdateTracker(mode, streamChangeInfo);
}

void AudioCoreService::EventEntry::RegisteredTrackerClientDied(pid_t uid, pid_t pid)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("[ADeviceEvent] withlock uid %{public}d", uid);
    coreService_->RegisteredTrackerClientDied(uid, pid);
}

bool AudioCoreService::EventEntry::ConnectServiceAdapter()
{
    AUDIO_INFO_LOG("nolock");
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

void AudioCoreService::EventEntry::OnReceiveUpdateDeviceNameEvent(const std::string macAddress,
    const std::string deviceName)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->OnReceiveUpdateDeviceNameEvent(macAddress, deviceName);
}

int32_t AudioCoreService::EventEntry::SelectOutputDevice(sptr<AudioRendererFilter> audioRendererFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc, const int32_t audioDeviceSelectMode)
{
    Trace trace("KeyAction AudioCoreService::SelectOutputDevice");
    if (!selectedDesc.empty() && selectedDesc[0] && coreService_ &&
        audioRendererFilter && audioRendererFilter->rendererInfo.streamUsage == STREAM_USAGE_UNKNOWN) {
        coreService_->NotifyDistributedOutputChange(selectedDesc[0]);
    }
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->SelectOutputDevice(audioRendererFilter, selectedDesc, audioDeviceSelectMode);
}

int32_t AudioCoreService::EventEntry::SetMediaOutputDeviceByUid(DeviceType deviceType, const int32_t uid)
{
    Trace trace("KeyAction AudioCoreService::SetMediaOutputDeviceByUid");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->SetMediaOutputDeviceByUid(deviceType, uid);
}

int32_t AudioCoreService::EventEntry::SelectInputDevice(sptr<AudioCapturerFilter> audioCapturerFilter,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> selectedDesc)
{
    Trace trace("KeyAction AudioCoreService::SelectInputDevice");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->SelectInputDevice(audioCapturerFilter, selectedDesc);
}

int32_t AudioCoreService::EventEntry::SelectInputDeviceByUid(const std::shared_ptr<AudioDeviceDescriptor> &descriptor,
    int32_t uid)
{
    Trace trace("KeyAction AudioCoreService::SelectInputDeviceByUid");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->SelectInputDeviceByUid(descriptor, uid);
}

std::shared_ptr<AudioDeviceDescriptor> AudioCoreService::EventEntry::GetSelectedInputDeviceByUid(int32_t uid)
{
    Trace trace("KeyAction AudioCoreService::GetSelectedInputDeviceByUid");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetSelectedInputDeviceByUid(uid);
}

int32_t AudioCoreService::EventEntry::ClearSelectedInputDeviceByUid(int32_t uid)
{
    Trace trace("KeyAction AudioCoreService::ClearSelectedInputDeviceByUid");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->ClearSelectedInputDeviceByUid(uid);
}

int32_t AudioCoreService::EventEntry::PreferBluetoothAndNearlinkRecordByUid(int32_t uid,
    BluetoothAndNearlinkPreferredRecordCategory category)
{
    Trace trace("KeyAction AudioCoreService::PreferBluetoothAndNearlinkRecordByUid");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->PreferBluetoothAndNearlinkRecordByUid(uid, category);
}

BluetoothAndNearlinkPreferredRecordCategory AudioCoreService::EventEntry::GetPreferBluetoothAndNearlinkRecordByUid(
    int32_t uid)
{
    Trace trace("KeyAction AudioCoreService::GetPreferBluetoothAndNearlinkRecordByUid");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetPreferBluetoothAndNearlinkRecordByUid(uid);
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

void AudioCoreService::EventEntry::CloseWakeUpAudioCapturer()
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    coreService_->CloseWakeUpAudioCapturer();
}

int32_t AudioCoreService::EventEntry::TriggerFetchDevice(AudioStreamDeviceChangeReasonExt reason)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("[ADeviceEvent] withlock");
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

int32_t AudioCoreService::EventEntry::ExcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    Trace trace("AudioCoreService::EventEntry::ExcludeOutputDevices");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERR_MEMORY_ALLOC_FAILED, "coreService_ is nullptr");
    return coreService_->ExcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

int32_t AudioCoreService::EventEntry::UnexcludeOutputDevices(AudioDeviceUsage audioDevUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &audioDeviceDescriptors)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERR_MEMORY_ALLOC_FAILED, "coreService_ is nullptr");
    return coreService_->UnexcludeOutputDevices(audioDevUsage, audioDeviceDescriptors);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioCoreService::EventEntry::GetExcludedDevices(
    AudioDeviceUsage audioDevUsage)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetExcludedDevices(audioDevUsage);
}

int32_t AudioCoreService::EventEntry::SetSessionDefaultOutputDevice(
    const int32_t callerPid, const DeviceType &deviceType, int32_t callerUid)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->SetSessionDefaultOutputDevice(callerPid, deviceType, false, callerUid);
}

int32_t AudioCoreService::EventEntry::GetSessionDefaultOutputDevice(const int32_t callerPid, DeviceType &deviceType)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->GetSessionDefaultOutputDevice(callerPid, deviceType);
}

int32_t AudioCoreService::EventEntry::SetWakeUpAudioCapturerFromAudioServer(const AudioProcessConfig &config)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    return coreService_->SetWakeUpAudioCapturerFromAudioServer(config);
}

uint32_t AudioCoreService::EventEntry::GetPaIndexByPortName(const std::string &portName)
{
    return coreService_->GetPaIndexByPortName(portName);
}

int32_t AudioCoreService::EventEntry::ReleaseOffloadPipe(AudioIOHandle id, uint32_t paIndex, OffloadType type)
{
    CHECK_AND_RETURN_RET_LOG(coreService_, ERR_INVALID_PARAM, "coreService_ is nullptr");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("After wait, isOffloadOpened_: %{public}d", coreService_->isOffloadOpened_[type].load());
    if (coreService_->isOffloadOpened_[type].load()) {
        coreService_->isOffloadInRelease_[type].store(false);
        AUDIO_INFO_LOG("offload restart");
        return ERROR;
    }
    AUDIO_INFO_LOG("Close hdi port id: %{public}u, index %{public}u", id, paIndex);
    coreService_->audioPolicyManager_.CloseAudioPort(id, paIndex);
    CHECK_AND_RETURN_RET_LOG(coreService_->pipeManager_, ERROR, "pipeManager_ is nullptr");
    coreService_->pipeManager_->RemoveAudioPipeInfo(id);
    coreService_->audioIOHandleMap_.DelIOHandleInfo(OFFLOAD_PRIMARY_SPEAKER);
    coreService_->isOffloadInRelease_[type].store(false);
    coreService_->audioPolicyManager_.RemoveAudioPipeVolume(id);
    return SUCCESS;
}

int32_t AudioCoreService::EventEntry::ReleaseMultiChannelPipe(AudioIOHandle id, uint32_t paIndex)
{
    CHECK_AND_RETURN_RET_LOG(coreService_, ERR_INVALID_PARAM, "coreService_ is nullptr");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    AUDIO_INFO_LOG("After wait, isMultiChannelOpened_: %{public}d", coreService_->isMultiChannelOpened_.load());
    if (coreService_->isMultiChannelOpened_.load()) {
        coreService_->isMultiChannelInRelease_.store(false);
        AUDIO_INFO_LOG("MultiChannel restart");
        return ERROR;
    }
    AUDIO_INFO_LOG("Close hdi port id: %{public}u, index %{public}u", id, paIndex);
    coreService_->audioPolicyManager_.CloseAudioPort(id, paIndex);
    CHECK_AND_RETURN_RET_LOG(coreService_->pipeManager_, ERROR, "pipeManager_ is nullptr");
    coreService_->pipeManager_->RemoveAudioPipeInfo(id);
    coreService_->audioIOHandleMap_.DelIOHandleInfo(MCH_PRIMARY_SPEAKER);
    coreService_->isMultiChannelInRelease_.store(false);
    return SUCCESS;
}

int32_t AudioCoreService::EventEntry::ReleasePrimaryExtraPipe(AudioIOHandle id, uint32_t paIndex,
    const std::string &pipeName, const std::string &moduleName)
{
    CHECK_AND_RETURN_RET_LOG(coreService_, ERR_INVALID_PARAM, "coreService_ is nullptr");
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    std::lock_guard<std::mutex> primaryLock(coreService_->primaryExtraCloseMutex_);
    // Guard: if isInRelease was set false, the pipe was already sync-closed by RemoveUnusedPipe
    auto releaseIt = coreService_->isPrimaryExtraInReleaseMap_.find(pipeName);
    if (releaseIt == coreService_->isPrimaryExtraInReleaseMap_.end() || !releaseIt->second) {
        AUDIO_DEBUG_LOG("primary_extra %{public}s already force-closed or not in release, skip",
            pipeName.c_str());
        return ERROR;
    }
    if (coreService_->isPrimaryExtraOpenedMap_[pipeName]) {
        coreService_->isPrimaryExtraInReleaseMap_[pipeName] = false;
        AUDIO_INFO_LOG("primary_extra %{public}s restart", pipeName.c_str());
        return ERROR;
    }
    AUDIO_INFO_LOG("Close %{public}s pipe hdi port id: %{public}u, index %{public}u", pipeName.c_str(), id, paIndex);
    coreService_->audioPolicyManager_.CloseAudioPort(id, paIndex);
    CHECK_AND_RETURN_RET_LOG(coreService_->pipeManager_, ERROR, "pipeManager_ is nullptr");
    coreService_->pipeManager_->RemoveAudioPipeInfo(id);
    coreService_->audioIOHandleMap_.DelIOHandleInfo(moduleName);
    coreService_->isPrimaryExtraInReleaseMap_[pipeName] = false;
    coreService_->audioPolicyManager_.RemoveAudioPipeVolume(id);
    return SUCCESS;
}

int32_t AudioCoreService::EventEntry::ReleaseFastPipe(AudioIOHandle id, uint32_t paIndex,
    const std::string &pipeName, const std::string &moduleName)
{
    CHECK_AND_RETURN_RET_LOG(coreService_, ERR_INVALID_PARAM, "coreService_ is nullptr");
    std::scoped_lock lock(eventMutex_, coreService_->fastCloseMutex_);
    if (coreService_->isFastOpenedMap_[pipeName]) {
        coreService_->isFastInReleaseMap_[pipeName] = false;
        AUDIO_INFO_LOG("FAST pipe %{public}s restart", pipeName.c_str());
        return ERROR;
    }
    return coreService_->ReleaseFastPipeInner(id, paIndex, pipeName, moduleName);
}

int32_t AudioCoreService::EventEntry::SetRendererTarget(RenderTarget target, RenderTarget lastTarget,
    uint32_t sessionId)
{
    return SetRendererTarget(target, lastTarget, sessionId, INVALID_UID, INVALID_STREAM_ID);
}

int32_t AudioCoreService::EventEntry::SetRendererTarget(RenderTarget target, RenderTarget lastTarget,
    uint32_t sessionId, int32_t targetUid, uint32_t targetStreamId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERROR, "Injector::coreService_ is nullptr");
    return coreService_->SetRendererTarget(target, lastTarget, sessionId, targetUid, targetStreamId);
}

int32_t AudioCoreService::EventEntry::StartInjection(uint32_t sessionId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERROR, "Injector::coreService_ is nullptr");
    return coreService_->StartInjection(sessionId);
}

void AudioCoreService::EventEntry::RemoveIdForInjector(uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(coreService_ != nullptr, "Injector::coreService_ is nullptr");
    coreService_->RemoveIdForInjector(streamId);
}

void AudioCoreService::EventEntry::ReleaseCaptureInjector()
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(coreService_ != nullptr, "Injector::coreService_ is nullptr");
    coreService_->ReleaseCaptureInjector();
}

void AudioCoreService::EventEntry::RebuildCaptureInjector(uint32_t streamId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(coreService_ != nullptr, "Injector::coreService_ is nullptr");
    coreService_->RebuildCaptureInjector(streamId);
}

int32_t AudioCoreService::EventEntry::A2dpOffloadGetRenderPosition(uint32_t &delayValue, uint64_t &sendDataSize,
                                                                   uint32_t &timeStamp)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERROR, "coreService_ is nullptr");
    return coreService_->A2dpOffloadGetRenderPosition(delayValue, sendDataSize, timeStamp);
}

void AudioCoreService::EventEntry::OnCheckActiveMusicTime(const std::string &reason)
{
    coreService_->OnCheckActiveMusicTime(reason);
}

int32_t AudioCoreService::EventEntry::CaptureConcurrentCheck(uint32_t sessionId)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERROR, "coreService_ is nullptr");
    return coreService_->CaptureConcurrentCheck(sessionId);
}

void AudioCoreService::EventEntry::NotifySessionStandbyStatus(uint32_t sessionId, bool standbyStatus)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(coreService_ != nullptr, "coreService_ is nullptr");
    coreService_->NotifyStandbyStatus(sessionId, standbyStatus);
}

int32_t AudioCoreService::EventEntry::NotifyStreamStandby(int32_t pid, uint32_t streamId)
{
    std::shared_ptr<AudioCoreService> coreService = nullptr;
    {
        std::lock_guard<std::shared_mutex> lock(eventMutex_);
        CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERROR, "coreService_ is nullptr");
        coreService = coreService_;
    }
    return coreService->NotifyStreamStandby(pid, streamId);
}

int32_t AudioCoreService::EventEntry::NotifyStreamStandbyResume(int32_t pid, uint32_t streamId)
{
    std::shared_ptr<AudioCoreService> coreService = nullptr;
    {
        std::lock_guard<std::shared_mutex> lock(eventMutex_);
        CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERROR, "coreService_ is nullptr");
        coreService = coreService_;
    }
    return coreService->NotifyStreamStandbyResume(pid, streamId);
}

// EventEntry proxy method: notify silent state change
void AudioCoreService::EventEntry::NotifySilentStateChange(uint32_t sessionId, bool isSilent)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(coreService_ != nullptr, "coreService_ is nullptr");
    coreService_->NotifySilentStateChange(sessionId, isSilent);
}

void AudioCoreService::EventEntry::NotifyDataTransSilentStateChange(uint32_t sessionId, bool isSilent)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(coreService_ != nullptr, "coreService_ is nullptr");
    coreService_->NotifyDataTransSilentStateChange(sessionId, isSilent);
}

int32_t AudioCoreService::EventEntry::SetCapturerMuteHint(uint32_t sessionId, bool mute)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERROR, "coreService_ is nullptr");
    return coreService_->SetCapturerMuteHint(sessionId, mute);
}

void AudioCoreService::EventEntry::OnDeviceStateChange(const std::string &networkId, DeviceRole role, bool isAvailable)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(coreService_ != nullptr, "coreService_ is nullptr");
    coreService_->OnDeviceStateChange(networkId, role, isAvailable);
}

void AudioCoreService::EventEntry::SetStreamInterruptMute(uint32_t streamId, bool mute)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(coreService_ != nullptr, "coreService_ is nullptr");
    coreService_->SetStreamInterruptMute(streamId, mute);
}

bool AudioCoreService::EventEntry::IsSupportIndependentPipe()
{
    std::shared_lock<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, false, "coreService_ is nullptr");
    return coreService_->IsSupportIndependentPipe();
}

void AudioCoreService::EventEntry::HandleDeviceConfigChanged(const std::shared_ptr<AudioDeviceDescriptor>
    &selectedAudioDevice)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(coreService_ != nullptr, "Injector::coreService_ is nullptr");
    coreService_->HandleDeviceConfigChanged(selectedAudioDevice);
}

void AudioCoreService::EventEntry::NotifyRemoteRouteStateChange(const std::string &networkId, DeviceType deviceType,
    bool enable)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(coreService_ != nullptr, "coreService_ is nullptr");
    coreService_->NotifyRemoteRouteStateChange(networkId, deviceType, enable);
}

void AudioCoreService::EventEntry::ReloadSourceForEffect(
    const AudioEffectPropertyArray oldPropertyArray, const AudioEffectPropertyArray  propertyArray)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(coreService_ != nullptr, "coreService_ is nullptr");
    coreService_->ReloadSourceForEffect(oldPropertyArray, propertyArray);
}

void AudioCoreService::EventEntry::TriggerVoipPriorityReload(uint32_t sessionId, bool lowered)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_LOG(coreService_ != nullptr, "coreService_ is nullptr");
    coreService_->NotifyVoipPriorityLoweredByMute(sessionId, lowered);
}

int32_t AudioCoreService::EventEntry::GetActiveStreamRendererInfosForDevice(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc,
    std::vector<AudioRendererInfo> &rendererInfos)
{
    std::lock_guard<std::shared_mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(coreService_ != nullptr, ERR_NULL_POINTER, "coreService_ is nullptr");
    return coreService_->GetActiveStreamRendererInfosForDevice(deviceDesc, rendererInfos);
}
}
}
