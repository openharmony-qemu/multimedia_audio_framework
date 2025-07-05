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
#ifndef LOG_TAG
#define LOG_TAG "HdiSource"
#endif

#include <config.h>
#include <pulse/rtclock.h>
#include <pulse/timeval.h>
#include <pulse/util.h>
#include <pulse/xmalloc.h>
#include <pulsecore/core.h>
#include <pulsecore/log.h>
#include <pulsecore/memchunk.h>
#include <pulsecore/modargs.h>
#include <pulsecore/module.h>
#include <pulsecore/rtpoll.h>
#include <pulsecore/thread-mq.h>
#include <pulsecore/thread.h>
#include <pulsecore/mix.h>
#include <pulsecore/memblockq.h>
#include <pulsecore/source.h>
#include <pulsecore/source-output.h>

#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#include "source_userdata.h"
#include "securec.h"
#include "audio_hdi_log.h"
#include "audio_schedule.h"
#include "audio_source_type.h"
#include "common/hdi_adapter_info.h"
#include "source/source_intf.h"
#include "v5_0/audio_types.h"
#include "v5_0/iaudio_manager.h"
#include "audio_enhance_chain_adapter.h"
#include "audio_utils_c.h"

#define DEFAULT_SOURCE_NAME "hdi_input"
#define DEFAULT_DEVICE_CLASS "primary"
#define DEFAULT_AUDIO_DEVICE_NAME "Internal Mic"
#define DEFAULT_DEVICE_NETWORKID "LocalDevice"

#define DEFAULT_BUFFER_SIZE (1024 * 16)
#define MAX_VOLUME_VALUE 15.0
#define DEFAULT_LEFT_VOLUME MAX_VOLUME_VALUE
#define DEFAULT_RIGHT_VOLUME MAX_VOLUME_VALUE
#define MAX_LATENCY_USEC (PA_USEC_PER_SEC * 2)
#define MIN_LATENCY_USEC 500
#define AUDIO_POINT_NUM  1024
#define AUDIO_FRAME_NUM_IN_BUF 30
#define HDI_WAKEUP_BUFFER_TIME (PA_USEC_PER_SEC * 2)
#define DEVICE_TYPE_MIC 15
#define FRAME_DURATION_DEFAULT 20
#define MILLISECOND_PER_SECOND 1000
#define HDI_POST 100
#define MAX_SEND_COMMAND_LATANCY 10000
#define RTPOLL_RUN_WAKEUP_INTERVAL_USEC 500000
#define DOMAIN_ID 0xD002B89

const char *DEVICE_CLASS_REMOTE = "remote";
const char *DEVICE_CLASS_A2DP = "a2dp";
const char *ACCESSORY_SOURCE = "accessory_mic";
const int32_t SUCCESS = 0;
const int32_t ERROR = -1;

static int PaHdiCapturerInit(struct Userdata *u);
static void PaHdiCapturerExit(struct Userdata *u);

static char *GetStateInfo(pa_source_state_t state)
{
    switch (state) {
        case PA_SOURCE_INVALID_STATE:
            return "INVALID";
        case PA_SOURCE_RUNNING:
            return "RUNNING";
        case PA_SOURCE_IDLE:
            return "IDLE";
        case PA_SOURCE_SUSPENDED:
            return "SUSPENDED";
        case PA_SOURCE_INIT:
            return "INIT";
        case PA_SOURCE_UNLINKED:
            return "UNLINKED";
        default:
            return "error state";
    }
}

static uint32_t GetByteSizeByFormat(int32_t format)
{
    uint32_t byteSize = 0;
    switch (format) {
        case SAMPLE_U8:
            byteSize = BYTE_SIZE_SAMPLE_U8;
            break;
        case SAMPLE_S16:
            byteSize = BYTE_SIZE_SAMPLE_S16;
            break;
        case SAMPLE_S24:
            byteSize = BYTE_SIZE_SAMPLE_S24;
            break;
        case SAMPLE_S32:
            byteSize = BYTE_SIZE_SAMPLE_S32;
            break;
        default:
            byteSize = BYTE_SIZE_SAMPLE_S16;
            break;
    }
    return byteSize;
}

static uint64_t CalculateFrameLen(uint32_t sampleRate, uint32_t channels, int32_t format)
{
    return sampleRate * channels * GetByteSizeByFormat(format) * FRAME_DURATION_DEFAULT / MILLISECOND_PER_SECOND;
}

static struct SourceAdapterFrameDesc *AllocateFrameDesc(char *frame, uint64_t frameLen)
{
    struct SourceAdapterFrameDesc *fdesc = (struct SourceAdapterFrameDesc *)calloc(1,
        sizeof(struct SourceAdapterFrameDesc));
    if (fdesc != NULL) {
        fdesc->frame = frame;
        fdesc->frameLen = frameLen;
    }

    return fdesc;
}

static void FreeFrameDesc(struct SourceAdapterFrameDesc *fdesc)
{
    if (fdesc != NULL) {
        // frame in desc is allocated outside, do not free here
        free(fdesc);
    }
}

static void InitAuxCapture(struct Userdata *u)
{
    if (u->sourceAdapterEc != NULL) {
        u->sourceAdapterEc->SourceAdapterInit(u->sourceAdapterEc, u->sourceAdapterEc->attr);
    }
    if (u->sourceAdapterMicRef != NULL) {
        u->sourceAdapterMicRef->SourceAdapterInit(u->sourceAdapterMicRef, u->sourceAdapterMicRef->attr);
    }
}

static void DeinitAuxCapture(struct Userdata *u)
{
    if (u->sourceAdapterEc != NULL) {
        u->sourceAdapterEc->SourceAdapterDeInit(u->sourceAdapterEc);
    }
    if (u->sourceAdapterMicRef != NULL) {
        u->sourceAdapterMicRef->SourceAdapterDeInit(u->sourceAdapterMicRef);
    }
}

static void StartAuxCapture(struct Userdata *u)
{
    if (u->sourceAdapterEc != NULL) {
        u->sourceAdapterEc->SourceAdapterStart(u->sourceAdapterEc);
    }
    if (u->sourceAdapterMicRef != NULL) {
        u->sourceAdapterMicRef->SourceAdapterStart(u->sourceAdapterMicRef);
    }
}

static void StopAuxCapture(struct Userdata *u)
{
    if (u->sourceAdapterEc != NULL) {
        u->sourceAdapterEc->SourceAdapterStop(u->sourceAdapterEc);
    }
    if (u->sourceAdapterMicRef != NULL) {
        u->sourceAdapterMicRef->SourceAdapterStop(u->sourceAdapterMicRef);
    }
}

static void FreeSceneMapsAndResampler(struct Userdata *u)
{
    if (u->sceneToCountMap) {
        pa_hashmap_free(u->sceneToCountMap);
    }
    if (u->sceneToPreResamplerMap) {
        pa_hashmap_free(u->sceneToPreResamplerMap);
    }
    if (u->sceneToEcResamplerMap) {
        pa_hashmap_free(u->sceneToEcResamplerMap);
    }
    if (u->sceneToMicRefResamplerMap) {
        pa_hashmap_free(u->sceneToMicRefResamplerMap);
    }

    if (u->defaultSceneResampler) {
        pa_resampler_free(u->defaultSceneResampler);
    }
}

