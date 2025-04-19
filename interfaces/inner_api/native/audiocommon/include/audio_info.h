/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#ifndef AUDIO_INFO_H
#define AUDIO_INFO_H

#ifdef __MUSL__
#include <stdint.h>
#endif // __MUSL__

#include <cmath>
#include <limits>
#include <string>
#include <vector>
#include <array>
#include <unistd.h>
#include <unordered_map>
#include <parcel.h>
#include <audio_source_type.h>
#include <audio_device_info.h>
#include <audio_interrupt_info.h>
#include <audio_session_info.h>
#include <audio_stream_info.h>
#include <audio_asr.h>

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t INVALID_PID = -1;
constexpr int32_t CLEAR_PID = 0;
constexpr int32_t SYSTEM_PID = 1;
constexpr int32_t CLEAR_UID = 0;
constexpr int32_t SYSTEM_UID = 1;
constexpr int32_t INVALID_UID = -1;
constexpr int32_t NETWORK_ID_SIZE = 80;
constexpr int32_t DEFAULT_VOLUME_GROUP_ID = 1;
constexpr int32_t AUDIO_FLAG_INVALID = -1;
constexpr int32_t AUDIO_FLAG_NORMAL = 0;
constexpr int32_t AUDIO_FLAG_MMAP = 1;
constexpr int32_t AUDIO_FLAG_VOIP_FAST = 2;
constexpr int32_t AUDIO_FLAG_DIRECT = 3;
constexpr int32_t AUDIO_FLAG_VOIP_DIRECT = 4;
constexpr int32_t AUDIO_FLAG_FORCED_NORMAL = 10;
constexpr int32_t AUDIO_USAGE_NORMAL = 0;
constexpr int32_t AUDIO_USAGE_VOIP = 1;
constexpr uint32_t STREAM_FLAG_FAST = 1;
constexpr float MAX_STREAM_SPEED_LEVEL = 4.0f;
constexpr float MIN_STREAM_SPEED_LEVEL = 0.125f;
constexpr int32_t EMPTY_UID = 0;
constexpr int32_t AUDIO_NORMAL_MANAGER_TYPE = 0;
constexpr int32_t AUDIO_DIRECT_MANAGER_TYPE = 2;

constexpr uint32_t MIN_STREAMID = 100000;
constexpr uint32_t MAX_STREAMID = UINT32_MAX - MIN_STREAMID;

const float MIN_FLOAT_VOLUME = 0.0f;
const float MAX_FLOAT_VOLUME = 1.0f;

const char* MICROPHONE_PERMISSION = "ohos.permission.MICROPHONE";
const char* MODIFY_AUDIO_SETTINGS_PERMISSION = "ohos.permission.MODIFY_AUDIO_SETTINGS";
const char* ACCESS_NOTIFICATION_POLICY_PERMISSION = "ohos.permission.ACCESS_NOTIFICATION_POLICY";
const char* CAPTURER_VOICE_DOWNLINK_PERMISSION = "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO";
const char* RECORD_VOICE_CALL_PERMISSION = "ohos.permission.RECORD_VOICE_CALL";

const char* PRIMARY_WAKEUP = "Built_in_wakeup";
const char* INNER_CAPTURER_SINK = "InnerCapturerSink_";
const char* REMOTE_CAST_INNER_CAPTURER_SINK_NAME = "RemoteCastInnerCapturer";
const char* DUP_STREAM = "DupStream";
}

#ifdef FEATURE_DTMF_TONE
// Maximun number of sine waves in a tone segment
constexpr uint32_t TONEINFO_MAX_WAVES = 3;
//Maximun number of SupportedTones
constexpr uint32_t MAX_SUPPORTED_TONEINFO_SIZE = 65535;
// Maximun number of segments in a tone descriptor
constexpr uint32_t TONEINFO_MAX_SEGMENTS = 12;
constexpr uint32_t TONEINFO_INF = 0xFFFFFFFF;
class ToneSegment : public Parcelable {
public:
    uint32_t duration;
    uint16_t waveFreq[TONEINFO_MAX_WAVES+1];
    uint16_t loopCnt;
    uint16_t loopIndx;
    bool Marshalling(Parcel &parcel) const override
    {
        parcel.WriteUint32(duration);
        parcel.WriteUint16(loopCnt);
        parcel.WriteUint16(loopIndx);
        for (uint32_t i = 0; i < TONEINFO_MAX_WAVES + 1; i++) {
            parcel.WriteUint16(waveFreq[i]);
        }
        return true;
    }
    void Unmarshalling(Parcel &parcel)
    {
        duration = parcel.ReadUint32();
        loopCnt = parcel.ReadUint16();
        loopIndx = parcel.ReadUint16();
        for (uint32_t i = 0; i < TONEINFO_MAX_WAVES + 1; i++) {
            waveFreq[i] = parcel.ReadUint16();
        }
    }
};

class ToneInfo : public Parcelable {
public:
    ToneSegment segments[TONEINFO_MAX_SEGMENTS+1];
    uint32_t segmentCnt;
    uint32_t repeatCnt;
    uint32_t repeatSegment;
    bool Marshalling(Parcel &parcel) const override
    {
        parcel.WriteUint32(segmentCnt);
        parcel.WriteUint32(repeatCnt);
        parcel.WriteUint32(repeatSegment);
        if (!(segmentCnt >= 0 && segmentCnt <= TONEINFO_MAX_SEGMENTS + 1)) {
            return false;
        }
        for (uint32_t i = 0; i < segmentCnt; i++) {
            segments[i].Marshalling(parcel);
        }
        return true;
    }
    void Unmarshalling(Parcel &parcel)
    {
        segmentCnt = parcel.ReadUint32();
        repeatCnt = parcel.ReadUint32();
        repeatSegment = parcel.ReadUint32();
        if (!(segmentCnt >= 0 && segmentCnt <= TONEINFO_MAX_SEGMENTS + 1)) {
            return;
        }
        for (uint32_t i = 0; i < segmentCnt; i++) {
            segments[i].Unmarshalling(parcel);
        }
    }
};
#endif

