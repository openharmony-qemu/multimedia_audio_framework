/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#undef LOG_TAG
#define LOG_TAG "ModuleSplitStreamSink"

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "securec.h"

#include <pulse/rtclock.h>
#include <pulse/timeval.h>
#include <pulse/util.h>
#include <pulse/xmalloc.h>

#include <pulsecore/i18n.h>
#include <pulsecore/macro.h>
#include <pulsecore/sink.h>
#include <pulsecore/module.h>
#include <pulsecore/core-util.h>
#include <pulsecore/modargs.h>
#include <pulsecore/log.h>
#include <pulsecore/thread.h>
#include <pulsecore/thread-mq.h>
#include <pulsecore/rtpoll.h>
#include <pulsecore/mix.h>
#include <pulsecore/memblockq.h>
#include <pulsecore/memblock.h>

#include <pthread.h>
#include "audio_pulseaudio_log.h"
#include "audio_schedule.h"
#include "audio_utils_c.h"
#include "volume_tools_c.h"
#include "audio_volume_c.h"
#include "intf_def.h"
#include "sink/sink_intf.h"

#define DEFAULT_SINK_NAME "hdi_output"
#define DEFAULT_DEVICE_CLASS "primary"
#define DEFAULT_DEVICE_NETWORKID "LocalDevice"
#define DEFAULT_BUFFER_SIZE 8192
#define MAX_SINK_VOLUME_LEVEL 1.0
#define MIX_BUFFER_LENGTH (pa_page_size())
#define MAX_REWIND (7000 * PA_USEC_PER_MSEC)
#define USEC_PER_SEC 1000000
#define SCENE_TYPE_NUM 7
#define PA_ERR (-1)
#define MAX_PARTS 10

#define STREAM_TYPE_MEDIA "1"
#define STREAM_TYPE_COMMUNICATION "2"
#define STREAM_TYPE_NAVIGATION "13"
#define STREAM_TYPE_VIDEO_COMMUNICATION "17"

char *g_splitArr[MAX_PARTS];
int g_splitNums = 0;
const char *SPLIT_MODE;
const uint32_t SPLIT_ONE_STREAM = 1;
const uint32_t SPLIT_TWO_STREAM = 2;
const uint32_t SPLIT_THREE_STREAM = 3;

PA_MODULE_AUTHOR("OpenHarmony");
PA_MODULE_DESCRIPTION(_("Split Stream Sink"));
PA_MODULE_VERSION(PACKAGE_VERSION);
PA_MODULE_LOAD_ONCE(false);
PA_MODULE_USAGE(
        "sink_name=<name of sink> "
        "sink_properties=<properties for the sink> "
        "format=<sample format> "
        "rate=<sample rate> "
        "channels=<number of channels> "
        "channel_map=<channel map>"
        "buffer_size=<custom buffer size>"
        "formats=<semi-colon separated sink formats>");

static ssize_t SplitRenderWrite(struct SinkAdapter *sinkAdapter, pa_memchunk *pchunk, char *streamType);

struct userdata {
    pa_core *core;
    pa_module *module;
    pa_sink *sink;
    pa_thread *thread;
    pa_thread_mq thread_mq;
    pa_rtpoll *rtpoll;
    uint32_t buffer_size;
    pa_usec_t block_usec;
    pa_usec_t timestamp;
    pa_idxset *formats;
    pa_thread *thread_split_hdi;
    bool isHDISinkStarted;
    struct SinkAdapter *sinkAdapter;
    pa_asyncmsgq *dq;
    pa_atomic_t dflag;
    pa_usec_t writeTime;
    pa_usec_t prewrite;
    pa_sink_state_t previousState;
    pa_usec_t timestampLastLog;
    const char *deviceNetworkId;
    const char *adapterName;
    uint32_t open_mic_speaker;
    pa_sample_spec ss;
    pa_channel_map map;
    int32_t deviceType;
    size_t bytesDropped;
    uint32_t writeCount;
    uint32_t renderCount;
    uint32_t fixed_latency;
    pa_usec_t lastProcessDataTime;
    uint32_t renderInIdleState;
    uint32_t defaultAdapterEnable;
};

static const char * const VALID_MODARGS[] = {
    "sink_name",
    "device_class",
    "sink_properties",
    "format",
    "rate",
    "channels",
    "channel_map",
    "buffer_size",
    "file_path",
    "adapter_name",
    "fixed_latency",
    "sink_latency",
    "render_in_idle_state",
    "open_mic_speaker",
    "test_mode_on",
    "network_id",
    "device_type",
    "offload_enable",
    "default_adapter_enable",
    "split_mode",
    NULL
};
   
char *const SCENE_TYPE_SET[SCENE_TYPE_NUM] = {"SCENE_MUSIC", "SCENE_GAME", "SCENE_MOVIE", "SCENE_SPEECH", "SCENE_RING",
    "SCENE_OTHERS", "EFFECT_NONE"};

enum {
    HDI_INIT,
    HDI_DEINIT,
    HDI_START,
    HDI_STOP,
    HDI_RENDER,
    QUIT,
    HDI_RENDER_MEDIA,
    HDI_RENDER_NAVIGATION,
    HDI_RENDER_COMMUNICATION
};

static enum AudioSampleFormatIntf ConvertPaToHdiAdapterFormat(pa_sample_format_t format)
{
    enum AudioSampleFormatIntf adapterFormat;
    switch (format) {
        case PA_SAMPLE_U8:
            adapterFormat = SAMPLE_U8;
            break;
        case PA_SAMPLE_S16LE:
            adapterFormat = SAMPLE_S16;
            break;
        case PA_SAMPLE_S24LE:
            adapterFormat = SAMPLE_S24;
            break;
        case PA_SAMPLE_S32LE:
            adapterFormat = SAMPLE_S32;
            break;
        default:
            adapterFormat = INVALID_WIDTH;
            break;
    }

    return adapterFormat;
}

