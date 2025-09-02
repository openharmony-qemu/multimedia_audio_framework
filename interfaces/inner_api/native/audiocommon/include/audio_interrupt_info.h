/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#ifndef AUDIO_INTERRUPT_INFO_H
#define AUDIO_INTERRUPT_INFO_H

#include <parcel.h>
#include <audio_stream_info.h>
#include <audio_source_type.h>
#include <audio_session_info.h>

namespace OHOS {
namespace AudioStandard {
static constexpr int32_t AUDIO_INTERRUPT_INFO_SIZE_LIMIT = 65535;

enum ActionTarget {
    CURRENT = 0,
    INCOMING,
    BOTH
};

/**
 * larger enumeration values has higher priority
 */
enum AudioFocuState {
    ACTIVE = 0,
    MUTED,
    DUCK,
    PAUSE,
    STOP,
    PLACEHOLDER,
    PAUSEDBYREMOTE,
};

enum InterruptMode {
    SHARE_MODE = 0,
    INDEPENDENT_MODE = 1
};

/**
 * Enumerates the audio interrupt request type.
 */
enum InterruptRequestType {
    INTERRUPT_REQUEST_TYPE_DEFAULT = 0,
};

/**
 * Enumerates audio interrupt event.
 */
enum InterruptCallbackEvent {
    NO_EVENT = 0,
    FORCE_EVENT = 1,
    FORCE_PAUSED_TO_RESUME_EVENT = 2
};

/**
 * Enumerates audio interrupt request result type.
 */
enum InterruptRequestResultType {
    INTERRUPT_REQUEST_GRANT = 0,
    INTERRUPT_REQUEST_REJECT = 1
};

enum InterruptType {
    INTERRUPT_TYPE_BEGIN = 1,
    INTERRUPT_TYPE_END = 2,
};

enum InterruptHint {
    INTERRUPT_HINT_NONE = 0,
    INTERRUPT_HINT_RESUME,
    INTERRUPT_HINT_PAUSE,
    INTERRUPT_HINT_STOP,
    INTERRUPT_HINT_DUCK,
    INTERRUPT_HINT_UNDUCK,
    INTERRUPT_HINT_MUTE,
    INTERRUPT_HINT_UNMUTE,
    INTERRUPT_HINT_EXIT_STANDALONE
};

enum InterruptForceType {
    /**
     * Force type, system change audio state.
     */
    INTERRUPT_FORCE = 0,
    /**
     * Share type, application change audio state.
     */
    INTERRUPT_SHARE
};

struct InterruptEvent : public Parcelable {
    InterruptEvent() {}
    InterruptEvent(InterruptType eventTypeIn, InterruptForceType forceTypeIn,
        InterruptHint hintType, bool callbackToAppIn = true)
        : eventType(eventTypeIn), forceType(forceTypeIn), hintType(hintType), callbackToApp(callbackToAppIn) {}
    bool Marshalling(Parcel &parcel) const override
    {
        parcel.WriteInt32(static_cast<int32_t>(eventType));
        parcel.WriteInt32(static_cast<int32_t>(forceType));
        parcel.WriteInt32(static_cast<int32_t>(hintType));
        parcel.WriteBool(callbackToApp);
        return true;
    }

    static InterruptEvent *Unmarshalling(Parcel &parcel)
    {
        auto info = new(std::nothrow) InterruptEvent();
        if (info == nullptr) {
            return nullptr;
        }
        info->eventType = static_cast<InterruptType>(parcel.ReadInt32());
        info->forceType = static_cast<InterruptForceType>(parcel.ReadInt32());
        info->hintType = static_cast<InterruptHint>(parcel.ReadInt32());
        info->callbackToApp = parcel.ReadBool();
        return info;
    }

    /**
     * Interrupt event type, begin or end
     */
    InterruptType eventType = INTERRUPT_TYPE_BEGIN;
    /**
     * Interrupt force type, force or share
     */
    InterruptForceType forceType = INTERRUPT_FORCE;
    /**
     * Interrupt hint type. In force type, the audio state already changed,
     * but in share mode, only provide a hint for application to decide.
     */
    InterruptHint hintType = INTERRUPT_HINT_NONE;
    /**
     * Should callback to app. Default true;
     * If false, interruptEvent should not callback to app.
     */
    bool callbackToApp = true;
};

// Used internally only by AudioFramework
struct InterruptEventInternal : public Parcelable {
    InterruptType eventType = INTERRUPT_TYPE_BEGIN;
    InterruptForceType forceType = INTERRUPT_FORCE;
    InterruptHint hintType = INTERRUPT_HINT_NONE;
    float duckVolume = 1.0f;
    bool callbackToApp = true;
    int64_t eventTimestamp = 0;

    InterruptEventInternal() = default;

    InterruptEventInternal(InterruptType eventtype, InterruptForceType forcetype,
        InterruptHint hinttype, float duckvolume)
    {
        eventType = eventtype;
        forceType = forcetype;
        hintType = hinttype;
        duckVolume = duckvolume;
        eventTimestamp = 0;
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return parcel.WriteInt32(static_cast<int32_t>(eventType))
            && parcel.WriteInt32(static_cast<int32_t>(forceType))
            && parcel.WriteInt32(static_cast<int32_t>(hintType))
            && parcel.WriteFloat(duckVolume)
            && parcel.WriteBool(callbackToApp)
            && parcel.WriteInt64(eventTimestamp);
    }

