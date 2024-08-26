/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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
#include "audio_info.h"
#include "audio_system_manager.h"
#include "audio_policy_client.h"
#include "i_standard_concurrency_state_listener.h"
#include "i_standard_audio_policy_manager_listener.h"
#include "i_standard_audio_routing_manager_listener.h"
#include "i_audio_interrupt_event_dispatcher.h"
#include "i_audio_concurrency_event_dispatcher.h"

namespace OHOS {
namespace AudioStandard {
constexpr int32_t MAX_DELAY_TIME = 4 * 1000;

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
        MIC_STATE_CHANGE_EVENT,
        MIC_STATE_CHANGE_EVENT_WITH_CLIENTID,
        INTERRUPT_EVENT,
        INTERRUPT_EVENT_WITH_SESSIONID,
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
        PIPE_STREAM_CLEAN_EVENT,
        CONCURRENCY_EVENT_WITH_SESSIONID,
        AUDIO_SESSION_DEACTIVE_EVENT,
    };
    /* event data */
    class EventContextObj {
    public:
        DeviceChangeAction deviceChangeAction;
        VolumeEvent volumeEvent;
        AudioInterrupt audioInterrupt;
        std::list<std::pair<AudioInterrupt, AudioFocuState>> focusInfoList;
        AudioRingerMode ringMode;
        MicStateChangeEvent micStateChangeEvent;
        InterruptEventInternal interruptEvent;
        uint32_t sessionId;
        int32_t clientId;
        sptr<AudioDeviceDescriptor> descriptor;
        CastType type;
        bool spatializationEnabled;
        bool headTrackingEnabled;
        std::vector<std::unique_ptr<AudioRendererChangeInfo>> audioRendererChangeInfos;
        std::vector<std::unique_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
        int32_t streamFlag;
        std::unordered_map<std::string, bool> headTrackingDeviceChangeInfo;
        AudioStreamDeviceChangeReasonExt reason_ = AudioStreamDeviceChangeReasonExt::ExtEnum::UNKNOWN;
        std::pair<int32_t, AudioSessionDeactiveEvent> sessionDeactivePair;
    };

    struct RendererDeviceChangeEvent {
        RendererDeviceChangeEvent() = delete;
        RendererDeviceChangeEvent(const int32_t clientPid, const uint32_t sessionId,
            const DeviceInfo outputDeviceInfo, const AudioStreamDeviceChangeReason &reason)
            : clientPid_(clientPid), sessionId_(sessionId), outputDeviceInfo_(outputDeviceInfo), reason_(reason)
        {}

        const int32_t clientPid_;
        const uint32_t sessionId_;
        const DeviceInfo outputDeviceInfo_;
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

