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
#include "audio_common_log.h"

namespace OHOS {
namespace AudioStandard {
static const int32_t MAX_STREAM_DESCRIPTORS_SIZE = 1000;

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

void AudioStreamDescriptor::Dump(std::string &dumpString)
{
    dumpString += "\nstreamId: " + std::to_string(sessionId_) + "\t\tcallerUid: " + std::to_string(callerUid_);
    dumpString += "\nSampleRate: " + std::to_string(streamInfo_.samplingRate) +
        " Channels: " + std::to_string(streamInfo_.channels) + " Format: " + std::to_string(streamInfo_.format) +
        " Encoding: " + std::to_string(streamInfo_.encoding) +
        " ChannelLayout: " + std::to_string(streamInfo_.channelLayout);

    dumpString += "\naudioMode: ";
    if (audioMode_ == AUDIO_MODE_PLAYBACK) {
        dumpString += "PLAYBACK";
        dumpString += "\n\tstreamUsage: " + std::to_string(rendererInfo_.streamUsage);
        dumpString += "\n\toriginalFlag: " + std::to_string(rendererInfo_.originalFlag) +
            "rendererFlags: " + std::to_string(rendererInfo_.rendererFlags);
        dumpString += "\n\tisOffloadAllowed: ";
        dumpString += (rendererInfo_.isOffloadAllowed ? "true" : "false");
    } else {
        dumpString += "RECORD";
        dumpString += "\n\tsourceType: " + std::to_string(capturerInfo_.sourceType);
        dumpString += "\n\toriginalFlag: " + std::to_string(capturerInfo_.originalFlag) +
            "capturerFlags: " + std::to_string(capturerInfo_.capturerFlags);
    }

    dumpString += "\naudioFlag: " + std::to_string(audioFlag_);
    dumpString += "\nrouteFlag: " + std::to_string(routeFlag_);
    dumpString += "\nstartTimeStamp: " + std::to_string(startTimeStamp_);
    dumpString += "\nstreamStatus: " + std::to_string(streamStatus_);
    dumpString += "\nstreamAction: " + std::to_string(streamAction_);
    dumpString += "appUid: " + std::to_string(appInfo_.appUid) + "appTokenId: " + std::to_string(appInfo_.appTokenId) +
        "appPid: " + std::to_string(appInfo_.appPid) + "appFullTokenId: " + std::to_string(appInfo_.appFullTokenId);
    dumpString += "\noldDevices:\n";
    for (size_t idx = 0; idx < oldDeviceDescs_.size(); ++idx) {
        dumpString += "\t" + std::to_string(idx + 1) + ". ";
        oldDeviceDescs_[idx]->Dump(dumpString);
    }
    dumpString += "\nnewDevices:\n";
    for (size_t idx = 0; idx < newDeviceDescs_.size(); ++idx) {
        dumpString += "\t" + std::to_string(idx + 1) + ". ";
        newDeviceDescs_[idx]->Dump(dumpString);
    }
}

} // AudioStandard
} // namespace OHOS