enum VolumeAdjustType {
    /**
     * Adjust volume up
     */
    VOLUME_UP = 0,
    /**
     * Adjust volume down
     */
    VOLUME_DOWN = 1,
};

enum ChannelBlendMode {
    /**
     * No channel process.
     */
    MODE_DEFAULT = 0,
    /**
     * Blend left and right channel.
     */
    MODE_BLEND_LR = 1,
    /**
     * Replicate left to right channel.
     */
    MODE_ALL_LEFT = 2,
    /**
     * Replicate right to left channel.
     */
    MODE_ALL_RIGHT = 3,
};

enum ConnectType {
    /**
     * Group connect type of local device
     */
    CONNECT_TYPE_LOCAL = 0,
    /**
     * Group connect type of distributed device
     */
    CONNECT_TYPE_DISTRIBUTED
};

typedef AudioStreamType AudioVolumeType;

enum VolumeFlag {
    /**
     * Show system volume bar
     */
    FLAG_SHOW_SYSTEM_UI = 1,
};

enum AudioOffloadType {
    /**
     * Indicates audio offload state default.
     */
    OFFLOAD_DEFAULT = -1,
    /**
     * Indicates audio offload state : screen is active & app is foreground.
     */
    OFFLOAD_ACTIVE_FOREGROUND = 0,
    /**
     * Indicates audio offload state : screen is active & app is background.
     */
    OFFLOAD_ACTIVE_BACKGROUND = 1,
    /**
     * Indicates audio offload state : screen is inactive & app is background.
     */
    OFFLOAD_INACTIVE_BACKGROUND = 3,
};

enum FocusType {
    /**
     * Recording type.
     */
    FOCUS_TYPE_RECORDING = 0,
};

enum AudioErrors {
    /**
     * Common errors.
     */
    ERROR_INVALID_PARAM = 6800101,
    ERROR_NO_MEMORY     = 6800102,
    ERROR_ILLEGAL_STATE = 6800103,
    ERROR_UNSUPPORTED   = 6800104,
    ERROR_TIMEOUT       = 6800105,
    /**
     * Audio specific errors.
     */
    ERROR_STREAM_LIMIT  = 6800201,
    /**
     * Default error.
     */
    ERROR_SYSTEM        = 6800301
};

// Ringer Mode
enum AudioRingerMode {
    RINGER_MODE_SILENT = 0,
    RINGER_MODE_VIBRATE = 1,
    RINGER_MODE_NORMAL = 2
};

/**
 * Enumerates audio stream privacy type for playback capture.
 */
enum AudioPrivacyType {
    PRIVACY_TYPE_PUBLIC = 0,
    PRIVACY_TYPE_PRIVATE = 1
};

/**
* Enumerates the renderer playback speed.
*/
enum AudioRendererRate {
    RENDER_RATE_NORMAL = 0,
    RENDER_RATE_DOUBLE = 1,
    RENDER_RATE_HALF = 2,
};

/**
* media safe volume status
*/
enum SafeStatus : int32_t {
    SAFE_UNKNOWN = -1,
    SAFE_INACTIVE = 0,
    SAFE_ACTIVE = 1,
};

enum CallbackChange : int32_t {
    CALLBACK_UNKNOWN = 0,
    CALLBACK_FOCUS_INFO_CHANGE,
    CALLBACK_RENDERER_STATE_CHANGE,
    CALLBACK_CAPTURER_STATE_CHANGE,
    CALLBACK_MICMUTE_STATE_CHANGE,
    CALLBACK_AUDIO_SESSION,
    CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE,
    CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE,
    CALLBACK_SET_VOLUME_KEY_EVENT,
    CALLBACK_SET_DEVICE_CHANGE,
    CALLBACK_SET_RINGER_MODE,
    CALLBACK_APP_VOLUME_CHANGE,
    CALLBACK_SELF_APP_VOLUME_CHANGE,
    CALLBACK_SET_MIC_STATE_CHANGE,
    CALLBACK_SPATIALIZATION_ENABLED_CHANGE,
    CALLBACK_HEAD_TRACKING_ENABLED_CHANGE,
    CALLBACK_SET_MICROPHONE_BLOCKED,
    CALLBACK_DEVICE_CHANGE_WITH_INFO,
    CALLBACK_HEAD_TRACKING_DATA_REQUESTED_CHANGE,
    CALLBACK_NN_STATE_CHANGE,
    CALLBACK_SET_AUDIO_SCENE_CHANGE,
    CALLBACK_SPATIALIZATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE,
    CALLBACK_DISTRIBUTED_OUTPUT_CHANGE,
    CALLBACK_MAX,
};