static void FreeThread(struct Userdata *u)
{
    if (u->threadCap) {
        pa_thread_free(u->threadCap);
    }

    if (u->thread) {
        pa_asyncmsgq_send(u->threadMq.inq, NULL, PA_MESSAGE_SHUTDOWN, NULL, 0, NULL);
        pa_thread_free(u->thread);
    }

    if (u->CaptureMq) {
        pa_memchunk chunk;
        int32_t code = 0;
        int32_t missedMsgqNum = 0;
        while (pa_asyncmsgq_get(u->CaptureMq, NULL, &code, NULL, NULL, &chunk, 0) == 0) {
            pa_memblock_unref(chunk.memblock);
            pa_asyncmsgq_done(u->CaptureMq, 0);
            missedMsgqNum++;
        }
        if (missedMsgqNum > 0) {
            AUDIO_ERR_LOG("OS_ProcessCapData missed message num: %{public}u", missedMsgqNum);
        }
        pa_asyncmsgq_unref(u->CaptureMq);
    } else {
        AUDIO_ERR_LOG("CaptureMq is null");
    }

    pa_thread_mq_done(&u->threadMq);
    if (u->eventFd != 0) {
        fdsan_close_with_tag(u->eventFd, DOMAIN_ID);
        u->eventFd = 0;
    }
    if (u->rtpollItem) {
        pa_rtpoll_item_free(u->rtpollItem);
        u->rtpollItem = NULL;
    }
}

static void UserdataFree(struct Userdata *u)
{
    if (u == NULL) {
        AUDIO_INFO_LOG("Userdata is null, free done");
        return;
    }
    pa_atomic_store(&u->quitCaptureFlag, 1);
    if (u->source) {
        pa_source_unlink(u->source);
    }

    FreeThread(u);

    if (u->source) {
        pa_source_unref(u->source);
    }

    if (u->rtpoll) {
        pa_rtpoll_free(u->rtpoll);
    }

    if (u->sourceAdapter) {
        u->sourceAdapter->SourceAdapterStop(u->sourceAdapter);
        u->sourceAdapter->SourceAdapterDeInit(u->sourceAdapter);
        StopAuxCapture(u);
        DeinitAuxCapture(u);
        ReleaseSourceAdapter(u->sourceAdapterEc);
        u->sourceAdapterEc = NULL;
        ReleaseSourceAdapter(u->sourceAdapterMicRef);
        u->sourceAdapterMicRef = NULL;
        ReleaseSourceAdapter(u->sourceAdapter);
        u->sourceAdapter = NULL;
    }

    if (u->bufferEc) {
        free(u->bufferEc);
        u->bufferEc = NULL;
    }

    if (u->bufferMicRef) {
        free(u->bufferMicRef);
        u->bufferMicRef = NULL;
    }

    FreeSceneMapsAndResampler(u);

    pa_xfree(u);
}

static int SourceProcessMsg(pa_msgobject *o, int code, void *data, int64_t offset, pa_memchunk *chunk)
{
    AUTO_CTRACE("hdi_source::SourceProcessMsg code: %d", code);
    struct Userdata *u = PA_SOURCE(o)->userdata;
    CHECK_AND_RETURN_RET_LOG(u != NULL, 0, "userdata is null");
    if (code == PA_SOURCE_MESSAGE_GET_LATENCY) {
        pa_usec_t now;
        now = pa_rtclock_now();
        *((int64_t*)data) = (int64_t)now - (int64_t)u->timestamp;
        return 0;
    }
    AUDIO_DEBUG_LOG("SourceProcessMsg default case");
    return pa_source_process_msg(o, code, data, offset, chunk);
}

static void SendInitCommandToAlgo(void)
{
    pa_usec_t now = pa_rtclock_now();
    int32_t ret = EnhanceChainManagerSendInitCommand();
    CHECK_AND_RETURN_LOG(ret == SUCCESS, "send init command failed");
    pa_usec_t cost = pa_rtclock_now() - now;
    if (cost > MAX_SEND_COMMAND_LATANCY) { // send command cost more than 10 ms
        AUDIO_WARNING_LOG("send int command cost time:%{public}" PRIu64, cost);
    }
}

/* Called from the IO thread. */
static int SourceSetStateInIoThreadCb(pa_source *s, pa_source_state_t newState,
    pa_suspend_cause_t newSuspendCause)
{
    CHECK_AND_RETURN_RET_LOG(s != NULL, 0, "source is null");
    struct Userdata *u = s->userdata;
    CHECK_AND_RETURN_RET_LOG(u != NULL, 0, "userdata is null");
    AUDIO_INFO_LOG("Source[%{public}s] state change:[%{public}s]-->[%{public}s]",
        u->sourceAdapter->deviceClass, GetStateInfo(s->thread_info.state), GetStateInfo(newState));

    if ((s->thread_info.state == PA_SOURCE_SUSPENDED || s->thread_info.state == PA_SOURCE_INIT) &&
        PA_SOURCE_IS_OPENED(newState)) {
        u->timestamp = pa_rtclock_now();
        if (newState == PA_SOURCE_RUNNING && !u->isCapturerStarted) {
            if (u->sourceAdapter->SourceAdapterStart(u->sourceAdapter)) {
                AUDIO_ERR_LOG("HDI capturer start failed");
                return -PA_ERR_IO;
            }
            StartAuxCapture(u);
            u->isCapturerStarted = true;
            AUDIO_DEBUG_LOG("Successfully started HDI capturer");
        }
    } else if (s->thread_info.state == PA_SOURCE_IDLE) {
        if (newState == PA_SOURCE_SUSPENDED) {
            if (u->isCapturerStarted) {
                u->sourceAdapter->SourceAdapterStop(u->sourceAdapter);
                u->isCapturerStarted = false;
                AUDIO_DEBUG_LOG("Stopped HDI capturer");
                StopAuxCapture(u);
                SendInitCommandToAlgo();
            }
        } else if (newState == PA_SOURCE_RUNNING && !u->isCapturerStarted) {
            AUDIO_DEBUG_LOG("Idle to Running starting HDI capturing device");
            if (u->sourceAdapter->SourceAdapterStart(u->sourceAdapter)) {
                AUDIO_ERR_LOG("Idle to Running HDI capturer start failed");
                return -PA_ERR_IO;
            }
            StartAuxCapture(u);
            u->isCapturerStarted = true;
            AUDIO_DEBUG_LOG("Idle to Running: Successfully reinitialized HDI renderer");
        }
    }

    return 0;
}

static void PushData(pa_source_output *sourceOutput, pa_memchunk *chunk)
{
    CHECK_AND_RETURN_LOG(sourceOutput != NULL, "sourceOutput is null");
    pa_source_output_assert_ref(sourceOutput);
    pa_source_output_assert_io_context(sourceOutput);
    CHECK_AND_RETURN_LOG(chunk != NULL, "chunk is null");
    AUDIO_DEBUG_LOG("PushData chunk length: %{public}zu", chunk->length);

    if (!sourceOutput->thread_info.direct_on_input) {
        pa_source_output_push(sourceOutput, chunk);
    }
}

static void PostSourceData(pa_source *source, pa_source_output *sourceOutput, pa_memchunk *chunk)
{
    AUTO_CTRACE("PostSourceData");
    CHECK_AND_RETURN_LOG(source != NULL, "source is null");
    pa_source_assert_ref(source);
    pa_source_assert_io_context(source);
    pa_assert(PA_SOURCE_IS_LINKED(source->thread_info.state));
    CHECK_AND_RETURN_LOG(chunk != NULL, "chunk is null");

    if (source->thread_info.state == PA_SOURCE_SUSPENDED) {
        return;
    }

    if (source->thread_info.soft_muted || !pa_cvolume_is_norm(&source->thread_info.soft_volume)) {
        pa_memchunk vchunk = *chunk;
        pa_memblock_ref(vchunk.memblock);
        pa_memchunk_make_writable(&vchunk, 0);
        if (source->thread_info.soft_muted || pa_cvolume_is_muted(&source->thread_info.soft_volume)) {
            pa_silence_memchunk(&vchunk, &source->sample_spec);
        } else {
            pa_volume_memchunk(&vchunk, &source->sample_spec, &source->thread_info.soft_volume);
        }
        PushData(sourceOutput, &vchunk);
        pa_memblock_unref(vchunk.memblock);
    } else {
        PushData(sourceOutput, chunk);
    }
}