static void ConvertToSplitArr(const char *str)
{
    for (int i = 0; i < MAX_PARTS; ++i) {
        g_splitArr[i] = NULL;
    }
    char *token;
    char *copy = strdup(str);
    CHECK_AND_RETURN_LOG(copy != NULL, "copy is null");
    int count = 0;
    token = strtok(copy, ":");
    while (token != NULL && count < MAX_PARTS) {
        g_splitArr[count] = (char *)malloc(strlen(token) + 1);
        if (g_splitArr[count] != NULL) {
            if (strcpy_s(g_splitArr[count], strlen(token) + 1, token) != 0) {
                AUDIO_ERR_LOG("strcpy_s failed.");
            };
            count++;
        } else {
            AUDIO_ERR_LOG("Memory allocation failed.\n");
            break;
        }
        token = strtok(NULL, ":");
    }
    g_splitNums = count;
    free(copy);
}

static int SinkProcessMsg(pa_msgobject *o, int code, void *data, int64_t offset, pa_memchunk *chunk)
{
    switch (code) {
        case PA_SINK_MESSAGE_GET_LATENCY:
            *((int64_t*) data) = 0;
            return 0;
        default:
            break;
    }

    return pa_sink_process_msg(o, code, data, offset, chunk);
}

/* Called from the IO thread. */
static int SinkSetStateInIoThreadCb(pa_sink *s, pa_sink_state_t new_state, pa_suspend_cause_t new_suspend_cause)
{
    struct userdata *u;

    CHECK_AND_RETURN_RET_LOG(s != NULL, -1, "s is null");
    pa_assert_se(u = s->userdata);
    CHECK_AND_RETURN_RET_LOG(u != NULL, -1, "u is null");

    if (s->thread_info.state == PA_SINK_SUSPENDED || s->thread_info.state == PA_SINK_INIT) {
        if (PA_SINK_IS_OPENED(new_state)) {
            u->timestamp = pa_rtclock_now();
        }
    }

    return 0;
}

static void SinkUpdateRequestedLatencyCb(pa_sink *s)
{
    struct userdata *u;
    size_t nbytes;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);

    u->block_usec = pa_sink_get_requested_latency_within_thread(s);

    if (u->block_usec == (pa_usec_t) -1) {
        u->block_usec = s->thread_info.max_latency;
    }

    nbytes = pa_usec_to_bytes(u->block_usec, &s->sample_spec);
    pa_sink_set_max_rewind_within_thread(s, nbytes);
    pa_sink_set_max_request_within_thread(s, nbytes);
}

static void SinkReconfigureCb(pa_sink *s, pa_sample_spec *spec, bool passthrough)
{
    s->sample_spec = *spec;
}

static bool SinkSetFormatsCb(pa_sink *s, pa_idxset *formats)
{
    CHECK_AND_RETURN_RET_LOG(s != NULL, false, "s is null");
    CHECK_AND_RETURN_RET_LOG(formats != NULL, false, "formats is null");
    struct userdata *u = s->userdata;

    CHECK_AND_RETURN_RET_LOG(u != NULL, false, "u is null");

    pa_idxset_free(u->formats, (pa_free_cb_t) pa_format_info_free);
    u->formats = pa_idxset_copy(formats, (pa_copy_func_t) pa_format_info_copy);

    return true;
}

static pa_idxset* SinkGetFormatsCb(pa_sink *s)
{
    CHECK_AND_RETURN_RET_LOG(s != NULL, NULL, "s is null");
    struct userdata *u = s->userdata;

    CHECK_AND_RETURN_RET_LOG(u != NULL, NULL, "u is null");

    return pa_idxset_copy(u->formats, (pa_copy_func_t) pa_format_info_copy);
}

static void ProcessRewind(struct userdata *u, pa_usec_t now)
{
    size_t rewindNbytes;
    size_t inBuffer;
    pa_usec_t delay;

    CHECK_AND_RETURN_LOG(u != NULL, "u is null");

    rewindNbytes = u->sink->thread_info.rewind_nbytes;
    if (!PA_SINK_IS_OPENED(u->sink->thread_info.state) || rewindNbytes == 0) {
        goto do_nothing;
    }
    AUDIO_DEBUG_LOG("Requested to rewind %lu bytes.", (unsigned long) rewindNbytes);

    if (u->timestamp <= now) {
        goto do_nothing;
    }

    delay = u->timestamp - now;
    inBuffer = pa_usec_to_bytes(delay, &u->sink->sample_spec);
    if (inBuffer == 0) {
        goto do_nothing;
    }

    if (rewindNbytes > inBuffer) {
        rewindNbytes = inBuffer;
    }

    pa_sink_process_rewind(u->sink, rewindNbytes);
    u->timestamp -= pa_bytes_to_usec(rewindNbytes, &u->sink->sample_spec);

    AUDIO_DEBUG_LOG("Rewound %lu bytes.", (unsigned long) rewindNbytes);
    return;

do_nothing:
    pa_sink_process_rewind(u->sink, 0);
}

static void StartSplitStreamHdiIfRunning(struct userdata *u)
{
    AUTO_CTRACE("split_stream_sink::StartPrimaryHdiIfRunning");
    if (u->isHDISinkStarted) {
        return;
    }

    if (u->sinkAdapter->SinkAdapterStart(u->sinkAdapter)) {
        AUDIO_ERR_LOG("split_stream_sink,audiorenderer control start failed!");
        u->sinkAdapter->SinkAdapterDeInit(u->sinkAdapter);
    } else {
        u->isHDISinkStarted = true;
        u->writeCount = 0;
        u->renderCount = 0;
        AUDIO_INFO_LOG("StartPrimaryHdiIfRunning, Successfully restarted HDI renderer");
        u->renderInIdleState = 1;
    }
}

