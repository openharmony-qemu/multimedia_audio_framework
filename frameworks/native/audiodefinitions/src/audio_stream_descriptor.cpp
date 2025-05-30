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

AudioStreamDescriptor::AudioStreamDescriptor()
{
}

AudioStreamDescriptor::~AudioStreamDescriptor()
{
}

bool AudioStreamDescriptor::Marshalling(Parcel &parcel) const
{
    return streamInfo_.Marshalling(parcel) &&
        parcel.WriteUint32(audioMode_) &&
        parcel.WriteUint32(audioFlag_) &&
        parcel.WriteUint32(routeFlag_) &&
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
        WriteDeviceDescVectorToParcel(parcel, oldDeviceDescs_) &&
        WriteDeviceDescVectorToParcel(parcel, newDeviceDescs_);
}

void AudioStreamDescriptor::Unmarshalling(Parcel &parcel)
{
    streamInfo_.Unmarshalling(parcel);
    audioMode_ = static_cast<AudioMode>(parcel.ReadUint32());
    audioFlag_ = static_cast<AudioFlag>(parcel.ReadUint32());
    routeFlag_ = static_cast<uint32_t>(parcel.ReadUint32());
    startTimeStamp_ = parcel.ReadInt64();
    rendererInfo_.Unmarshalling(parcel);
    capturerInfo_.Unmarshalling(parcel);
    appInfo_.appUid = parcel.ReadInt32();
    appInfo_.appTokenId = parcel.ReadUint32();
    appInfo_.appPid = parcel.ReadInt32();
    appInfo_.appFullTokenId = parcel.ReadUint64();
    sessionId_ = parcel.ReadUint32();
    callerUid_ = parcel.ReadInt32();
    callerPid_ = parcel.ReadInt32();
    streamAction_ = static_cast<AudioStreamAction>(parcel.ReadUint32());
    UnmarshallingDeviceDescVector(parcel, oldDeviceDescs_);
    UnmarshallingDeviceDescVector(parcel, newDeviceDescs_);
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
    int32_t size = 0;
    parcel.ReadInt32(size);
    if (size == -1 || size > MAX_STREAM_DESCRIPTORS_SIZE) {
        AUDIO_ERR_LOG("Invalid vector size");
        return;
    }
    for (int32_t i = 0; i < size; i++) {
        descs.push_back(AudioDeviceDescriptor::UnmarshallingPtr(parcel));
    }
}

void AudioStreamDescriptor::SetBunduleName(std::string &bundleName)
{
    AUDIO_INFO_LOG("Bundle name: %{public}s", bundleName.c_str());
    bundleName_ = bundleName;
}

void AudioStreamDescriptor::Dump(std::string &dumpString)
{
    AppendFormat(dumpString, "  Stream %d:\n", sessionId_);

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

    AppendFormat(dumpString, "    - CallerUid: %d\n", callerUid_);
    AppendFormat(dumpString, "    - CallerPid: %d\n", callerPid_);
    AppendFormat(dumpString, "    - AppUid: %d\n", appInfo_.appUid);
    AppendFormat(dumpString, "    - AppPid: %d\n", appInfo_.appPid);

    AppendFormat(dumpString, "    - SampleRate: %d\n", streamInfo_.samplingRate);
    AppendFormat(dumpString, "    - ChannelCount: %u\n", streamInfo_.channels);
    AppendFormat(dumpString, "    - ChannelLayout: %" PRIu64"\n", streamInfo_.channelLayout);
    AppendFormat(dumpString, "    - Format: %u\n", streamInfo_.format);
    AppendFormat(dumpString, "    - Encoding: %d\n", streamInfo_.encoding);

    AppendFormat(dumpString, "    - AudioFlag: 0x%x\n", audioFlag_);
    AppendFormat(dumpString, "    - RouteFlag: 0x%x\n", routeFlag_);
    AppendFormat(dumpString, "    - StartTimestamp: %" PRId64"\n", startTimeStamp_);
}

void AudioStreamDescriptor::DumpRendererStreamAttrs(std::string &dumpString)
{
    AppendFormat(dumpString, "    - StreamUsage: %d\n", rendererInfo_.streamUsage);
    AppendFormat(dumpString, "    - OriginalFlag: %d\n", rendererInfo_.originalFlag);
    AppendFormat(dumpString, "    - RendererFlag: %d\n", rendererInfo_.rendererFlags);
    AppendFormat(dumpString, "    - OffloadAllowed: %d\n", rendererInfo_.isOffloadAllowed);
    AppendFormat(dumpString, "    - PlayerType: %d\n", rendererInfo_.playerType);
}

void AudioStreamDescriptor::DumpCapturerStreamAttrs(std::string &dumpString)
{
    AppendFormat(dumpString, "    - SourceType: %d\n", capturerInfo_.sourceType);
    AppendFormat(dumpString, "    - OriginalFlag: %d\n", capturerInfo_.originalFlag);
    AppendFormat(dumpString, "    - RendererFlag: %d\n", capturerInfo_.capturerFlags);
    AppendFormat(dumpString, "    - RecorderType: %d\n", capturerInfo_.recorderType);
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

} // AudioStandard
} // namespace OHOS