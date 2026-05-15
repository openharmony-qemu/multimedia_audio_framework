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
#ifndef LOG_TAG
#define LOG_TAG "AudioCapturer"
#endif

#include "audio_capturer.h"
#include "shared_audio_capturer_wrapper.h"

#include <cinttypes>
#include <cstring>

#include "audio_capturer_private.h"
#include "audio_errors.h"
#include "audio_capturer_log.h"
#include "audio_policy_manager.h"

#include "media_monitor_manager.h"
#include "audio_stream_descriptor.h"
#include "audio_info.h"
#include "audio_system_client_engine_manager.h"
#include "audio_capturer_types.h"
#include "audio_debug_manager.h"
#include "audio_debug_info.h"
#include "audio_debug_callback.h"
#include "stream_dfx_manager.h"

#undef LOG_DOMAIN
#define LOG_DOMAIN 0xD002B82
namespace OHOS {
namespace AudioStandard {
static constexpr uid_t UID_MSDP_SA = 6699;
static constexpr int32_t WRITE_OVERFLOW_NUM = 100;
static constexpr int32_t AUDIO_SOURCE_TYPE_INVALID_5 = 5;
static constexpr uint32_t BLOCK_INTERRUPT_CALLBACK_IN_MS = 1000; // 1000ms
static constexpr uint32_t BLOCK_INTERRUPT_OVERTIMES_IN_MS = 3000; // 3s
static constexpr int32_t MINIMUM_BUFFER_SIZE_MSEC = 5;
static constexpr int32_t MAXIMUM_BUFFER_SIZE_MSEC = 20;
static constexpr int32_t UID_MEDIA_SA = 1013;
static constexpr uint8_t AUDIO_MAX_CAPTURE_CHANNELS = 16;

std::map<AudioStreamType, SourceType> AudioCapturerPrivate::streamToSource_ = {
    {AudioStreamType::STREAM_MUSIC, SourceType::SOURCE_TYPE_MIC},
    {AudioStreamType::STREAM_MEDIA, SourceType::SOURCE_TYPE_MIC},
    {AudioStreamType::STREAM_MUSIC, SourceType::SOURCE_TYPE_UNPROCESSED},
    {AudioStreamType::STREAM_CAMCORDER, SourceType::SOURCE_TYPE_CAMCORDER},
    {AudioStreamType::STREAM_VOICE_CALL, SourceType::SOURCE_TYPE_VOICE_COMMUNICATION},
    {AudioStreamType::STREAM_ULTRASONIC, SourceType::SOURCE_TYPE_ULTRASONIC},
    {AudioStreamType::STREAM_WAKEUP, SourceType::SOURCE_TYPE_WAKEUP},
    {AudioStreamType::STREAM_SOURCE_VOICE_CALL, SourceType::SOURCE_TYPE_VOICE_CALL},
    {AudioStreamType::STREAM_MUSIC, SourceType::SOURCE_TYPE_LIVE},
    {AudioStreamType::STREAM_MEDIA, SourceType::SOURCE_TYPE_LIVE},
};

static const std::map<uint32_t, IAudioStream::StreamClass> AUDIO_INPUT_FLAG_GROUP_MAP = {
    {AUDIO_INPUT_FLAG_NORMAL, IAudioStream::StreamClass::PA_STREAM},
    {AUDIO_INPUT_FLAG_FAST, IAudioStream::StreamClass::FAST_STREAM},
    {AUDIO_INPUT_FLAG_VOIP_FAST, IAudioStream::StreamClass::VOIP_STREAM},
    {AUDIO_INPUT_FLAG_WAKEUP, IAudioStream::StreamClass::PA_STREAM},
};

static const std::map<AudioFlag, int32_t> INPUT_ROUTE_TO_STREAM_MAP = {
    {AUDIO_OUTPUT_FLAG_NORMAL, AUDIO_FLAG_NORMAL},
    {AUDIO_OUTPUT_FLAG_DIRECT, AUDIO_FLAG_DIRECT},
    {AUDIO_OUTPUT_FLAG_FAST, AUDIO_FLAG_MMAP},
};

class AudioCapturerDebugCallbackImpl : public AudioCapturerDebugCallback {
public:
    explicit AudioCapturerDebugCallbackImpl(const std::weak_ptr<AudioCapturer> &audioCapturer)
        : audioCapturer_(audioCapturer) {}

    int32_t GetCapturerDebugInfo(AudioCapturerDebugInfo &debugInfo) override
    {
        auto audioCapturer = audioCapturer_.lock();
        return audioCapturer == nullptr ? ERR_ILLEGAL_STATE : audioCapturer->GetCapturerDebugInfo(debugInfo);
    }

private:
    std::weak_ptr<AudioCapturer> audioCapturer_;
};

static void RegisterAudioCapturerDebugManager(const std::shared_ptr<AudioCapturer> &audioCapturer)
{
    if (audioCapturer == nullptr) {
        return;
    }
    auto debugCallback = std::make_shared<AudioCapturerDebugCallbackImpl>(std::weak_ptr<AudioCapturer>(audioCapturer));
    if (debugCallback == nullptr) {
        return;
    }
    (void)AudioDebugManager::GetInstance().RegisterAudioCapturer(
        reinterpret_cast<uintptr_t>(audioCapturer.get()), debugCallback);
}

AudioCapturer::~AudioCapturer() = default;

AudioCapturerPrivate::~AudioCapturerPrivate()
{
    AUDIO_INFO_LOG("~AudioCapturerPrivate");
    std::shared_ptr<InputDeviceChangeWithInfoCallbackImpl> inputDeviceChangeCallback = inputDeviceChangeCallback_;
    if (inputDeviceChangeCallback != nullptr) {
        inputDeviceChangeCallback->UnsetAudioCapturerObj();
    }
    AudioPolicyManager::GetInstance().UnregisterDeviceChangeWithInfoCallback(sessionID_);
    CapturerState state = GetStatus();
    if (state != CAPTURER_RELEASED && state != CAPTURER_NEW) {
        Release();
    }
    AudioPolicyManager::GetInstance().RemoveClientTrackerStub(sessionID_);
    if (audioStateChangeCallback_ != nullptr) {
        audioStateChangeCallback_->HandleCapturerDestructor();
    }
    DumpFileUtil::CloseDumpFile(&dumpFile_);
    DumpFileUtil::CloseDumpFile(&dumpProcessFile_);
    DumpFileUtil::CloseDumpFile(&dumpMicInFile_);
    DumpFileUtil::CloseDumpFile(&dumpEcFile_);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(AudioStreamType audioStreamType)
{
    AppInfo appInfo = {};
    return Create(audioStreamType, appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(AudioStreamType audioStreamType, const AppInfo &appInfo)
{
    std::shared_ptr<AudioCapturer> sharedCapturer = std::make_shared<AudioCapturerPrivate>(audioStreamType,
        appInfo, true);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(sharedCapturer != nullptr, nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_NULL_POINTER, "capturer is nullptr", true),
        "capturer is nullptr");
    return std::make_unique<SharedCapturerWrapper>(sharedCapturer);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &options)
{
    AppInfo appInfo = {};
    return Create(options, appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &options, const std::string cachePath)
{
    AppInfo appInfo = {};
    return Create(options, appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &options,
    const std::string cachePath, const AppInfo &appInfo)
{
    return Create(options, appInfo);
}

std::unique_ptr<AudioCapturer> AudioCapturer::Create(const AudioCapturerOptions &options,
    const AppInfo &appInfo)
{
    auto tempSharedPtr = CreateCapturer(options, appInfo);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(tempSharedPtr != nullptr, nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_NULL_POINTER, "capturer is nullptr", true),
        "capturer is nullptr");

    return std::make_unique<SharedCapturerWrapper>(tempSharedPtr);
}

void AudioCapturerPrivate::SetCapturerInfoByOptions(const AudioCapturerOptions &capturerOptions,
    const AppInfo &appInfo)
{
    capturerInfo_.sourceType = capturerOptions.capturerInfo.sourceType;
    capturerInfo_.capturerFlags = capturerOptions.capturerInfo.capturerFlags;
    capturerInfo_.originalFlag = ((capturerOptions.capturerInfo.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION) &&
        (capturerOptions.capturerInfo.capturerFlags == AUDIO_FLAG_MMAP)) ?
        AUDIO_FLAG_NORMAL : capturerOptions.capturerInfo.capturerFlags;
    capturerInfo_.samplingRate = capturerOptions.streamInfo.samplingRate;
    capturerInfo_.recorderType = capturerOptions.capturerInfo.recorderType;
    capturerInfo_.isLoopback = capturerOptions.capturerInfo.isLoopback;
    capturerInfo_.loopbackMode = capturerOptions.capturerInfo.loopbackMode;
    capturerInfo_.loopBackEffectEnabled = capturerOptions.capturerInfo.loopBackEffectEnabled;
    // InitPlaybackCapturer will be replaced by UpdatePlaybackCaptureConfig.
    filterConfig_ = capturerOptions.playbackCaptureConfig;
    strategy_ = capturerOptions.strategy;
    mixWithWakeUp_ = capturerOptions.mixWithWakeUp;
    capturerInfo_.mixWithWakeUp = mixWithWakeUp_;
}

// LCOV_EXCL_START
static inline bool IsMicInEcRequested(const AudioCapturerOptions &capturerOptions)
{
    return capturerOptions.micInStreamInfo.channels > CHANNEL_UNKNOW ||
        capturerOptions.ecStreamInfo.channels > CHANNEL_UNKNOW;
}

static inline bool IsMicInRequested(const AudioCapturerOptions &capturerOptions)
{
    return capturerOptions.micInStreamInfo.channels > CHANNEL_UNKNOW;
}

static bool CheckMicInEcParam(SourceType sourceType, const AudioCapturerOptions &capturerOptions)
{
    if (sourceType == SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT &&
        (capturerOptions.ecStreamInfo.samplingRate != capturerOptions.streamInfo.samplingRate ||
        capturerOptions.ecStreamInfo.format != capturerOptions.streamInfo.format)) {
        AUDIO_ERR_LOG("Create failed: SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT"
            "can only be samplingRate and format same");
        return false;
    }

    if (sourceType == SOURCE_TYPE_VOICE_RECOGNITION && IsMicInEcRequested(capturerOptions)) {
        if (capturerOptions.micInStreamInfo.channels > CHANNEL_UNKNOW &&
            (capturerOptions.micInStreamInfo.samplingRate != capturerOptions.streamInfo.samplingRate ||
            capturerOptions.micInStreamInfo.format != capturerOptions.streamInfo.format)) {
            AUDIO_ERR_LOG("Create failed: mic-in samplingRate and format should follow process stream");
            return false;
        }
        if (capturerOptions.ecStreamInfo.channels > CHANNEL_UNKNOW &&
            (capturerOptions.ecStreamInfo.samplingRate != capturerOptions.streamInfo.samplingRate ||
            capturerOptions.ecStreamInfo.format != capturerOptions.streamInfo.format)) {
            AUDIO_ERR_LOG("Create failed: ec samplingRate and format should follow process stream");
            return false;
        }
    }

    if (sourceType == SOURCE_TYPE_CAMCORDER && IsMicInRequested(capturerOptions)) {
        if ((capturerOptions.micInStreamInfo.samplingRate != capturerOptions.streamInfo.samplingRate ||
            capturerOptions.micInStreamInfo.format != capturerOptions.streamInfo.format)) {
            AUDIO_ERR_LOG("Create failed: camcorder mic-in samplingRate and format should follow process stream");
            return false;
        }
    }

    return true;
}
 
static inline void FillEcParams(AudioCapturerParams &params, const AudioStreamInfo &ec)
{
    params.audioEcSampleFormat = ec.format;
    params.ecSamplingRate = ec.samplingRate;
    params.audioEcChannel = (ec.channels == AudioChannel::CHANNEL_3) ? AudioChannel::STEREO : ec.channels;
    params.audioEcEncoding = ec.encoding;
    params.ecChannelLayout = ec.channelLayout;
}

static inline void FillMicInParams(AudioCapturerParams &params, const AudioStreamInfo &micIn)
{
    params.audioMicInSampleFormat = micIn.format;
    params.micInSamplingRate = micIn.samplingRate;
    params.audioMicInChannel = (micIn.channels == AudioChannel::CHANNEL_3) ? AudioChannel::STEREO : micIn.channels;
    params.audioMicInEncoding = micIn.encoding;
    params.micInChannelLayout = micIn.channelLayout;
}

struct MicInEcBufferLayout {
    size_t byteSizePerSample = 0;
    size_t processChannels = 0;
    size_t micInChannels = 0;
    size_t ecChannels = 0;
    size_t byteSizePerFrame = 0;
};

struct MicInEcBufferSizes {
    size_t processBufSize = 0;
    size_t micInBufSize = 0;
    size_t ecBufSize = 0;
};

struct DeinterleaveTargetBuffers {
    BufferDesc &processBufDesc;
    BufferDesc &micInBufDesc;
    BufferDesc &ecBufferDesc;
};

struct DeinterleaveDumpFiles {
    FILE *processDumpFile = nullptr;
    FILE *micInDumpFile = nullptr;
    FILE *ecDumpFile = nullptr;
};

static int32_t ParseMicInEcBufferLayout(const AudioStreamParams &params, MicInEcBufferLayout &layout)
{
    int32_t sampleByteSize = GetFormatByteSize(params.format);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(sampleByteSize > 0, ERROR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_INVALID_PARAM, "invalid sample format", true),
        "invalid sample format");
    layout.processChannels = static_cast<size_t>(params.channels);
    layout.micInChannels = static_cast<size_t>(params.micInChannels);
    layout.ecChannels = static_cast<size_t>(params.ecChannels);
    size_t totalChannels = layout.processChannels + layout.micInChannels + layout.ecChannels;
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(layout.processChannels > 0, ERROR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_INVALID_PARAM, "invalid process channels", true),
        "invalid process channels");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(totalChannels > layout.processChannels, ERROR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_INVALID_PARAM, "micIn and ec channels are not configured", true),
        "micIn and ec channels are not configured");
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(totalChannels <= AUDIO_MAX_CAPTURE_CHANNELS, ERROR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_INVALID_PARAM, "total channels over limit", true),
        "total channels over limit");
    layout.byteSizePerSample = static_cast<size_t>(sampleByteSize);
    layout.byteSizePerFrame = layout.byteSizePerSample * totalChannels;
    return SUCCESS;
}

static inline int32_t GetMicInEcBufferLayoutFromStream(const std::shared_ptr<IAudioStream> &currentStream,
    MicInEcBufferLayout &layout)
{
    AudioStreamParams audioStreamParams;
    int32_t ret = currentStream->GetAudioStreamInfo(audioStreamParams);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_INVALID_PARAM, "GetAudioStreamInfo failed", true), "GetAudioStreamInfo failed");
    ret = ParseMicInEcBufferLayout(audioStreamParams, layout);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_INVALID_PARAM, "ParseMicInEcBufferLayout fail", true), "ParseMicInEcBufferLayout fail");
    return SUCCESS;
}

static inline int32_t ValidateDeinterleaveOutputBuffer(const DeinterleaveTargetBuffers &targetBuffers,
    const MicInEcBufferSizes &bufferSizes)
{
    CHECK_AND_RETURN_RET_LOG(targetBuffers.processBufDesc.buffer != nullptr &&
        targetBuffers.processBufDesc.bufLength >= bufferSizes.processBufSize,
        ERROR_INVALID_PARAM, "process buffer is invalid");
    CHECK_AND_RETURN_RET_LOG(bufferSizes.micInBufSize == 0 || (targetBuffers.micInBufDesc.buffer != nullptr &&
        targetBuffers.micInBufDesc.bufLength >= bufferSizes.micInBufSize),
        ERROR_INVALID_PARAM, "micIn buffer is invalid");
    CHECK_AND_RETURN_RET_LOG(bufferSizes.ecBufSize == 0 || (targetBuffers.ecBufferDesc.buffer != nullptr &&
        targetBuffers.ecBufferDesc.bufLength >= bufferSizes.ecBufSize),
        ERROR_INVALID_PARAM, "ec buffer is invalid");
    return SUCCESS;
}

static inline void UpdateDeinterleaveDataLength(BufferDesc &processBufDesc, BufferDesc &micInBufDesc,
    BufferDesc &ecBufferDesc, const MicInEcBufferSizes &bufferSizes)
{
    processBufDesc.dataLength = bufferSizes.processBufSize;
    micInBufDesc.dataLength = bufferSizes.micInBufSize;
    ecBufferDesc.dataLength = bufferSizes.ecBufSize;
}

static void DumpDeinterleaveBuffersImpl(const DeinterleaveDumpFiles &dumpFiles,
    const DeinterleaveTargetBuffers &targetBuffers, const MicInEcBufferSizes &bufferSizes)
{
    if (bufferSizes.processBufSize > 0) {
        DumpFileUtil::WriteDumpFile(dumpFiles.processDumpFile,
            static_cast<void *>(targetBuffers.processBufDesc.buffer),
            bufferSizes.processBufSize);
    }
    if (bufferSizes.micInBufSize > 0) {
        DumpFileUtil::WriteDumpFile(dumpFiles.micInDumpFile,
            static_cast<void *>(targetBuffers.micInBufDesc.buffer),
            bufferSizes.micInBufSize);
    }
    if (bufferSizes.ecBufSize > 0) {
        DumpFileUtil::WriteDumpFile(dumpFiles.ecDumpFile,
            static_cast<void *>(targetBuffers.ecBufferDesc.buffer), bufferSizes.ecBufSize);
    }
}

static inline void DumpDeinterleaveBuffers(const DeinterleaveDumpFiles &dumpFiles,
    const DeinterleaveTargetBuffers &targetBuffers, const MicInEcBufferSizes &bufferSizes)
{
    DumpDeinterleaveBuffersImpl(dumpFiles, targetBuffers, bufferSizes);
}

static int32_t FillDegradeFallbackSilentBuffersImpl(const BufferDesc &srcBufDesc,
    BufferDesc &processBufDesc, BufferDesc &micInBufDesc, BufferDesc &ecBufferDesc,
    const MicInEcBufferSizes &bufferSizes)
{
    int32_t copyRet = memcpy_s(processBufDesc.buffer, processBufDesc.bufLength, srcBufDesc.buffer,
        bufferSizes.processBufSize);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(copyRet == EOK, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_OPERATION_FAILED, "copy process frame failed", true),
        "copy process frame failed");
    if (bufferSizes.micInBufSize > 0) {
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(micInBufDesc.buffer != nullptr, ERROR_INVALID_PARAM,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_SEND_DATA_INVALID_PARAM, "micIn buffer is null", true),
            "micIn buffer is null");
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(memset_s(micInBufDesc.buffer, micInBufDesc.bufLength, 0,
            bufferSizes.micInBufSize) == EOK, ERR_OPERATION_FAILED,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_SEND_DATA_OPERATION_FAILED, "clear micIn buffer failed", true),
            "clear micIn buffer failed");
    }
    if (bufferSizes.ecBufSize > 0) {
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ecBufferDesc.buffer != nullptr, ERROR_INVALID_PARAM,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_SEND_DATA_INVALID_PARAM, "ec buffer is null", true),
            "ec buffer is null");
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(memset_s(ecBufferDesc.buffer, ecBufferDesc.bufLength, 0,
            bufferSizes.ecBufSize) == EOK, ERR_OPERATION_FAILED,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_SEND_DATA_OPERATION_FAILED, "clear ec buffer failed", true),
            "clear ec buffer failed");
    }
    return SUCCESS;
}