static void SplitSinkRenderInputsDrop(pa_sink *si, pa_mix_info *infoIn, unsigned n, pa_memchunk *chunkIn)
{
    CHECK_AND_RETURN_LOG(si != NULL, "s is null");
    pa_sink_assert_io_context(si);
    CHECK_AND_RETURN_LOG(chunkIn != NULL, "chunkIn is null");
    CHECK_AND_RETURN_LOG(chunkIn->memblock != NULL, "chunkIn->memblock is null");
    CHECK_AND_RETURN_LOG(chunkIn->length > 0, "chunkIn->length < 0");
    AUTO_CTRACE("split_stream_sink::SplitSinkRenderInputsDrop:%u:len:%zu", n, chunkIn->length);

    /* We optimize for the case where the order of the inputs has not changed */

    pa_mix_info *infoCur = NULL;
    pa_sink_input *sinkInput = NULL;
    for (uint32_t k = 0; k < n; k++) {
        sinkInput = infoIn[k].userdata;
        pa_sink_input_assert_ref(sinkInput);
        AUTO_CTRACE("hdi_sink::InnerCap:pa_sink_input_drop:%u:len:%zu", sinkInput->index, chunkIn->length);
        pa_sink_input_drop(sinkInput, chunkIn->length);

        infoCur = infoIn + k;
        if (infoCur) {
            if (infoCur->chunk.memblock) {
                pa_memblock_unref(infoCur->chunk.memblock);
                pa_memchunk_reset(&infoCur->chunk);
            }

            pa_sink_input_unref(infoCur->userdata);
        }
    }
}

static int IsPeekCurrentSinkInput(char *streamType, const char *usageStr)
{
    CHECK_AND_RETURN_RET_LOG(usageStr != NULL, -1, "usageStr is null");
    int flag = 0;
    if (g_splitNums == SPLIT_ONE_STREAM) {
        flag = 1;
    }

    if (g_splitNums == SPLIT_TWO_STREAM) {
        if (strcmp(usageStr, STREAM_TYPE_NAVIGATION) && !strcmp(streamType, STREAM_TYPE_MEDIA)) {
            flag = 1;
        } else if (!strcmp(usageStr, STREAM_TYPE_NAVIGATION) && !strcmp(streamType, STREAM_TYPE_NAVIGATION)) {
            flag = 1;
        }
    }

    if (g_splitNums == SPLIT_THREE_STREAM) {
        if (strcmp(usageStr, STREAM_TYPE_NAVIGATION) && strcmp(usageStr, STREAM_TYPE_COMMUNICATION) &&
            strcmp(usageStr, STREAM_TYPE_VIDEO_COMMUNICATION) && !strcmp(streamType, STREAM_TYPE_MEDIA)) {
            flag = 1;
        } else if (!strcmp(usageStr, STREAM_TYPE_NAVIGATION) && !strcmp(streamType, STREAM_TYPE_NAVIGATION)) {
            flag = 1;
        } else if ((!strcmp(usageStr, STREAM_TYPE_COMMUNICATION) ||
            !strcmp(usageStr, STREAM_TYPE_VIDEO_COMMUNICATION)) &&
            !strcmp(streamType, STREAM_TYPE_COMMUNICATION)) {
            flag = 1;
        }
    }

    return flag;
}

static const char *SafeProplistGets(const pa_proplist *p, const char *key, const char *defstr)
{
    const char *res = pa_proplist_gets(p, key);
    if (res == NULL) {
        return defstr;
    }
    return res;
}

static void ProcessAudioVolume(pa_sink_input *sinkIn, size_t length, pa_memchunk *pchunk, pa_sink *si)
{
    AUTO_CTRACE("module_split_stream_sink::ProcessAudioVolume: len:%zu", length);
    struct userdata *u;
    if (sinkIn == NULL || pchunk == NULL || si == NULL) {
        AUDIO_ERR_LOG("Null pointer");
        return;
    }
    pa_assert_se(u = si->userdata);
    const char *streamType = SafeProplistGets(sinkIn->proplist, "stream.type", "NULL");
    const char *sessionIDStr = SafeProplistGets(sinkIn->proplist, "stream.sessionID", "NULL");
    const char *deviceClass = u->sinkAdapter->deviceClass;
    uint32_t sessionID = sessionIDStr != NULL ? (uint32_t)atoi(sessionIDStr) : 0;
    struct VolumeValues volumes = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
    float volumeEnd = GetCurVolume(sessionID, streamType, deviceClass, &volumes);
    float volumeBeg = volumes.volumeHistory;
    if (pa_memblock_is_silence(pchunk->memblock)) {
        AUTO_CTRACE("module_split_stream_sink::ProcessAudioVolume: is_silence");
        AUDIO_PRERELEASE_LOGI("pa_memblock_is_silence");
    } else {
        AudioRawFormat rawFormat;
        rawFormat.format = (uint32_t)ConvertPaToHdiAdapterFormat(si->sample_spec.format);
        rawFormat.channels = (uint32_t)si->sample_spec.channels;

        pa_memchunk_make_writable(pchunk, 0);
        void *data = pa_memblock_acquire_chunk(pchunk);

        AUDIO_DEBUG_LOG("length:%{public}zu channels:%{public}d format:%{public}d"
            " volumeBeg:%{public}f, volumeEnd:%{public}f",
            length, rawFormat.channels, rawFormat.format, volumeBeg, volumeEnd);
        int32_t ret = ProcessVol(data, length, rawFormat, volumeBeg, volumeEnd);
        if (ret != 0) {
            AUDIO_WARNING_LOG("ProcessVol failed:%{public}d", ret);
        }
        pa_memblock_release(pchunk->memblock);
    }
    if (volumeBeg != volumeEnd) {
        AUDIO_INFO_LOG("sessionID:%{public}u, volumeBeg:%{public}f, volumeEnd:%{public}f",
            sessionID, volumeBeg, volumeEnd);
        SetPreVolume(sessionID, volumeEnd);
        MonitorVolume(sessionID, true);
    }
}

