/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "audio_service_client.h"

#include <fstream>
#include <sstream>

#include "safe_map.h"
#include "iservice_registry.h"
#include "audio_log.h"
#include "audio_utils.h"
#include "hisysevent.h"
#include "securec.h"
#include "system_ability_definition.h"
#include "unistd.h"
#include "audio_errors.h"
#include "audio_info.h"
#include "parameters.h"
#ifdef FEATURE_POWER_MANAGER
#include "power_mgr_client.h"
#endif

using namespace std;

namespace OHOS {
namespace AudioStandard {
static SafeMap<AudioServiceClient *, bool> serviceClientInstanceMap_;
AudioRendererCallbacks::~AudioRendererCallbacks() = default;
AudioCapturerCallbacks::~AudioCapturerCallbacks() = default;
const uint32_t CHECK_UTIL_SUCCESS = 0;
const uint32_t INIT_TIMEOUT_IN_SEC = 3;
const uint32_t CONNECT_TIMEOUT_IN_SEC = 10;
const uint32_t DRAIN_TIMEOUT_IN_SEC = 3;
const uint32_t CORK_TIMEOUT_IN_SEC = 3;
const uint32_t WRITE_TIMEOUT_IN_SEC = 8;
const uint32_t READ_TIMEOUT_IN_SEC = 5;
const uint32_t FLUSH_TIMEOUT_IN_SEC = 5;
const uint32_t MAINLOOP_WAIT_TIMEOUT_IN_SEC = 5;
const uint32_t DOUBLE_VALUE = 2;
const uint32_t MAX_LENGTH_FACTOR = 5;
const uint32_t T_LENGTH_FACTOR = 4;
const uint32_t MAX_LENGTH_OFFLOAD = 500;
const uint64_t MIN_BUF_DURATION_IN_USEC = 92880;
const uint32_t LATENCY_THRESHOLD = 35;
const int32_t NO_OF_PREBUF_TIMES = 6;
const int32_t OFFLOAD_HDI_CACHE1 = 200; // ms, should equal with val in hdi_sink.c
const int32_t OFFLOAD_HDI_CACHE2 = 7000; // ms, should equal with val in hdi_sink.c
const uint32_t OFFLOAD_BUFFER = 50;
const uint64_t AUDIO_US_PER_MS = 1000;
const uint64_t AUDIO_NS_PER_US = 1000;
const uint64_t AUDIO_S_TO_NS = 1000000000;
const uint64_t HDI_OFFLOAD_SAMPLE_RATE = 48000;
const int64_t SECOND_TO_MICROSECOND = 1000000;
const uint64_t AUDIO_FIRST_FRAME_LATENCY = 230; //ms

const std::string FORCED_DUMP_PULSEAUDIO_STACKTRACE = "dump_pulseaudio_stacktrace";
const std::string RECOVERY_AUDIO_SERVER = "recovery_audio_server";

static const std::unordered_map<AudioStreamType, std::string> STREAM_TYPE_ENUM_STRING_MAP = {
    {STREAM_VOICE_CALL, "voice_call"},
    {STREAM_MUSIC, "music"},
    {STREAM_RING, "ring"},
    {STREAM_MEDIA, "media"},
    {STREAM_VOICE_ASSISTANT, "voice_assistant"},
    {STREAM_SYSTEM, "system"},
    {STREAM_ALARM, "alarm"},
    {STREAM_NOTIFICATION, "notification"},
    {STREAM_BLUETOOTH_SCO, "bluetooth_sco"},
    {STREAM_ENFORCED_AUDIBLE, "enforced_audible"},
    {STREAM_DTMF, "dtmf"},
    {STREAM_TTS, "tts"},
    {STREAM_ACCESSIBILITY, "accessibility"},
    {STREAM_RECORDING, "recording"},
    {STREAM_MOVIE, "movie"},
    {STREAM_GAME, "game"},
    {STREAM_SPEECH, "speech"},
    {STREAM_SYSTEM_ENFORCED, "system_enforced"},
    {STREAM_ULTRASONIC, "ultrasonic"},
    {STREAM_WAKEUP, "wakeup"},
    {STREAM_VOICE_MESSAGE, "voice_message"},
    {STREAM_NAVIGATION, "navigation"},
    {STREAM_SOURCE_VOICE_CALL, "source_voice_call"}
};

static int32_t CheckReturnIfinvalid(bool expr, const int32_t retVal)
{
    do {
        if (!(expr)) {
            return retVal;
        }
    } while (false);
    return CHECK_UTIL_SUCCESS;
}

static int32_t CheckPaStatusIfinvalid(pa_threaded_mainloop *mainLoop, pa_context *context,
    pa_stream *paStream, const int32_t retVal)
{
    return CheckReturnIfinvalid(mainLoop && context &&
        paStream && PA_CONTEXT_IS_GOOD(pa_context_get_state(context)) &&
        PA_STREAM_IS_GOOD(pa_stream_get_state(paStream)), retVal);
}

static int32_t CheckPaStatusIfinvalid(pa_threaded_mainloop *mainLoop, pa_context *context,
    pa_stream *paStream, const int32_t retVal, int32_t &pError)
{
    if (CheckPaStatusIfinvalid(mainLoop, context, paStream, -1) < 0) {
        pError = pa_context_errno(context);
        return -1;
    }
    return retVal;
}

AudioStreamParams AudioServiceClient::ConvertFromPAAudioParams(pa_sample_spec paSampleSpec)
{
    AudioStreamParams audioParams;

    audioParams.channels = paSampleSpec.channels;
    audioParams.samplingRate = paSampleSpec.rate;
    audioParams.encoding = ENCODING_PCM;

    switch (paSampleSpec.format) {
        case PA_SAMPLE_U8:
            audioParams.format = SAMPLE_U8;
            break;
        case PA_SAMPLE_S16LE:
            audioParams.format = SAMPLE_S16LE;
            break;
        case PA_SAMPLE_S24LE:
            audioParams.format = SAMPLE_S24LE;
            break;
        case PA_SAMPLE_S32LE:
            audioParams.format = SAMPLE_S32LE;
            break;
        default:
            audioParams.format = INVALID_WIDTH;
            break;
    }

    return audioParams;
}

pa_sample_spec AudioServiceClient::ConvertToPAAudioParams(AudioStreamParams audioParams)
{
    pa_sample_spec paSampleSpec;

    paSampleSpec.channels = audioParams.channels;
    paSampleSpec.rate     = audioParams.samplingRate;

    switch ((AudioSampleFormat)audioParams.format) {
        case SAMPLE_U8:
            paSampleSpec.format = (pa_sample_format_t)PA_SAMPLE_U8;
            break;
        case SAMPLE_S16LE:
            paSampleSpec.format = (pa_sample_format_t)PA_SAMPLE_S16LE;
            break;
        case SAMPLE_S24LE:
            paSampleSpec.format = (pa_sample_format_t)PA_SAMPLE_S24LE;
            break;
        case SAMPLE_S32LE:
            paSampleSpec.format = (pa_sample_format_t)PA_SAMPLE_S32LE;
            break;
        default:
            paSampleSpec.format = (pa_sample_format_t)PA_SAMPLE_INVALID;
            break;
    }

    return paSampleSpec;
}

static size_t AlignToAudioFrameSize(size_t l, const pa_sample_spec &ss)
{
    size_t fs = pa_frame_size(&ss);
    CHECK_AND_RETURN_RET_LOG(fs != 0, 0, "Error: pa_frame_size returned 0");

    return (l / fs) * fs;
}

void AudioServiceClient::PAStreamStartSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");
    AudioServiceClient *asClient = static_cast<AudioServiceClient *>(userdata);
    pa_threaded_mainloop *mainLoop = static_cast<pa_threaded_mainloop *>(asClient->mainLoop);

    asClient->state_ = RUNNING;
    asClient->breakingWritePa_ = false;
    asClient->offloadTsLast_ = 0;
    std::shared_ptr<AudioStreamCallback> streamCb = asClient->streamCallback_.lock();
    if (streamCb != nullptr) {
        streamCb->OnStateChange(RUNNING, asClient->stateChangeCmdType_);
    }
    asClient->stateChangeCmdType_ = CMD_FROM_CLIENT;
    asClient->streamCmdStatus_ = success;
    pa_threaded_mainloop_signal(mainLoop, 0);
}

void AudioServiceClient::PAStreamStopSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    AUDIO_DEBUG_LOG("PAStreamStopSuccessCb in");
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");

    AudioServiceClient *asClient = static_cast<AudioServiceClient *>(userdata);
    pa_threaded_mainloop *mainLoop = static_cast<pa_threaded_mainloop *>(asClient->mainLoop);

    asClient->state_ = STOPPED;
    asClient->breakingWritePa_ = false;
    asClient->offloadTsLast_ = 0;
    std::shared_ptr<AudioStreamCallback> streamCb = asClient->streamCallback_.lock();
    if (streamCb != nullptr) {
        streamCb->OnStateChange(STOPPED);
    }
    asClient->streamCmdStatus_ = success;
    AUDIO_DEBUG_LOG("PAStreamStopSuccessCb: start signal");
    pa_threaded_mainloop_signal(mainLoop, 0);
    AUDIO_DEBUG_LOG("PAStreamStopSuccessCb out");
}

void AudioServiceClient::PAStreamAsyncStopSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    AUDIO_DEBUG_LOG("PAStreamAsyncStopSuccessCb in");
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");

    AudioServiceClient *asClient = static_cast<AudioServiceClient *>(userdata);
    pa_threaded_mainloop *mainLoop = static_cast<pa_threaded_mainloop *>(asClient->mainLoop);
    unique_lock<mutex> lockstopping(asClient->stoppingMutex_);

    if (asClient->offloadEnable_) {
        asClient->audioSystemManager_->OffloadDrain();
    }
    asClient->state_ = STOPPED;
    std::shared_ptr<AudioStreamCallback> streamCb = asClient->streamCallback_.lock();
    if (streamCb != nullptr) {
        streamCb->OnStateChange(STOPPED);
    }
    asClient->streamCmdStatus_ = success;
    AUDIO_DEBUG_LOG("PAStreamAsyncStopSuccessCb: start signal");
    lockstopping.unlock();
    pa_threaded_mainloop_signal(mainLoop, 0);
    asClient->dataCv_.notify_one();
    AUDIO_DEBUG_LOG("PAStreamAsyncStopSuccessCb out");
}

void AudioServiceClient::PAStreamPauseSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");

    AudioServiceClient *asClient = static_cast<AudioServiceClient *>(userdata);
    pa_threaded_mainloop *mainLoop = static_cast<pa_threaded_mainloop *>(asClient->mainLoop);

    asClient->state_ = PAUSED;
    asClient->breakingWritePa_ = false;
    asClient->offloadTsLast_ = 0;
    std::shared_ptr<AudioStreamCallback> streamCb = asClient->streamCallback_.lock();
    if (streamCb != nullptr) {
        streamCb->OnStateChange(PAUSED, asClient->stateChangeCmdType_);
    }
    asClient->stateChangeCmdType_ = CMD_FROM_CLIENT;
    asClient->streamCmdStatus_ = success;
    pa_threaded_mainloop_signal(mainLoop, 0);
}

void AudioServiceClient::PAStreamDrainSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");

    AudioServiceClient *asClient = (AudioServiceClient *)userdata;
    pa_threaded_mainloop *mainLoop = (pa_threaded_mainloop *)asClient->mainLoop;

    asClient->streamDrainStatus_ = success;
    pa_threaded_mainloop_signal(mainLoop, 0);
}

void AudioServiceClient::PAStreamDrainInStopCb(pa_stream *stream, int32_t success, void *userdata)
{
    AudioServiceClient *asClient = (AudioServiceClient *)userdata;
    pa_operation *operation = pa_stream_cork(asClient->paStream, 1,
        AudioServiceClient::PAStreamAsyncStopSuccessCb, userdata);
    pa_operation_unref(operation);

    asClient->streamDrainStatus_ = success;
}

void AudioServiceClient::PAStreamFlushSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");

    AudioServiceClient *asClient = (AudioServiceClient *)userdata;
    pa_threaded_mainloop *mainLoop = (pa_threaded_mainloop *)asClient->mainLoop;

    asClient->streamFlushStatus_ = success;
    pa_threaded_mainloop_signal(mainLoop, 0);
}

static size_t MsToAlignedSize(size_t ms, const pa_sample_spec &ss)
{
    return AlignToAudioFrameSize(pa_usec_to_bytes(ms * PA_USEC_PER_MSEC, &ss), ss);
}

void AudioServiceClient::PAStreamSetBufAttrSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");

    AudioServiceClient *asClient = (AudioServiceClient *)userdata;
    pa_threaded_mainloop *mainLoop = (pa_threaded_mainloop *)asClient->mainLoop;

    AUDIO_DEBUG_LOG("SetBufAttr %{public}s", success ? "success" : "faild");

    const pa_buffer_attr *bufferAttr = pa_stream_get_buffer_attr(stream);
    if (asClient->offloadEnable_ &&
        MsToAlignedSize(OFFLOAD_BUFFER, asClient->sampleSpec) >= bufferAttr->tlength / 2) { // 2 tlength need bigger
        asClient->acache_.totalCacheSizeTgt = MsToAlignedSize(OFFLOAD_BUFFER, asClient->sampleSpec);
    } else {
        asClient->acache_.totalCacheSizeTgt = bufferAttr->minreq;
    }
    pa_threaded_mainloop_signal(mainLoop, 0);
}

void AudioServiceClient::PAStreamUpdateTimingInfoSuccessCb(pa_stream *stream, int32_t success, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");

    AudioServiceClient *asClient = (AudioServiceClient *)userdata;
    pa_threaded_mainloop *mainLoop = (pa_threaded_mainloop *)asClient->mainLoop;
    int negative = 0;
    asClient->paLatency_ = 0;
    asClient->isGetLatencySuccess_ = true;
    if (pa_stream_get_latency(stream, &asClient->paLatency_, &negative) >= 0 && negative) {
        asClient->isGetLatencySuccess_ = false;
    }
    const pa_timing_info *info = pa_stream_get_timing_info(stream);
    asClient->offloadWriteIndex_ = pa_bytes_to_usec(info->write_index, &asClient->sampleSpec);
    asClient->offloadReadIndex_ = pa_bytes_to_usec(info->read_index, &asClient->sampleSpec);
    asClient->offloadTimeStamp_ = info->timestamp.tv_sec * AUDIO_US_PER_SECOND + info->timestamp.tv_usec;

    pa_threaded_mainloop_signal(mainLoop, 0);
}

int32_t AudioServiceClient::SetAudioRenderMode(AudioRenderMode renderMode)
{
    AUDIO_DEBUG_LOG("SetAudioRenderMode begin");
    renderMode_ = renderMode;

    CHECK_AND_RETURN_RET(renderMode_ == RENDER_MODE_CALLBACK, AUDIO_CLIENT_SUCCESS);

    int32_t ret = CheckReturnIfinvalid(mainLoop && context && paStream, AUDIO_CLIENT_ERR);
    CHECK_AND_RETURN_RET(ret >= 0, AUDIO_CLIENT_ERR);

    pa_threaded_mainloop_lock(mainLoop);
    pa_buffer_attr bufferAttr;
    bufferAttr.fragsize = static_cast<uint32_t>(-1);
    bufferAttr.prebuf = AlignToAudioFrameSize(pa_usec_to_bytes(MIN_BUF_DURATION_IN_USEC, &sampleSpec), sampleSpec);
    bufferAttr.maxlength = bufferAttr.prebuf * MAX_LENGTH_FACTOR;
    bufferAttr.tlength = bufferAttr.prebuf * T_LENGTH_FACTOR;
    bufferAttr.minreq = bufferAttr.prebuf;
    pa_operation *operation = pa_stream_set_buffer_attr(paStream, &bufferAttr,
        PAStreamSetBufAttrSuccessCb, (void *)this);
        while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mainLoop);
    }
    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(mainLoop);

    AUDIO_DEBUG_LOG("SetAudioRenderMode end");

    return AUDIO_CLIENT_SUCCESS;
}

AudioRenderMode AudioServiceClient::GetAudioRenderMode()
{
    return renderMode_;
}

int32_t AudioServiceClient::SetAudioCaptureMode(AudioCaptureMode captureMode)
{
    AUDIO_DEBUG_LOG("SetAudioCaptureMode.");
    captureMode_ = captureMode;

    return AUDIO_CLIENT_SUCCESS;
}

AudioCaptureMode AudioServiceClient::GetAudioCaptureMode()
{
    return captureMode_;
}

void AudioServiceClient::PAStreamWriteCb(pa_stream *stream, size_t length, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");

    auto asClient = static_cast<AudioServiceClient *>(userdata);
    Trace trace("AudioServiceClient::PAStreamWriteCb sink-input:" + std::to_string(asClient->streamId_) + " length:" +
        std::to_string(length));
    int64_t now = ClockTime::GetCurNano() / AUDIO_US_PER_SECOND;
    int64_t duration = now - asClient->writeCbStamp_;
    if (duration > 40) { // 40 ms
        AUDIO_INFO_LOG("Inside PA write callback cost[%{public}" PRId64 "]ms, sessionID %u", duration,
            asClient->sessionID_);
    }
    asClient->writeCbStamp_ = now;
    auto mainLoop = static_cast<pa_threaded_mainloop *>(asClient->mainLoop);
    pa_threaded_mainloop_signal(mainLoop, 0);
}

void AudioServiceClient::PAStreamReadCb(pa_stream *stream, size_t length, void *userdata)
{
    int32_t logMode = system::GetIntParameter("persist.multimedia.audiolog.switch", 0);
    if (logMode) {
        AUDIO_DEBUG_LOG("PAStreamReadCb Inside PA read callback");
    }
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");
    auto asClient = static_cast<AudioServiceClient *>(userdata);
    auto mainLoop = static_cast<pa_threaded_mainloop *>(asClient->mainLoop);
    pa_threaded_mainloop_signal(mainLoop, 0);
}

void AudioServiceClient::PAStreamUnderFlowCb(pa_stream *stream, void *userdata)
{
    Trace trace("PAStreamUnderFlow");
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");

    AudioServiceClient *asClient = (AudioServiceClient *)userdata;
    asClient->underFlowCount++;
    AUDIO_WARNING_LOG("AudioServiceClient underrun: %{public}d!", asClient->underFlowCount);
}