constexpr CallbackChange CALLBACK_ENUMS[] = {
    CALLBACK_UNKNOWN,
    CALLBACK_FOCUS_INFO_CHANGE,
    CALLBACK_RENDERER_STATE_CHANGE,
    CALLBACK_CAPTURER_STATE_CHANGE,
    CALLBACK_MICMUTE_STATE_CHANGE,
    CALLBACK_AUDIO_SESSION,
    CALLBACK_PREFERRED_OUTPUT_DEVICE_CHANGE,
    CALLBACK_PREFERRED_INPUT_DEVICE_CHANGE,
    CALLBACK_SET_VOLUME_KEY_EVENT,
    CALLBACK_SET_DEVICE_CHANGE,
    CALLBACK_SET_VOLUME_KEY_EVENT,
    CALLBACK_SET_DEVICE_CHANGE,
    CALLBACK_SET_RINGER_MODE,
    CALLBACK_SET_MIC_STATE_CHANGE,
    CALLBACK_SPATIALIZATION_ENABLED_CHANGE,
    CALLBACK_HEAD_TRACKING_ENABLED_CHANGE,
    CALLBACK_SET_MICROPHONE_BLOCKED,
    CALLBACK_DEVICE_CHANGE_WITH_INFO,
    CALLBACK_HEAD_TRACKING_DATA_REQUESTED_CHANGE,
    CALLBACK_NN_STATE_CHANGE,
    CALLBACK_SET_AUDIO_SCENE_CHANGE,
    CALLBACK_SPATIALIZATION_ENABLED_CHANGE_FOR_CURRENT_DEVICE,
    CALLBACK_DISTRIBUTED_OUTPUT_CHANGE,
};

static_assert((sizeof(CALLBACK_ENUMS) / sizeof(CallbackChange)) == static_cast<size_t>(CALLBACK_MAX),
    "check CALLBACK_ENUMS");

struct VolumeEvent {
    AudioVolumeType volumeType;
    int32_t volume;
    bool updateUi;
    int32_t volumeGroupId;
    std::string networkId;
    AudioVolumeMode volumeMode;
    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteInt32(static_cast<int32_t>(volumeType))
            && parcel.WriteInt32(volume)
            && parcel.WriteBool(updateUi)
            && parcel.WriteInt32(volumeGroupId)
            && parcel.WriteString(networkId)
            && parcel.WriteInt32(static_cast<int32_t>(volumeMode));
    }
    void Unmarshalling(Parcel &parcel)
    {
        volumeType = static_cast<AudioVolumeType>(parcel.ReadInt32());
        volume = parcel.ReadInt32();
        updateUi = parcel.ReadInt32();
        volumeGroupId = parcel.ReadInt32();
        networkId = parcel.ReadString();
        volumeMode = static_cast<AudioVolumeMode>(parcel.ReadInt32());
    }
};

struct AudioParameters {
    AudioSampleFormat format;
    AudioChannel channels;
    AudioSamplingRate samplingRate;
    AudioEncodingType encoding;
    ContentType contentType;
    StreamUsage usage;
    DeviceRole deviceRole;
    DeviceType deviceType;
    AudioVolumeMode mode;
};

struct A2dpDeviceConfigInfo {
    DeviceStreamInfo streamInfo;
    bool absVolumeSupport = false;
    int32_t volumeLevel = -1;
    bool mute = false;
};

enum PlayerType : int32_t {
    PLAYER_TYPE_DEFAULT = 0,

    // AudioFramework internal type.
    PLAYER_TYPE_OH_AUDIO_RENDERER = 100,
    PLAYER_TYPE_ARKTS_AUDIO_RENDERER = 101,
    PLAYER_TYPE_CJ_AUDIO_RENDERER = 102,
    PLAYER_TYPE_OPENSL_ES = 103,

    // Indicates a type from the system internals, but not from the AudioFramework.
    PLAYER_TYPE_SOUND_POOL = 1000,
    PLAYER_TYPE_AV_PLAYER = 1001,
    PLAYER_TYPE_SYSTEM_WEBVIEW = 1002,
    PLAYER_TYPE_TONE_PLAYER = 1003,
};

struct AudioRendererInfo {
    ContentType contentType = CONTENT_TYPE_UNKNOWN;
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
    int32_t rendererFlags = AUDIO_FLAG_NORMAL;
    AudioVolumeMode volumeMode = AUDIOSTREAM_VOLUMEMODE_SYSTEM_GLOBAL;
    std::string sceneType = "";
    bool spatializationEnabled = false;
    bool headTrackingEnabled = false;
    int32_t originalFlag = AUDIO_FLAG_NORMAL;
    AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
    AudioSamplingRate samplingRate = SAMPLE_RATE_8000;
    uint8_t encodingType = 0;
    uint64_t channelLayout = 0ULL;
    AudioSampleFormat format = SAMPLE_S16LE;
    bool isOffloadAllowed = true;
    bool isSatellite = false;
    PlayerType playerType = PLAYER_TYPE_DEFAULT;
    // Expected length of audio stream to be played.
    // Currently only used for making decisions on fade-in and fade-out strategies.
    // 0 is the default value, it is considered that no
    uint64_t expectedPlaybackDurationBytes = 0;
    int32_t effectMode = 1;

    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteInt32(static_cast<int32_t>(contentType))
            && parcel.WriteInt32(static_cast<int32_t>(streamUsage))
            && parcel.WriteInt32(rendererFlags)
            && parcel.WriteInt32(originalFlag)
            && parcel.WriteString(sceneType)
            && parcel.WriteBool(spatializationEnabled)
            && parcel.WriteBool(headTrackingEnabled)
            && parcel.WriteInt32(static_cast<int32_t>(pipeType))
            && parcel.WriteInt32(static_cast<int32_t>(samplingRate))
            && parcel.WriteUint8(encodingType)
            && parcel.WriteUint64(channelLayout)
            && parcel.WriteInt32(format)
            && parcel.WriteBool(isOffloadAllowed)
            && parcel.WriteInt32(playerType)
            && parcel.WriteUint64(expectedPlaybackDurationBytes)
            && parcel.WriteInt32(effectMode)
            && parcel.WriteInt32(static_cast<int32_t>(volumeMode));
    }
    void Unmarshalling(Parcel &parcel)
    {
        contentType = static_cast<ContentType>(parcel.ReadInt32());
        streamUsage = static_cast<StreamUsage>(parcel.ReadInt32());
        rendererFlags = parcel.ReadInt32();
        originalFlag = parcel.ReadInt32();
        sceneType = parcel.ReadString();
        spatializationEnabled = parcel.ReadBool();
        headTrackingEnabled = parcel.ReadBool();
        pipeType = static_cast<AudioPipeType>(parcel.ReadInt32());
        samplingRate = static_cast<AudioSamplingRate>(parcel.ReadInt32());
        encodingType = parcel.ReadUint8();
        channelLayout = parcel.ReadUint64();
        format = static_cast<AudioSampleFormat>(parcel.ReadInt32());
        isOffloadAllowed = parcel.ReadBool();
        playerType = static_cast<PlayerType>(parcel.ReadInt32());
        expectedPlaybackDurationBytes = parcel.ReadUint64();
        effectMode = parcel.ReadInt32();
        volumeMode = static_cast<AudioVolumeMode>(parcel.ReadInt32());
    }
};