static void EnhanceProcess(const uint64_t sceneKeyCode, pa_memchunk *chunk)
{
    CHECK_AND_RETURN_LOG(chunk != NULL, "chunk is null");
    void *src = pa_memblock_acquire_chunk(chunk);
    AUDIO_DEBUG_LOG("EnhanceProcess chunk length: %{public}zu sceneKey: %{public}" PRIu64,
        chunk->length, sceneKeyCode);
    pa_memblock_release(chunk->memblock);

    if (CopyToEnhanceBufferAdapter(src, chunk->length) != 0) {
        return;
    }
    if (EnhanceChainManagerProcess(sceneKeyCode, chunk->length) != 0) {
        return;
    }
    void *dst = pa_memblock_acquire_chunk(chunk);
    CopyFromEnhanceBufferAdapter(dst, chunk->length);
    pa_memblock_release(chunk->memblock);
}

static void EnhanceProcessDefault(const uint32_t captureId, pa_memchunk *chunk)
{
    CHECK_AND_RETURN_LOG(chunk != NULL, "chunk is null");
    void *src = pa_memblock_acquire_chunk(chunk);
    AUDIO_DEBUG_LOG("EnhanceProcessDefault chunk length: %{public}zu captureId: %{public}u", chunk->length, captureId);
    pa_memblock_release(chunk->memblock);

    if (CopyToEnhanceBufferAdapter(src, chunk->length) != 0) {
        return;
    }
    if (EnhanceChainManagerProcessDefault(captureId, chunk->length) != 0) {
        return;
    }
    void *dst = pa_memblock_acquire_chunk(chunk);
    CopyFromEnhanceBufferAdapter(dst, chunk->length);
    pa_memblock_release(chunk->memblock);
}

static void EnhanceProcessAndPost(struct Userdata *u, const uint64_t sceneKeyCode, pa_memchunk *enhanceChunk)
{
    AUTO_CTRACE("EnhanceProcessAndPost");
    CHECK_AND_RETURN_LOG(u != NULL, "userdata is null");
    CHECK_AND_RETURN_LOG(enhanceChunk != NULL, "enhanceChunk is null");
    pa_source *source = u->source;
    CHECK_AND_RETURN_LOG(source != NULL, "source is null");
    pa_source_assert_ref(source);

    void *state = NULL;
    pa_source_output *sourceOutput;
    EnhanceProcess(sceneKeyCode, enhanceChunk);

    uint32_t captureId = u->captureId;
    uint32_t renderId = u->renderId;
    while ((sourceOutput = pa_hashmap_iterate(source->thread_info.outputs, &state, NULL))) {
        pa_source_output_assert_ref(sourceOutput);
        const char *sourceOutputSceneType = pa_proplist_gets(sourceOutput->proplist, "scene.type");
        const char *defaultFlag = pa_proplist_gets(sourceOutput->proplist, "scene.default");
        // do not process sceneDefault
        if (pa_safe_streq(defaultFlag, "1")) {
            continue;
        }
        uint64_t sceneTypeCode = 0;
        if (GetSceneTypeCode(sourceOutputSceneType, &sceneTypeCode) != 0) {
            continue;
        }
        uint64_t sceneKeyCodeTemp = 0;
        sceneKeyCodeTemp = (sceneTypeCode << SCENE_TYPE_OFFSET) + (captureId << CAPTURER_ID_OFFSET) + renderId;
        if (sceneKeyCode != sceneKeyCodeTemp) {
            continue;
        }
        PostSourceData(source, sourceOutput, enhanceChunk);
    }
}

static void PostDataBypass(pa_source *source, pa_memchunk *chunk)
{
    CHECK_AND_RETURN_LOG(source != NULL, "source is null");
    pa_source_assert_ref(source);
    CHECK_AND_RETURN_LOG(chunk != NULL, "chunk is null");
    void *state = NULL;
    pa_source_output *sourceOutput;
    while ((sourceOutput = pa_hashmap_iterate(source->thread_info.outputs, &state, NULL))) {
        pa_source_output_assert_ref(sourceOutput);
        const char *sourceOutputSceneBypass = pa_proplist_gets(sourceOutput->proplist, "scene.bypass");
        if (sourceOutputSceneBypass == NULL) {
            continue;
        }
        if (strcmp(sourceOutputSceneBypass, DEFAULT_SCENE_BYPASS) == 0) {
            PostSourceData(source, sourceOutput, chunk);
        }
    }
}

static int32_t CheckSameAdapterEcLength(uint64_t request, uint64_t reply, uint64_t requestEc, uint64_t replyEc)
{
    if ((reply == 0) || (replyEc == 0) || (request != reply) || (requestEc != replyEc)) {
        return -1;
    }
    return 0;
}

static int32_t CheckDiffAdapterEcLength(uint64_t request, uint64_t reply, uint64_t requestEc, uint64_t replyEc)
{
    if ((reply == 0) || (replyEc == 0) || (request != reply) || (requestEc != replyEc)) {
        return -1;
    }
    return 0;
}

static int32_t HandleCaptureFrame(struct Userdata *u, char *buffer, uint64_t requestBytes, uint64_t *replyBytes)
{
    uint64_t replyBytesEc = 0;
    if (u->ecType == EC_NONE) {
        u->sourceAdapter->SourceAdapterCaptureFrame(u->sourceAdapter, buffer, requestBytes, replyBytes);
    }
    if (u->ecType == EC_SAME_ADAPTER) {
        struct SourceAdapterFrameDesc *fdesc = AllocateFrameDesc(buffer, requestBytes);
        struct SourceAdapterFrameDesc *fdescEc = AllocateFrameDesc((char *)(u->bufferEc), u->requestBytesEc);
        u->sourceAdapter->SourceAdapterCaptureFrameWithEc(u->sourceAdapter,
            fdesc, replyBytes, fdescEc, &replyBytesEc);
        FreeFrameDesc(fdesc);
        FreeFrameDesc(fdescEc);
        if (CheckSameAdapterEcLength(requestBytes, *replyBytes, u->requestBytesEc, replyBytesEc)) {
            u->requestBytesEc = 0;
        }
    }
    if (u->ecType == EC_DIFFERENT_ADAPTER) {
        u->sourceAdapter->SourceAdapterCaptureFrame(u->sourceAdapter, buffer, requestBytes, replyBytes);
        if (u->sourceAdapterEc != NULL) {
            struct SourceAdapterFrameDesc *fdesc = AllocateFrameDesc(NULL, requestBytes);
            struct SourceAdapterFrameDesc *fdescEc = AllocateFrameDesc((char *)(u->bufferEc), u->requestBytesEc);
            uint64_t replyBytesUnused = 0;
            u->sourceAdapterEc->SourceAdapterCaptureFrameWithEc(u->sourceAdapterEc,
                fdesc, &replyBytesUnused, fdescEc, &replyBytesEc);
            FreeFrameDesc(fdesc);
            FreeFrameDesc(fdescEc);
            if (CheckDiffAdapterEcLength(requestBytes, *replyBytes, u->requestBytesEc, replyBytesEc)) {
                u->requestBytesEc = 0;
            }
        }
    }
    uint64_t replyBytesMicRef = 0;
    if (u->micRef == REF_ON) {
        u->sourceAdapterMicRef->SourceAdapterCaptureFrame(u->sourceAdapterMicRef,
            (char *)(u->bufferMicRef), u->requestBytesMicRef, &replyBytesMicRef);
        if ((replyBytesMicRef == 0) && (u->requestBytesMicRef != replyBytesMicRef)) {
            u->bufferMicRef = 0;
        }
    }
    return 0;
}