void AudioServiceClient::PAStreamEventCb(pa_stream *stream, const char *event, pa_proplist *pl, void *userdata)
{
    Trace trace("PAStreamEvent");
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");

    AudioServiceClient *asClient = (AudioServiceClient *)userdata;
    if (!strcmp(event, "signal_mainloop")) {
        pa_threaded_mainloop_signal(asClient->mainLoop, 0);
        AUDIO_DEBUG_LOG("receive event signal_mainloop");
    }

    if (!strcmp(event, "state_changed")) {
        const char *old_state = pa_proplist_gets(pl, "old_state");
        const char *new_state = pa_proplist_gets(pl, "new_state");
        AUDIO_INFO_LOG("old state : %{public}s, new state : %{public}s",
            old_state, new_state);
        if (asClient != nullptr) {
            if (!strcmp(old_state, "RUNNING") && !strcmp(new_state, "CORKED")) {
                asClient->UpdateStreamPosition(UpdatePositionTimeNode::CORKED_NODE);
            }
            if (!strcmp(old_state, "CORKED") && !strcmp(new_state, "RUNNING")) {
                asClient->UpdateStreamPosition(UpdatePositionTimeNode::RUNNING_NODE);
            }
        }
    }
}

void AudioServiceClient::PAStreamLatencyUpdateCb(pa_stream *stream, void *userdata)
{
    pa_threaded_mainloop *mainLoop = (pa_threaded_mainloop *)userdata;
    pa_threaded_mainloop_signal(mainLoop, 0);
}

void AudioServiceClient::PAStreamMovedCb(pa_stream *stream, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");
    CHECK_AND_RETURN_LOG(stream, "stream is null");

    // get stream informations.
    uint32_t deviceIndex = pa_stream_get_device_index(stream); // pa_context_get_sink_info_by_index

    // Return 1 if the sink or source this stream is connected to has been suspended.
    // This will return 0 if not, and a negative value on error.
    int res = pa_stream_is_suspended(stream);
    AUDIO_DEBUG_LOG("PAstream moved to index:[%{public}d] suspended:[%{public}d]",
        deviceIndex, res);
}

void AudioServiceClient::PAStreamStateCb(pa_stream *stream, void *userdata)
{
    CHECK_AND_RETURN_LOG(userdata, "userdata is null");
    CHECK_AND_RETURN_LOG(stream, "stream is null");

    AudioServiceClient *asClient = (AudioServiceClient *)userdata;
    bool isClientExist;
    if (serviceClientInstanceMap_.Find(asClient, isClientExist) == false) {
        AUDIO_ERR_LOG("asClient is null");
        return;
    }
    std::lock_guard<std::mutex> lock(asClient->serviceClientLock_);
    pa_threaded_mainloop *mainLoop = (pa_threaded_mainloop *)asClient->mainLoop;

    if (asClient->mAudioRendererCallbacks)
        asClient->mAudioRendererCallbacks->OnStreamStateChangeCb();

    AUDIO_INFO_LOG("Current Stream State: %{public}d", pa_stream_get_state(stream));

    switch (pa_stream_get_state(stream)) {
        case PA_STREAM_READY:
        case PA_STREAM_FAILED:
        case PA_STREAM_TERMINATED:
            pa_threaded_mainloop_signal(mainLoop, 0);
            break;

        case PA_STREAM_UNCONNECTED:
        case PA_STREAM_CREATING:
        default:
            break;
    }
}

void AudioServiceClient::PAContextStateCb(pa_context *context, void *userdata)
{
    pa_threaded_mainloop *mainLoop = (pa_threaded_mainloop *)userdata;
    AUDIO_INFO_LOG("Current Context State: %{public}d", pa_context_get_state(context));

    switch (pa_context_get_state(context)) {
        case PA_CONTEXT_READY:
            AudioSystemManager::GetInstance()->RequestThreadPriority(gettid());
            pa_threaded_mainloop_signal(mainLoop, 0);
            break;
        case PA_CONTEXT_TERMINATED:
        case PA_CONTEXT_FAILED:
            pa_threaded_mainloop_signal(mainLoop, 0);
            break;

        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
        default:
            break;
    }
}

AudioServiceClient::AudioServiceClient()
    : AppExecFwk::EventHandler(AppExecFwk::EventRunner::Create("OS_ACRunner"))
{
    sinkDevices.clear();
    sourceDevices.clear();
    sinkInputs.clear();
    sourceOutputs.clear();
    clientInfo.clear();

    renderRate = RENDER_RATE_NORMAL;
    renderMode_ = RENDER_MODE_NORMAL;

    captureMode_ = CAPTURE_MODE_NORMAL;

    eAudioClientType = AUDIO_SERVICE_CLIENT_PLAYBACK;
    effectSceneName = "SCENE_MUSIC";
    effectMode = EFFECT_DEFAULT;

    mFrameSize = 0;
    mFrameMarkPosition = 0;
    mMarkReached = false;
    mFramePeriodNumber = 0;

    mTotalBytesWritten = 0;
    mFramePeriodWritten = 0;
    mTotalBytesRead = 0;
    mFramePeriodRead = 0;
    mRenderPositionCb = nullptr;
    mRenderPeriodPositionCb = nullptr;

    mAudioRendererCallbacks = nullptr;
    mAudioCapturerCallbacks = nullptr;
    internalReadBuffer_ = nullptr;
    mainLoop = nullptr;
    paStream = nullptr;
    context  = nullptr;
    api = nullptr;

    internalRdBufIndex_ = 0;
    internalRdBufLen_ = 0;
    streamCmdStatus_ = 0;
    streamDrainStatus_ = 0;
    streamFlushStatus_ = 0;
    underFlowCount = 0;

    acache_.readIndex = 0;
    acache_.writeIndex = 0;
    acache_.isFull = false;
    acache_.totalCacheSize = 0;
    acache_.totalCacheSizeTgt = 0;
    acache_.buffer = nullptr;

    setBufferSize_ = 0;
    PAStreamCorkSuccessCb = PAStreamStopSuccessCb;
    rendererSampleRate = 0;

    mPrivacyType = PRIVACY_TYPE_PUBLIC;
    mStreamUsage = STREAM_USAGE_UNKNOWN;
    streamClass_ = IAudioStream::StreamClass::PA_STREAM;
    serviceClientInstanceMap_.Insert(this, true);
}

void AudioServiceClient::ResetPAAudioClient()
{
    AUDIO_INFO_LOG("In");
    lock_guard<mutex> lock(ctrlMutex_);
    if (mainLoop && (isMainLoopStarted_ == true))
        pa_threaded_mainloop_stop(mainLoop);

    if (paStream) {
        pa_stream_set_state_callback(paStream, nullptr, nullptr);
        pa_stream_set_write_callback(paStream, nullptr, nullptr);
        pa_stream_set_read_callback(paStream, nullptr, nullptr);
        pa_stream_set_latency_update_callback(paStream, nullptr, nullptr);
        pa_stream_set_underflow_callback(paStream, nullptr, nullptr);

        if (isStreamConnected_ == true) {
            pa_stream_disconnect(paStream);
            pa_stream_unref(paStream);
            isStreamConnected_  = false;
            paStream = nullptr;
        }
    }

    if (context) {
        pa_context_set_state_callback(context, nullptr, nullptr);
        if (isContextConnected_ == true) {
            pa_threaded_mainloop_lock(mainLoop);
            pa_context_disconnect(context);
            pa_context_unref(context);
            pa_threaded_mainloop_unlock(mainLoop);
            isContextConnected_ = false;
            context = nullptr;
        }
    }

    if (mainLoop) {
        pa_threaded_mainloop_free(mainLoop);
        isMainLoopStarted_  = false;
        mainLoop = nullptr;
    }

    for (auto &thread : mPositionCBThreads) {
        if (thread && thread->joinable()) {
            thread->join();
        }
    }

    for (auto &thread : mPeriodPositionCBThreads) {
        if (thread && thread->joinable()) {
            thread->join();
        }
    }

    if (appCookiePath.compare("")) {
        remove(appCookiePath.c_str());
        appCookiePath = "";
    }

    sinkDevices.clear();
    sourceDevices.clear();
    sinkInputs.clear();
    sourceOutputs.clear();
    clientInfo.clear();

    mAudioRendererCallbacks = nullptr;
    mAudioCapturerCallbacks = nullptr;
    internalReadBuffer_      = nullptr;

    api      = nullptr;

    internalRdBufIndex_ = 0;
    internalRdBufLen_   = 0;
    underFlowCount     = 0;

    acache_.buffer = nullptr;
    acache_.readIndex = 0;
    acache_.writeIndex = 0;
    acache_.isFull = false;
    acache_.totalCacheSize = 0;
    acache_.totalCacheSizeTgt = 0;

    setBufferSize_ = 0;
    PAStreamCorkSuccessCb = nullptr;
}

AudioServiceClient::~AudioServiceClient()
{
    lock_guard<mutex> lockdata(dataMutex_);
    AUDIO_INFO_LOG("start ~AudioServiceClient");
    UnregisterSpatializationStateEventListener(spatializationRegisteredSessionID_);
    ResetPAAudioClient();
    StopTimer();
    std::lock_guard<std::mutex> lock(serviceClientLock_);
    serviceClientInstanceMap_.Erase(this);
}

void AudioServiceClient::SetEnv()
{
    AUDIO_DEBUG_LOG("SetEnv called");

    int ret = setenv("HOME", PA_HOME_DIR, 1);
    AUDIO_DEBUG_LOG("set env HOME: %{public}d", ret);

    ret = setenv("PULSE_RUNTIME_PATH", PA_RUNTIME_DIR, 1);
    AUDIO_DEBUG_LOG("set env PULSE_RUNTIME_DIR: %{public}d", ret);

    ret = setenv("PULSE_STATE_PATH", PA_STATE_DIR, 1);
    AUDIO_DEBUG_LOG("set env PULSE_STATE_PATH: %{public}d", ret);
}

void AudioServiceClient::SetApplicationCachePath(const std::string cachePath)
{
    AUDIO_DEBUG_LOG("SetApplicationCachePath in");

    char realPath[PATH_MAX] = {0x00};

    CHECK_AND_RETURN_LOG((strlen(cachePath.c_str()) < PATH_MAX) && (realpath(cachePath.c_str(), realPath) != nullptr),
        "Invalid cache path. err = %{public}d", errno);

    cachePath_ = realPath;
}

bool AudioServiceClient::CheckRecordingCreate(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
    SourceType sourceType)
{
    return AudioPolicyManager::GetInstance().CheckRecordingCreate(appTokenId, appFullTokenId, appUid, sourceType);
}

bool AudioServiceClient::CheckRecordingStateChange(uint32_t appTokenId, uint64_t appFullTokenId, int32_t appUid,
    AudioPermissionState state)
{
    return AudioPolicyManager::GetInstance().CheckRecordingStateChange(appTokenId, appFullTokenId, appUid, state);
}