class AudioCapturerInfo {
public:
    SourceType sourceType = SOURCE_TYPE_INVALID;
    int32_t capturerFlags = 0;
    int32_t originalFlag = AUDIO_FLAG_NORMAL;
    AudioPipeType pipeType = PIPE_TYPE_UNKNOWN;
    AudioSamplingRate samplingRate = SAMPLE_RATE_8000;
    uint8_t encodingType = 0;
    uint64_t channelLayout = 0ULL;
    std::string sceneType = "";

    AudioCapturerInfo(SourceType sourceType_, int32_t capturerFlags_) : sourceType(sourceType_),
        capturerFlags(capturerFlags_) {}
    AudioCapturerInfo(const AudioCapturerInfo &audioCapturerInfo)
    {
        *this = audioCapturerInfo;
    }
    AudioCapturerInfo() = default;
    ~AudioCapturerInfo()= default;
    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteInt32(static_cast<int32_t>(sourceType)) &&
            parcel.WriteInt32(capturerFlags) &&
            parcel.WriteInt32(originalFlag) &&
            parcel.WriteInt32(static_cast<int32_t>(pipeType)) &&
            parcel.WriteInt32(static_cast<int32_t>(samplingRate)) &&
            parcel.WriteUint8(encodingType) &&
            parcel.WriteUint64(channelLayout) &&
            parcel.WriteString(sceneType);
    }
    void Unmarshalling(Parcel &parcel)
    {
        sourceType = static_cast<SourceType>(parcel.ReadInt32());
        capturerFlags = parcel.ReadInt32();
        originalFlag = parcel.ReadInt32();
        pipeType = static_cast<AudioPipeType>(parcel.ReadInt32());
        samplingRate = static_cast<AudioSamplingRate>(parcel.ReadInt32());
        encodingType = parcel.ReadUint8();
        channelLayout = parcel.ReadUint64();
        sceneType = parcel.ReadString();
    }
};

struct AudioRendererDesc {
    ContentType contentType = CONTENT_TYPE_UNKNOWN;
    StreamUsage streamUsage = STREAM_USAGE_UNKNOWN;
};

struct AudioRendererOptions {
    AudioStreamInfo streamInfo;
    AudioRendererInfo rendererInfo;
    AudioPrivacyType privacyType = PRIVACY_TYPE_PUBLIC;
    AudioSessionStrategy strategy = { AudioConcurrencyMode::INVALID };
};

struct MicStateChangeEvent {
    bool mute;
};

enum AudioScene : int32_t {
    /**
     * Invalid
     */
    AUDIO_SCENE_INVALID = -1,
    /**
     * Default audio scene
     */
    AUDIO_SCENE_DEFAULT,
    /**
     * Ringing audio scene
     * Only available for system api.
     */
    AUDIO_SCENE_RINGING,
    /**
     * Phone call audio scene
     * Only available for system api.
     */
    AUDIO_SCENE_PHONE_CALL,
    /**
     * Voice chat audio scene
     */
    AUDIO_SCENE_PHONE_CHAT,
    /**
     * AvSession set call start flag
     */
    AUDIO_SCENE_CALL_START,
    /**
     * AvSession set call end flag
     */
    AUDIO_SCENE_CALL_END,
    /**
     * Voice ringing audio scene
     * Only available for system api.
     */
    AUDIO_SCENE_VOICE_RINGING,
    /**
     * Max
     */
    AUDIO_SCENE_MAX,
};

enum AudioDeviceUsage : uint32_t {
    /**
     * Media output devices.
     * @syscap SystemCapability.Multimedia.Audio.Device
     * @systemapi
     * @since 11
     */
    MEDIA_OUTPUT_DEVICES = 1,
    /**
     * Media input devices.
     * @syscap SystemCapability.Multimedia.Audio.Device
     * @systemapi
     * @since 11
     */
    MEDIA_INPUT_DEVICES = 2,
    /**
     * All media devices.
     * @syscap SystemCapability.Multimedia.Audio.Device
     * @systemapi
     * @since 11
     */
    ALL_MEDIA_DEVICES = 3,
    /**
     * Call output devices.
     * @syscap SystemCapability.Multimedia.Audio.Device
     * @systemapi
     * @since 11
     */
    CALL_OUTPUT_DEVICES = 4,
    /**
     * Call input devices.
     * @syscap SystemCapability.Multimedia.Audio.Device
     * @systemapi
     * @since 11
     */
    CALL_INPUT_DEVICES = 8,
    /**
     * All call devices.
     * @syscap SystemCapability.Multimedia.Audio.Device
     * @systemapi
     * @since 11
     */
    ALL_CALL_DEVICES = 12,
    /**
     * All devices.
     * @syscap SystemCapability.Multimedia.Audio.Device
     * @systemapi
     * @since 11
     */
    D_ALL_DEVICES = 15,
};

enum FilterMode : uint32_t {
    INCLUDE = 0,
    EXCLUDE,
    MAX_FILTER_MODE
};