static int GetCapturerFrameFromHdi(pa_memchunk *chunk, struct Userdata *u)
{
    AUTO_CTRACE("GetCapturerFrameFromHdi");
    uint64_t requestBytes = 0;
    uint64_t replyBytes = 0;

    void *p = NULL;
    CHECK_AND_RETURN_RET_LOG(chunk != NULL, 0, "chunk is null");
    CHECK_AND_RETURN_RET_LOG(chunk->memblock != NULL, 0, "chunk->memblock is null");
    p = pa_memblock_acquire(chunk->memblock);
    CHECK_AND_RETURN_RET_LOG(p != NULL, 0, "p is null");
    requestBytes = pa_memblock_get_length(chunk->memblock);
    HandleCaptureFrame(u, (char *)p, requestBytes, &replyBytes);
    pa_memblock_release(chunk->memblock);

    AUDIO_DEBUG_LOG("HDI Source: request bytes: %{public}" PRIu64 ", replyBytes: %{public}" PRIu64,
            requestBytes, replyBytes);

    if (replyBytes > requestBytes) {
        AUDIO_ERR_LOG("HDI Source: Error replyBytes > requestBytes. Requested data Length: "
                "%{public}" PRIu64 ", Read: %{public}" PRIu64 " bytes", requestBytes, replyBytes);
        pa_memblock_unref(chunk->memblock);
        return -1;
    }
    if (replyBytes == 0) {
        AUDIO_ERR_LOG("HDI Source: Failed to read, Requested data Length: %{public}" PRIu64 " bytes,"
                " Read: %{public}" PRIu64 " bytes", requestBytes, replyBytes);
        pa_memblock_unref(chunk->memblock);
        return -1;
    }
    chunk->index = 0;
    chunk->length = replyBytes;

    return 0;
}

static int32_t SampleAlignment(const char *sceneKey, pa_memchunk *enhanceChunk, pa_memchunk *rChunk, struct Userdata *u)
{
    CHECK_AND_RETURN_RET_LOG(sceneKey != NULL, ERROR, "sceneKey is null");
    CHECK_AND_RETURN_RET_LOG(enhanceChunk != NULL, ERROR, "enhanceChunk is null");
    CHECK_AND_RETURN_RET_LOG(u != NULL, ERROR, "Userdata is null");

    pa_resampler *resampler = (pa_resampler *)pa_hashmap_get(u->sceneToPreResamplerMap, sceneKey);
    if (resampler != NULL) {
        pa_resampler_run(resampler, enhanceChunk, rChunk);
    } else {
        *rChunk = *enhanceChunk;
        pa_memblock_ref(rChunk->memblock);
    }
    return SUCCESS;
}

static void PostDataDefault(pa_source *source, pa_memchunk *chunk, struct Userdata *u)
{
    CHECK_AND_RETURN_LOG(source != NULL, "source is null");
    pa_source_assert_ref(source);
    CHECK_AND_RETURN_LOG(chunk != NULL, "chunk is null");

    bool hasDefaultStream = false;
    pa_source_output *sourceOutput;
    void *state = NULL;
    while ((sourceOutput = pa_hashmap_iterate(source->thread_info.outputs, &state, NULL))) {
        pa_source_output_assert_ref(sourceOutput);
        const char *defaultFlag = pa_proplist_gets(sourceOutput->proplist, "scene.default");
        // process only sceneDefault
        if (!pa_safe_streq(defaultFlag, "1")) {
            continue;
        }
        hasDefaultStream = true;
    }
    if (!hasDefaultStream) { return; }

    pa_memchunk enhanceChunk;
    pa_memchunk rChunk;
    enhanceChunk.length = chunk->length;
    enhanceChunk.memblock = pa_memblock_new(u->core->mempool, enhanceChunk.length);
    pa_memchunk_memcpy(&enhanceChunk, chunk);

    pa_resampler *resampler = u->defaultSceneResampler;
    if (resampler) {
        pa_resampler_run(resampler, &enhanceChunk, &rChunk);
    } else {
        rChunk = enhanceChunk;
        pa_memblock_ref(rChunk.memblock);
    }
    EnhanceProcessDefault(u->captureId, &rChunk);

    while ((sourceOutput = pa_hashmap_iterate(source->thread_info.outputs, &state, NULL))) {
        pa_source_output_assert_ref(sourceOutput);
        const char *defaultFlag = pa_proplist_gets(sourceOutput->proplist, "scene.default");
        // process only sceneDefault
        if (!pa_safe_streq(defaultFlag, "1")) {
            continue;
        }
        PostSourceData(source, sourceOutput, &rChunk);
    }

    pa_memblock_unref(enhanceChunk.memblock);
    if (rChunk.memblock) {
        pa_memblock_unref(rChunk.memblock);
    }
}

static int32_t EcResample(const char *sceneKey, struct Userdata *u)
{
    pa_resampler *ecResampler = (pa_resampler *)pa_hashmap_get(u->sceneToEcResamplerMap, sceneKey);

    CHECK_AND_RETURN_RET_LOG(u->bufferEc != NULL, ERROR, "bufferEc is null");
    CHECK_AND_RETURN_RET_LOG(u->requestBytesEc != 0, ERROR, "requestBytesEc is 0");
    if (ecResampler != NULL) {
        pa_memchunk ecChunk;
        pa_memchunk rEcChunk;
        ecChunk.length = u->requestBytesEc;
        ecChunk.memblock = pa_memblock_new_fixed(u->core->mempool, u->bufferEc, ecChunk.length, 1);
        pa_resampler_run(ecResampler, &ecChunk, &rEcChunk);
        void *srcEc = pa_memblock_acquire_chunk(&rEcChunk);
        AUDIO_DEBUG_LOG("ec chunk length: %{public}zu sceneKey: %{public}s", rEcChunk.length, sceneKey);
        CopyEcdataToEnhanceBufferAdapter(srcEc, rEcChunk.length);
        pa_memblock_release(rEcChunk.memblock);
        pa_memblock_unref(ecChunk.memblock);
        pa_memblock_unref(rEcChunk.memblock);
    } else {
        CopyEcdataToEnhanceBufferAdapter(u->bufferEc, u->requestBytesEc);
    }
    return SUCCESS;
}