int32_t AudioServiceClient::Initialize(ASClientType eClientType)
{
    int error = PA_ERR_INTERNAL;
    eAudioClientType = eClientType;

    mMarkReached = false;
    mTotalBytesWritten = 0;
    mFramePeriodWritten = 0;
    mTotalBytesRead = 0;
    mFramePeriodRead = 0;

    SetEnv();

    audioSystemManager_ = AudioSystemManager::GetInstance();
    lock_guard<mutex> lockdata(dataMutex_);
    mainLoop = pa_threaded_mainloop_new();
    if (mainLoop == nullptr)
        return AUDIO_CLIENT_INIT_ERR;
    api = pa_threaded_mainloop_get_api(mainLoop);
    pa_threaded_mainloop_set_name(mainLoop, "OS_AudioML");
    if (api == nullptr) {
        ResetPAAudioClient();
        return AUDIO_CLIENT_INIT_ERR;
    }
    stringstream ss;
    string packageName = "";
    ss << "app-pid<" << getpid() << ">-uid<" << getuid() << ">";
    ss >> packageName;
    AUDIO_DEBUG_LOG("AudioServiceClient:Initialize [%{public}s]", packageName.c_str());
    context = pa_context_new(api, packageName.c_str());
    if (context == nullptr) {
        ResetPAAudioClient();
        return AUDIO_CLIENT_INIT_ERR;
    }

    pa_context_set_state_callback(context, PAContextStateCb, mainLoop);

    if (pa_context_connect(context, nullptr, PA_CONTEXT_NOFAIL, nullptr) < 0) {
        error = pa_context_errno(context);
        AUDIO_ERR_LOG("context connect error: %{public}s", pa_strerror(error));
        ResetPAAudioClient();
        return AUDIO_CLIENT_INIT_ERR;
    }
    isContextConnected_ = true;
    CHECK_AND_RETURN_RET_LOG(HandleMainLoopStart() == AUDIO_CLIENT_SUCCESS, AUDIO_CLIENT_INIT_ERR,
        "Start main loop failed");

    if (appCookiePath.compare("")) {
        remove(appCookiePath.c_str());
        appCookiePath = "";
    }

    pa_threaded_mainloop_unlock(mainLoop);
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::HandleMainLoopStart()
{
    int error = PA_ERR_INTERNAL;
    pa_threaded_mainloop_lock(mainLoop);

    if (pa_threaded_mainloop_start(mainLoop) < 0) {
        pa_threaded_mainloop_unlock(mainLoop);
        ResetPAAudioClient();
        return AUDIO_CLIENT_INIT_ERR;
    }

    isMainLoopStarted_ = true;
    while (true) {
        pa_context_state_t state = pa_context_get_state(context);
        if (state == PA_CONTEXT_READY)
            break;

        if (!PA_CONTEXT_IS_GOOD(state)) {
            error = pa_context_errno(context);
            AUDIO_ERR_LOG("context bad state error: %{public}s", pa_strerror(error));
            pa_threaded_mainloop_unlock(mainLoop);
            ResetPAAudioClient();
            return AUDIO_CLIENT_INIT_ERR;
        }

        static bool triggerDumpStacktraceAndKill = true;
        if (triggerDumpStacktraceAndKill == true) {
            AudioXCollie audioXCollie("AudioServiceClient::InitDumpTrace", INIT_TIMEOUT_IN_SEC, [this](void *) {
                audioSystemManager_->GetAudioParameter(RECOVERY_AUDIO_SERVER);
                triggerDumpStacktraceAndKill = false;
                pa_threaded_mainloop_signal(this->mainLoop, 0);
            }, nullptr, 0);
            pa_threaded_mainloop_wait(mainLoop);
        } else {
            StartTimer(INIT_TIMEOUT_IN_SEC);
            pa_threaded_mainloop_wait(mainLoop);
            StopTimer();
            if (IsTimeOut()) {
                AUDIO_ERR_LOG("Initialize timeout");
                pa_threaded_mainloop_unlock(mainLoop);
                return AUDIO_CLIENT_INIT_ERR;
            }
        }
    }
    return AUDIO_CLIENT_SUCCESS;
}

const std::string AudioServiceClient::GetStreamName(AudioStreamType audioType)
{
    std::string name = "unknown";
    if (STREAM_TYPE_ENUM_STRING_MAP.find(audioType) != STREAM_TYPE_ENUM_STRING_MAP.end()) {
        name = STREAM_TYPE_ENUM_STRING_MAP.at(audioType);
    } else {
        AUDIO_WARNING_LOG("GetStreamName: Invalid stream type [%{public}d], return unknown", audioType);
    }
    const std::string streamName = name;
    return streamName;
}

std::pair<const int32_t, const std::string> AudioServiceClient::GetDeviceNameForConnect()
{
    string deviceName;
    if (eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK) {
        const std::string selectDevice =  AudioSystemManager::GetInstance()->GetSelectedDeviceInfo(clientUid_,
            clientPid_, streamType_);
        deviceName = (selectDevice.empty() ? "" : selectDevice);
        return {AUDIO_CLIENT_SUCCESS, deviceName};
    }

    if (eAudioClientType == AUDIO_SERVICE_CLIENT_RECORD) {
        if (isInnerCapturerStream_) {
            return {AUDIO_CLIENT_SUCCESS, INNER_CAPTURER_SOURCE};
        }

        if (isWakeupCapturerStream_) {
            AUDIO_ERR_LOG("non-IPC channels will no longer support voice wakeup");
            return {AUDIO_CLIENT_ERR, deviceName};
        }
        NotifyCapturerAdded(sessionID_);
    }
    return {AUDIO_CLIENT_SUCCESS, deviceName};
}

int32_t AudioServiceClient::ConnectStreamToPA()
{
    AUDIO_DEBUG_LOG("AudioServiceClient::ConnectStreamToPA");

    int32_t ret = CheckReturnIfinvalid(mainLoop && context && paStream, AUDIO_CLIENT_ERR);
    CHECK_AND_RETURN_RET(ret >= 0, AUDIO_CLIENT_ERR);
    int32_t latency_in_msec = AudioSystemManager::GetInstance()->GetAudioLatencyFromXml();
    CHECK_AND_RETURN_RET_LOG(latency_in_msec >= 0, AUDIO_CLIENT_CREATE_STREAM_ERR,
        "Get audio latency failed.");
    sinkLatencyInMsec_ = AudioSystemManager::GetInstance()->GetSinkLatencyFromXml();

    auto [errorCode, deviceNameS] = GetDeviceNameForConnect();
    CHECK_AND_RETURN_RET(errorCode == AUDIO_CLIENT_SUCCESS, errorCode);

    pa_threaded_mainloop_lock(mainLoop);

    if (HandlePAStreamConnect(deviceNameS, latency_in_msec) != SUCCESS || WaitStreamReady() != SUCCESS) {
        pa_threaded_mainloop_unlock(mainLoop);
        return AUDIO_CLIENT_CREATE_STREAM_ERR;
    }

    isStreamConnected_ = true;
    streamId_ = pa_stream_get_index(paStream);
    pa_threaded_mainloop_unlock(mainLoop);
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::HandlePAStreamConnect(const std::string &deviceNameS, int32_t latencyInMSec)
{
    const char *deviceName = deviceNameS.empty() ? nullptr : deviceNameS.c_str();
    pa_buffer_attr bufferAttr;
    bufferAttr.fragsize = static_cast<uint32_t>(-1);
    if (latencyInMSec <= LATENCY_THRESHOLD) {
        bufferAttr.prebuf = AlignToAudioFrameSize(pa_usec_to_bytes(latencyInMSec * PA_USEC_PER_MSEC, &sampleSpec),
            sampleSpec);
        bufferAttr.maxlength =  NO_OF_PREBUF_TIMES * bufferAttr.prebuf;
        bufferAttr.tlength = static_cast<uint32_t>(-1);
    } else {
        bufferAttr.prebuf = pa_usec_to_bytes(latencyInMSec * PA_USEC_PER_MSEC, &sampleSpec);
        bufferAttr.maxlength = pa_usec_to_bytes(latencyInMSec * PA_USEC_PER_MSEC * MAX_LENGTH_FACTOR, &sampleSpec);
        bufferAttr.tlength = pa_usec_to_bytes(latencyInMSec * PA_USEC_PER_MSEC * T_LENGTH_FACTOR, &sampleSpec);
    }
    bufferAttr.minreq = bufferAttr.prebuf;
    int32_t result = 0;
    if (eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK) {
        result = pa_stream_connect_playback(paStream, deviceName, &bufferAttr,
            (pa_stream_flags_t)(PA_STREAM_ADJUST_LATENCY | PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_START_CORKED |
            PA_STREAM_VARIABLE_RATE), nullptr, nullptr);
        preBuf_ = make_unique<uint8_t[]>(bufferAttr.maxlength);
        if (preBuf_ == nullptr) {
            AUDIO_ERR_LOG("Allocate memory for buffer failed.");
            return AUDIO_CLIENT_INIT_ERR;
        }
        if (memset_s(preBuf_.get(), bufferAttr.maxlength, 0, bufferAttr.maxlength) != 0) {
            AUDIO_ERR_LOG("memset_s for buffer failed.");
            return AUDIO_CLIENT_INIT_ERR;
        }
    } else {
        AUDIO_DEBUG_LOG("pa_stream_connect_record connect to:%{public}s", deviceName ? deviceName : "nullptr");
        result = pa_stream_connect_record(paStream, deviceName, nullptr,
            (pa_stream_flags_t)(PA_STREAM_INTERPOLATE_TIMING | PA_STREAM_ADJUST_LATENCY
            | PA_STREAM_START_CORKED | PA_STREAM_AUTO_TIMING_UPDATE));
    }
    if (result < 0) {
        int error = pa_context_errno(context);
        AUDIO_ERR_LOG("connection to stream error: %{public}d", error);
        ResetPAAudioClient();
        return AUDIO_CLIENT_CREATE_STREAM_ERR;
    }
    return SUCCESS;
}

int32_t AudioServiceClient::WaitStreamReady()
{
    while (true) {
        pa_stream_state_t state = pa_stream_get_state(paStream);
        if (state == PA_STREAM_READY)
            break;

        if (!PA_STREAM_IS_GOOD(state)) {
            int error = pa_context_errno(context);
            AUDIO_ERR_LOG("connection to stream error: %{public}d", error);
            ResetPAAudioClient();
            return AUDIO_CLIENT_CREATE_STREAM_ERR;
        }

        static bool recoveryAudioServer = true;
        if (recoveryAudioServer == true) {
            AudioXCollie audioXCollie("AudioServiceClient::RecoveryConnect", CONNECT_TIMEOUT_IN_SEC, [this](void *) {
                audioSystemManager_->GetAudioParameter(RECOVERY_AUDIO_SERVER);
                recoveryAudioServer = false;
                pa_threaded_mainloop_signal(this->mainLoop, 0);
            }, nullptr, 0);
            pa_threaded_mainloop_wait(mainLoop);
        } else {
            StartTimer(CONNECT_TIMEOUT_IN_SEC);
            pa_threaded_mainloop_wait(mainLoop);
            StopTimer();
            if (IsTimeOut()) {
                AUDIO_ERR_LOG("Initialize timeout");
                return AUDIO_CLIENT_CREATE_STREAM_ERR;
            }
        }
    }
    return SUCCESS;
}

int32_t AudioServiceClient::InitializeAudioCache()
{
    AUDIO_DEBUG_LOG("Initializing internal audio cache");

    int32_t ret = CheckReturnIfinvalid(mainLoop && context && paStream, AUDIO_CLIENT_PA_ERR);
    CHECK_AND_RETURN_RET(ret >= 0, AUDIO_CLIENT_PA_ERR);

    const pa_buffer_attr *bufferAttr = pa_stream_get_buffer_attr(paStream);
    CHECK_AND_RETURN_RET_LOG(bufferAttr != nullptr, AUDIO_CLIENT_INIT_ERR,
        "pa stream get buffer attribute returned null");

    acache_.buffer = make_unique<uint8_t[]>(max((uint32_t)bufferAttr->minreq,
        static_cast<uint32_t>(pa_usec_to_bytes(200 * PA_USEC_PER_MSEC, &sampleSpec)))); // 200 is init size

    CHECK_AND_RETURN_RET_LOG(acache_.buffer != nullptr, AUDIO_CLIENT_INIT_ERR,
        "Allocate memory for buffer failed");

    acache_.readIndex = 0;
    acache_.writeIndex = 0;
    acache_.totalCacheSize = bufferAttr->minreq;
    acache_.totalCacheSizeTgt = bufferAttr->minreq;
    acache_.isFull = false;
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::SetPaProplist(pa_proplist *propList, pa_channel_map &map,
    AudioStreamParams &audioParams, const std::string &streamName, const std::string &streamStartTime)
{
    if (propList == nullptr) {
        AUDIO_ERR_LOG("pa_proplist_new failed");
        pa_threaded_mainloop_unlock(mainLoop);
        ResetPAAudioClient();
        return AUDIO_CLIENT_CREATE_STREAM_ERR;
    }

    // for remote audio device router filter.
    pa_proplist_sets(propList, "stream.client.uid", std::to_string(clientUid_).c_str());
    pa_proplist_sets(propList, "stream.client.pid", std::to_string(clientPid_).c_str());

    pa_proplist_sets(propList, "stream.type", streamName.c_str());
    pa_proplist_sets(propList, "stream.volumeFactor", std::to_string(volumeFactor_).c_str());
    pa_proplist_sets(propList, "stream.powerVolumeFactor", std::to_string(powerVolumeFactor_).c_str());
    sessionID_ = pa_context_get_index(context);
    pa_proplist_sets(propList, "stream.sessionID", std::to_string(sessionID_).c_str());
    pa_proplist_sets(propList, "stream.startTime", streamStartTime.c_str());

    if (eAudioClientType == AUDIO_SERVICE_CLIENT_RECORD) {
        pa_proplist_sets(propList, "stream.isInnerCapturer", std::to_string(isInnerCapturerStream_).c_str());
        pa_proplist_sets(propList, "stream.isWakeupCapturer", std::to_string(isWakeupCapturerStream_).c_str());
        pa_proplist_sets(propList, "stream.isIpcCapturer", std::to_string(false).c_str());
        pa_proplist_sets(propList, "stream.capturerSource", std::to_string(capturerSource_).c_str());
    } else if (eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK) {
        pa_proplist_sets(propList, "stream.privacyType", std::to_string(mPrivacyType).c_str());
        pa_proplist_sets(propList, "stream.usage", std::to_string(mStreamUsage).c_str());
    }

    AUDIO_DEBUG_LOG("Creating stream of channels %{public}d", audioParams.channels);
    if (audioParams.channelLayout == 0) {
        audioParams.channelLayout = defaultChCountToLayoutMap[audioParams.channels];
    }
    pa_proplist_sets(propList, "stream.channelLayout", std::to_string(audioParams.channelLayout).c_str());
    pa_proplist_sets(propList, "spatialization.enabled", spatializationEnabled_.c_str());
    pa_proplist_sets(propList, "headtracking.enabled", headTrackingEnabled_.c_str());
    pa_channel_map_init(&map);
    map.channels = audioParams.channels;
    uint32_t channelsInLayout = ConvertChLayoutToPaChMap(audioParams.channelLayout, map);
    if (channelsInLayout != audioParams.channels || !channelsInLayout) {
        AUDIO_ERR_LOG("Invalid channel Layout");
        pa_proplist_free(propList);
        pa_threaded_mainloop_unlock(mainLoop);
        ResetPAAudioClient();
        return AUDIO_CLIENT_CREATE_STREAM_ERR;
    }

    ResetOffload();
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::CreateStreamWithPa(AudioStreamParams audioParams, AudioStreamType audioType)
{
    int error;
    pa_threaded_mainloop_lock(mainLoop);
    streamType_ = audioType;
    const std::string streamName = GetStreamName(audioType);

    auto timenow = chrono::system_clock::to_time_t(chrono::system_clock::now());
    const std::string streamStartTime = ctime(&timenow);

    sampleSpec = ConvertToPAAudioParams(audioParams);
    mFrameSize = pa_frame_size(&sampleSpec);
    channelLayout_ = audioParams.channelLayout;
    AudioSpatializationState spatializationState =
        AudioPolicyManager::GetInstance().GetSpatializationState(mStreamUsage);
    spatializationEnabled_ = std::to_string(spatializationState.spatializationEnabled);
    headTrackingEnabled_ = std::to_string(spatializationState.headTrackingEnabled);
    if (mStreamUsage == STREAM_USAGE_SYSTEM || mStreamUsage == STREAM_USAGE_DTMF ||
        mStreamUsage == STREAM_USAGE_ENFORCED_TONE || mStreamUsage == STREAM_USAGE_ULTRASONIC ||
        mStreamUsage == STREAM_USAGE_NAVIGATION || mStreamUsage == STREAM_USAGE_NOTIFICATION) {
        effectMode = EFFECT_NONE;
    }

    pa_proplist *propList = pa_proplist_new();
    pa_channel_map map;
    int32_t res = SetPaProplist(propList, map, audioParams, streamName, streamStartTime);
    CHECK_AND_RETURN_RET(res == AUDIO_CLIENT_SUCCESS, res);

    paStream = pa_stream_new_with_proplist(context, streamName.c_str(), &sampleSpec, &map, propList);
    if (!paStream) {
        error = pa_context_errno(context);
        AUDIO_ERR_LOG("create stream Failed, error: %{public}d", error);
        pa_proplist_free(propList);
        pa_threaded_mainloop_unlock(mainLoop);
        ResetPAAudioClient();
        AUDIO_ERR_LOG("pa_stream_new_with_proplist failed, error: %{public}d", error);
        return AUDIO_CLIENT_CREATE_STREAM_ERR;
    }

    pa_proplist_free(propList);
    pa_stream_set_state_callback(paStream, PAStreamStateCb, (void *)this);
    pa_stream_set_moved_callback(paStream, PAStreamMovedCb, (void *)this); // used to notify sink/source moved
    pa_stream_set_write_callback(paStream, PAStreamWriteCb, (void *)this);
    pa_stream_set_read_callback(paStream, PAStreamReadCb, (void *)this);
    pa_stream_set_latency_update_callback(paStream, PAStreamLatencyUpdateCb, mainLoop);
    pa_stream_set_underflow_callback(paStream, PAStreamUnderFlowCb, (void *)this);
    pa_stream_set_event_callback(paStream, PAStreamEventCb, (void *)this);

    pa_threaded_mainloop_unlock(mainLoop);
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::CreateStream(AudioStreamParams audioParams, AudioStreamType audioType)
{
    AUDIO_INFO_LOG("AudioServiceClient::CreateStream");
    int error;
    lock_guard<mutex> lockdata(dataMutex_);
    int32_t ret = CheckReturnIfinvalid(mainLoop && context, AUDIO_CLIENT_ERR);
    CHECK_AND_RETURN_RET(ret >= 0, AUDIO_CLIENT_ERR);

    CHECK_AND_RETURN_RET(eAudioClientType != AUDIO_SERVICE_CLIENT_CONTROLLER, AUDIO_CLIENT_INVALID_PARAMS_ERR);

    error = CreateStreamWithPa(audioParams, audioType);
    if (error < 0) {
        AUDIO_ERR_LOG("Create Stream With Pa Failed");
        return AUDIO_CLIENT_CREATE_STREAM_ERR;
    }
    error = ConnectStreamToPA();
    streamInfoUpdated_ = false;
    if (error < 0) {
        AUDIO_ERR_LOG("Create Stream Failed");
        ResetPAAudioClient();
        return AUDIO_CLIENT_CREATE_STREAM_ERR;
    }
    RegisterSpatializationStateEventListener();

    if (eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK) {
        error = InitializeAudioCache();
        if (error < 0) {
            AUDIO_ERR_LOG("Initialize audio cache failed");
            ResetPAAudioClient();
            return AUDIO_CLIENT_CREATE_STREAM_ERR;
        }

        if (SetStreamRenderRate(renderRate) != AUDIO_CLIENT_SUCCESS) {
            AUDIO_ERR_LOG("Set render rate failed");
        }

        effectSceneName = IAudioStream::GetEffectSceneName(audioType);
        if (SetStreamAudioEffectMode(effectMode) != AUDIO_CLIENT_SUCCESS) {
            AUDIO_ERR_LOG("Set audio effect mode failed");
        }
    }

    state_ = PREPARED;
    std::shared_ptr<AudioStreamCallback> streamCb = streamCallback_.lock();
    if (streamCb != nullptr) {
        streamCb->OnStateChange(PREPARED);
    }
    return AUDIO_CLIENT_SUCCESS;
}

uint32_t AudioServiceClient::GetUnderflowCount()
{
    return underFlowCount;
}

int32_t AudioServiceClient::GetSessionID(uint32_t &sessionID) const
{
    AUDIO_DEBUG_LOG("GetSessionID sessionID: %{public}d", sessionID_);
    CHECK_AND_RETURN_RET(sessionID_ != PA_INVALID_INDEX && sessionID_ != 0, AUDIO_CLIENT_ERR);
    sessionID = sessionID_;
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::StartStream(StateChangeCmdType cmdType)
{
    AUDIO_INFO_LOG("AudioServiceClient::StartStream");
    writeCbStamp_ = ClockTime::GetCurNano() / AUDIO_US_PER_SECOND;
    Trace trace("AudioServiceClient::StartStream " + std::to_string(sessionID_));
    int error;
    lock_guard<mutex> lockdata(dataMutex_);
    unique_lock<mutex> stoppinglock(stoppingMutex_);

    // wait 1 second, timeout return error
    CHECK_AND_RETURN_RET_LOG(dataCv_.wait_for(stoppinglock, 1s, [this] {return state_ != STOPPING;}),
        AUDIO_CLIENT_START_STREAM_ERR, "StartStream: wait stopping timeout");
    stoppinglock.unlock();
    int32_t ret = CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR);
    CHECK_AND_RETURN_RET(ret >= 0, AUDIO_CLIENT_PA_ERR);
    pa_operation *operation = nullptr;

    pa_threaded_mainloop_lock(mainLoop);

    pa_stream_state_t state = pa_stream_get_state(paStream);
    if (state != PA_STREAM_READY) {
        error = pa_context_errno(context);
        pa_threaded_mainloop_unlock(mainLoop);
        AUDIO_ERR_LOG("Stream Start Failed, error: %{public}d", error);
        ResetPAAudioClient();
        return AUDIO_CLIENT_START_STREAM_ERR;
    }

    hasFirstFrameWrited_ = false;

    streamCmdStatus_ = 0;
    stateChangeCmdType_ = cmdType;
    operation = pa_stream_cork(paStream, 0, PAStreamStartSuccessCb, (void *)this);

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mainLoop);
    }
    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(mainLoop);

    if (!streamCmdStatus_) {
        AUDIO_ERR_LOG("Stream Start Failed");
        ResetPAAudioClient();
        return AUDIO_CLIENT_START_STREAM_ERR;
    } else {
        return AUDIO_CLIENT_SUCCESS;
    }
}

int32_t AudioServiceClient::PauseStream(StateChangeCmdType cmdType)
{
    AUDIO_INFO_LOG("AudioServiceClient::PauseStream");
    CheckOffloadBreakWaitWrite();
    lock_guard<mutex> lockdata(dataMutex_);
    lock_guard<mutex> lockctrl(ctrlMutex_);
    PAStreamCorkSuccessCb = PAStreamPauseSuccessCb;
    stateChangeCmdType_ = cmdType;

    int32_t ret = CorkStream();
    breakingWritePa_ = false;
    if (ret) {
        return ret;
    }

    if (!streamCmdStatus_) {
        AUDIO_ERR_LOG("Stream Pause Failed");
        return AUDIO_CLIENT_ERR;
    } else {
        return AUDIO_CLIENT_SUCCESS;
    }
}

int32_t AudioServiceClient::StopStreamPlayback()
{
    if (state_ != PAUSED) {
        state_ = STOPPING;
        DrainAudioCache();

        int32_t ret = CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR);
        CHECK_AND_RETURN_RET(ret >= 0, AUDIO_CLIENT_PA_ERR);

        pa_threaded_mainloop_lock(mainLoop);

        streamDrainStatus_ = 0;
        pa_operation *operation = pa_stream_drain(paStream, PAStreamDrainInStopCb, (void *)this);

        if (operation == nullptr) {
            pa_threaded_mainloop_unlock(mainLoop);
            AUDIO_ERR_LOG("pa_stream_drain operation is null");
            return AUDIO_CLIENT_ERR;
        }

        pa_operation_unref(operation);
        pa_threaded_mainloop_unlock(mainLoop);
    } else {
        state_ = STOPPED;
        std::shared_ptr<AudioStreamCallback> streamCb = streamCallback_.lock();
        if (streamCb != nullptr) {
            streamCb->OnStateChange(STOPPED);
        }
    }
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::StopStream()
{
    AUDIO_INFO_LOG("AudioServiceClient::StopStream");
    if (eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK && offloadEnable_) {
        return OffloadStopStream();
    }
    lock_guard<mutex> lockdata(dataMutex_);
    lock_guard<mutex> lockctrl(ctrlMutex_);
    if (eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK) {
        return StopStreamPlayback();
    } else {
        PAStreamCorkSuccessCb = PAStreamStopSuccessCb;
        int32_t ret = CorkStream();
        if (ret) {
            return ret;
        }

        if (!streamCmdStatus_) {
            AUDIO_ERR_LOG("Stream Stop Failed");
            return AUDIO_CLIENT_ERR;
        } else {
            if (internalRdBufLen_) {
                (void)pa_stream_drop(paStream);
                internalReadBuffer_ = nullptr;
                internalRdBufLen_ = 0;
                internalRdBufIndex_ = 0;
            }
            return AUDIO_CLIENT_SUCCESS;
        }
    }
}

int32_t AudioServiceClient::OffloadStopStream()
{
    AUDIO_INFO_LOG("AudioServiceClient::OffloadStopStream");
    CheckOffloadBreakWaitWrite();
    lock_guard<mutex> lockdata(dataMutex_);
    lock_guard<mutex> lockctrl(ctrlMutex_);
    state_ = STOPPING;
    DrainAudioCache();

    int32_t res = CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR);
    CHECK_AND_RETURN_RET(res >= 0, AUDIO_CLIENT_PA_ERR);

    PAStreamCorkSuccessCb = PAStreamStopSuccessCb;
    int32_t ret = CorkStream();
    breakingWritePa_ = false;
    if (ret) {
        return ret;
    }
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::CorkStream()
{
    int32_t res = CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR);
    CHECK_AND_RETURN_RET(res >= 0, AUDIO_CLIENT_PA_ERR);

    pa_operation *operation = nullptr;

    pa_threaded_mainloop_lock(mainLoop);
    pa_stream_state_t state = pa_stream_get_state(paStream);
    if (state != PA_STREAM_READY) {
        int32_t error = pa_context_errno(context);
        pa_threaded_mainloop_unlock(mainLoop);
        AUDIO_ERR_LOG("Stream Stop Failed : %{public}d", error);
        return AUDIO_CLIENT_ERR;
    }

    streamCmdStatus_ = 0;
    operation = pa_stream_cork(paStream, 1, PAStreamCorkSuccessCb, (void *)this);

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        StartTimer(CORK_TIMEOUT_IN_SEC);
        pa_threaded_mainloop_wait(mainLoop);
        StopTimer();
        if (IsTimeOut()) {
            pa_threaded_mainloop_unlock(mainLoop);
            AUDIO_ERR_LOG("CorkStream timeout");
            return AUDIO_CLIENT_ERR;
        }
    }
    pa_operation_unref(operation);
    if (InitializePAProbListOffload() != AUDIO_CLIENT_SUCCESS) {
        pa_threaded_mainloop_unlock(mainLoop);
        return AUDIO_CLIENT_ERR;
    }
    pa_threaded_mainloop_unlock(mainLoop);

    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::FlushStream()
{
    AUDIO_INFO_LOG("In");
    lock_guard<mutex> lock(dataMutex_);
    int32_t res = CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR);
    CHECK_AND_RETURN_RET(res >= 0, AUDIO_CLIENT_PA_ERR);

    pa_operation *operation = nullptr;
    pa_threaded_mainloop_lock(mainLoop);

    pa_stream_state_t state = pa_stream_get_state(paStream);
    if (state != PA_STREAM_READY) {
        int error = pa_context_errno(context);
        pa_threaded_mainloop_unlock(mainLoop);
        AUDIO_ERR_LOG("Stream Flush Failed, error: %{public}d", error);
        return AUDIO_CLIENT_ERR;
    }

    streamFlushStatus_ = 0;
    operation = pa_stream_flush(paStream, PAStreamFlushSuccessCb, (void *)this);
    if (operation == nullptr) {
        AUDIO_ERR_LOG("Stream Flush Operation Failed");
        pa_threaded_mainloop_unlock(mainLoop);
        return AUDIO_CLIENT_ERR;
    }

    UpdatePropListForFlush();

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        static bool triggerDumpStacktrace_ = true;
        if (triggerDumpStacktrace_ == true) {
            AudioXCollie audioXCollie("AudioServiceClient::DumpTrace", MAINLOOP_WAIT_TIMEOUT_IN_SEC, [this](void *) {
                audioSystemManager_->GetAudioParameter(FORCED_DUMP_PULSEAUDIO_STACKTRACE);
                triggerDumpStacktrace_ = false;
                pa_threaded_mainloop_signal(this->mainLoop, 0);
            }, nullptr, 0);
            pa_threaded_mainloop_wait(mainLoop);
        } else {
            StartTimer(FLUSH_TIMEOUT_IN_SEC);
            pa_threaded_mainloop_wait(mainLoop);
            StopTimer();
            if (IsTimeOut()) {
                pa_threaded_mainloop_unlock(mainLoop);
                AUDIO_ERR_LOG("FlushStream timeout");
                return AUDIO_CLIENT_ERR;
            }
        }
    }
    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(mainLoop);

    CHECK_AND_RETURN_RET_LOG(streamFlushStatus_ == 1, AUDIO_CLIENT_ERR, "Stream Flush failed");
    acache_.readIndex = 0;
    acache_.writeIndex = 0;
    acache_.isFull = false;
    return AUDIO_CLIENT_SUCCESS;
}