static inline int32_t FillDegradeFallbackSilentBuffers(const BufferDesc &srcBufDesc,
    BufferDesc &processBufDesc, BufferDesc &micInBufDesc, BufferDesc &ecBufferDesc,
    const MicInEcBufferSizes &bufferSizes)
{
    return FillDegradeFallbackSilentBuffersImpl(srcBufDesc, processBufDesc, micInBufDesc, ecBufferDesc, bufferSizes);
}

static int32_t DeinterleaveMicInEcFrames(const BufferDesc &srcBufDesc, const MicInEcBufferLayout &layout,
    const MicInEcBufferSizes &bufferSizes, DeinterleaveTargetBuffers &targetBuffers)
{
    size_t processFrameSize = layout.processChannels * layout.byteSizePerSample;
    size_t micInFrameSize = layout.micInChannels * layout.byteSizePerSample;
    size_t ecFrameSize = layout.ecChannels * layout.byteSizePerSample;
    size_t frameCount = bufferSizes.processBufSize / processFrameSize;
    for (size_t frame = 0; frame < frameCount; ++frame) {
        uint8_t *srcFramePos = srcBufDesc.buffer + frame * layout.byteSizePerFrame;
        if (processFrameSize > 0) {
            int32_t copyRet = memcpy_s(targetBuffers.processBufDesc.buffer + frame * processFrameSize, processFrameSize,
                srcFramePos, processFrameSize);
            CHECK_AND_CALL_FUNC_RETURN_RET_LOG(copyRet == EOK, ERR_OPERATION_FAILED,
                StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                    RECORD_SEND_DATA_OPERATION_FAILED, "copy process frame failed", true),
                "copy process frame failed");
            srcFramePos += processFrameSize;
        }
        if (micInFrameSize > 0) {
            int32_t copyRet = memcpy_s(targetBuffers.micInBufDesc.buffer + frame * micInFrameSize, micInFrameSize,
                srcFramePos, micInFrameSize);
            CHECK_AND_CALL_FUNC_RETURN_RET_LOG(copyRet == EOK, ERR_OPERATION_FAILED,
                StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                    RECORD_SEND_DATA_OPERATION_FAILED, "copy micIn frame failed", true),
                "copy micIn frame failed");
            srcFramePos += micInFrameSize;
        }
        if (ecFrameSize > 0) {
            int32_t copyRet = memcpy_s(targetBuffers.ecBufferDesc.buffer + frame * ecFrameSize, ecFrameSize,
                srcFramePos, ecFrameSize);
            CHECK_AND_CALL_FUNC_RETURN_RET_LOG(copyRet == EOK, ERR_OPERATION_FAILED,
                StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                    RECORD_SEND_DATA_OPERATION_FAILED, "copy ec frame failed", true),
                "copy ec frame failed");
        }
    }
    return SUCCESS;
}

static inline uint32_t GetClientDumpChannels(const AudioCapturerInfo &capturerInfo,
    const AudioStreamParams &audioStreamParams, uint32_t appChannels)
{
    if (capturerInfo.sourceType == SOURCE_TYPE_VOICE_RECOGNITION &&
        (audioStreamParams.micInChannels > CHANNEL_UNKNOW || audioStreamParams.ecChannels > CHANNEL_UNKNOW)) {
        return appChannels + static_cast<uint32_t>(audioStreamParams.micInChannels) +
            static_cast<uint32_t>(audioStreamParams.ecChannels);
    }
    return appChannels;
}

static inline bool IsVoiceRecognitionMicInEcRequest(const AudioCapturerInfo &capturerInfo,
    const AudioStreamParams &audioStreamParams)
{
    return capturerInfo.sourceType == SOURCE_TYPE_VOICE_RECOGNITION &&
        (audioStreamParams.micInChannels > CHANNEL_UNKNOW || audioStreamParams.ecChannels > CHANNEL_UNKNOW);
}

static bool IsVoiceRecognitionMicInEcSupportedInputDeviceTypeImpl(DeviceType deviceType)
{
    switch (deviceType) {
        case DEVICE_TYPE_MIC:
        case DEVICE_TYPE_WAKEUP:
        case DEVICE_TYPE_WIRED_HEADSET:
        case DEVICE_TYPE_WIRED_HEADPHONES:
        case DEVICE_TYPE_USB_HEADSET:
        case DEVICE_TYPE_BLUETOOTH_SCO:
            return true;
        default:
            return false;
    }
}

static inline bool IsVoiceRecognitionMicInEcSupportedInputDeviceType(DeviceType deviceType)
{
    return IsVoiceRecognitionMicInEcSupportedInputDeviceTypeImpl(deviceType);
}

static inline bool IsVoiceRecognitionMicInEcSupportedInputDeviceDesc(
    const std::shared_ptr<AudioDeviceDescriptor> &deviceDesc)
{
    return deviceDesc != nullptr && deviceDesc->networkId_ == LOCAL_NETWORK_ID &&
        IsVoiceRecognitionMicInEcSupportedInputDeviceType(deviceDesc->deviceType_);
}

static void UpdateVoiceRecognitionMicInEcDegradeFlagImpl(const AudioCapturerInfo &capturerInfo,
    AudioStreamParams &audioStreamParams)
{
    if (!IsVoiceRecognitionMicInEcRequest(capturerInfo, audioStreamParams)) {
        return;
    }

    DeviceType activeInputDevice = AudioPolicyManager::GetInstance().GetActiveInputDevice();
    if (activeInputDevice != DEVICE_TYPE_INVALID && activeInputDevice != DEVICE_TYPE_NONE) {
        audioStreamParams.isVoiceRecognitionMicInEcDegrade =
            !IsVoiceRecognitionMicInEcSupportedInputDeviceType(activeInputDevice);
        if (audioStreamParams.isVoiceRecognitionMicInEcDegrade) {
            return;
        }
    }

    AudioCapturerInfo queryInfo = capturerInfo;
    queryInfo.samplingRate = static_cast<AudioSamplingRate>(audioStreamParams.samplingRate);
    queryInfo.encodingType = audioStreamParams.encoding;
    queryInfo.channelLayout = audioStreamParams.channelLayout;
    auto preferredInputDevices = AudioPolicyManager::GetInstance().GetPreferredInputDeviceDescriptors(queryInfo);
    for (const auto &deviceDesc : preferredInputDevices) {
        if (deviceDesc == nullptr) {
            continue;
        }
        audioStreamParams.isVoiceRecognitionMicInEcDegrade =
            !IsVoiceRecognitionMicInEcSupportedInputDeviceDesc(deviceDesc);
        return;
    }
}

static inline void UpdateVoiceRecognitionMicInEcDegradeFlag(const AudioCapturerInfo &capturerInfo,
    AudioStreamParams &audioStreamParams)
{
    UpdateVoiceRecognitionMicInEcDegradeFlagImpl(capturerInfo, audioStreamParams);
}

static inline bool IsDegradeVoiceRecognitionMicInEcRequest(const AudioCapturerInfo &capturerInfo,
    const AudioStreamParams &audioStreamParams)
{
    return IsVoiceRecognitionMicInEcRequest(capturerInfo, audioStreamParams) &&
        audioStreamParams.isVoiceRecognitionMicInEcDegrade;
}

static inline void FillCreateCapturerParams(AudioCapturerParams &params, const AudioCapturerOptions &capturerOptions)
{
    FillEcParams(params, capturerOptions.ecStreamInfo);
    FillMicInParams(params, capturerOptions.micInStreamInfo);
}

static void ReportWakeupEvent(SourceType sourceType,
    int32_t stage, int32_t wakeupResult, int32_t wakeupErrCode)
{
    CHECK_AND_RETURN((sourceType == SOURCE_TYPE_WAKEUP || sourceType == SOURCE_TYPE_VOICE_RECOGNITION ||
        sourceType == SOURCE_TYPE_UNPROCESSED_VOICE_ASSISTANT));
    AUDIO_INFO_LOG("sourceType: %{public}d, stage: %{public}d, result: %{public}d",
        sourceType, stage, wakeupResult);
    AudioWakeupTrackInfo audioWakeupTrackInfo;
    audioWakeupTrackInfo.stage = static_cast<AudioWakeupStageCode>(stage);
    audioWakeupTrackInfo.result = static_cast<AudioWakeupResult>(wakeupResult);
    audioWakeupTrackInfo.errCode = static_cast<AudioWakeupErrorCode>(wakeupErrCode);
    AudioSystemClientEngineManager::GetInstance().ReportWakeupEvent(audioWakeupTrackInfo);
}

static void InitializeCapturer(std::shared_ptr<AudioCapturerPrivate>& capturer,
    const AudioCapturerOptions& capturerOptions)
{
    if (capturer != nullptr && AudioChannel::CHANNEL_3 == capturerOptions.streamInfo.channels) {
        capturer->isChannelChange_ = true;
    }

    AudioWakeupErrorCode errCode = (capturer == nullptr)
                    ? WAKEUP_TRACK_FWK_CREATE_ERROR
                    : WAKEUP_TRACK_FWK_NO_ERROR;
    AudioWakeupResult result = (capturer == nullptr)
                        ? WAKEUP_RESULT_FAIL
                        : WAKEUP_RESULT_DEFAULT;
    ReportWakeupEvent(capturerOptions.capturerInfo.sourceType,
        STAGE_FWK_CREATE_EXIT, result, errCode);
}

int32_t AudioCapturer::CheckAndReportErrorEvent(SourceType sourceType,
    const AudioCapturerOptions &capturerOptions)
{
    if (sourceType == SOURCE_TYPE_VIRTUAL_CAPTURE) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_INVALID_PARAM, "Invalid sourceType SOURCE_TYPE_VIRTUAL_CAPTURE", true);
        AUDIO_ERR_LOG("Invalid sourceType %{public}d!", sourceType);
        return ERR_INVALID_PARAM;
    }
    if (sourceType < SOURCE_TYPE_MIC || sourceType > SOURCE_TYPE_MAX || sourceType == AUDIO_SOURCE_TYPE_INVALID_5) {
        AudioCapturer::SendCapturerCreateError(sourceType, ERR_INVALID_PARAM);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_INVALID_PARAM, "Invalid sourceType range", true);
        AUDIO_ERR_LOG("Invalid sourceType %{public}d!", sourceType);
        return ERR_INVALID_PARAM;
    }
    if (sourceType == SOURCE_TYPE_ULTRASONIC && getuid() != UID_MSDP_SA) {
        AudioCapturer::SendCapturerCreateError(sourceType, ERR_INVALID_PARAM);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_PERMISSION_DENIED, "ULTRASONIC can only create by MSDP", true);
        AUDIO_ERR_LOG("Create failed: SOURCE_TYPE_ULTRASONIC can only be used by MSDP");
        return ERR_PERMISSION_DENIED;
    }

    if (!CheckMicInEcParam(sourceType, capturerOptions)) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_INVALID_PARAM, "CheckMicInEcParam failed", true);
        return ERR_INVALID_PARAM;
    }
    return SUCCESS;
}

std::shared_ptr<AudioCapturer> AudioCapturer::CreateCapturer(const AudioCapturerOptions &capturerOptions,
    const AppInfo &appInfo)
{
    Trace trace("KeyAction AudioCapturer::Create");
    auto sourceType = capturerOptions.capturerInfo.sourceType;
    ReportWakeupEvent(sourceType, STAGE_FWK_CREATE_ENTER, WAKEUP_RESULT_DEFAULT, WAKEUP_TRACK_FWK_NO_ERROR);
    int32_t ret = AudioCapturer::CheckAndReportErrorEvent(sourceType, capturerOptions);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_NULL_POINTER, "Check fail", true), "Check fail");
    AUDIO_INFO_LOG("StreamClientState for Capturer::CreateCapturer sourceType:%{public}d, capturerFlags:%{public}d, "
        "AppInfo:[%{public}d] [%{public}s] [%{public}s], ", sourceType, capturerOptions.capturerInfo.capturerFlags,
        appInfo.appUid, appInfo.appTokenId == 0 ? "T" : "F", appInfo.appFullTokenId == 0 ? "T" : "F");

    AudioStreamType audioStreamType = FindStreamTypeBySourceType(sourceType);
    AudioCapturerParams params;
    params.preferredInputDevice = capturerOptions.preferredInputDevice;
    params.audioSampleFormat = capturerOptions.streamInfo.format;
    params.samplingRate = capturerOptions.streamInfo.samplingRate;
    params.audioChannel = AudioChannel::CHANNEL_3 == capturerOptions.streamInfo.channels ? AudioChannel::STEREO :
        capturerOptions.streamInfo.channels;
    params.audioEncoding = capturerOptions.streamInfo.encoding;
    params.channelLayout = capturerOptions.streamInfo.channelLayout;
    FillCreateCapturerParams(params, capturerOptions);
    auto capturer = std::make_shared<AudioCapturerPrivate>(audioStreamType, appInfo, false);

    if (capturer == nullptr) {
        AudioCapturer::SendCapturerCreateError(sourceType, ERR_OPERATION_FAILED);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_NULL_POINTER, "Failed to create capturer object", true);
        AUDIO_ERR_LOG("Failed to create capturer object");
        return nullptr;
    }
    capturer->SetCapturerInfoByOptions(capturerOptions, appInfo);
    if (capturer->SetParams(params) != SUCCESS) {
        AudioCapturer::SendCapturerCreateError(sourceType, ERR_OPERATION_FAILED);
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "SetParams failed", true);
        capturer = nullptr;
    }
    InitializeCapturer(capturer, capturerOptions);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(capturer != nullptr, capturer,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_NULL_POINTER, "capturer is nullptr", true),
        "capturer is nullptr");
    RegisterAudioCapturerDebugManager(capturer);
    return capturer;
}

// This will be called in Create and after Create.
int32_t AudioCapturerPrivate::UpdatePlaybackCaptureConfig(const AudioPlaybackCaptureConfig &config)
{
    // UpdatePlaybackCaptureConfig will only work for InnerCap streams.
    if (capturerInfo_.sourceType != SOURCE_TYPE_PLAYBACK_CAPTURE) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_INVALID_OPERATION, "This is not a PLAYBACK_CAPTURE stream", true);
        AUDIO_WARNING_LOG("This is not a PLAYBACK_CAPTURE stream.");
        return ERR_INVALID_OPERATION;
    }

#ifdef HAS_FEATURE_INNERCAPTURER
    if (config.filterOptions.usages.size() == 0 && config.filterOptions.pids.size() == 0) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_INVALID_PARAM, "Both usages and pids are empty", true);
        AUDIO_WARNING_LOG("Both usages and pids are empty!");
    }

    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStream_ != nullptr, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_ILLEGAL_STATE, "Failed with null audioStream_", true),
        "Failed with null audioStream_");

    return audioStream_->UpdatePlaybackCaptureConfig(config);
#else
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_NOT_SUPPORTED, "Inner capture is not supported", true);
    AUDIO_WARNING_LOG("Inner capture is not supported.");
    return ERR_NOT_SUPPORTED;
#endif
}

void AudioCapturer::SendCapturerCreateError(const SourceType &sourceType,
    const int32_t &errorCode)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::AUDIO_STREAM_CREATE_ERROR_STATS,
        Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);
    bean->Add("IS_PLAYBACK", 0);
    bean->Add("CLIENT_UID", static_cast<int32_t>(getuid()));
    bean->Add("STREAM_TYPE", sourceType);
    bean->Add("ERROR_CODE", errorCode);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

AudioCapturerPrivate::AudioCapturerPrivate(AudioStreamType audioStreamType, const AppInfo &appInfo, bool createStream)
{
    if (audioStreamType < STREAM_VOICE_CALL || audioStreamType > STREAM_ALL) {
        AUDIO_WARNING_LOG("audioStreamType is invalid!");
    }
    audioStreamType_ = audioStreamType;
    auto iter = streamToSource_.find(audioStreamType);
    if (iter != streamToSource_.end()) {
        capturerInfo_.sourceType = iter->second;
    }
    appInfo_ = appInfo;
    if (!(appInfo_.appPid)) {
        appInfo_.appPid = getpid();
    }

    if (appInfo_.appUid < 0) {
        appInfo_.appUid = static_cast<int32_t>(getuid());
    }
    if (createStream) {
        AudioStreamParams tempParams = {};
        audioStream_ = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, tempParams, audioStreamType_,
            appInfo_.appUid);
        AUDIO_INFO_LOG("create normal stream for old mode.");
    }

    capturerProxyObj_ = std::make_shared<AudioCapturerProxyObj>();
    if (!capturerProxyObj_) {
        AUDIO_WARNING_LOG("AudioCapturerProxyObj Memory Allocation Failed !!");
    }
}

int32_t AudioCapturerPrivate::GetFrameCount(uint32_t &frameCount) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "currentStream is nullptr", true),
        "currentStream is nullptr");
    return currentStream->GetFrameCount(frameCount);
}

int32_t AudioCapturerPrivate::InitAndReportErrorEvent(AudioStreamParams audioStreamParams,
    IAudioStream::StreamClass streamClass, const AudioCapturerParams params)
{
    int32_t ret = InitAudioStream(audioStreamParams);
    if (ret != SUCCESS) {
        // if the normal stream creation fails, return fail, other try create normal stream
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(streamClass != IAudioStream::PA_STREAM, ret,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CREATE_OPERATION_FAILED, "Normal Stream Init Failed", true),
            "Normal Stream Init Failed");
        ret = HandleCreateFastStreamError(audioStreamParams);
    }
    CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(ret == SUCCESS, ret, HILOG_COMM_ERROR("[SetParams]InitAudioStream failed"),
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "[SetParams]InitAudioStream failed", true));

    RegisterCapturerPolicyServiceDiedCallback(audioStream_);

    ret = InitSessionAndOpenDumpFiles(params, audioStreamParams);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "InitSessionAndOpenDumpFiles failed", true),
        "InitSessionAndOpenDumpFiles failed");

    ret = InitInputDeviceChangeCallback();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "Init input device change callback failed", true),
        "Init input device change callback failed");
    return ret;
}

