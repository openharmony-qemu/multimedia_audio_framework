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

#include "audio_stream_descriptor.h"

#include <cinttypes>
#include "audio_common_log.h"
#include "audio_utils.h"

namespace OHOS {
namespace AudioStandard {
static const int32_t MAX_STREAM_DESCRIPTORS_SIZE = 1000;
static constexpr int32_t MEDIA_SERVICE_UID = 1013;

static const char *StreamStatusToString(AudioStreamStatus status)
{
    switch (status) {
        case STREAM_STATUS_NEW:
            return "NEW";
        case STREAM_STATUS_STARTED:
            return "STARTED";
        case STREAM_STATUS_PAUSED:
            return "PAUSED";
        case STREAM_STATUS_STOPPED:
            return "STOPPED";
        case STREAM_STATUS_RELEASED:
            return "RELEASED";
        default:
            return "UNKNOWN";
    }
}

AudioStreamDescriptor::AudioStreamDescriptor(
    AudioStreamInfo streamInfo, AudioRendererInfo rendererInfo, AppInfo appInfo)
    : streamInfo_(streamInfo),
    audioMode_(AUDIO_MODE_PLAYBACK),
    rendererInfo_(rendererInfo),
    appInfo_(appInfo)
{
}

AudioStreamDescriptor::AudioStreamDescriptor(
    AudioStreamInfo streamInfo, AudioCapturerInfo capturerInfo, AppInfo appInfo)
    : streamInfo_(streamInfo),
    audioMode_(AUDIO_MODE_RECORD),
    capturerInfo_(capturerInfo),
    appInfo_(appInfo)
{
}

void AudioStreamDescriptor::CopyToStruct(AudioStreamDescriptor &streamDesc)
{
    streamDesc.streamInfo_ = streamInfo_;
    streamDesc.audioMode_ = audioMode_;
    streamDesc.audioFlag_ = audioFlag_;
    streamDesc.routeFlag_ = routeFlag_;
    streamDesc.oldRouteFlag_ = oldRouteFlag_;
    streamDesc.createTimeStamp_ = createTimeStamp_;
    streamDesc.startTimeStamp_ = startTimeStamp_;
    streamDesc.rendererInfo_ = rendererInfo_;
    streamDesc.capturerInfo_ = capturerInfo_;
    streamDesc.appInfo_ = appInfo_;
    streamDesc.sessionId_ = sessionId_;
    streamDesc.callerUid_ = callerUid_;
    streamDesc.callerPid_ = callerPid_;
    streamDesc.streamStatus_ = streamStatus_;
    streamDesc.streamAction_ = streamAction_;
    streamDesc.oldDeviceDescs_ = oldDeviceDescs_;
    streamDesc.newDeviceDescs_ = newDeviceDescs_;
    streamDesc.bundleName_ = bundleName_;
}

bool AudioStreamDescriptor::Marshalling(Parcel &parcel) const
{
    return streamInfo_.Marshalling(parcel) &&
        parcel.WriteUint32(audioMode_) &&
        parcel.WriteUint32(audioFlag_) &&
        parcel.WriteUint32(routeFlag_) &&
        parcel.WriteUint32(oldRouteFlag_) &&
        parcel.WriteInt64(createTimeStamp_) &&
        parcel.WriteInt64(startTimeStamp_) &&
        rendererInfo_.Marshalling(parcel) &&
        capturerInfo_.Marshalling(parcel) &&
        parcel.WriteInt32(appInfo_.appUid) &&
        parcel.WriteUint32(appInfo_.appTokenId) &&
        parcel.WriteInt32(appInfo_.appPid) &&
        parcel.WriteUint64(appInfo_.appFullTokenId) &&
        parcel.WriteUint32(sessionId_) &&
        parcel.WriteInt32(callerUid_) &&
        parcel.WriteInt32(callerPid_) &&
        parcel.WriteUint32(streamAction_) &&
        preferredInputDevice.Marshalling(parcel) &&
        WriteDeviceDescVectorToParcel(parcel, oldDeviceDescs_) &&
        WriteDeviceDescVectorToParcel(parcel, newDeviceDescs_) &&
        parcel.WriteInt32(oldOriginalFlag_);
}

AudioStreamDescriptor *AudioStreamDescriptor::Unmarshalling(Parcel &parcel)
{
    auto info = new(std::nothrow) AudioStreamDescriptor();
    if (info == nullptr) {
        return nullptr;
    }

    info->streamInfo_.UnmarshallingSelf(parcel);
    info->audioMode_ = static_cast<AudioMode>(parcel.ReadUint32());
    info->audioFlag_ = static_cast<AudioFlag>(parcel.ReadUint32());
    info->routeFlag_ = static_cast<uint32_t>(parcel.ReadUint32());
    info->oldRouteFlag_ = static_cast<uint32_t>(parcel.ReadUint32());
    info->createTimeStamp_ = parcel.ReadInt64();
    info->startTimeStamp_ = parcel.ReadInt64();
    info->rendererInfo_.UnmarshallingSelf(parcel);
    info->capturerInfo_.UnmarshallingSelf(parcel);
    info->appInfo_.appUid = parcel.ReadInt32();
    info->appInfo_.appTokenId = parcel.ReadUint32();
    info->appInfo_.appPid = parcel.ReadInt32();
    info->appInfo_.appFullTokenId = parcel.ReadUint64();
    info->sessionId_ = parcel.ReadUint32();
    info->callerUid_ = parcel.ReadInt32();
    info->callerPid_ = parcel.ReadInt32();
    info->streamAction_ = static_cast<AudioStreamAction>(parcel.ReadUint32());
    info->preferredInputDevice = std::shared_ptr<AudioDeviceDescriptor>(AudioDeviceDescriptor::Unmarshalling(parcel));
    info->UnmarshallingDeviceDescVector(parcel, info->oldDeviceDescs_);
    info->UnmarshallingDeviceDescVector(parcel, info->newDeviceDescs_);
    info->oldOriginalFlag_ = parcel.ReadInt32();

    return info;
}

bool AudioStreamDescriptor::WriteDeviceDescVectorToParcel(
    Parcel &parcel, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs) const
{
    size_t size = descs.size();
    if (size > MAX_STREAM_DESCRIPTORS_SIZE) {
        return parcel.WriteInt32(-1);
    }
    bool ret = parcel.WriteInt32(static_cast<int32_t>(size));
    CHECK_AND_RETURN_RET_LOG(ret, false, "write vector size failed");

    for (auto desc : descs) {
        ret = desc->Marshalling(parcel);
        CHECK_AND_RETURN_RET_LOG(ret, false, "Marshalling device desc failed");
    }
    return true;
}

void AudioStreamDescriptor::UnmarshallingDeviceDescVector(
    Parcel &parcel, std::vector<std::shared_ptr<AudioDeviceDescriptor>> &descs)
{
    int32_t size = parcel.ReadInt32();
    if (size == -1 || size > MAX_STREAM_DESCRIPTORS_SIZE) {
        AUDIO_ERR_LOG("Invalid vector size");
        return;
    }
    for (int32_t i = 0; i < size; i++) {
        descs.push_back(std::shared_ptr<AudioDeviceDescriptor>(AudioDeviceDescriptor::Unmarshalling(parcel)));
    }
}

void AudioStreamDescriptor::SetBunduleName(std::string &bundleName)
{
    AUDIO_INFO_LOG("Bundle name: %{public}s", bundleName.c_str());
    bundleName_ = bundleName;
}

void AudioStreamDescriptor::Dump(std::string &dumpString)
{
    AppendFormat(dumpString, "  Stream %u:\n", sessionId_);

    DumpCommonAttrs(dumpString);

    if (audioMode_ == AUDIO_MODE_PLAYBACK) {
        DumpRendererStreamAttrs(dumpString);
    } else {
        DumpCapturerStreamAttrs(dumpString);
    }

    DumpDeviceAttrs(dumpString);
}

void AudioStreamDescriptor::DumpCommonAttrs(std::string &dumpString)
{
    AppendFormat(dumpString, "    - StreamStatus: %u (%s)\n",
        streamStatus_, StreamStatusToString(streamStatus_));

    AppendFormat(dumpString, "    - CallerUid: %d CallerPid: %d AppUid: %d AppPid: %d\n",
        callerUid_, callerPid_, appInfo_.appUid, appInfo_.appPid);

    AppendFormat(dumpString, "    - SampleRate: %d ChannelCount: %u ChannelLayout: %" PRIu64"" \
        " Format: %u Encoding: %d\n",
        streamInfo_.samplingRate, streamInfo_.channels, streamInfo_.channelLayout,
        streamInfo_.format, streamInfo_.encoding);

    AppendFormat(dumpString, "    - AudioFlag: 0x%x RouteFlag: 0x%x OldRouteFlag: 0x%x\n",
        audioFlag_, routeFlag_, oldRouteFlag_);
    AppendFormat(dumpString, "    - CreateTimestamp: %" PRId64"\n", createTimeStamp_);
    AppendFormat(dumpString, "    - StartTimestamp: %" PRId64"\n", startTimeStamp_);
}

void AudioStreamDescriptor::DumpRendererStreamAttrs(std::string &dumpString)
{
    AppendFormat(dumpString, "    - StreamUsage: %d\n", rendererInfo_.streamUsage);
    AppendFormat(dumpString, "    - PlayerType: %d\n", rendererInfo_.playerType);
    AppendFormat(dumpString, "    - OriginalFlag: %d RendererFlags: %d\n",
        rendererInfo_.originalFlag, rendererInfo_.rendererFlags);
    AppendFormat(dumpString, "    - OffloadAllowed: %d\n", rendererInfo_.isOffloadAllowed);
}

void AudioStreamDescriptor::DumpCapturerStreamAttrs(std::string &dumpString)
{
    AppendFormat(dumpString, "    - SourceType: %d\n", capturerInfo_.sourceType);
    AppendFormat(dumpString, "    - RecorderType: %d\n", capturerInfo_.recorderType);
    AppendFormat(dumpString, "    - OriginalFlag: %d CapturerFlags: %d\n",
        capturerInfo_.originalFlag, capturerInfo_.capturerFlags);
}

void AudioStreamDescriptor::DumpDeviceAttrs(std::string &dumpString)
{
    AppendFormat(dumpString, "    - OldDevices:\n");
    for (auto &desc : oldDeviceDescs_) {
        if (desc != nullptr) {
            desc->Dump(dumpString);
        }
    }

    AppendFormat(dumpString, "    - NewDevices:\n");
    for (auto &desc : newDeviceDescs_) {
        if (desc != nullptr) {
            desc->Dump(dumpString);
        }
    }
}

std::string AudioStreamDescriptor::GetNewDevicesTypeString()
{
    std::string out = "";
    for (auto &desc : newDeviceDescs_) {
        if (desc != nullptr) {
            out += (desc->GetDeviceTypeString() + ":");
        }
    }
    return out;
}

std::string AudioStreamDescriptor::GetNewDupDevicesTypeString()
{
    std::string out = "";
    for (auto &desc : newDupDeviceDescs_) {
        if (desc != nullptr) {
            out += (desc->GetDeviceTypeString() + ":");
        }
    }
    return out;
}

std::string AudioStreamDescriptor::GetDeviceInfo(std::shared_ptr<AudioDeviceDescriptor> desc)
{
    CHECK_AND_RETURN_RET_LOG(desc != nullptr, "", "desc is nullptr");
    std::string out = "[";
    out.append(std::to_string(static_cast<uint32_t>(desc->deviceType_)));
    out.append(":" + std::to_string(static_cast<uint32_t>(desc->deviceId_)));
    return out + "]";
}

std::string AudioStreamDescriptor::GetNewDevicesInfo()
{
    std::string out = "";
    for (auto &desc : newDeviceDescs_) {
        CHECK_AND_CONTINUE(desc != nullptr);
        out += GetDeviceInfo(desc);
    }
    return out;
}

void AudioStreamDescriptor::ResetToNormalRoute(bool updateRoute)
{
    if (updateRoute) {
        oldRouteFlag_ = routeFlag_;
    }
    routeFlag_ = (IsPlayback() ? AUDIO_OUTPUT_FLAG_NORMAL : AUDIO_INPUT_FLAG_NORMAL);
}

int32_t AudioStreamDescriptor::GetRealUid() const
{
    if (callerUid_ == MEDIA_SERVICE_UID) {
        return appInfo_.appUid;
    }
    return callerUid_;
}
} // AudioStandard
} // namespace OHOS