void AudioServiceClient::UpdatePropListForFlush()
{
    pa_proplist *propListFlushTrue = pa_proplist_new();
    pa_proplist_sets(propListFlushTrue, "stream.flush", "true");
    pa_operation *updatePropOperationTrue = pa_stream_proplist_update(paStream, PA_UPDATE_REPLACE, propListFlushTrue,
        nullptr, nullptr);
    pa_proplist_free(propListFlushTrue);
    pa_operation_unref(updatePropOperationTrue);

    pa_proplist *propListFlushFalse = pa_proplist_new();
    pa_proplist_sets(propListFlushFalse, "stream.flush", "false");
    pa_operation *updatePropOperationFalse = pa_stream_proplist_update(paStream, PA_UPDATE_REPLACE,
        propListFlushFalse, nullptr, nullptr);
    pa_proplist_free(propListFlushFalse);
    pa_operation_unref(updatePropOperationFalse);
}

int32_t AudioServiceClient::DrainStream()
{
    AUDIO_INFO_LOG("AudioServiceClient::DrainStream");
    int32_t error;

    CHECK_AND_RETURN_RET_LOG(eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK,
        AUDIO_CLIENT_ERR, "Drain is not supported");

    lock_guard<mutex> lock(dataMutex_);

    error = DrainAudioCache();
    CHECK_AND_RETURN_RET_LOG(error == AUDIO_CLIENT_SUCCESS, AUDIO_CLIENT_ERR,
        "Audio cache drain failed");
    int32_t res = CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR);
    CHECK_AND_RETURN_RET(res >= 0, AUDIO_CLIENT_PA_ERR);

    pa_operation *operation = nullptr;

    pa_threaded_mainloop_lock(mainLoop);

    pa_stream_state_t state = pa_stream_get_state(paStream);
    if (state != PA_STREAM_READY) {
        error = pa_context_errno(context);
        pa_threaded_mainloop_unlock(mainLoop);
        AUDIO_ERR_LOG("Stream Drain Failed");
        return AUDIO_CLIENT_ERR;
    }

    streamDrainStatus_ = 0;
    operation = pa_stream_drain(paStream, PAStreamDrainSuccessCb, (void *)this);

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        StartTimer(DRAIN_TIMEOUT_IN_SEC);
        pa_threaded_mainloop_wait(mainLoop);
        StopTimer();
        if (IsTimeOut()) {
            pa_threaded_mainloop_unlock(mainLoop);
            AUDIO_ERR_LOG("Drain timeout");
            return AUDIO_CLIENT_ERR;
        }
    }
    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(mainLoop);

    if (!streamDrainStatus_) {
        AUDIO_ERR_LOG("Stream Drain Failed");
        return AUDIO_CLIENT_ERR;
    } else {
        return AUDIO_CLIENT_SUCCESS;
    }
}

int32_t AudioServiceClient::PaWriteStream(const uint8_t *buffer, size_t &length)
{
    int error = 0;
    if (firstFrame_) {
        AudioSystemManager::GetInstance()->RequestThreadPriority(gettid());
        firstFrame_ = false;
    }
    if (!hasFirstFrameWrited_) { OnFirstFrameWriting(); }

    if ((lastOffloadUpdateFinishTime_ != 0) &&
        (chrono::system_clock::to_time_t(chrono::system_clock::now()) > lastOffloadUpdateFinishTime_)) {
        AUDIO_INFO_LOG("PaWriteStream switching curTime %{public}" PRIu64 ", switchTime %{public}" PRIu64,
            chrono::system_clock::to_time_t(chrono::system_clock::now()), lastOffloadUpdateFinishTime_);
        error = UpdatePolicyOffload(offloadNextStateTargetPolicy_);
        lastOffloadUpdateFinishTime_ = 0;
    }
    CHECK_AND_RETURN_RET(error == AUDIO_CLIENT_SUCCESS, error);

    while (length > 0) {
        size_t writableSize = 0;
        size_t origWritableSize = 0;

        error = WaitWriteable(length, writableSize);
        if (error != 0) {
            return error;
        }

        AUDIO_DEBUG_LOG("Write stream: writable size = %{public}zu, length = %{public}zu", writableSize, length);
        origWritableSize = writableSize;
        if (writableSize > length) {
            writableSize = length;
        }

        writableSize = AlignToAudioFrameSize(writableSize, sampleSpec);
        if (writableSize == 0) {
            AUDIO_ERR_LOG("Align to frame size failed");
            error = AUDIO_CLIENT_WRITE_STREAM_ERR;
            break;
        }

        Trace trace2("PaWriteStream Write:" + std::to_string(writableSize) + "/" + std::to_string(origWritableSize));
        error = pa_stream_write(paStream, (void *)buffer, writableSize, nullptr, 0LL,
                                PA_SEEK_RELATIVE);
        if (error < 0) {
            AUDIO_ERR_LOG("Write stream failed");
            error = AUDIO_CLIENT_WRITE_STREAM_ERR;
            break;
        }

        AUDIO_DEBUG_LOG("Writable size: %{public}zu, bytes to write: %{public}zu, return val: %{public}d",
                        writableSize, length, error);
        buffer = buffer + writableSize;
        length -= writableSize;

        HandleRenderPositionCallbacks(writableSize * speed_);
    }

    return error;
}

int32_t AudioServiceClient::WaitWriteable(size_t length, size_t& writableSize)
{
    Trace trace1("PaWriteStream WaitWriteable");
    while (true) {
        writableSize = pa_stream_writable_size(paStream);
        if (writableSize != 0) {
            if (!offloadEnable_) {
                break;
            }
            uint32_t tgt = acache_.totalCacheSizeTgt != 0 ? acache_.totalCacheSizeTgt : acache_.totalCacheSize;
            if (writableSize < tgt) {
                AUDIO_DEBUG_LOG("PaWriteStream: WaitWriteable writableSize %zu < %u wait for more",
                    writableSize, tgt);
            } else {
                writableSize = writableSize / tgt * tgt;
                break;
            }
        }
        AUDIO_DEBUG_LOG("PaWriteStream: WaitWriteable");
        StartTimer(WRITE_TIMEOUT_IN_SEC);
        if (!breakingWritePa_ && state_ == RUNNING) {
            pa_threaded_mainloop_wait(mainLoop);
        }
        StopTimer();
        if (IsTimeOut()) {
            AUDIO_ERR_LOG("Write timeout");
            return AUDIO_CLIENT_WRITE_STREAM_ERR;
        }
        if (breakingWritePa_ || state_ != RUNNING) {
            AUDIO_WARNING_LOG("PaWriteStream: WaitWriteable breakingWritePa(%d) or state_(%d) "
                "not running, Writable size: %{public}zu, bytes to write: %{public}zu",
                breakingWritePa_, state_, writableSize, length);
            return AUDIO_CLIENT_WRITE_STREAM_ERR;
        }
    }
    return 0;
}

void AudioServiceClient::HandleRenderPositionCallbacks(size_t bytesWritten)
{
    mTotalBytesWritten += bytesWritten;
    CHECK_AND_RETURN_LOG(mFrameSize != 0, "capturePeriodPositionCb not set");

    uint64_t writtenFrameNumber = mTotalBytesWritten / mFrameSize;
    AUDIO_DEBUG_LOG("frame size: %{public}d", mFrameSize);

    {
        std::lock_guard<std::mutex> lock(rendererMarkReachedMutex_);
        if (!mMarkReached) {
            AUDIO_DEBUG_LOG("frame mark position: %{public}" PRIu64 ", Total frames written: %{public}" PRIu64,
                static_cast<uint64_t>(mFrameMarkPosition), static_cast<uint64_t>(writtenFrameNumber));
            if (writtenFrameNumber >= mFrameMarkPosition) {
                AUDIO_DEBUG_LOG("audio service client OnMarkReached");
                SendRenderMarkReachedRequestEvent(mFrameMarkPosition);
                mMarkReached = true;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(rendererPeriodReachedMutex_);
        mFramePeriodWritten += (bytesWritten / mFrameSize);
        AUDIO_DEBUG_LOG("frame period number: %{public}" PRIu64 ", Total frames written: %{public}" PRIu64,
            static_cast<uint64_t>(mFramePeriodNumber), static_cast<uint64_t>(writtenFrameNumber));
        if (mFramePeriodWritten >= mFramePeriodNumber && mFramePeriodNumber > 0) {
            mFramePeriodWritten %= mFramePeriodNumber;
            AUDIO_DEBUG_LOG("OnPeriodReached, remaining frames: %{public}" PRIu64,
                static_cast<uint64_t>(mFramePeriodWritten));
            SendRenderPeriodReachedRequestEvent(mFramePeriodNumber);
        }
    }
}

int32_t AudioServiceClient::DrainAudioCache()
{
    int32_t res = CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR);
    CHECK_AND_RETURN_RET(res >= 0, AUDIO_CLIENT_PA_ERR);

    pa_threaded_mainloop_lock(mainLoop);

    int32_t error = 0;
    if (acache_.buffer == nullptr) {
        AUDIO_ERR_LOG("Drain cache failed");
        pa_threaded_mainloop_unlock(mainLoop);
        return AUDIO_CLIENT_ERR;
    }

    size_t length = acache_.writeIndex - acache_.readIndex;
    const uint8_t *buffer = acache_.buffer.get();

    error = PaWriteStream(buffer, length);

    acache_.readIndex = 0;
    acache_.writeIndex = 0;

    pa_threaded_mainloop_unlock(mainLoop);
    return error;
}

size_t AudioServiceClient::WriteToAudioCache(const StreamBuffer &stream)
{
    if (stream.buffer == nullptr) {
        return 0;
    }

    const uint8_t *inputBuffer = stream.buffer;
    uint8_t *cacheBuffer = acache_.buffer.get() + acache_.writeIndex;

    size_t inputLen = stream.bufferLen;
    if (acache_.totalCacheSize != acache_.totalCacheSizeTgt && acache_.totalCacheSizeTgt != 0) {
        uint32_t tgt = acache_.totalCacheSizeTgt;
        if (tgt < acache_.totalCacheSize && tgt < acache_.writeIndex) {
            tgt = acache_.writeIndex;
        }
        acache_.totalCacheSize = tgt;
    }
    while (inputLen > 0) {
        size_t writableSize = acache_.totalCacheSize - acache_.writeIndex;

        if (writableSize > inputLen) {
            writableSize = inputLen;
        }

        if (writableSize == 0) {
            break;
        }

        if (memcpy_s(cacheBuffer, acache_.totalCacheSize, inputBuffer, writableSize)) {
            break;
        }

        inputBuffer = inputBuffer + writableSize;
        cacheBuffer = cacheBuffer + writableSize;
        inputLen -= writableSize;
        acache_.writeIndex += writableSize;
    }

    if ((acache_.writeIndex - acache_.readIndex) == acache_.totalCacheSize) {
        acache_.isFull = true;
    }

    return (stream.bufferLen - inputLen);
}

size_t AudioServiceClient::WriteStreamInCb(const StreamBuffer &stream, int32_t &pError)
{
    lock_guard<mutex> lock(dataMutex_);
    int error = 0;
    if (CheckPaStatusIfinvalid(mainLoop, context, paStream, 0, pError) < 0) {
        return 0;
    }

    pa_threaded_mainloop_lock(mainLoop);

    const uint8_t *buffer = stream.buffer;
    size_t length = stream.bufferLen;
    error = PaWriteStream(buffer, length);
    pa_threaded_mainloop_unlock(mainLoop);
    pError = error;
    return (stream.bufferLen - length);
}

size_t AudioServiceClient::WriteStream(const StreamBuffer &stream, int32_t &pError)
{
    lock_guard<timed_mutex> lockoffload(offloadWaitWriteableMutex_);
    lock_guard<mutex> lock(dataMutex_);
    int error = 0;
    size_t cachedLen = WriteToAudioCache(stream);

    if (!acache_.isFull) {
        pError = error;
        return cachedLen;
    }
    if (CheckPaStatusIfinvalid(mainLoop, context, paStream, 0, pError) < 0) {
        return 0;
    }
    pa_threaded_mainloop_lock(mainLoop);

    if (acache_.buffer == nullptr) {
        AUDIO_ERR_LOG("Buffer is null");
        pa_threaded_mainloop_unlock(mainLoop);
        pError = AUDIO_CLIENT_WRITE_STREAM_ERR;
        return cachedLen;
    }

    const uint8_t *buffer = acache_.buffer.get();
    size_t length = acache_.totalCacheSize;

    error = PaWriteStream(buffer, length);
    const size_t written = acache_.totalCacheSize - length;
    if (!error && written > 0) {
        acache_.readIndex += written;
        acache_.isFull = false;
    }
    if (acache_.totalCacheSize < length) {
        AUDIO_ERR_LOG("WriteStream totalCacheSize(%u) < length(%zu)", acache_.totalCacheSize, length);
    }

    if (!error && (length >= 0) && !acache_.isFull) {
        error = AdjustAcache(stream, cachedLen);
    }

    pa_threaded_mainloop_unlock(mainLoop);
    pError = error;
    return cachedLen;
}

int32_t AudioServiceClient::AdjustAcache(const StreamBuffer& stream, size_t& cachedLen)
{
    if (acache_.isFull) {
        return 0;
    }
    uint8_t *cacheBuffer = acache_.buffer.get();
    uint32_t offset = acache_.readIndex;
    if (acache_.writeIndex > acache_.readIndex) {
        uint32_t size = (acache_.writeIndex - acache_.readIndex);
        auto* func = memcpy_s;
        if (offset < size) { // overlop
            func = memmove_s;
        }
        CHECK_AND_RETURN_RET_LOG(!func(cacheBuffer, acache_.totalCacheSize, cacheBuffer + offset, size),
            AUDIO_CLIENT_WRITE_STREAM_ERR, "Update cache failed, offset %u, size %u", offset, size);
        acache_.readIndex = 0;
        acache_.writeIndex = size;
    } else {
        acache_.readIndex = 0;
        acache_.writeIndex = 0;
    }

    if (cachedLen < stream.bufferLen) {
        StreamBuffer str;
        str.buffer = stream.buffer + cachedLen;
        str.bufferLen = stream.bufferLen - cachedLen;
        AUDIO_DEBUG_LOG("writing pending data to audio cache: %{public}d", str.bufferLen);
        cachedLen += WriteToAudioCache(str);
    }
    return 0;
}


int32_t AudioServiceClient::UpdateReadBuffer(uint8_t *buffer, size_t &length, size_t &readSize)
{
    size_t l = (internalRdBufLen_ < length) ? internalRdBufLen_ : length;
    CHECK_AND_RETURN_RET_LOG(!(memcpy_s(buffer, length, (const uint8_t*)internalReadBuffer_ + internalRdBufIndex_, l)),
        AUDIO_CLIENT_READ_STREAM_ERR, "Update read buffer failed");

    length -= l;
    internalRdBufIndex_ += l;
    internalRdBufLen_ -= l;
    readSize += l;

    if (!internalRdBufLen_) {
        int retVal = pa_stream_drop(paStream);
        internalReadBuffer_ = nullptr;
        internalRdBufLen_ = 0;
        internalRdBufIndex_ = 0;
        CHECK_AND_RETURN_RET_LOG(retVal >= 0, AUDIO_CLIENT_READ_STREAM_ERR,
            "pa_stream_drop failed, retVal: %{public}d", retVal);
    }

    return 0;
}

int32_t AudioServiceClient::RenderPrebuf(uint32_t writeLen)
{
    Trace trace("RenderPrebuf");
    if (CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR) < 0) {
        return AUDIO_CLIENT_PA_ERR;
    }
    const pa_buffer_attr *bufferAttr = pa_stream_get_buffer_attr(paStream);
    CHECK_AND_RETURN_RET_LOG(bufferAttr != nullptr, AUDIO_CLIENT_ERR,
        "pa_stream_get_buffer_attr returned nullptr");

    CHECK_AND_RETURN_RET(bufferAttr->maxlength > writeLen, AUDIO_CLIENT_SUCCESS);
    size_t diff = bufferAttr->maxlength - writeLen;

    int32_t writeError;
    StreamBuffer prebufStream;
    prebufStream.buffer = preBuf_.get();
    if (writeLen == 0) {
        return AUDIO_CLIENT_SUCCESS;
    } else if (writeLen > diff) {
        prebufStream.bufferLen = diff;
    } else {
        prebufStream.bufferLen = writeLen;
    }

    preFrameNum_ = prebufStream.bufferLen / mFrameSize;

    size_t bytesWritten {0};
    AUDIO_INFO_LOG("RenderPrebuf start");
    while (true) {
        bytesWritten += WriteStream(prebufStream, writeError);
        CHECK_AND_RETURN_RET_LOG(!writeError, AUDIO_CLIENT_ERR,
            "RenderPrebuf failed: %{public}d", writeError);

        if (diff <= bytesWritten) {
            break;
        }

        if ((diff - bytesWritten) < writeLen) {
            prebufStream.bufferLen = diff - bytesWritten;
        }
    }

    return AUDIO_CLIENT_SUCCESS;
}