int32_t AudioCapturerPrivate::SetParams(const AudioCapturerParams params)
{
    Trace trace("AudioCapturer::SetParams");
    AUDIO_INFO_LOG("enter");
    std::shared_lock<std::shared_mutex> lockShared;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lockShared = std::shared_lock<std::shared_mutex>(capturerMutex_);
    }
    AudioStreamParams audioStreamParams = ConvertToAudioStreamParams(params);

    // Create Client
    std::shared_ptr<AudioStreamDescriptor> streamDesc = ConvertToStreamDescriptor(audioStreamParams);
    streamDesc->preferredInputDevice = AudioDeviceDescriptor(params.preferredInputDevice);
    int32_t ret = IAudioStream::CheckCapturerAudioStreamInfo(audioStreamParams);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_INVALID_PARAM, "CheckCapturerAudioStreamInfo fail", true),
        "CheckCapturerAudioStreamInfo fail!");

    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    ret = AudioPolicyManager::GetInstance().CreateCapturerClient(streamDesc, flag, audioStreamParams.originalSessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "CreateCapturerClient failed", true),
        "CreateCapturerClient failed");
    UpdateVoiceRecognitionMicInEcDegradeFlag(capturerInfo_, audioStreamParams);
    HILOG_COMM_INFO("StreamClientState for Capturer::CreateClient. id %{public}u, flag :%{public}u",
        audioStreamParams.originalSessionId, flag);

    IAudioStream::StreamClass streamClass = DecideStreamClassAndUpdateCapturerInfo(flag);
    // check AudioStreamParams for fast stream
    if (audioStream_ == nullptr) {
        audioStream_ = IAudioStream::GetRecordStream(streamClass, audioStreamParams, audioStreamType_,
            appInfo_.appUid);
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStream_ != nullptr, ERR_INVALID_PARAM,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CREATE_NULL_POINTER, "SetParams GetRecordStream faied", true),
            "SetParams GetRecordStream faied.");
        AUDIO_INFO_LOG("IAudioStream::GetStream success");
    }
    ret = InitAndReportErrorEvent(audioStreamParams, streamClass, params);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "InitAndReportErrorEvent fail", true),
        "InitAndReportErrorEvent fail");

    return InitAudioInterruptCallback();
}

int32_t AudioCapturerPrivate::InitSessionAndOpenDumpFiles(const AudioCapturerParams &params,
    const AudioStreamParams &audioStreamParams)
{
    if (audioStream_->GetAudioSessionID(sessionID_) != 0) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_INVALID_INDEX, "GetAudioSessionID failed", true);
        AUDIO_ERR_LOG("GetAudioSessionID failed!");
        return ERR_INVALID_INDEX;
    }
    uint32_t dumpChannels = GetClientDumpChannels(capturerInfo_, audioStreamParams,
        static_cast<uint32_t>(params.audioChannel));
    AUDIO_INFO_LOG("open cap_client_out dump, appChannels:%{public}u, micInChannels:%{public}u, "
        "ecChannels:%{public}u, dumpChannels:%{public}u", static_cast<uint32_t>(params.audioChannel),
        static_cast<uint32_t>(audioStreamParams.micInChannels), static_cast<uint32_t>(audioStreamParams.ecChannels),
        dumpChannels);
    OpenCaptureDumpFiles(params, audioStreamParams, dumpChannels);
    return SUCCESS;
}

void AudioCapturerPrivate::OpenCaptureDumpFiles(const AudioCapturerParams &params,
    const AudioStreamParams &audioStreamParams, uint32_t dumpChannels)
{
    // eg: 100009_44100_9_1_cap_client_out.pcm (voice recognition with micIn/ec)
    std::string dumpFileName = std::to_string(sessionID_) + "_" + std::to_string(params.samplingRate) + "_" +
        std::to_string(dumpChannels) + "_" + std::to_string(params.audioSampleFormat) + "_cap_client_out.pcm";
    DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_CLIENT_PARA, dumpFileName, &dumpFile_);
    if (capturerInfo_.sourceType != SOURCE_TYPE_VOICE_RECOGNITION) {
        return;
    }
    if (audioStreamParams.micInChannels > CHANNEL_UNKNOW || audioStreamParams.ecChannels > CHANNEL_UNKNOW) {
        std::string processDumpFileName = std::to_string(sessionID_) + "_" +
            std::to_string(audioStreamParams.samplingRate) + "_" +
            std::to_string(audioStreamParams.channels) + "_" +
            std::to_string(audioStreamParams.format) + "_cap_client_process_out.pcm";
        DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_CLIENT_PARA, processDumpFileName, &dumpProcessFile_);
    }
    if (audioStreamParams.micInChannels > CHANNEL_UNKNOW) {
        std::string micInDumpFileName = std::to_string(sessionID_) + "_" +
            std::to_string(audioStreamParams.micInSamplingRate) + "_" +
            std::to_string(audioStreamParams.micInChannels) + "_" +
            std::to_string(audioStreamParams.micInFormat) + "_cap_client_micin_out.pcm";
        DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_CLIENT_PARA, micInDumpFileName, &dumpMicInFile_);
    }
    if (audioStreamParams.ecChannels > CHANNEL_UNKNOW) {
        std::string ecDumpFileName = std::to_string(sessionID_) + "_" +
            std::to_string(audioStreamParams.ecSamplingRate) + "_" +
            std::to_string(audioStreamParams.ecChannels) + "_" +
            std::to_string(audioStreamParams.ecFormat) + "_cap_client_ec_out.pcm";
        DumpFileUtil::OpenDumpFile(DumpFileUtil::DUMP_CLIENT_PARA, ecDumpFileName, &dumpEcFile_);
    }
}

IAudioStream::StreamClass AudioCapturerPrivate::SetCaptureInfo(AudioStreamParams &audioStreamParams)
{
    IAudioStream::StreamClass streamClass = IAudioStream::PA_STREAM;
    if (capturerInfo_.sourceType != SOURCE_TYPE_PLAYBACK_CAPTURE) {
        capturerInfo_.originalFlag = AUDIO_FLAG_FORCED_NORMAL;
        capturerInfo_.capturerFlags = AUDIO_FLAG_NORMAL;
        streamClass = IAudioStream::PA_STREAM;
    }
    return streamClass;
}

std::shared_ptr<AudioStreamDescriptor> AudioCapturerPrivate::ConvertToStreamDescriptor(
    const AudioStreamParams &audioStreamParams)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = std::make_shared<AudioStreamDescriptor>();
    streamDesc->streamInfo_.format = static_cast<AudioSampleFormat>(audioStreamParams.format);
    streamDesc->streamInfo_.samplingRate = static_cast<AudioSamplingRate>(audioStreamParams.samplingRate);
    streamDesc->streamInfo_.channels = static_cast<AudioChannel>(audioStreamParams.channels);
    streamDesc->streamInfo_.encoding = static_cast<AudioEncodingType>(audioStreamParams.encoding);
    streamDesc->streamInfo_.channelLayout = static_cast<AudioChannelLayout>(audioStreamParams.channelLayout);
    streamDesc->micInStreamInfo_.format = static_cast<AudioSampleFormat>(audioStreamParams.micInFormat);
    streamDesc->micInStreamInfo_.samplingRate = static_cast<AudioSamplingRate>(audioStreamParams.micInSamplingRate);
    streamDesc->micInStreamInfo_.channels = static_cast<AudioChannel>(audioStreamParams.micInChannels);
    streamDesc->micInStreamInfo_.encoding = static_cast<AudioEncodingType>(audioStreamParams.micInEncoding);
    streamDesc->micInStreamInfo_.channelLayout = static_cast<AudioChannelLayout>(audioStreamParams.micInChannelLayout);
    streamDesc->ecStreamInfo_.format = static_cast<AudioSampleFormat>(audioStreamParams.ecFormat);
    streamDesc->ecStreamInfo_.samplingRate = static_cast<AudioSamplingRate>(audioStreamParams.ecSamplingRate);
    streamDesc->ecStreamInfo_.channels = static_cast<AudioChannel>(audioStreamParams.ecChannels);
    streamDesc->ecStreamInfo_.encoding = static_cast<AudioEncodingType>(audioStreamParams.ecEncoding);
    streamDesc->ecStreamInfo_.channelLayout = static_cast<AudioChannelLayout>(audioStreamParams.ecChannelLayout);
    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->capturerInfo_ = capturerInfo_;
    streamDesc->appInfo_ = appInfo_;
    streamDesc->callerUid_ = static_cast<int32_t>(getuid());
    streamDesc->callerPid_ = static_cast<int32_t>(getpid());
    streamDesc->sessionId_ = audioStreamParams.originalSessionId;
    return streamDesc;
}

IAudioStream::StreamClass AudioCapturerPrivate::DecideStreamClassAndUpdateCapturerInfo(uint32_t flag)
{
    IAudioStream::StreamClass ret = IAudioStream::StreamClass::PA_STREAM;
    if (flag & AUDIO_INPUT_FLAG_FAST) {
        if (flag & AUDIO_INPUT_FLAG_VOIP) {
            capturerInfo_.originalFlag = AUDIO_FLAG_VOIP_FAST;
            capturerInfo_.capturerFlags = AUDIO_FLAG_VOIP_FAST;
            capturerInfo_.pipeType = PIPE_TYPE_IN_VOIP;
            ret = IAudioStream::StreamClass::VOIP_STREAM;
        } else {
            capturerInfo_.originalFlag = AUDIO_FLAG_MMAP;
            capturerInfo_.capturerFlags = AUDIO_FLAG_MMAP;
            capturerInfo_.pipeType = PIPE_TYPE_IN_LOWLATENCY;
            ret = IAudioStream::StreamClass::FAST_STREAM;
        }
    } else {
        capturerInfo_.capturerFlags = AUDIO_FLAG_NORMAL;
        capturerInfo_.pipeType = PIPE_TYPE_IN_NORMAL;
    }
    AUDIO_INFO_LOG("Route flag: %{public}u, streamClass: %{public}d, capturerFlags: %{public}d, pipeType: %{public}d",
        flag, ret, capturerInfo_.capturerFlags, capturerInfo_.pipeType);
    return ret;
}

int32_t AudioCapturerPrivate::InitInputDeviceChangeCallback()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(GetCurrentInputDevicesInner(currentDeviceInfo_) == SUCCESS, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "Get current device info failed", true),
        "Get current device info failed");

    if (!inputDeviceChangeCallback_) {
        inputDeviceChangeCallback_ = std::make_shared<InputDeviceChangeWithInfoCallbackImpl>();
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(inputDeviceChangeCallback_ != nullptr, ERROR,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CREATE_MEMORY_ALLOC_FAILED, "Memory allocation failed", true),
            "Memory allocation failed");
    }

    inputDeviceChangeCallback_->SetAudioCapturerObj(weak_from_this());

    uint32_t sessionId;
    int32_t ret = GetAudioStreamId(sessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_INVALID_HANDLE, "Get sessionId failed", true),
        "Get sessionId failed");

    ret = AudioPolicyManager::GetInstance().RegisterDeviceChangeWithInfoCallback(sessionId,
        inputDeviceChangeCallback_);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "Register failed", true),
        "Register failed");

    return SUCCESS;
}

int32_t AudioCapturerPrivate::SetInputDevice(DeviceType deviceType) const
{
    AUDIO_INFO_LOG("AudioCapturerPrivate::SetInputDevice %{public}d", deviceType);
    if (audioStream_ == NULL) {
        return SUCCESS;
    }
    uint32_t currentSessionID = 0;
    audioStream_->GetAudioSessionID(currentSessionID);
    int32_t ret = AudioPolicyManager::GetInstance().SetInputDevice(deviceType, currentSessionID,
        capturerInfo_.sourceType, GetStatus() == CAPTURER_RUNNING);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            ERR_RECORD_DEVICE_SWITCH_OPERATION_FAILED, "select input device failed", true),
        "select input device failed");
    return SUCCESS;
}

int32_t AudioCapturerPrivate::SelectInputDeviceInner(const std::shared_ptr<AudioDeviceDescriptor> &desc) const
{
    std::vector<std::shared_ptr<AudioDeviceDescriptor>> audioDeviceDescriptors = { desc };
    sptr<AudioCapturerFilter> audioCapturerFilter = new(std::nothrow) AudioCapturerFilter();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioCapturerFilter != nullptr, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            ERR_RECORD_DEVICE_SWITCH_MEMORY_ALLOC_FAILED, "create capturer filter failed", true),
        "create capturer filter failed");
    audioCapturerFilter->uid = static_cast<int32_t>(getuid());
    audioCapturerFilter->capturerInfo = capturerInfo_;
    audioCapturerFilter->audioDeviceSelectMode = SELECT_STRATEGY_STREAM;
    audioCapturerFilter->streamId = static_cast<int32_t>(sessionID_);
    return AudioPolicyManager::GetInstance().SelectInputDevice(audioCapturerFilter, audioDeviceDescriptors);
}

int32_t AudioCapturerPrivate::SelectInputDevice(const std::shared_ptr<AudioDeviceDescriptor> &desc) const
{
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }
    selectedDevice_ = desc;
    return SelectInputDeviceInner(selectedDevice_);
}

FastStatus AudioCapturerPrivate::GetFastStatus()
{
    std::unique_lock<std::shared_mutex> lock(capturerMutex_, std::defer_lock);
    if (callbackLoopTid_ != gettid()) {
        lock.lock();
    }

    return GetFastStatusInner();
}

int32_t AudioCapturerPrivate::GetCapturerDebugInfo(AudioCapturerDebugInfo &debugInfo) const
{
    AUDIO_DEBUG_LOG("GetCapturerDebugInfo enter");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");

    AudioDebugInfo audioDebugInfo;
    int32_t ret = currentStream->GetAudioDebugInfo(audioDebugInfo);
    if (ret != SUCCESS) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_OPERATION_FAILED, "GetAudioDebugInfo failed", true);
        AUDIO_WARNING_LOG("GetAudioDebugInfo failed, ret:%{public}d", ret);
        uint32_t sessionId = INVALID_STREAM_ID;
        if (GetAudioStreamId(sessionId) == SUCCESS) {
            audioDebugInfo.sessionId = sessionId;
        }
        AudioStreamInfo streamInfo;
        if (GetStreamInfo(streamInfo) == SUCCESS) {
            audioDebugInfo.streamParams.samplingRate = static_cast<uint32_t>(streamInfo.samplingRate);
            audioDebugInfo.streamParams.channels = static_cast<uint8_t>(streamInfo.channels);
            audioDebugInfo.streamParams.format = static_cast<uint8_t>(streamInfo.format);
            audioDebugInfo.streamParams.encoding = static_cast<uint8_t>(streamInfo.encoding);
            audioDebugInfo.streamParams.channelLayout = static_cast<uint64_t>(streamInfo.channelLayout);
        }
        AudioCapturerInfo capturerInfo;
        if (GetCapturerInfo(capturerInfo) == SUCCESS) {
            *audioDebugInfo.capturerInfo = capturerInfo;
        }
    }

    debugInfo.sessionId = audioDebugInfo.sessionId;
    debugInfo.samplingRate = static_cast<int32_t>(audioDebugInfo.streamParams.samplingRate);
    debugInfo.channels = static_cast<AudioChannel>(audioDebugInfo.streamParams.channels);
    debugInfo.format = static_cast<AudioSampleFormat>(audioDebugInfo.streamParams.format);
    debugInfo.encoding = static_cast<AudioEncodingType>(audioDebugInfo.streamParams.encoding);
    debugInfo.channelLayout = static_cast<AudioChannelLayout>(audioDebugInfo.streamParams.channelLayout);
    debugInfo.sourceType = audioDebugInfo.capturerInfo->sourceType;
    debugInfo.capturerFlag = audioDebugInfo.capturerInfo->capturerFlags;
    debugInfo.captureTimestamp = audioDebugInfo.captureTimestamp;
    debugInfo.bufferSize = audioDebugInfo.captureBufferSize;
    debugInfo.overflowCount = audioDebugInfo.captureOverflowCount;
    debugInfo.muteWhenInterrupted = audioDebugInfo.muteWhenInterrupted;
    debugInfo.inputDeviceInfo = audioDebugInfo.inputDeviceInfo;
    debugInfo.pipeRole = audioDebugInfo.pipeRole;
    debugInfo.pipeStreamInfo = *audioDebugInfo.pipeStreamInfo;

    return SUCCESS;
}

FastStatus AudioCapturerPrivate::GetFastStatusInner()
{
    // inner function. Must be called with AudioCapturerPrivate::capturerMutex_ held.
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStream_ != nullptr, FASTSTATUS_INVALID,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    return audioStream_->GetFastStatus();
}

int32_t AudioCapturerPrivate::InitAudioStream(const AudioStreamParams &audioStreamParams)
{
    Trace trace("AudioCapturer::InitAudioStream");
    capturerProxyObj_->SaveCapturerObj(weak_from_this());

    audioStream_->SetCapturerInfo(capturerInfo_);
    SetInnerStreamFastStatusChangeCallback(audioStream_);

    audioStream_->SetClientID(appInfo_.appPid, appInfo_.appUid, appInfo_.appTokenId, appInfo_.appFullTokenId);

    audioStream_->SetClientDeviceId(appInfo_.deviceId);

    if (capturerInfo_.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        audioStream_->SetInnerCapturerState(true);
    } else if (capturerInfo_.sourceType == SourceType::SOURCE_TYPE_WAKEUP) {
        audioStream_->SetWakeupCapturerState(true);
    }

    audioStream_->SetCapturerSource(capturerInfo_.sourceType);
    int32_t ret = audioStream_->SetAudioStreamInfo(audioStreamParams, capturerProxyObj_, filterConfig_);
    CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(ret == SUCCESS, ret,
        HILOG_COMM_ERROR("[InitAudioStream]SetAudioStreamInfo failed"),
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "[InitAudioStream]SetAudioStreamInfo failed", true));
    // for inner-capturer
    if (capturerInfo_.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        ret = UpdatePlaybackCaptureConfig(filterConfig_);
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CREATE_OPERATION_FAILED, "UpdatePlaybackCaptureConfig Failed", true),
            "UpdatePlaybackCaptureConfig Failed");
    }
    InitLatencyMeasurement(audioStreamParams);
    return ret;
}

