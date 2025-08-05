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
class AudioRendererChangeInfo : public Parcelable {
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
    mutable std::shared_ptr<AudioDeviceDescriptor::ClientInfo> clientInfo_ = nullptr;

    AudioRendererChangeInfo(const AudioRendererChangeInfo &audioRendererChangeInfo)
    {
        *this = audioRendererChangeInfo;
    }
    AudioRendererChangeInfo() = default;
    ~AudioRendererChangeInfo() = default;

    void SetClientInfo(std::shared_ptr<AudioDeviceDescriptor::ClientInfo> clientInfo) const
    {
        clientInfo_ = clientInfo;
        outputDeviceInfo.SetClientInfo(clientInfo);
    }

    bool Marshalling(Parcel &parcel) const override
    {
        int32_t clientUIDTemp = clientUID;
        RendererState rendererStateTemp = rendererState;
        if (clientInfo_ != nullptr) {
            clientUIDTemp = clientInfo_->hasSystemPermission_ ? clientUID : EMPTY_UID;
            rendererStateTemp = clientInfo_->hasSystemPermission_ ? rendererState : RENDERER_INVALID;
            clientInfo_ = nullptr;
        }
        return parcel.WriteInt32(createrUID)
            && parcel.WriteInt32(clientUIDTemp)
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
            && parcel.WriteInt32(static_cast<int32_t>(rendererStateTemp))
            && outputDeviceInfo.Marshalling(parcel)
            && parcel.WriteInt32(appVolume);
    }

    void UnmarshallingSelf(Parcel &parcel)
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
        rendererInfo.UnmarshallingSelf(parcel);
        rendererState = static_cast<RendererState>(parcel.ReadInt32());
        outputDeviceInfo.UnmarshallingSelf(parcel);
        appVolume = parcel.ReadInt32();
    }

    static AudioRendererChangeInfo *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) AudioRendererChangeInfo();
        if (info == nullptr) {
            return nullptr;
        }
        info->UnmarshallingSelf(parcel);
        return info;
    }
};

class AudioCapturerChangeInfo : public Parcelable {
public:
    int32_t createrUID;
    int32_t clientUID;
    int32_t sessionId;
    int32_t callerPid;
    int32_t clientPid;
    AudioCapturerInfo capturerInfo;
    CapturerState capturerState;
    AudioDeviceDescriptor inputDeviceInfo = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
    bool prerunningState = false;
    bool muted;
    uint32_t appTokenId;
    mutable std::shared_ptr<AudioDeviceDescriptor::ClientInfo> clientInfo_ = nullptr;

    AudioCapturerChangeInfo(const AudioCapturerChangeInfo &audioCapturerChangeInfo)
    {
        *this = audioCapturerChangeInfo;
    }
    AudioCapturerChangeInfo() = default;
    ~AudioCapturerChangeInfo() = default;

    void SetClientInfo(std::shared_ptr<AudioDeviceDescriptor::ClientInfo> clientInfo) const
    {
        clientInfo_ = clientInfo;
        inputDeviceInfo.SetClientInfo(clientInfo);
    }

    bool Marshalling(Parcel &parcel) const override
    {
        int32_t clientUIDTemp = clientUID;
        CapturerState capturerStateTemp = capturerState;
        if (clientInfo_ != nullptr) {
            clientUIDTemp = clientInfo_->hasSystemPermission_ ? clientUID : EMPTY_UID;
            capturerStateTemp = clientInfo_->hasSystemPermission_ ? capturerState : CAPTURER_INVALID;
            clientInfo_ = nullptr;
        }
        return parcel.WriteInt32(createrUID)
            && parcel.WriteInt32(clientUIDTemp)
            && parcel.WriteInt32(sessionId)
            && parcel.WriteInt32(callerPid)
            && parcel.WriteInt32(clientPid)
            && capturerInfo.Marshalling(parcel)
            && parcel.WriteInt32(static_cast<int32_t>(capturerStateTemp))
            && inputDeviceInfo.Marshalling(parcel)
            && parcel.WriteBool(muted)
            && parcel.WriteUint32(appTokenId);
    }

    void UnmarshallingSelf(Parcel &parcel)
    {
        createrUID = parcel.ReadInt32();
        clientUID = parcel.ReadInt32();
        sessionId = parcel.ReadInt32();
        callerPid = parcel.ReadInt32();
        clientPid = parcel.ReadInt32();
        capturerInfo.UnmarshallingSelf(parcel);
        capturerState = static_cast<CapturerState>(parcel.ReadInt32());
        inputDeviceInfo.UnmarshallingSelf(parcel);
        muted = parcel.ReadBool();
        appTokenId = parcel.ReadUint32();
    }

    static AudioCapturerChangeInfo *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) AudioCapturerChangeInfo();
        if (info == nullptr) {
            return nullptr;
        }
        info->UnmarshallingSelf(parcel);
        return info;
    }
};

struct AudioStreamChangeInfo : public Parcelable {
    AudioRendererChangeInfo audioRendererChangeInfo;
    AudioCapturerChangeInfo audioCapturerChangeInfo;

    bool Marshalling(Parcel &parcel) const override
    {
        return audioRendererChangeInfo.Marshalling(parcel)
            && audioCapturerChangeInfo.Marshalling(parcel);
    }

    static AudioStreamChangeInfo *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) AudioStreamChangeInfo();
        if (info == nullptr) {
            return nullptr;
        }
        info->audioRendererChangeInfo.UnmarshallingSelf(parcel);
        info->audioCapturerChangeInfo.UnmarshallingSelf(parcel);
        return info;
    }
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_STREAM_CHANGE_INFO_H