/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioRouterCenter"
#endif

#include "audio_router_center.h"
#include "audio_policy_service.h"
#include "audio_zone_service.h"
#include "audio_scene_manager.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

const string MEDIA_RENDER_ROUTERS = "MediaRenderRouters";
const string CALL_RENDER_ROUTERS = "CallRenderRouters";
const string RECORD_CAPTURE_ROUTERS = "RecordCaptureRouters";
const string CALL_CAPTURE_ROUTERS = "CallCaptureRouters";
const string RING_RENDER_ROUTERS = "RingRenderRouters";
const string TONE_RENDER_ROUTERS = "ToneRenderRouters";

shared_ptr<AudioDeviceDescriptor> AudioRouterCenter::FetchMediaRenderDevice(
    StreamUsage streamUsage, int32_t clientUID, RouterType &routerType, const RouterType &bypassType)
{
    for (auto &router : mediaRenderRouters_) {
        if (router->GetRouterType() == bypassType) {
            AUDIO_INFO_LOG("Fetch media render device bypass %{public}d", bypassType);
            continue;
        }
        shared_ptr<AudioDeviceDescriptor> desc = router->GetMediaRenderDevice(streamUsage, clientUID);
        if ((desc != nullptr) && (desc->deviceType_ != DEVICE_TYPE_NONE)) {
            routerType = router->GetRouterType();
            return desc;
        }
    }
    return make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> AudioRouterCenter::FetchCallRenderDevice(StreamUsage streamUsage, int32_t clientUID,
    RouterType &routerType, const RouterType &bypassType, const RouterType &bypassWithSco)
{
    for (auto &router : callRenderRouters_) {
        if (router->GetRouterType() == bypassType || router->GetRouterType() == bypassWithSco) {
            AUDIO_INFO_LOG("Fetch call render device bypass %{public}d, bypassWithSco %{public}d",
                bypassType, bypassWithSco);
            continue;
        }
        shared_ptr<AudioDeviceDescriptor> desc = router->GetCallRenderDevice(streamUsage, clientUID);
        if ((desc != nullptr) && (desc->deviceType_ != DEVICE_TYPE_NONE)) {
            routerType = router->GetRouterType();
            return desc;
        }
    }
    return make_shared<AudioDeviceDescriptor>();
}

vector<shared_ptr<AudioDeviceDescriptor>> AudioRouterCenter::FetchRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID, RouterType &routerType)
{
    for (auto &router : ringRenderRouters_) {
        CHECK_AND_CONTINUE_LOG(router != nullptr, "Invalid router.");
        vector<shared_ptr<AudioDeviceDescriptor>> descs = router->GetRingRenderDevices(streamUsage, clientUID);
        CHECK_AND_CONTINUE_LOG(!descs.empty(), "FetchRingRenderDevices is empty.");
        if (descs.front() != nullptr && descs.front()->deviceType_ != DEVICE_TYPE_NONE) {
            AUDIO_INFO_LOG("RingRender streamUsage %{public}d clientUID %{public}d"
                " fetch descs front:%{public}d", streamUsage, clientUID, descs.front()->deviceType_);
            routerType = router->GetRouterType();
            if (descs.size() > 1 && VolumeUtils::IsPCVolumeEnable()) {
                vector<shared_ptr<AudioDeviceDescriptor>> newDescs;
                newDescs.push_back(descs.front());
                return newDescs;
            }
            return descs;
        }
    }
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    if (streamUsage == STREAM_USAGE_RINGTONE || streamUsage == STREAM_USAGE_VOICE_RINGTONE) {
        AudioRingerMode curRingerMode = AudioPolicyManagerFactory::GetAudioPolicyManager().GetRingerMode();
        if (curRingerMode == RINGER_MODE_NORMAL) {
            descs.push_back(AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice());
        } else {
            descs.push_back(make_shared<AudioDeviceDescriptor>());
        }
    } else {
        descs.push_back(AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice());
    }
    return descs;
}

