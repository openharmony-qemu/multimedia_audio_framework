/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#ifndef AUDIO_DEBUG_INFO_H
#define AUDIO_DEBUG_INFO_H

#include <cstdint>
#include <memory>
#include <new>
#include <string>

#include "parcel.h"
#include "refbase.h"
#include "audio_info.h"
#include "audio_effect.h"
#include "audio_stream_descriptor.h"
#include "timestamp.h"
#include "audio_device_descriptor.h"

namespace OHOS {
namespace AudioStandard {
#define PIPE_INVALID_ID 0xFFFFFFFF

enum class AudioSessionState {
    SESSION_INVALID = -1,
    SESSION_NEW = 0,
    SESSION_ACTIVE = 1,
    SESSION_DEACTIVE = 2,
    SESSION_RELEASED = 3,
};

struct AudioRendererDebugInfo {
    uint32_t sessionId = 0;
    int32_t samplingRate = 0;
    AudioChannel channels = AudioChannel::CHANNEL_UNKNOW;
    AudioSampleFormat format = AudioSampleFormat::INVALID_WIDTH;
    AudioEncodingType encoding = AudioEncodingType::ENCODING_PCM;
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    int32_t rendererFlags = AUDIO_FLAG_NORMAL;
    float speed = 1.0f;
    float pitch = 1.0f; // Audio pitch debug is not implemented currently, keep default value.

    AudioEffectMode effectMode = EFFECT_NONE;

    uint32_t pipeId = PIPE_INVALID_ID;
    uint32_t paIndex = 0;
    AudioPipeRole pipeRole = PIPE_ROLE_OUTPUT;
    std::string pipeName = "undefine";
    uint32_t routeFlag = 0;
    std::string adapterName = "";
    std::string moduleName = "";
    std::string pipeFormat = "";
    std::string pipeRate = "";
    std::string pipeChannels = "";
    std::string pipeChannelLayout = "";
    std::string pipeDeviceType = "";
    DeviceType mainDeviceType = DEVICE_TYPE_NONE;
    std::string networkId = "";
    std::string macAddress = "";

    AudioStreamType volumeType = STREAM_DEFAULT;
    float streamVolume = 0.0f;
    float systemVolume = 0.0f;
    float volume = 0.0f;

    int32_t hintType = -1;
    std::string historyHintType = "";
};

struct AudioCapturerDebugInfo {
    uint32_t sessionId = 0;
    int32_t samplingRate = 0;
    AudioChannel channels = AudioChannel::CHANNEL_UNKNOW;
    AudioSampleFormat format = AudioSampleFormat::INVALID_WIDTH;
    AudioEncodingType encoding = AudioEncodingType::ENCODING_PCM;
    AudioChannelLayout channelLayout = AudioChannelLayout::CH_LAYOUT_UNKNOWN;
    SourceType sourceType = AudioCapturerInfo {}.sourceType;
    int32_t capturerFlag = AudioCapturerInfo {}.capturerFlags;
    Timestamp captureTimestamp;
    uint64_t bufferSize = 0;
    uint32_t overflowCount = 0;
    bool muteWhenInterrupted = false;
    AudioDeviceDescriptor inputDeviceInfo = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);
    AudioPipeRole pipeRole = PIPE_ROLE_INPUT;
    AudioStreamInfo pipeStreamInfo = AudioStreamInfo(SAMPLE_RATE_8000, AudioEncodingType::ENCODING_PCM,
        AudioSampleFormat::INVALID_WIDTH, AudioChannel::CHANNEL_UNKNOW, AudioChannelLayout::CH_LAYOUT_UNKNOWN);
};

struct AudioLoopbackDebugInfo {
    // Application information
    int32_t appUid = -1;
    std::string appName;

    // Loopback status
    AudioLoopbackMode mode = LOOPBACK_HARDWARE;
    AudioLoopbackState currentState = LOOPBACK_STATE_IDLE;

    // Device information
    DeviceType activeOutputDevice = DEVICE_TYPE_NONE;
    DeviceType activeInputDevice = DEVICE_TYPE_NONE;

    // Audio effect information
    AudioLoopbackReverbPreset reverbPreset = REVERB_PRESET_THEATER;
    AudioLoopbackEqualizerPreset equalizerPreset = EQUALIZER_PRESET_FULL;
    int32_t volume = 0;

