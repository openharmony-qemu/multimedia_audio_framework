/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef AUDIO_POLICY_SERVER_HANDLER_H
#define AUDIO_POLICY_SERVER_HANDLER_H
#include <mutex>

#include "singleton.h"
#include "event_handler.h"
#include "event_runner.h"

#include "audio_policy_log.h"
#include "audio_system_manager.h"
#include "istandard_audio_policy_manager_listener.h"
#include "i_audio_interrupt_event_dispatcher.h"
#include "i_audio_zone_event_dispatcher.h"

namespace OHOS {
namespace AudioStandard {

class IStandardAudioRoutingManagerListener;
class AudioPolicyClientHolder;
class AudioPolicyManagerListenerCallback;
class AudioPolicyServerHandler : public AppExecFwk::EventHandler {
    DECLARE_DELAYED_SINGLETON(AudioPolicyServerHandler)
public:
    enum FocusCallbackCategory : int32_t {
        NONE_CALLBACK_CATEGORY,
        REQUEST_CALLBACK_CATEGORY,
        ABANDON_CALLBACK_CATEGORY,
    };

    enum EventAudioServerCmd {
        AUDIO_DEVICE_CHANGE,
        AVAILABLE_AUDIO_DEVICE_CHANGE,
        VOLUME_KEY_EVENT,
        REQUEST_CATEGORY_EVENT,
        ABANDON_CATEGORY_EVENT,
        FOCUS_INFOCHANGE,
        RINGER_MODEUPDATE_EVENT,
        APP_VOLUME_CHANGE_EVENT,
        ACTIVE_VOLUME_TYPE_CHANGE_EVENT,
        MIC_STATE_CHANGE_EVENT,
        MIC_STATE_CHANGE_EVENT_WITH_CLIENTID,
        INTERRUPT_EVENT,
        INTERRUPT_EVENT_WITH_STREAMID,
        INTERRUPT_EVENT_WITH_CLIENTID,
        PREFERRED_OUTPUT_DEVICE_UPDATED,
        PREFERRED_INPUT_DEVICE_UPDATED,
        DISTRIBUTED_ROUTING_ROLE_CHANGE,
        RENDERER_INFO_EVENT,
        CAPTURER_INFO_EVENT,
        RENDERER_DEVICE_CHANGE_EVENT,
        ON_CAPTURER_CREATE,
        ON_CAPTURER_REMOVED,
        ON_WAKEUP_CLOSE,
        RECREATE_RENDERER_STREAM_EVENT,
        RECREATE_CAPTURER_STREAM_EVENT,
        HEAD_TRACKING_DEVICE_CHANGE,
        SPATIALIZATION_ENABLED_CHANGE,
        SPATIALIZATION_ENABLED_CHANGE_FOR_ANY_DEVICE,
        HEAD_TRACKING_ENABLED_CHANGE,
        HEAD_TRACKING_ENABLED_CHANGE_FOR_ANY_DEVICE,
        AUDIO_SESSION_DEACTIVE_EVENT,
        MICROPHONE_BLOCKED,
        NN_STATE_CHANGE,
        AUDIO_SCENE_CHANGE,
        SPATIALIZATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE,
        AUDIO_ZONE_EVENT,
        FORMAT_UNSUPPORTED_ERROR,
        SESSION_DEVICE_CHANGE,
        SESSION_INPUT_DEVICE_CHANGE,
        INTERRUPT_EVENT_FOR_AUDIO_SESSION,
        VOLUME_DEGREE_EVENT,
        AUDIO_DEVICE_INFO_UPDATE,
        COLLABORATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE,
    };
    /* event data */
    class EventContextObj {
    public:
        DeviceChangeAction deviceChangeAction;
        MicrophoneBlockedInfo microphoneBlockedInfo;
        VolumeEvent volumeEvent;
        AudioVolumeType volumeType;
        AudioInterrupt audioInterrupt;
        std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
        AudioRingerMode ringMode;
        MicStateChangeEvent micStateChangeEvent;
        InterruptEventInternal interruptEvent;
        uint32_t sessionId;
        int32_t clientId;
        std::shared_ptr<AudioDeviceDescriptor> descriptor;
        CastType type;
        bool spatializationEnabled;
        bool headTrackingEnabled;
        AudioScene audioScene;
        int32_t nnState;
        std::vector<std::shared_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
        std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
        int32_t streamFlag;
        int32_t appUid;
        std::unordered_map<std::string, bool> headTrackingDeviceChangeInfo;
        AudioStreamDeviceChangeReasonExt reason_ = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
        std::pair<int32_t, AudioSessionDeactiveEvent> sessionDeactivePair;
        std::shared_ptr<AudioZoneEvent> audioZoneEvent;
        uint32_t routeFlag;
        AudioErrors errorCode;
        int32_t callerPid_ = -1;
        bool collaborationEnabled;
    };