void AudioServiceClient::OnTimeOut()
{
    AUDIO_ERR_LOG("Inside timeout callback with pid:%{public}d uid:%{public}d", clientPid_, clientUid_);

    CHECK_AND_RETURN_LOG(mainLoop != nullptr, "OnTimeOut failed: mainLoop == nullptr");
    pa_threaded_mainloop_signal(mainLoop, 0);
}

void AudioServiceClient::SetClientID(int32_t clientPid, int32_t clientUid, uint32_t appTokenId)
{
    AUDIO_DEBUG_LOG("Set client PID: %{public}d, UID: %{public}d", clientPid, clientUid);
    clientPid_ = clientPid;
    clientUid_ = clientUid;
    appTokenId_ = appTokenId;
}

IAudioStream::StreamClass AudioServiceClient::GetStreamClass()
{
    return streamClass_;
}

void AudioServiceClient::GetStreamSwitchInfo(SwitchInfo& info)
{
    info.cachePath = cachePath_;
    info.rendererSampleRate = rendererSampleRate;
    info.underFlowCount = underFlowCount;
    info.effectMode = effectMode;
    info.renderMode = renderMode_;
    info.captureMode = captureMode_;
    info.renderRate = renderRate;
    info.clientPid = clientPid_;
    info.clientUid = clientUid_;
    info.volume = volumeFactor_;

    info.frameMarkPosition = mFrameMarkPosition;
    info.renderPositionCb = mRenderPositionCb;
    info.capturePositionCb = mCapturePositionCb;

    info.framePeriodNumber = mFramePeriodNumber;
    info.renderPeriodPositionCb = mRenderPeriodPositionCb;
    info.capturePeriodPositionCb = mCapturePeriodPositionCb;

    info.rendererWriteCallback = writeCallback_;
}

void AudioServiceClient::HandleCapturePositionCallbacks(size_t bytesRead)
{
    mTotalBytesRead += bytesRead;
    CHECK_AND_RETURN_LOG(mFrameSize != 0, "HandleCapturePositionCallbacks: capturePeriodPositionCb not set");

    uint64_t readFrameNumber = mTotalBytesRead / mFrameSize;
    AUDIO_DEBUG_LOG("frame size: %{public}d", mFrameSize);
    {
        std::lock_guard<std::mutex> lock(capturerMarkReachedMutex_);
        if (!mMarkReached) {
            AUDIO_DEBUG_LOG("frame mark position: %{public}" PRIu64 ", Total frames read: %{public}" PRIu64,
                static_cast<uint64_t>(mFrameMarkPosition), static_cast<uint64_t>(readFrameNumber));
            if (readFrameNumber >= mFrameMarkPosition) {
                AUDIO_DEBUG_LOG("audio service client capturer OnMarkReached");
                SendCapturerMarkReachedRequestEvent(mFrameMarkPosition);
                mMarkReached = true;
            }
        }
    }

    {
        std::lock_guard<std::mutex> lock(capturerPeriodReachedMutex_);
        mFramePeriodRead += (bytesRead / mFrameSize);
        AUDIO_DEBUG_LOG("frame period number: %{public}" PRIu64 ", Total frames read: %{public}" PRIu64,
            static_cast<uint64_t>(mFramePeriodNumber), static_cast<uint64_t>(readFrameNumber));
        if (mFramePeriodRead >= mFramePeriodNumber && mFramePeriodNumber > 0) {
            mFramePeriodRead %= mFramePeriodNumber;
            AUDIO_DEBUG_LOG("audio service client OnPeriodReached, remaining frames: %{public}" PRIu64,
                static_cast<uint64_t>(mFramePeriodRead));
            SendCapturerPeriodReachedRequestEvent(mFramePeriodNumber);
        }
    }
}

int32_t AudioServiceClient::ReadStream(StreamBuffer &stream, bool isBlocking)
{
    uint8_t *buffer = stream.buffer;
    size_t length = stream.bufferLen;
    size_t readSize = 0;
    lock_guard<mutex> lock(dataMutex_);
    int32_t ret = CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR);
    CHECK_AND_RETURN_RET(ret >= 0, AUDIO_CLIENT_PA_ERR);

    pa_threaded_mainloop_lock(mainLoop);
    while (length > 0) {
        while (!internalReadBuffer_) {
            int retVal = pa_stream_peek(paStream, &internalReadBuffer_, &internalRdBufLen_);
            if (retVal < 0) {
                AUDIO_ERR_LOG("pa_stream_peek failed, retVal: %{public}d", retVal);
                pa_threaded_mainloop_unlock(mainLoop);
                return AUDIO_CLIENT_READ_STREAM_ERR;
            }

            if (internalRdBufLen_ <= 0) {
                if (isBlocking) {
                    StartTimer(READ_TIMEOUT_IN_SEC);
                    pa_threaded_mainloop_wait(mainLoop);
                    StopTimer();
                    if (IsTimeOut()) {
                        AUDIO_ERR_LOG("Read timeout");
                        pa_threaded_mainloop_unlock(mainLoop);
                        return AUDIO_CLIENT_READ_STREAM_ERR;
                    }
                } else {
                    pa_threaded_mainloop_unlock(mainLoop);
                    HandleCapturePositionCallbacks(readSize);
                    return readSize;
                }
            } else if (!internalReadBuffer_) {
                retVal = pa_stream_drop(paStream);
                if (retVal < 0) {
                    AUDIO_ERR_LOG("pa_stream_drop failed, retVal: %{public}d", retVal);
                    pa_threaded_mainloop_unlock(mainLoop);
                    return AUDIO_CLIENT_READ_STREAM_ERR;
                }
            } else {
                internalRdBufIndex_ = 0;
                AUDIO_DEBUG_LOG("buffer size from PA: %{public}zu", internalRdBufLen_);
            }
        }

        if (UpdateReadBuffer(buffer, length, readSize) != 0) {
            pa_threaded_mainloop_unlock(mainLoop);
            return AUDIO_CLIENT_READ_STREAM_ERR;
        }
        buffer = stream.buffer + readSize;
    }
    pa_threaded_mainloop_unlock(mainLoop);
    HandleCapturePositionCallbacks(readSize);

    return readSize;
}

int32_t AudioServiceClient::ReleaseStream(bool releaseRunner)
{
    state_ = RELEASED;
    lock_guard<mutex> lockdata(dataMutex_);
    ResetPAAudioClient();

    std::shared_ptr<AudioStreamCallback> streamCb = streamCallback_.lock();
    if (streamCb != nullptr) {
        streamCb->OnStateChange(RELEASED);
    }

    {
        lock_guard<mutex> runnerlock(runnerMutex_);
        if (releaseRunner) {
            AUDIO_INFO_LOG("runner remove");
            SetEventRunner(nullptr);
            runnerReleased_ = true;
        }
    }

    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::SetBufferSizeInMsec(int32_t bufferSizeInMsec)
{
    size_t bufferSize =  pa_usec_to_bytes(bufferSizeInMsec * PA_USEC_PER_MSEC, &sampleSpec);
    setBufferSize_ = bufferSize;
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::GetMinimumBufferSize(size_t &minBufferSize) const
{
    if (CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR) < 0) {
        return AUDIO_CLIENT_PA_ERR;
    }

    const pa_buffer_attr *bufferAttr = pa_stream_get_buffer_attr(paStream);

    CHECK_AND_RETURN_RET_LOG(bufferAttr != nullptr, AUDIO_CLIENT_ERR,
        "pa_stream_get_buffer_attr returned nullptr");

    if (eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK) {
        if (renderMode_ == RENDER_MODE_CALLBACK) {
            minBufferSize = (size_t)bufferAttr->minreq;
        } else {
            if (setBufferSize_) {
                minBufferSize = setBufferSize_;
            } else {
                minBufferSize = (size_t)bufferAttr->minreq;
            }
        }
    }

    if (eAudioClientType == AUDIO_SERVICE_CLIENT_RECORD) {
        minBufferSize = (size_t)bufferAttr->fragsize;
    }

    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::GetMinimumFrameCount(uint32_t &frameCount) const
{
    int32_t ret = CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR);
    CHECK_AND_RETURN_RET(ret >= 0, AUDIO_CLIENT_PA_ERR);

    size_t minBufferSize = 0;

    const pa_buffer_attr *bufferAttr = pa_stream_get_buffer_attr(paStream);

    CHECK_AND_RETURN_RET_LOG(bufferAttr != nullptr, AUDIO_CLIENT_ERR,
        "pa_stream_get_buffer_attr returned nullptr");

    if (eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK) {
        if (renderMode_ == RENDER_MODE_CALLBACK) {
            minBufferSize = (size_t)bufferAttr->minreq;
        } else {
            if (setBufferSize_) {
                minBufferSize = setBufferSize_;
            } else {
                minBufferSize = (size_t)bufferAttr->minreq;
            }
        }
    }

    if (eAudioClientType == AUDIO_SERVICE_CLIENT_RECORD) {
        minBufferSize = (size_t)bufferAttr->fragsize;
    }

    uint32_t bytesPerSample = pa_frame_size(&sampleSpec);
    CHECK_AND_RETURN_RET_LOG(bytesPerSample != 0, AUDIO_CLIENT_ERR,
        "GetMinimumFrameCount Failed");

    frameCount = minBufferSize / bytesPerSample;
    AUDIO_INFO_LOG("frame count: %{public}d", frameCount);
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::GetBufferSizeForCapturer(size_t &bufferSize)
{
    bufferSize = pa_usec_to_bytes(DEFAULT_BUFFER_TIME_MS * PA_USEC_PER_MSEC, &sampleSpec);
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::GetFrameCountForCapturer(uint32_t &frameCount)
{
    size_t bufferSize;
    GetBufferSizeForCapturer(bufferSize);
    size_t sampleSize = pa_sample_size_of_format(sampleSpec.format);
    frameCount = bufferSize / (sampleSize * sampleSpec.channels);
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::GetClientPid()
{
    return clientPid_;
}

uint32_t AudioServiceClient::GetSamplingRate() const
{
    return DEFAULT_SAMPLING_RATE;
}

uint8_t AudioServiceClient::GetChannelCount() const
{
    return DEFAULT_CHANNEL_COUNT;
}

uint8_t AudioServiceClient::GetSampleSize() const
{
    return DEFAULT_SAMPLE_SIZE;
}

int32_t AudioServiceClient::GetAudioStreamParams(AudioStreamParams& audioParams) const
{
    int32_t ret = CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR);
    CHECK_AND_RETURN_RET(ret >= 0, AUDIO_CLIENT_PA_ERR);

    const pa_sample_spec *paSampleSpec = pa_stream_get_sample_spec(paStream);

    if (!paSampleSpec) {
        AUDIO_ERR_LOG("GetAudioStreamParams Failed");
        return AUDIO_CLIENT_ERR;
    }

    audioParams = ConvertFromPAAudioParams(*paSampleSpec);
    audioParams.channelLayout = channelLayout_;
    return AUDIO_CLIENT_SUCCESS;
}

void AudioServiceClient::GetOffloadCurrentTimeStamp(uint64_t paTimeStamp, uint64_t paWriteIndex, uint64_t &outTimeStamp)
{
    if (!offloadEnable_ || eAudioClientType != AUDIO_SERVICE_CLIENT_PLAYBACK) {
        return;
    }

    uint64_t cacheTime = 0;
    GetOffloadApproximatelyCacheTime(paTimeStamp, paWriteIndex, cacheTime);
    outTimeStamp = paWriteIndex + cacheTime;
}

int32_t AudioServiceClient::GetCurrentTimeStamp(uint64_t &timeStamp)
{
    if (offloadWaitWriteableMutex_.try_lock_for(chrono::milliseconds(OFFLOAD_HDI_CACHE1))) {
        lock_guard<timed_mutex> lockoffload(offloadWaitWriteableMutex_, adopt_lock);
    } else if (offloadEnable_ && eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK && offloadTsLast_ != 0) {
        GetOffloadCurrentTimeStamp(0, 0, timeStamp);
        return AUDIO_CLIENT_SUCCESS;
    }
    lock_guard<mutex> lock(dataMutex_);
    int32_t ret = CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR);
    CHECK_AND_RETURN_RET(ret >= 0, AUDIO_CLIENT_PA_ERR);
    pa_threaded_mainloop_lock(mainLoop);

    if (eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK) {
        pa_operation *operation = pa_stream_update_timing_info(paStream, NULL, NULL);
        if (operation != nullptr) {
            while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
                pa_threaded_mainloop_wait(mainLoop);
            }
            pa_operation_unref(operation);
        } else {
            AUDIO_ERR_LOG("pa_stream_update_timing_info failed");
        }
    }

    const pa_timing_info *info = pa_stream_get_timing_info(paStream);
    if (info == nullptr) {
        AUDIO_ERR_LOG("pa_stream_get_timing_info failed");
        pa_threaded_mainloop_unlock(mainLoop);
        return AUDIO_CLIENT_ERR;
    }

    if (eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK) {
        timeStamp = pa_bytes_to_usec(info->write_index, &sampleSpec);
        uint64_t paTimeStamp = info->timestamp.tv_sec * AUDIO_US_PER_SECOND + info->timestamp.tv_usec;
        GetOffloadCurrentTimeStamp(paTimeStamp, timeStamp, timeStamp);
    } else if (eAudioClientType == AUDIO_SERVICE_CLIENT_RECORD) {
        if (pa_stream_get_time(paStream, &timeStamp)) {
            AUDIO_ERR_LOG("failed for AUDIO_SERVICE_CLIENT_RECORD");
            pa_threaded_mainloop_unlock(mainLoop);
            return AUDIO_CLIENT_ERR;
        }
        int32_t uid = static_cast<int32_t>(getuid());

        // 1013 is media_service's uid
        int32_t media_service = 1013;
        if (uid == media_service) {
            timeStamp = pa_bytes_to_usec(mTotalBytesRead, &sampleSpec);
        }
    }

    pa_threaded_mainloop_unlock(mainLoop);

    return AUDIO_CLIENT_SUCCESS;
}

void AudioServiceClient::GetOffloadApproximatelyCacheTime(uint64_t paTimeStamp, uint64_t paWriteIndex,
    uint64_t &cacheTimePaDsp)
{
    if (!offloadEnable_ || eAudioClientType != AUDIO_SERVICE_CLIENT_PLAYBACK) {
        return;
    }

    if (paTimeStamp == 0 && paWriteIndex == 0) {
        paTimeStamp = offloadTimeStamp_;
        paWriteIndex = offloadWriteIndex_;
    }

    bool first = offloadTsLast_ == 0;
    offloadTsLast_ = paWriteIndex;

    uint64_t ppTimeStamp = 0;
    uint64_t frames = 0;
    int64_t timeNowSteady = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count());
    bool offloadPwrActive = offloadStatePolicy_ != OFFLOAD_INACTIVE_BACKGROUND;
    if (offloadPwrActive ||
        timeNowSteady >
        static_cast<int64_t>(offloadLastHdiPosTs_ + AUDIO_US_PER_SECOND / 20)) { // 20 times per sec is max
        int64_t timeSec;
        int64_t timeNanoSec;
        int32_t ret = audioSystemManager_->OffloadGetPresentationPosition(frames, timeSec, timeNanoSec);
        if (ret) {
            return;
        }
        ppTimeStamp = timeSec * AUDIO_US_PER_SECOND + timeNanoSec / AUDIO_NS_PER_US;
        offloadLastHdiPosTs_ = ppTimeStamp;
        offloadLastHdiPosFrames_ = frames;
    } else {
        ppTimeStamp = timeNowSteady;
        int64_t timeDelta = static_cast<int64_t>(timeNowSteady) - static_cast<int64_t>(offloadLastHdiPosTs_);
        timeDelta = timeDelta > 0 ? timeDelta : 0;
        frames = offloadLastHdiPosFrames_ + timeDelta;
    }

    int64_t timeDelta = static_cast<int64_t>(paTimeStamp) - static_cast<int64_t>(ppTimeStamp);
    int64_t framesInt = static_cast<int64_t>(frames) + timeDelta;
    framesInt = framesInt > 0 ? framesInt : 0;
    int64_t writeIndexInt = static_cast<int64_t>(paWriteIndex);
    if (framesInt + offloadTsOffset_ < writeIndexInt - static_cast<int64_t>(
        (OFFLOAD_HDI_CACHE2 + MAX_LENGTH_OFFLOAD + OFFLOAD_BUFFER) * AUDIO_US_PER_MS) ||
        framesInt + offloadTsOffset_ > writeIndexInt || first) {
        offloadTsOffset_ = writeIndexInt - framesInt;
    }
    cacheTimePaDsp = static_cast<uint64_t>(writeIndexInt - (framesInt + offloadTsOffset_));
}