    // Stream states
    CapturerState uplinkStreamState = CAPTURER_INVALID;
    RendererState downlinkStreamState = RENDERER_INVALID;
};

struct AudioStreamDebugInfo : public Parcelable {
    uint32_t streamId = 0;
    AudioStreamType streamType = STREAM_DEFAULT;
    AudioFocuState focusState = ACTIVE;

    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteUint32(streamId) &&
            parcel.WriteInt32(static_cast<int32_t>(streamType)) &&
            parcel.WriteInt32(static_cast<int32_t>(focusState));
    }

    static AudioStreamDebugInfo *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) AudioStreamDebugInfo();
        if (info == nullptr) {
            return nullptr;
        }
        info->streamId = parcel.ReadUint32();
        info->streamType = static_cast<AudioStreamType>(parcel.ReadInt32());
        info->focusState = static_cast<AudioFocuState>(parcel.ReadInt32());
        return info;
    }
};

struct AudioSessionDebugInfo : public Parcelable {
    AudioSessionStrategy strategy {AudioConcurrencyMode::INVALID};
    AudioSessionScene audioSessionScene {AudioSessionScene::INVALID};
    DeviceType defaultDeviceType = DEVICE_TYPE_INVALID;
    AudioSessionState state = AudioSessionState::SESSION_INVALID;
    AudioFocuState fakeFocusState = ACTIVE;
    int32_t pid { -1 };
    int32_t uid { -1 };
    std::vector<AudioStreamDebugInfo> streamInfos;

    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteInt32(static_cast<int32_t>(strategy.concurrencyMode)) &&
            parcel.WriteInt32(static_cast<int32_t>(audioSessionScene)) &&
            parcel.WriteInt32(static_cast<int32_t>(defaultDeviceType)) &&
            parcel.WriteInt32(static_cast<int32_t>(state)) &&
            parcel.WriteInt32(static_cast<int32_t>(fakeFocusState)) &&
            parcel.WriteInt32(pid) &&
            parcel.WriteInt32(uid) &&
            parcel.WriteUint32(static_cast<uint32_t>(streamInfos.size())) &&
            [&parcel, this]() -> bool {
                for (const auto &item : streamInfos) {
                    if (!item.Marshalling(parcel)) {
                        return false;
                    }
                }
                return true;
            }();
    }

    static AudioSessionDebugInfo *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) AudioSessionDebugInfo();
        if (info == nullptr) {
            return nullptr;
        }
        info->strategy.concurrencyMode = static_cast<AudioConcurrencyMode>(parcel.ReadInt32());
        info->audioSessionScene = static_cast<AudioSessionScene>(parcel.ReadInt32());
        info->defaultDeviceType = static_cast<DeviceType>(parcel.ReadInt32());
        info->state = static_cast<AudioSessionState>(parcel.ReadInt32());
        info->fakeFocusState = static_cast<AudioFocuState>(parcel.ReadInt32());
        info->pid = parcel.ReadInt32();
        info->uid = parcel.ReadInt32();
        uint32_t streamsSize = parcel.ReadUint32();
        for (uint32_t i = 0; i < streamsSize; i++) {
            std::unique_ptr<AudioStreamDebugInfo> streamInfo(AudioStreamDebugInfo::Unmarshalling(parcel));
            if (streamInfo == nullptr) {
                delete info;
                return nullptr;
            }
            info->streamInfos.push_back(*streamInfo);
        }
        return info;
    }
};

struct AudioDebugInfo : public Parcelable {
    uint32_t sessionId = 0;
    AudioRendererInfo rendererInfo = AudioRendererInfo {};
    AudioStreamParams streamParams = AudioStreamParams {};
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();

    // debug info about pipe
    std::shared_ptr<AudioStreamInfo> pipeStreamInfo = std::make_shared<AudioStreamInfo>(SAMPLE_RATE_8000,
        AudioEncodingType::ENCODING_PCM, AudioSampleFormat::INVALID_WIDTH, AudioChannel::CHANNEL_UNKNOW,
        AudioChannelLayout::CH_LAYOUT_UNKNOWN);
    uint32_t pipeId = PIPE_INVALID_ID;
    uint32_t paIndex = 0;
    AudioPipeRole pipeRole = PIPE_ROLE_OUTPUT;
    std::string pipeName = "undefine";
    uint32_t routeFlag = 0;
    std::string adapterName = "";
    std::string moduleName = "";
    std::string pipeFormat = "";
    std::string pipeRate = "";
    std::string pipeChannels = "";
    std::string pipeChannelLayout = "";
    std::string pipeDeviceType = "";
    std::string pipeClassName = "";