// 1.If the size of usages or pids is 0, FilterMode will not work.
// 2.Filters will only works with FileterMode INCLUDE or EXCLUDE while the vector size is not zero.
// 3.If usages and pids are both not empty, the result is the intersection of the two Filter.
// 4.If usages.size() == 0, defalut usages will be filtered with FilterMode::INCLUDE.
// 5.Default usages are MEDIA MUSIC MOVIE GAME and BOOK.
struct CaptureFilterOptions {
    std::vector<StreamUsage> usages;
    FilterMode usageFilterMode {FilterMode::INCLUDE};
    std::vector<int32_t> pids;
    FilterMode pidFilterMode {FilterMode::INCLUDE};

    bool operator ==(CaptureFilterOptions& filter)
    {
        std::sort(filter.usages.begin(), filter.usages.end());
        std::sort(filter.pids.begin(), filter.pids.end());
        std::sort(usages.begin(), usages.end());
        std::sort(pids.begin(), pids.end());
        return (filter.usages == usages && filter.usageFilterMode == usageFilterMode
            && filter.pids == pids && filter.pidFilterMode == pidFilterMode);
    }
};

struct AudioPlaybackCaptureConfig {
    CaptureFilterOptions filterOptions;
    bool silentCapture {false}; // To be deprecated since 12

    bool operator ==(AudioPlaybackCaptureConfig& filter)
    {
        return (filter.filterOptions == filterOptions && filter.silentCapture == silentCapture);
    }
};

struct AudioCapturerOptions {
    AudioStreamInfo streamInfo;
    AudioCapturerInfo capturerInfo;
    AudioPlaybackCaptureConfig playbackCaptureConfig;
    AudioSessionStrategy strategy = { AudioConcurrencyMode::INVALID };
};

struct AppInfo {
    int32_t appUid { INVALID_UID };
    uint32_t appTokenId { 0 };
    int32_t appPid { 0 };
    uint64_t appFullTokenId { 0 };
};

struct BufferQueueState {
    uint32_t numBuffers;
    uint32_t currentIndex;
};

enum AudioRenderMode {
    RENDER_MODE_NORMAL,
    RENDER_MODE_CALLBACK
};

enum AudioCaptureMode {
    CAPTURE_MODE_NORMAL,
    CAPTURE_MODE_CALLBACK
};

struct SinkInfo {
    uint32_t sinkId; // sink id
    std::string sinkName;
    std::string adapterName;
};

struct SinkInput {
    int32_t streamId;
    AudioStreamType streamType;

    // add for routing stream.
    int32_t uid; // client uid
    int32_t pid; // client pid
    uint32_t paStreamId; // streamId
    uint32_t deviceSinkId; // sink id
    std::string sinkName; // sink name
    int32_t statusMark; // mark the router status
    uint64_t startTime; // when this router is created
    bool Marshalling(Parcel &parcel) const
    {
        return parcel.WriteInt32(streamId) &&
               parcel.WriteInt32(static_cast<int32_t>(streamType)) &&
               parcel.WriteInt32(uid) &&
               parcel.WriteInt32(pid) &&
               parcel.WriteUint32(paStreamId);
    }
    void Unmarshalling(Parcel &parcel)
    {
        streamId = parcel.ReadInt32();
        streamType = static_cast<AudioStreamType>(parcel.ReadInt32());
        uid = parcel.ReadInt32();
        pid = parcel.ReadInt32();
        paStreamId = parcel.ReadUint32();
    }
};

struct SourceOutput {
    int32_t streamId;
    AudioStreamType streamType;

    // add for routing stream.
    int32_t uid; // client uid
    int32_t pid; // client pid
    uint32_t paStreamId; // streamId
    uint32_t deviceSourceId; // sink id
    int32_t statusMark; // mark the router status
    uint64_t startTime; // when this router is created
};

typedef uint32_t AudioIOHandle;

static inline bool FLOAT_COMPARE_EQ(const float& x, const float& y)
{
    return (std::abs((x) - (y)) <= (std::numeric_limits<float>::epsilon()));
}

enum AudioServiceIndex {
    HDI_SERVICE_INDEX = 0,
    AUDIO_SERVICE_INDEX
};

/**
 * @brief Enumerates the rendering states of the current device.
 */
enum RendererState {
    /** INVALID state */
    RENDERER_INVALID = -1,
    /** Create New Renderer instance */
    RENDERER_NEW,
    /** Reneder Prepared state */
    RENDERER_PREPARED,
    /** Rendere Running state */
    RENDERER_RUNNING,
    /** Renderer Stopped state */
    RENDERER_STOPPED,
    /** Renderer Released state */
    RENDERER_RELEASED,
    /** Renderer Paused state */
    RENDERER_PAUSED
};

/**
 * @brief Enumerates the capturing states of the current device.
 */
enum CapturerState {
    /** Capturer INVALID state */
    CAPTURER_INVALID = -1,
    /** Create new capturer instance */
    CAPTURER_NEW,
    /** Capturer Prepared state */
    CAPTURER_PREPARED,
    /** Capturer Running state */
    CAPTURER_RUNNING,
    /** Capturer Stopped state */
    CAPTURER_STOPPED,
    /** Capturer Released state */
    CAPTURER_RELEASED,
    /** Capturer Paused state */
    CAPTURER_PAUSED
};

enum State {
    /** INVALID */
    INVALID = -1,
    /** New */
    NEW,
    /** Prepared */
    PREPARED,
    /** Running */
    RUNNING,
    /** Stopped */
    STOPPED,
    /** Released */
    RELEASED,
    /** Paused */
    PAUSED,
    /** Stopping */
    STOPPING
};

struct StreamSwitchingInfo {
    bool isSwitching_ = false;
    State state_ = INVALID;
};