static int32_t MicRefResample(const char *sceneKey, struct Userdata *u)
{
    pa_resampler *micRefResampler = (pa_resampler *)pa_hashmap_get(u->sceneToMicRefResamplerMap, sceneKey);

    CHECK_AND_RETURN_RET_LOG(u->bufferMicRef != NULL, ERROR, "bufferMicRef is null");
    CHECK_AND_RETURN_RET_LOG(u->requestBytesMicRef != 0, ERROR, "requestBytesMicRef is 0");
    if (micRefResampler != NULL) {
        pa_memchunk micRefChunk;
        pa_memchunk rMicRefChunk;
        micRefChunk.length = u->requestBytesMicRef;
        micRefChunk.memblock = pa_memblock_new_fixed(u->core->mempool, u->bufferMicRef, micRefChunk.length, 1);
        pa_resampler_run(micRefResampler, &micRefChunk, &rMicRefChunk);
        void *srcMicRef = pa_memblock_acquire_chunk(&rMicRefChunk);
        AUDIO_DEBUG_LOG("micRef chunk length: %{public}zu sceneKey: %{public}s", rMicRefChunk.length, sceneKey);
        CopyMicRefdataToEnhanceBufferAdapter(srcMicRef, rMicRefChunk.length);
        pa_memblock_release(rMicRefChunk.memblock);
        pa_memblock_unref(micRefChunk.memblock);
        pa_memblock_unref(rMicRefChunk.memblock);
    } else {
        CopyMicRefdataToEnhanceBufferAdapter(u->bufferMicRef, u->requestBytesMicRef);
    }
    return SUCCESS;
}

static int32_t AudioEnhanceExistAndProcess(pa_memchunk *chunk, struct Userdata *u)
{
    AUTO_CTRACE("AudioEnhanceExistAndProcess");

    bool ret = EnhanceChainManagerIsEmptyEnhanceChain();
    if (ret) {
        // if none enhance chain exist, post data as the original method
        pa_source_post(u->source, chunk);
        pa_memblock_unref(chunk->memblock);
        return 0;
    }

    PostDataBypass(u->source, chunk);
    PostDataDefault(u->source, chunk, u);
    void *state = NULL;
    uint32_t *sceneKeyNum;
    const void *sceneKey;
    while ((sceneKeyNum = pa_hashmap_iterate(u->sceneToCountMap, &state, &sceneKey))) {
        uint64_t sceneKeyCode = (uint64_t)strtoul((char *)sceneKey, NULL, BASE_TEN);
        AUDIO_DEBUG_LOG("Now sceneKeyCode is : %{public}" PRIu64, sceneKeyCode);

        pa_memchunk enhanceChunk;
        pa_memchunk rChunk;
        enhanceChunk.length = chunk->length;
        enhanceChunk.memblock = pa_memblock_new(u->core->mempool, enhanceChunk.length);
        pa_memchunk_memcpy(&enhanceChunk, chunk);
        SampleAlignment((char *)sceneKey, &enhanceChunk, &rChunk, u);
        if (u->ecType != EC_NONE) {
            EcResample((char *)sceneKey, u);
        }
        if (u->micRef == REF_ON) {
            MicRefResample((char *)sceneKey, u);
        }
        EnhanceProcessAndPost(u, sceneKeyCode, &rChunk);
        pa_memblock_unref(enhanceChunk.memblock);
        pa_memblock_unref(rChunk.memblock);
    }
    pa_memblock_unref(chunk->memblock);

    return 0;
}

static void ThreadCaptureSleep(pa_usec_t sleepTime)
{
    struct timespec req;
    struct timespec rem;
    req.tv_sec = 0;
    req.tv_nsec = (int64_t)(sleepTime * MILLISECOND_PER_SECOND);
    clock_nanosleep(CLOCK_REALTIME, 0, &req, &rem);
    AUDIO_DEBUG_LOG("ThreadCaptureData sleep:%{public}" PRIu64, sleepTime);
}

static void ThreadCaptureData(void *userdata)
{
    struct Userdata *u = userdata;
    CHECK_AND_RETURN_LOG(u != NULL, "u is null");
    // set audio thread priority
    ScheduleThreadInServer(getpid(), gettid());

    pa_memchunk chunk;
    int32_t ret = 0;
    pa_usec_t now = 0;
    pa_usec_t cost = 0;

    while (!pa_atomic_load(&u->quitCaptureFlag)) {
        if (pa_atomic_load(&u->captureFlag) == 1) {
            AUTO_CTRACE("ThreadCaptureDataLoop");
            now = pa_rtclock_now();
            chunk.length = u->bufferSize;
            AUDIO_DEBUG_LOG("HDI Source: chunk.length = u->bufferSize: %{public}zu", chunk.length);
            chunk.memblock = pa_memblock_new(u->core->mempool, chunk.length);
            ret = GetCapturerFrameFromHdi(&chunk, u);
            if (ret != 0) {
                AUDIO_ERR_LOG("GetCapturerFrameFromHdi failed");
                continue;
            }
            pa_asyncmsgq_post(u->CaptureMq, NULL, HDI_POST, NULL, 0, &chunk, NULL);
            eventfd_t writEvent = 1;
            int32_t writeRes = eventfd_write(u->eventFd, writEvent);
            if (writeRes != 0) {
                AUDIO_ERR_LOG("Failed to write to eventfd");
                continue;
            }
            cost = pa_rtclock_now() - now;
            AUDIO_DEBUG_LOG("capture frame cost :%{public}" PRIu64, cost);
        } else {
            ThreadCaptureSleep(u->blockUsec);
        }
    }
    UnscheduleThreadInServer(getpid(), gettid());
    AUDIO_INFO_LOG("ThreadCaptureData quit pid %{public}d, tid %{public}d", getpid(), gettid());
}

static void PaRtpollProcessFunc(struct Userdata *u)
{
    AUTO_CTRACE("PaRtpollProcessFunc");

    eventfd_t value;
    int32_t readRet = eventfd_read(u->eventFd, &value);
    CHECK_AND_RETURN_LOG((u->source->thread_info.state == PA_SOURCE_RUNNING) || (readRet == 0),
        "Failed to read from eventfd");

    pa_memchunk chunk;
    int32_t code = 0;
    pa_usec_t now = pa_rtclock_now();

    while (pa_asyncmsgq_get(u->CaptureMq, NULL, &code, NULL, NULL, &chunk, 0) == 0) {
        if (u->source->thread_info.state != PA_SOURCE_RUNNING) {
            // when the source is not in running state, but we still recive data from CaptureMq.
            pa_memblock_unref(chunk.memblock);
            pa_asyncmsgq_done(u->CaptureMq, 0);
            continue;
        }
        AudioEnhanceExistAndProcess(&chunk, u);
        pa_asyncmsgq_done(u->CaptureMq, 0);
    }

    int32_t appsUid[PA_MAX_OUTPUTS_PER_SOURCE];
    int32_t sessionId[PA_MAX_OUTPUTS_PER_SOURCE];
    size_t count = 0;
    size_t sessionIdCount = 0;
    memset_s(sessionId, PA_MAX_OUTPUTS_PER_SOURCE * sizeof(uint32_t), 0, PA_MAX_OUTPUTS_PER_SOURCE * sizeof(uint32_t));

    void *state = NULL;
    pa_source_output *sourceOutput;
    while ((sourceOutput = pa_hashmap_iterate(u->source->thread_info.outputs, &state, NULL))) {
        const char *cstringClientUid = pa_proplist_gets(sourceOutput->proplist, "stream.client.uid");
        if (cstringClientUid && (sourceOutput->thread_info.state == PA_SOURCE_OUTPUT_RUNNING)) {
            appsUid[count++] = atoi(cstringClientUid);
        }
        const char *sessionIdInChar = pa_proplist_gets(sourceOutput->proplist, "stream.sessionID");
        if (sessionIdInChar && (sourceOutput->thread_info.state == PA_SOURCE_OUTPUT_RUNNING) && u->sourceAdapter) {
            sessionId[sessionIdCount++] = atoi(sessionIdInChar);
        }
    }

    if (u->sourceAdapter) {
        u->sourceAdapter->SourceAdapterUpdateAppsUid(u->sourceAdapter, appsUid, count);
        u->sourceAdapter->SourceAdapterUpdateSessionUid(u->sourceAdapter, sessionId, sessionIdCount);
    }

    pa_usec_t costTime = pa_rtclock_now() - now;
    AUDIO_DEBUG_LOG("enhance process and post costTime:%{public}" PRIu64, costTime);
    return;
}

