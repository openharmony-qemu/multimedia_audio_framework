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

#include "boot_animation_state_info.h"
#include "audio_router_center.h"
#include "audio_policy_service.h"
#include "audio_zone_service.h"
#include "audio_scene_manager.h"
#include "parameters.h"
#ifdef FEATURE_DEVICE_MANAGER
#include "dm_device_info.h"
#endif

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B87
using namespace std;

namespace OHOS {
namespace AudioStandard {

constexpr const char *MULTI_STREAM_DEVICE_SUPPORT = "const.multimedia.audio.sys_multidevice_capability.enable";
static const bool IS_DEVICE_ENHANCED_SUPPORTED = OHOS::system::GetBoolParameter(MULTI_STREAM_DEVICE_SUPPORT, false);

const string MEDIA_RENDER_ROUTERS = "MediaRenderRouters";
const string CALL_RENDER_ROUTERS = "CallRenderRouters";
const string RECORD_CAPTURE_ROUTERS = "RecordCaptureRouters";
const string CALL_CAPTURE_ROUTERS = "CallCaptureRouters";
const string RING_RENDER_ROUTERS = "RingRenderRouters";
const string TONE_RENDER_ROUTERS = "ToneRenderRouters";

shared_ptr<AudioDeviceDescriptor> AudioRouterCenter::FetchMediaRenderDevice(
    StreamUsage streamUsage, int32_t clientUID, RouterType &routerType, const std::set<RouterType> &bypassTypeSet,
    const uint32_t streamId)
{
    for (auto &router : mediaRenderRouters_) {
        RouterType usedType = router->GetRouterType();
        if (bypassTypeSet.count(usedType)) {
            AUDIO_INFO_LOG("Fetch media render device bypass %{public}d", usedType);
            continue;
        }
        shared_ptr<AudioDeviceDescriptor> desc = router->GetMediaRenderDevice(streamUsage, clientUID, streamId);
        if ((desc != nullptr) && (desc->deviceType_ != DEVICE_TYPE_NONE)) {
            routerType = usedType;
            return desc;
        }
    }
    return make_shared<AudioDeviceDescriptor>();
}

shared_ptr<AudioDeviceDescriptor> AudioRouterCenter::FetchMediaRenderDevice(
    FetchDeviceInfo &info, RouterType &routerType, const RouterType &bypassType, const uint32_t streamId)
{
    if (vehiclePriority_.load()) {
        // fetch by higher priority router
        shared_ptr<AudioDeviceDescriptor> desc = FetchMediaRenderDevice(info.streamUsage, info.clientUID, routerType,
            {bypassType, ROUTER_TYPE_PRIVACY_PRIORITY}, streamId);
        static std::set<RouterType> higherRouterSet = {
            ROUTER_TYPE_APP_SELECT, ROUTER_TYPE_USER_SELECT, ROUTER_TYPE_PAIR_DEVICE};
        CHECK_AND_RETURN_RET(higherRouterSet.count(routerType) == 0, desc);
        RouterType lowerRouterType = routerType;

        // no privacy device or not wireless privacy device
        shared_ptr<AudioDeviceDescriptor> privacyDesc = FetchMediaRenderDevice(info.streamUsage, info.clientUID,
            mediaRenderRouters_, ROUTER_TYPE_PRIVACY_PRIORITY, streamId);
        CHECK_AND_RETURN_RET(privacyDesc!=nullptr && privacyDesc->deviceType_!=DEVICE_TYPE_NONE, desc);
        routerType = ROUTER_TYPE_PRIVACY_PRIORITY;
        CHECK_AND_RETURN_RET(IsWirelessPrivacyDevice(privacyDesc), privacyDesc);

        std::vector<std::shared_ptr<AudioDeviceDescriptor>> descs = {desc};
        if (audioDeviceRefinerCb_ != nullptr && !NeedSkipSelectAudioOutputDeviceRefined(info.streamUsage, descs)) {
            FetchDeviceInfo bak = {
                info.streamUsage, info.streamUsage, info.clientUID, lowerRouterType,
                PIPE_TYPE_OUT_NORMAL, info.privacyType
            };
            audioDeviceRefinerCb_->OnAudioOutputDeviceRefined(descs, bak);
        }
        CHECK_AND_RETURN_RET(!descs.empty() && descs.front()!=nullptr, privacyDesc);
        desc = descs.front();
        if (IsVehicleDevice(desc)) {
            routerType = ROUTER_TYPE_USER_SELECT;
            return desc;
        }

        return privacyDesc;
    } else {
        return FetchMediaRenderDevice(info.streamUsage, info.clientUID, routerType, {bypassType}, streamId);
    }
}

shared_ptr<AudioDeviceDescriptor> AudioRouterCenter::FetchMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID,
    std::vector<std::unique_ptr<RouterBase>> &routers, const RouterType routerType, const uint32_t streamId)
{
    auto it = std::find_if(routers.begin(), routers.end(), [routerType](const auto &router) {
        return router->GetRouterType() == routerType;
    });
    CHECK_AND_RETURN_RET_LOG(it != routers.end(), std::make_shared<AudioDeviceDescriptor>(), "miss target router");
    return (*it)->GetMediaRenderDevice(streamUsage, clientUID, streamId);
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
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO || desc->deviceType_ == DEVICE_TYPE_NEARLINK) {
            return true;
        }
    }

    vector<shared_ptr<AudioDeviceDescriptor>> publicDescs =
        AudioDeviceManager::GetAudioDeviceManager().GetCommRenderPublicDevices();
    for (auto &desc : publicDescs) {
        if ((desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO || desc->deviceType_ == DEVICE_TYPE_NEARLINK) &&
            desc->deviceCategory_ == BT_CAR) {
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

void AudioRouterCenter::PostProcessOutPutDeviceLog(std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs,
    FetchDeviceInfo &bak, RouterType &routerType, const FetchDeviceInfo &info)
{
    if (descs.size() > 0 && descs[0] != nullptr) {
        int32_t audioId = descs[0]->deviceId_;
        DeviceType type = descs[0]->deviceType_;
        descs[0]->routerType_ = routerType;
        HILOG_COMM_INFO("[FetchOutputDevicesInner][%{public}s] usage:%{public}d uid:%{public}d size:[%{public}zu], "
            "1st type:[%{public}d], id:[%{public}d], router:%{public}d ", info.caller.c_str(), bak.streamUsage,
            info.clientUID, descs.size(), type, audioId, routerType);
    }
}

std::shared_ptr<AudioDeviceDescriptor> AudioRouterCenter::MediaFollowRingStrategy(FetchDeviceInfo &info,
    StreamUsage ringStreamUsage, RouterType &routerType, const RouterType &bypassType, const uint32_t streamId)
{
    FetchDeviceInfo ringInfo = {
        ringStreamUsage, ringStreamUsage, info.clientUID, routerType, info.audioPipeType, info.privacyType
    };
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> ringDevices;
    DealRingRenderRouters(ringDevices, ringInfo, routerType);
    if (!ringDevices.empty() && ringDevices[0] != nullptr &&
        ringDevices[0]->deviceType_ != DEVICE_TYPE_NONE) {
        AUDIO_INFO_LOG("Media follow ring strategy, replace usage %{public}d to %{public}d",
            info.streamUsage, ringStreamUsage);
        return ringDevices[0];
    }
    AUDIO_INFO_LOG("Media follow ring strategy fallback");
    return FetchMediaRenderDevice(info, routerType, bypassType, streamId);
}

bool AudioRouterCenter::IsMediaFollowCallStrategy(AudioScene audioScene)
{
    if (IS_DEVICE_ENHANCED_SUPPORTED) {
        return false;
    }
    if (audioScene == AUDIO_SCENE_PHONE_CALL) {
        return true;
    }
    if (audioScene == AUDIO_SCENE_PHONE_CHAT) {
        return true;
    }
    return false;
}

bool AudioRouterCenter::IsAlarmFollowRingStrategy(AudioScene audioScene, StreamUsage streamUsage)
{
    if (IS_DEVICE_ENHANCED_SUPPORTED) {
        return false;
    }
    auto &streamCollector = AudioStreamCollector::GetAudioStreamCollector();
    const bool isRingScene = (audioScene == AUDIO_SCENE_RINGING || audioScene == AUDIO_SCENE_VOICE_RINGING);
    const bool isAlarmUsage = (streamUsage == STREAM_USAGE_ALARM);
    return isRingScene && isAlarmUsage;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRouterCenter::FetchOutputDevicesInner(FetchDeviceInfo &info)
{
    StreamUsage streamUsage = info.streamUsage;
    int32_t clientUID = info.clientUID;
    RouterType routerType = ROUTER_TYPE_NONE;
    vector<std::shared_ptr<AudioDeviceDescriptor>> descs;
    FetchDeviceInfo bak = info;
    bak.preStreamUsage = streamUsage;
    bak.audioPipeType = PIPE_TYPE_OUT_NORMAL;

    if (renderConfigMap_[streamUsage] == MEDIA_RENDER_ROUTERS ||
        renderConfigMap_[streamUsage] == TONE_RENDER_ROUTERS) {
        bool hasSystemPermission = (clientUID == BOOT_ANIMATION_UID || PermissionUtil::VerifySystemPermission());
        AudioPolicyDump::GetInstance().RecordBootStateTime(BootAnimationState::VERIFY_PERMISSION_IN_START, clientUID,
            ClockTime::GetCurMilli());
        AudioScene audioScene = AudioSceneManager::GetInstance().GetAudioScene(hasSystemPermission);
        shared_ptr<AudioDeviceDescriptor> desc = make_shared<AudioDeviceDescriptor>();
        if (IsMediaFollowCallStrategy(audioScene)) {
            bak.streamUsage = AudioPipeManager::GetPipeManager()->GetLastestRunningCallStreamUsage();
            bak.streamUsage = (bak.streamUsage == STREAM_USAGE_UNKNOWN) ? STREAM_USAGE_VOICE_COMMUNICATION :
                bak.streamUsage;
            AUDIO_INFO_LOG("Media follow call strategy, replace usage %{public}d to %{public}d", streamUsage,
                bak.streamUsage);
            desc = FetchCallRenderDevice(bak.streamUsage, clientUID, routerType, bak.bypassType,
                GetBypassWithSco(audioScene));
        } else if (audioScene == AUDIO_SCENE_RINGING || audioScene == AUDIO_SCENE_VOICE_RINGING) {
            StreamUsage ringStreamUsage = (audioScene == AUDIO_SCENE_RINGING) ?
                STREAM_USAGE_RINGTONE : STREAM_USAGE_VOICE_RINGTONE;
            desc = MediaFollowRingStrategy(bak, ringStreamUsage, routerType, bak.bypassType, info.streamId);
        } else {
            desc = FetchMediaRenderDevice(bak, routerType, bak.bypassType, info.streamId);
        }
        descs.push_back(move(desc));
    } else if (renderConfigMap_[streamUsage] == RING_RENDER_ROUTERS) {
        DealRingRenderRouters(descs, bak, routerType);
    } else if (renderConfigMap_[streamUsage] == CALL_RENDER_ROUTERS) {
        descs.push_back(FetchCallRenderDevice(streamUsage, clientUID, routerType, bak.bypassType));
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
    PostProcessOutPutDeviceLog(descs, bak, routerType, info);
    AudioPolicyDump::GetInstance().RecordBootStateTime(BootAnimationState::FETCH_OUTPUT_DEVICES, clientUID,
        ClockTime::GetCurMilli());
    return descs;
}

std::vector<std::shared_ptr<AudioDeviceDescriptor>> AudioRouterCenter::FetchOutputDevices(FetchDeviceInfo info)
{
    std::lock_guard<std::mutex> lock(routerMutex_);
    StartArbiterContext();
    
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    int32_t zoneId = AudioZoneService::GetInstance().FindAudioZone(info.clientUID, info.streamUsage, OUTPUT_DEVICE,
        info.streamId);
    if (zoneId != 0) {
        vector<shared_ptr<AudioDeviceDescriptor>> zoneDescs =
            AudioZoneService::GetInstance().FetchOutputDevices(zoneId, info.streamUsage,
                info.clientUID, ROUTER_TYPE_NONE);
        if (zoneDescs.size() != 0) {
            StopArbiterContext();
            return zoneDescs;
        }
    }
    if (info.streamUsage == STREAM_USAGE_ULTRASONIC &&
        AudioStreamCollector::GetAudioStreamCollector().GetRunningStreamUsageNoUltrasonic() == STREAM_USAGE_INVALID) {
        AUDIO_INFO_LOG("Stream ULTRASONIC always choose spk");
        descs.push_back(AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice());
        StopArbiterContext();
        return descs;
    }
    descs = FetchOutputDevicesInner(info);
    StopArbiterContext();
    return descs;
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
        info.audioPipeType = PIPE_TYPE_OUT_NORMAL;
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
                AudioPipeManager::GetPipeManager()->GetLastestRunningCallStreamUsage();
    bool isVoipStream = AudioStreamCollector::GetAudioStreamCollector().IsCallStreamUsage(callStreamUsage);
    AUDIO_INFO_LOG("ring render router streamUsage:%{public}d, audioScene:%{public}d, isVoipStream:%{public}d.",
        info.streamUsage, audioScene, isVoipStream);
    if (IS_DEVICE_ENHANCED_SUPPORTED) {
        descs = FetchRingRenderDevices(info.streamUsage, info.clientUID, routerType);
    } else if (audioScene == AUDIO_SCENE_PHONE_CALL || audioScene == AUDIO_SCENE_PHONE_CHAT ||
        (audioScene == AUDIO_SCENE_VOICE_RINGING && isVoipStream)) {
        shared_ptr<AudioDeviceDescriptor> desc = make_shared<AudioDeviceDescriptor>();
        if (desc->deviceType_ == DEVICE_TYPE_NONE) {
            info.streamUsage = callStreamUsage;
            AUDIO_INFO_LOG("Ring follow call strategy, replace usage %{public}d to %{public}d",
                info.preStreamUsage, info.streamUsage);
            desc = FetchCallRenderDevice(info.streamUsage, info.clientUID, routerType);
        }
        descs.push_back(move(desc));
    } else if (IsAlarmFollowRingStrategy(audioScene, info.streamUsage)) {
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
    if (IS_DEVICE_ENHANCED_SUPPORTED) {
        if (capturerConfigMap_[sourceType] == "RecordCaptureRouters") {
            return FetchRecordCaptureDevice(sourceType, clientUID, routerType, sessionID);
        } else if (capturerConfigMap_[sourceType] == "CallCaptureRouters") {
            return FetchCallCaptureDevice(sourceType, clientUID, routerType, sessionID);
        } else if (capturerConfigMap_[sourceType] == "VoiceMessages") {
            return FetchVoiceMessageCaptureDevice(sourceType, clientUID, routerType, sessionID);
        }
    }
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
    RouterType &routerType, const uint32_t sessionID)
{
    std::lock_guard<std::mutex> lock(routerMutex_);
    StartArbiterContext();
    
    shared_ptr<AudioDeviceDescriptor> desc = make_shared<AudioDeviceDescriptor>();
    RouterType innerRouterType = ROUTER_TYPE_NONE;
    int32_t zoneId = AudioZoneService::GetInstance().FindAudioZoneByUid(clientUID, INPUT_DEVICE);
    if (zoneId != 0) {
        AUDIO_INFO_LOG("FetchInputDevice zoneId %{public}d", zoneId);
        StopArbiterContext();
        shared_ptr<AudioDeviceDescriptor> zoneDesc =
            AudioZoneService::GetInstance().FetchInputDevice(zoneId, sourceType, clientUID);
        CHECK_AND_RETURN_RET(zoneDesc == nullptr, zoneDesc);
    }
    if (sourceType == SOURCE_TYPE_ULTRASONIC &&
        AudioStreamCollector::GetAudioStreamCollector().GetRunningSourceTypeNoUltrasonic() == SOURCE_TYPE_INVALID) {
        AUDIO_INFO_LOG("Source ULTRASONIC always choose mic");
        StopArbiterContext();
        return AudioDeviceManager::GetAudioDeviceManager().GetCaptureDefaultDevice();
    }
    if (IsConfigRouterStrategy(sourceType)) {
        desc = FetchCapturerInputDevice(sourceType, clientUID, innerRouterType, sessionID);
    } else {
        AUDIO_INFO_LOG("sourceType %{public}d didn't config router strategy, skipped", sourceType);
        StopArbiterContext();
        return desc;
    }
    routerType = innerRouterType;
    vector<shared_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(make_shared<AudioDeviceDescriptor>(*desc));
    if (audioDeviceRefinerCb_ != nullptr) {
        audioDeviceRefinerCb_->OnAudioInputDeviceRefined(descs, innerRouterType,
            sourceType, clientUID, PIPE_TYPE_IN_NORMAL);
    }
    if (descs.size() > 0 && descs[0] != nullptr) {
        int32_t audioId = descs[0]->deviceId_;
        DeviceType type = descs[0]->deviceType_;
        AUDIO_PRERELEASE_LOGI("source:%{public}d uid:%{public}d fetch type:%{public}d id:%{public}d router:%{public}d",
            sourceType, clientUID, type, audioId, innerRouterType);
    }
    StopArbiterContext();
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

void AudioRouterCenter::RegisterConflictHandlers(std::shared_ptr<ConflictHandler> handler)
{
    if (handler == nullptr) {
        return;
    }
    ForEachRouter([&handler](std::unique_ptr<RouterBase> &router) {
        router->RegisterConflictHandler(handler);
    });
}

void AudioRouterCenter::ClearConflictHandlers()
{
    ForEachRouter([](std::unique_ptr<RouterBase> &router) {
        router->ClearConflictHandler();
    });
}

int32_t AudioRouterCenter::SetSplitModeReady()
{
    CHECK_AND_RETURN_RET_LOG(audioDeviceRefinerCb_ != nullptr, ERROR, "nullptr");
    return audioDeviceRefinerCb_->SetSplitModeReadyRefined();
}

bool AudioRouterCenter::UpdateVehiclePriority(bool enable)
{
    CHECK_AND_RETURN_RET(enable != vehiclePriority_.load(), false);
    vehiclePriority_.store(enable);
    return true;
}

bool AudioRouterCenter::IsWirelessPrivacyDevice(shared_ptr<AudioDeviceDescriptor> desc)
{
    if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        switch (desc->deviceCategory_) {
            case BT_HEADPHONE:
            case BT_GLASSES:
                return true;
            default:
                return false;
        }
    }

    if (desc->deviceType_ == DEVICE_TYPE_NEARLINK) {
        switch (desc->deviceCategory_) {
            case BT_HEADPHONE:
            case BT_GLASSES:
                return true;
            default:
                return false;
        }
    }

    return false;
}

bool AudioRouterCenter::IsVehicleDevice(shared_ptr<AudioDeviceDescriptor> desc)
{
    if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_A2DP) {
        return desc->deviceCategory_ == BT_CAR;
    }

    if (desc->deviceType_ == DEVICE_TYPE_NEARLINK) {
        return desc->deviceCategory_ == BT_CAR;
    }

#ifdef FEATURE_DEVICE_MANAGER
    return desc->dmDeviceType_ == DistributedHardware::DEVICE_TYPE_CAR;
#endif
    return false;
}

void AudioRouterCenter::StartArbiterContext()
{
    ClearConflictHandlers();
    
    auto pipeManager = AudioPipeManager::GetPipeManager();
    CHECK_AND_RETURN(pipeManager != nullptr);
    auto allOutputStreams = pipeManager->GetAllOutputStreamDescs();
    auto allInputStreams = pipeManager->GetAllInputStreamDescs();
    
    std::vector<StreamDeviceInfo> outputDevices;
    for (auto &desc : allOutputStreams) {
        CHECK_AND_CONTINUE(desc != nullptr);
        StreamDeviceInfo info;
        info.uid = desc->GetRealUid();
        info.sessionId = desc->sessionId_;
        info.streamUsage = desc->rendererInfo_.streamUsage;
        info.isRunning = (desc->streamStatus_ == STREAM_STATUS_STARTED);
        
        FetchDeviceInfo fetchInfo(info.streamUsage, info.uid, "Arbiter");
        auto devices = FetchOutputDevicesInner(fetchInfo);
        if (!devices.empty()) {
            info.device = devices.front();
            info.routerType = fetchInfo.routerType;
        }
        outputDevices.push_back(info);
    }
    
    std::vector<StreamDeviceInfo> inputDevices;
    for (auto &desc : allInputStreams) {
        CHECK_AND_CONTINUE(desc != nullptr);
        StreamDeviceInfo info;
        info.uid = desc->GetRealUid();
        info.sessionId = desc->sessionId_;
        info.sourceType = desc->capturerInfo_.sourceType;
        info.isRunning = (desc->streamStatus_ == STREAM_STATUS_STARTED);
        
        RouterType routerType = ROUTER_TYPE_NONE;
        info.device = FetchCapturerInputDevice(info.sourceType, info.uid, routerType, info.sessionId);
        info.routerType = routerType;
        inputDevices.push_back(info);
    }
    
    if (wirelessConflictHandler_ != nullptr) {
        wirelessConflictHandler_->SetSelectedDevices(outputDevices, inputDevices);
        RegisterConflictHandlers(wirelessConflictHandler_);
    }
}

void AudioRouterCenter::StopArbiterContext()
{
    ClearConflictHandlers();
    if (wirelessConflictHandler_ != nullptr) {
        wirelessConflictHandler_->ClearSelectedDevices();
    }
}

} // namespace AudioStandard
} // namespace OHOS