struct AudioRegisterTrackerInfo {
    uint32_t sessionId;
    int32_t clientPid;
    State state;
    AudioRendererInfo rendererInfo;
    AudioCapturerInfo capturerInfo;
    int32_t channelCount;
    uint32_t appTokenId;
};

enum StateChangeCmdType {
    CMD_FROM_CLIENT = 0,
    CMD_FROM_SYSTEM = 1
};

enum AudioMode {
    AUDIO_MODE_PLAYBACK,
    AUDIO_MODE_RECORD
};

// LEGACY_INNER_CAP: Called from hap build with api < 12, work normally.
// LEGACY_MUTE_CAP: Called from hap build with api >= 12, will cap mute data.
// MODERN_INNER_CAP: Called from SA with inner-cap right, work with filter.
enum InnerCapMode : uint32_t {
    LEGACY_INNER_CAP = 0,
    LEGACY_MUTE_CAP,
    MODERN_INNER_CAP,
    INVALID_CAP_MODE
};

struct AudioProcessConfig {
    int32_t callerUid = INVALID_UID;

    AppInfo appInfo;

    AudioStreamInfo streamInfo;

    AudioMode audioMode = AUDIO_MODE_PLAYBACK;

    AudioRendererInfo rendererInfo;

    AudioCapturerInfo capturerInfo;

    AudioStreamType streamType = STREAM_DEFAULT;

    DeviceType deviceType = DEVICE_TYPE_INVALID;

    bool isInnerCapturer = false;

    bool isWakeupCapturer = false;

    uint32_t originalSessionId = 0;

    AudioPrivacyType privacyType = PRIVACY_TYPE_PUBLIC;

    InnerCapMode innerCapMode {InnerCapMode::INVALID_CAP_MODE};

    int32_t innerCapId = 0;
};

struct Volume {
    bool isMute = false;
    float volumeFloat = 1.0f;
    uint32_t volumeInt = 0;
};

enum StreamSetState {
    STREAM_PAUSE,
    STREAM_RESUME,
    STREAM_MUTE,
    STREAM_UNMUTE
};

enum SwitchState {
    SWITCH_STATE_WAITING,
    SWITCH_STATE_TIMEOUT,
    SWITCH_STATE_CREATED,
    SWITCH_STATE_STARTED,
    SWITCH_STATE_FINISHED
};

struct SwitchStreamInfo {
    uint32_t sessionId = 0;
    int32_t callerUid = INVALID_UID;
    int32_t appUid = INVALID_UID;
    int32_t appPid = 0;
    uint32_t appTokenId = 0;
    CapturerState nextState = CAPTURER_INVALID;
    bool operator==(const SwitchStreamInfo& info) const
    {
        return sessionId == info.sessionId && callerUid == info.callerUid &&
            appUid == info.appUid && appPid == info.appPid && appTokenId == info.appTokenId;
    }
    bool operator!=(const SwitchStreamInfo& info) const
    {
        return !(*this == info);
    }

    bool operator<(const SwitchStreamInfo& info) const
    {
        if (sessionId != info.sessionId) {
            return sessionId < info.sessionId;
        }
        if (callerUid != info.callerUid) {
            return callerUid < info.callerUid;
        }
        if (appUid != info.appUid) {
            return appUid < info.appUid;
        }
        if (appPid != info.appPid) {
            return appPid < info.appPid;
        }
        return appTokenId < info.appTokenId;
    }

    bool operator<=(const SwitchStreamInfo& info) const
    {
        return *this < info || *this == info;
    }
    bool operator>(const SwitchStreamInfo& info) const
    {
        return !(*this <= info);
    }
    bool operator>=(const SwitchStreamInfo& info) const
    {
        return !(*this < info);
    }
};

struct StreamSetStateEventInternal {
    StreamSetState streamSetState;
    StreamUsage streamUsage;
};

enum AudioPin {
    AUDIO_PIN_NONE = 0, // Invalid pin
    AUDIO_PIN_OUT_SPEAKER = 1 << 0, // Speaker output pin
    AUDIO_PIN_OUT_HEADSET = 1 << 1, // Wired headset pin for output
    AUDIO_PIN_OUT_LINEOUT = 1 << 2, // Line-out pin
    AUDIO_PIN_OUT_HDMI = 1 << 3, // HDMI output pin
    AUDIO_PIN_OUT_USB = 1 << 4, // USB output pin
    AUDIO_PIN_OUT_USB_EXT = 1 << 5, // Extended USB output pin
    AUDIO_PIN_OUT_BLUETOOTH_SCO = 1 << 6, // Bluetooth SCO output pin
    AUDIO_PIN_OUT_DAUDIO_DEFAULT = 1 << 7, // Daudio default output pin
    AUDIO_PIN_OUT_HEADPHONE = 1 << 8, // Wired headphone output pin
    AUDIO_PIN_OUT_USB_HEADSET = 1 << 9,  // Arm usb output pin
    AUDIO_PIN_OUT_DP = 1 << 11,
    AUDIO_PIN_IN_MIC = 1 << 27 | 1 << 0, // Microphone input pin
    AUDIO_PIN_IN_HS_MIC = 1 << 27 | 1 << 1, // Wired headset microphone pin for input
    AUDIO_PIN_IN_LINEIN = 1 << 27 | 1 << 2, // Line-in pin
    AUDIO_PIN_IN_USB_EXT = 1 << 27 | 1 << 3, // Extended USB input pin
    AUDIO_PIN_IN_BLUETOOTH_SCO_HEADSET = 1 << 27 | 1 << 4, // Bluetooth SCO headset input pin
    AUDIO_PIN_IN_DAUDIO_DEFAULT = 1 << 27 | 1 << 5, // Daudio default input pin
    AUDIO_PIN_IN_USB_HEADSET = 1 << 27 | 1 << 6,  // Arm usb input pin
};