static void ThreadFuncProcessTimer(void *userdata)
{
    struct Userdata *u = userdata;

    // set audio thread priority
    ScheduleThreadInServer(getpid(), gettid());
    CHECK_AND_RETURN_LOG(u != NULL, "u is null");

    pa_thread_mq_install(&u->threadMq);
    u->timestamp = pa_rtclock_now();

    AUDIO_DEBUG_LOG("HDI Source: u->timestamp : %{public}" PRIu64, u->timestamp);

    if (u->rtpollItem) {
        struct pollfd *pollFd = pa_rtpoll_item_get_pollfd(u->rtpollItem, NULL);
        CHECK_AND_BREAK_LOG(pollFd != NULL, "pollFd is null");
        pollFd->events = POLLIN;
    }

    while (true) {
        bool flag = (u->attrs.sourceType == SOURCE_TYPE_WAKEUP) ?
            (u->source->thread_info.state == PA_SOURCE_RUNNING && u->isCapturerStarted) :
            (PA_SOURCE_IS_OPENED(u->source->thread_info.state) && u->isCapturerStarted);
        pa_atomic_store(&u->captureFlag, flag);
        if (flag) {
            pa_rtpoll_set_timer_relative(u->rtpoll, RTPOLL_RUN_WAKEUP_INTERVAL_USEC);
        } else {
            pa_rtpoll_set_timer_disabled(u->rtpoll);
        }
        AUTO_CTRACE("Process Capture Data Loop");
        /* Hmm, nothing to do. Let's sleep */
        int ret = pa_rtpoll_run(u->rtpoll);
        if (ret < 0) {
            /* If this was no regular exit from the loop we have to continue
            * processing messages until we received PA_MESSAGE_SHUTDOWN */
            AUDIO_ERR_LOG("HDI Source: pa_rtpoll_run ret:%{public}d failed", ret);
            pa_asyncmsgq_post(u->threadMq.outq, PA_MSGOBJECT(u->core), PA_CORE_MESSAGE_UNLOAD_MODULE, u->module,
                0, NULL, NULL);
            pa_asyncmsgq_wait_for(u->threadMq.inq, PA_MESSAGE_SHUTDOWN);
            break;
        }
        if (ret == 0) {
            AUDIO_INFO_LOG("Thread OS_ProcessCapData shutting down, pid %{public}d, tid %{public}d",
                getpid(), gettid());
            break;
        }
        PaRtpollProcessFunc(u);
    }
    UnscheduleThreadInServer(getpid(), gettid());
}

static int PaHdiCapturerInit(struct Userdata *u)
{
    int ret;
    ret = u->sourceAdapter->SourceAdapterInit(u->sourceAdapter, &u->attrs);
    if (ret != 0) {
        AUDIO_ERR_LOG("Audio capturer init failed!");
        return ret;
    }
    InitAuxCapture(u);

    u->captureId = u->sourceAdapter->captureId;
    u->renderId = 0;

#ifdef IS_EMULATOR
    // Due to the peculiar implementation of the emulator's HDI,
    // an initial start and stop sequence is required to circumvent protential issues and ensure proper functionality.
    AUDIO_INFO_LOG("do start and stop");
    u->sourceAdapter->CapturerSourceStart(u->sourceAdapter->wapper);
    u->sourceAdapter->CapturerSourceStop(u->sourceAdapter->wapper);
#endif

    u->isCapturerStarted = false;
    return ret;
}

static void PaHdiCapturerExit(struct Userdata *u)
{
    CHECK_AND_RETURN_LOG(u != NULL, "u is null");
    CHECK_AND_RETURN_LOG((u->sourceAdapter) != NULL, " u->sourceAdapter is null");
    u->sourceAdapter->SourceAdapterStop(u->sourceAdapter);
    u->sourceAdapter->SourceAdapterDeInit(u->sourceAdapter);
    StopAuxCapture(u);
    DeinitAuxCapture(u);
}

static int PaSetSourceProperties(pa_module *m, pa_modargs *ma, const pa_sample_spec *ss, const pa_channel_map *map,
    struct Userdata *u)
{
    pa_source_new_data data;

    pa_source_new_data_init(&data);
    data.driver = __FILE__;
    data.module = m;

    pa_source_new_data_set_name(&data, pa_modargs_get_value(ma, "source_name", DEFAULT_SOURCE_NAME));
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_STRING,
        (u->attrs.adapterName ? u->attrs.adapterName : DEFAULT_AUDIO_DEVICE_NAME));
    pa_proplist_setf(data.proplist, PA_PROP_DEVICE_DESCRIPTION, "HDI source is %s",
        (u->attrs.adapterName ? u->attrs.adapterName : DEFAULT_AUDIO_DEVICE_NAME));
    pa_source_new_data_set_sample_spec(&data, ss);
    pa_source_new_data_set_channel_map(&data, map);
    pa_proplist_setf(data.proplist, PA_PROP_DEVICE_BUFFERING_BUFFER_SIZE, "%lu", (unsigned long)u->bufferSize);

    // set suspend on idle timeout to 0s
    pa_proplist_setf(data.proplist, "module-suspend-on-idle.timeout", "%d", 0);

    if (pa_modargs_get_proplist(ma, "source_properties", data.proplist, PA_UPDATE_REPLACE) < 0) {
        AUDIO_ERR_LOG("Invalid properties");
        pa_source_new_data_done(&data);
        return -1;
    }

    u->source = pa_source_new(m->core, &data, PA_SOURCE_HARDWARE | PA_SOURCE_LATENCY | PA_SOURCE_DYNAMIC_LATENCY);
    pa_source_new_data_done(&data);

    if (!u->source) {
        AUDIO_ERR_LOG("Failed to create source object");
        return -1;
    }

    u->source->parent.process_msg = SourceProcessMsg;
    u->source->set_state_in_io_thread = SourceSetStateInIoThreadCb;
    u->source->userdata = u;

    pa_source_set_asyncmsgq(u->source, u->threadMq.inq);
    pa_source_set_rtpoll(u->source, u->rtpoll);

    u->blockUsec = pa_bytes_to_usec(u->bufferSize, &u->source->sample_spec);
    pa_source_set_latency_range(u->source, 0, u->blockUsec);
    u->source->thread_info.max_rewind = pa_usec_to_bytes(u->blockUsec, &u->source->sample_spec);

    return 0;
}

static enum AudioSampleFormatIntf ConvertPaToHdiAdapterFormat(pa_sample_format_t format)
{
    enum AudioSampleFormatIntf adapterFormat;
    switch (format) {
        case PA_SAMPLE_U8:
            adapterFormat = SAMPLE_U8;
            break;
        case PA_SAMPLE_S16LE:
        case PA_SAMPLE_S16BE:
            adapterFormat = SAMPLE_S16;
            break;
        case PA_SAMPLE_S24LE:
        case PA_SAMPLE_S24BE:
            adapterFormat = SAMPLE_S24;
            break;
        case PA_SAMPLE_S32LE:
        case PA_SAMPLE_S32BE:
            adapterFormat = SAMPLE_S32;
            break;
        default:
            adapterFormat = SAMPLE_S16;
            break;
    }

    return adapterFormat;
}

