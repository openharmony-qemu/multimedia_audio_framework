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

using namespace std;

namespace OHOS {
namespace AudioStandard {

const string MEDIA_RENDER_ROUTERS = "MediaRenderRouters";
const string CALL_RENDER_ROUTERS = "CallRenderRouters";
const string RECORD_CAPTURE_ROUTERS = "RecordCaptureRouters";
const string CALL_CAPTURE_ROUTERS = "CallCaptureRouters";
const string RING_RENDER_ROUTERS = "RingRenderRouters";
const string TONE_RENDER_ROUTERS = "ToneRenderRouters";

unique_ptr<AudioDeviceDescriptor> AudioRouterCenter::FetchMediaRenderDevice(
    StreamUsage streamUsage, int32_t clientUID, RouterType &routerType, const RouterType &bypassType)
{
    for (auto &router : mediaRenderRouters_) {
        if (router->GetRouterType() == bypassType) {
            AUDIO_INFO_LOG("Fetch media render device bypass %{public}d", bypassType);
            continue;
        }
        unique_ptr<AudioDeviceDescriptor> desc = router->GetMediaRenderDevice(streamUsage, clientUID);
        if (desc->deviceType_ != DEVICE_TYPE_NONE) {
            routerType = router->GetRouterType();
            return desc;
        }
    }
    return make_unique<AudioDeviceDescriptor>();
}

unique_ptr<AudioDeviceDescriptor> AudioRouterCenter::FetchCallRenderDevice(
    StreamUsage streamUsage, int32_t clientUID, RouterType &routerType, const RouterType &bypassType)
{
    for (auto &router : callRenderRouters_) {
        if (router->GetRouterType() == bypassType) {
            AUDIO_INFO_LOG("Fetch call render device bypass %{public}d", bypassType);
            continue;
        }
        unique_ptr<AudioDeviceDescriptor> desc = router->GetCallRenderDevice(streamUsage, clientUID);
        if (desc->deviceType_ != DEVICE_TYPE_NONE) {
            routerType = router->GetRouterType();
            return desc;
        }
    }
    return make_unique<AudioDeviceDescriptor>();
}

vector<unique_ptr<AudioDeviceDescriptor>> AudioRouterCenter::FetchRingRenderDevices(StreamUsage streamUsage,
    int32_t clientUID, RouterType &routerType)
{
    for (auto &router : ringRenderRouters_) {
        CHECK_AND_CONTINUE_LOG(router != nullptr, "Invalid router.");
        vector<unique_ptr<AudioDeviceDescriptor>> descs = router->GetRingRenderDevices(streamUsage, clientUID);
        CHECK_AND_CONTINUE_LOG(!descs.empty(), "FetchRingRenderDevices is empty.");
        if (descs.front() != nullptr && descs.front()->deviceType_ != DEVICE_TYPE_NONE) {
            AUDIO_INFO_LOG("RingRender streamUsage %{public}d clientUID %{public}d"
                " fetch descs front:%{public}d", streamUsage, clientUID, descs.front()->deviceType_);
            routerType = router->GetRouterType();
            return descs;
        }
    }
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    if (streamUsage == STREAM_USAGE_RINGTONE || streamUsage == STREAM_USAGE_VOICE_RINGTONE) {
        AudioRingerMode curRingerMode = AudioPolicyService::GetAudioPolicyService().GetRingerMode();
        if (curRingerMode == RINGER_MODE_NORMAL) {
            descs.push_back(AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice());
        } else {
            descs.push_back(make_unique<AudioDeviceDescriptor>());
        }
    } else {
        descs.push_back(AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice());
    }
    return descs;
}

bool AudioRouterCenter::HasScoDevice()
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs =
        AudioDeviceManager::GetAudioDeviceManager().GetCommRenderPrivacyDevices();
    for (auto &desc : descs) {
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO) {
            return true;
        }
    }

    vector<unique_ptr<AudioDeviceDescriptor>> publicDescs =
        AudioDeviceManager::GetAudioDeviceManager().GetCommRenderPublicDevices();
    for (auto &desc : publicDescs) {
        if (desc->deviceType_ == DEVICE_TYPE_BLUETOOTH_SCO && desc->deviceCategory_ == BT_CAR) {
            return true;
        }
    }
    return false;
}

