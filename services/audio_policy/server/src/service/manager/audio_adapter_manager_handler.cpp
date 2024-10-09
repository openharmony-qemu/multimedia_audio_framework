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
#define LOG_TAG "AudioAdapterManagerHandler"
#endif

#include "audio_adapter_manager_handler.h"
#include "audio_policy_manager_factory.h"

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t MAX_DELAY_TIME = 4 * 1000;
}
AudioAdapterManagerHandler::AudioAdapterManagerHandler() : AppExecFwk::EventHandler(
    AppExecFwk::EventRunner::Create("OS_APAdapterAsyncRunner"))
{
    AUDIO_DEBUG_LOG("ctor");
}

AudioAdapterManagerHandler::~AudioAdapterManagerHandler()
{
    AUDIO_WARNING_LOG("dtor should not happen");
};

bool AudioAdapterManagerHandler::SendKvDataUpdate(const bool &isFirstBoot)
{
    auto eventContextObj = std::make_shared<bool>(isFirstBoot);
    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = true;
    if (isFirstBoot) {
        ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAdapterManagerServerCmd::DATABASE_UPDATE, eventContextObj),
            MAX_DELAY_TIME);
    } else {
        ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAdapterManagerServerCmd::DATABASE_UPDATE, eventContextObj));
    }
    CHECK_AND_RETURN_RET_LOG(ret, ret, "SendKvDataUpdate event failed");
    return ret;
}

bool AudioAdapterManagerHandler::SendSaveVolume(const DeviceType &deviceType,
    const AudioStreamType &streamType, const int32_t &volumeLevel)
{
    auto eventContextObj = std::make_shared<VolumeDataEvent>(deviceType, streamType, volumeLevel);
    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = true;
    ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAdapterManagerServerCmd::VOLUME_DATABASE_SAVE, eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "SendSaveVolume event failed");
    return ret;
}

bool AudioAdapterManagerHandler::SendStreamMuteStatusUpdate(const AudioStreamType &streamType, const bool &mute,
    const StreamUsage &streamUsage)
{
    auto eventContextObj = std::make_shared<StreamMuteStatusEvent>(streamType, mute, streamUsage);
    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = true;
    ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAdapterManagerServerCmd::STREAM_MUTE_STATUS_UPDATE,
        eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "SendStreamMuteStatusUpdate event failed");
    return ret;
}

bool AudioAdapterManagerHandler::SendRingerModeUpdate(const AudioRingerMode &ringerMode)
{
    auto eventContextObj = std::make_shared<RingerModeEvent>(ringerMode);
    lock_guard<mutex> runnerlock(runnerMutex_);
    bool ret = true;
    ret = SendEvent(AppExecFwk::InnerEvent::Get(EventAdapterManagerServerCmd::RINGER_MODE_UPDATE, eventContextObj));
    CHECK_AND_RETURN_RET_LOG(ret, ret, "SendRingerModeUpdate event failed");
    return ret;
}

void AudioAdapterManagerHandler::HandleUpdateKvDataEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<bool> eventContextObj = event->GetSharedObject<bool>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    bool isFristBoot = *eventContextObj;
    AudioPolicyManagerFactory::GetAudioPolicyManager().HandleKvData(isFristBoot);
}

void AudioAdapterManagerHandler::HandleVolumeDataBaseSave(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<VolumeDataEvent> eventContextObj = event->GetSharedObject<VolumeDataEvent>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    AudioPolicyManagerFactory::GetAudioPolicyManager().HandleSaveVolume(eventContextObj->deviceType_,
        eventContextObj->streamType_, eventContextObj->volumeLevel_);
}

void AudioAdapterManagerHandler::HandleUpdateStreamMuteStatus(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<StreamMuteStatusEvent> eventContextObj = event->GetSharedObject<StreamMuteStatusEvent>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    AudioPolicyManagerFactory::GetAudioPolicyManager().HandleStreamMuteStatus(eventContextObj->streamType_,
        eventContextObj->mute_, eventContextObj->streamUsage_);
}

void AudioAdapterManagerHandler::HandleUpdateRingerMode(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<RingerModeEvent> eventContextObj = event->GetSharedObject<RingerModeEvent>();
    CHECK_AND_RETURN_LOG(eventContextObj != nullptr, "EventContextObj get nullptr");
    AudioPolicyManagerFactory::GetAudioPolicyManager().HandleRingerMode(eventContextObj->ringerMode_);
}

void AudioAdapterManagerHandler::ReleaseEventRunner()
{
    SetEventRunner(nullptr);
}

void AudioAdapterManagerHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventId = event->GetInnerEventId();
    AUDIO_DEBUG_LOG("handler process eventId:%{public}u", eventId);

    switch (eventId) {
        case EventAdapterManagerServerCmd::DATABASE_UPDATE:
            HandleUpdateKvDataEvent(event);
            break;
        case EventAdapterManagerServerCmd::VOLUME_DATABASE_SAVE:
            HandleVolumeDataBaseSave(event);
            break;
        case STREAM_MUTE_STATUS_UPDATE:
            HandleUpdateStreamMuteStatus(event);
            break;
        case RINGER_MODE_UPDATE:
            HandleUpdateRingerMode(event);
            break;
        default:
            break;
    }
}
} // namespace AudioStandard
} // namespace OHOS