int32_t AudioServiceClient::UpdateOffloadStreamPosition(UpdatePositionTimeNode node, uint64_t& frames,
    int64_t& timeSec, int64_t& timeNanoSec)
{
    if (node == UpdatePositionTimeNode::CORKED_NODE) {
        lastOffloadStreamCorkedPosition_ = mTotalBytesWritten / mFrameSize;
        return AUDIO_CLIENT_SUCCESS;
    }
    frames = frames * HDI_OFFLOAD_SAMPLE_RATE / SECOND_TO_MICROSECOND;
    lastStreamPosition_ = lastOffloadStreamCorkedPosition_ + frames;
    lastPositionTimestamp_ = timeSec * AUDIO_S_TO_NS + timeNanoSec;
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::UpdateStreamPosition(UpdatePositionTimeNode node)
{
    uint64_t frames;
    int64_t timeSec;
    int64_t timeNanoSec;
    unique_lock<mutex> positionLock(streamPositionMutex_);
    std::string deviceClass = offloadEnable_ ? "offload" : "primary";
    if (eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK &&
        audioSystemManager_->GetRenderPresentationPosition(deviceClass, frames, timeSec, timeNanoSec) == 0) {
        if (offloadEnable_) {
            return UpdateOffloadStreamPosition(node, frames, timeSec, timeNanoSec);
        }
        frames = frames * sampleSpec.rate / HDI_OFFLOAD_SAMPLE_RATE; // revert hdi frame size to client size
        if (frames < lastHdiPosition_) {
            AUDIO_ERR_LOG("The frame position should be continuously increasing");
            return AUDIO_CLIENT_ERR;
        }
        if (firstUpdatePosition_) {
            lastHdiPosition_ = frames;
            firstUpdatePosition_ = false;
        }
        if (node == UpdatePositionTimeNode::USER_NODE && state_ == RUNNING) {
            lastStreamPosition_ = lastStreamPosition_ + frames - lastHdiPosition_;
            lastPositionTimestamp_ = timeSec * AUDIO_S_TO_NS + timeNanoSec;
        } else if (node == UpdatePositionTimeNode::CORKED_NODE) {
            // In the current paused state, all data will be flushed
            lastStreamPosition_ = mTotalBytesWritten / mFrameSize;
        } else {
            AUDIO_INFO_LOG("other node value %{public}d!", node);
        }
        lastHdiPosition_ = frames;
        return AUDIO_CLIENT_SUCCESS;
    } else if (eAudioClientType == AUDIO_SERVICE_CLIENT_RECORD &&
        audioSystemManager_->GetCapturePresentationPosition("primary", frames, timeSec, timeNanoSec) == 0) {
        lastStreamPosition_ = frames;
        lastPositionTimestamp_ = timeSec * AUDIO_S_TO_NS + timeNanoSec;
        return AUDIO_CLIENT_SUCCESS;
    } else {
        AUDIO_ERR_LOG("UpdateStreamPosition from hdi failed!");
        return AUDIO_CLIENT_ERR;
    }
}

int32_t AudioServiceClient::GetCurrentPosition(uint64_t &framePosition, uint64_t &timeStamp)
{
    // Since the START processing between the framework service and hdf service is asynchronous, the application
    // may get the frame information without the hdf service completing the stream initialization.
    // This time will get an exception value, here will not return the exception to the application.Return the
    // last frame information instead
    UpdateStreamPosition(UpdatePositionTimeNode::USER_NODE);
    if (lastStreamPosition_ > preFrameNum_) {
        framePosition = lastStreamPosition_ - preFrameNum_;
    } else {
        framePosition = 0;
    }
    uint64_t latency = 0;
    GetAudioLatency(latency);
    latency = latency * sampleSpec.rate / SECOND_TO_MICROSECOND;
    if (framePosition > latency) {
        framePosition -= latency;
    } else {
        framePosition = 0;
    }
    timeStamp = lastPositionTimestamp_;
    return AUDIO_CLIENT_SUCCESS;
}

void AudioServiceClient::GetAudioLatencyOffload(uint64_t &latency)
{
    int64_t timeNow = static_cast<int64_t>(std::chrono::duration_cast<std::chrono::microseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count());
    bool offloadPwrActive = offloadStatePolicy_ != OFFLOAD_INACTIVE_BACKGROUND;
    if (offloadPwrActive ||
        timeNow >
        static_cast<int64_t>(offloadLastUpdatePaInfoTs_ + AUDIO_US_PER_SECOND / 20)) { // 20 times per sec is max
        pa_threaded_mainloop_lock(mainLoop);
        pa_operation *operation = pa_stream_update_timing_info(
            paStream, PAStreamUpdateTimingInfoSuccessCb, (void *)this);
        if (operation != nullptr) {
            pa_operation_unref(operation);
        } else {
            AUDIO_ERR_LOG("pa_stream_update_timing_info failed");
        }
        pa_threaded_mainloop_unlock(mainLoop);
        offloadLastUpdatePaInfoTs_ = timeNow;
    }

    uint64_t paTimeStamp = offloadTimeStamp_;
    uint64_t paWriteIndex = offloadWriteIndex_;
    uint64_t paReadIndex = offloadReadIndex_;

    uint64_t cacheTime = 0;
    GetOffloadApproximatelyCacheTime(paTimeStamp, paWriteIndex, cacheTime);

    pa_usec_t cacheLatency = pa_bytes_to_usec((acache_.totalCacheSize - acache_.writeIndex), &sampleSpec);

    // Total latency will be sum of audio write cache latency + PA latency
    uint64_t paLatency = paWriteIndex > paReadIndex ? paWriteIndex - paReadIndex : 0;
    uint64_t fwLatency = paLatency + cacheLatency + cacheTime;
    uint64_t sinkLatency = sinkLatencyInMsec_ * PA_USEC_PER_MSEC;
    latency = fwLatency > sinkLatency ? fwLatency - sinkLatency : fwLatency;
    AUDIO_DEBUG_LOG("total latency: %{public}" PRIu64 ", pa latency: %{public}" PRIu64
        ", cache latency: %{public}" PRIu64 ", dsp latency: %{public}" PRIu64,
        latency, paLatency, cacheLatency, cacheTime);
}

int32_t AudioServiceClient::GetAudioLatency(uint64_t &latency)
{
    if (offloadEnable_ && eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK) {
        GetAudioLatencyOffload(paLatency_);
        return AUDIO_CLIENT_SUCCESS;
    }

    lock_guard<mutex> lock(dataMutex_);
    if (CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR) < 0) {
        return AUDIO_CLIENT_PA_ERR;
    }
    pa_usec_t cacheLatency {0};

    // Get PA latency
    pa_threaded_mainloop_lock(mainLoop);
    pa_operation *operation = pa_stream_update_timing_info(paStream, PAStreamUpdateTimingInfoSuccessCb, (void *)this);
    if (operation != nullptr) {
        pa_operation_unref(operation);
    } else {
        AUDIO_ERR_LOG("pa_stream_update_timing_info failed");
    }
    AUDIO_DEBUG_LOG("waiting for audio latency information");
    StartTimer(INIT_TIMEOUT_IN_SEC);
    pa_threaded_mainloop_wait(mainLoop);
    StopTimer();
    if (IsTimeOut()) {
        AUDIO_ERR_LOG("Get audio latency timeout");
        pa_threaded_mainloop_unlock(mainLoop);
        return AUDIO_CLIENT_ERR;
    }
    pa_threaded_mainloop_unlock(mainLoop);
    CHECK_AND_RETURN_RET_LOG(isGetLatencySuccess_ != false, AUDIO_CLIENT_ERR, "Get audio latency failed");

    if (eAudioClientType == AUDIO_SERVICE_CLIENT_PLAYBACK) {
        // Get audio write cache latency
        cacheLatency = pa_bytes_to_usec((acache_.totalCacheSize - acache_.writeIndex), &sampleSpec);

        // Total latency will be sum of audio write cache latency + PA latency
        uint64_t fwLatency = paLatency_ + cacheLatency;
        uint64_t sinkLatency = sinkLatencyInMsec_ * PA_USEC_PER_MSEC;
        latency = fwLatency > sinkLatency ? fwLatency - sinkLatency : fwLatency;
        AUDIO_DEBUG_LOG("total latency: %{public}" PRIu64 ", pa latency: %{public}"
            PRIu64 ", cache latency: %{public}" PRIu64, latency, paLatency_, cacheLatency);
    } else if (eAudioClientType == AUDIO_SERVICE_CLIENT_RECORD) {
        // Get audio read cache latency
        cacheLatency = pa_bytes_to_usec(internalRdBufLen_, &sampleSpec);

        // Total latency will be sum of audio read cache latency + PA latency
        latency = paLatency_ + cacheLatency;
        AUDIO_DEBUG_LOG("total latency: %{public}" PRIu64 ", pa latency: %{public}" PRIu64, latency, paLatency_);
    }

    return AUDIO_CLIENT_SUCCESS;
}

void AudioServiceClient::RegisterAudioRendererCallbacks(const AudioRendererCallbacks &cb)
{
    AUDIO_DEBUG_LOG("Registering audio render callbacks");
    mAudioRendererCallbacks = (AudioRendererCallbacks *)&cb;
}

void AudioServiceClient::RegisterAudioCapturerCallbacks(const AudioCapturerCallbacks &cb)
{
    AUDIO_DEBUG_LOG("Registering audio record callbacks");
    mAudioCapturerCallbacks = (AudioCapturerCallbacks *)&cb;
}

void AudioServiceClient::SetRendererPositionCallback(int64_t markPosition,
    const std::shared_ptr<RendererPositionCallback> &callback)
{
    std::lock_guard<std::mutex> lock(rendererMarkReachedMutex_);
    AUDIO_INFO_LOG("Registering render frame position callback mark position: %{public}" PRIu64, markPosition);
    mFrameMarkPosition = markPosition;
    SendSetRenderMarkReachedRequestEvent(callback);
    mMarkReached = false;
}

void AudioServiceClient::UnsetRendererPositionCallback()
{
    AUDIO_INFO_LOG("Unregistering render frame position callback");
    std::lock_guard<std::mutex> lock(rendererMarkReachedMutex_);
    SendUnsetRenderMarkReachedRequestEvent();
    mMarkReached = false;
    mFrameMarkPosition = 0;
}

int32_t AudioServiceClient::SetRendererFirstFrameWritingCallback(
    const std::shared_ptr<AudioRendererFirstFrameWritingCallback> &callback)
{
    AUDIO_INFO_LOG("SetRendererFirstFrameWritingCallback in.");
    CHECK_AND_RETURN_RET_LOG(callback, ERR_INVALID_PARAM, "callback is nullptr");
    firstFrameWritingCb_ = callback;
    return SUCCESS;
}

void AudioServiceClient::OnFirstFrameWriting()
{
    UpdateStreamPosition(UpdatePositionTimeNode::START_NODE);
    hasFirstFrameWrited_ = true;
    CHECK_AND_RETURN_LOG(firstFrameWritingCb_!= nullptr, "firstFrameWritingCb_ is null.");
    uint64_t latency = AUDIO_FIRST_FRAME_LATENCY;
    AUDIO_DEBUG_LOG("OnFirstFrameWriting: latency %{public}" PRIu64 "", latency);
    firstFrameWritingCb_->OnFirstFrameWriting(latency);
}

void AudioServiceClient::SetRendererPeriodPositionCallback(int64_t periodPosition,
    const std::shared_ptr<RendererPeriodPositionCallback> &callback)
{
    std::lock_guard<std::mutex> lock(rendererPeriodReachedMutex_);
    AUDIO_INFO_LOG("Registering render period position callback");
    mFramePeriodNumber = periodPosition;
    if ((mFrameSize != 0) && (mFramePeriodNumber != 0)) {
        mFramePeriodWritten = (mTotalBytesWritten / mFrameSize) % mFramePeriodNumber;
    } else {
        AUDIO_ERR_LOG("SetRendererPeriodPositionCallback failed");
        return;
    }
    SendSetRenderPeriodReachedRequestEvent(callback);
}

void AudioServiceClient::UnsetRendererPeriodPositionCallback()
{
    AUDIO_INFO_LOG("Unregistering render period position callback");
    std::lock_guard<std::mutex> lock(rendererPeriodReachedMutex_);
    SendUnsetRenderPeriodReachedRequestEvent();
    mFramePeriodWritten = 0;
    mFramePeriodNumber = 0;
}

void AudioServiceClient::SetCapturerPositionCallback(int64_t markPosition,
    const std::shared_ptr<CapturerPositionCallback> &callback)
{
    std::lock_guard<std::mutex> lock(capturerMarkReachedMutex_);
    AUDIO_INFO_LOG("Registering capture frame position callback, mark position: %{public}" PRIu64, markPosition);
    mFrameMarkPosition = markPosition;
    SendSetCapturerMarkReachedRequestEvent(callback);
    mMarkReached = false;
}

void AudioServiceClient::UnsetCapturerPositionCallback()
{
    AUDIO_INFO_LOG("Unregistering capture frame position callback");
    std::lock_guard<std::mutex> lock(capturerMarkReachedMutex_);
    SendUnsetCapturerMarkReachedRequestEvent();
    mMarkReached = false;
    mFrameMarkPosition = 0;
}

void AudioServiceClient::SetCapturerPeriodPositionCallback(int64_t periodPosition,
    const std::shared_ptr<CapturerPeriodPositionCallback> &callback)
{
    std::lock_guard<std::mutex> lock(capturerPeriodReachedMutex_);
    AUDIO_INFO_LOG("Registering period position callback");
    mFramePeriodNumber = periodPosition;
    if ((mFrameSize != 0) && (mFramePeriodNumber != 0)) {
        mFramePeriodRead = (mTotalBytesRead / mFrameSize) % mFramePeriodNumber;
    } else {
        AUDIO_ERR_LOG("SetCapturerPeriodPositionCallback failed");
        return;
    }
    SendSetCapturerPeriodReachedRequestEvent(callback);
}

void AudioServiceClient::UnsetCapturerPeriodPositionCallback()
{
    AUDIO_INFO_LOG("Unregistering period position callback");
    std::lock_guard<std::mutex> lock(capturerPeriodReachedMutex_);
    SendUnsetCapturerPeriodReachedRequestEvent();
    mFramePeriodRead = 0;
    mFramePeriodNumber = 0;
}

int32_t AudioServiceClient::SetStreamType(AudioStreamType audioStreamType)
{
    AUDIO_INFO_LOG("SetStreamType: %{public}d", audioStreamType);

    CHECK_AND_RETURN_RET_LOG(context != nullptr, AUDIO_CLIENT_ERR,
        "context is null");

    pa_threaded_mainloop_lock(mainLoop);

    streamType_ = audioStreamType;
    const std::string streamName = GetStreamName(audioStreamType);
    effectSceneName = IAudioStream::GetEffectSceneName(audioStreamType);

    pa_proplist *propList = pa_proplist_new();
    if (propList == nullptr) {
        AUDIO_ERR_LOG("pa_proplist_new failed");
        pa_threaded_mainloop_unlock(mainLoop);
        return AUDIO_CLIENT_ERR;
    }

    pa_proplist_sets(propList, "stream.type", streamName.c_str());
    pa_proplist_sets(propList, "media.name", streamName.c_str());
    pa_proplist_sets(propList, "scene.type", effectSceneName.c_str());
    pa_proplist_sets(propList, "stream.flush", "false");
    pa_operation *updatePropOperation = pa_stream_proplist_update(paStream, PA_UPDATE_REPLACE, propList,
        nullptr, nullptr);
    pa_proplist_free(propList);
    pa_operation_unref(updatePropOperation);

    pa_threaded_mainloop_unlock(mainLoop);

    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::SetStreamVolume(float volume)
{
    Trace trace("AudioServiceClient::SetStreamVolume " +std::to_string(volume));
    lock_guard<mutex> lock(ctrlMutex_);
    AUDIO_INFO_LOG("SetVolume volume: %{public}f", volume);

    int32_t res = CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR);
    CHECK_AND_RETURN_RET_LOG(res >= 0, AUDIO_CLIENT_PA_ERR, "set stream volume: invalid stream state");

    /* Validate and return INVALID_PARAMS error */
    CHECK_AND_RETURN_RET_LOG((volume >= MIN_STREAM_VOLUME_LEVEL) && (volume <= MAX_STREAM_VOLUME_LEVEL),
        AUDIO_CLIENT_INVALID_PARAMS_ERR, "Invalid Volume Input!");

    pa_threaded_mainloop_lock(mainLoop);
    int32_t ret = SetStreamVolumeInML(volume);
    pa_threaded_mainloop_unlock(mainLoop);

    if (offloadEnable_) {
        audioSystemManager_->OffloadSetVolume(GetSingleStreamVol() * duckVolumeFactor_);
    }

    return ret;
}

int32_t AudioServiceClient::SetStreamVolumeInML(float volume)
{
    volumeFactor_ = volume;
    pa_proplist *propList = pa_proplist_new();
    if (propList == nullptr) {
        AUDIO_ERR_LOG("pa_proplist_new failed");
        return AUDIO_CLIENT_ERR;
    }

    pa_proplist_sets(propList, "stream.volumeFactor", std::to_string(volumeFactor_).c_str());
    pa_operation *updatePropOperation = pa_stream_proplist_update(paStream, PA_UPDATE_REPLACE, propList,
        nullptr, nullptr);
    if (updatePropOperation == nullptr) {
        AUDIO_ERR_LOG("pa_stream_proplist_update returned null");
        pa_proplist_free(propList);
        return AUDIO_CLIENT_ERR;
    }

    pa_proplist_free(propList);
    pa_operation_unref(updatePropOperation);

    if (audioSystemManager_ == nullptr) {
        AUDIO_ERR_LOG("System manager instance is null");
        return AUDIO_CLIENT_ERR;
    }

    if (!streamInfoUpdated_) {
        uint32_t idx = pa_stream_get_index(paStream);
        pa_operation *operation = pa_context_get_sink_input_info(context, idx, AudioServiceClient::GetSinkInputInfoCb,
            reinterpret_cast<void *>(this));
        if (operation == nullptr) {
            AUDIO_ERR_LOG("pa_context_get_sink_input_info_list returned null");
            return AUDIO_CLIENT_ERR;
        }

        while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
            pa_threaded_mainloop_wait(mainLoop);
        }

        pa_operation_unref(operation);
    } else {
        SetPaVolume(*this);
    }

    return AUDIO_CLIENT_SUCCESS;
}

float AudioServiceClient::GetStreamVolume()
{
    return volumeFactor_;
}