std::vector<std::unique_ptr<AudioDeviceDescriptor>> AudioRouterCenter::FetchOutputDevices(StreamUsage streamUsage,
    int32_t clientUID, const RouterType &bypassType)
{
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    RouterType routerType = ROUTER_TYPE_NONE;
    if (streamUsage == STREAM_USAGE_ULTRASONIC &&
        AudioStreamCollector::GetAudioStreamCollector().GetRunningStreamUsageNoUltrasonic() == STREAM_USAGE_INVALID) {
        AUDIO_INFO_LOG("Stream ULTRASONIC always choose spk");
        descs.push_back(AudioDeviceManager::GetAudioDeviceManager().GetRenderDefaultDevice());
        return descs;
    }
    if (renderConfigMap_[streamUsage] == MEDIA_RENDER_ROUTERS ||
        renderConfigMap_[streamUsage] == TONE_RENDER_ROUTERS) {
        AudioScene audioScene = AudioPolicyService::GetAudioPolicyService().GetAudioScene();
        unique_ptr<AudioDeviceDescriptor> desc = make_unique<AudioDeviceDescriptor>();
        if (audioScene == AUDIO_SCENE_PHONE_CALL || audioScene == AUDIO_SCENE_PHONE_CHAT ||
            ((audioScene == AUDIO_SCENE_RINGING || audioScene == AUDIO_SCENE_VOICE_RINGING) && HasScoDevice())) {
            if (desc->deviceType_ == DEVICE_TYPE_NONE) {
                StreamUsage callStreamUsage =
                    AudioStreamCollector::GetAudioStreamCollector().GetLastestRunningCallStreamUsage();
                AUDIO_INFO_LOG("Media follow call strategy, replace usage %{public}d to %{public}d", streamUsage,
                    callStreamUsage);
                desc = FetchCallRenderDevice(callStreamUsage, clientUID, routerType, bypassType);
            }
        } else {
            desc = FetchMediaRenderDevice(streamUsage, clientUID, routerType, bypassType);
        }
        descs.push_back(move(desc));
    } else if (renderConfigMap_[streamUsage] == RING_RENDER_ROUTERS) {
        DealRingRenderRouters(descs, streamUsage, clientUID, routerType);
    } else if (renderConfigMap_[streamUsage] == CALL_RENDER_ROUTERS) {
        descs.push_back(FetchCallRenderDevice(streamUsage, clientUID, routerType, bypassType));
    } else {
        AUDIO_INFO_LOG("streamUsage %{public}d didn't config router strategy, skipped", streamUsage);
        descs.push_back(make_unique<AudioDeviceDescriptor>());
        return descs;
    }
    if (audioDeviceRefinerCb_ != nullptr) {
        audioDeviceRefinerCb_->OnAudioOutputDeviceRefined(descs, routerType,
            streamUsage, clientUID, PIPE_TYPE_NORMAL_OUT);
    }
    int32_t audioId_ = descs[0]->deviceId_;
    DeviceType type = descs[0]->deviceType_;
    AUDIO_PRERELEASE_LOGI("usage:%{public}d uid:%{public}d size:[%{public}zu], 1st type:[%{public}d], id:[%{public}d],"
        " router:%{public}d ", streamUsage, clientUID, descs.size(), type, audioId_, routerType);
    return descs;
}