static unsigned SplitFillMixInfo(pa_sink *s, size_t *length, pa_mix_info *info, unsigned maxInfo, char *streamType)
{
    CHECK_AND_RETURN_RET_LOG(s != NULL, 0, "s is null");
    CHECK_AND_RETURN_RET_LOG(length != NULL, 0, "length is null");
    AUTO_CTRACE("split_stream_sink::SplitFillMixInfo:len:%zu", *length);
    pa_sink_input *i;
    unsigned n = 0;
    void *state = NULL;
    size_t mixlength = *length;

    pa_sink_assert_ref(s);
    pa_sink_assert_io_context(s);
    CHECK_AND_RETURN_RET_LOG(info != NULL, 0, "info is null");

    while ((i = pa_hashmap_iterate(s->thread_info.inputs, &state, NULL)) && maxInfo > 0) {
        const char *usageStr = pa_proplist_gets(i->proplist, "stream.usage");
        AUDIO_DEBUG_LOG("splitFillMixInfo usageStr = %{public}s, streamType = %{public}s", usageStr, streamType);
        if (IsPeekCurrentSinkInput(streamType, usageStr)) {
            pa_sink_input_assert_ref(i);

            AUTO_CTRACE("module_split_stream_sink::splitFillMixInfo::pa_sink_input_peek:%u len:%zu", i->index, *length);
            pa_sink_input_peek(i, *length, &info->chunk, &info->volume);

            if (mixlength == 0 || info->chunk.length < mixlength)
                mixlength = info->chunk.length;

            ProcessAudioVolume(i, mixlength, &info->chunk, s);

            if (pa_memblock_is_silence(info->chunk.memblock)) {
                pa_memblock_unref(info->chunk.memblock);
                continue;
            }

            info->userdata = pa_sink_input_ref(i);
            pa_assert(info->chunk.memblock);
            pa_assert(info->chunk.length > 0);

            info++;
            n++;
            maxInfo--;

            if (mixlength > 0) {
                *length = mixlength;
            }
        }
    }
    return n;
}

static void SplitSinkRenderMix(pa_sink *s, size_t length, pa_mix_info *info, unsigned n, pa_memchunk *result)
{
    CHECK_AND_RETURN_LOG(s != NULL, "s is null");
    CHECK_AND_RETURN_LOG(info != NULL, "info is null");
    if (n == 0) {
        *result = s->silence;
        pa_memblock_ref(result->memblock);

        if (result->length > length)
            result->length = length;
    } else if (n == 1) {
        pa_cvolume volume;

        *result = info[0].chunk;
        pa_memblock_ref(result->memblock);

        if (result->length > length)
            result->length = length;

        pa_sw_cvolume_multiply(&volume, &s->thread_info.soft_volume, &info[0].volume);

        if (s->thread_info.soft_muted || pa_cvolume_is_muted(&volume)) {
            pa_memblock_unref(result->memblock);
            pa_silence_memchunk_get(
                &s->core->silence_cache, s->core->mempool, result, &s->sample_spec, result->length);
        } else if (!pa_cvolume_is_norm(&volume)) {
            pa_memchunk_make_writable(result, 0);
            pa_volume_memchunk(result, &s->sample_spec, &volume);
        }
    } else {
        void *ptr;
        result->memblock = pa_memblock_new(s->core->mempool, length);

        ptr = pa_memblock_acquire(result->memblock);
        result->length =
            pa_mix(info, n, ptr, length, &s->sample_spec, &s->thread_info.soft_volume, s->thread_info.soft_muted);
        pa_memblock_release(result->memblock);

        result->index = 0;
    }
}

static unsigned SplitPaSinkRender(pa_sink *s, size_t length, pa_memchunk *result, char *streamType)
{
    AUTO_CTRACE("module_split_stream_sink::SplitPaSinkRender:len:%zu", length);
    unsigned streamCount = 0;
    pa_mix_info info[MAX_MIX_CHANNELS];
    unsigned n;
    size_t blockSizeMax;

    CHECK_AND_RETURN_RET_LOG(s != NULL, 0, "s is null");
    pa_sink_assert_io_context(s);
    pa_assert(PA_SINK_IS_LINKED(s->thread_info.state));
    pa_assert(pa_frame_aligned(length, &s->sample_spec));
    CHECK_AND_RETURN_RET_LOG(result != NULL, 0, "result is null");

    pa_assert(!s->thread_info.rewind_requested);
    pa_assert(s->thread_info.rewind_nbytes == 0);

    if (s->thread_info.state == PA_SINK_SUSPENDED) {
        result->memblock = pa_memblock_ref(s->silence.memblock);
        result->index = s->silence.index;
        result->length = PA_MIN(s->silence.length, length);
        return 0;
    }

    pa_sink_ref(s);

    AUDIO_DEBUG_LOG("module_split_stream_sink, splitSinkRender in  length = %{public}zu", length);
    if (length == 0) {
        length = pa_frame_align(MIX_BUFFER_LENGTH, &s->sample_spec);
    }

    blockSizeMax = pa_mempool_block_size_max(s->core->mempool);
    if (length > blockSizeMax)
        length = pa_frame_align(blockSizeMax, &s->sample_spec);

    pa_assert(length > 0);

    n = SplitFillMixInfo(s, &length, info, MAX_MIX_CHANNELS, streamType);
    streamCount = n;
    SplitSinkRenderMix(s, length, info, n, result);

    SplitSinkRenderInputsDrop(s, info, n, result);

    pa_sink_unref(s);
    return streamCount;
}