static bool GetEndianInfo(pa_sample_format_t format)
{
    bool isBigEndian = false;
    switch (format) {
        case PA_SAMPLE_S16BE:
        case PA_SAMPLE_S24BE:
        case PA_SAMPLE_S32BE:
        case PA_SAMPLE_FLOAT32BE:
        case PA_SAMPLE_S24_32BE:
            isBigEndian = true;
            break;
        default:
            isBigEndian = false;
            break;
    }

    return isBigEndian;
}

static void InitUserdataAttrs(pa_modargs *ma, struct Userdata *u, const pa_sample_spec *ss)
{
    if (pa_modargs_get_value_s32(ma, "source_type", &u->attrs.sourceType) < 0) {
        AUDIO_ERR_LOG("Failed to parse source_type argument");
    }

    if (pa_modargs_get_value_u32(ma, "buffer_size", &u->bufferSize) < 0) {
        AUDIO_ERR_LOG("Failed to parse buffer_size argument.");
        u->bufferSize = DEFAULT_BUFFER_SIZE;
    }
    u->attrs.bufferSize = u->bufferSize;

    u->attrs.sampleRate = ss->rate;
    u->attrs.filePath = pa_modargs_get_value(ma, "file_path", "");
    if (pa_modargs_get_value_u32(ma, "open_mic_speaker", &u->openMicSpeaker) < 0) {
        AUDIO_ERR_LOG("Failed to parse open_mic_speaker argument");
    }
    u->attrs.channel = ss->channels;
    u->attrs.format = ConvertPaToHdiAdapterFormat(ss->format);
    u->attrs.isBigEndian = GetEndianInfo(ss->format);
    u->attrs.adapterName = pa_modargs_get_value(ma, "adapter_name", DEFAULT_DEVICE_CLASS);
    u->attrs.deviceNetworkId = pa_modargs_get_value(ma, "network_id", DEFAULT_DEVICE_NETWORKID);
    if (pa_modargs_get_value_s32(ma, "device_type", &u->attrs.deviceType) < 0) {
        AUDIO_ERR_LOG("Failed to parse deviceType argument");
    }

    AUDIO_DEBUG_LOG("AudioDeviceCreateCapture format: %{public}d, isBigEndian: %{public}d channel: %{public}d,"
        "sampleRate: %{public}d", u->attrs.format, u->attrs.isBigEndian, u->attrs.channel, u->attrs.sampleRate);

    u->attrs.openMicSpeaker = u->openMicSpeaker;

    u->sceneToCountMap = pa_hashmap_new_full(pa_idxset_string_hash_func, pa_idxset_string_compare_func,
        pa_xfree, pa_xfree);

    u->sceneToPreResamplerMap = pa_hashmap_new_full(pa_idxset_string_hash_func, pa_idxset_string_compare_func,
        pa_xfree, (pa_free_cb_t) pa_resampler_free);

    u->sceneToEcResamplerMap = pa_hashmap_new_full(pa_idxset_string_hash_func, pa_idxset_string_compare_func,
        pa_xfree, (pa_free_cb_t) pa_resampler_free);

    u->sceneToMicRefResamplerMap = pa_hashmap_new_full(pa_idxset_string_hash_func, pa_idxset_string_compare_func,
        pa_xfree, (pa_free_cb_t) pa_resampler_free);
}

static void InitDifferentAdapterEcAttr(struct Userdata *u, struct SourceAdapterAttr *attr)
{
    // set attr for different adapter ec
    attr->sourceType = SOURCE_TYPE_EC;
    // device attrs
    attr->adapterName = u->ecAdapaterName;
    attr->deviceType = DEVICE_TYPE_MIC; // not needed, updateAudioRoute later
    // common audio attrs
    attr->sampleRate = u->ecSamplingRate;
    attr->channel = u->ecChannels;
    attr->format = u->ecFormat;
    attr->isBigEndian = false;
    attr->openMicSpeaker = u->openMicSpeaker;
}

static void InitMicRefAttr(struct Userdata *u, struct SourceAdapterAttr *attr)
{
    // set attr for mic ref
    attr->sourceType = SOURCE_TYPE_MIC_REF;
    // device attrs
    attr->adapterName = "primary";
    attr->deviceType = DEVICE_TYPE_MIC;
    // common audio attrs
    attr->sampleRate = u->micRefRate;
    attr->channel = u->micRefChannels;
    attr->format = u->micRefFormat;
    attr->isBigEndian = false;
    attr->openMicSpeaker = u->openMicSpeaker;
}

static void InitSampleSpec(pa_sample_spec *spec, const uint32_t sampleRate,
    const pa_sample_format_t format, const uint32_t channels)
{
    pa_sample_spec_init(spec);
    spec->rate = sampleRate;
    spec->channels = (uint8_t)channels;
    spec->format = format;
}

static void InitEcAndMicRefAttrs(pa_modargs *ma, struct Userdata *u)
{
    if (pa_modargs_get_value_u32(ma, "ec_type", &u->ecType) < 0) {
        u->ecType = EC_NONE;
    }
    u->ecAdapaterName = pa_modargs_get_value(ma, "ec_adapter", "");
    if (pa_modargs_get_value_u32(ma, "ec_sampling_rate", &u->ecSamplingRate) < 0) {
        u->ecSamplingRate = 0;
    }
    const char *ecFormatStr = pa_modargs_get_value(ma, "ec_format", "");
    u->ecFormat = ConvertPaToHdiAdapterFormat(pa_parse_sample_format(ecFormatStr));
    if (pa_modargs_get_value_u32(ma, "ec_channels", &u->ecChannels) < 0) {
        u->ecChannels = 0;
    }
    InitSampleSpec(&u->ecSpec, u->ecSamplingRate, pa_parse_sample_format(ecFormatStr), u->ecChannels);
    if (pa_modargs_get_value_u32(ma, "open_mic_ref", &u->micRef) < 0) {
        u->micRef = REF_OFF;
    }
    if (pa_modargs_get_value_u32(ma, "mic_ref_rate", &u->micRefRate) < 0) {
        u->micRefRate = 0;
    }
    const char *micRefFormatStr = pa_modargs_get_value(ma, "mic_ref_format", "");
    u->micRefFormat = ConvertPaToHdiAdapterFormat(pa_parse_sample_format(micRefFormatStr));
    if (pa_modargs_get_value_u32(ma, "mic_ref_channels", &u->micRefChannels) < 0) {
        u->micRefChannels = 0;
    }
    InitSampleSpec(&u->micRefSpec, u->micRefRate, pa_parse_sample_format(micRefFormatStr), u->micRefChannels);
    AUDIO_INFO_LOG("ecType: %{public}d, ecAdapaterName: %{public}s, ecSamplingRate: %{public}d ecFormat: %{public}d,"
        " ecChannels: %{public}d, micRef: %{public}d, micRefRate: %{public}d, micRefFormat: %{public}d,"
        " micRefChannels: %{public}d", u->ecType, u->ecAdapaterName, u->ecSamplingRate, u->ecFormat,
        u->ecChannels, u->micRef, u->micRefRate, u->micRefFormat, u->micRefChannels);
}