void AudioCapturerPrivate::CheckSignalData(uint8_t *buffer, size_t bufferSize) const
{
    std::lock_guard lock(signalDetectAgentMutex_);
    if (!latencyMeasEnabled_) {
        return;
    }
    CHECK_AND_CALL_FUNC_RETURN_LOG(signalDetectAgent_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_NULL_POINTER, "LatencyMeas signalDetectAgent_ is nullptr", true),
        "LatencyMeas signalDetectAgent_ is nullptr");
    bool detected = signalDetectAgent_->CheckAudioData(buffer, bufferSize);
    if (detected) {
        std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
        CHECK_AND_CALL_FUNC_RETURN_LOG(currentStream != nullptr,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_SEND_DATA_ILLEGAL_STATE, "audioStream_ is nullptr", true),
            "audioStream_ is nullptr");
        if (capturerInfo_.capturerFlags == IAudioStream::FAST_STREAM) {
            AUDIO_INFO_LOG("LatencyMeas fast capturer signal detected");
        } else {
            AUDIO_INFO_LOG("LatencyMeas normal capturer signal detected");
        }
        currentStream->UpdateLatencyTimestamp(signalDetectAgent_->lastPeakBufferTime_, false);
    }
}

void AudioCapturerPrivate::InitLatencyMeasurement(const AudioStreamParams &audioStreamParams)
{
    std::lock_guard lock(signalDetectAgentMutex_);
    latencyMeasEnabled_ = AudioLatencyMeasurement::CheckIfEnabled();
    AUDIO_INFO_LOG("LatencyMeas enabled in capturer:%{public}d", latencyMeasEnabled_);
    if (!latencyMeasEnabled_) {
        return;
    }
    signalDetectAgent_ = std::make_shared<SignalDetectAgent>();
    CHECK_AND_CALL_FUNC_RETURN_LOG(signalDetectAgent_ != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_MEMORY_ALLOC_FAILED, "LatencyMeas signalDetectAgent_ is nullptr", true),
        "LatencyMeas signalDetectAgent_ is nullptr");
    signalDetectAgent_->sampleFormat_ = audioStreamParams.format;
    signalDetectAgent_->formatByteSize_ = GetFormatByteSize(audioStreamParams.format);
}

int32_t AudioCapturerPrivate::InitAudioInterruptCallback()
{
    if (audioInterrupt_.streamId != 0) {
        AUDIO_INFO_LOG("old session already has interrupt, need to reset");
        (void)AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
        (void)AudioPolicyManager::GetInstance().UnsetAudioInterruptCallback(audioInterrupt_.streamId);
    }

    if (audioStream_->GetAudioSessionID(sessionID_) != 0) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_INVALID_INDEX, "GetAudioSessionID failed for INDEPENDENT_MODE", true);
        AUDIO_ERR_LOG("GetAudioSessionID failed for INDEPENDENT_MODE");
        return ERR_INVALID_INDEX;
    }
    audioInterrupt_.streamId = sessionID_;
    audioInterrupt_.pid = appInfo_.appPid;
    audioInterrupt_.uid = appInfo_.appUid;
    audioInterrupt_.audioFocusType.sourceType = capturerInfo_.sourceType;
    audioInterrupt_.sessionStrategy = strategy_;
    audioInterrupt_.mixWithWakeUp = mixWithWakeUp_;
    audioInterrupt_.bundleName = AudioSystemManager::GetInstance()->GetSelfBundleName(appInfo_.appUid);
    if (audioInterrupt_.bundleName.empty()) {
        audioInterrupt_.bundleName = AudioSystemManager::GetInstance()->GetSelfBundleName();
    }
    if (audioInterrupt_.audioFocusType.sourceType == SOURCE_TYPE_VIRTUAL_CAPTURE) {
        isVoiceCallCapturer_ = true;
        audioInterrupt_.audioFocusType.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    }
    if (audioInterruptCallback_ == nullptr) {
        audioInterruptCallback_ = std::make_shared<AudioCapturerInterruptCallbackImpl>(audioStream_);
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioInterruptCallback_ != nullptr, ERROR,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CREATE_MEMORY_ALLOC_FAILED,
                "Failed to allocate memory for audioInterruptCallback_", true),
            "Failed to allocate memory for audioInterruptCallback_");
    }
    return AudioPolicyManager::GetInstance().SetAudioInterruptCallback(sessionID_, audioInterruptCallback_,
        appInfo_.appUid);
}

int32_t AudioCapturerPrivate::SetCapturerCallback(const std::shared_ptr<AudioCapturerCallback> &callback)
{
    std::lock_guard<std::mutex> lock(setCapturerCbMutex_);
    // If the client is using the deprecated SetParams API. SetCapturerCallback must be invoked, after SetParams.
    // In general, callbacks can only be set after the capturer state is  PREPARED.
    CapturerState state = GetStatus();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(state != CAPTURER_NEW && state != CAPTURER_RELEASED, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE,
            "SetCapturerCallback incorrect state to register cb", true),
        "SetCapturerCallback ncorrect state:%{public}d to register cb", state);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_INVALID_PARAM,
            "SetCapturerCallback callback param is null", true),
        "SetCapturerCallback callback param is null");

    // Save reference for interrupt callback
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioInterruptCallback_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE,
            "SetCapturerCallback audioInterruptCallback_ is nullptr", true),
        "SetCapturerCallback audioInterruptCallback_ == nullptr");
    std::shared_ptr<AudioCapturerInterruptCallbackImpl> cbInterrupt =
        std::static_pointer_cast<AudioCapturerInterruptCallbackImpl>(audioInterruptCallback_);
    cbInterrupt->SaveCallback(callback);

    // Save and Set reference for stream callback. Order is important here.
    if (audioStreamCallback_ == nullptr) {
        audioStreamCallback_ = std::make_shared<AudioStreamCallbackCapturer>(weak_from_this());
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStreamCallback_ != nullptr, ERROR,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_MEMORY_ALLOC_FAILED,
                "Failed to allocate memory for audioStreamCallback_", true),
            "Failed to allocate memory for audioStreamCallback_");
    }
    std::shared_ptr<AudioStreamCallbackCapturer> cbStream =
        std::static_pointer_cast<AudioStreamCallbackCapturer>(audioStreamCallback_);
    cbStream->SaveCallback(callback);
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    (void)currentStream->SetStreamCallback(audioStreamCallback_);

    return SUCCESS;
}

void AudioCapturerPrivate::SetAudioCapturerErrorCallback(std::shared_ptr<AudioCapturerErrorCallback> errorCallback)
{
    std::shared_lock sharedLock(switchStreamMutex_);
    std::lock_guard lock(audioCapturerErrCallbackMutex_);
    audioCapturerErrorCallback_ = errorCallback;
}

int32_t AudioCapturerPrivate::RegisterAudioPolicyServerDiedCb(const int32_t clientPid,
    const std::shared_ptr<AudioCapturerPolicyServiceDiedCallback> &callback)
{
    AUDIO_INFO_LOG("RegisterAudioPolicyServerDiedCb client id: %{public}d", clientPid);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_INVALID_PARAM, "callback is null", true),
        "callback is null");

    std::lock_guard<std::mutex> lock(policyServiceDiedCallbackMutex_);

    policyServiceDiedCallback_ = callback;
    return AudioPolicyManager::GetInstance().RegisterAudioPolicyServerDiedCb(clientPid, callback);
}

void AudioCapturerPrivate::SetFastStatusChangeCallback(
    const std::shared_ptr<AudioCapturerFastStatusChangeCallback> &callback)
{
    {
        std::lock_guard lock(fastStatusChangeCallbackMutex_);
        fastStatusChangeCallback_ = callback;
    }
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    SetInnerStreamFastStatusChangeCallback(currentStream);
}

void AudioCapturerPrivate::SetPlaybackCaptureStartStateCallback(
    const std::shared_ptr<AudioCapturerOnPlaybackCaptureStartCallback> &callback)
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_LOG(currentStream != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    currentStream->SetPlaybackCaptureStartStateCallback(callback);
}

int32_t AudioCapturerPrivate::GetParams(AudioCapturerParams &params) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    AudioStreamParams audioStreamParams;
    int32_t result = currentStream->GetAudioStreamInfo(audioStreamParams);
    if (SUCCESS == result) {
        params.audioSampleFormat = static_cast<AudioSampleFormat>(audioStreamParams.format);
        params.samplingRate = static_cast<AudioSamplingRate>(audioStreamParams.samplingRate);
        params.audioChannel = static_cast<AudioChannel>(audioStreamParams.channels);
        params.audioEncoding = static_cast<AudioEncodingType>(audioStreamParams.encoding);
    }

    return result;
}

int32_t AudioCapturerPrivate::GetCapturerInfo(AudioCapturerInfo &capturerInfo) const
{
    capturerInfo = capturerInfo_;

    return SUCCESS;
}

int32_t AudioCapturerPrivate::GetStreamInfo(AudioStreamInfo &streamInfo) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    AudioStreamParams audioStreamParams;
    int32_t result = currentStream->GetAudioStreamInfo(audioStreamParams);
    if (SUCCESS == result) {
        streamInfo.format = static_cast<AudioSampleFormat>(audioStreamParams.format);
        streamInfo.samplingRate = static_cast<AudioSamplingRate>(audioStreamParams.samplingRate);
        if (this->isChannelChange_) {
            streamInfo.channels = AudioChannel::CHANNEL_3;
        } else {
            streamInfo.channels = static_cast<AudioChannel>(audioStreamParams.channels);
        }
        streamInfo.encoding = static_cast<AudioEncodingType>(audioStreamParams.encoding);
    }

    return result;
}

int32_t AudioCapturerPrivate::SetCapturerPositionCallback(int64_t markPosition,
    const std::shared_ptr<CapturerPositionCallback> &callback)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG((callback != nullptr) && (markPosition > 0), ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_INVALID_PARAM, "input param is invalid", true),
        "input param is invalid");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    currentStream->SetCapturerPositionCallback(markPosition, callback);

    return SUCCESS;
}

void AudioCapturerPrivate::UnsetCapturerPositionCallback()
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_LOG(currentStream != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    currentStream->UnsetCapturerPositionCallback();
}

int32_t AudioCapturerPrivate::SetCapturerPeriodPositionCallback(int64_t frameNumber,
    const std::shared_ptr<CapturerPeriodPositionCallback> &callback)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG((callback != nullptr) && (frameNumber > 0), ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_INVALID_PARAM, "input param is invalid", true),
        "input param is invalid");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    currentStream->SetCapturerPeriodPositionCallback(frameNumber, callback);

    return SUCCESS;
}

void AudioCapturerPrivate::UnsetCapturerPeriodPositionCallback()
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_LOG(currentStream != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    currentStream->UnsetCapturerPeriodPositionCallback();
}

int32_t AudioCapturerPrivate::CheckAndRestoreAudioCapturer(std::string callingFunc)
{
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }

    if (abortRestore_) {
        AUDIO_INFO_LOG("abort restore new audio capturer");
        return SUCCESS;
    }
    // Return in advance if there's no need for restore.
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStream_, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_ILLEGAL_STATE, "audioStream_ is nullptr", true), "audioStream_ is nullptr");
    RestoreStatus restoreStatus = audioStream_->CheckRestoreStatus();
    if (restoreStatus == NO_NEED_FOR_RESTORE) {
        return SUCCESS;
    }
    if (restoreStatus == RESTORING) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_ILLEGAL_STATE, "restoring in progress", true);
        AUDIO_WARNING_LOG("%{public}s when restoring, return", callingFunc.c_str());
        return ERR_ILLEGAL_STATE;
    }

    // Get restore info and target stream class for switching.
    RestoreInfo restoreInfo;
    audioStream_->GetRestoreInfo(restoreInfo);
    IAudioStream::StreamClass targetClass = DecideStreamClassAndUpdateCapturerInfo(restoreInfo.routeFlag);
    if (restoreStatus == NEED_RESTORE_TO_NORMAL) {
        restoreInfo.targetStreamFlag = AUDIO_FLAG_FORCED_NORMAL;
    }

    // Block interrupt calback, avoid pausing wrong stream.
    std::shared_ptr<AudioCapturerInterruptCallbackImpl> interruptCbImpl = nullptr;
    if (audioInterruptCallback_ != nullptr) {
        interruptCbImpl = std::static_pointer_cast<AudioCapturerInterruptCallbackImpl>(audioInterruptCallback_);
        interruptCbImpl->StartSwitch();
    }

    FastStatus fastStatus = GetFastStatusInner();
    // Switch to target audio stream. Deactivate audio interrupt if switch failed.
    AUDIO_INFO_LOG("Before %{public}s, restore audio capturer %{public}u", callingFunc.c_str(), sessionID_);
    if (!SwitchToTargetStream(targetClass, restoreInfo)) {
        AudioInterrupt audioInterrupt = audioInterrupt_;
        int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt);
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_DEVICE_SWITCH_OPERATION_FAILED, "DeactivateAudioInterrupt Failed", true),
            "DeactivateAudioInterrupt Failed");
    } else {
        FastStatusChangeCallback(fastStatus);
    }

    // Unblock interrupt callback.
    if (interruptCbImpl) {
        interruptCbImpl->FinishSwitch();
    }
    return SUCCESS;
}

void AudioCapturerPrivate::SetInSwitchingFlag(bool inSwitchingFlag)
{
    std::unique_lock<std::mutex> lock(inSwitchingMtx_);
    inSwitchingFlag_ = inSwitchingFlag;
    if (!inSwitchingFlag_) {
        taskLoopCv_.notify_all();
    }
}

bool AudioCapturerPrivate::IsRestoreOrStopNeeded()
{
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }
    CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(audioStream_ != nullptr, false,
        HILOG_COMM_ERROR("[IsRestoreOrStopNeeded]audio stream is null"),
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_NULL_POINTER, "[IsRestoreOrStopNeeded]audio stream is null", true));
    return audioStream_->IsRestoreNeeded() || audioStream_->GetStopFlag();
}

int32_t AudioCapturerPrivate::AsyncCheckAudioCapturer(std::string callingFunc)
{
    if (switchStreamInNewThreadTaskCount_.fetch_add(1) > 0) {
        return SUCCESS;
    }
    auto weakCapturer = weak_from_this();
    taskLoop_.PostTask([weakCapturer, callingFunc] () {
        auto sharedCapturer = weakCapturer.lock();
        CHECK_AND_CALL_FUNC_RETURN_LOG(sharedCapturer,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_DEVICE_SWITCH_ILLEGAL_STATE, "capturer is null", true),
            "capturer is null");
        uint32_t taskCount;
        do {
            taskCount = sharedCapturer->switchStreamInNewThreadTaskCount_.load();
            sharedCapturer->SetInSwitchingFlag(true);
            sharedCapturer->CheckAudioCapturer(callingFunc + "withNewThread");
            sharedCapturer->SetInSwitchingFlag(false);
        } while (sharedCapturer->switchStreamInNewThreadTaskCount_.fetch_sub(taskCount) > taskCount);
    });
    return SUCCESS;
}

void AudioCapturerPrivate::CheckStartResultAndReport(bool result)
{
    AudioWakeupErrorCode errCode = (result == true)
                                ? WAKEUP_TRACK_FWK_NO_ERROR
                                : WAKEUP_TRACK_FWK_START_ERROR;
    AudioWakeupResult wakeupResult = (result == true)
                                ? WAKEUP_RESULT_DEFAULT
                                : WAKEUP_RESULT_FAIL;
    ReportWakeupEvent(audioInterrupt_.audioFocusType.sourceType,
        STAGE_FWK_START_EXIT, wakeupResult, errCode);
}

int32_t AudioCapturerPrivate::CheckStateAndReportErrorEvent(CapturerState state)
{
    HILOG_COMM_INFO("StreamClientState for Capturer::Start. id %{public}u, sourceType: %{public}d",
        sessionID_, audioInterrupt_.audioFocusType.sourceType);
    ReportWakeupEvent(audioInterrupt_.audioFocusType.sourceType, STAGE_FWK_START_ENTER,
        WAKEUP_RESULT_DEFAULT, WAKEUP_TRACK_FWK_NO_ERROR);

    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        (state == CAPTURER_PREPARED) || (state == CAPTURER_STOPPED) || (state == CAPTURER_PAUSED), ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_START_ILLEGAL_STATE, "Start failed. Illegal state", true),
        "Start failed. Illegal state %{public}u.", state);

    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(!isSwitching_, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_START_ILLEGAL_STATE, "Operation failed, in switching", true),
        "Operation failed, in switching");

    CHECK_AND_CALL_FUNC_RETURN_RET(audioInterrupt_.audioFocusType.sourceType != SOURCE_TYPE_INVALID &&
        audioInterrupt_.streamId != INVALID_STREAM_ID, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_START_ILLEGAL_STATE, "audioInterrupt not initialized", true));
    return SUCCESS;
}

int32_t AudioCapturerPrivate::CheckForStartImpl(CapturerState state)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStream_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_START_ILLEGAL_STATE, "audioStream_ is null", true), "audioStream_ is null");
    if (state == CAPTURER_STOPPED && getuid() == UID_MEDIA_SA) {
        AUDIO_INFO_LOG("Media SA is startting, flush data.");
        audioStream_->FlushAudioStream();
    }
    if (capturerInfo_.loopBackEffectEnabled) {
        AudioPolicyManager::GetInstance().SetKaraokeParameters(DEVICE_TYPE_SPEAKER, "Karaoke_capture_reverb=true");
    }
    return SUCCESS;
}

void AudioCapturerPrivate::SendAudioErrorEventAndProcessOther(int32_t uid, int32_t errorCode,
    const std::string &erroDesc, bool isClient, CapturerState state) const
{
    StreamDfxManager::GetInstance().SendAudioErrorEvent(uid, errorCode, erroDesc, isClient);
    audioStream_->SetFocusState(static_cast<StreamFocusState>(state));
}