enum AudioParamKey {
    NONE = 0,
    VOLUME = 1,
    INTERRUPT = 2,
    PARAM_KEY_STATE = 5,
    A2DP_SUSPEND_STATE = 6,  // for bluetooth sink
    BT_HEADSET_NREC = 7,
    BT_WBS = 8,
    A2DP_OFFLOAD_STATE = 9, // for a2dp offload
    GET_DP_DEVICE_INFO = 10, // for dp sink
    USB_DEVICE = 101, // Check USB device type ARM or HIFI
    PERF_INFO = 201,
    MMI = 301,
    PARAM_KEY_LOWPOWER = 1000,
};

struct DStatusInfo {
    char networkId[NETWORK_ID_SIZE];
    AudioPin hdiPin = AUDIO_PIN_NONE;
    int32_t mappingVolumeId = 0;
    int32_t mappingInterruptId = 0;
    int32_t deviceId;
    int32_t channelMasks;
    std::string deviceName = "";
    bool isConnected = false;
    std::string macAddress;
    DeviceStreamInfo streamInfo = {};
    ConnectType connectType = CONNECT_TYPE_LOCAL;
};

struct AudioRendererDataInfo {
    uint8_t *buffer;
    size_t flag;
};

enum AudioPermissionState {
    AUDIO_PERMISSION_START = 0,
    AUDIO_PERMISSION_STOP = 1,
};

class AudioRendererPolicyServiceDiedCallback {
public:
    virtual ~AudioRendererPolicyServiceDiedCallback() = default;

    /**
     * Called when audio policy service died.
     * @since 10
     */
    virtual void OnAudioPolicyServiceDied() = 0;
};

class AudioCapturerPolicyServiceDiedCallback {
public:
    virtual ~AudioCapturerPolicyServiceDiedCallback() = default;

    /**
     * Called when audio policy service died.
     * @since 10
     */
    virtual void OnAudioPolicyServiceDied() = 0;
};

class AudioStreamPolicyServiceDiedCallback {
public:
    virtual ~AudioStreamPolicyServiceDiedCallback() = default;

    /**
     * Called when audio policy service died.
     * @since 11
     */
    virtual void OnAudioPolicyServiceDied() = 0;
};

/**
 * Describes three-dimensional value.
 * @since 11
 */
struct Vector3D {
    /**
     * X-axis value.
     * @since 11
     */
    float x;
    /**
     * Y-axis value.
     * @since 11
     */
    float y;
    /**
     * Z-axis value.
     * @since 11
     */
    float z;
};

struct SessionInfo {
    SourceType sourceType;
    uint32_t rate;
    uint32_t channels;
};

enum CastType {
    CAST_TYPE_NULL = 0,
    CAST_TYPE_ALL,
    CAST_TYPE_PROJECTION,
    CAST_TYPE_COOPERATION,
};

class AudioPnpDeviceChangeCallback {
public:
    virtual ~AudioPnpDeviceChangeCallback() = default;
    virtual void OnPnpDeviceStatusChanged(const std::string &info) = 0;
    virtual void OnMicrophoneBlocked(const std::string &info) = 0;
};

struct SourceInfo {
    SourceType sourceType_;
    uint32_t rate_;
    uint32_t channels_;
};

/**
 * @brief Device group used by set/get volume.
 */
enum DeviceGroup {
    /** Invalid device group */
    DEVICE_GROUP_INVALID = -1,
    /** Built in device */
    DEVICE_GROUP_BUILT_IN,
    /** Wired device */
    DEVICE_GROUP_WIRED,
    /** Wireless device */
    DEVICE_GROUP_WIRELESS,
    /** Remote cast device */
    DEVICE_GROUP_REMOTE_CAST,
    /* earpiece device*/
    DEVICE_GROUP_EARPIECE,
};

static inline DeviceGroup GetVolumeGroupForDevice(DeviceType deviceType)
{
    static const std::map<DeviceType, DeviceGroup> DEVICE_GROUP_FOR_VOLUME = {
        {DEVICE_TYPE_EARPIECE, DEVICE_GROUP_EARPIECE}, {DEVICE_TYPE_SPEAKER, DEVICE_GROUP_BUILT_IN},
        {DEVICE_TYPE_DP, DEVICE_GROUP_BUILT_IN}, {DEVICE_TYPE_WIRED_HEADSET, DEVICE_GROUP_WIRED},
        {DEVICE_TYPE_USB_HEADSET, DEVICE_GROUP_WIRED}, {DEVICE_TYPE_USB_ARM_HEADSET, DEVICE_GROUP_WIRED},
        {DEVICE_TYPE_BLUETOOTH_A2DP, DEVICE_GROUP_WIRELESS}, {DEVICE_TYPE_BLUETOOTH_SCO, DEVICE_GROUP_WIRELESS},
        {DEVICE_TYPE_REMOTE_CAST, DEVICE_GROUP_REMOTE_CAST}, {DEVICE_TYPE_HDMI, DEVICE_GROUP_BUILT_IN},
    };
    auto it = DEVICE_GROUP_FOR_VOLUME.find(deviceType);
    return it == DEVICE_GROUP_FOR_VOLUME.end() ? DEVICE_GROUP_INVALID : it->second;
}