int32_t AudioServiceClient::InitializePAProbListOffload()
{
    if (CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR) < 0) {
        AUDIO_ERR_LOG("set offload mode: invalid stream state");
        return AUDIO_CLIENT_PA_ERR;
    }

    pa_proplist *propList = pa_proplist_new();
    if (propList == nullptr) {
        pa_threaded_mainloop_unlock(mainLoop);
        return AUDIO_CLIENT_ERR;
    }

    pa_proplist_sets(propList, "stream.offload.statePolicy", "0");

    pa_operation *updatePropOperation =
        pa_stream_proplist_update(paStream, PA_UPDATE_REPLACE, propList, nullptr, nullptr);
    if (updatePropOperation == nullptr) {
        pa_proplist_free(propList);
        pa_threaded_mainloop_unlock(mainLoop);
        return AUDIO_CLIENT_ERR;
    }

    pa_proplist_free(propList);
    pa_operation_unref(updatePropOperation);
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::UpdatePAProbListOffload(AudioOffloadType statePolicy)
{
    if (CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR) < 0) {
        AUDIO_ERR_LOG("Set offload mode: invalid stream state, quit SetStreamOffloadMode due err");
        return AUDIO_CLIENT_PA_ERR;
    }

    // if possible turn on the buffer immediately(long buffer -> short buffer), turn it at once.
    if ((statePolicy < offloadStatePolicy_) || (offloadStatePolicy_ == OFFLOAD_DEFAULT)) {
        AUDIO_DEBUG_LOG("Update statePolicy immediately: %{public}d -> %{public}d", offloadStatePolicy_, statePolicy);
        lastOffloadUpdateFinishTime_ = 0;
        pa_threaded_mainloop_lock(mainLoop);
        int32_t ret = UpdatePolicyOffload(statePolicy);
        pa_threaded_mainloop_unlock(mainLoop);
        offloadNextStateTargetPolicy_ = statePolicy; // Fix here if sometimes can't cut into state 3
        return ret;
    } else {
        // Otherwise, hdi_sink.c's times detects the stateTarget change and switches later
        // this time is checked the PaWriteStream to check if the switch has been made
        AUDIO_DEBUG_LOG("Update statePolicy in 3 seconds: %{public}d -> %{public}d", offloadStatePolicy_, statePolicy);
        lastOffloadUpdateFinishTime_ = chrono::system_clock::to_time_t(
            chrono::system_clock::now() + std::chrono::seconds(3)); // add 3s latency to change offload state
        offloadNextStateTargetPolicy_ = statePolicy;
    }

    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::UpdatePolicyOffload(AudioOffloadType statePolicy)
{
    pa_proplist *propList = pa_proplist_new();
    CHECK_AND_RETURN_RET_LOG(propList != nullptr, AUDIO_CLIENT_ERR,
        "pa_proplist_new failed");
    if (offloadEnable_) {
        pa_proplist_sets(propList, "stream.offload.enable", "1");
    } else {
        pa_proplist_sets(propList, "stream.offload.enable", "0");
    }
    pa_proplist_sets(propList, "stream.offload.statePolicy", std::to_string(statePolicy).c_str());

    pa_operation *updatePropOperation =
        pa_stream_proplist_update(paStream, PA_UPDATE_REPLACE, propList, nullptr, nullptr);
    if (updatePropOperation == nullptr) {
        AUDIO_ERR_LOG("pa_stream_proplist_update failed!");
        pa_proplist_free(propList);
        return AUDIO_CLIENT_ERR;
    }
    pa_proplist_free(propList);
    pa_operation_unref(updatePropOperation);

    const uint32_t bufLenMs = statePolicy > 1 ? OFFLOAD_HDI_CACHE2 : OFFLOAD_HDI_CACHE1;
    audioSystemManager_->OffloadSetBufferSize(bufLenMs);

    offloadStatePolicy_ = statePolicy;

    return AUDIO_CLIENT_SUCCESS;
}

void AudioServiceClient::ResetOffload()
{
    offloadEnable_ = false;
    offloadTsOffset_ = 0;
    offloadTsLast_ = 0;
    offloadStatePolicy_ = OFFLOAD_DEFAULT;
    offloadNextStateTargetPolicy_ = OFFLOAD_DEFAULT;
    lastOffloadUpdateFinishTime_ = 0;
    acache_.totalCacheSizeTgt = 0;
    if (paStream != nullptr) {
        const pa_buffer_attr *bufferAttr = pa_stream_get_buffer_attr(paStream);
        if (bufferAttr != nullptr) {
            acache_.totalCacheSizeTgt = bufferAttr->minreq;
        }
    }
}

int32_t AudioServiceClient::SetStreamOffloadMode(int32_t state, bool isAppBack)
{
#ifdef FEATURE_POWER_MANAGER
    AudioOffloadType statePolicy = OFFLOAD_DEFAULT;
    auto powerState = static_cast<PowerMgr::PowerState>(state);
    if ((powerState != PowerMgr::PowerState::AWAKE) && (powerState != PowerMgr::PowerState::FREEZE)) {
        statePolicy = OFFLOAD_INACTIVE_BACKGROUND;
    } else if (isAppBack) {
        statePolicy = OFFLOAD_ACTIVE_FOREGROUND;
    } else if (!isAppBack) {
        statePolicy = OFFLOAD_ACTIVE_FOREGROUND;
    }

    if (statePolicy == OFFLOAD_DEFAULT) {
        AUDIO_ERR_LOG("impossible INPUT branch error");
        return AUDIO_CLIENT_ERR;
    }

    AUDIO_INFO_LOG("calling set stream offloadMode PowerState: %{public}d, isAppBack: %{public}d", state, isAppBack);

    if (offloadNextStateTargetPolicy_ == statePolicy) {
        return AUDIO_CLIENT_SUCCESS;
    }

    if ((offloadStatePolicy_ == offloadNextStateTargetPolicy_) && (offloadStatePolicy_ == statePolicy)) {
        return AUDIO_CLIENT_SUCCESS;
    }

    lock_guard<mutex> lock(ctrlMutex_);

    offloadEnable_ = true;
    if (UpdatePAProbListOffload(statePolicy) != AUDIO_CLIENT_SUCCESS) {
        return AUDIO_CLIENT_ERR;
    }
    if (statePolicy == OFFLOAD_ACTIVE_FOREGROUND) {
        pa_threaded_mainloop_signal(mainLoop, 0);
    }
#else
    AUDIO_INFO_LOG("SetStreamOffloadMode not available, FEATURE_POWER_MANAGER no define");
#endif
    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::UnsetStreamOffloadMode()
{
    lastOffloadUpdateFinishTime_ = 0;
    offloadEnable_ = false;
    pa_threaded_mainloop_lock(mainLoop);
    int32_t ret = UpdatePolicyOffload(OFFLOAD_ACTIVE_FOREGROUND);
    pa_threaded_mainloop_unlock(mainLoop);
    offloadNextStateTargetPolicy_ = OFFLOAD_DEFAULT;
    offloadStatePolicy_ = OFFLOAD_DEFAULT;
    return ret;
}

void AudioServiceClient::GetSinkInputInfoCb(pa_context *context, const pa_sink_input_info *info, int eol,
    void *userdata)
{
    AUDIO_INFO_LOG("GetSinkInputInfoVolumeCb in");
    AudioServiceClient *thiz = reinterpret_cast<AudioServiceClient *>(userdata);

    CHECK_AND_RETURN_LOG(eol >= 0, "Failed to get sink input information: %{public}s",
        pa_strerror(pa_context_errno(context)));

    if (eol) {
        pa_threaded_mainloop_signal(thiz->mainLoop, 0);
        return;
    }

    CHECK_AND_RETURN_LOG(info->proplist != nullptr, "Invalid prop list for sink input (%{public}d).", info->index);

    uint32_t sessionID = 0;
    const char *sessionCStr = pa_proplist_gets(info->proplist, "stream.sessionID");
    if (sessionCStr != nullptr) {
        std::stringstream sessionStr;
        sessionStr << sessionCStr;
        sessionStr >> sessionID;
    }

    thiz->cvolume = info->volume;
    thiz->streamIndex_ = info->index;
    thiz->sessionID_ = sessionID;
    thiz->volumeChannels_ = info->channel_map.channels;
    thiz->streamInfoUpdated_ = true;

    SetPaVolume(*thiz);

    return;
}

void AudioServiceClient::SetPaVolume(const AudioServiceClient &client)
{
    pa_cvolume cv = client.cvolume;
    AudioVolumeType volumeType = GetVolumeTypeFromStreamType(client.streamType_);
    int32_t systemVolumeLevel = AudioSystemManager::GetInstance()->GetVolume(volumeType);
    DeviceType deviceType = AudioSystemManager::GetInstance()->GetActiveOutputDevice();
    bool isAbsVolumeScene = AudioPolicyManager::GetInstance().IsAbsVolumeScene();
    float systemVolumeDb = AudioPolicyManager::GetInstance().GetSystemVolumeInDb(volumeType,
        systemVolumeLevel, deviceType);
    if (isAbsVolumeScene && (deviceType == DEVICE_TYPE_BLUETOOTH_A2DP || deviceType == DEVICE_TYPE_BLUETOOTH_SCO)) {
        systemVolumeDb = 1.0f; // Transfer raw pcm data to bluetooth device
    }
    float vol = systemVolumeDb * client.volumeFactor_ * client.powerVolumeFactor_ * client.duckVolumeFactor_;

    uint32_t volume = pa_sw_volume_from_linear(vol);
    pa_cvolume_set(&cv, client.volumeChannels_, volume);
    pa_operation_unref(pa_context_set_sink_input_volume(client.context, client.streamIndex_, &cv, nullptr, nullptr));

    AUDIO_INFO_LOG("Applied volume %{public}f, systemVolume %{public}f, volumeFactor %{public}f", vol, systemVolumeDb,
        client.volumeFactor_);
    HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO,
        "VOLUME_CHANGE", HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "ISOUTPUT", 1,
        "STREAMID", client.sessionID_,
        "APP_UID", client.clientUid_,
        "APP_PID", client.clientPid_,
        "STREAMTYPE", client.streamType_,
        "VOLUME", vol,
        "SYSVOLUME", systemVolumeLevel,
        "VOLUMEFACTOR", client.volumeFactor_,
        "POWERVOLUMEFACTOR", client.powerVolumeFactor_);
}

AudioVolumeType AudioServiceClient::GetVolumeTypeFromStreamType(AudioStreamType streamType)
{
    switch (streamType) {
        case STREAM_VOICE_CALL:
        case STREAM_VOICE_MESSAGE:
            return STREAM_VOICE_CALL;
        case STREAM_RING:
        case STREAM_SYSTEM:
        case STREAM_NOTIFICATION:
        case STREAM_SYSTEM_ENFORCED:
        case STREAM_DTMF:
            return STREAM_RING;
        case STREAM_MUSIC:
        case STREAM_MEDIA:
        case STREAM_MOVIE:
        case STREAM_GAME:
        case STREAM_SPEECH:
        case STREAM_NAVIGATION:
            return STREAM_MUSIC;
        case STREAM_VOICE_ASSISTANT:
            return STREAM_VOICE_ASSISTANT;
        case STREAM_ALARM:
            return STREAM_ALARM;
        case STREAM_ACCESSIBILITY:
            return STREAM_ACCESSIBILITY;
        case STREAM_ULTRASONIC:
            return STREAM_ULTRASONIC;
        default:
            AUDIO_ERR_LOG("GetVolumeTypeFromStreamType streamType = %{public}d not supported", streamType);
            return streamType;
    }
}

int32_t AudioServiceClient::SetStreamRenderRate(AudioRendererRate audioRendererRate)
{
    AUDIO_INFO_LOG("SetStreamRenderRate in");
    CHECK_AND_RETURN_RET(paStream, AUDIO_CLIENT_SUCCESS);

    uint32_t rate = sampleSpec.rate;
    switch (audioRendererRate) {
        case RENDER_RATE_NORMAL:
            break;
        case RENDER_RATE_DOUBLE:
            rate *= DOUBLE_VALUE;
            break;
        case RENDER_RATE_HALF:
            rate /= DOUBLE_VALUE;
            break;
        default:
            return AUDIO_CLIENT_INVALID_PARAMS_ERR;
    }
    renderRate = audioRendererRate;

    pa_threaded_mainloop_lock(mainLoop);
    pa_operation *operation = pa_stream_update_sample_rate(paStream, rate, nullptr, nullptr);
    if (operation != nullptr) {
        pa_operation_unref(operation);
    } else {
        AUDIO_WARNING_LOG("SetStreamRenderRate: operation is nullptr");
    }
    pa_threaded_mainloop_unlock(mainLoop);

    return AUDIO_CLIENT_SUCCESS;
}

int32_t AudioServiceClient::SetRendererSamplingRate(uint32_t sampleRate)
{
    AUDIO_INFO_LOG("SetStreamRendererSamplingRate %{public}d", sampleRate);
    CHECK_AND_RETURN_RET(paStream, AUDIO_CLIENT_SUCCESS);

    if (sampleRate <= 0) {
        return AUDIO_CLIENT_INVALID_PARAMS_ERR;
    }
    rendererSampleRate = sampleRate;

    pa_threaded_mainloop_lock(mainLoop);
    pa_operation *operation = pa_stream_update_sample_rate(paStream, sampleRate, nullptr, nullptr);
    pa_operation_unref(operation);
    pa_threaded_mainloop_unlock(mainLoop);

    return AUDIO_CLIENT_SUCCESS;
}

uint32_t AudioServiceClient::GetRendererSamplingRate()
{
    if (rendererSampleRate == 0) {
        return sampleSpec.rate;
    }
    return rendererSampleRate;
}

AudioRendererRate AudioServiceClient::GetStreamRenderRate()
{
    return renderRate;
}

void AudioServiceClient::SaveStreamCallback(const std::weak_ptr<AudioStreamCallback> &callback)
{
    streamCallback_ = callback;

    if (state_ != PREPARED) {
        return;
    }

    std::shared_ptr<AudioStreamCallback> streamCb = streamCallback_.lock();
    if (streamCb != nullptr) {
        streamCb->OnStateChange(PREPARED);
    }
}

int32_t AudioServiceClient::SetStreamLowPowerVolume(float powerVolumeFactor)
{
    lock_guard<mutex> lock(ctrlMutex_);
    AUDIO_INFO_LOG("SetPowerVolumeFactor volume: %{public}f", powerVolumeFactor);

    CHECK_AND_RETURN_RET_LOG(context != nullptr, AUDIO_CLIENT_ERR, "context is null");

    /* Validate and return INVALID_PARAMS error */
    CHECK_AND_RETURN_RET_LOG((powerVolumeFactor >= MIN_STREAM_VOLUME_LEVEL) &&
        (powerVolumeFactor <= MAX_STREAM_VOLUME_LEVEL),
        AUDIO_CLIENT_INVALID_PARAMS_ERR, "Invalid Power Volume Set!");

    pa_threaded_mainloop_lock(mainLoop);

    powerVolumeFactor_ = powerVolumeFactor;
    pa_proplist *propList = pa_proplist_new();
    if (propList == nullptr) {
        AUDIO_ERR_LOG("pa_proplist_new failed");
        pa_threaded_mainloop_unlock(mainLoop);
        return AUDIO_CLIENT_ERR;
    }

    pa_proplist_sets(propList, "stream.powerVolumeFactor", std::to_string(powerVolumeFactor_).c_str());
    pa_operation *updatePropOperation = pa_stream_proplist_update(paStream, PA_UPDATE_REPLACE, propList,
        nullptr, nullptr);
    pa_proplist_free(propList);
    pa_operation_unref(updatePropOperation);

    if (audioSystemManager_ == nullptr) {
        AUDIO_ERR_LOG("System manager instance is null");
        pa_threaded_mainloop_unlock(mainLoop);
        return AUDIO_CLIENT_ERR;
    }

    if (!streamInfoUpdated_) {
        uint32_t idx = pa_stream_get_index(paStream);
        pa_operation *operation = pa_context_get_sink_input_info(context, idx, AudioServiceClient::GetSinkInputInfoCb,
            reinterpret_cast<void *>(this));
        if (operation == nullptr) {
            AUDIO_ERR_LOG("pa_context_get_sink_input_info_list returned null");
            pa_threaded_mainloop_unlock(mainLoop);
            return AUDIO_CLIENT_ERR;
        }

        while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
            pa_threaded_mainloop_wait(mainLoop);
        }

        pa_operation_unref(operation);
    } else {
        SetPaVolume(*this);
    }

    pa_threaded_mainloop_unlock(mainLoop);

    return AUDIO_CLIENT_SUCCESS;
}

float AudioServiceClient::GetStreamLowPowerVolume()
{
    return powerVolumeFactor_;
}

float AudioServiceClient::GetSingleStreamVol()
{
    int32_t systemVolumeLevel = audioSystemManager_->GetVolume(static_cast<AudioVolumeType>(streamType_));
    DeviceType deviceType = audioSystemManager_->GetActiveOutputDevice();
    float systemVolumeDb = AudioPolicyManager::GetInstance().GetSystemVolumeInDb(streamType_,
        systemVolumeLevel, deviceType);
    return systemVolumeDb * volumeFactor_ * powerVolumeFactor_;
}

// OnRenderMarkReach by eventHandler
void AudioServiceClient::SendRenderMarkReachedRequestEvent(uint64_t mFrameMarkPosition)
{
    lock_guard<mutex> runnerlock(runnerMutex_);
    if (runnerReleased_) {
        AUDIO_WARNING_LOG("SendRenderMarkReachedRequestEvent runner released");
        return;
    }
    SendEvent(AppExecFwk::InnerEvent::Get(RENDERER_MARK_REACHED_REQUEST, mFrameMarkPosition));
}

void AudioServiceClient::HandleRenderMarkReachedEvent(uint64_t mFrameMarkPosition)
{
    if (mRenderPositionCb) {
        mRenderPositionCb->OnMarkReached(mFrameMarkPosition);
    }
}

// SetRenderMarkReached by eventHandler
void AudioServiceClient::SendSetRenderMarkReachedRequestEvent(
    const std::shared_ptr<RendererPositionCallback> &callback)
{
    lock_guard<mutex> runnerlock(runnerMutex_);
    if (runnerReleased_) {
        AUDIO_WARNING_LOG("SendSetRenderMarkReachedRequestEvent runner released");
        return;
    }
    SendSyncEvent(AppExecFwk::InnerEvent::Get(SET_RENDERER_MARK_REACHED_REQUEST, callback));
}

void AudioServiceClient::HandleSetRenderMarkReachedEvent(const std::shared_ptr<RendererPositionCallback> &callback)
{
    mRenderPositionCb = callback;
}

// UnsetRenderMarkReach by eventHandler
void AudioServiceClient::SendUnsetRenderMarkReachedRequestEvent()
{
    lock_guard<mutex> runnerlock(runnerMutex_);
    if (runnerReleased_) {
        AUDIO_WARNING_LOG("SendUnsetRenderMarkReachedRequestEvent runner released");
        return;
    }
    SendSyncEvent(AppExecFwk::InnerEvent::Get(UNSET_RENDERER_MARK_REACHED_REQUEST));
}

void AudioServiceClient::HandleUnsetRenderMarkReachedEvent()
{
    mRenderPositionCb = nullptr;
}

// OnRenderPeriodReach by eventHandler
void AudioServiceClient::SendRenderPeriodReachedRequestEvent(uint64_t mFramePeriodNumber)
{
    lock_guard<mutex> runnerlock(runnerMutex_);
    if (runnerReleased_) {
        AUDIO_WARNING_LOG("SendRenderPeriodReachedRequestEvent runner released");
        return;
    }
    SendEvent(AppExecFwk::InnerEvent::Get(RENDERER_PERIOD_REACHED_REQUEST, mFramePeriodNumber));
}

void AudioServiceClient::HandleRenderPeriodReachedEvent(uint64_t mFramePeriodNumber)
{
    if (mRenderPeriodPositionCb) {
        mRenderPeriodPositionCb->OnPeriodReached(mFramePeriodNumber);
    }
}

// SetRenderPeriodReach by eventHandler
void AudioServiceClient::SendSetRenderPeriodReachedRequestEvent(
    const std::shared_ptr<RendererPeriodPositionCallback> &callback)
{
    lock_guard<mutex> runnerlock(runnerMutex_);
    if (runnerReleased_) {
        AUDIO_WARNING_LOG("SendSetRenderPeriodReachedRequestEvent runner released");
        return;
    }
    SendSyncEvent(AppExecFwk::InnerEvent::Get(SET_RENDERER_PERIOD_REACHED_REQUEST, callback));
}