int32_t AudioCapturerPrivate::StartImpl()
{
    AsyncCheckAudioCapturer("Start");
    Trace trace("KeyAction AudioCapturer::Start" + std::to_string(sessionID_));
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }
    CapturerState state = GetStatusInner();
    int32_t ret = CheckStateAndReportErrorEvent(state);
    CHECK_AND_CALL_FUNC_RETURN_RET(ret == SUCCESS, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_START_ILLEGAL_STATE, "Check state fail", true));
    std::unique_lock<std::mutex> audioInterruptLock(audioInterruptMutex_);
    AudioInterrupt audioInterrupt = audioInterrupt_;
    audioInterruptLock.unlock();
    audioStream_->SetFocusState(FOCUS_STARTING);
    ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt);
    CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(ret == 0, ERROR, HILOG_COMM_ERROR("[Start]ActivateAudioInterrupt Failed"),
        SendAudioErrorEventAndProcessOther(static_cast<int32_t>(getuid()), RECORD_START_OPERATION_FAILED,
            "[Start]ActivateAudioInterrupt Failed", true, state));
    // When the cellular call stream is starting, only need to activate audio interrupt.
    CHECK_AND_CALL_FUNC_RETURN_RET(!isVoiceCallCapturer_, SUCCESS,
        SendAudioErrorEventAndProcessOther(static_cast<int32_t>(getuid()),
            RECORD_START_ILLEGAL_STATE, "isVoiceCallCapturer_ skip", true, state));
    ret = CheckForStartImpl(state);
    CHECK_AND_CALL_FUNC_RETURN_RET(ret == SUCCESS, ERR_ILLEGAL_STATE,
        SendAudioErrorEventAndProcessOther(static_cast<int32_t>(getuid()),
            RECORD_START_ILLEGAL_STATE, "CheckForStartImpl fail", true, state));
    bool result = audioStream_->StartAudioStream();
    if (!result) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_START_OPERATION_FAILED, "Start audio stream failed", true);
        AUDIO_ERR_LOG("Start audio stream failed");
        if (capturerInfo_.loopBackEffectEnabled) {
            AudioPolicyManager::GetInstance().SetKaraokeParameters(DEVICE_TYPE_SPEAKER,
                "Karaoke_capture_reverb=false");
        }
        ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
        if (ret != 0) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_START_OPERATION_FAILED, "DeactivateAudioInterrupt Failed", true);
            AUDIO_WARNING_LOG("DeactivateAudioInterrupt Failed");
        }
    }
    CheckStartResultAndReport(result);
    return result ? SUCCESS : ERROR;
}

int32_t AudioCapturerPrivate::Read(uint8_t &buffer, size_t userSize, bool isBlockingRead)
{
    Trace trace("AudioCapturer::Read");
    CheckSignalData(&buffer, userSize);
    AsyncCheckAudioCapturer("Read");

    std::unique_lock<std::mutex> lock(inSwitchingMtx_);
    taskLoopCv_.wait_for(lock, std::chrono::milliseconds(BLOCK_INTERRUPT_OVERTIMES_IN_MS), [this] {
        return inSwitchingFlag_ == false;
    });
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    int size = currentStream->Read(buffer, userSize, isBlockingRead);
    if (size > 0) {
        DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(&buffer), size);
    }
    return size;
}

CapturerState AudioCapturerPrivate::GetStatus() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, CAPTURER_INVALID,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    return static_cast<CapturerState>(currentStream->GetState());
}

bool AudioCapturerPrivate::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    return currentStream->GetAudioTime(timestamp, base);
}

bool AudioCapturerPrivate::Pause() const
{
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }
    Trace trace("KeyAction AudioCapturer::Pause" + std::to_string(sessionID_));

    HILOG_COMM_INFO("StreamClientState for Capturer::Pause. id %{public}u", sessionID_);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(!isSwitching_, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_PAUSE_ILLEGAL_STATE, "Operation failed, in switching", true),
        "Operation failed, in switching");
    CapturerState state = GetStatusInner();
    audioStream_->SetFocusState(FOCUS_APP_PAUSING);
    // When user is intentionally pausing , Deactivate to remove from audio focus info list
    int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
    if (ret != 0) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_PAUSE_OPERATION_FAILED, "DeactivateAudioInterrupt Failed", true);
        AUDIO_WARNING_LOG("AudioRenderer: DeactivateAudioInterrupt Failed");
    }

    // When the cellular call stream is pausing, only need to deactivate audio interrupt.
    CHECK_AND_CALL_FUNC_RETURN_RET(!isVoiceCallCapturer_, true,
        SendAudioErrorEventAndProcessOther(static_cast<int32_t>(getuid()), RECORD_PAUSE_ILLEGAL_STATE,
            "isVoiceCallCapturer_ skip", true, state));
    if (capturerInfo_.loopBackEffectEnabled) {
        AudioPolicyManager::GetInstance().SetKaraokeParameters(DEVICE_TYPE_SPEAKER, "Karaoke_capture_reverb=false");
    }
    return audioStream_->PauseAudioStream();
}

int32_t AudioCapturerPrivate::StopImpl() const
{
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }
    Trace trace("KeyAction AudioCapturer::Stop" + std::to_string(sessionID_));
    HILOG_COMM_INFO("StreamClientState for Capturer::Stop. id %{public}u", sessionID_);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(!isSwitching_, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_STOP_ILLEGAL_STATE, "Operation failed, in switching", true),
        "Operation failed, in switching");

    WriteOverflowEvent();
    CapturerState state = GetStatusInner();
    audioStream_->SetFocusState(FOCUS_APP_STOPPING);
    int32_t ret = AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);
    if (ret != 0) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_STOP_OPERATION_FAILED, "DeactivateAudioInterrupt Failed", true);
        AUDIO_WARNING_LOG("AudioCapturer: DeactivateAudioInterrupt Failed");
    }

    CHECK_AND_CALL_FUNC_RETURN_RET(isVoiceCallCapturer_ != true, SUCCESS,
        SendAudioErrorEventAndProcessOther(static_cast<int32_t>(getuid()),
            RECORD_STOP_ILLEGAL_STATE, "isVoiceCallCapturer_ skip", true, state));
    if (capturerInfo_.loopBackEffectEnabled) {
        AudioPolicyManager::GetInstance().SetKaraokeParameters(DEVICE_TYPE_SPEAKER, "Karaoke_capture_reverb=false");
    }

    return audioStream_->StopAudioStream() ? SUCCESS : ERROR;
}

bool AudioCapturerPrivate::Flush() const
{
    Trace trace("KeyAction AudioCapturer::Flush " + std::to_string(sessionID_));
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_FLUSH_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    AUDIO_INFO_LOG("StreamClientState for Capturer::Flush. id %{public}u", sessionID_);
    return currentStream->FlushAudioStream();
}

int32_t AudioCapturerPrivate::ReleaseImpl()
{
    Trace trace("KeyAction AudioCapturer::Release" + std::to_string(sessionID_));
    HILOG_COMM_INFO("StreamClientState for Capturer::Release. id %{public}u", sessionID_);
    std::unique_lock<std::shared_mutex> releaseLock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        releaseLock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }
    abortRestore_ = true;
    std::lock_guard<std::mutex> lock(lock_);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(isValid_, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_RELEASE_ILLEGAL_STATE, "Release when capturer invalid", true),
        "Release when capturer invalid");

    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStream_ != nullptr, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_RELEASE_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    audioInterrupt_.state = State::RELEASED;
    (void)AudioPolicyManager::GetInstance().DeactivateAudioInterrupt(audioInterrupt_);

    // Unregister the callaback in policy server
    (void)AudioPolicyManager::GetInstance().UnsetAudioInterruptCallback(sessionID_);

    RemoveCapturerPolicyServiceDiedCallback(audioStream_);
    (void)AudioDebugManager::GetInstance().UnregisterAudioCapturer(reinterpret_cast<uintptr_t>(this));
    if (capturerInfo_.loopBackEffectEnabled) {
        AudioPolicyManager::GetInstance().SetKaraokeParameters(DEVICE_TYPE_SPEAKER, "Karaoke_capture_reverb=false");
    }

    return audioStream_->ReleaseAudioStream() ? SUCCESS : ERROR;
}

int32_t AudioCapturerPrivate::GetBufferSize(size_t &bufferSize) const
{
    Trace trace("AudioCapturer::GetBufferSize");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    return currentStream->GetBufferSize(bufferSize);
}

int32_t AudioCapturerPrivate::GetAudioStreamId(uint32_t &sessionID) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERR_INVALID_HANDLE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_INVALID_HANDLE, "GetAudioStreamId faied", true),
        "GetAudioStreamId faied.");
    return currentStream->GetAudioSessionID(sessionID);
}

int32_t AudioCapturerPrivate::SetBufferDuration(uint64_t bufferDuration) const
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(
        bufferDuration >= MINIMUM_BUFFER_SIZE_MSEC && bufferDuration <= MAXIMUM_BUFFER_SIZE_MSEC, ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_INVALID_PARAM, "Please set the buffer duration between 5ms ~ 20ms", true),
        "Error: Please set the buffer duration between 5ms ~ 20ms");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    return currentStream->SetBufferSizeInMsec(bufferDuration);
}

bool AudioCapturerPrivate::GetTimeStampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    return currentStream->GetTimeStampInfo(timestamp, base);
}

// diffrence from GetAudioPosition only when set speed
int32_t AudioCapturerPrivate::GetAudioTimestampInfo(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStream_ != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    return audioStream_->GetAudioTimestampInfo(timestamp, base);
}

AudioCapturerInterruptCallbackImpl::AudioCapturerInterruptCallbackImpl(const std::shared_ptr<IAudioStream> &audioStream)
    : audioStream_(audioStream)
{
    AUDIO_DEBUG_LOG("AudioCapturerInterruptCallbackImpl constructor");
}

AudioCapturerInterruptCallbackImpl::~AudioCapturerInterruptCallbackImpl()
{
    AUDIO_DEBUG_LOG("AudioCapturerInterruptCallbackImpl: instance destroy");
}

void AudioCapturerInterruptCallbackImpl::SaveCallback(const std::weak_ptr<AudioCapturerCallback> &callback)
{
    callback_ = callback;
}

void AudioCapturerInterruptCallbackImpl::UpdateAudioStream(const std::shared_ptr<IAudioStream> &audioStream)
{
    std::lock_guard<std::mutex> lock(mutex_);
    audioStream_ = audioStream;
}

void AudioCapturerInterruptCallbackImpl::StartSwitch()
{
    std::lock_guard<std::mutex> lock(mutex_);
    switching_ = true;
    AUDIO_INFO_LOG("SwitchStream start, block interrupt callback");
}

void AudioCapturerInterruptCallbackImpl::FinishSwitch()
{
    std::lock_guard<std::mutex> lock(mutex_);
    switching_ = false;
    switchStreamCv_.notify_all();
    AUDIO_INFO_LOG("SwitchStream finish, notify interrupt callback");
}

bool AudioCapturerInterruptCallbackImpl::NoNeedNotifyEvent(const InterruptEvent &interruptEvent)
{
    StreamFocusState currentFocusState = audioStream_->GetFocusState();
    bool noNeedSendNotify = (currentFocusState == FOCUS_STARTING || currentFocusState == FOCUS_RUNNING
        || currentFocusState == FOCUS_STOPPED || currentFocusState == FOCUS_APP_STOPPING);
    if (interruptEvent.hintType == INTERRUPT_HINT_PAUSE && noNeedSendNotify) {
        AUDIO_INFO_LOG("hintType is pause, currentFocusState is %{public}d, no need send event.", currentFocusState);
        return true;
    }
    noNeedSendNotify = (currentFocusState == FOCUS_RUNNING || currentFocusState == FOCUS_STARTING);
    if (interruptEvent.hintType == INTERRUPT_HINT_STOP && noNeedSendNotify) {
        AUDIO_INFO_LOG("hintType is stop, currentFocusState is %{public}d, no need send event.", currentFocusState);
        return true;
    }
    noNeedSendNotify = (currentFocusState == FOCUS_STARTING  || currentFocusState == FOCUS_RUNNING
        || currentFocusState ==FOCUS_STOPPED || currentFocusState == FOCUS_APP_STOPPING);
    if (interruptEvent.hintType == INTERRUPT_HINT_RESUME && noNeedSendNotify) {
        AUDIO_INFO_LOG("hintType is resume, currentFocusState is %{public}d, no need send event.", currentFocusState);
        return true;
    }
    return false;
}

void AudioCapturerInterruptCallbackImpl::NotifyEvent(const InterruptEvent &interruptEvent)
{
    AUDIO_INFO_LOG("NotifyEvent: Hint: %{public}d, eventType: %{public}d",
        interruptEvent.hintType, interruptEvent.eventType);
    if (NoNeedNotifyEvent(interruptEvent) == true) {
        return;
    }
    if (cb_ != nullptr) {
        cb_->OnInterrupt(interruptEvent);
        AUDIO_DEBUG_LOG("OnInterrupt : NotifyEvent to app complete");
    } else {
        AUDIO_DEBUG_LOG("cb_ == nullptr cannont NotifyEvent to app");
    }
}

int32_t AudioCapturerInterruptCallbackImpl::InterruptHintResumeHandle(StreamFocusState focusState)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG((focusState == FOCUS_PAUSED || focusState == FOCUS_PREPARED) &&
        isForcePaused_ == true, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "OnInterrupt state check failed", true),
        "OnInterrupt streamFocusState %{public}d or not forced pause %{public}d before", focusState, isForcePaused_);
    AUDIO_INFO_LOG("set force pause false");
    isForcePaused_ = false;
    return SUCCESS;
}

int32_t AudioCapturerInterruptCallbackImpl::InterruptHintPauseHandle(StreamFocusState focusState)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(focusState == FOCUS_RUNNING || focusState == FOCUS_PREPARED
        || focusState == FOCUS_STARTING, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "OnInterrupt state no need to pause", true),
        "OnInterrupt streamFocusState %{public}d, no need to pause", focusState); // Just Pause, do not deactivate here
    audioStream_->SetFocusState(FOCUS_SYSTEM_PAUSING);
    (void)audioStream_->PauseAudioStream();
    AUDIO_INFO_LOG("set force pause true");
    isForcePaused_ = true;
    return SUCCESS;
}

void AudioCapturerInterruptCallbackImpl::WaitForSwitchTimeout(bool result)
{
    if (!result) {
        switching_ = false;
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_TIMEOUT, "Wait for SwitchStream time out", true);
        AUDIO_WARNING_LOG("Wait for SwitchStream time out, could handle interrupt event with old stream");
    }
}

void AudioCapturerInterruptCallbackImpl::OnInterrupt(const InterruptEventInternal &interruptEvent)
{
    std::unique_lock<std::mutex> lock(mutex_);

    if (switching_) {
        AUDIO_INFO_LOG("Wait for SwitchStream");
        bool ret = switchStreamCv_.wait_for(lock, std::chrono::milliseconds(BLOCK_INTERRUPT_CALLBACK_IN_MS),
            [this] {return !switching_;});
        WaitForSwitchTimeout(ret);
    }
    cb_ = callback_.lock();
    InterruptForceType forceType = interruptEvent.forceType;
    AUDIO_INFO_LOG("InterruptForceType: %{public}d", forceType);
    InterruptEvent event;

    if (forceType == INTERRUPT_SHARE) { // INTERRUPT_SHARE
        AUDIO_DEBUG_LOG("AudioCapturerPrivate ForceType: INTERRUPT_SHARE. Let app handle the event");
        event = InterruptEvent {interruptEvent.eventType, interruptEvent.forceType, interruptEvent.hintType};
    } else {
        CHECK_AND_CALL_FUNC_RETURN_LOG(audioStream_ != nullptr,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_ILLEGAL_STATE, "Stream is not alive", true), "Stream is not alive");
        StreamFocusState currentFocusState = audioStream_->GetFocusState();

        int32_t interruptRet = 0;
        switch (interruptEvent.hintType) {
            case INTERRUPT_HINT_RESUME:
                interruptRet = InterruptHintResumeHandle(currentFocusState);
                CHECK_AND_CALL_FUNC_RETURN(interruptRet == SUCCESS,
                    StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                        RECORD_CALLBACK_ILLEGAL_STATE, "InterruptHintResumeHandle failed", true));
                event = InterruptEvent {interruptEvent.eventType, INTERRUPT_SHARE, interruptEvent.hintType};
                lock.unlock();
                NotifyEvent(event);
                return;
            case INTERRUPT_HINT_PAUSE:
                interruptRet = InterruptHintPauseHandle(currentFocusState);
                CHECK_AND_CALL_FUNC_RETURN(interruptRet == SUCCESS,
                    StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                        RECORD_CALLBACK_ILLEGAL_STATE, "InterruptHintPauseHandle failed", true));
                break;
            case INTERRUPT_HINT_STOP:
                audioStream_->SetFocusState(FOCUS_SYSTEM_STOPPING);
                (void)audioStream_->StopAudioStream();
                break;
            default:
                break;
        }
        // Notify valid forced event callbacks to app
        event = InterruptEvent {interruptEvent.eventType, interruptEvent.forceType, interruptEvent.hintType};
    }

    lock.unlock();
    NotifyEvent(event);
}

AudioStreamCallbackCapturer::AudioStreamCallbackCapturer(std::weak_ptr<AudioCapturerPrivate> capturer)
    : capturer_(capturer)
{
}

void AudioStreamCallbackCapturer::SaveCallback(const std::weak_ptr<AudioCapturerCallback> &callback)
{
    callback_ = callback;
}

void AudioStreamCallbackCapturer::OnStateChange(const State state,
    const StateChangeCmdType __attribute__((unused)) cmdType)
{
    std::shared_ptr<AudioCapturerPrivate> capturerObj = capturer_.lock();
    CHECK_AND_CALL_FUNC_RETURN_LOG(capturerObj != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "capturerObj is nullptr", true),
        "capturerObj is nullptr");
    std::shared_ptr<AudioCapturerCallback> cb = callback_.lock();
    CHECK_AND_CALL_FUNC_RETURN_LOG(cb != nullptr,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "callback is nullptr", true),
        "AudioStreamCallbackCapturer::OnStateChange cb == nullptr.");

    auto captureState = static_cast<CapturerState>(state);
    cb->OnStateChange(captureState);

    AudioInterrupt audioInterrupt;
    capturerObj->GetAudioInterrupt(audioInterrupt);
    audioInterrupt.state = state;
    capturerObj->SetAudioInterrupt(audioInterrupt);
}

std::vector<AudioSampleFormat> AudioCapturer::GetSupportedFormats()
{
    return AUDIO_SUPPORTED_FORMATS;
}

std::vector<AudioChannel> AudioCapturer::GetSupportedChannels()
{
    return CAPTURER_SUPPORTED_CHANNELS;
}

std::vector<AudioEncodingType> AudioCapturer::GetSupportedEncodingTypes()
{
    return AUDIO_SUPPORTED_ENCODING_TYPES;
}

std::vector<AudioSamplingRate> AudioCapturer::GetSupportedSamplingRates()
{
    return AUDIO_SUPPORTED_SAMPLING_RATES;
}

AudioStreamType AudioCapturer::FindStreamTypeBySourceType(SourceType sourceType)
{
    switch (sourceType) {
        case SOURCE_TYPE_VOICE_COMMUNICATION:
        case SOURCE_TYPE_VIRTUAL_CAPTURE:
            return STREAM_VOICE_CALL;
        case SOURCE_TYPE_WAKEUP:
            return STREAM_WAKEUP;
        case SOURCE_TYPE_VOICE_CALL:
            return STREAM_SOURCE_VOICE_CALL;
        case SOURCE_TYPE_CAMCORDER:
            return STREAM_CAMCORDER;
        default:
            return STREAM_MUSIC;
    }
}