static void SplitSinkRenderIntoMix(pa_sink *s, size_t length, pa_mix_info *info, unsigned n, pa_memchunk *target)
{
    if (n == 0) {
        if (target->length > length)
            target->length = length;

        pa_silence_memchunk(target, &s->sample_spec);
    } else if (n == 1) {
        pa_cvolume volume;

        if (target->length > length)
            target->length = length;

        pa_sw_cvolume_multiply(&volume, &s->thread_info.soft_volume, &info[0].volume);

        if (s->thread_info.soft_muted || pa_cvolume_is_muted(&volume)) {
            pa_silence_memchunk(target, &s->sample_spec);
        } else {
            pa_memchunk vChunk;

            vChunk = info[0].chunk;
            pa_memblock_ref(vChunk.memblock);

            if (vChunk.length > length)
                vChunk.length = length;

            if (!pa_cvolume_is_norm(&volume)) {
                pa_memchunk_make_writable(&vChunk, 0);
                pa_volume_memchunk(&vChunk, &s->sample_spec, &volume);
            }

            pa_memchunk_memcpy(target, &vChunk);
            pa_memblock_unref(vChunk.memblock);
        }
    } else {
        void *ptr;

        ptr = pa_memblock_acquire(target->memblock);

        target->length = pa_mix(info, n,
                                (uint8_t*) ptr + target->index, length,
                                &s->sample_spec,
                                &s->thread_info.soft_volume,
                                s->thread_info.soft_muted);

        pa_memblock_release(target->memblock);
    }
}

static void  SplitPaSinkRenderInto(pa_sink *s, pa_memchunk *target, char *streamType)
{
    pa_mix_info info[MAX_MIX_CHANNELS];
    unsigned n;
    size_t length;
    size_t blockSizeMax;

    pa_sink_assert_ref(s);
    pa_sink_assert_io_context(s);
    pa_assert(PA_SINK_IS_LINKED(s->thread_info.state));
    pa_assert(pa_frame_aligned(target->length, &s->sample_spec));

    pa_assert(!s->thread_info.rewind_requested);
    pa_assert(s->thread_info.rewind_nbytes == 0);

    if (s->thread_info.state == PA_SINK_SUSPENDED) {
        pa_silence_memchunk(target, &s->sample_spec);
        return;
    }

    pa_sink_ref(s);

    length = target->length;
    blockSizeMax = pa_mempool_block_size_max(s->core->mempool);
    if (length > blockSizeMax)
        length = pa_frame_align(blockSizeMax, &s->sample_spec);

    pa_assert(length > 0);

    n = SplitFillMixInfo(s, &length, info, MAX_MIX_CHANNELS, streamType);
    SplitSinkRenderIntoMix(s, length, info, n, target);

    SplitSinkRenderInputsDrop(s, info, n, target);

    pa_sink_unref(s);
}

static void SplitPaSinkRenderIntoFull(pa_sink *s, pa_memchunk *target, char *streamType)
{
    pa_memchunk chunk;
    size_t l;
    size_t d;

    CHECK_AND_RETURN_LOG(s != NULL, "s is null");
    pa_sink_assert_io_context(s);
    CHECK_AND_RETURN_LOG(target != NULL, "target is null");
    CHECK_AND_RETURN_LOG(target->memblock != NULL, "target->memblock is null");
    CHECK_AND_RETURN_LOG(target->length > 0, "target->length < 0");
    pa_assert(pa_frame_aligned(target->length, &s->sample_spec));

    if (s->thread_info.state == PA_SINK_SUSPENDED) {
        pa_silence_memchunk(target, &s->sample_spec);
        return;
    }

    pa_sink_ref(s);

    l = target->length;
    d = 0;
    while (l > 0) {
        chunk = *target;
        chunk.index += d;
        chunk.length -=d;

        SplitPaSinkRenderInto(s, &chunk, streamType);

        d += chunk.length;
        l -= chunk.length;
    }

    pa_sink_unref(s);
}

static unsigned SplitPaSinkRenderFull(pa_sink *s, size_t length, pa_memchunk *result, char *streamType)
{
    unsigned nSink;
    pa_sink_assert_ref(s);
    pa_sink_assert_io_context(s);
    pa_assert(PA_SINK_IS_LINKED(s->thread_info.state));
    pa_assert(length > 0);
    pa_assert(pa_frame_aligned(length, &s->sample_spec));
    CHECK_AND_RETURN_RET_LOG(result != NULL, 0, "result is null");
    
    pa_assert(!s->thread_info.rewind_requested);
    pa_assert(s->thread_info.rewind_nbytes == 0);

    if (s->thread_info.state == PA_SINK_SUSPENDED) {
        result->memblock = pa_memblock_ref(s->silence.memblock);
        result->index = s->silence.index;
        result->length = PA_MIN(s->silence.length, length);
        return 0;
    }

    pa_sink_ref(s);

    AUDIO_DEBUG_LOG("module_split_stream_sink, splitSinkRender in  length = %{public}zu", length);
    nSink = SplitPaSinkRender(s, length, result, streamType);
    if (nSink == 0) {
        return nSink;
    }

    if (result->length < length) {
        pa_memchunk chunk;

        pa_memchunk_make_writable(result, length);

        chunk.memblock = result->memblock;
        chunk.index = result->index + result->length;
        chunk.length = length - result->length;

        SplitPaSinkRenderIntoFull(s, &chunk, streamType);

        result->length = length;
    }

    pa_sink_unref(s);
    return nSink;
}

static void SendStreamData(struct userdata *u, int num, pa_memchunk chunk)
{
    if (num < 0 || num >= g_splitNums) {
        return;
    }
    // start hdi
    StartSplitStreamHdiIfRunning(u);
    // send msg post data
    if (!strcmp(g_splitArr[num], STREAM_TYPE_NAVIGATION)) {
        pa_asyncmsgq_post(u->dq, NULL, HDI_RENDER_NAVIGATION, NULL, 0, &chunk, NULL);
    } else if (!strcmp(g_splitArr[num], STREAM_TYPE_COMMUNICATION)) {
        pa_asyncmsgq_post(u->dq, NULL, HDI_RENDER_COMMUNICATION, NULL, 0, &chunk, NULL);
    } else {
        pa_asyncmsgq_post(u->dq, NULL, HDI_RENDER_MEDIA, NULL, 0, &chunk, NULL);
    }
}