    // debug info about device
    DeviceType mainDeviceType = DEVICE_TYPE_NONE;
    std::string networkId = "";
    std::string macAddress = "";

    // debug info about volume
    AudioStreamType volumeType = STREAM_DEFAULT;
    float streamVolume = 0.0f;
    float systemVolume = 0.0f;
    float volume = 0.0f;

    // debug info about renderer
    float speed = 1.0f;
    float pitch = 1.0f; // Audio pitch debug is not implemented currently, keep default value.
    AudioEffectMode effectMode = EFFECT_NONE;

    // debug info about Interrupt
    AudioFocuState focusState = ACTIVE;
    std::vector<AudioFocusBehavior> focusHistory;

    // debug info about capturer
    std::shared_ptr<AudioCapturerInfo> capturerInfo = std::make_shared<AudioCapturerInfo>();
    Timestamp captureTimestamp;
    uint64_t captureBufferSize = 0;
    uint32_t captureOverflowCount = 0;
    bool muteWhenInterrupted = false;
    AudioDeviceDescriptor inputDeviceInfo = AudioDeviceDescriptor(AudioDeviceDescriptor::DEVICE_INFO);

    // debug info about loopback
    int32_t loopbackStatus = -1;
    int32_t loopbackMode = -1;
    DeviceType loopbackOutputDeviceType = DEVICE_TYPE_NONE;
    DeviceType loopbackInputDeviceType = DEVICE_TYPE_NONE;

    bool MarshallingStreamParams(Parcel &parcel) const
    {
        return parcel.WriteUint32(streamParams.samplingRate) &&
            parcel.WriteUint8(streamParams.encoding) &&
            parcel.WriteUint8(streamParams.format) &&
            parcel.WriteUint8(streamParams.channels) &&
            parcel.WriteUint64(streamParams.channelLayout) &&
            parcel.WriteUint32(streamParams.originalSessionId) &&
            parcel.WriteUint32(streamParams.customSampleRate) &&
            parcel.WriteBool(streamParams.isRemoteSpatialChannel) &&
            parcel.WriteUint64(streamParams.remoteChannelLayout) &&
            parcel.WriteUint32(streamParams.ecSamplingRate) &&
            parcel.WriteUint8(streamParams.ecEncoding) &&
            parcel.WriteUint8(streamParams.ecFormat) &&
            parcel.WriteUint8(streamParams.ecChannels) &&
            parcel.WriteUint64(streamParams.ecChannelLayout) &&
            parcel.WriteUint32(streamParams.micInSamplingRate) &&
            parcel.WriteUint8(streamParams.micInEncoding) &&
            parcel.WriteUint8(streamParams.micInFormat) &&
            parcel.WriteUint8(streamParams.micInChannels) &&
            parcel.WriteUint64(streamParams.micInChannelLayout) &&
            parcel.WriteUint8(streamParams.ultraFastFlag);
    }

    bool MarshallingPipeInfo(Parcel &parcel) const
    {
        return pipeStreamInfo->Marshalling(parcel) &&
            parcel.WriteUint32(pipeId) &&
            parcel.WriteUint32(paIndex) &&
            parcel.WriteInt32(static_cast<int32_t>(pipeRole)) &&
            parcel.WriteString(pipeName) &&
            parcel.WriteUint32(routeFlag) &&
            parcel.WriteString(adapterName) &&
            parcel.WriteString(moduleName) &&
            parcel.WriteString(pipeFormat) &&
            parcel.WriteString(pipeRate) &&
            parcel.WriteString(pipeChannels) &&
            parcel.WriteString(pipeChannelLayout) &&
            parcel.WriteString(pipeDeviceType) &&
            parcel.WriteString(pipeClassName);
    }

    bool MarshallingDeviceInfo(Parcel &parcel) const
    {
        return parcel.WriteInt32(static_cast<int32_t>(mainDeviceType)) &&
            parcel.WriteString(networkId) &&
            parcel.WriteString(macAddress);
    }

    bool MarshallingVolumeInfo(Parcel &parcel) const
    {
        return parcel.WriteInt32(static_cast<int32_t>(volumeType)) &&
            parcel.WriteFloat(streamVolume) &&
            parcel.WriteFloat(systemVolume) &&
            parcel.WriteFloat(volume) &&
            parcel.WriteFloat(speed) &&
            parcel.WriteFloat(pitch) &&
            parcel.WriteInt32(static_cast<int32_t>(effectMode));
    }