int32_t AudioCapturerPrivate::SetAudioSourceConcurrency(const std::vector<SourceType> &targetSources)
{
    if (targetSources.size() <= 0) {
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_INVALID_PARAM, "TargetSources size is 0", true);
        AUDIO_ERR_LOG("TargetSources size is 0, set audio source concurrency failed.");
        return ERR_INVALID_PARAM;
    }
    AUDIO_INFO_LOG("Set audio source concurrency success.");
    std::lock_guard<std::mutex> lock(audioInterruptMutex_);
    audioInterrupt_.currencySources.sourcesTypes = targetSources;
    return SUCCESS;
}

int32_t AudioCapturerPrivate::SetInterruptStrategy(InterruptStrategy strategy)
{
    CapturerState state = GetStatusInner();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(state == CAPTURER_PREPARED, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_ILLEGAL_STATE, "incorrect state", true),
        "incorrect state:%{public}d", state);
    audioInterrupt_.strategy = strategy;
    AUDIO_INFO_LOG("set InterruptStrategy to %{public}d", static_cast<int32_t>(strategy));
    return SUCCESS;
}

int32_t AudioCapturerPrivate::SetMuteHint(bool mute)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(GetStatus() == CAPTURER_RUNNING, ERR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_ILLEGAL_STATE, "SetMuteHint only support running state", true),
        "SetMuteHint only support running state");

    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");

    return currentStream->SetMuteHint(mute);
}

int32_t AudioCapturerPrivate::SetCaptureMode(AudioCaptureMode captureMode)
{
    AUDIO_INFO_LOG("Capture mode: %{public}d", captureMode);
    audioCaptureMode_ = captureMode;
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    int32_t ret = currentStream->SetCaptureMode(captureMode);
    callbackLoopTid_ = audioStream_->GetCallbackLoopTid();
    return ret;
}

AudioCaptureMode AudioCapturerPrivate::GetCaptureMode() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, CAPTURE_MODE_NORMAL,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    return currentStream->GetCaptureMode();
}

int32_t AudioCapturerPrivate::SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback)
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    return currentStream->SetCapturerReadCallback(callback);
}

int32_t AudioCapturerPrivate::GetBufferDesc(BufferDesc &bufDesc)
{
    AsyncCheckAudioCapturer("GetBufferDesc");
    std::shared_ptr<IAudioStream> currentStream = audioStream_;
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    int32_t ret = currentStream->GetBufferDesc(bufDesc);
    DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(bufDesc.buffer), bufDesc.bufLength);
    return ret;
}

int32_t AudioCapturerPrivate::GetMicInBufferSize(BufferDesc &bufDesc, size_t &processBufSize,
    size_t &micInBufSize, size_t &ecBufSize)
{
    processBufSize = 0;
    micInBufSize = 0;
    ecBufSize = 0;
    CHECK_AND_RETURN_RET_LOG(bufDesc.buffer != nullptr, ERROR_INVALID_PARAM, "bufDesc buffer is null");

    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE, "audioStream_ is nullptr");

    AudioStreamParams audioStreamParams;
    int32_t ret = currentStream->GetAudioStreamInfo(audioStreamParams);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "GetAudioStreamInfo failed");
    int32_t sampleByteSize = GetFormatByteSize(audioStreamParams.format);
    CHECK_AND_RETURN_RET_LOG(sampleByteSize > 0, ERROR_INVALID_PARAM, "invalid sample format");
    const size_t processFrameSize =
        static_cast<size_t>(audioStreamParams.channels) * static_cast<size_t>(sampleByteSize);
    CHECK_AND_RETURN_RET_LOG(processFrameSize > 0, ERROR_INVALID_PARAM, "invalid process frame size");
    bool degradeFallback = IsDegradeVoiceRecognitionMicInEcRequest(capturerInfo_, audioStreamParams);

    size_t validDataLength = (bufDesc.dataLength > 0 && bufDesc.dataLength <= bufDesc.bufLength) ?
        bufDesc.dataLength : bufDesc.bufLength;
    if (!degradeFallback) {
        MicInEcBufferLayout layout {};
        ret = ParseMicInEcBufferLayout(audioStreamParams, layout);
        CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, ret, "ParseMicInEcBufferLayout failed");
        CHECK_AND_RETURN_RET_LOG(validDataLength >= layout.byteSizePerFrame, ERROR_INVALID_PARAM,
            "buffer length is invalid");
        CHECK_AND_RETURN_RET_LOG(validDataLength % layout.byteSizePerFrame == 0, ERROR_INVALID_PARAM,
            "buffer length is not frame aligned");
        size_t frameCount = validDataLength / layout.byteSizePerFrame;
        processBufSize = frameCount * layout.processChannels * layout.byteSizePerSample;
        micInBufSize = frameCount * layout.micInChannels * layout.byteSizePerSample;
        ecBufSize = frameCount * layout.ecChannels * layout.byteSizePerSample;
        return SUCCESS;
    }

    CHECK_AND_RETURN_RET_LOG(validDataLength >= processFrameSize, ERROR_INVALID_PARAM, "buffer length is invalid");
    CHECK_AND_RETURN_RET_LOG(validDataLength % processFrameSize == 0, ERROR_INVALID_PARAM,
        "buffer length is not process frame aligned");
    size_t frameCount = validDataLength / processFrameSize;
    processBufSize = frameCount * processFrameSize;
    micInBufSize = frameCount * static_cast<size_t>(audioStreamParams.micInChannels) *
        static_cast<size_t>(sampleByteSize);
    ecBufSize = frameCount * static_cast<size_t>(audioStreamParams.ecChannels) * static_cast<size_t>(sampleByteSize);
    AUDIO_INFO_LOG("GetMicInBufferSize use degrade fallback, frameCount:%{public}zu", frameCount);
    return SUCCESS;
}

int32_t AudioCapturerPrivate::DeinterleaveBuffer(BufferDesc &bufDesc, BufferDesc &processBufDesc,
    BufferDesc &micInBufDesc, BufferDesc &ecBufferDesc)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(bufDesc.buffer != nullptr, ERROR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_INVALID_PARAM, "bufDesc buffer is null", true), "bufDesc buffer is null");
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_ILLEGAL_STATE, "audioStream_ is nullptr", true), "audioStream_ is nullptr");
    AudioStreamParams audioStreamParams;
    int32_t ret = currentStream->GetAudioStreamInfo(audioStreamParams);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_OPERATION_FAILED, "GetAudioStreamInfo failed", true), "GetAudioStreamInfo failed");

    MicInEcBufferSizes bufferSizes {};
    ret = GetMicInBufferSize(bufDesc, bufferSizes.processBufSize, bufferSizes.micInBufSize,
        bufferSizes.ecBufSize);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_OPERATION_FAILED, "GetMicInBufferSize failed", true), "GetMicInBufferSize failed");
    DeinterleaveTargetBuffers targetBuffers {processBufDesc, micInBufDesc, ecBufferDesc};
    DeinterleaveDumpFiles dumpFiles {dumpProcessFile_, dumpMicInFile_, dumpEcFile_};
    ret = ValidateDeinterleaveOutputBuffer(targetBuffers, bufferSizes);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_INVALID_PARAM, "output buffer is invalid", true), "output buffer is invalid");

    if (IsDegradeVoiceRecognitionMicInEcRequest(capturerInfo_, audioStreamParams)) {
        ret = FillDegradeFallbackSilentBuffers(bufDesc, processBufDesc, micInBufDesc, ecBufferDesc, bufferSizes);
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_SEND_DATA_OPERATION_FAILED, "FillDegradeFallbackSilentBuffers failed", true),
            "FillDegradeFallbackSilentBuffers failed");
        UpdateDeinterleaveDataLength(processBufDesc, micInBufDesc, ecBufferDesc, bufferSizes);
        DumpDeinterleaveBuffers(dumpFiles, targetBuffers, bufferSizes);
        AUDIO_INFO_LOG("DeinterleaveBuffer use degrade fallback, micIn/ec filled with silence");
        return SUCCESS;
    }

    MicInEcBufferLayout layout {};
    ret = GetMicInEcBufferLayoutFromStream(currentStream, layout);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_OPERATION_FAILED, "GetMicInEcBufferLayoutFromStream failed", true),
        "GetMicInEcBufferLayoutFromStream failed");
    ret = DeinterleaveMicInEcFrames(bufDesc, layout, bufferSizes, targetBuffers);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_OPERATION_FAILED, "DeinterleaveMicInEcFrames err", true), "DeinterleaveMicInEcFrames err");

    UpdateDeinterleaveDataLength(processBufDesc, micInBufDesc, ecBufferDesc, bufferSizes);
    DumpDeinterleaveBuffers(dumpFiles, targetBuffers, bufferSizes);
    return SUCCESS;
}

int32_t AudioCapturerPrivate::Enqueue(const BufferDesc &bufDesc)
{
    AsyncCheckAudioCapturer("Enqueue");
    std::shared_ptr<IAudioStream> currentStream = audioStream_;
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    CheckSignalData(bufDesc.buffer, bufDesc.bufLength);
    return currentStream->Enqueue(bufDesc);
}

int32_t AudioCapturerPrivate::Clear() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_SEND_DATA_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    return currentStream->Clear();
}

int32_t AudioCapturerPrivate::GetBufQueueState(BufferQueueState &bufState) const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    return currentStream->GetBufQueueState(bufState);
}

void AudioCapturerPrivate::SetValid(bool valid)
{
    std::lock_guard<std::mutex> lock(lock_);
    isValid_ = valid;
}

int64_t AudioCapturerPrivate::GetFramesRead() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    return currentStream->GetFramesRead();
}

int32_t AudioCapturerPrivate::GetCurrentInputDevices(AudioDeviceDescriptor &deviceInfo) const
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    uint32_t sessionId = static_cast<uint32_t>(-1);
    int32_t ret = GetAudioStreamId(sessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(!ret, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_INVALID_HANDLE, "Get sessionId failed", true),
        "Get sessionId failed");

    ret = AudioPolicyManager::GetInstance().GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(!ret, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_OPERATION_FAILED, "Get current capturer devices failed", true),
        "Get current capturer devices failed");

    for (auto it = audioCapturerChangeInfos.begin(); it != audioCapturerChangeInfos.end(); it++) {
        if ((*it)->sessionId == static_cast<int32_t>(sessionId)) {
            deviceInfo = (*it)->inputDeviceInfo;
        }
    }
    return SUCCESS;
}

int32_t AudioCapturerPrivate::GetCurrentCapturerChangeInfo(AudioCapturerChangeInfo &changeInfo) const
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    uint32_t sessionId = static_cast<uint32_t>(-1);
    int32_t ret = GetAudioStreamId(sessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(!ret, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_INVALID_HANDLE, "Get sessionId failed", true),
        "Get sessionId failed");

    ret = AudioPolicyManager::GetInstance().GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(!ret, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_OPERATION_FAILED, "Get current capturer devices failed", true),
        "Get current capturer devices failed");

    for (auto it = audioCapturerChangeInfos.begin(); it != audioCapturerChangeInfos.end(); it++) {
        if ((*it)->sessionId == static_cast<int32_t>(sessionId)) {
            changeInfo = *(*it);
        }
    }
    return SUCCESS;
}

std::vector<sptr<MicrophoneDescriptor>> AudioCapturerPrivate::GetCurrentMicrophones() const
{
    uint32_t sessionId = static_cast<uint32_t>(-1);
    GetAudioStreamId(sessionId);
    return AudioPolicyManager::GetInstance().GetAudioCapturerMicrophoneDescriptors(static_cast<int32_t>(sessionId));
}

int32_t AudioCapturerPrivate::SetAudioCapturerDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(callback != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_INVALID_PARAM, "Callback is null", true),
        "Callback is null");

    if (RegisterAudioCapturerEventListener() != SUCCESS) {
        return ERROR;
    }

    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStateChangeCallback_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "audioStateChangeCallback_ is null", true),
        "audioStateChangeCallback_ is null");
    audioStateChangeCallback_->SaveDeviceChangeCallback(callback);
    return SUCCESS;
}

int32_t AudioCapturerPrivate::RemoveAudioCapturerDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStateChangeCallback_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "audioStateChangeCallback_ is null", true),
        "audioStateChangeCallback_ is null");

    audioStateChangeCallback_->RemoveDeviceChangeCallback(callback);
    if (UnregisterAudioCapturerEventListener() != SUCCESS) {
        return ERROR;
    }
    return SUCCESS;
}

bool AudioCapturerPrivate::IsDeviceChanged(AudioDeviceDescriptor &newDeviceInfo)
{
    bool deviceUpdated = false;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);

    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(GetCurrentInputDevicesInner(deviceInfo) == SUCCESS, deviceUpdated,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_OPERATION_FAILED, "GetCurrentInputDevices failed", true),
        "GetCurrentInputDevices failed");

    if (currentDeviceInfo_.deviceType_ != deviceInfo.deviceType_) {
        currentDeviceInfo_ = deviceInfo;
        newDeviceInfo = currentDeviceInfo_;
        deviceUpdated = true;
    }
    return deviceUpdated;
}

void AudioCapturerPrivate::GetAudioInterrupt(AudioInterrupt &audioInterrupt)
{
    std::lock_guard<std::mutex> lock(audioInterruptMutex_);
    audioInterrupt = audioInterrupt_;
}

void AudioCapturerPrivate::SetAudioInterrupt(const AudioInterrupt &audioInterrupt)
{
    std::lock_guard<std::mutex> lock(audioInterruptMutex_);
    audioInterrupt_ = audioInterrupt;
}

void AudioCapturerPrivate::WriteOverflowEvent() const
{
    AUDIO_INFO_LOG("Write overflowEvent to media monitor");
    if (GetOverflowCountInner() < WRITE_OVERFLOW_NUM) {
        return;
    }
    AudioPipeType pipeType = PIPE_TYPE_IN_NORMAL;
    IAudioStream::StreamClass streamClass = audioStream_->GetStreamClass();
    if (streamClass == IAudioStream::FAST_STREAM) {
        pipeType = PIPE_TYPE_IN_LOWLATENCY;
    }
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::PERFORMANCE_UNDER_OVERRUN_STATS,
        Media::MediaMonitor::EventType::FREQUENCY_AGGREGATION_EVENT);
    bean->Add("IS_PLAYBACK", 0);
    bean->Add("CLIENT_UID", appInfo_.appUid);
    bean->Add("PIPE_TYPE", pipeType);
    bean->Add("STREAM_TYPE", capturerInfo_.sourceType);
    Media::MediaMonitor::MediaMonitorManager::GetInstance().WriteLogMsg(bean);
}

int32_t AudioCapturerPrivate::RegisterAudioCapturerEventListener()
{
    if (!audioStateChangeCallback_) {
        audioStateChangeCallback_ = std::make_shared<AudioCapturerStateChangeCallbackImpl>();
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStateChangeCallback_, ERROR,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_MEMORY_ALLOC_FAILED, "Memory allocation failed", true),
            "Memory allocation failed!!");

        int32_t ret =
            AudioPolicyManager::GetInstance().RegisterAudioCapturerEventListener(getpid(), audioStateChangeCallback_);
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == 0, ERROR,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_OPERATION_FAILED, "RegisterAudioCapturerEventListener failed", true),
            "RegisterAudioCapturerEventListener failed");
        audioStateChangeCallback_->SetAudioCapturerObj(weak_from_this());
    }
    return SUCCESS;
}

int32_t AudioCapturerPrivate::UnregisterAudioCapturerEventListener()
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStateChangeCallback_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "audioStateChangeCallback_ is null", true),
        "audioStateChangeCallback_ is null");
    if (audioStateChangeCallback_->DeviceChangeCallbackArraySize() == 0 &&
        audioStateChangeCallback_->GetCapturerInfoChangeCallbackArraySize() == 0) {
        int32_t ret =
            AudioPolicyManager::GetInstance().UnregisterAudioCapturerEventListener(getpid());
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == 0, ERROR,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_OPERATION_FAILED, "UnregisterAudioCapturerEventListener failed", true),
            "failed");
        audioStateChangeCallback_->HandleCapturerDestructor();
        audioStateChangeCallback_ = nullptr;
    }
    return SUCCESS;
}

int32_t AudioCapturerPrivate::SetAudioCapturerInfoChangeCallback(
    const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(callback != nullptr, ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_INVALID_PARAM, "Callback is null", true),
        "Callback is null");

    CHECK_AND_CALL_FUNC_RETURN_RET(RegisterAudioCapturerEventListener() == SUCCESS, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_OPERATION_FAILED, "RegisterAudioCapturerEventListener failed", true));

    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStateChangeCallback_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "audioStateChangeCallback_ is null", true),
        "audioStateChangeCallback_ is null");
    audioStateChangeCallback_->SaveCapturerInfoChangeCallback(callback);
    return SUCCESS;
}

int32_t AudioCapturerPrivate::RemoveAudioCapturerInfoChangeCallback(
    const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStateChangeCallback_ != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "audioStateChangeCallback_ is null", true),
        "audioStateChangeCallback_ is null");
    audioStateChangeCallback_->RemoveCapturerInfoChangeCallback(callback);
    CHECK_AND_CALL_FUNC_RETURN_RET(UnregisterAudioCapturerEventListener() == SUCCESS, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_OPERATION_FAILED, "UnregisterAudioCapturerEventListener failed", true));
    return SUCCESS;
}

int32_t AudioCapturerPrivate::RegisterCapturerPolicyServiceDiedCallback(const std::shared_ptr<IAudioStream> &ipcStream)
{
    std::lock_guard<std::mutex> lock(capturerPolicyServiceDiedCbMutex_);
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::SetCapturerPolicyServiceDiedCallback");
    if (!audioPolicyServiceDiedCallback_) {
        audioPolicyServiceDiedCallback_ = std::make_shared<CapturerPolicyServiceDiedCallback>();
        if (!audioPolicyServiceDiedCallback_) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_MEMORY_ALLOC_FAILED, "Memory allocation failed", true);
            AUDIO_ERR_LOG("Memory allocation failed!!");
            return ERROR;
        }
        audioPolicyServiceDiedCallback_->SetAudioCapturerObj(weak_from_this());
        audioPolicyServiceDiedCallback_->SetAudioInterrupt(audioInterrupt_);
    }
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ipcStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CALLBACK_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    ipcStream->RegisterRemoteDiedCallback(audioPolicyServiceDiedCallback_);
    return SUCCESS;
}

int32_t AudioCapturerPrivate::RemoveCapturerPolicyServiceDiedCallback(const std::shared_ptr<IAudioStream> &ipcStream)
{
    std::lock_guard<std::mutex> lock(capturerPolicyServiceDiedCbMutex_);
    AUDIO_DEBUG_LOG("AudioCapturerPrivate::RemoveCapturerPolicyServiceDiedCallback");
    if (audioPolicyServiceDiedCallback_) {
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ipcStream != nullptr, ERROR_ILLEGAL_STATE,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_ILLEGAL_STATE, "audioStream_ is nullptr", true),
            "audioStream_ is nullptr");
        ipcStream->UnregisterRemoteDiedCallback();
    }
    audioPolicyServiceDiedCallback_ = nullptr;
    return SUCCESS;
}