    void AddAudioPolicyClientProxyMap(int32_t clientPid, const sptr<IAudioPolicyClient> &cb);
    void RemoveAudioPolicyClientProxyMap(pid_t clientPid);
    void AddExternInterruptCbsMap(int32_t clientId, const std::shared_ptr<AudioInterruptCallback> &callback);
    int32_t RemoveExternInterruptCbsMap(int32_t clientId);
    void AddAvailableDeviceChangeMap(int32_t clientId, const AudioDeviceUsage usage,
        const sptr<IStandardAudioPolicyManagerListener> &callback);
    void RemoveAvailableDeviceChangeMap(const int32_t clientId, AudioDeviceUsage usage);
    void AddDistributedRoutingRoleChangeCbsMap(int32_t clientId,
        const sptr<IStandardAudioRoutingManagerListener> &callback);
    int32_t RemoveDistributedRoutingRoleChangeCbsMap(int32_t clientId);
    void AddConcurrencyEventDispatcher(std::shared_ptr<IAudioConcurrencyEventDispatcher> dispatcher);
    bool SendDeviceChangedCallback(const std::vector<sptr<AudioDeviceDescriptor>> &desc, bool isConnected);
    bool SendAvailableDeviceChange(const std::vector<sptr<AudioDeviceDescriptor>> &desc, bool isConnected);
    bool SendVolumeKeyEventCallback(const VolumeEvent &volumeEvent);
    bool SendAudioFocusInfoChangeCallback(int32_t callbackCategory, const AudioInterrupt &audioInterrupt,
        const std::list<std::pair<AudioInterrupt, AudioFocuState>> &focusInfoList);
    bool SendRingerModeUpdatedCallback(const AudioRingerMode &ringMode);
    bool SendMicStateUpdatedCallback(const MicStateChangeEvent &micStateChangeEvent);
    bool SendMicStateWithClientIdCallback(const MicStateChangeEvent &micStateChangeEvent, int32_t clientId);
    bool SendInterruptEventInternalCallback(const InterruptEventInternal &interruptEvent);
    bool SendInterruptEventWithSessionIdCallback(const InterruptEventInternal &interruptEvent,
        const uint32_t &sessionId);
    bool SendInterruptEventWithClientIdCallback(const InterruptEventInternal &interruptEvent,
        const int32_t &clientId);
    bool SendPreferredOutputDeviceUpdated();
    bool SendPreferredInputDeviceUpdated();
    bool SendDistributedRoutingRoleChange(const sptr<AudioDeviceDescriptor> descriptor,
        const CastType &type);
    bool SendRendererInfoEvent(const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos);
    bool SendCapturerInfoEvent(const std::vector<std::unique_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos);
    bool SendRendererDeviceChangeEvent(const int32_t clientPid, const uint32_t sessionId,
        const DeviceInfo &outputDeviceInfo, const AudioStreamDeviceChangeReasonExt reason);
    bool SendCapturerCreateEvent(AudioCapturerInfo capturerInfo, AudioStreamInfo streamInfo,
        uint64_t sessionId, bool isSync, int32_t &error);
    bool SendCapturerRemovedEvent(uint64_t sessionId, bool isSync);
    bool SendWakeupCloseEvent(bool isSync);
    bool SendRecreateRendererStreamEvent(int32_t clientId, uint32_t sessionID, int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason);
    bool SendRecreateCapturerStreamEvent(int32_t clientId, uint32_t sessionID, int32_t streamFlag,
        const AudioStreamDeviceChangeReasonExt reason);
    bool SendHeadTrackingDeviceChangeEvent(const std::unordered_map<std::string, bool> &changeInfo);
    void AddAudioDeviceRefinerCb(const sptr<IStandardAudioRoutingManagerListener> &callback);
    int32_t RemoveAudioDeviceRefinerCb();
    bool SendSpatializatonEnabledChangeEvent(const bool &enabled);
    bool SendSpatializatonEnabledChangeForAnyDeviceEvent(const sptr<AudioDeviceDescriptor> &selectedAudioDevice,
        const bool &enabled);
    bool SendHeadTrackingEnabledChangeEvent(const bool &enabled);
    bool SendHeadTrackingEnabledChangeForAnyDeviceEvent(const sptr<AudioDeviceDescriptor> &selectedAudioDevice,
    const bool &enabled);
    bool SendPipeStreamCleanEvent(AudioPipeType pipeType);
    bool SendConcurrencyEventWithSessionIDCallback(const uint32_t sessionID);
    int32_t SetClientCallbacksEnable(const CallbackChange &callbackchange, const bool &enable);
    bool SendAudioSessionDeactiveCallback(const std::pair<int32_t, AudioSessionDeactiveEvent> &sessionDeactivePair);

protected:
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;

private:
    /* Handle Event*/
    void HandleDeviceChangedCallback(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleAvailableDeviceChange(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleVolumeKeyEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleRequestCateGoryEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleAbandonCateGoryEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleFocusInfoChangeEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleRingerModeUpdatedEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleMicStateUpdatedEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleMicStateUpdatedEventWithClientId(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleInterruptEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleInterruptEventWithSessionId(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleInterruptEventWithClientId(const AppExecFwk::InnerEvent::Pointer &event);
    void HandlePreferredOutputDeviceUpdated();
    void HandlePreferredInputDeviceUpdated();
    void HandleDistributedRoutingRoleChangeEvent(const AppExecFwk::InnerEvent::Pointer &event);
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
    void HandleHeadTrackingEnabledChangeEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleHeadTrackingEnabledChangeForAnyDeviceEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandlePipeStreamCleanEvent(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleConcurrencyEventWithSessionID(const AppExecFwk::InnerEvent::Pointer &event);
    void HandleAudioSessionDeactiveCallback(const AppExecFwk::InnerEvent::Pointer &event);

    void HandleServiceEvent(const uint32_t &eventId, const AppExecFwk::InnerEvent::Pointer &event);

    void HandleOtherServiceEvent(const uint32_t &eventId, const AppExecFwk::InnerEvent::Pointer &event);

    void ResetRingerModeMute(const std::vector<std::unique_ptr<AudioRendererChangeInfo>> &audioRendererChangeInfos);

    std::mutex runnerMutex_;
    std::weak_ptr<IAudioInterruptEventDispatcher> interruptEventDispatcher_;
    std::weak_ptr<IAudioConcurrencyEventDispatcher> concurrencyEventDispatcher_;

    std::unordered_map<int32_t, sptr<IAudioPolicyClient>> audioPolicyClientProxyAPSCbsMap_;
    std::unordered_map<int32_t, std::shared_ptr<AudioInterruptCallback>> amInterruptCbsMap_;
    std::map<std::pair<int32_t, AudioDeviceUsage>,
        sptr<IStandardAudioPolicyManagerListener>> availableDeviceChangeCbsMap_;
    std::unordered_map<int32_t, sptr<IStandardAudioRoutingManagerListener>> distributedRoutingRoleChangeCbsMap_;
    std::unordered_map<int32_t,  std::unordered_map<CallbackChange, bool>> clientCallbacksMap_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_POLICY_SERVER_HANDLER_H