void AudioRouterCenter::DealRingRenderRouters(std::vector<std::unique_ptr<AudioDeviceDescriptor>> &descs,
    StreamUsage streamUsage, int32_t clientUID, RouterType &routerType)
{
    AudioScene audioScene = AudioPolicyService::GetAudioPolicyService().GetAudioScene();
    StreamUsage callStreamUsage =
                AudioStreamCollector::GetAudioStreamCollector().GetLastestRunningCallStreamUsage();
    bool isVoipStream = AudioStreamCollector::GetAudioStreamCollector().IsCallStreamUsage(callStreamUsage);
    AUDIO_INFO_LOG("ring render router streamUsage:%{public}d, audioScene:%{public}d, isVoipStream:%{public}d.",
        streamUsage, audioScene, isVoipStream);
    if (audioScene == AUDIO_SCENE_PHONE_CALL || audioScene == AUDIO_SCENE_PHONE_CHAT ||
        (audioScene == AUDIO_SCENE_VOICE_RINGING && isVoipStream)) {
        unique_ptr<AudioDeviceDescriptor> desc = make_unique<AudioDeviceDescriptor>();
        if (desc->deviceType_ == DEVICE_TYPE_NONE) {
            AUDIO_INFO_LOG("Ring follow call strategy, replace usage %{public}d to %{public}d", streamUsage,
                callStreamUsage);
            desc = FetchCallRenderDevice(callStreamUsage, clientUID, routerType);
        }
        descs.push_back(move(desc));
    } else {
        descs = FetchRingRenderDevices(streamUsage, clientUID, routerType);
    }
}

unique_ptr<AudioDeviceDescriptor> AudioRouterCenter::FetchInputDevice(SourceType sourceType, int32_t clientUID)
{
    unique_ptr<AudioDeviceDescriptor> desc = make_unique<AudioDeviceDescriptor>();
    RouterType routerType = ROUTER_TYPE_NONE;
    if (sourceType == SOURCE_TYPE_ULTRASONIC &&
        AudioStreamCollector::GetAudioStreamCollector().GetRunningSourceTypeNoUltrasonic() == SOURCE_TYPE_INVALID) {
        AUDIO_INFO_LOG("Source ULTRASONIC always choose mic");
        return AudioDeviceManager::GetAudioDeviceManager().GetCaptureDefaultDevice();
    }
    if (capturerConfigMap_[sourceType] == "RecordCaptureRouters") {
        for (auto &router : recordCaptureRouters_) {
            desc = router->GetRecordCaptureDevice(sourceType, clientUID);
            if (desc->deviceType_ != DEVICE_TYPE_NONE) {
                routerType = router->GetRouterType();
                break;
            }
        }
    } else if (capturerConfigMap_[sourceType] == "CallCaptureRouters") {
        for (auto &router : callCaptureRouters_) {
            desc = router->GetCallCaptureDevice(sourceType, clientUID);
            if (desc->deviceType_ != DEVICE_TYPE_NONE) {
                routerType = router->GetRouterType();
                break;
            }
        }
    } else if (capturerConfigMap_[sourceType] == "VoiceMessages") {
        for (auto &router : voiceMessageRouters_) {
            desc = router->GetRecordCaptureDevice(sourceType, clientUID);
            if (desc->deviceType_ != DEVICE_TYPE_NONE) {
                routerType = router->GetRouterType();
                break;
            }
        }
    } else {
        AUDIO_INFO_LOG("sourceType %{public}d didn't config router strategy, skipped", sourceType);
        return desc;
    }
    vector<unique_ptr<AudioDeviceDescriptor>> descs;
    descs.push_back(make_unique<AudioDeviceDescriptor>(*desc));
    if (audioDeviceRefinerCb_ != nullptr) {
        audioDeviceRefinerCb_->OnAudioInputDeviceRefined(descs, routerType, sourceType, clientUID, PIPE_TYPE_NORMAL_IN);
    }
    int32_t audioId_ = descs[0]->deviceId_;
    DeviceType type = descs[0]->deviceType_;
    AUDIO_PRERELEASE_LOGI("source:%{public}d uid:%{public}d fetch type:%{public}d id:%{public}d router:%{public}d",
        sourceType, clientUID, type, audioId_, routerType);
    return move(descs[0]);
}

int32_t AudioRouterCenter::SetAudioDeviceRefinerCallback(const sptr<IRemoteObject> &object)
{
    sptr<IStandardAudioRoutingManagerListener> listener = iface_cast<IStandardAudioRoutingManagerListener>(object);
    if (listener != nullptr) {
        audioDeviceRefinerCb_ = listener;
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
} // namespace AudioStandard
} // namespace OHOS