static void ProcessRender(struct userdata *u, pa_usec_t now)
{
    AUTO_CTRACE("module_split_stream_sink: ProcessRender");

    CHECK_AND_RETURN_LOG(u != NULL, "u is null");

    /* Fill the buffer up the latency size */
    int count = 0;
    for (int i = 0; i < g_splitNums; i++) {
        AUTO_CTRACE("module_split_stream_sink::ProcessRender:streamType:%s", g_splitArr[i]);
        AUDIO_DEBUG_LOG("module_split_stream_sink: ProcessRender:streamType:%{public}s", g_splitArr[i]);
        
        pa_memchunk chunk;
        unsigned chunkIsNull = 0;
        chunkIsNull = SplitPaSinkRenderFull(u->sink, u->sink->thread_info.max_request, &chunk, g_splitArr[i]);
        if (chunkIsNull == 0) {
            count++;
            if (count != g_splitNums) {
                continue;
            }
            for (int j = 0; j < g_splitNums; j++) {
                SendStreamData(u, j, chunk);
            }
            break;
        }
        SendStreamData(u, i, chunk);
    }
    u->timestamp += pa_bytes_to_usec(u->sink->thread_info.max_request, &u->sink->sample_spec);
}

static bool MonitorLinkedState(pa_sink *si, bool isRunning)
{
    if (isRunning) {
        return si->monitor_source && PA_SOURCE_IS_RUNNING(si->monitor_source->thread_info.state);
    } else {
        return si->monitor_source && PA_SOURCE_IS_LINKED(si->monitor_source->thread_info.state);
    }
}

static void ThreadFunc(void *userdata)
{
    ScheduleReportData(getpid(), gettid(), "audio_server");
    struct userdata *u = userdata;
    CHECK_AND_RETURN_LOG(u != NULL, "u is null");
    AUDIO_DEBUG_LOG("Thread starting up");
    if (u->core->realtime_scheduling) {
        pa_thread_make_realtime(u->core->realtime_priority);
    }
    pa_thread_mq_install(&u->thread_mq);
    u->timestamp = pa_rtclock_now();
    for (;;) {
        pa_usec_t now = 0;
        int ret;

        if (PA_SINK_IS_OPENED(u->sink->thread_info.state)) {
            now = pa_rtclock_now();
        }
        
        bool flag = (((u->renderInIdleState && PA_SINK_IS_OPENED(u->sink->thread_info.state)) ||
            (!u->renderInIdleState && PA_SINK_IS_RUNNING(u->sink->thread_info.state))) &&
            !(u->sink->thread_info.state == PA_SINK_IDLE && u->previousState == PA_SINK_SUSPENDED) &&
            !(u->sink->thread_info.state == PA_SINK_IDLE && u->previousState == PA_SINK_INIT)) ||
            (u->sink->thread_info.state == PA_SINK_IDLE && MonitorLinkedState(u->sink, true));
        if (flag) {
            now = pa_rtclock_now();
        }

        if (PA_UNLIKELY(u->sink->thread_info.rewind_requested)) {
            ProcessRewind(u, now);
        }

        /* Render some data and drop it immediately */
        if (flag) {
            if (u->timestamp <= now) {
                ProcessRender(u, now);
            }
            pa_rtpoll_set_timer_absolute(u->rtpoll, u->timestamp);
        } else {
            pa_rtpoll_set_timer_disabled(u->rtpoll);
        }
        /* Hmm, nothing to do. Let's sleep */
        if ((ret = pa_rtpoll_run(u->rtpoll)) < 0) {
            goto fail;
        }
        if (ret == 0) {
            goto finish;
        }
    }

fail:
    /* If this was no regular exit from the loop we have to continue
     * processing messages until we received PA_MESSAGE_SHUTDOWN */
    pa_asyncmsgq_post(u->thread_mq.outq, PA_MSGOBJECT(u->core),
        PA_CORE_MESSAGE_UNLOAD_MODULE, u->module, 0, NULL, NULL);
    pa_asyncmsgq_wait_for(u->thread_mq.inq, PA_MESSAGE_SHUTDOWN);

finish:
    AUDIO_DEBUG_LOG("Thread shutting down");
}

static void ProcessSplitHdiRender(struct userdata *u, pa_memchunk *chunk, char *streamType)
{
    pa_usec_t now = pa_rtclock_now();
    if (!u->isHDISinkStarted && now - u->timestampLastLog > USEC_PER_SEC) {
        u->timestampLastLog = now;
        const char *deviceClass = u->sinkAdapter->deviceClass;
        AUDIO_DEBUG_LOG("HDI not started, skip RenderWrite, wait sink[%s] suspend", deviceClass);
        pa_memblock_unref(chunk->memblock);
    } else if (!u->isHDISinkStarted) {
        pa_memblock_unref(chunk->memblock);
    } else if (SplitRenderWrite(u->sinkAdapter, chunk, streamType) < 0) {
        u->bytesDropped += chunk->length;
        AUDIO_ERR_LOG("RenderWrite failed");
    }
    if (pa_atomic_load(&u->dflag) == 1) {
        pa_atomic_sub(&u->dflag, 1);
    }
    u->writeTime = pa_rtclock_now() - now;
}

static void ThreadFuncWriteHDI(void *userdata)
{
    // set audio thread priority
    ScheduleReportData(getpid(), gettid(), "pulseaudio");

    struct userdata *u = userdata;
    CHECK_AND_RETURN_LOG(u != NULL, "u is null");

    int32_t quit = 0;

    do {
        int32_t code = 0;
        pa_memchunk chunk;

        pa_assert_se(pa_asyncmsgq_get(u->dq, NULL, &code, NULL, NULL, &chunk, 1) == 0);

        switch (code) {
            case HDI_RENDER_MEDIA: {
                ProcessSplitHdiRender(u, &chunk, STREAM_TYPE_MEDIA);
                break;
            }
            case HDI_RENDER_COMMUNICATION: {
                ProcessSplitHdiRender(u, &chunk, STREAM_TYPE_COMMUNICATION);
                break;
            }
            case HDI_RENDER_NAVIGATION: {
                ProcessSplitHdiRender(u, &chunk, STREAM_TYPE_NAVIGATION);
                break;
            }
            case QUIT:
                quit = 1;
                break;
            default:
                break;
        }
        pa_asyncmsgq_done(u->dq, 0);
    } while (!quit);
}