void AudioServiceClient::HandleSetRenderPeriodReachedEvent(
    const std::shared_ptr<RendererPeriodPositionCallback> &callback)
{
    if (callback) {
        mRenderPeriodPositionCb = callback;
    }
}

// UnsetRenderPeriodReach by eventHandler
void AudioServiceClient::SendUnsetRenderPeriodReachedRequestEvent()
{
    lock_guard<mutex> runnerlock(runnerMutex_);
    if (runnerReleased_) {
        AUDIO_WARNING_LOG("SendUnsetRenderPeriodReachedRequestEvent runner released");
        return;
    }
    SendSyncEvent(AppExecFwk::InnerEvent::Get(UNSET_RENDERER_PERIOD_REACHED_REQUEST));
}

void AudioServiceClient::HandleUnsetRenderPeriodReachedEvent()
{
    mRenderPeriodPositionCb = nullptr;
}

// OnCapturerMarkReach by eventHandler
void AudioServiceClient::SendCapturerMarkReachedRequestEvent(uint64_t mFrameMarkPosition)
{
    lock_guard<mutex> runnerlock(runnerMutex_);
    if (runnerReleased_) {
        AUDIO_WARNING_LOG("SendCapturerMarkReachedRequestEvent runner released");
        return;
    }
    SendEvent(AppExecFwk::InnerEvent::Get(CAPTURER_MARK_REACHED_REQUEST, mFrameMarkPosition));
}

void AudioServiceClient::HandleCapturerMarkReachedEvent(uint64_t mFrameMarkPosition)
{
    if (mCapturePositionCb) {
        mCapturePositionCb->OnMarkReached(mFrameMarkPosition);
    }
}

// SetCapturerMarkReach by eventHandler
void AudioServiceClient::SendSetCapturerMarkReachedRequestEvent(
    const std::shared_ptr<CapturerPositionCallback> &callback)
{
    lock_guard<mutex> runnerlock(runnerMutex_);
    if (runnerReleased_) {
        AUDIO_WARNING_LOG("SendSetCapturerMarkReachedRequestEvent runner released");
        return;
    }
    SendSyncEvent(AppExecFwk::InnerEvent::Get(SET_CAPTURER_MARK_REACHED_REQUEST, callback));
}

void AudioServiceClient::HandleSetCapturerMarkReachedEvent(const std::shared_ptr<CapturerPositionCallback> &callback)
{
    mCapturePositionCb = callback;
}

// UnsetCapturerMarkReach by eventHandler
void AudioServiceClient::SendUnsetCapturerMarkReachedRequestEvent()
{
    lock_guard<mutex> runnerlock(runnerMutex_);
    if (runnerReleased_) {
        AUDIO_WARNING_LOG("runner released");
        return;
    }
    SendSyncEvent(AppExecFwk::InnerEvent::Get(UNSET_CAPTURER_MARK_REACHED_REQUEST));
}

void AudioServiceClient::HandleUnsetCapturerMarkReachedEvent()
{
    mCapturePositionCb = nullptr;
}

// OnCapturerPeriodReach by eventHandler
void AudioServiceClient::SendCapturerPeriodReachedRequestEvent(uint64_t mFramePeriodNumber)
{
    lock_guard<mutex> runnerlock(runnerMutex_);
    if (runnerReleased_) {
        AUDIO_WARNING_LOG("runner released");
        return;
    }
    SendEvent(AppExecFwk::InnerEvent::Get(CAPTURER_PERIOD_REACHED_REQUEST, mFramePeriodNumber));
}

void AudioServiceClient::HandleCapturerPeriodReachedEvent(uint64_t mFramePeriodNumber)
{
    if (mCapturePeriodPositionCb) {
        mCapturePeriodPositionCb->OnPeriodReached(mFramePeriodNumber);
    }
}

// SetCapturerPeriodReach by eventHandler
void AudioServiceClient::SendSetCapturerPeriodReachedRequestEvent(
    const std::shared_ptr<CapturerPeriodPositionCallback> &callback)
{
    lock_guard<mutex> runnerlock(runnerMutex_);
    if (runnerReleased_) {
        AUDIO_WARNING_LOG("runner released");
        return;
    }
    SendSyncEvent(AppExecFwk::InnerEvent::Get(SET_CAPTURER_PERIOD_REACHED_REQUEST, callback));
}

void AudioServiceClient::HandleSetCapturerPeriodReachedEvent(
    const std::shared_ptr<CapturerPeriodPositionCallback> &callback)
{
    mCapturePeriodPositionCb = callback;
}

// UnsetCapturerPeriodReach by eventHandler
void AudioServiceClient::SendUnsetCapturerPeriodReachedRequestEvent()
{
    lock_guard<mutex> runnerlock(runnerMutex_);
    if (runnerReleased_) {
        AUDIO_WARNING_LOG("runner released");
        return;
    }
    SendSyncEvent(AppExecFwk::InnerEvent::Get(UNSET_CAPTURER_PERIOD_REACHED_REQUEST));
}

void AudioServiceClient::HandleUnsetCapturerPeriodReachedEvent()
{
    mCapturePeriodPositionCb = nullptr;
}

int32_t AudioServiceClient::SetRendererWriteCallback(const std::shared_ptr<AudioRendererWriteCallback> &callback)
{
    std::lock_guard<std::mutex> lockSet(writeCallbackMutex_);
    CHECK_AND_RETURN_RET_LOG(callback, ERR_INVALID_PARAM, "SetRendererWriteCallback callback is nullptr");
    writeCallback_ = callback;
    return SUCCESS;
}

int32_t AudioServiceClient::SetCapturerReadCallback(const std::shared_ptr<AudioCapturerReadCallback> &callback)
{
    CHECK_AND_RETURN_RET_LOG(callback, ERR_INVALID_PARAM, "SetCapturerReadCallback callback is nullptr");
    readCallback_ = callback;
    return SUCCESS;
}

void AudioServiceClient::CheckOffloadBreakWaitWrite()
{
    if (offloadEnable_) {
        breakingWritePa_ = true;
        pa_threaded_mainloop_signal(mainLoop, 0);
    }
}

void AudioServiceClient::SendWriteBufferRequestEvent()
{
    // send write event to handler
    lock_guard<mutex> runnerlock(runnerMutex_);
    if (runnerReleased_) {
        AUDIO_WARNING_LOG("SendWriteBufferRequestEvent after runner released");
        return;
    }
    SendEvent(AppExecFwk::InnerEvent::Get(WRITE_BUFFER_REQUEST));
}

void AudioServiceClient::SendReadBufferRequestEvent()
{
    // send write event to handler
    lock_guard<mutex> runnerlock(runnerMutex_);
    if (runnerReleased_) {
        AUDIO_WARNING_LOG("SendReadBufferRequestEvent after runner released");
        return;
    }
    SendEvent(AppExecFwk::InnerEvent::Get(READ_BUFFER_REQUEST));
}

void AudioServiceClient::HandleWriteRequestEvent()
{
    std::lock_guard<std::mutex> lockSet(writeCallbackMutex_);
    // do callback to application
    if (writeCallback_) {
        size_t requestSize;
        GetMinimumBufferSize(requestSize);
        writeCallback_->OnWriteData(requestSize);
    }
}

void AudioServiceClient::HandleReadRequestEvent()
{
    // do callback to application
    if (readCallback_) {
        size_t requestSize;
        GetMinimumBufferSize(requestSize);
        readCallback_->OnReadData(requestSize);
    }
}

void AudioServiceClient::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    uint32_t eventId = event->GetInnerEventId();
    uint64_t mFrameMarkPosition;
    uint64_t mFramePeriodNumber;
    std::shared_ptr<RendererPositionCallback> renderPositionCb;
    std::shared_ptr<RendererPeriodPositionCallback> renderPeriodPositionCb;
    std::shared_ptr<CapturerPositionCallback> capturePositionCb;
    std::shared_ptr<CapturerPeriodPositionCallback> capturePeriodPositionCb;

    switch (eventId) {
        case WRITE_BUFFER_REQUEST:
            HandleWriteRequestEvent();
            break;
        case READ_BUFFER_REQUEST:
            HandleReadRequestEvent();
            break;

        // RenderMarkReach
        case RENDERER_MARK_REACHED_REQUEST:
            mFrameMarkPosition = event->GetParam();
            HandleRenderMarkReachedEvent(mFrameMarkPosition);
            break;
        case SET_RENDERER_MARK_REACHED_REQUEST:
            renderPositionCb = event->GetSharedObject<RendererPositionCallback>();
            HandleSetRenderMarkReachedEvent(renderPositionCb);
            break;
        case UNSET_RENDERER_MARK_REACHED_REQUEST:
            HandleUnsetRenderMarkReachedEvent();
            break;

        // RenderPeriodReach
        case RENDERER_PERIOD_REACHED_REQUEST:
            mFramePeriodNumber = event->GetParam();
            HandleRenderPeriodReachedEvent(mFramePeriodNumber);
            break;
        case SET_RENDERER_PERIOD_REACHED_REQUEST:
            renderPeriodPositionCb = event->GetSharedObject<RendererPeriodPositionCallback>();
            HandleSetRenderPeriodReachedEvent(renderPeriodPositionCb);
            break;
        case UNSET_RENDERER_PERIOD_REACHED_REQUEST:
            HandleUnsetRenderPeriodReachedEvent();
            break;

        // CapturerMarkReach
        case CAPTURER_MARK_REACHED_REQUEST:
            mFrameMarkPosition = event->GetParam();
            HandleCapturerMarkReachedEvent(mFrameMarkPosition);
            break;
        case SET_CAPTURER_MARK_REACHED_REQUEST:
            capturePositionCb = event->GetSharedObject<CapturerPositionCallback>();
            HandleSetCapturerMarkReachedEvent(capturePositionCb);
            break;
        case UNSET_CAPTURER_MARK_REACHED_REQUEST:
            HandleUnsetCapturerMarkReachedEvent();
            break;

        // CapturerPeriodReach
        case CAPTURER_PERIOD_REACHED_REQUEST:
            mFramePeriodNumber = event->GetParam();
            HandleCapturerPeriodReachedEvent(mFramePeriodNumber);
            break;
        case SET_CAPTURER_PERIOD_REACHED_REQUEST:
            capturePeriodPositionCb = event->GetSharedObject<CapturerPeriodPositionCallback>();
            HandleSetCapturerPeriodReachedEvent(capturePeriodPositionCb);
            break;
        case UNSET_CAPTURER_PERIOD_REACHED_REQUEST:
            HandleUnsetCapturerPeriodReachedEvent();
            break;

        default:
            break;
    }
}

const std::string AudioServiceClient::GetEffectModeName(AudioEffectMode effectMode)
{
    std::string name;
    switch (effectMode) {
        case EFFECT_NONE:
            name = "EFFECT_NONE";
            break;
        default:
            name = "EFFECT_DEFAULT";
    }

    const std::string modeName = name;
    return modeName;
}

AudioEffectMode AudioServiceClient::GetStreamAudioEffectMode()
{
    return effectMode;
}

int64_t AudioServiceClient::GetStreamFramesWritten()
{
    CHECK_AND_RETURN_RET_LOG(mFrameSize != 0, ERROR, "Error frame size");
    return mTotalBytesWritten / mFrameSize;
}

int64_t AudioServiceClient::GetStreamFramesRead()
{
    CHECK_AND_RETURN_RET_LOG(mFrameSize != 0, ERROR, "Error frame size");
    return mTotalBytesRead / mFrameSize;
}

int32_t AudioServiceClient::SetStreamAudioEffectMode(AudioEffectMode audioEffectMode)
{
    AUDIO_INFO_LOG("Mode: %{public}d", audioEffectMode);

    int32_t ret = CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR);
    CHECK_AND_RETURN_RET_LOG(ret >= 0, AUDIO_CLIENT_PA_ERR,
        "set stream audio effect mode: invalid stream state");

    pa_threaded_mainloop_lock(mainLoop);

    effectMode = audioEffectMode;

    const std::string effectModeName = GetEffectModeName(audioEffectMode);

    pa_proplist *propList = pa_proplist_new();
    if (propList == nullptr) {
        AUDIO_ERR_LOG("pa_proplist_new failed");
        pa_threaded_mainloop_unlock(mainLoop);
        return AUDIO_CLIENT_ERR;
    }

    pa_proplist_sets(propList, "scene.type", effectSceneName.c_str());
    pa_proplist_sets(propList, "scene.mode", effectModeName.c_str());
    pa_operation *updatePropOperation = pa_stream_proplist_update(paStream, PA_UPDATE_REPLACE, propList,
        nullptr, nullptr);
    pa_proplist_free(propList);
    pa_operation_unref(updatePropOperation);

    pa_threaded_mainloop_unlock(mainLoop);

    return AUDIO_CLIENT_SUCCESS;
}

void AudioServiceClient::SetStreamInnerCapturerState(bool isInnerCapturer)
{
    AUDIO_DEBUG_LOG("SetInnerCapturerState: %{public}d", isInnerCapturer);
    isInnerCapturerStream_ = isInnerCapturer;
}

void AudioServiceClient::SetStreamPrivacyType(AudioPrivacyType privacyType)
{
    AUDIO_DEBUG_LOG("SetInnerCapturerState: %{public}d", privacyType);
    mPrivacyType = privacyType;
}

void AudioServiceClient::SetStreamUsage(StreamUsage usage)
{
    AUDIO_DEBUG_LOG("SetStreamUsage: %{public}d", usage);
    mStreamUsage = usage;
}

void AudioServiceClient::SetWakeupCapturerState(bool isWakeupCapturer)
{
    AUDIO_DEBUG_LOG("SetWakeupCapturerState: %{public}d", isWakeupCapturer);
    isWakeupCapturerStream_ = isWakeupCapturer;
}

void AudioServiceClient::SetCapturerSource(int capturerSource)
{
    AUDIO_DEBUG_LOG("SetCapturerSource: %{public}d", capturerSource);
    capturerSource_ = capturerSource;
}

uint32_t AudioServiceClient::ConvertChLayoutToPaChMap(const uint64_t &channelLayout, pa_channel_map &paMap)
{
    uint32_t channelNum = 0;
    uint64_t mode = (channelLayout & CH_MODE_MASK) >> CH_MODE_OFFSET;
    switch (mode) {
        case 0: {
            for (auto bit = chSetToPaPositionMap.begin(); bit != chSetToPaPositionMap.end(); ++bit) {
                if ((channelLayout & (bit->first)) != 0) {
                    paMap.map[channelNum++] = bit->second;
                }
            }
            break;
        }
        case 1: {
            uint64_t order = (channelLayout & CH_HOA_ORDNUM_MASK) >> CH_HOA_ORDNUM_OFFSET;
            channelNum = (order + 1) * (order + 1);
            for (uint32_t i = 0; i < channelNum; ++i) {
                paMap.map[i] = chSetToPaPositionMap[FRONT_LEFT];
            }
            break;
        }
        default:
            channelNum = 0;
            break;
    }
    return channelNum;
}

int32_t AudioServiceClient::RegisterSpatializationStateEventListener()
{
    if (firstSpatializationRegistered_) {
        firstSpatializationRegistered_ = false;
    } else {
        UnregisterSpatializationStateEventListener(spatializationRegisteredSessionID_);
    }

    if (!spatializationStateChangeCallback_) {
        spatializationStateChangeCallback_ = std::make_shared<AudioSpatializationStateChangeCallbackImpl>();
        CHECK_AND_RETURN_RET_LOG(spatializationStateChangeCallback_, ERROR, "Memory Allocation Failed !!");
    }
    spatializationStateChangeCallback_->setAudioServiceClientObj(this);

    int32_t ret = AudioPolicyManager::GetInstance().RegisterSpatializationStateEventListener(
        sessionID_, mStreamUsage, spatializationStateChangeCallback_);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "RegisterSpatializationStateEventListener failed");
    spatializationRegisteredSessionID_ = sessionID_;

    return SUCCESS;
}

int32_t AudioServiceClient::UnregisterSpatializationStateEventListener(uint32_t sessionID)
{
    int32_t ret = AudioPolicyManager::GetInstance().UnregisterSpatializationStateEventListener(sessionID);
    CHECK_AND_RETURN_RET_LOG(ret == 0, ERROR, "UnregisterSpatializationStateEventListener failed");
    return SUCCESS;
}

void AudioServiceClient::OnSpatializationStateChange(const AudioSpatializationState &spatializationState)
{
    int32_t res = CheckPaStatusIfinvalid(mainLoop, context, paStream, AUDIO_CLIENT_PA_ERR);
    CHECK_AND_RETURN_LOG(res >= 0, "OnSpatializationStateChange: invalid stream state");
    spatializationEnabled_ = std::to_string(spatializationState.spatializationEnabled);
    headTrackingEnabled_ = std::to_string(spatializationState.headTrackingEnabled);
    CHECK_AND_RETURN_LOG(context != nullptr, "context is null");

    pa_threaded_mainloop_lock(mainLoop);

    pa_proplist *propList = pa_proplist_new();
    if (propList == nullptr) {
        AUDIO_ERR_LOG("pa_proplist_new failed");
        pa_threaded_mainloop_unlock(mainLoop);
        return;
    }

    pa_proplist_sets(propList, "spatialization.enabled", spatializationEnabled_.c_str());
    pa_proplist_sets(propList, "headtracking.enabled", headTrackingEnabled_.c_str());
    pa_operation *updatePropOperation = pa_stream_proplist_update(paStream, PA_UPDATE_REPLACE, propList,
        nullptr, nullptr);
    pa_proplist_free(propList);
    pa_operation_unref(updatePropOperation);

    pa_threaded_mainloop_unlock(mainLoop);
}

int32_t AudioServiceClient::SetStreamSpeed(float speed)
{
    speed_ = speed;
    return SUCCESS;
}

float AudioServiceClient::GetStreamSpeed()
{
    return speed_;
}

uint32_t AudioServiceClient::GetAppTokenId() const
{
    return appTokenId_;
}

AudioSpatializationStateChangeCallbackImpl::AudioSpatializationStateChangeCallbackImpl()
{
    AUDIO_INFO_LOG("AudioSpatializationStateChangeCallbackImpl instance create");
}

AudioSpatializationStateChangeCallbackImpl::~AudioSpatializationStateChangeCallbackImpl()
{
    AUDIO_INFO_LOG("AudioSpatializationStateChangeCallbackImpl instance destory");
}

void AudioSpatializationStateChangeCallbackImpl::setAudioServiceClientObj(AudioServiceClient *serviceClientObj)
{
    serviceClient_ = serviceClientObj;
}

void AudioSpatializationStateChangeCallbackImpl::OnSpatializationStateChange(
    const AudioSpatializationState &spatializationState)
{
    if (serviceClient_ == nullptr) {
        return;
    }
    serviceClient_->OnSpatializationStateChange(spatializationState);
}
} // namespace AudioStandard
} // namespace OHOS