bool AudioRouterCenter::HasScoDevice()
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetCommRenderPrivacyDevices();
    for (auto &desc : descs) {
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
            return true;
        }
    }

    vector<shared_ptr<AudioDeviceDescriptor>> publicDescs =
        AudioDeviceManager::GetAudioDeviceManager().GetCommRenderPublicDevices();
    for (auto &desc : publicDescs) {
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && desc->deviceCategory_ == BT_CAR) {
            return true;
        }
    }
    return false;
}

bool AudioRouterCenter::NeedSkipSelectAudioOutputDeviceRefined(StreamUsage streamUsage,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
{
    if (AudioPolicyManagerFactory::GetAudioPolicyManager().GetRingerMode() == RINGER_MODE_NORMAL) {
        return false;
    }
    if (!Util::IsRingerOrAlarmerStreamUsage(streamUsage)) {
        return false;
    }
    if (descs.size() != 1) {
        return false;
    }
    CHECK_AND_RETURN_RET(descs.front() != nullptr, false);
    if (descs.front()->deviceType_ == DEVICE_TYPE_SPEAKER) {
        return false;
    }
    AUDIO_INFO_LOG("Don't add ring ext device when ringer mode is not normal and no speaker added");
    return true;
}

RouterType AudioRouterCenter::GetBypassWithSco(AudioScene audioScene)
{
    RouterType bypassWithSco = RouterType::ROUTER_TYPE_NONE;
    if (audioScene == AUDIO_SCENE_DEFAULT && AudioDeviceManager::GetAudioDeviceManager().GetScoState()) {
        AUDIO_INFO_LOG("Audio scene default and sco state is true, bypassWithSco set to user select");
        bypassWithSco = RouterType::ROUTER_TYPE_USER_SELECT;
    }
    return bypassWithSco;
}

bool AudioRouterCenter::IsMediaFollowCallStrategy(AudioScene audioScene)
{
    if (audioScene == AUDIO_SCENE_PHONE_CALL) {
        return true;
    }
    if (audioScene == AUDIO_SCENE_PHONE_CHAT) {
        return true;
    }
    if ((audioScene == AUDIO_SCENE_RINGING || audioScene == AUDIO_SCENE_VOICE_RINGING) &&
        HasScoDevice()) {
        return true;
    }
    return false;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRouterCenter::FetchOutputDevicesInner(
    FetchDeviceInfo info, RouterType &routerType, const RouterType &bypassType,
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
{
    StreamUsage streamUsage = info.streamUsage;
    int32_t clientUID = info.clientUID;
    FetchDeviceInfo bak = {
        streamUsage, streamUsage, clientUID, routerType, PIPE_TYPE_NORMAL_OUT, PRIVACY_TYPE_PUBLIC
    };
    if (renderConfigMap_[streamUsage] == MEDIA_RENDER_ROUTERS ||
        renderConfigMap_[streamUsage] == TONE_RENDER_ROUTERS) {
        bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
        AudioScene audioScene = AudioSceneManager::GetInstance().GetAudioScene(hasSystemPermission);
        shared_ptr<AudioDeviceDescriptor> desc = make_shared<AudioDeviceDescriptor>();
        if (IsMediaFollowCallStrategy(audioScene)) {
            bak.streamUsage = AudioStreamCollector::GetAudioStreamCollector().GetLastestRunningCallStreamUsage();
            bak.streamUsage = (bak.streamUsage == STREAM_USAGE_UNKNOWN) ? STREAM_USAGE_VOICE_COMMUNICATION :
                bak.streamUsage;
            AUDIO_INFO_LOG("Media follow call strategy, replace usage %{public}d to %{public}d", streamUsage,
                bak.streamUsage);
            desc = FetchCallRenderDevice(bak.streamUsage, clientUID, routerType, bypassType,
                GetBypassWithSco(audioScene));
        } else {
            desc = FetchMediaRenderDevice(streamUsage, clientUID, routerType, bypassType);
        }
        descs.push_back(move(desc));
    } else if (renderConfigMap_[streamUsage] == RING_RENDER_ROUTERS) {
        DealRingRenderRouters(descs, bak, routerType);
    } else if (renderConfigMap_[streamUsage] == CALL_RENDER_ROUTERS) {
        descs.push_back(FetchCallRenderDevice(streamUsage, clientUID, routerType, bypassType));
    } else {
        AUDIO_INFO_LOG("streamUsage %{public}d didn't config router strategy, skipped", streamUsage);
        descs.push_back(make_shared<AudioDeviceDescriptor>());
        return descs;
    }
    if (audioDeviceRefinerCb_ != nullptr &&
        !NeedSkipSelectAudioOutputDeviceRefined(bak.streamUsage, descs)) {
        bak.routerType = routerType;
        audioDeviceRefinerCb_->OnAudioOutputDeviceRefined(descs, bak);
    }
    if (descs.size() > 0 && descs[0] != nullptr) {
        int32_t audioId_ = descs[0]->deviceId_;
        DeviceType type = descs[0]->deviceType_;
        descs[0]->routerType_ = routerType;
        HILOG_COMM_INFO("[%{public}s] usage:%{public}d uid:%{public}d size:[%{public}zu], 1st type:[%{public}d], "
            "id:[%{public}d], router:%{public}d ", info.caller.c_str(), streamUsage,
            clientUID, descs.size(), type, audioId_, routerType);
    }
    return descs;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRouterCenter::FetchOutputDevices(StreamUsage streamUsage,
    int32_t clientUID, std::string caller, const RouterType &bypassType, AudioPrivacyType privacyType)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    RouterType routerType = ROUTER_TYPE_NONE;
    int32_t zoneId = AudioZoneService::GetInstance().FindAudioZone(clientUID, STREAM_USAGE_INVALID);
    if (zoneId != 0) {
        vector<shared_ptr<AudioDeviceDescriptor>> zoneDescs =
            AudioZoneService::GetInstance().FetchOutputDevices(zoneId, streamUsage, clientUID, routerType);
        CHECK_AND_RETURN_RET(zoneDescs.size() == 0, zoneDescs);
    }
    if (streamUsage == STREAM_USAGE_ULTRASONIC &&
        AudioStreamCollector::GetAudioStreamCollector().GetRunningStreamUsageNoUltrasonic() == STREAM_USAGE_INVALID) {
        AUDIO_INFO_LOG("Stream ULTRASONIC always choose spk");
        descs.push_back(AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice());
        return descs;
    }
    FetchDeviceInfo info = {};
    info.streamUsage = streamUsage;
    info.clientUID = clientUID;
    info.caller = caller;
    info.privacyType = privacyType;
    return FetchOutputDevicesInner(info, routerType, bypassType, descs);
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRouterCenter::FetchDupDevices(
    const FetchDeviceInfo &fetchDeviceInfo)
{
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    RouterType routerType = ROUTER_TYPE_NONE;

    if (audioDeviceRefinerCb_ != nullptr) {
        FetchDeviceInfo info = {};
        info.streamUsage = fetchDeviceInfo.streamUsage;
        info.clientUID = fetchDeviceInfo.clientUID;
        info.routerType = ROUTER_TYPE_NONE;
        info.audioPipeType = PIPE_TYPE_NORMAL_OUT;
        info.privacyType = fetchDeviceInfo.privacyType;

        audioDeviceRefinerCb_->OnAudioDupDeviceRefined(descs, info);
    }

    return descs;
}

int32_t AudioRouterCenter::NotifyDistributedOutputChange(bool isRemote)
{
    CHECK_AND_RETURN_RET(audioDeviceRefinerCb_, SUCCESS);
    return audioDeviceRefinerCb_->OnDistributedOutputChange(isRemote);
}

void AudioRouterCenter::DealRingRenderRouters(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
    FetchDeviceInfo &info, RouterType &routerType)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    AudioScene audioScene = AudioSceneManager::GetInstance().GetAudioScene(hasSystemPermission);
    StreamUsage callStreamUsage =
                AudioStreamCollector::GetAudioStreamCollector().GetLastestRunningCallStreamUsage();
    bool isVoipStream = AudioStreamCollector::GetAudioStreamCollector().IsCallStreamUsage(callStreamUsage);
    AUDIO_INFO_LOG("ring render router streamUsage:%{public}d, audioScene:%{public}d, isVoipStream:%{public}d.",
        info.streamUsage, audioScene, isVoipStream);
    if (audioScene == AUDIO_SCENE_PHONE_CALL || audioScene == AUDIO_SCENE_PHONE_CHAT ||
        (audioScene == AUDIO_SCENE_VOICE_RINGING && isVoipStream)) {
        shared_ptr<AudioDeviceDescriptor> desc = make_shared<AudioDeviceDescriptor>();
        if (desc->deviceType_ == DEVICE_TYPE_NONE) {
            info.streamUsage = callStreamUsage;
            AUDIO_INFO_LOG("Ring follow call strategy, replace usage %{public}d to %{public}d",
                info.preStreamUsage, info.streamUsage);
            desc = FetchCallRenderDevice(info.streamUsage, info.clientUID, routerType);
        }
        descs.push_back(move(desc));
    } else if ((audioScene == AUDIO_SCENE_RINGING || audioScene == AUDIO_SCENE_VOICE_RINGING) &&
        info.streamUsage == STREAM_USAGE_ALARM) {
        AUDIO_INFO_LOG("alarm follow ring strategy, replace usage alarm to ringtone");
        descs = FetchRingRenderDevices(STREAM_USAGE_RINGTONE, info.clientUID, routerType);
    } else {
        descs = FetchRingRenderDevices(info.streamUsage, info.clientUID, routerType);
    }
}


bool AudioRouterCenter::IsConfigRouterStrategy(SourceType sourceType)
{
    if (capturerConfigMap_[sourceType] == "RecordCaptureRouters" ||
        capturerConfigMap_[sourceType] == "CallCaptureRouters" ||
        capturerConfigMap_[sourceType] == "VoiceMessages") {
        return true;
    }
    return false;
}

shared_ptr<AudioDeviceDescriptor> AudioRouterCenter::FetchCapturerInputDevice(SourceType sourceType,
    int32_t clientUID, RouterType &routerType, const uint32_t sessionID)
{
    bool hasSystemPermission = PermissionUtil::VerifySystemPermission();
    AudioScene audioScene = AudioSceneManager::GetInstance().GetAudioScene(hasSystemPermission);
    if (capturerConfigMap_[sourceType] == "RecordCaptureRouters") {
        if (audioScene != AUDIO_SCENE_DEFAULT) {
            return FetchCallCaptureDevice(sourceType, clientUID, routerType, sessionID);
        } else {
            return FetchRecordCaptureDevice(sourceType, clientUID, routerType, sessionID);
        }
    } else if (capturerConfigMap_[sourceType] == "CallCaptureRouters") {
        if (audioScene != AUDIO_SCENE_DEFAULT) {
            return FetchCallCaptureDevice(sourceType, clientUID, routerType, sessionID);
        } else {
            return FetchRecordCaptureDevice(sourceType, clientUID, routerType, sessionID);
        }
    } else if (capturerConfigMap_[sourceType] == "VoiceMessages") {
        if (audioScene != AUDIO_SCENE_DEFAULT) {
            return FetchCallCaptureDevice(sourceType, clientUID, routerType, sessionID);
        } else {
            return FetchVoiceMessageCaptureDevice(sourceType, clientUID, routerType, sessionID);
        }
    }
    return make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> AudioRouterCenter::FetchInputDevice(SourceType sourceType, int32_t clientUID,
    const uint32_t sessionID)
{
    shared_ptr<AudioDeviceDescriptor> desc = make_shared<AudioDeviceDescriptor>();
    RouterType routerType = ROUTER_TYPE_NONE;
    int32_t zoneId = AudioZoneService::GetInstance().FindAudioZoneByUid(clientUID);
    if (zoneId != 0) {
        AUDIO_INFO_LOG("FetchInputDevice zoneId %{public}d", zoneId);
        return AudioZoneService::GetInstance().FetchInputDevice(zoneId, sourceType, clientUID);
    }
    if (sourceType == SOURCE_TYPE_ULTRASONIC &&
        AudioStreamCollector::GetAudioStreamCollector().GetRunningSourceTypeNoUltrasonic() == SOURCE_TYPE_INVALID) {
        AUDIO_INFO_LOG("Source ULTRASONIC always choose mic");
        return AudioDeviceManager::GetAudioDeviceManager().GetCaptureDefaultDevice();
    }
    if (IsConfigRouterStrategy(sourceType)) {
        desc = FetchCapturerInputDevice(sourceType, clientUID, routerType, sessionID);
    } else {
        AUDIO_INFO_LOG("sourceType %{public}d didn't config router strategy, skipped", sourceType);
        return desc;
    }
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    if (audioDeviceRefinerCb_ != nullptr) {
        audioDeviceRefinerCb_->OnAudioInputDeviceRefined(descs, routerType, sourceType, clientUID, PIPE_TYPE_NORMAL_IN);
    }
    if (descs.size() > 0 && descs[0] != nullptr) {
        int32_t audioId_ = descs[0]->deviceId_;
        DeviceType type = descs[0]->deviceType_;
        AUDIO_PRERELEASE_LOGI("source:%{public}d uid:%{public}d fetch type:%{public}d id:%{public}d router:%{public}d",
            sourceType, clientUID, type, audioId_, routerType);
    }
    return move(descs[0]);
}

shared_ptr<AudioDeviceDescriptor> AudioRouterCenter::FetchCallCaptureDevice(SourceType sourceType,
    int32_t clientUID, RouterType &routerType, const uint32_t sessionID)
{
    for (auto &router : callCaptureRouters_) {
        shared_ptr<AudioDeviceDescriptor> desc = router->GetCallCaptureDevice(sourceType, clientUID, sessionID);
        if ((desc != nullptr) && (desc->deviceType_ != DEVICE_TYPE_NONE)) {
            routerType = router->GetRouterType();
            return desc;
        }
    }
    return make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> AudioRouterCenter::FetchRecordCaptureDevice(SourceType sourceType,
    int32_t clientUID, RouterType &routerType, const uint32_t sessionID)
{
    for (auto &router : recordCaptureRouters_) {
        shared_ptr<AudioDeviceDescriptor> desc = router->GetRecordCaptureDevice(sourceType, clientUID, sessionID);
        if (desc == nullptr) {
            continue;
        }
        if (desc->deviceType_ != DEVICE_TYPE_NONE) {
            routerType = router->GetRouterType();
            return desc;
        }
    }
    return make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> AudioRouterCenter::FetchVoiceMessageCaptureDevice(SourceType sourceType,
    int32_t clientUID, RouterType &routerType, const uint32_t sessionID)
{
    for (auto &router : voiceMessageRouters_) {
        shared_ptr<AudioDeviceDescriptor> desc = router->GetRecordCaptureDevice(sourceType, clientUID, sessionID);
        if ((desc != nullptr) && (desc->deviceType_ != DEVICE_TYPE_NONE)) {
            routerType = router->GetRouterType();
            return desc;
        }
    }
    return make_shared<AudioDeviceDescriptor>();
}

int32_t AudioRouterCenter::SetAudioDeviceRefinerCallback(const sptr<IRemoteObject> &object)
{
    sptr<IStandardAudioRoutingManagerListener> listener = iface_cast<IStandardAudioRoutingManagerListener>(object);
    if (listener != nullptr) {
        audioDeviceRefinerCb_ = listener;
        if (AudioCoreService::GetCoreService()->IsDistributeServiceOnline()) {
            AUDIO_INFO_LOG("distribute service online");
            listener->OnDistributedServiceOnline();
        }
        return SUCCESS;
    } else {
        return ERROR;
    }
}

int32_t AudioRouterCenter::UnsetAudioDeviceRefinerCallback()
{
    audioDeviceRefinerCb_ = nullptr;
    return SUCCESS;
}

bool AudioRouterCenter::isCallRenderRouter(StreamUsage streamUsage)
{
    return renderConfigMap_[streamUsage] == CALL_RENDER_ROUTERS;
}

int32_t AudioRouterCenter::GetSplitInfo(std::string &splitInfo)
{
    if (audioDeviceRefinerCb_ == nullptr) {
        AUDIO_INFO_LOG("nullptr");
        return ERROR;
    }

    return audioDeviceRefinerCb_->GetSplitInfoRefined(splitInfo);
}
} // namespace AudioStandard
} // namespace OHOS