static ssize_t SplitRenderWrite(struct SinkAdapter *sinkAdapter, pa_memchunk *pchunk, char *streamType)
{
    size_t index;
    size_t length;
    ssize_t count = 0;
    void *p = NULL;

    CHECK_AND_RETURN_RET_LOG(pchunk != NULL, 0, "pchunk is null");

    index = pchunk->index;
    length = pchunk->length;
    p = pa_memblock_acquire(pchunk->memblock);
    CHECK_AND_RETURN_RET_LOG(p != NULL, 0, "p is null");

    while (true) {
        uint64_t writeLen = 0;

        int32_t ret = sinkAdapter->SinkAdapterSplitRenderFrame(sinkAdapter, ((char*)p + index),
            (uint64_t)length, &writeLen, streamType);
        if (writeLen > length) {
            AUDIO_ERR_LOG("Error writeLen > actual bytes. Length: %zu, Written: %" PRIu64 " bytes, %d ret",
                         length, writeLen, ret);
            count = -1 - count;
            break;
        }
        if (writeLen == 0) {
            AUDIO_ERR_LOG("Failed to render Length: %{public}zu, Written: %{public}" PRIu64 " bytes, %{public}d ret",
                length, writeLen, ret);
            count = -1 - count;
            break;
        } else {
            count += (ssize_t)writeLen;
            index += writeLen;
            length -= writeLen;
            if (length == 0) {
                break;
            }
        }
    }
    pa_memblock_release(pchunk->memblock);
    pa_memblock_unref(pchunk->memblock);

    return count;
}

static int CreateSink(pa_module *m, pa_modargs *ma, struct userdata *u)
{
    pa_sample_spec ss;
    pa_channel_map map;
    pa_sink_new_data data;
    pa_format_info *format;

    CHECK_AND_RETURN_RET_LOG(m != NULL, -1, "m is null");

    ss = m->core->default_sample_spec;
    map = m->core->default_channel_map;
    if (pa_modargs_get_sample_spec_and_channel_map(ma, &ss, &map, PA_CHANNEL_MAP_DEFAULT) < 0) {
        AUDIO_ERR_LOG("Invalid sample format specification or channel map");
        return PA_ERR;
    }

    pa_sink_new_data_init(&data);
    data.driver = __FILE__;
    data.module = m;
    pa_sink_new_data_set_name(&data, pa_modargs_get_value(ma, "sink_name", DEFAULT_SINK_NAME));
    pa_sink_new_data_set_sample_spec(&data, &ss);
    pa_sink_new_data_set_channel_map(&data, &map);
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_DESCRIPTION, _("Split Stream Output"));
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_STRING, "splitStream");

    u->formats = pa_idxset_new(NULL, NULL);
    format = pa_format_info_new();
    format->encoding = PA_ENCODING_PCM;
    pa_idxset_put(u->formats, format, NULL);

    if (pa_modargs_get_proplist(ma, "sink_properties", data.proplist, PA_UPDATE_REPLACE) < 0) {
        AUDIO_ERR_LOG("Invalid properties");
        pa_sink_new_data_done(&data);
        return PA_ERR;
    }

    u->sink = pa_sink_new(m->core, &data, PA_SINK_LATENCY | PA_SINK_DYNAMIC_LATENCY);
    pa_sink_new_data_done(&data);

    if (!u->sink) {
        AUDIO_ERR_LOG("Failed to create sink.");
        return PA_ERR;
    }

    u->sink->parent.process_msg = SinkProcessMsg;
    u->sink->set_state_in_io_thread = SinkSetStateInIoThreadCb;
    u->sink->update_requested_latency = SinkUpdateRequestedLatencyCb;
    u->sink->reconfigure = SinkReconfigureCb;
    u->sink->get_formats = SinkGetFormatsCb;
    u->sink->set_formats = SinkSetFormatsCb;
    u->sink->userdata = u;

    return 0;
}

static int32_t InitRemoteSink(struct userdata *u, const char *filePath)
{
    struct SinkAdapterAttr sample_attrs;
    int32_t ret;

    sample_attrs.format = ConvertPaToHdiAdapterFormat(u->ss.format);
    sample_attrs.adapterName = u->adapterName;
    sample_attrs.openMicSpeaker = u->open_mic_speaker;
    sample_attrs.sampleRate = (uint32_t) u->ss.rate;
    sample_attrs.channel = u->ss.channels;
    sample_attrs.volume = MAX_SINK_VOLUME_LEVEL;
    sample_attrs.filePath = filePath;
    sample_attrs.deviceNetworkId = u->deviceNetworkId;
    sample_attrs.deviceType =  u->deviceType;
    sample_attrs.aux =  SPLIT_MODE;
    
    ret = u->sinkAdapter->SinkAdapterInit(u->sinkAdapter, &sample_attrs);
    if (ret != 0) {
        AUDIO_ERR_LOG("audiorenderer Init failed!");
        return -1;
    }

    return 0;
}

static void UserdataFree(struct userdata *u)
{
    if (u->sink) {
        pa_sink_unlink(u->sink);
    }

    if (u->thread) {
        pa_asyncmsgq_send(u->thread_mq.inq, NULL, PA_MESSAGE_SHUTDOWN, NULL, 0, NULL);
        pa_thread_free(u->thread);
    }

    if (u->sinkAdapter) {
        u->sinkAdapter->SinkAdapterStop(u->sinkAdapter);
        u->sinkAdapter->SinkAdapterDeInit(u->sinkAdapter);
        ReleaseSinkAdapter(u->sinkAdapter);
        u->sinkAdapter = NULL;
    }

    pa_thread_mq_done(&u->thread_mq);

    if (u->sink) {
        pa_sink_unref(u->sink);
    }

    if (u->rtpoll) {
        pa_rtpoll_free(u->rtpoll);
    }

    if (u->formats) {
        pa_idxset_free(u->formats, (pa_free_cb_t)pa_format_info_free);
    }

    pa_xfree(u);

    for (int32_t i = 0; i < MAX_PARTS; ++i) {
        if (g_splitArr[i] == NULL) {
            continue;
        }
        free(g_splitArr[i]);
        g_splitArr[i] = NULL;
    }
}