uint32_t AudioCapturerPrivate::GetOverflowCount() const
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    return currentStream->GetOverflowCount();
}

void AudioCapturerPrivate::ReconfigBufferSize(IAudioStream::SwitchInfo &info, std::shared_ptr<IAudioStream> audioStream)
{
    CHECK_AND_RETURN(info.userSettedPreferredFrameSize.has_value());
    // audioStream is checked in SetSwitchInfo
    audioStream->SetPreferredFrameSize(info.userSettedPreferredFrameSize.value(), true);
}

int32_t AudioCapturerPrivate::SetSwitchInfo(IAudioStream::SwitchInfo info, std::shared_ptr<IAudioStream> audioStream)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStream != nullptr, ERROR,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_ILLEGAL_STATE, "stream is nullptr", true),
        "stream is nullptr");

    audioStream->SetStreamTrackerState(false);
    audioStream->SetClientID(info.clientPid, info.clientUid, appInfo_.appTokenId, appInfo_.appFullTokenId);
    audioStream->SetClientDeviceId(appInfo_.deviceId);
    audioStream->SetCapturerInfo(info.capturerInfo);
    SetInnerStreamFastStatusChangeCallback(audioStream);
    int32_t res = audioStream->SetAudioStreamInfo(info.params, capturerProxyObj_);
    CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(res == SUCCESS, ERROR,
        HILOG_COMM_ERROR("[SetSwitchInfo]SetAudioStreamInfo failed"),
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_ILLEGAL_STATE, "[SetSwitchInfo]SetAudioStreamInfo failed", true));
    audioStream->SetCaptureMode(info.captureMode);
    callbackLoopTid_ = audioStream->GetCallbackLoopTid();

    ReconfigBufferSize(info, audioStream);

    // set callback
    if ((info.renderPositionCb != nullptr) && (info.frameMarkPosition > 0)) {
        audioStream->SetRendererPositionCallback(info.frameMarkPosition, info.renderPositionCb);
    }

    if ((info.capturePositionCb != nullptr) && (info.frameMarkPosition > 0)) {
        audioStream->SetCapturerPositionCallback(info.frameMarkPosition, info.capturePositionCb);
    }

    if ((info.renderPeriodPositionCb != nullptr) && (info.framePeriodNumber > 0)) {
        audioStream->SetRendererPeriodPositionCallback(info.framePeriodNumber, info.renderPeriodPositionCb);
    }

    if ((info.capturePeriodPositionCb != nullptr) && (info.framePeriodNumber > 0)) {
        audioStream->SetCapturerPeriodPositionCallback(info.framePeriodNumber, info.capturePeriodPositionCb);
    }

    audioStream->SetCapturerReadCallback(info.capturerReadCallback);

    audioStream->SetStreamCallback(info.audioStreamCallback);
    return SUCCESS;
}

void AudioCapturerPrivate::InitSwitchInfo(IAudioStream::SwitchInfo &switchInfo)
{
    audioStream_->GetSwitchInfo(switchInfo);

    switchInfo.captureMode = audioCaptureMode_;
    switchInfo.params.originalSessionId = sessionID_;
    return;
}

bool AudioCapturerPrivate::FinishOldStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo,
    CapturerState previousState, IAudioStream::SwitchInfo &switchInfo)
{
    bool switchResult = false;
    if (previousState == CAPTURER_RUNNING) {
        // stop old stream
        switchResult = audioStream_->StopAudioStream();
        if (restoreInfo.restoreReason != SERVER_DIED) {
            JUDGE_AND_ERR_LOG(!switchResult, "StopAudioStream failed.");
        }
    }
    // switch new stream
    InitSwitchInfo(switchInfo);
    if (restoreInfo.restoreReason == SERVER_DIED) {
        AUDIO_INFO_LOG("Server died, reset session id: %{public}d", switchInfo.params.originalSessionId);
        switchInfo.params.originalSessionId = 0;
        switchInfo.sessionId = 0;
    }

    RemoveCapturerPolicyServiceDiedCallback(audioStream_);
    // release old stream and restart audio stream
    switchResult = audioStream_->ReleaseAudioStream(true, true);
    if (restoreInfo.restoreReason != SERVER_DIED) {
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(switchResult, false,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_DEVICE_SWITCH_OPERATION_FAILED, "release old stream failed", true),
            "release old stream failed.");
    }
    return true;
}

bool AudioCapturerPrivate::CheckForGenerateNewStream()
{
    if (capturerInfo_.sourceType == SOURCE_TYPE_PLAYBACK_CAPTURE) {
        auto ret = UpdatePlaybackCaptureConfig(filterConfig_);
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, false,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_DEVICE_SWITCH_OPERATION_FAILED, "UpdatePlaybackCaptureConfig Failed", true),
            "UpdatePlaybackCaptureConfig Failed!");
    }
    if (audioInterruptCallback_ != nullptr) {
        std::shared_ptr<AudioCapturerInterruptCallbackImpl> interruptCbImpl =
            std::static_pointer_cast<AudioCapturerInterruptCallbackImpl>(audioInterruptCallback_);
        interruptCbImpl->UpdateAudioStream(audioStream_);
    }
    return true;
}

bool AudioCapturerPrivate::GenerateNewStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo,
    CapturerState previousState, IAudioStream::SwitchInfo &switchInfo)
{
    std::shared_ptr<AudioStreamDescriptor> streamDesc = GenerateStreamDesc(switchInfo, restoreInfo);
    int32_t ret = IAudioStream::CheckCapturerAudioStreamInfo(switchInfo.params);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_INVALID_PARAM, "CheckCapturerAudioStreamInfo fail", true),
        "CheckCapturerAudioStreamInfo fail!");

    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    ret = AudioPolicyManager::GetInstance().CreateCapturerClient(
        streamDesc, flag, switchInfo.params.originalSessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_OPERATION_FAILED, "CreateCapturerClient failed", true),
        "CreateCapturerClient failed");
    UpdateVoiceRecognitionMicInEcDegradeFlag(switchInfo.capturerInfo, switchInfo.params);

    targetClass = DecideStreamClassAndUpdateCapturerInfo(flag);
    if (targetClass == IAudioStream::VOIP_STREAM) {
        switchInfo.capturerInfo.originalFlag = AUDIO_FLAG_VOIP_FAST;
        switchInfo.capturerInfo.capturerFlags = AUDIO_FLAG_VOIP_FAST;
    } else if (targetClass == IAudioStream::FAST_STREAM) {
        switchInfo.capturerInfo.originalFlag = AUDIO_FLAG_MMAP;
        switchInfo.capturerInfo.capturerFlags = AUDIO_FLAG_MMAP;
    }
    std::shared_ptr<IAudioStream> newAudioStream = IAudioStream::GetRecordStream(targetClass, switchInfo.params,
        switchInfo.eStreamType, appInfo_.appUid);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(newAudioStream != nullptr, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
        RECORD_DEVICE_SWITCH_NULL_POINTER, "GetRecordStream failed", true), "GetRecordStream failed.");

    // set new stream info
    int32_t initResult = SetSwitchInfo(switchInfo, newAudioStream);
    if (initResult != SUCCESS && switchInfo.capturerInfo.originalFlag != AUDIO_FLAG_NORMAL) {
        CHECK_AND_CALL_FUNC_RETURN_RET_LOG(FallbackToNormalStreamForGenerateNewStream(restoreInfo, streamDesc, flag,
            switchInfo, newAudioStream), false,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_OPERATION_FAILED, "FallbackToNormalStreamForGenerateNewStream failed", true),
            "FallbackToNormalStreamForGenerateNewStream failed");
    }

    std::shared_ptr<IAudioStream> oldAudioStream = audioStream_;
    // Operation of replace audioStream_ must be performed before StartAudioStream.
    // Otherwise GetBufferDesc will return the buffer pointer of oldStream (causing Use-After-Free).
    audioStream_ = newAudioStream;
    RegisterCapturerPolicyServiceDiedCallback(audioStream_);
    auto checkRet = CheckForGenerateNewStream();
    CHECK_AND_CALL_FUNC_RETURN_RET(checkRet == true, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
        RECORD_DEVICE_SWITCH_OPERATION_FAILED, "UpdatePlaybackCaptureConfig Failed", true));

    bool restartResult = RestartAudioStream(newAudioStream, previousState);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(restartResult, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
        RECORD_DEVICE_SWITCH_OPERATION_FAILED, "start new stream failed", true), "start new stream failed.");
    return true;
}

bool AudioCapturerPrivate::FallbackToNormalStreamForGenerateNewStream(const RestoreInfo &restoreInfo,
    std::shared_ptr<AudioStreamDescriptor> &streamDesc, uint32_t &flag, IAudioStream::SwitchInfo &switchInfo,
    std::shared_ptr<IAudioStream> &newAudioStream)
{
    AUDIO_ERR_LOG("Re-create stream failed, crate normal ipc stream");
    if (restoreInfo.restoreReason == SERVER_DIED) {
        switchInfo.sessionId = switchInfo.params.originalSessionId;
        streamDesc->sessionId_ = switchInfo.params.originalSessionId;
    }
    streamDesc->capturerInfo_.capturerFlags = AUDIO_FLAG_FORCED_NORMAL;
    streamDesc->routeFlag_ = AUDIO_FLAG_NONE;
    int32_t ret = AudioPolicyManager::GetInstance().CreateCapturerClient(
        streamDesc, flag, switchInfo.params.originalSessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET_REPORT(ret == SUCCESS, false,
        HILOG_COMM_ERROR("[FallbackToNormalStreamForGenerateNewStream]CreateRendererClient failed"),
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_DEVICE_SWITCH_ILLEGAL_STATE,
                "[FallbackToNormalStreamForGenerateNewStream]CreateRendererClient failed", true));
    UpdateVoiceRecognitionMicInEcDegradeFlag(switchInfo.capturerInfo, switchInfo.params);

    newAudioStream = IAudioStream::GetRecordStream(IAudioStream::PA_STREAM, switchInfo.params,
        switchInfo.eStreamType, appInfo_.appUid);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(newAudioStream != nullptr, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_NULL_POINTER, "Get ipc stream failed", true),
        "Get ipc stream failed");
    ret = SetSwitchInfo(switchInfo, newAudioStream);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_OPERATION_FAILED, "Init ipc stream failed", true),
        "Init ipc strean failed");
    return true;
}

bool AudioCapturerPrivate::RestartAudioStream(std::shared_ptr<IAudioStream> newAudioStream,
    CapturerState previousState)
{
    bool switchResult = true;
    if (previousState == CAPTURER_RUNNING) {
        // restart audio stream
        newAudioStream->SetRebuildFlag();
        switchResult = newAudioStream->StartAudioStream();
    }
    return switchResult;
}

bool AudioCapturerPrivate::ContinueAfterSplit(RestoreInfo restoreInfo)
{
    CHECK_AND_CALL_FUNC_RETURN_RET(restoreInfo.restoreReason == STREAM_SPLIT, true,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_ILLEGAL_STATE, "restoreReason not STREAM_SPLIT", true));
    audioStream_->FetchDeviceForSplitStream();
    return false;
}

bool AudioCapturerPrivate::SwitchToTargetStream(IAudioStream::StreamClass targetClass, RestoreInfo restoreInfo)
{
    bool switchResult = false;

    Trace trace("KeyAction AudioCapturer::SwitchToTargetStream " + std::to_string(sessionID_)
        + ", target class " + std::to_string(targetClass) + ", reason " + std::to_string(restoreInfo.restoreReason)
        + ", device change reason " + std::to_string(restoreInfo.deviceChangeReason)
        + ", target flag " + std::to_string(restoreInfo.targetStreamFlag));
    AUDIO_INFO_LOG("Restore AudioCapturer %{public}u, target class %{public}d, reason: %{public}d, "
        "device change reason %{public}d, target flag %{public}d", sessionID_, targetClass,
        restoreInfo.restoreReason, restoreInfo.deviceChangeReason, restoreInfo.targetStreamFlag);

    CHECK_AND_CALL_FUNC_RETURN_RET(ContinueAfterSplit(restoreInfo), true,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_ILLEGAL_STATE, "ContinueAfterSplit returned true", true));

    isSwitching_ = true;
    CapturerState previousState = GetStatusInner();
    IAudioStream::SwitchInfo switchInfo;

    // Stop old stream, get stream info and frames written for new stream, and release old stream.
    switchResult = FinishOldStream(targetClass, restoreInfo, previousState, switchInfo);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(switchResult, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_OPERATION_FAILED, "Finish old stream failed", true),
        "Finish old stream failed");

    // Create and start new stream.
    switchResult = GenerateNewStream(targetClass, restoreInfo, previousState, switchInfo);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(switchResult, false,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_OPERATION_FAILED, "Generate new stream failed", true),
        "Generate new stream failed");

    // Activate audio interrupt again when restoring for audio server died.
    if (restoreInfo.restoreReason == SERVER_DIED) {
        HandleAudioInterruptWhenServerDied();
    }

    isSwitching_ = false;
    switchResult = true;

    return switchResult;
}

void AudioCapturerPrivate::HandleAudioInterruptWhenServerDied()
{
    if (GetStatusInner() == CAPTURER_RUNNING) {
        AudioInterrupt audioInterrupt = audioInterrupt_;
        int32_t ret = AudioPolicyManager::GetInstance().ActivateAudioInterrupt(audioInterrupt);
        if (ret != SUCCESS) {
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_DEVICE_SWITCH_OPERATION_FAILED, "Activate audio interrupt failed", true);
            AUDIO_WARNING_LOG("Activate audio interrupt failed when restoring from server died");
        }
    }
}

void AudioCapturerPrivate::FastStatusChangeCallback(FastStatus status)
{
    FastStatus newStatus = GetFastStatusInner();
    if (newStatus != status) {
        NotifyFastStatusChange(newStatus);
    }
}

void AudioCapturerPrivate::NotifyFastStatusChange(FastStatus status)
{
    std::shared_ptr<AudioCapturerFastStatusChangeCallback> callback;
    {
        std::lock_guard lock(fastStatusChangeCallbackMutex_);
        callback = fastStatusChangeCallback_;
    }
    CHECK_AND_RETURN_LOG(callback != nullptr, "fastStatusChangeCallback_ is nullptr");
    callback->OnFastStatusChange(status);
}

void AudioCapturerPrivate::SetInnerStreamFastStatusChangeCallback(const std::shared_ptr<IAudioStream> &audioStream)
{
    CHECK_AND_RETURN_LOG(audioStream != nullptr, "audioStream is nullptr");
    std::weak_ptr<AudioCapturerPrivate> weakCapturer = weak_from_this();
    audioStream->SetFastStatusChangeCallback([weakCapturer] (FastStatus status) {
        auto capturer = weakCapturer.lock();
        CHECK_AND_RETURN_LOG(capturer != nullptr, "capturer is nullptr");
        capturer->NotifyFastStatusChange(status);
    });
}

AudioCapturerStateChangeCallbackImpl::AudioCapturerStateChangeCallbackImpl()
{
    AUDIO_DEBUG_LOG("AudioCapturerStateChangeCallbackImpl instance create");
}

AudioCapturerStateChangeCallbackImpl::~AudioCapturerStateChangeCallbackImpl()
{
    AUDIO_DEBUG_LOG("AudioCapturerStateChangeCallbackImpl instance destory");
}

void AudioCapturerStateChangeCallbackImpl::SaveCapturerInfoChangeCallback(
    const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback)
{
    std::lock_guard<std::mutex> lock(capturerMutex_);
    auto iter = find(capturerInfoChangeCallbacklist_.begin(), capturerInfoChangeCallbacklist_.end(), callback);
    if (iter == capturerInfoChangeCallbacklist_.end()) {
        capturerInfoChangeCallbacklist_.emplace_back(callback);
    }
}

void AudioCapturerStateChangeCallbackImpl::RemoveCapturerInfoChangeCallback(
    const std::shared_ptr<AudioCapturerInfoChangeCallback> &callback)
{
    std::lock_guard<std::mutex> lock(capturerMutex_);
    if (callback == nullptr) {
        capturerInfoChangeCallbacklist_.clear();
        return;
    }

    auto iter = find(capturerInfoChangeCallbacklist_.begin(), capturerInfoChangeCallbacklist_.end(), callback);
    if (iter != capturerInfoChangeCallbacklist_.end()) {
        capturerInfoChangeCallbacklist_.erase(iter);
    }
}

int32_t AudioCapturerStateChangeCallbackImpl::GetCapturerInfoChangeCallbackArraySize()
{
    std::lock_guard<std::mutex> lock(capturerMutex_);
    return capturerInfoChangeCallbacklist_.size();
}

void AudioCapturerStateChangeCallbackImpl::SaveDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback)
{
    std::lock_guard<std::mutex> lock(deviceChangeCallbackMutex_);
    auto iter = find(deviceChangeCallbacklist_.begin(), deviceChangeCallbacklist_.end(), callback);
    if (iter == deviceChangeCallbacklist_.end()) {
        deviceChangeCallbacklist_.emplace_back(callback);
    }
}

void AudioCapturerStateChangeCallbackImpl::RemoveDeviceChangeCallback(
    const std::shared_ptr<AudioCapturerDeviceChangeCallback> &callback)
{
    std::lock_guard<std::mutex> lock(deviceChangeCallbackMutex_);
    if (callback == nullptr) {
        deviceChangeCallbacklist_.clear();
        return;
    }

    auto iter = find(deviceChangeCallbacklist_.begin(), deviceChangeCallbacklist_.end(), callback);
    if (iter != deviceChangeCallbacklist_.end()) {
        deviceChangeCallbacklist_.erase(iter);
    }
}

int32_t AudioCapturerStateChangeCallbackImpl::DeviceChangeCallbackArraySize()
{
    std::lock_guard<std::mutex> lock(deviceChangeCallbackMutex_);
    return deviceChangeCallbacklist_.size();
}

void AudioCapturerStateChangeCallbackImpl::SetAudioCapturerObj(
    std::weak_ptr<AudioCapturerPrivate> capturerObj)
{
    std::lock_guard<std::mutex> lock(capturerMutex_);
    capturer_ = capturerObj;
}