    bool MarshallingInterruptInfo(Parcel &parcel) const
    {
        return parcel.WriteInt32(static_cast<int32_t>(focusState)) &&
            parcel.WriteUint32(static_cast<uint32_t>(focusHistory.size())) &&
            [&parcel, this]() -> bool {
                for (const auto &item : focusHistory) {
                    if (!parcel.WriteUint32(item.streamId) ||
                        !parcel.WriteInt32(item.pid) ||
                        !parcel.WriteInt32(item.uid) ||
                        !parcel.WriteInt32(static_cast<int32_t>(item.streamType)) ||
                        !parcel.WriteInt32(static_cast<int32_t>(item.sourceType)) ||
                        !parcel.WriteInt32(static_cast<int32_t>(item.hintType)) ||
                        !parcel.WriteInt64(item.timestamp)) {
                        return false;
                    }
                }
                return true;
            }();
    }

    bool MarshallingCapturerInfo(Parcel &parcel) const
    {
        return capturerInfo->Marshalling(parcel) &&
            parcel.WriteInt32(loopbackStatus) &&
            parcel.WriteInt32(loopbackMode) &&
            parcel.WriteInt32(static_cast<int32_t>(loopbackOutputDeviceType)) &&
            parcel.WriteInt32(static_cast<int32_t>(loopbackInputDeviceType)) &&
            parcel.WriteInt64(captureTimestamp.framePosition) &&
            parcel.WriteInt64(static_cast<int64_t>(captureTimestamp.time.tv_sec)) &&
            parcel.WriteInt64(static_cast<int64_t>(captureTimestamp.time.tv_nsec)) &&
            parcel.WriteUint64(captureBufferSize) &&
            parcel.WriteUint32(captureOverflowCount) &&
            parcel.WriteBool(muteWhenInterrupted) &&
            inputDeviceInfo.Marshalling(parcel);
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteUint32(sessionId) &&
            rendererInfo.Marshalling(parcel) &&
            MarshallingStreamParams(parcel) &&
            streamDesc->Marshalling(parcel) &&
            MarshallingPipeInfo(parcel) &&
            MarshallingDeviceInfo(parcel) &&
            MarshallingVolumeInfo(parcel) &&
            MarshallingInterruptInfo(parcel) &&
            MarshallingCapturerInfo(parcel);
    }

    bool UnmarshallingStreamParams(Parcel &parcel)
    {
        streamParams.samplingRate = parcel.ReadUint32();
        streamParams.encoding = parcel.ReadUint8();
        streamParams.format = parcel.ReadUint8();
        streamParams.channels = parcel.ReadUint8();
        streamParams.channelLayout = parcel.ReadUint64();
        streamParams.originalSessionId = parcel.ReadUint32();
        streamParams.customSampleRate = parcel.ReadUint32();
        streamParams.isRemoteSpatialChannel = parcel.ReadBool();
        streamParams.remoteChannelLayout = parcel.ReadUint64();
        streamParams.ecSamplingRate = parcel.ReadUint32();
        streamParams.ecEncoding = parcel.ReadUint8();
        streamParams.ecFormat = parcel.ReadUint8();
        streamParams.ecChannels = parcel.ReadUint8();
        streamParams.ecChannelLayout = parcel.ReadUint64();
        streamParams.micInSamplingRate = parcel.ReadUint32();
        streamParams.micInEncoding = parcel.ReadUint8();
        streamParams.micInFormat = parcel.ReadUint8();
        streamParams.micInChannels = parcel.ReadUint8();
        streamParams.micInChannelLayout = parcel.ReadUint64();
        streamParams.ultraFastFlag = parcel.ReadUint8();
        return true;
    }

    bool UnmarshallingPipeInfo(Parcel &parcel)
    {
        std::unique_ptr<AudioStreamInfo> pipeStreamInfoPtr(AudioStreamInfo::Unmarshalling(parcel));
        if (pipeStreamInfoPtr == nullptr) {
            return false;
        }
        pipeStreamInfo = std::move(pipeStreamInfoPtr);

        pipeId = parcel.ReadUint32();
        paIndex = parcel.ReadUint32();
        pipeRole = static_cast<AudioPipeRole>(parcel.ReadInt32());
        pipeName = parcel.ReadString();
        routeFlag = parcel.ReadUint32();
        adapterName = parcel.ReadString();
        moduleName = parcel.ReadString();
        pipeFormat = parcel.ReadString();
        pipeRate = parcel.ReadString();
        pipeChannels = parcel.ReadString();
        pipeChannelLayout = parcel.ReadString();
        pipeDeviceType = parcel.ReadString();
        pipeClassName = parcel.ReadString();
        return true;
    }