static int InitFailed(pa_module *m, pa_modargs *ma)
{
    AUDIO_ERR_LOG("Split Stream Sink Init Failed");
    UserdataFree(m->userdata);
    m->userdata = NULL;
    if (ma)
        pa_modargs_free(ma);

    pa__done(m);

    return PA_ERR;
}

static int32_t PaHdiSinkNewInit(pa_module *m, pa_modargs *ma, struct userdata *u)
{
    size_t nbytes;
    int mg;
    pa_sink_set_asyncmsgq(u->sink, u->thread_mq.inq);
    pa_sink_set_rtpoll(u->sink, u->rtpoll);

    u->buffer_size = DEFAULT_BUFFER_SIZE;

    mg = pa_modargs_get_value_u32(ma, "buffer_size", &u->buffer_size);
    CHECK_AND_RETURN_RET_LOG(mg >= 0, PA_ERR,
        "Failed to parse buffer_size arg in capturer sink");

    u->block_usec = pa_bytes_to_usec(u->buffer_size, &u->sink->sample_spec);
    
    nbytes = pa_usec_to_bytes(u->block_usec, &u->sink->sample_spec);
    pa_sink_set_max_rewind(u->sink, nbytes);
    pa_sink_set_max_request(u->sink, u->buffer_size);

    if (u->fixed_latency) {
        pa_sink_set_fixed_latency(u->sink, u->block_usec);
    } else {
        pa_sink_set_latency_range(u->sink, 0, u->block_usec);
    }

    const char *deviceClass = pa_modargs_get_value(ma, "device_class", DEFAULT_DEVICE_CLASS);
    u->sinkAdapter = GetSinkAdapter(deviceClass, pa_modargs_get_value(ma, "network_id", DEFAULT_DEVICE_NETWORKID));
    if (u->sinkAdapter == NULL) {
        AUDIO_ERR_LOG("Load adapter failed");
        return -1;
    }

    if (pa_modargs_get_value_s32(ma, "device_type", &u->deviceType) < 0) {
        AUDIO_ERR_LOG("Failed to parse deviceType argument.");
        return -1;
    }

    u->adapterName = pa_modargs_get_value(ma, "adapter_name", DEFAULT_DEVICE_CLASS);
    u->deviceNetworkId = pa_modargs_get_value(ma, "network_id", DEFAULT_DEVICE_NETWORKID);

    u->ss = m->core->default_sample_spec;
    u->map = m->core->default_channel_map;
    if (pa_modargs_get_sample_spec_and_channel_map(ma, &u->ss, &u->map, PA_CHANNEL_MAP_DEFAULT) < 0) {
        AUDIO_ERR_LOG("Failed to parse sample specification and channel map");
        return -1;
    }

    if (InitRemoteSink(u, pa_modargs_get_value(ma, "file_path", "")) < 0) {
        AUDIO_ERR_LOG("Failed to init remote audio render sink.");
        return -1;
    }

    return 0;
}

int pa__init(pa_module *m)
{
    AUDIO_INFO_LOG("module_split_stream_sink pa__init start");
    struct userdata *u = NULL;
    pa_modargs *ma = NULL;
    int mq;

    CHECK_AND_RETURN_RET_LOG(m != NULL, -1, "m is null");

    ma = pa_modargs_new(m->argument, VALID_MODARGS);
    CHECK_AND_RETURN_RET_LOG(ma != NULL, InitFailed(m, ma), "Failed to parse module arguments:%{public}s", m->argument);

    SPLIT_MODE = pa_modargs_get_value(ma, "split_mode", "1");
    AUDIO_INFO_LOG("module_split_stream_sink pa__init splitMode is %{public}s", SPLIT_MODE);
    ConvertToSplitArr(SPLIT_MODE);

    m->userdata = u = pa_xnew0(struct userdata, 1);
    u->core = m->core;
    u->module = m;
    u->rtpoll = pa_rtpoll_new();

    mq = pa_thread_mq_init(&u->thread_mq, m->core->mainloop, u->rtpoll);
    CHECK_AND_RETURN_RET_LOG(mq >=0, InitFailed(m, ma), "pa_thread_mq_init() failed.");

    if (CreateSink(m, ma, u) != 0) {
        return InitFailed(m, ma);
    }

    if (PaHdiSinkNewInit(m, ma, u) < 0) {
        AUDIO_ERR_LOG("PaHdiSinkNewInit failed");
        return InitFailed(m, ma);
    }

    u->dq = pa_asyncmsgq_new(0);

    if (!(u->thread = pa_thread_new("OS_SplitStream", ThreadFunc, u))) {
        AUDIO_ERR_LOG("Failed to create thread.");
        return InitFailed(m, ma);
    }

    if (!(u->thread_split_hdi = pa_thread_new("OS_splitToHdi", ThreadFuncWriteHDI, u))) {
        AUDIO_ERR_LOG("Failed to create OS_splitToHdi.");
        return InitFailed(m, ma);
    }

    pa_sink_put(u->sink);

    pa_modargs_free(ma);

    return 0;
}

int pa__get_n_used(pa_module *m)
{
    struct userdata *u;

    CHECK_AND_RETURN_RET_LOG(m != NULL, 0, "m is null");
    pa_assert_se(u = m->userdata);

    return pa_sink_linked_by(u->sink);
}

void pa__done(pa_module*m)
{
    struct userdata *u;

    CHECK_AND_RETURN_LOG(m != NULL, "m is null");

    if (!(u = m->userdata)) {
        return;
    }
    UserdataFree(u);
    m->userdata = NULL;
}