    struct RendererDeviceChangeEvent {
        RendererDeviceChangeEvent() = delete;
        RendererDeviceChangeEvent(const int32_t clientPid, const uint32_t sessionId,
            const AudioDeviceDescriptor outputDeviceInfo, const AudioStreamDeviceChangeReason &reason)
            : clientPid_(clientPid), sessionId_(sessionId), outputDeviceInfo_(outputDeviceInfo), reason_(reason)
        {}

        const int32_t clientPid_;
        const uint32_t sessionId_;
        const AudioDeviceDescriptor outputDeviceInfo_ = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
        AudioStreamDeviceChangeReasonExt reason_ = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
    };

    struct CapturerCreateEvent {
        CapturerCreateEvent() = delete;
        CapturerCreateEvent(const AudioCapturerInfo &capturerInfo, const AudioStreamInfo &streamInfo,
            uint64_t sessionId, int32_t error)
            : capturerInfo_(capturerInfo), streamInfo_(streamInfo), sessionId_(sessionId), error_(error)
        {}
        AudioCapturerInfo capturerInfo_;
        AudioStreamInfo streamInfo_;
        uint64_t sessionId_;
        int32_t error_;
    };

    void Init(std::shared_ptr<IAudioInterruptEventDispatcher> dispatcher);

    void AddAudioPolicyClientProxyMap(int32_t clientPid, const std::shared_ptr<AudioPolicyClientHolder> &cb);
    void RemoveAudioPolicyClientProxyMap(pid_t clientPid);
    void AddExternInterruptCbsMap(int32_t clientId, const std::shared_ptr<AudioInterruptCallback> &callback);
    int32_t RemoveExternInterruptCbsMap(int32_t clientId);
    void AddAvailableDeviceChangeMap(int32_t clientId, const AudioDeviceUsage usage,
        const std::shared_ptr<AudioPolicyManagerListenerCallback> &callback);
    void RemoveAvailableDeviceChangeMap(const int32_t clientId, AudioDeviceUsage usage);
    void AddDistributedRoutingRoleChangeCbsMap(int32_t clientId,
        const sptr<IStandardAudioRoutingManagerListener> &callback);
    int32_t RemoveDistributedRoutingRoleChangeCbsMap(int32_t clientId);
    bool SendDeviceChangedCallback(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc, bool isConnected);
    bool SendDeviceInfoUpdatedCallback(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc);
    bool SendAvailableDeviceChange(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc, bool isConnected);
    bool SendMicrophoneBlockedCallback(const std::vector<std::shared_ptr<AudioDeviceDescriptor>> &desc,
        DeviceBlockStatus status);
    void HandleMicrophoneBlockedCallback(const AppExecFwk::InnerEvent::Pointer &event);
    bool SendVolumeKeyEventCallback(const VolumeEvent &volumeEvent);
    bool SendVolumeDegreeEventCallback(const VolumeEvent &volumeEvent);
    bool SendAudioFocusInfoChangeCallback(int32_t callbackCategory, const AudioInterrupt &audioInterrupt,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList);
    bool SendRingerModeUpdatedCallback(const AudioRingerMode &ringMode);
    bool SendActiveVolumeTypeChangeCallback(const AudioVolumeType &volumeType);
    bool SendAppVolumeChangeCallback(int32_t appUid, const VolumeEvent &volumeEvent);
    bool SendMicStateUpdatedCallback(const MicStateChangeEvent &micStateChangeEvent);
    bool SendMicStateWithClientIdCallback(const MicStateChangeEvent &micStateChangeEvent, int32_t clientId);
    bool SendInterruptEventInternalCallback(const InterruptEventInternal &interruptEvent);
    bool SendInterruptEventWithStreamIdCallback(const InterruptEventInternal &interruptEvent,
        const uint32_t &streamId);
    bool SendInterruptEventCallbackForAudioSession(const InterruptEventInternal &interruptEvent,
        const AudioInterrupt &audioInterrupt);
    bool SendInterruptEventWithClientIdCallback(const InterruptEventInternal &interruptEvent,
        const int32_t &clientId);
    bool SendPreferredOutputDeviceUpdated();
    bool SendPreferredInputDeviceUpdated();
    bool SendDistributedRoutingRoleChange(const std::shared_ptr<AudioDeviceDescriptor> descriptor,
        const CastType &type);
    bool SendRendererInfoEvent(const std::vector<std::shared_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos);
    bool SendCapturerInfoEvent(const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos);
    bool SendRendererDeviceChangeEvent(const int32_t clientPid, const uint32_t sessionId,
        const AudioDeviceDescriptor &outputDeviceInfo, const AudioStreamDeviceChangeReasonExt reason);
    bool SendCapturerCreateEvent(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
        uint64_t sessionId, bool isSync, int32_t &error);
    bool SendCapturerRemovedEvent(uint64_t sessionId, bool isSync);
    bool SendWakeupCloseEvent(bool isSync);
    bool SendRecreateRendererStreamEvent(int32_t clientId, uint32_t sessionID, uint32_t routeFlag,
        const AudioStreamDeviceChangeReasonExt reason);
    bool SendRecreateCapturerStreamEvent(int32_t clientId, uint32_t sessionID, uint32_t routeFlag,
        const AudioStreamDeviceChangeReasonExt reason);
    bool SendHeadTrackingDeviceChangeEvent(const std::unordered_map<std::string, bool> &changeInfo);
    void AddAudioDeviceRefinerCb(const sptr<IStandardAudioRoutingManagerListener> &callback);
    int32_t RemoveAudioDeviceRefinerCb();
    bool SendSpatializatonEnabledChangeEvent(const bool &enabled);
    bool SendSpatializatonEnabledChangeForAnyDeviceEvent(
        const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, const bool &enabled);
    bool SendSpatializatonEnabledChangeForCurrentDeviceEvent(const bool &enabled);
    bool SendHeadTrackingEnabledChangeEvent(const bool &enabled);
    bool SendHeadTrackingEnabledChangeForAnyDeviceEvent(
        const std::shared_ptr<AudioDeviceDescriptor> &selectedAudioDevice, const bool &enabled);
    int32_t SetClientCallbacksEnable(const CallbackChange &callbackchange, const bool &enable);
    int32_t SetCallbackRendererInfo(const AudioRendererInfo &rendererInfo, const int32_t uid = -1);
    int32_t SetCallbackCapturerInfo(const AudioCapturerInfo &capturerInfo);
    bool SendAudioSceneChangeEvent(const AudioScene &audioScene);
    bool SendAudioSessionDeactiveCallback(const std::pair<int32_t, AudioSessionDeactiveEvent> &sessionDeactivePair);
    bool SendNnStateChangeCallback(const int32_t &state);
    void SetAudioZoneEventDispatcher(const std::shared_ptr<IAudioZoneEventDispatcher> dispatcher);
    bool SendAudioZoneEvent(std::shared_ptr<AudioZoneEvent> event);
    bool SendFormatUnsupportedErrorEvent(const AudioErrors &errorCode);
    int32_t SetCallbackStreamUsageInfo(const std::set<StreamUsage> &streamUsages);
    bool SendAudioSessionDeviceChange(const AudioStreamDeviceChangeReason changeReason, int32_t callerPid = -1);
    bool SendAudioSessionInputDeviceChange(const AudioStreamDeviceChangeReason changeReason, int32_t callerPid = -1);
    void SendCollaborationEnabledChangeForCurrentDeviceEvent(const bool &enabled);

protected:
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;

private:
    /* Handle Event*/
    void HandleDeviceChangedCallback(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleDeviceInfoUpdatedCallback(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleAvailableDeviceChange(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleVolumeKeyEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleVolumeDegreeEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleRequestCateGoryEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleAbandonCateGoryEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleFocusInfoChangeEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleRingerModeUpdatedEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleMicStateUpdatedEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleMicStateUpdatedEventWithClientId(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleInterruptEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleInterruptEventForAudioSession(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleInterruptEventWithStreamId(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleInterruptEventWithClientId(const AppExecFwk::InnerEvent::Pointer &event);
    void HandlePreferredOutputDeviceUpdated();
    void HandlePreferredInputDeviceUpdated();
    void HandleDistributedRoutingRoleChangeEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleAudioSessionDeviceChangeEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleAudioSessionInputDeviceChangeEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleRendererInfoEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleCapturerInfoEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleRendererDeviceChangeEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleCapturerCreateEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleCapturerRemovedEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleWakeupCloseEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleSendRecreateRendererStreamEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleSendRecreateCapturerStreamEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleHeadTrackingDeviceChangeEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleSpatializatonEnabledChangeEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleSpatializatonEnabledChangeForAnyDeviceEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleSpatializatonEnabledChangeForCurrentDeviceEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleHeadTrackingEnabledChangeEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleHeadTrackingEnabledChangeForAnyDeviceEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleAudioSessionDeactiveCallback(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleNnStateChangeEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleAudioSceneChange(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleAppVolumeChangeEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleAudioZoneEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleFormatUnsupportedErrorEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleActiveVolumeTypeChangeEvent(const AppExecFwk::InnerEvent::Pointer &event);

    void HandleServiceEvent(const uint32_t &eventId, const AppExecFwk::InnerEvent::Pointer &event);

    void HandleOtherServiceEvent(const uint32_t &eventId, const AppExecFwk::InnerEvent::Pointer &event);
    void HandleOtherServiceSecondEvent(const uint32_t &eventId, const AppExecFwk::InnerEvent::Pointer &event);

    void HandleVolumeChangeCallback(int32_t clientId, std::shared_ptr<AudioPolicyClientHolder> audioPolicyClient,
        const VolumeEvent &volumeEvent);

    void HandleVolumeKeyEventToRssWhenAccountsChange(std::shared_ptr<EventContextObj> &eventContextObj);
    void HandleCollaborationEnabledChangeForCurrentDeviceEvent(const AppExecFwk::InnerEvent::Pointer &event);

    std::vector<AudioRendererFilter> GetCallbackRendererInfoList(int32_t clientPid);
    std::vector<AudioCapturerInfo> GetCallbackCapturerInfoList(int32_t clientPid);

    std::mutex runnerMutex_;
    std::mutex handleMapMutex_;
    std::mutex clientCbRendererInfoMapMutex_;
    std::mutex clientCbCapturerInfoMapMutex_;
    std::mutex clientCbStreamUsageMapMutex_;
    std::weak_ptr<IAudioInterruptEventDispatcher> interruptEventDispatcher_;
    std::weak_ptr<IAudioZoneEventDispatcher> audioZoneEventDispatcher_;

    std::unordered_map<int32_t, std::shared_ptr<AudioPolicyClientHolder>> audioPolicyClientProxyAPSCbsMap_;
    std::string pidsStrForPrinting_ = "[]";

    std::unordered_map<int32_t, std::shared_ptr<AudioInterruptCallback>> amInterruptCbsMap_;
    std::map<std::pair<int32_t, AudioDeviceUsage>,
        std::shared_ptr<AudioPolicyManagerListenerCallback>> availableDeviceChangeCbsMap_;
    std::unordered_map<int32_t, sptr<IStandardAudioRoutingManagerListener>> distributedRoutingRoleChangeCbsMap_;
    std::unordered_map<int32_t,  std::unordered_map<CallbackChange, bool>> clientCallbacksMap_;
    int32_t pidOfRss_ = -1;
    std::unordered_map<int32_t, std::vector<AudioRendererFilter>> clientCbRendererInfoMap_;
    std::unordered_map<int32_t, std::vector<AudioCapturerInfo>> clientCbCapturerInfoMap_;
    std::unordered_map<int32_t, std::set<StreamUsage>> clientCbStreamUsageMap_;
    std::unordered_map<int32_t, int32_t> pidUidMap_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_SERVER_HANDLER_H