    bool UnmarshallingDeviceInfo(Parcel &parcel)
    {
        mainDeviceType = static_cast<DeviceType>(parcel.ReadInt32());
        networkId = parcel.ReadString();
        macAddress = parcel.ReadString();
        return true;
    }

    bool UnmarshallingVolumeInfo(Parcel &parcel)
    {
        volumeType = static_cast<AudioStreamType>(parcel.ReadInt32());
        streamVolume = parcel.ReadFloat();
        systemVolume = parcel.ReadFloat();
        volume = parcel.ReadFloat();
        speed = parcel.ReadFloat();
        pitch = parcel.ReadFloat();
        effectMode = static_cast<AudioEffectMode>(parcel.ReadInt32());
        return true;
    }
     
    bool UnmarshallingInterruptInfo(Parcel &parcel)
    {
        focusState = static_cast<AudioFocuState>(parcel.ReadInt32());
        uint32_t focusHistorySize = parcel.ReadUint32();
        for (uint32_t i = 0; i < focusHistorySize; i++) {
            AudioFocusBehavior item;
            item.streamId = parcel.ReadUint32();
            item.pid = parcel.ReadInt32();
            item.uid = parcel.ReadInt32();
            item.streamType = static_cast<AudioStreamType>(parcel.ReadInt32());
            item.sourceType = static_cast<SourceType>(parcel.ReadInt32());
            item.hintType = static_cast<InterruptHint>(parcel.ReadInt32());
            item.timestamp = parcel.ReadInt64();
            focusHistory.push_back(item);
        }
        return true;
    }

    bool UnmarshallingCapturerInfo(Parcel &parcel)
    {
        std::unique_ptr<AudioCapturerInfo> capturerInfoPtr(AudioCapturerInfo::Unmarshalling(parcel));
        if (capturerInfoPtr == nullptr) {
            return false;
        }
        capturerInfo = std::move(capturerInfoPtr);

        loopbackStatus = parcel.ReadInt32();
        loopbackMode = parcel.ReadInt32();
        loopbackOutputDeviceType = static_cast<DeviceType>(parcel.ReadInt32());
        loopbackInputDeviceType = static_cast<DeviceType>(parcel.ReadInt32());
        captureTimestamp.framePosition = parcel.ReadInt64();
        captureTimestamp.time.tv_sec = static_cast<time_t>(parcel.ReadInt64());
        captureTimestamp.time.tv_nsec = static_cast<long>(parcel.ReadInt64());
        captureBufferSize = parcel.ReadUint64();
        captureOverflowCount = parcel.ReadUint32();
        muteWhenInterrupted = parcel.ReadBool();
        inputDeviceInfo.UnmarshallingSelf(parcel);
        return true;
    }

    static AudioDebugInfo *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) AudioDebugInfo();
        if (info == nullptr) {
            return nullptr;
        }
        info->sessionId = parcel.ReadUint32();

        std::unique_ptr<AudioRendererInfo> rendererInfoPtr(AudioRendererInfo::Unmarshalling(parcel));
        if (rendererInfoPtr == nullptr) {
            delete info;
            return nullptr;
        }
        info->rendererInfo = *rendererInfoPtr;

        if (!info->UnmarshallingStreamParams(parcel)) {
            delete info;
            return nullptr;
        }

        std::unique_ptr<AudioStreamDescriptor> streamDescPtr(AudioStreamDescriptor::Unmarshalling(parcel));
        if (streamDescPtr == nullptr) {
            delete info;
            return nullptr;
        }
        info->streamDesc = std::move(streamDescPtr);

        if (!info->UnmarshallingPipeInfo(parcel) ||
            !info->UnmarshallingDeviceInfo(parcel) ||
            !info->UnmarshallingVolumeInfo(parcel) ||
            !info->UnmarshallingInterruptInfo(parcel) ||
            !info->UnmarshallingCapturerInfo(parcel)) {
            delete info;
            return nullptr;
        }
        return info;
    }
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_DEBUG_INFO_H