void AudioCapturerStateChangeCallbackImpl::NotifyAudioCapturerInfoChange(
    const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    uint32_t sessionId = static_cast<uint32_t>(-1);
    bool found = false;
    AudioCapturerChangeInfo capturerChangeInfo;
    std::vector<std::shared_ptr<AudioCapturerInfoChangeCallback>> capturerInfoChangeCallbacklist;

    {
        std::unique_lock lock(capturerMutex_);
        auto sharedCapturer = capturer_.lock();
        lock.unlock();
        CHECK_AND_CALL_FUNC_RETURN_LOG(sharedCapturer != nullptr,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_ILLEGAL_STATE, "sharedCapturer is nullptr", true),
            "sharedCapturer is nullptr");
        int32_t ret = sharedCapturer->GetAudioStreamId(sessionId);
        CHECK_AND_CALL_FUNC_RETURN_LOG(!ret,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_INVALID_HANDLE, "Get sessionId failed", true),
            "Get sessionId failed");
    }

    for (auto it = audioCapturerChangeInfos.begin(); it != audioCapturerChangeInfos.end(); it++) {
        if ((*it)->sessionId == static_cast<int32_t>(sessionId)) {
            capturerChangeInfo = *(*it);
            found = true;
        }
    }

    {
        std::lock_guard<std::mutex> lock(capturerMutex_);
        capturerInfoChangeCallbacklist = capturerInfoChangeCallbacklist_;
    }
    if (found) {
        for (auto it = capturerInfoChangeCallbacklist.begin(); it != capturerInfoChangeCallbacklist.end(); ++it) {
            if (*it != nullptr) {
                (*it)->OnStateChange(capturerChangeInfo);
            }
        }
    }
}

void AudioCapturerStateChangeCallbackImpl::NotifyAudioCapturerDeviceChange(
    const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    std::vector<std::shared_ptr<AudioCapturerDeviceChangeCallback>> deviceChangeCallbacklist;
    AudioDeviceDescriptor deviceInfo(AudioDeviceDescriptor::DEVICE_INFO);
    {
        std::unique_lock lock(capturerMutex_);
        auto sharedCapturer = capturer_.lock();
        lock.unlock();
        CHECK_AND_CALL_FUNC_RETURN_LOG(sharedCapturer != nullptr,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_ILLEGAL_STATE, "sharedCapturer is nullptr", true),
            "sharedCapturer is nullptr");
        CHECK_AND_CALL_FUNC_RETURN_LOG(sharedCapturer->IsDeviceChanged(deviceInfo),
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_DEVICE_SWITCH_DEVICE_MISMATCH, "Device not change, no need callback", true),
            "Device not change, no need callback.");
    }

    {
        std::lock_guard<std::mutex> lock(deviceChangeCallbackMutex_);
        deviceChangeCallbacklist = deviceChangeCallbacklist_;
    }
    for (auto it = deviceChangeCallbacklist.begin(); it != deviceChangeCallbacklist.end(); ++it) {
        if (*it != nullptr) {
            (*it)->OnStateChange(deviceInfo);
        }
    }
}

void AudioCapturerStateChangeCallbackImpl::OnCapturerStateChange(
    const std::vector<std::shared_ptr<AudioCapturerChangeInfo>> &audioCapturerChangeInfos)
{
    if (deviceChangeCallbacklist_.size() != 0) {
        NotifyAudioCapturerDeviceChange(audioCapturerChangeInfos);
    }

    if (capturerInfoChangeCallbacklist_.size() != 0) {
        NotifyAudioCapturerInfoChange(audioCapturerChangeInfos);
    }
}

void AudioCapturerStateChangeCallbackImpl::HandleCapturerDestructor()
{
    std::lock_guard<std::mutex> lock(capturerMutex_);
    capturer_.reset();
}

void InputDeviceChangeWithInfoCallbackImpl::OnDeviceChangeWithInfo(
    const uint32_t sessionId, const AudioDeviceDescriptor &deviceInfo, const AudioStreamDeviceChangeReasonExt reason,
    const AudioDeviceDescriptor &preDeviceInfo)
{
    AUDIO_INFO_LOG("For capturer, OnDeviceChangeWithInfo callback is not support");
}

void InputDeviceChangeWithInfoCallbackImpl::OnRecreateStreamEvent(const uint32_t sessionId, const int32_t streamFlag,
    const AudioStreamDeviceChangeReasonExt reason)
{
    AUDIO_INFO_LOG("Enter");
}

CapturerPolicyServiceDiedCallback::CapturerPolicyServiceDiedCallback()
{
    AUDIO_DEBUG_LOG("CapturerPolicyServiceDiedCallback create");
}

CapturerPolicyServiceDiedCallback::~CapturerPolicyServiceDiedCallback()
{
    AUDIO_DEBUG_LOG("CapturerPolicyServiceDiedCallback destroy");
}

void CapturerPolicyServiceDiedCallback::SetAudioCapturerObj(
    std::weak_ptr<AudioCapturerPrivate> capturerObj)
{
    capturer_ = capturerObj;
}

void CapturerPolicyServiceDiedCallback::SetAudioInterrupt(AudioInterrupt &audioInterrupt)
{
    audioInterrupt_ = audioInterrupt;
}

void CapturerPolicyServiceDiedCallback::OnAudioPolicyServiceDied()
{
    AUDIO_INFO_LOG("CapturerPolicyServiceDiedCallback OnAudioPolicyServiceDied");
    if (taskCount_.fetch_add(1) > 0) {
        AUDIO_INFO_LOG("direct ret");
        return;
    }

    auto weakRefCb = weak_from_this();
    std::thread restoreThread ([weakRefCb] {
        auto strongRefCb = weakRefCb.lock();
        CHECK_AND_CALL_FUNC_RETURN_LOG(strongRefCb != nullptr,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_CALLBACK_ILLEGAL_STATE, "strongRef is nullptr", true),
            "strongRef is nullptr");
        int32_t count;
        do {
            count = strongRefCb->taskCount_.load();
            strongRefCb->RestoreTheadLoop();
        } while (strongRefCb->taskCount_.fetch_sub(count) > count);
    });
    pthread_setname_np(restoreThread.native_handle(), "OS_ACPSRestore");
    restoreThread.detach();
}

void CapturerPolicyServiceDiedCallback::RestoreTheadLoop()
{
    int32_t tryCounter = 10;
    uint32_t sleepTime = 300000;
    bool restoreResult = false;
    while (!restoreResult && tryCounter > 0) {
        tryCounter--;
        usleep(sleepTime);
        auto sharedCapturer = capturer_.lock();
        CHECK_AND_CALL_FUNC_RETURN_LOG(sharedCapturer != nullptr,
            StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
                RECORD_DEVICE_SWITCH_ILLEGAL_STATE, "sharedCapturer is nullptr", true),
            "sharedRenderer is nullptr");
        if (sharedCapturer->audioStream_ == nullptr || sharedCapturer->abortRestore_) {
            AUDIO_INFO_LOG("abort restore");
            break;
        }
        sharedCapturer->RestoreAudioInLoop(restoreResult, tryCounter);
    }
}

void AudioCapturerPrivate::RestoreAudioInLoop(bool &restoreResult, int32_t &tryCounter)
{
    std::unique_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::unique_lock<std::shared_mutex>(capturerMutex_);
    }
    CHECK_AND_CALL_FUNC_RETURN_LOG(audioStream_,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_ILLEGAL_STATE, "audioStream_ is nullptr, no need for restore", true),
        "audioStream_ is nullptr, no need for restore");
    AUDIO_INFO_LOG("Restore AudioCapturer %{public}u when server died", sessionID_);
    RestoreInfo restoreInfo;
    restoreInfo.restoreReason = SERVER_DIED;
    restoreResult = SwitchToTargetStream(audioStream_->GetStreamClass(), restoreInfo);
    AUDIO_INFO_LOG("Set restore status when server died, restore result %{public}d", restoreResult);
    CHECK_AND_RETURN(restoreResult);
    CHECK_AND_RETURN(selectedDevice_ != nullptr);
    SelectInputDeviceInner(selectedDevice_);
    return;
}

// Inner function. Must be called with AudioCapturerPrivate::capturerMutex_ held
int32_t AudioCapturerPrivate::GetCurrentInputDevicesInner(AudioDeviceDescriptor &deviceInfo) const
{
    std::vector<std::shared_ptr<AudioCapturerChangeInfo>> audioCapturerChangeInfos;
    uint32_t sessionId = static_cast<uint32_t>(-1);
    int32_t ret = GetAudioStreamIdInner(sessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(!ret, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_INVALID_HANDLE, "Get sessionId failed", true),
        "Get sessionId failed");

    ret = AudioPolicyManager::GetInstance().GetCurrentCapturerChangeInfos(audioCapturerChangeInfos);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(!ret, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_OPERATION_FAILED, "Get current capturer devices failed", true),
        "Get current capturer devices failed");

    for (auto it = audioCapturerChangeInfos.begin(); it != audioCapturerChangeInfos.end(); it++) {
        if ((*it)->sessionId == static_cast<int32_t>(sessionId)) {
            deviceInfo = (*it)->inputDeviceInfo;
        }
    }
    return SUCCESS;
}

// Inner function. Must be called with AudioCapturerPrivate::capturerMutex_ held
int32_t AudioCapturerPrivate::GetAudioStreamIdInner(uint32_t &sessionID) const
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStream_ != nullptr, ERR_INVALID_HANDLE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_QUERY_ILLEGAL_STATE, "GetAudioStreamId failed", true),
        "GetAudioStreamId faied.");
    return audioStream_->GetAudioSessionID(sessionID);
}

// Inner function. Must be called with AudioCapturerPrivate::capturerMutex_ held
uint32_t AudioCapturerPrivate::GetOverflowCountInner() const
{
    return audioStream_->GetOverflowCount();
}

// Inner function. Must be called with AudioCapturerPrivate::capturerMutex_ held
CapturerState AudioCapturerPrivate::GetStatusInner() const
{
    return static_cast<CapturerState>(audioStream_->GetState());
}

std::shared_ptr<IAudioStream> AudioCapturerPrivate::GetInnerStream() const
{
    std::shared_lock<std::shared_mutex> lock;
    if (callbackLoopTid_ != gettid()) { // No need to add lock in callback thread to prevent deadlocks
        lock = std::shared_lock<std::shared_mutex>(capturerMutex_);
    }
    return audioStream_;
}
// LCOV_EXCL_STOP

std::shared_ptr<AudioStreamDescriptor> AudioCapturerPrivate::GenerateStreamDesc(
    const IAudioStream::SwitchInfo &switchInfo, const RestoreInfo &restoreInfo)
{
    auto streamDesc = std::make_shared<AudioStreamDescriptor>();

    streamDesc->audioMode_ = AUDIO_MODE_RECORD;
    streamDesc->createTimeStamp_ = ClockTime::GetCurNano();
    streamDesc->appInfo_ = appInfo_;
    streamDesc->callerUid_ = static_cast<int32_t>(getuid());
    streamDesc->callerPid_ = static_cast<int32_t>(getpid());

    // update with switchInfo
    AudioStreamInfo &streamInfo = streamDesc->streamInfo_;
    streamInfo.format = static_cast<AudioSampleFormat>(switchInfo.params.format);
    streamInfo.samplingRate = static_cast<AudioSamplingRate>(switchInfo.params.samplingRate);
    streamInfo.channels = static_cast<AudioChannel>(switchInfo.params.channels);
    streamInfo.encoding = static_cast<AudioEncodingType>(switchInfo.params.encoding);
    streamInfo.channelLayout = static_cast<AudioChannelLayout>(switchInfo.params.channelLayout);
    streamDesc->micInStreamInfo_.format = static_cast<AudioSampleFormat>(switchInfo.params.micInFormat);
    streamDesc->micInStreamInfo_.samplingRate = static_cast<AudioSamplingRate>(switchInfo.params.micInSamplingRate);
    streamDesc->micInStreamInfo_.channels = static_cast<AudioChannel>(switchInfo.params.micInChannels);
    streamDesc->micInStreamInfo_.encoding = static_cast<AudioEncodingType>(switchInfo.params.micInEncoding);
    streamDesc->micInStreamInfo_.channelLayout = static_cast<AudioChannelLayout>(switchInfo.params.micInChannelLayout);
    streamDesc->ecStreamInfo_.format = static_cast<AudioSampleFormat>(switchInfo.params.ecFormat);
    streamDesc->ecStreamInfo_.samplingRate = static_cast<AudioSamplingRate>(switchInfo.params.ecSamplingRate);
    streamDesc->ecStreamInfo_.channels = static_cast<AudioChannel>(switchInfo.params.ecChannels);
    streamDesc->ecStreamInfo_.encoding = static_cast<AudioEncodingType>(switchInfo.params.ecEncoding);
    streamDesc->ecStreamInfo_.channelLayout = static_cast<AudioChannelLayout>(switchInfo.params.ecChannelLayout);
    streamDesc->capturerInfo_= switchInfo.capturerInfo;
    streamDesc->sessionId_ = switchInfo.sessionId;

    // update with restoreInfo
    streamDesc->routeFlag_ = restoreInfo.routeFlag;
    if (restoreInfo.targetStreamFlag == AUDIO_FLAG_FORCED_NORMAL) {
        streamDesc->capturerInfo_.originalFlag = AUDIO_FLAG_FORCED_NORMAL;
    }

    return streamDesc;
}

void AudioCapturerPrivate::SetInterruptEventCallbackType(InterruptEventCallbackType callbackType)
{
    audioInterrupt_.callbackType = callbackType;
}

int32_t AudioCapturerPrivate::HandleCreateFastStreamError(AudioStreamParams &audioStreamParams)
{
    AUDIO_INFO_LOG("Create fast Stream fail, record by normal stream");
    IAudioStream::StreamClass streamClass = IAudioStream::PA_STREAM;
    capturerInfo_.capturerFlags = AUDIO_FLAG_FORCED_NORMAL;

    // Create stream desc and pipe
    std::shared_ptr<AudioStreamDescriptor> streamDesc = ConvertToStreamDescriptor(audioStreamParams);
    uint32_t flag = AUDIO_INPUT_FLAG_NORMAL;
    int32_t ret = AudioPolicyManager::GetInstance().CreateCapturerClient(streamDesc, flag,
        audioStreamParams.originalSessionId);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ERR_OPERATION_FAILED,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "CreateCapturerClient failed", true),
        "CreateCapturerClient failed");
    UpdateVoiceRecognitionMicInEcDegradeFlag(capturerInfo_, audioStreamParams);
    AUDIO_INFO_LOG("Create normal capturer, id: %{public}u", audioStreamParams.originalSessionId);

    audioStream_ = IAudioStream::GetRecordStream(streamClass, audioStreamParams, audioStreamType_, appInfo_.appUid);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStream_ != nullptr, ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_NULL_POINTER, "Get normal record stream failed", true),
        "Get normal record stream failed");
    ret = InitAudioStream(audioStreamParams);
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(ret == SUCCESS, ret,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CREATE_OPERATION_FAILED, "Init normal audio stream failed", true),
        "Init normal audio stream failed");
    audioStream_->SetCaptureMode(CAPTURE_MODE_CALLBACK);
    callbackLoopTid_ = audioStream_->GetCallbackLoopTid();
    return ret;
}

int32_t AudioCapturerPrivate::CheckAudioCapturer(std::string callingFunc)
{
    CheckAndStopAudioCapturer(callingFunc);
    return CheckAndRestoreAudioCapturer(callingFunc);
}

int32_t AudioCapturerPrivate::CheckAndStopAudioCapturer(std::string callingFunc)
{
    std::unique_lock<std::shared_mutex> lock(capturerMutex_, std::defer_lock);
    if (callbackLoopTid_ != gettid()) {
        lock.lock();
    }
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStream_, ERR_INVALID_PARAM,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_DEVICE_SWITCH_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    bool isNeedStop = audioStream_->GetStopFlag();
    if (!isNeedStop) {
        return SUCCESS;
    }

    AUDIO_INFO_LOG("Before %{public}s, stop audio capturer %{public}u", callingFunc.c_str(), sessionID_);
    if (lock.owns_lock()) {
        lock.unlock();
    }
    Stop();
    return SUCCESS;
}

int32_t AudioCapturerPrivate::StartPlaybackCapture()
{
    std::shared_ptr<IAudioStream> currentStream = GetInnerStream();
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(currentStream != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_START_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    return currentStream->RequestUserPrivacyAuthority(sessionID_);
}

int32_t AudioCapturerPrivate::SetInMainThreadState(bool isInMainThread)
{
    CHECK_AND_CALL_FUNC_RETURN_RET_LOG(audioStream_ != nullptr, ERROR_ILLEGAL_STATE,
        StreamDfxManager::GetInstance().SendAudioErrorEvent(static_cast<int32_t>(getuid()),
            RECORD_CONFIG_ILLEGAL_STATE, "audioStream_ is nullptr", true),
        "audioStream_ is nullptr");
    if (!isFirstTimeSetMainThreadState_) {
        return ERROR_ILLEGAL_STATE;
    }
    return audioStream_->SetInMainThreadState(isInMainThread);
}

int32_t AudioCapturerPrivate::SetIndependentAudioSessionStrategy(
    const AudioSessionStrategy &strategy, const uint32_t behavior)
{
    audioInterrupt_.behaviorFlags_ = {};
    if ((behavior & static_cast<uint32_t>(AudioSessionBehaviorFlags::VOIP_PRIVACY_TYPE_PUBLIC)) &&
        (audioInterrupt_.audioFocusType.sourceType == SOURCE_TYPE_VOICE_COMMUNICATION)) {
        audioInterrupt_.behaviorFlags_.voipNoPrivacyFlag = true;
        AUDIO_INFO_LOG("Capturer id: %{public}d activated VOIP_PRIVACY_TYPE_PUBLIC success", sessionID_);
    }
    if (behavior & static_cast<uint32_t>(AudioSessionBehaviorFlags::MUTE_WHEN_INTERRUPTED)) {
        audioInterrupt_.behaviorFlags_.muteWhenInterruptFlag = true;
        AUDIO_INFO_LOG("Capturer id: %{public}d activated MUTE_WHEN_INTERRUPTED success", sessionID_);
    }
    if ((behavior & static_cast<uint32_t>(AudioSessionBehaviorFlags::AIBASE_MUTE_VOIP)) &&
        PermissionUtil::VerifySystemPermission()) {
        audioInterrupt_.behaviorFlags_.aibaseMuteVoip = true;
        AUDIO_INFO_LOG("Capturer id: %{public}d activated AIBASE_MUTE_VOIP success", sessionID_);
    }
    audioInterrupt_.independentStrategy = strategy;
    AUDIO_INFO_LOG("Capturer id: %{public}d set independentStrategy: %{public}d",
        sessionID_, static_cast<int32_t>(strategy.concurrencyMode));
    return SUCCESS;
}
}  // namespace AudioStandard
}  // namespace OHOS