    static InterruptEventInternal *Unmarshalling(Parcel &parcel)
    {
        auto interrupt = new(std::nothrow) InterruptEventInternal();
        if (interrupt == nullptr) {
            return nullptr;
        }
        interrupt->eventType = static_cast<InterruptType>(parcel.ReadInt32());
        interrupt->forceType = static_cast<InterruptForceType>(parcel.ReadInt32());
        interrupt->hintType = static_cast<InterruptHint>(parcel.ReadInt32());
        interrupt->duckVolume = parcel.ReadFloat();
        interrupt->callbackToApp = parcel.ReadBool();
        interrupt->eventTimestamp = parcel.ReadInt64();
        return interrupt;
    }
};

enum AudioInterruptChangeType {
    ACTIVATE_AUDIO_INTERRUPT = 0,
    DEACTIVATE_AUDIO_INTERRUPT = 1,
};

// Below APIs are added to handle compilation error in call manager
// Once call manager adapt to new interrupt APIs, this will be removed
enum InterruptActionType {
    TYPE_ACTIVATED = 0,
    TYPE_INTERRUPT = 1
};

struct InterruptAction {
    InterruptActionType actionType;
    InterruptType interruptType;
    InterruptHint interruptHint;
    bool activated;
};

struct AudioFocusEntry {
    InterruptForceType forceType;
    InterruptHint hintType;
    ActionTarget actionOn;
    bool isReject;
};

struct AudioFocusConcurrency {
    std::vector<SourceType> sourcesTypes;
};

struct AudioFocusType {
    AudioStreamType streamType = STREAM_DEFAULT;
    SourceType sourceType = SOURCE_TYPE_INVALID;
    bool isPlay = true;
    bool operator==(const AudioFocusType &value) const
    {
        return streamType == value.streamType && sourceType == value.sourceType && isPlay == value.isPlay;
    }

    bool operator<(const AudioFocusType &value) const
    {
        return streamType < value.streamType || (streamType == value.streamType && sourceType < value.sourceType);
    }

    bool operator>(const AudioFocusType &value) const
    {
        return streamType > value.streamType || (streamType == value.streamType && sourceType > value.sourceType);
    }
};

enum InterruptStage {
    INTERRUPT_STAGE_START = 0x10,
    INTERRUPT_STAGE_RESTART = 0x11,
    INTERRUPT_STAGE_STOP = 0x12,
    INTERRUPT_STAGE_PAUSED = 0x20,
    INTERRUPT_STAGE_RESUMED = 0x21,
    INTERRUPT_STAGE_STOPPED = 0x30,
    INTERRUPT_STAGE_DUCK_BEGIN = 0x40,
    INTERRUPT_STAGE_DUCK_END = 0x41,
    INTERRUPT_STAGE_TIMEOUT = 0x50
};

enum InterruptSummary {
    INTERRUPT_SUMMARY_INTERRUPT_OTHERS = 0,
    INTERRUPT_SUMMARY_INTERRUPTED,
    INTERRUPT_SUMMARY_INTERRUPT_BACKGROUND,
};

enum InterruptRole {
    INTERRUPT_ROLE_DEFAULT = 0,
    INTERRUPT_ROLE_AUDIO_SESSION,
};

enum InterruptStrategy {
    DEFAULT = 0,
    MUTE = 1,
};

enum InterruptEventCallbackType {
    /**
     * Use OH_AudioRenderer_Callbacks.OH_AudioRenderer_OnInterruptEvent
     */
    INTERRUPT_EVENT_CALLBACK_COMBINED = 0,
    /**
     * Use OH_AudioRenderer_OnInterruptEventCallback
     */
    INTERRUPT_EVENT_CALLBACK_SEPERATED = 1,
    /**
     * Not use any OH_AudioRenderer InterruptEvent
     */
    INTERRUPT_EVENT_CALLBACK_DEFAULT = 2
};

class AudioInterrupt : public Parcelable {
public:
    static constexpr int32_t MAX_SOURCE_TYPE_NUM = 20;
    StreamUsage streamUsage = STREAM_USAGE_INVALID;
    ContentType contentType = CONTENT_TYPE_UNKNOWN;
    AudioFocusType audioFocusType;
    uint32_t streamId = 0;
    bool pauseWhenDucked = false;
    int32_t pid { -1 };
    int32_t uid { -1 };
    std::string deviceTag;
    mutable std::string bundleName;
    InterruptMode mode { SHARE_MODE };
    bool isAudioSessionInterrupt {false};
    AudioFocusConcurrency currencySources;
    AudioSessionStrategy sessionStrategy = { AudioConcurrencyMode::INVALID };
    int32_t api = 0;
    int32_t state {-1};
    InterruptStrategy strategy { InterruptStrategy::DEFAULT };
    InterruptEventCallbackType callbackType {INTERRUPT_EVENT_CALLBACK_DEFAULT};

