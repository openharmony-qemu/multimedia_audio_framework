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

#ifndef AUDIO_STREAM_CHANGE_INFO_H
#define AUDIO_STREAM_CHANGE_INFO_H

#include "audio_info.h"
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {
class AudioRendererChangeInfo {
public:
    int32_t createrUID;
    int32_t clientUID;
    int32_t sessionId;
    int32_t callerPid;
    int32_t clientPid;
    int32_t tokenId;
    int32_t channelCount;
    AudioRendererInfo rendererInfo;
    RendererState rendererState;
    AudioDeviceDescriptor outputDeviceInfo = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
    bool prerunningState = false;
    bool backMute = false;
    int32_t appVolume;

    AudioRendererChangeInfo(const AudioRendererChangeInfo &audioRendererChangeInfo)
    {
        *this = audioRendererChangeInfo;
    }
    AudioRendererChangeInfo() = default;
    ~AudioRendererChangeInfo() = default;
    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteInt32(createrUID)
            && parcel.WriteInt32(clientUID)
            && parcel.WriteInt32(sessionId)
            && parcel.WriteInt32(callerPid)
            && parcel.WriteInt32(clientPid)
            && parcel.WriteInt32(tokenId)
            && parcel.WriteInt32(channelCount)
            && parcel.WriteBool(backMute)
            && parcel.WriteInt32(static_cast<int32_t>(rendererInfo.contentType))
            && parcel.WriteInt32(static_cast<int32_t>(rendererInfo.streamUsage))
            && parcel.WriteInt32(rendererInfo.rendererFlags)
            && parcel.WriteInt32(rendererInfo.originalFlag)
            && parcel.WriteInt32(rendererInfo.samplingRate)
            && parcel.WriteInt32(rendererInfo.format)
            && rendererInfo.Marshalling(parcel)
            && parcel.WriteInt32(static_cast<int32_t>(rendererState))
            && outputDeviceInfo.Marshalling(parcel)
            && parcel.WriteInt32(appVolume);
    }
    bool Marshalling(Parcel &parcel, bool hasBTPermission, bool hasSystemPermission, int32_t apiVersion) const
    {
        return parcel.WriteInt32(createrUID)
            && parcel.WriteInt32(hasSystemPermission ? clientUID : EMPTY_UID)
            && parcel.WriteInt32(sessionId)
            && parcel.WriteInt32(callerPid)
            && parcel.WriteInt32(clientPid)
            && parcel.WriteInt32(tokenId)
            && parcel.WriteInt32(channelCount)
            && parcel.WriteBool(backMute)
            && parcel.WriteInt32(static_cast<int32_t>(rendererInfo.contentType))
            && parcel.WriteInt32(static_cast<int32_t>(rendererInfo.streamUsage))
            && parcel.WriteInt32(rendererInfo.rendererFlags)
            && parcel.WriteInt32(rendererInfo.originalFlag)
            && parcel.WriteInt32(rendererInfo.samplingRate)
            && parcel.WriteInt32(rendererInfo.format)
            && rendererInfo.Marshalling(parcel)
            && parcel.WriteInt32(hasSystemPermission ? static_cast<int32_t>(rendererState) :
                RENDERER_INVALID)
            && outputDeviceInfo.Marshalling(parcel, hasBTPermission, hasSystemPermission, apiVersion)
            && parcel.WriteInt32(appVolume);
    }
    void Unmarshalling(Parcel &parcel)
    {
        createrUID = parcel.ReadInt32();
        clientUID = parcel.ReadInt32();
        sessionId = parcel.ReadInt32();
        callerPid = parcel.ReadInt32();
        clientPid = parcel.ReadInt32();
        tokenId = parcel.ReadInt32();
        channelCount = parcel.ReadInt32();
        backMute = parcel.ReadBool();

        rendererInfo.contentType = static_cast<ContentType>(parcel.ReadInt32());
        rendererInfo.streamUsage = static_cast<StreamUsage>(parcel.ReadInt32());
        rendererInfo.rendererFlags = parcel.ReadInt32();
        rendererInfo.originalFlag = parcel.ReadInt32();
        rendererInfo.samplingRate = static_cast<AudioSamplingRate>(parcel.ReadInt32());
        rendererInfo.format = static_cast<AudioSampleFormat>(parcel.ReadInt32());
        rendererInfo.Unmarshalling(parcel);

        rendererState = static_cast<RendererState>(parcel.ReadInt32());
        outputDeviceInfo.Unmarshalling(parcel);
        appVolume = parcel.ReadInt32();
    }
};

class AudioCapturerChangeInfo {
public:
    int32_t createrUID;
    int32_t clientUID;
    int32_t sessionId;
    int32_t callerPid;
    int32_t clientPid;
    AudioCapturerInfo capturerInfo;
    CapturerState capturerState;
    AudioDeviceDescriptor inputDeviceInfo = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
    bool muted;
    uint32_t appTokenId;

    AudioCapturerChangeInfo(const AudioCapturerChangeInfo &audioCapturerChangeInfo)
    {
        *this = audioCapturerChangeInfo;
    }
    AudioCapturerChangeInfo() = default;
    ~AudioCapturerChangeInfo() = default;
    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteInt32(createrUID)
            && parcel.WriteInt32(clientUID)
            && parcel.WriteInt32(sessionId)
            && parcel.WriteInt32(callerPid)
            && parcel.WriteInt32(clientPid)
            && capturerInfo.Marshalling(parcel)
            && parcel.WriteInt32(static_cast<int32_t>(capturerState))
            && inputDeviceInfo.Marshalling(parcel)
            && parcel.WriteBool(muted)
            && parcel.WriteUint32(appTokenId);
    }

    bool Marshalling(Parcel &parcel, bool hasBTPermission, bool hasSystemPermission, int32_t apiVersion) const
    {
        return parcel.WriteInt32(createrUID)
            && parcel.WriteInt32(hasSystemPermission ? clientUID : EMPTY_UID)
            && parcel.WriteInt32(sessionId)
            && parcel.WriteInt32(callerPid)
            && parcel.WriteInt32(clientPid)
            && capturerInfo.Marshalling(parcel)
            && parcel.WriteInt32(hasSystemPermission ? static_cast<int32_t>(capturerState) : CAPTURER_INVALID)
            && inputDeviceInfo.Marshalling(parcel, hasBTPermission, hasSystemPermission, apiVersion)
            && parcel.WriteBool(muted)
            && parcel.WriteUint32(appTokenId);
    }

    void Unmarshalling(Parcel &parcel)
    {
        createrUID = parcel.ReadInt32();
        clientUID = parcel.ReadInt32();
        sessionId = parcel.ReadInt32();
        callerPid = parcel.ReadInt32();
        clientPid = parcel.ReadInt32();
        capturerInfo.Unmarshalling(parcel);
        capturerState = static_cast<CapturerState>(parcel.ReadInt32());
        inputDeviceInfo.Unmarshalling(parcel);
        muted = parcel.ReadBool();
        appTokenId = parcel.ReadUint32();
    }
};

struct AudioStreamChangeInfo {
    AudioRendererChangeInfo audioRendererChangeInfo;
    AudioCapturerChangeInfo audioCapturerChangeInfo;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_STREAM_CHANGE_INFO_H