enum RouterType {
    /**
     * None router.
     * @since 12
     */
    ROUTER_TYPE_NONE = 0,
    /**
     * Default router.
     * @since 12
     */
    ROUTER_TYPE_DEFAULT,
    /**
     * Stream filter router.
     * @since 12
     */
    ROUTER_TYPE_STREAM_FILTER,
    /**
     * Package filter router.
     * @since 12
     */
    ROUTER_TYPE_PACKAGE_FILTER,
    /**
     * Cockpit phone router.
     * @since 12
     */
    ROUTER_TYPE_COCKPIT_PHONE,
    /**
     * Privacy priority router.
     * @since 12
     */
    ROUTER_TYPE_PRIVACY_PRIORITY,
    /**
     * Public priority router.
     * @since 12
     */
    ROUTER_TYPE_PUBLIC_PRIORITY,
    /**
     * Pair device router.
     * @since 12
     */
    ROUTER_TYPE_PAIR_DEVICE,
    /**
     * User select router.
     * @since 12
     */
    ROUTER_TYPE_USER_SELECT,

    /**
     * App select router.
     * @since 12
     */
    ROUTER_TYPE_APP_SELECT,
};

enum RenderMode {
    /**
     * Primary render mode.
     * @since 12
     */
    PRIMARY,
    /**
     * VOIP render mode.
     * @since 12
     */
    VOIP,
    /**
     * Offload render mode.
     * @since 12
     */
    OFFLOAD,
    /**
     * Low latency render mode.
     * @since 12
     */
    LOW_LATENCY,
};

enum WriteDataCallbackType {
    /**
     * Use OH_AudioRenderer_Callbacks.OH_AudioRenderer_OnWriteData
     * @since 12
     */
    WRITE_DATA_CALLBACK_WITHOUT_RESULT = 0,
    /**
     * Use OH_AudioRenderer_OnWriteDataCallback.
     * @since 12
     */
    WRITE_DATA_CALLBACK_WITH_RESULT = 1
};

enum ReadDataCallbackType {
    /**
     * Use OH_AudioCapturer_Callbacks.OH_AudioCapturer_OnReadData
     * @since 12
     */
    READ_DATA_CALLBACK_WITHOUT_RESULT = 0,
    /**
     * Use OH_AudioCapturer_OnReadDataCallback.
     * @since 12
     */
    READ_DATA_CALLBACK_WITH_RESULT = 1
};

enum StreamEventCallbackType {
    /**
     * Use OH_AudioCapturer_Callbacks.OH_AudioCapturer_OnStreamEvent
     * @since 12
     */
    STREAM_EVENT_CALLBACK_WITHOUT_RESULT = 0,
    /**
     * Use OH_AudioCapturer_OnStreamEventCallback.
     * @since 12
     */
    STREAM_EVENT_CALLBACK_WITH_RESULT = 1
};

enum InterruptEventCallbackType {
    /**
     * Use OH_AudioRenderer_Callbacks.OH_AudioRenderer_OnInterruptEvent
     * @since 12
     */
    INTERRUPT_EVENT_CALLBACK_WITHOUT_RESULT = 0,
    /**
     * Use OH_AudioRenderer_OnInterruptEventCallback.
     * @since 12
     */
    INTERRUPT_EVENT_CALLBACK_WITH_RESULT = 1
};

enum ErrorCallbackType {
    /**
     * Use OH_AudioRenderer_Callbacks.OH_AudioRenderer_OnError
     *
     * @since 12
     */
    ERROR_CALLBACK_WITHOUT_RESULT = 0,
    /**
     * Use OH_AudioRenderer_OnErrorCallback.
     * @since 12
     */
    ERROR_CALLBACK_WITH_RESULT = 1
};

enum PolicyType {
    EDM_POLICY_TYPE = 0,
    PRIVACY_POLCIY_TYPE = 1,
    TEMPORARY_POLCIY_TYPE = 2,
};

enum SuscribeResultCode {
    SUCCESS_SUBSCRIBE = 0,
    /**
     * Volume button input error
     */
    ERR_SUBSCRIBE_INVALID_PARAM,
     /**
     * The keyOption creation failed
     */
    ERR_SUBSCRIBE_KEY_OPTION_NULL,
     /**
     * The im pointer creation failed
     */
    ERR_SUBSCRIBE_MMI_NULL,
    /**
     * Volume key multimode subscription results
     */
    ERR_MODE_SUBSCRIBE,
};

enum RendererStage {
    RENDERER_STAGE_UNKNOWN = 0,
    RENDERER_STAGE_START_OK = 0x10,
    RENDERER_STAGE_START_FAIL = 0x11,
    RENDERER_STAGE_PAUSE_OK = 0x20,
    RENDERER_STAGE_STOP_OK = 0x30,
    RENDERER_STAGE_STANDBY_BEGIN = 0x40,
    RENDERER_STAGE_STANDBY_END = 0x41,
    RENDERER_STAGE_SET_VOLUME_ZERO = 0x50,
    RENDERER_STAGE_SET_VOLUME_NONZERO = 0x51,
};

enum CapturerStage {
    CAPTURER_STAGE_START_OK = 0x10,
    CAPTURER_STAGE_START_FAIL = 0x11,
    CAPTURER_STAGE_PAUSE_OK = 0x20,
    CAPTURER_STAGE_STOP_OK = 0x30,
};


enum RestoreStatus : int32_t {
    NO_NEED_FOR_RESTORE = 0,
    NEED_RESTORE,
    RESTORING,
    RESTORE_ERROR,
};

enum RestoreReason : int32_t {
    DEFAULT_REASON = 0,
    DEVICE_CHANGED,
    STREAM_CONCEDED,
    STREAM_SPLIT,
    SERVER_DIED,
};

enum CheckPosTimeRes : int32_t {
    CHECK_SUCCESS = 0,
    CHECK_FAILED,
    NEED_MODIFY,
};

struct RestoreInfo {
    RestoreReason restoreReason = DEFAULT_REASON;
    int32_t deviceChangeReason = 0;
    int32_t targetStreamFlag = AUDIO_FLAG_NORMAL;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_INFO_H