    AudioInterrupt() = default;
    AudioInterrupt(StreamUsage streamUsage_, ContentType contentType_, AudioFocusType audioFocusType_,
        uint32_t streamId_) : streamUsage(streamUsage_), contentType(contentType_), audioFocusType(audioFocusType_),
        streamId(streamId_) {}
    ~AudioInterrupt() = default;

    bool operator==(const AudioInterrupt &other) const
    {
        return streamId == other.streamId &&
            streamUsage == other.streamUsage &&
            audioFocusType == other.audioFocusType &&
            pid == other.pid &&
            uid == other.uid;
    }

    bool operator<(const AudioInterrupt &other) const
    {
        return streamId < other.streamId || pid < other.pid || uid < other.uid;
    }

    bool operator>(const AudioInterrupt &other) const
    {
        return streamId > other.streamId || pid > other.pid || uid > other.uid;
    }

    static bool Marshalling(Parcel &parcel, const AudioInterrupt &interrupt)
    {
        bool res = parcel.WriteInt32(static_cast<int32_t>(interrupt.streamUsage));
        res = res && parcel.WriteInt32(static_cast<int32_t>(interrupt.contentType));
        res = res && parcel.WriteInt32(static_cast<int32_t>(interrupt.audioFocusType.streamType));
        res = res && parcel.WriteInt32(static_cast<int32_t>(interrupt.audioFocusType.sourceType));
        res = res && parcel.WriteBool(interrupt.audioFocusType.isPlay);
        res = res && parcel.WriteUint32(interrupt.streamId);
        res = res && parcel.WriteBool(interrupt.pauseWhenDucked);
        res = res && parcel.WriteInt32(interrupt.pid);
        res = res && parcel.WriteInt32(interrupt.uid);
        res = res && parcel.WriteString(interrupt.deviceTag);
        res = res && parcel.WriteString(interrupt.bundleName);
        res = res && parcel.WriteInt32(static_cast<int32_t>(interrupt.mode));
        res = res && parcel.WriteBool(interrupt.isAudioSessionInterrupt);
        size_t vct = interrupt.currencySources.sourcesTypes.size();
        res = res && parcel.WriteInt32(static_cast<int32_t>(vct));
        for (size_t i = 0; i < vct; i++) {
            res = res && parcel.WriteInt32(static_cast<int32_t>(interrupt.currencySources.sourcesTypes[i]));
        }
        res = res && parcel.WriteInt32(static_cast<int32_t>(interrupt.sessionStrategy.concurrencyMode));
        res = res && parcel.WriteInt32(interrupt.api);
        res = res && parcel.WriteInt32(interrupt.state);
        res = res && parcel.WriteInt32(static_cast<int32_t>(interrupt.strategy));
        res = res && parcel.WriteInt32(static_cast<int32_t>(interrupt.callbackType));
        return res;
    }
    static void Unmarshalling(Parcel &parcel, AudioInterrupt &interrupt)
    {
        interrupt.streamUsage = static_cast<StreamUsage>(parcel.ReadInt32());
        interrupt.contentType = static_cast<ContentType>(parcel.ReadInt32());
        interrupt.audioFocusType.streamType = static_cast<AudioStreamType>(parcel.ReadInt32());
        interrupt.audioFocusType.sourceType = static_cast<SourceType>(parcel.ReadInt32());
        interrupt.audioFocusType.isPlay = parcel.ReadBool();
        interrupt.streamId = parcel.ReadUint32();
        interrupt.pauseWhenDucked = parcel.ReadBool();
        interrupt.pid = parcel.ReadInt32();
        interrupt.uid = parcel.ReadInt32();
        interrupt.deviceTag = parcel.ReadString();
        interrupt.bundleName = parcel.ReadString();
        interrupt.mode = static_cast<InterruptMode>(parcel.ReadInt32());
        interrupt.isAudioSessionInterrupt = parcel.ReadBool();
        int32_t vct = parcel.ReadInt32();
        if (vct > MAX_SOURCE_TYPE_NUM) {
            return;
        }

        for (int32_t i = 0; i < vct; i++) {
            SourceType sourceType = static_cast<SourceType>(parcel.ReadInt32());
            interrupt.currencySources.sourcesTypes.push_back(sourceType);
        }
        interrupt.sessionStrategy.concurrencyMode = static_cast<AudioConcurrencyMode>(parcel.ReadInt32());
        interrupt.api = parcel.ReadInt32();
        interrupt.state = parcel.ReadInt32();
        interrupt.strategy = static_cast<InterruptStrategy>(parcel.ReadInt32());
        interrupt.callbackType = static_cast<InterruptEventCallbackType>(parcel.ReadInt32());
    }

    bool Marshalling(Parcel &parcel) const override
    {
        return Marshalling(parcel, *this);
    }

    static AudioInterrupt *Unmarshalling(Parcel &parcel)
    {
        auto interrupt = new(std::nothrow) AudioInterrupt();
        if (interrupt == nullptr) {
            return nullptr;
        }
        Unmarshalling(parcel, *interrupt);
        return interrupt;
    }
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_INTERRUPT_INFO_H