static void PrepareEcCapture(struct Userdata *u)
{
    // init to avoid unexpeceted condition
    u->attrs.hasEcConfig = false;
    u->sourceAdapterEc = NULL;
    u->requestBytesEc = 0;
    u->bufferEc = NULL;

    if (u->ecType == EC_NONE) {
        return;
    }

    if (u->ecType == EC_SAME_ADAPTER) {
        // basic record attrs already prepared, only prepare ec attrs here
        u->attrs.hasEcConfig = true;
        u->attrs.formatEc = u->ecFormat;
        u->attrs.sampleRateEc = u->ecSamplingRate;
        u->attrs.channelEc = u->ecChannels;

        u->requestBytesEc = CalculateFrameLen(u->ecSamplingRate, u->ecChannels, u->ecFormat);
        u->bufferEc = malloc(u->requestBytesEc);
        if (u->bufferEc == NULL) {
            AUDIO_ERR_LOG("malloc ec buffer in same adapter failed");
        }
    }

    if (u->ecType == EC_DIFFERENT_ADAPTER) {
        // only ec different adapter need create aux capture
        struct SourceAdapterAttr *attr = (struct SourceAdapterAttr *)calloc(1, sizeof(struct SourceAdapterAttr));
        if (attr == NULL) {
            AUDIO_ERR_LOG("capture attr allocate failed");
            return;
        }
        InitDifferentAdapterEcAttr(u, attr);
        u->sourceAdapterEc = GetSourceAdapter(DEFAULT_DEVICE_CLASS, -1, HDI_ID_INFO_EC);
        if (u->sourceAdapterEc == NULL) {
            AUDIO_ERR_LOG("create ec handle failed");
            free(attr);
            return;
        }
        u->sourceAdapterEc->attr = attr;
        u->requestBytesEc = CalculateFrameLen(u->ecSamplingRate, u->ecChannels, u->ecFormat);
        u->bufferEc = malloc(u->requestBytesEc);
        if (u->bufferEc == NULL) {
            AUDIO_ERR_LOG("malloc ec buffer in different adapter failed");
        }
    }
}

static void PrepareMicRefCapture(struct Userdata *u)
{
    u->sourceAdapterMicRef = NULL;
    u->bufferMicRef = NULL;
    u->requestBytesMicRef = 0;

    if (u->micRef != REF_ON) {
        return;
    }

    struct SourceAdapterAttr *attr = (struct SourceAdapterAttr *)calloc(1, sizeof(struct SourceAdapterAttr));
    if (attr == NULL) {
        AUDIO_ERR_LOG("capture attr allocate failed");
        return;
    }

    InitMicRefAttr(u, attr);
    u->sourceAdapterMicRef = GetSourceAdapter(DEFAULT_DEVICE_CLASS, -1, HDI_ID_INFO_MIC_REF);
    if (u->sourceAdapterMicRef == NULL) {
        AUDIO_ERR_LOG("create mic ref handle failed");
        free(attr);
        return;
    }
    u->sourceAdapterMicRef->attr = attr;
    u->requestBytesMicRef = CalculateFrameLen(u->micRefRate, u->micRefChannels, u->micRefFormat);
    u->bufferMicRef = malloc(u->requestBytesMicRef);
    if (u->bufferMicRef == NULL) {
        AUDIO_ERR_LOG("malloc micref buffer failed");
    }
}

int32_t CreateCaptureDataThread(pa_module *m, struct Userdata *u)
{
    CHECK_AND_RETURN_RET_LOG(m != NULL, -1, "m is null");
    CHECK_AND_RETURN_RET_LOG(u != NULL, -1, "u is null");

    pa_atomic_store(&u->captureFlag, 0);
    pa_atomic_store(&u->quitCaptureFlag, 0);

    if (!(u->CaptureMq = pa_asyncmsgq_new(0))) {
        AUDIO_ERR_LOG("Failed to create u->CaptureMq");
        return -1;
    }

    u->eventFd = eventfd(0, EFD_NONBLOCK);
    fdsan_exchange_owner_tag(u->eventFd, 0, DOMAIN_ID);
    u->rtpollItem = pa_rtpoll_item_new(u->rtpoll, PA_RTPOLL_NEVER, 1);
    struct pollfd *pollFd = pa_rtpoll_item_get_pollfd(u->rtpollItem, NULL);
    CHECK_AND_RETURN_RET_LOG(pollFd != NULL, -1, "get pollfd failed");
    pollFd->fd = u->eventFd;
    pollFd->events = 0;
    pollFd->revents = 0;

    if (!(u->thread = pa_thread_new("OS_ProcessCapData", ThreadFuncProcessTimer, u))) {
        AUDIO_ERR_LOG("Failed to create hdi-source-record thread!");
        return -1;
    }

    if (!(u->threadCap = pa_thread_new("OS_CaptureData", ThreadCaptureData, u))) {
        AUDIO_ERR_LOG("Failed to create capture-data thread!");
        return -1;
    }
    return 0;
}

static struct SourceAdapter *GetSourceAdapterBySourceType(const char *deviceClass, const int32_t sourceType,
    const char *sourceName, const char *networkId)
{
    if (sourceType == SOURCE_TYPE_WAKEUP) {
        return GetSourceAdapter(deviceClass, sourceType, sourceName);
    }
    return GetSourceAdapter(deviceClass, sourceType, networkId);
}

pa_source *PaHdiSourceNew(pa_module *m, pa_modargs *ma, const char *driver)
{
    CHECK_AND_RETURN_RET_LOG(m != NULL && ma != NULL, NULL, "m or ma is null");

    pa_sample_spec ss = m->core->default_sample_spec;
    pa_channel_map map = m->core->default_channel_map;

    /* Override with modargs if provided */
    if (pa_modargs_get_sample_spec_and_channel_map(ma, &ss, &map, PA_CHANNEL_MAP_DEFAULT) < 0) {
        AUDIO_ERR_LOG("Failed to parse sample specification and channel map");
        return NULL;
    }

    struct Userdata *u = pa_xnew0(struct Userdata, 1);
    if (u == NULL) {
        AUDIO_ERR_LOG("userdata alloc failed");
        goto fail;
    }

    u->core = m->core;
    u->module = m;
    u->rtpoll = pa_rtpoll_new();

    if (pa_thread_mq_init(&u->threadMq, m->core->mainloop, u->rtpoll) < 0) {
        AUDIO_ERR_LOG("pa_thread_mq_init() failed.");
        goto fail;
    }

    InitUserdataAttrs(ma, u, &ss);

    InitEcAndMicRefAttrs(ma, u);

    const char *deviceClass = pa_modargs_get_value(ma, "device_class", DEFAULT_DEVICE_CLASS);
    u->sourceAdapter = GetSourceAdapterBySourceType(deviceClass, u->attrs.sourceType, pa_modargs_get_value(ma,
        "source_name", DEFAULT_SOURCE_NAME), pa_modargs_get_value(ma, "network_id", DEFAULT_DEVICE_NETWORKID));
    if (u->sourceAdapter == NULL) {
        AUDIO_ERR_LOG("Load adapter failed");
        goto fail;
    }

    PrepareEcCapture(u);
    PrepareMicRefCapture(u);

    if (PaSetSourceProperties(m, ma, &ss, &map, u) != 0) {
        AUDIO_ERR_LOG("Failed to PaSetSourceProperties");
        goto fail;
    }

    if (PaHdiCapturerInit(u) != 0) {
        AUDIO_ERR_LOG("Failed to PaHdiCapturerInit");
        goto fail;
    }

    if (CreateCaptureDataThread(m, u) != 0) {
        goto fail;
    }
    return u->source;

fail:

    if (u->isCapturerStarted) {
        PaHdiCapturerExit(u);
    }
    UserdataFree(u);

    return NULL;
}

void PaHdiSourceFree(pa_source *s)
{
    AUTO_CTRACE("PaHdiSourceFree");
    struct Userdata *u = NULL;
    if (s == NULL) {
        AUDIO_INFO_LOG("pa_source is null, PaHdiSourceFree done");
        return;
    }
    pa_source_assert_ref(s);
    pa_assert_se(u = s->userdata);
    UserdataFree(u);
}