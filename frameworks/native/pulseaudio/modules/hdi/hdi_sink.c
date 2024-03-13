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

#include <config.h>
#include <pulse/rtclock.h>
#include <pulse/timeval.h>
#include <pulse/xmalloc.h>
#include <pulsecore/log.h>
#include <pulsecore/modargs.h>
#include <pulsecore/module.h>
#include <pulsecore/rtpoll.h>
#include <pulsecore/sink.h>
#include <pulsecore/thread-mq.h>
#include <pulsecore/thread.h>
#include <pulsecore/memblock.c>
#include <pulsecore/mix.h>
#include <pulse/volume.h>
#include <pulsecore/protocol-native.c>
#include <pulsecore/memblockq.c>

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <inttypes.h>
#include <sys/types.h>
#include <pthread.h>
#include <semaphore.h>

#include "securec.h"

#include "audio_log.h"
#include "audio_schedule.h"
#include "audio_utils_c.h"
#include "audio_hdiadapter_info.h"
#include "renderer_sink_adapter.h"
#include "audio_effect_chain_adapter.h"
#include "playback_capturer_adapter.h"


#define DEFAULT_SINK_NAME "hdi_output"
#define DEFAULT_AUDIO_DEVICE_NAME "Speaker"
#define DEFAULT_DEVICE_CLASS "primary"
#define DEFAULT_DEVICE_NETWORKID "LocalDevice"
#define DEFAULT_BUFFER_SIZE 8192
#define MAX_SINK_VOLUME_LEVEL 1.0
#define DEFAULT_WRITE_TIME 1000
#define MIX_BUFFER_LENGTH (pa_page_size())
#define MAX_MIX_CHANNELS 128
#define MAX_REWIND (7000 * PA_USEC_PER_MSEC)
#define USEC_PER_SEC 1000000
#define DEFAULT_IN_CHANNEL_NUM 2
#define PRIMARY_CHANNEL_NUM 2
#define IN_CHANNEL_NUM_MAX 16
#define OUT_CHANNEL_NUM_MAX 2
#define DEFAULT_FRAMELEN 2048
#define SCENE_TYPE_NUM 7
#define HDI_MIN_MS_MAINTAIN 30
#define OFFLOAD_HDI_CACHE1 200 // ms, should equal with val in audio_service_client.cpp
#define OFFLOAD_HDI_CACHE2 7000 // ms, should equal with val in audio_service_client.cpp
#define OFFLOAD_FRAME_SIZE 50
#define OFFLOAD_HDI_CACHE1_PLUS (OFFLOAD_HDI_CACHE1 + OFFLOAD_FRAME_SIZE + 5)   // ms, add 1 frame and 5ms
#define OFFLOAD_HDI_CACHE2_PLUS (OFFLOAD_HDI_CACHE2 + OFFLOAD_FRAME_SIZE + 5)   // to make sure get full
#define SPRINTF_STR_LEN 100
#define DEFAULT_MULTICHANNEL_NUM 6
#define DEFAULT_NUM_CHANNEL 2
#define DEFAULT_MULTICHANNEL_CHANNELLAYOUT 1551
#define DEFAULT_CHANNELLAYOUT 3

const char *DEVICE_CLASS_PRIMARY = "primary";
const char *DEVICE_CLASS_A2DP = "a2dp";
const char *DEVICE_CLASS_REMOTE = "remote";
const char *DEVICE_CLASS_OFFLOAD = "offload";
const char *DEVICE_CLASS_MULTICHANNEL = "multichannel";

char *const SCENE_TYPE_SET[SCENE_TYPE_NUM] = {"SCENE_MUSIC", "SCENE_GAME", "SCENE_MOVIE", "SCENE_SPEECH", "SCENE_RING",
    "SCENE_OTHERS", "EFFECT_NONE"};

enum HdiInputType { HDI_INPUT_TYPE_PRIMARY, HDI_INPUT_TYPE_OFFLOAD, HDI_INPUT_TYPE_MULTICHANNEL };

enum {
    HDI_INIT,
    HDI_DEINIT,
    HDI_START,
    HDI_STOP,
    HDI_RENDER,
    QUIT
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

struct Userdata {
    const char *adapterName;
    uint32_t buffer_size;
    uint32_t fixed_latency;
    uint32_t sink_latency;
    uint32_t render_in_idle_state;
    uint32_t open_mic_speaker;
    bool offload_enable;
    bool multichannel_enable;
    const char *deviceNetworkId;
    int32_t deviceType;
    size_t bytes_dropped;
    pa_thread_mq thread_mq;
    pa_memchunk memchunk;
    pa_usec_t block_usec;
    pa_thread *thread;
    pa_rtpoll *rtpoll;
    pa_core *core;
    pa_module *module;
    pa_sink *sink;
    pa_sample_spec ss;
    pa_channel_map map;
    bool test_mode_on;
    uint32_t writeCount;
    uint32_t renderCount;
    pa_sample_format_t format;
    BufferAttr *bufferAttr;
    int32_t processLen;
    size_t processSize;
    char *sinkSceneType;
    char *sinkSceneMode;
    bool spatialEnabled;
    pthread_mutex_t mutexPa;
    pthread_mutex_t mutexPa2;
    pthread_rwlock_t rwlockSleep;
    int64_t timestampSleep;
    pa_usec_t timestampLastLog;
    struct {
        int32_t sessionID;
        bool firstWrite;
        bool firstWriteHdi; // for set volume onstart, avoid mute
        pa_usec_t pos;
        pa_usec_t hdiPos;
        pa_usec_t hdiPosTs;
        pa_usec_t prewrite;
        pa_usec_t minWait;
        pa_thread *thread;
        pa_asyncmsgq *msgq;
        bool isHDISinkStarted;
        struct RendererSinkAdapter *sinkAdapter;
        pa_atomic_t hdistate; // 0:need_data 1:wait_consume 2:flushing
        pa_usec_t fullTs;
        bool runninglocked;
        pa_memchunk chunk;
        bool inited;
    } offload;
    struct {
        pa_usec_t timestamp;
        pa_thread *thread;
        pa_thread *thread_hdi;
        pa_asyncmsgq *msgq;
        bool isHDISinkStarted;
        struct RendererSinkAdapter *sinkAdapter;
        pa_asyncmsgq *dq;
        pa_atomic_t dflag;
        pa_usec_t writeTime;
        pa_usec_t prewrite;
        pa_sink_state_t previousState;
    } primary;
    struct {
        bool used;
        pa_usec_t timestamp;
        pa_thread *thread;
        pa_thread *thread_hdi;
        bool isHDISinkStarted;
        bool isHDISinkInited;
        struct RendererSinkAdapter *sinkAdapter;
        pa_asyncmsgq *msgq;
        pa_asyncmsgq *dq;
        pa_atomic_t dflag;
        pa_usec_t writeTime;
        pa_usec_t prewrite;
        pa_atomic_t hdistate;
        pa_memchunk chunk;
        SinkAttr sample_attrs;
    } multiChannel;
};

static void UserdataFree(struct Userdata *u);
static int32_t PrepareDevice(struct Userdata *u, const char *filePath);

static int32_t PrepareDeviceOffload(struct Userdata *u);
static char *GetStateInfo(pa_sink_state_t state);
static char *GetInputStateInfo(pa_sink_input_state_t state);
static void PaInputStateChangeCb(pa_sink_input *i, pa_sink_input_state_t state);
static void OffloadLock(struct Userdata *u);
static void OffloadUnlock(struct Userdata *u);
static int32_t UpdatePresentationPosition(struct Userdata *u);
static bool InputIsPrimary(pa_sink_input *i);
static bool InputIsOffload(pa_sink_input *i);
static void GetSinkInputName(pa_sink_input *i, char *str, int len);
static const char *safeProplistGets(const pa_proplist *p, const char *key, const char *defstr);
static void StartOffloadHdi(struct Userdata *u, pa_sink_input *i);
static void StopPrimaryHdiIfNoRunning(struct Userdata *u);
static void StartPrimaryHdiIfRunning(struct Userdata *u);
static void StartMultiChannelHdiIfRunning(struct Userdata *u);
static void CheckInputChangeToOffload(struct Userdata *u, pa_sink_input *i);

// BEGIN Utility functions
#define FLOAT_EPS 1e-9f
#define MEMBLOCKQ_MAXLENGTH (16*1024*16)
#define OFFSET_BIT_24 3
#define BIT_DEPTH_TWO 2
#define BIT_8 8
#define BIT_16 16
#define BIT_24 24
#define BIT_32 32
static uint32_t Read24Bit(const uint8_t *p)
{
    return ((uint32_t) p[BIT_DEPTH_TWO] << BIT_16) | ((uint32_t) p[1] << BIT_8) | ((uint32_t) p[0]);
}

static void Write24Bit(uint8_t *p, uint32_t u)
{
    p[BIT_DEPTH_TWO] = (uint8_t) (u >> BIT_16);
    p[1] = (uint8_t) (u >> BIT_8);
    p[0] = (uint8_t) u;
}

static void ConvertFrom16BitToFloat(unsigned n, const int16_t *a, float *b)
{
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1 << (BIT_16 - 1)));
    }
}

static void ConvertFrom24BitToFloat(unsigned n, const uint8_t *a, float *b)
{
    for (; n > 0; n--) {
        int32_t s = Read24Bit(a) << BIT_8;
        *b = s * (1.0f / (1U << (BIT_32 - 1)));
        a += OFFSET_BIT_24;
        b++;
    }
}

static void ConvertFrom32BitToFloat(unsigned n, const int32_t *a, float *b)
{
    for (; n > 0; n--) {
        *(b++) = *(a++) * (1.0f / (1U << (BIT_32 - 1)));
    }
}

static float CapMax(float v)
{
    float value = v;
    if (v > 1.0f) {
        value = 1.0f - FLOAT_EPS;
    } else if (v < -1.0f) {
        value = -1.0f + FLOAT_EPS;
    }
    return value;
}

static void ConvertFromFloatTo16Bit(unsigned n, const float *a, int16_t *b)
{
    for (; n > 0; n--) {
        float tmp = *a++;
        float v = CapMax(tmp) * (1 << (BIT_16 - 1));
        *(b++) = (int16_t) v;
    }
}

static void ConvertFromFloatTo24Bit(unsigned n, const float *a, uint8_t *b)
{
    for (; n > 0; n--) {
        float tmp = *a++;
        float v = CapMax(tmp) * (1U << (BIT_32 - 1));
        Write24Bit(b, ((int32_t) v) >> BIT_8);
        b += OFFSET_BIT_24;
    }
}

static void ConvertFromFloatTo32Bit(unsigned n, const float *a, int32_t *b)
{
    for (; n > 0; n--) {
        float tmp = *a++;
        float v = CapMax(tmp) * (1U << (BIT_32 - 1));
        *(b++) = (int32_t) v;
    }
}

static void ConvertToFloat(pa_sample_format_t format, unsigned n, void *src, float *dst)
{
    pa_assert(src);
    pa_assert(dst);
    int32_t ret;
    switch (format) {
        case PA_SAMPLE_S16LE:
            ConvertFrom16BitToFloat(n, src, dst);
            break;
        case PA_SAMPLE_S24LE:
            ConvertFrom24BitToFloat(n, src, dst);
            break;
        case PA_SAMPLE_S32LE:
            ConvertFrom32BitToFloat(n, src, dst);
            break;
        default:
            ret = memcpy_s(dst, n, src, n);
            if (ret != 0) {
                float *srcFloat = (float *)src;
                for (uint32_t i = 0; i < n; i++) {
                    dst[i] = srcFloat[i];
                }
            }
            break;
    }
}

static void ConvertFromFloat(pa_sample_format_t format, unsigned n, float *src, void *dst)
{
    pa_assert(src);
    pa_assert(dst);
    int32_t ret;
    switch (format) {
        case PA_SAMPLE_S16LE:
            ConvertFromFloatTo16Bit(n, src, dst);
            break;
        case PA_SAMPLE_S24LE:
            ConvertFromFloatTo24Bit(n, src, dst);
            break;
        case PA_SAMPLE_S32LE:
            ConvertFromFloatTo32Bit(n, src, dst);
            break;
        default:
            ret = memcpy_s(dst, n, src, n);
            if (ret != 0) {
                float *dstFloat = (float *)dst;
                for (uint32_t i = 0; i < n; i++) {
                    dstFloat[i] = src[i];
                }
            }
            break;
    }
}

static void updateResampler(pa_sink_input *sinkIn, const char *sceneType, bool mchFlag)
{
    uint32_t processChannels = DEFAULT_NUM_CHANNEL;
    uint64_t processChannelLayout = DEFAULT_CHANNELLAYOUT;
    if (mchFlag) {
        EffectChainManagerReturnMultiChannelInfo(&processChannels, &processChannelLayout);
    } else {
        EffectChainManagerReturnEffectChannelInfo(sceneType, &processChannels, &processChannelLayout);
    }

    pa_resampler *r;
    pa_sample_spec ss = sinkIn->thread_info.resampler->o_ss;
    pa_channel_map processCm;
    ConvertChLayoutToPaChMap(processChannelLayout, &processCm);
    if (processChannels == sinkIn->thread_info.resampler->i_ss.channels) {
        ss.channels = sinkIn->thread_info.resampler->i_ss.channels;
        pa_channel_map cm = sinkIn->thread_info.resampler->i_cm;
        if (ss.channels == sinkIn->thread_info.resampler->o_ss.channels) {
            return;
        }
        r = pa_resampler_new(sinkIn->thread_info.resampler->mempool,
                             &sinkIn->thread_info.resampler->i_ss,
                             &sinkIn->thread_info.resampler->i_cm,
                             &ss, &cm,
                             sinkIn->core->lfe_crossover_freq,
                             sinkIn->thread_info.resampler->method,
                             sinkIn->thread_info.resampler->flags);
    } else {
        ss.channels = processChannels;
        if (ss.channels == sinkIn->thread_info.resampler->o_ss.channels) {
            return;
        }
        r = pa_resampler_new(sinkIn->thread_info.resampler->mempool,
                             &sinkIn->thread_info.resampler->i_ss,
                             &sinkIn->thread_info.resampler->i_cm,
                             &ss, &processCm,
                             sinkIn->core->lfe_crossover_freq,
                             sinkIn->thread_info.resampler->method,
                             sinkIn->thread_info.resampler->flags);
    }
    pa_resampler_free(sinkIn->thread_info.resampler);
    sinkIn->thread_info.resampler = r;
    return;
}

static ssize_t RenderWrite(struct RendererSinkAdapter *sinkAdapter, pa_memchunk *pchunk)
{
    size_t index;
    size_t length;
    ssize_t count = 0;
    void *p = NULL;

    pa_assert(pchunk);

    index = pchunk->index;
    length = pchunk->length;
    p = pa_memblock_acquire(pchunk->memblock);
    pa_assert(p);

    while (true) {
        uint64_t writeLen = 0;

        int32_t ret = sinkAdapter->RendererRenderFrame(sinkAdapter, ((char*)p + index),
            (uint64_t)length, &writeLen);
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
            count += writeLen;
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

static enum AudioOffloadType GetInputPolicyState(pa_sink_input *i)
{
    return atoi(safeProplistGets(i->proplist, "stream.offload.statePolicy", "0"));
}

static void OffloadSetHdiVolumeBufferSize(pa_sink_input *i)
{
    if (!InputIsOffload(i)) {
        return;
    }

    struct Userdata *u = i->sink->userdata;
    float left;
    float right;
    u->offload.sinkAdapter->RendererSinkGetVolume(u->offload.sinkAdapter, &left, &right);
    u->offload.sinkAdapter->RendererSinkSetVolume(u->offload.sinkAdapter, left, right);
    const uint32_t bufSize = (GetInputPolicyState(i) == OFFLOAD_INACTIVE_BACKGROUND ?
                              OFFLOAD_HDI_CACHE2 : OFFLOAD_HDI_CACHE1);
    u->offload.sinkAdapter->RendererSinkSetBufferSize(u->offload.sinkAdapter, bufSize);
}
static int32_t RenderWriteOffload(struct Userdata *u, pa_sink_input *i, pa_memchunk *pchunk)
{
    size_t index;
    size_t length;
    void *p = NULL;

    pa_assert(pchunk);

    index = pchunk->index;
    length = pchunk->length;
    p = pa_memblock_acquire(pchunk->memblock);
    pa_assert(p);

    uint64_t writeLen = 0;
    uint64_t now = pa_rtclock_now();
    if (!u->offload.isHDISinkStarted) {
        AUDIO_DEBUG_LOG("StartOffloadHdi before write, because maybe sink switch");
        StartOffloadHdi(u, i);
    }
    int32_t ret = u->offload.sinkAdapter->RendererRenderFrame(u->offload.sinkAdapter, ((char*)p + index),
        (uint64_t)length, &writeLen);
    pa_memblock_release(pchunk->memblock);
    if (writeLen != length && writeLen != 0) {
        AUDIO_ERR_LOG("Error writeLen != actual bytes. Length: %zu, Written: %" PRIu64 " bytes, %d ret",
            length, writeLen, ret);
        return -1;
    }
    if (ret == 0 && u->offload.firstWriteHdi == true) {
        u->offload.firstWriteHdi = false;
        u->offload.hdiPosTs = now;
        u->offload.hdiPos = 0;
        OffloadSetHdiVolumeBufferSize(i);
    }
    if (ret == 0 && writeLen == 0) { // is full
        AUDIO_DEBUG_LOG("RenderWriteOffload, hdi is full, break");
        return 1; // 1 indicates full
    } else if (writeLen == 0) {
        AUDIO_ERR_LOG("Failed to render Length: %{public}zu, Written: %{public}" PRIu64 " bytes, %{public}d ret",
            length, writeLen, ret);
        return -1;
    }
    return 0;
}

static void OffloadCallback(const enum RenderCallbackType type, int8_t *userdata)
{
    struct Userdata *u = (struct Userdata *)userdata;
    switch (type) {
        case CB_NONBLOCK_WRITE_COMPLETED: { //need more data
            const int hdistate = pa_atomic_load(&u->offload.hdistate);
            if (hdistate == 1) {
                pa_atomic_store(&u->offload.hdistate, 0);
                OffloadLock(u);
                UpdatePresentationPosition(u);
                uint64_t cacheLenInHdi = u->offload.pos > u->offload.hdiPos ? u->offload.pos - u->offload.hdiPos : 0;
                pa_usec_t now = pa_rtclock_now();
                if (cacheLenInHdi > 200 * PA_USEC_PER_MSEC) { // 200 is min wait length
                    u->offload.minWait = now + 10 * PA_USEC_PER_MSEC; // 10ms for min wait
                }
            }
            if (u->thread_mq.inq) {
                pa_asyncmsgq_post(u->thread_mq.inq, NULL, 0, NULL, 0, NULL, NULL);
            }
            break;
        }
        case CB_DRAIN_COMPLETED:
        case CB_FLUSH_COMPLETED:
        case CB_RENDER_FULL:
        case CB_ERROR_OCCUR:
            break;
        default:
            break;
    }
}

static void RegOffloadCallback(struct Userdata *u)
{
    u->offload.sinkAdapter->RendererRegCallback(u->offload.sinkAdapter, (int8_t *)OffloadCallback, (int8_t *)u);
}

static ssize_t TestModeRenderWrite(struct Userdata *u, pa_memchunk *pchunk)
{
    size_t index;
    size_t length;
    ssize_t count = 0;
    void *p = NULL;

    pa_assert(pchunk);

    index = pchunk->index;
    length = pchunk->length;
    p = pa_memblock_acquire(pchunk->memblock);
    pa_assert(p);

    if (*((int32_t*)p) > 0) {
        AUDIO_DEBUG_LOG("RenderWrite Write: %{public}d", ++u->writeCount);
    }
    AUDIO_DEBUG_LOG("RenderWrite Write renderCount: %{public}d", ++u->renderCount);

    while (true) {
        uint64_t writeLen = 0;

        int32_t ret = u->primary.sinkAdapter->RendererRenderFrame(u->primary.sinkAdapter, ((char *)p + index),
            (uint64_t)length, &writeLen);
        if (writeLen > length) {
            AUDIO_ERR_LOG("Error writeLen > actual bytes. Length: %zu, Written: %" PRIu64 " bytes, %d ret",
                         length, writeLen, ret);
            count = -1 - count;
            break;
        }
        if (writeLen == 0) {
            AUDIO_ERR_LOG("Failed to render Length: %zu, Written: %" PRIu64 " bytes, %d ret",
                         length, writeLen, ret);
            count = -1 - count;
            break;
        } else {
            count += writeLen;
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

static bool IsInnerCapturer(pa_sink_input *sinkIn)
{
    pa_sink_input_assert_ref(sinkIn);

    if (!GetInnerCapturerState()) {
        return false;
    }

    const char *usageStr = pa_proplist_gets(sinkIn->proplist, "stream.usage");
    const char *privacyTypeStr = pa_proplist_gets(sinkIn->proplist, "stream.privacyType");
    int32_t usage = -1;
    int32_t privacyType = -1;
    bool usageSupport = false;
    bool privacySupport = true;

    if (privacyTypeStr != NULL) {
        pa_atoi(privacyTypeStr, &privacyType);
        privacySupport = IsPrivacySupportInnerCapturer(privacyType);
    }

    if (usageStr != NULL) {
        pa_atoi(usageStr, &usage);
        usageSupport = IsStreamSupportInnerCapturer(usage);
    }
    return privacySupport && usageSupport;
}

static const char *safeProplistGets(const pa_proplist *p, const char *key, const char *defstr)
{
    const char *res = pa_proplist_gets(p, key);
    if (res == NULL) {
        return defstr;
    }
    return res;
}

//modify from pa inputs_drop
static void InputsDropFromInputs(pa_mix_info *infoInputs, unsigned nInputs, pa_mix_info *info, unsigned n,
    pa_memchunk *result);

static unsigned GetInputsInfo(enum HdiInputType type, bool isRun, pa_sink *s, pa_mix_info *info, unsigned maxinfo);

static unsigned SinkRenderPrimaryClusterCap(pa_sink *si, size_t *length, pa_mix_info *infoIn, unsigned maxInfo)
{
    pa_sink_input *sinkIn;

    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(infoIn);

    unsigned n = 0;
    void *state = NULL;
    size_t mixlength = *length;
    while ((sinkIn = pa_hashmap_iterate(si->thread_info.inputs, &state, NULL)) && maxInfo > 0) {
        if (IsInnerCapturer(sinkIn) && InputIsPrimary(sinkIn)) {
            pa_sink_input_assert_ref(sinkIn);
            AUTO_CTRACE("hdi_sink::ClusterCap::pa_sink_input_peek:%d len:%zu", sinkIn->index, *length);
            pa_sink_input_peek(sinkIn, *length, &infoIn->chunk, &infoIn->volume);

            if (mixlength == 0 || infoIn->chunk.length < mixlength)
                mixlength = infoIn->chunk.length;

            if (pa_memblock_is_silence(infoIn->chunk.memblock)) {
                pa_memblock_unref(infoIn->chunk.memblock);
                continue;
            }

            infoIn->userdata = pa_sink_input_ref(sinkIn);
            pa_assert(infoIn->chunk.memblock);
            pa_assert(infoIn->chunk.length > 0);

            infoIn++;
            n++;
            maxInfo--;
        }
    }

    if (mixlength > 0) {
        *length = mixlength;
    }

    return n;
}

static void SinkRenderPrimaryMix(pa_sink *si, size_t length, pa_mix_info *infoIn, unsigned n, pa_memchunk *chunkIn)
{
    if (n == 0) {
        if (chunkIn->length > length)
            chunkIn->length = length;

        pa_silence_memchunk(chunkIn, &si->sample_spec);
    } else if (n == 1) {
        pa_cvolume volume;

        if (chunkIn->length > length)
            chunkIn->length = length;

        pa_sw_cvolume_multiply(&volume, &si->thread_info.soft_volume, &infoIn[0].volume);

        if (si->thread_info.soft_muted || pa_cvolume_is_muted(&volume)) {
            pa_silence_memchunk(chunkIn, &si->sample_spec);
        } else {
            pa_memchunk tmpChunk;

            tmpChunk = infoIn[0].chunk;
            pa_memblock_ref(tmpChunk.memblock);

            if (tmpChunk.length > length)
                tmpChunk.length = length;

            if (!pa_cvolume_is_norm(&volume)) {
                pa_memchunk_make_writable(&tmpChunk, 0);
                pa_volume_memchunk(&tmpChunk, &si->sample_spec, &volume);
            }

            pa_memchunk_memcpy(chunkIn, &tmpChunk);
            pa_memblock_unref(tmpChunk.memblock);
        }
    } else {
        void *ptr;

        ptr = pa_memblock_acquire(chunkIn->memblock);

        chunkIn->length = pa_mix(infoIn, n,
                                 (uint8_t*) ptr + chunkIn->index, length,
                                 &si->sample_spec,
                                 &si->thread_info.soft_volume,
                                 si->thread_info.soft_muted);

        pa_memblock_release(chunkIn->memblock);
    }
}

static void SinkRenderPrimaryInputsDropCap(pa_sink *si, pa_mix_info *infoIn, unsigned n, pa_memchunk *chunkIn)
{
    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(chunkIn);
    pa_assert(chunkIn->memblock);
    pa_assert(chunkIn->length > 0);

    /* We optimize for the case where the order of the inputs has not changed */

    pa_mix_info *infoCur = NULL;
    pa_sink_input *sceneSinkInput;
    bool isCaptureSilently = IsCaptureSilently();
    for (uint32_t k = 0; k < n; k++) {
        if (isCaptureSilently) {
            sceneSinkInput = infoIn[k].userdata;
            pa_sink_input_assert_ref(sceneSinkInput);
            pa_sink_input_drop(sceneSinkInput, chunkIn->length);
        }

        infoCur = infoIn + k;
        if (infoCur) {
            if (infoCur->chunk.memblock) {
                pa_memblock_unref(infoCur->chunk.memblock);
                pa_memchunk_reset(&infoCur->chunk);
            }

            pa_sink_input_unref(infoCur->userdata);

            if (isCaptureSilently) {
                infoCur->userdata = NULL;
            }
        }
    }
}

static int32_t SinkRenderPrimaryPeekCap(pa_sink *si, pa_memchunk *chunkIn)
{
    pa_mix_info infoIn[MAX_MIX_CHANNELS];
    unsigned n;
    size_t length;
    size_t blockSizeMax;

    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(PA_SINK_IS_LINKED(s->thread_info.state));
    pa_assert(chunkIn);
    pa_assert(chunkIn->memblock);
    pa_assert(chunkIn->length > 0);
    pa_assert(pa_frame_aligned(chunkIn->length, &si->sample_spec));

    pa_assert(!si->thread_info.rewind_requested);
    pa_assert(si->thread_info.rewind_nbytes == 0);

    if (si->thread_info.state == PA_SINK_SUSPENDED) {
        pa_silence_memchunk(chunkIn, &si->sample_spec);
        return 0;
    }

    pa_sink_ref(si);

    length = chunkIn->length;
    blockSizeMax = pa_mempool_block_size_max(si->core->mempool);
    if (length > blockSizeMax)
        length = pa_frame_align(blockSizeMax, &si->sample_spec);

    pa_assert(length > 0);

    n = SinkRenderPrimaryClusterCap(si, &length, infoIn, MAX_MIX_CHANNELS);
    SinkRenderPrimaryMix(si, length, infoIn, n, chunkIn);

    SinkRenderPrimaryInputsDropCap(si, infoIn, n, chunkIn);
    pa_sink_unref(si);

    return n;
}

static int32_t SinkRenderPrimaryGetDataCap(pa_sink *si, pa_memchunk *chunkIn)
{
    pa_memchunk chunk;
    size_t l;
    size_t d;
    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(PA_SINK_IS_LINKED(si->thread_info.state));
    pa_assert(chunkIn);
    pa_assert(chunkIn->memblock);
    pa_assert(chunkIn->length > 0);
    pa_assert(pa_frame_aligned(chunkIn->length, &si->sample_spec));

    pa_assert(!si->thread_info.rewind_requested);
    pa_assert(si->thread_info.rewind_nbytes == 0);

    if (si->thread_info.state == PA_SINK_SUSPENDED) {
        pa_silence_memchunk(chunkIn, &si->sample_spec);
        return 0;
    }

    pa_sink_ref(si);

    l = chunkIn->length;
    d = 0;

    int32_t nSinkInput = 0;
    while (l > 0) {
        chunk = *chunkIn;
        chunk.index += d;
        chunk.length -= d;

        nSinkInput = SinkRenderPrimaryPeekCap(si, &chunk);

        d += chunk.length;
        l -= chunk.length;
    }

    pa_sink_unref(si);

    return nSinkInput;
}

static bool monitorLinked(pa_sink *si, bool isRunning)
{
    if (isRunning) {
        return si->monitor_source && PA_SOURCE_IS_RUNNING(si->monitor_source->thread_info.state);
    } else {
        return si->monitor_source && PA_SOURCE_IS_LINKED(si->monitor_source->thread_info.state);
    }
}

static void SinkRenderCapProcess(pa_sink *si, size_t length, pa_memchunk *capResult)
{
    capResult->memblock = pa_memblock_new(si->core->mempool, length);
    capResult->index = 0;
    capResult->length = length;
    SinkRenderPrimaryGetDataCap(si, capResult);
    if (monitorLinked(si, false)) {
        pa_source_post(si->monitor_source, capResult);
    }
    return;
}

static void SinkRenderPrimaryInputsDrop(pa_sink *si, pa_mix_info *infoIn, unsigned n, pa_memchunk *chunkIn)
{
    unsigned nUnreffed = 0;

    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(chunkIn);
    pa_assert(chunkIn->memblock);
    pa_assert(chunkIn->length > 0);

    /* We optimize for the case where the order of the inputs has not changed */
    pa_mix_info *infoCur = NULL;
    for (uint32_t k = 0; k < n; k++) {
        pa_sink_input *sceneSinkInput = infoIn[k].userdata;
        pa_sink_input_assert_ref(sceneSinkInput);

        /* Drop read data */
        pa_sink_input_drop(sceneSinkInput, chunkIn->length);
        infoCur = infoIn + k;
        if (infoCur) {
            if (infoCur->chunk.memblock) {
                pa_memblock_unref(infoCur->chunk.memblock);
                pa_memchunk_reset(&infoCur->chunk);
            }

            pa_sink_input_unref(infoCur->userdata);
            infoCur->userdata = NULL;

            nUnreffed += 1;
        }
    }
    /* Now drop references to entries that are included in the
     * pa_mix_info array but don't exist anymore */

    if (nUnreffed < n) {
        for (; n > 0; infoIn++, n--) {
            if (infoIn->userdata)
                pa_sink_input_unref(infoIn->userdata);
            if (infoIn->chunk.memblock)
                pa_memblock_unref(infoIn->chunk.memblock);
        }
    }
}

static void SinkRenderMultiChannelInputsDrop(pa_sink *si, pa_mix_info *infoIn, unsigned n, pa_memchunk *chunkIn)
{
    AUDIO_DEBUG_LOG("mch inputs drop start");
    unsigned nUnreffed = 0;

    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(chunkIn);
    pa_assert(chunkIn->memblock);
    pa_assert(chunkIn->length > 0);

    /* We optimize for the case where the order of the inputs has not changed */
    pa_mix_info *infoCur = NULL;
    for (uint32_t k = 0; k < n; k++) {
        pa_sink_input *sceneSinkInput = infoIn[k].userdata;
        pa_sink_input_assert_ref(sceneSinkInput);

        /* Drop read data */
        pa_sink_input_drop(sceneSinkInput, chunkIn->length);
        infoCur = infoIn + k;
        if (infoCur) {
            if (infoCur->chunk.memblock) {
                pa_memblock_unref(infoCur->chunk.memblock);
                pa_memchunk_reset(&infoCur->chunk);
            }

            pa_sink_input_unref(infoCur->userdata);
            infoCur->userdata = NULL;

            nUnreffed += 1;
        }
    }
    /* Now drop references to entries that are included in the
     * pa_mix_info array but don't exist anymore */

    if (nUnreffed < n) {
        for (; n > 0; infoIn++, n--) {
            if (infoIn->userdata)
                pa_sink_input_unref(infoIn->userdata);
            if (infoIn->chunk.memblock)
                pa_memblock_unref(infoIn->chunk.memblock);
        }
    }
}

static unsigned SinkRenderPrimaryCluster(pa_sink *si, size_t *length, pa_mix_info *infoIn,
    unsigned maxInfo, char *sceneType)
{
    AUTO_CTRACE("hdi_sink::SinkRenderPrimaryCluster:%s len:%zu", sceneType, *length);
    pa_sink_input *sinkIn;
    unsigned n = 0;
    void *state = NULL;
    size_t mixlength = *length;

    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(infoIn);

    bool a2dpFlag = EffectChainManagerCheckA2dpOffload();
    bool isCaptureSilently = IsCaptureSilently();
    while ((sinkIn = pa_hashmap_iterate(si->thread_info.inputs, &state, NULL)) && maxInfo > 0) {
        const char *sinkSceneType = pa_proplist_gets(sinkIn->proplist, "scene.type");
        const char *sinkSceneMode = pa_proplist_gets(sinkIn->proplist, "scene.mode");
        const char *sinkSpatializationEnabled = pa_proplist_gets(sinkIn->proplist, "spatialization.enabled");
        bool existFlag = EffectChainManagerExist(sinkSceneType, sinkSceneMode, sinkSpatializationEnabled);
        int32_t sinkChannels = sinkIn->sample_spec.channels;
        if (a2dpFlag && !existFlag && sinkChannels > PRIMARY_CHANNEL_NUM) {
            continue;
        }
        if ((IsInnerCapturer(sinkIn) && isCaptureSilently) || !InputIsPrimary(sinkIn)) {
            continue;
        } else if ((pa_safe_streq(sinkSceneType, sceneType) && existFlag) ||
            (pa_safe_streq(sceneType, "EFFECT_NONE") && (!existFlag))) {
            pa_sink_input_assert_ref(sinkIn);
            updateResampler(sinkIn, sinkSceneType, false);

            AUTO_CTRACE("hdi_sink::PrimaryCluster:%u len:%zu", sinkIn->index, *length);
            pa_sink_input_peek(sinkIn, *length, &infoIn->chunk, &infoIn->volume);

            if (mixlength == 0 || infoIn->chunk.length < mixlength)
                mixlength = infoIn->chunk.length;

            if (pa_memblock_is_silence(infoIn->chunk.memblock)) {
                pa_memblock_unref(infoIn->chunk.memblock);
                continue;
            }

            infoIn->userdata = pa_sink_input_ref(sinkIn);
            pa_assert(infoIn->chunk.memblock);
            pa_assert(infoIn->chunk.length > 0);

            infoIn++;
            n++;
            maxInfo--;
        }
    }

    if (mixlength > 0) {
        *length = mixlength;
    }

    return n;
}


static unsigned SinkRenderMultiChannelCluster(pa_sink *si, size_t *length, pa_mix_info *infoIn,
    unsigned maxInfo)
{
    pa_sink_input *sinkIn;
    unsigned n = 0;
    void *state = NULL;
    size_t mixlength = *length;

    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(infoIn);

    bool a2dpFlag = EffectChainManagerCheckA2dpOffload();
    if (!a2dpFlag) {
        return 0;
    }

    while ((sinkIn = pa_hashmap_iterate(si->thread_info.inputs, &state, NULL)) && maxInfo > 0) {
        int32_t sinkChannels = sinkIn->sample_spec.channels;
        const char *sinkSceneType = pa_proplist_gets(sinkIn->proplist, "scene.type");
        const char *sinkSceneMode = pa_proplist_gets(sinkIn->proplist, "scene.mode");
        const char *sinkSpatializationEnabled = pa_proplist_gets(sinkIn->proplist, "spatialization.enabled");
        bool existFlag = EffectChainManagerExist(sinkSceneType, sinkSceneMode, sinkSpatializationEnabled);
        if (!existFlag && sinkChannels > PRIMARY_CHANNEL_NUM) {
            pa_sink_input_assert_ref(sinkIn);
            updateResampler(sinkIn, NULL, true);
            pa_sink_input_peek(sinkIn, *length, &infoIn->chunk, &infoIn->volume);

            if (mixlength == 0 || infoIn->chunk.length < mixlength)
                mixlength = infoIn->chunk.length;

            if (pa_memblock_is_silence(infoIn->chunk.memblock)) {
                pa_memblock_unref(infoIn->chunk.memblock);
                continue;
            }

            infoIn->userdata = pa_sink_input_ref(sinkIn);
            pa_assert(infoIn->chunk.memblock);
            pa_assert(infoIn->chunk.length > 0);

            infoIn++;
            n++;
            maxInfo--;
        }
    }

    if (mixlength > 0) {
        *length = mixlength;
    }

    return n;
}

static int32_t SinkRenderPrimaryPeek(pa_sink *si, pa_memchunk *chunkIn, char *sceneType)
{
    pa_mix_info info[MAX_MIX_CHANNELS];
    unsigned n;
    size_t length;
    size_t blockSizeMax;

    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(PA_SINK_IS_LINKED(s->thread_info.state));
    pa_assert(chunkIn);
    pa_assert(chunkIn->memblock);
    pa_assert(chunkIn->length > 0);
    pa_assert(pa_frame_aligned(chunkIn->length, &si->sample_spec));

    pa_assert(!si->thread_info.rewind_requested);
    pa_assert(si->thread_info.rewind_nbytes == 0);

    if (si->thread_info.state == PA_SINK_SUSPENDED) {
        AUTO_CTRACE("hdi_sink::Primary:PA_SINK_SUSPENDED");
        pa_silence_memchunk(chunkIn, &si->sample_spec);
        return 0;
    }

    pa_sink_ref(si);

    length = chunkIn->length;
    blockSizeMax = pa_mempool_block_size_max(si->core->mempool);
    if (length > blockSizeMax)
        length = pa_frame_align(blockSizeMax, &si->sample_spec);

    pa_assert(length > 0);
    n = SinkRenderPrimaryCluster(si, &length, info, MAX_MIX_CHANNELS, sceneType);

    AUTO_CTRACE("hdi_sink:Primary:SinkRenderPrimaryMix:%u len:%zu", n, length);
    SinkRenderPrimaryMix(si, length, info, n, chunkIn);

    SinkRenderPrimaryInputsDrop(si, info, n, chunkIn);
    pa_sink_unref(si);
    return n;
}

static int32_t SinkRenderMultiChannelPeek(pa_sink *si, pa_memchunk *chunkIn)
{
    pa_mix_info info[MAX_MIX_CHANNELS];
    unsigned n;
    size_t length;
    size_t blockSizeMax;

    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(PA_SINK_IS_LINKED(s->thread_info.state));
    pa_assert(chunkIn);
    pa_assert(chunkIn->memblock);
    pa_assert(chunkIn->length > 0);
    pa_assert(pa_frame_aligned(chunkIn->length, &si->sample_spec));

    pa_assert(!si->thread_info.rewind_requested);
    pa_assert(si->thread_info.rewind_nbytes == 0);

    if (si->thread_info.state == PA_SINK_SUSPENDED) {
        AUTO_CTRACE("hdi_sink::MultiCh:PA_SINK_SUSPENDED");
        pa_silence_memchunk(chunkIn, &si->sample_spec);
        return 0;
    }

    pa_sink_ref(si);

    length = chunkIn->length;
    blockSizeMax = pa_mempool_block_size_max(si->core->mempool);
    if (length > blockSizeMax)
        length = pa_frame_align(blockSizeMax, &si->sample_spec);

    pa_assert(length > 0);

    n = SinkRenderMultiChannelCluster(si, &length, info, MAX_MIX_CHANNELS);

    AUTO_CTRACE("hdi_sink:MultiCh:SinkRenderPrimaryMix:%u len:%zu", n, length);
    SinkRenderPrimaryMix(si, length, info, n, chunkIn);

    SinkRenderMultiChannelInputsDrop(si, info, n, chunkIn);
    pa_sink_unref(si);

    return n;
}

static int32_t SinkRenderPrimaryGetData(pa_sink *si, pa_memchunk *chunkIn, char *sceneType)
{
    AUTO_CTRACE("hdi_sink::SinkRenderPrimaryGetData:%s", sceneType);
    pa_memchunk chunk;
    size_t l;
    size_t d;
    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(PA_SINK_IS_LINKED(si->thread_info.state));
    pa_assert(chunkIn);
    pa_assert(chunkIn->memblock);
    pa_assert(chunkIn->length > 0);
    pa_assert(pa_frame_aligned(chunkIn->length, &si->sample_spec));

    pa_assert(!si->thread_info.rewind_requested);
    pa_assert(si->thread_info.rewind_nbytes == 0);

    if (si->thread_info.state == PA_SINK_SUSPENDED) {
        pa_silence_memchunk(chunkIn, &si->sample_spec);
        return 0;
    }

    pa_sink_ref(si);

    l = chunkIn->length;
    d = 0;

    int32_t nSinkInput = 0;
    while (l > 0) {
        chunk = *chunkIn;
        chunk.index += d;
        chunk.length -= d;

        nSinkInput = SinkRenderPrimaryPeek(si, &chunk, sceneType);

        d += chunk.length;
        l -= chunk.length;
    }
    pa_sink_unref(si);

    return nSinkInput;
}

static int32_t SinkRenderMultiChannelGetData(pa_sink *si, pa_memchunk *chunkIn)
{
    pa_memchunk chunk;
    size_t l;
    size_t d;
    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(PA_SINK_IS_LINKED(si->thread_info.state));
    pa_assert(chunkIn);
    pa_assert(chunkIn->memblock);
    pa_assert(chunkIn->length > 0);
    pa_assert(pa_frame_aligned(chunkIn->length, &si->sample_spec));

    pa_assert(!si->thread_info.rewind_requested);
    pa_assert(si->thread_info.rewind_nbytes == 0);

    if (si->thread_info.state == PA_SINK_SUSPENDED) {
        pa_silence_memchunk(chunkIn, &si->sample_spec);
        return 0;
    }

    pa_sink_ref(si);

    l = chunkIn->length;
    d = 0;

    int32_t nSinkInput = 0;
    while (l > 0) {
        chunk = *chunkIn;
        chunk.index += d;
        chunk.length -= d;

        nSinkInput = SinkRenderMultiChannelPeek(si, &chunk);

        d += chunk.length;
        l -= chunk.length;
    }

    pa_sink_unref(si);

    return nSinkInput;
}

static void AdjustProcessParamsBeforeGetData(pa_sink *si, uint8_t *sceneTypeLenRef)
{
    for (int32_t i = 0; i < SCENE_TYPE_NUM; i++) {
        sceneTypeLenRef[i] = DEFAULT_IN_CHANNEL_NUM;
    }
    pa_sink_input *sinkIn;
    void *state = NULL;
    unsigned maxInfo = MAX_MIX_CHANNELS;
    bool a2dpFlag = EffectChainManagerCheckA2dpOffload();
    while ((sinkIn = pa_hashmap_iterate(si->thread_info.inputs, &state, NULL)) && maxInfo > 0) {
        const char *sinkSceneType = pa_proplist_gets(sinkIn->proplist, "scene.type");
        const char *sinkSceneMode = pa_proplist_gets(sinkIn->proplist, "scene.mode");
        const uint8_t sinkChannels = sinkIn->sample_spec.channels;
        const char *sinkSpatializationEnabled = pa_proplist_gets(sinkIn->proplist, "spatialization.enabled");
        bool existFlag = EffectChainManagerExist(sinkSceneType, sinkSceneMode, sinkSpatializationEnabled);
        if (a2dpFlag && !existFlag && sinkChannels > PRIMARY_CHANNEL_NUM) {
            continue;
        }
        uint32_t processChannels = DEFAULT_NUM_CHANNEL;
        uint64_t processChannelLayout = DEFAULT_CHANNELLAYOUT;
        EffectChainManagerReturnEffectChannelInfo(sinkSceneType, &processChannels, &processChannelLayout);
        if (processChannels == DEFAULT_NUM_CHANNEL) {
            continue;
        }
        for (int32_t i = 0; i < SCENE_TYPE_NUM; i++) {
            if (pa_safe_streq(sinkSceneType, SCENE_TYPE_SET[i])) {
                sceneTypeLenRef[i] = processChannels;
            }
        }
        maxInfo--;
    }
}

static void SinkRenderPrimaryProcess(pa_sink *si, size_t length, pa_memchunk *chunkIn)
{
    pa_memchunk capResult;
    SinkRenderCapProcess(si, length, &capResult);

    uint8_t sceneTypeLenRef[SCENE_TYPE_NUM];
    struct Userdata *u;
    pa_assert_se(u = si->userdata);

    AdjustProcessParamsBeforeGetData(si, sceneTypeLenRef);
    size_t memsetInLen = sizeof(float) * DEFAULT_FRAMELEN * IN_CHANNEL_NUM_MAX;
    size_t memsetOutLen = sizeof(float) * DEFAULT_FRAMELEN * OUT_CHANNEL_NUM_MAX;
    memset_s(u->bufferAttr->tempBufIn, u->processSize, 0, memsetInLen);
    memset_s(u->bufferAttr->tempBufOut, u->processSize, 0, memsetOutLen);
    int32_t bitSize = pa_sample_size_of_format(u->format);
    chunkIn->memblock = pa_memblock_new(si->core->mempool, length * IN_CHANNEL_NUM_MAX / DEFAULT_IN_CHANNEL_NUM);
    for (int32_t i = 0; i < SCENE_TYPE_NUM; i++) {
        size_t tmpLength = length * sceneTypeLenRef[i] / DEFAULT_IN_CHANNEL_NUM;
        chunkIn->index = 0;
        chunkIn->length = tmpLength;
        int32_t nSinkInput = SinkRenderPrimaryGetData(si, chunkIn, SCENE_TYPE_SET[i]);
        if (nSinkInput == 0) { continue; }
        chunkIn->index = 0;
        chunkIn->length = tmpLength;
        void *src = pa_memblock_acquire_chunk(chunkIn);
        int32_t frameLen = bitSize > 0 ? (int32_t)(tmpLength / bitSize) : 0;

        ConvertToFloat(u->format, frameLen, src, u->bufferAttr->tempBufIn);
        memcpy_s(u->bufferAttr->bufIn, frameLen * sizeof(float), u->bufferAttr->tempBufIn, frameLen * sizeof(float));
        u->bufferAttr->numChanIn = sceneTypeLenRef[i];
        u->bufferAttr->frameLen = frameLen / u->bufferAttr->numChanIn;
        AUTO_CTRACE("hdi_sink::EffectChainManagerProcess:%s", SCENE_TYPE_SET[i]);
        EffectChainManagerProcess(SCENE_TYPE_SET[i], u->bufferAttr);
        for (int32_t k = 0; k < u->bufferAttr->frameLen * u->bufferAttr->numChanOut; k++) {
            u->bufferAttr->tempBufOut[k] += u->bufferAttr->bufOut[k];
        }
        pa_memblock_release(chunkIn->memblock);
        u->bufferAttr->numChanIn = DEFAULT_IN_CHANNEL_NUM;
    }
    void *dst = pa_memblock_acquire_chunk(chunkIn);
    int32_t frameLen = bitSize > 0 ? (int32_t)(length / bitSize) : 0;
    for (int32_t i = 0; i < frameLen; i++) {
        u->bufferAttr->tempBufOut[i] = u->bufferAttr->tempBufOut[i] > 0.99f ? 0.99f : u->bufferAttr->tempBufOut[i];
        u->bufferAttr->tempBufOut[i] = u->bufferAttr->tempBufOut[i] < -0.99f ? -0.99f : u->bufferAttr->tempBufOut[i];
    }
    ConvertFromFloat(u->format, frameLen, u->bufferAttr->tempBufOut, dst);

    chunkIn->index = 0;
    chunkIn->length = length;
    pa_memblock_release(chunkIn->memblock);
    pa_memblock_unref(capResult.memblock);
}

static void SinkRenderPrimary(pa_sink *si, size_t length, pa_memchunk *chunkIn)
{
    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(PA_SINK_IS_LINKED(si->thread_info.state));
    pa_assert(length > 0);
    pa_assert(pa_frame_aligned(length, &si->sample_spec));
    pa_assert(chunkIn);

    pa_assert(!si->thread_info.rewind_requested);
    pa_assert(si->thread_info.rewind_nbytes == 0);

    pa_sink_ref(si);

    size_t blockSizeMax;

    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(PA_SINK_IS_LINKED(si->thread_info.state));
    pa_assert(pa_frame_aligned(length, &si->sample_spec));
    pa_assert(chunkIn);

    pa_assert(!si->thread_info.rewind_requested);
    pa_assert(si->thread_info.rewind_nbytes == 0);

    if (si->thread_info.state == PA_SINK_SUSPENDED) {
        chunkIn->memblock = pa_memblock_ref(si->silence.memblock);
        chunkIn->index = si->silence.index;
        chunkIn->length = PA_MIN(si->silence.length, length);
        return;
    }

    if (length == 0)
        length = pa_frame_align(MIX_BUFFER_LENGTH, &si->sample_spec);

    blockSizeMax = pa_mempool_block_size_max(si->core->mempool);
    if (length > blockSizeMax)
        length = pa_frame_align(blockSizeMax, &si->sample_spec);

    pa_assert(length > 0);
    AUTO_CTRACE("hdi_sink::SinkRenderPrimaryProcess:len:%zu", length);
    SinkRenderPrimaryProcess(si, length, chunkIn);

    pa_sink_unref(si);
}

static void ProcessRenderUseTiming(struct Userdata *u, pa_usec_t now)
{
    pa_assert(u);

    // Fill the buffer up the latency size
    pa_memchunk chunk;

    AUTO_CTRACE("hdi_sink::SinkRenderPrimary");
    // Change from pa_sink_render to pa_sink_render_full for alignment issue in 3516
    SinkRenderPrimary(u->sink, u->sink->thread_info.max_request, &chunk);
    pa_assert(chunk.length > 0);

    StartPrimaryHdiIfRunning(u);
    pa_asyncmsgq_post(u->primary.dq, NULL, HDI_RENDER, NULL, 0, &chunk, NULL);
    u->primary.timestamp += pa_bytes_to_usec(chunk.length, &u->sink->sample_spec);
}

static bool InputIsOffload(pa_sink_input *i)
{
    if (monitorLinked(i->sink, true)) {
        return false;
    }
    struct Userdata *u = i->sink->userdata;
    if (!u->offload_enable || !u->offload.inited) {
        return false;
    }
    const char *offloadEnableStr = pa_proplist_gets(i->proplist, "stream.offload.enable");
    if (offloadEnableStr == NULL) {
        return false;
    }
    const bool offloadEnable = !strcmp(offloadEnableStr, "1");
    return offloadEnable;
}

static bool InputIsMultiChannel(pa_sink_input *i)
{
    bool a2dpFlag = EffectChainManagerCheckA2dpOffload();
    if (a2dpFlag) {
        int32_t sinkChannels = i->sample_spec.channels;
        const char *sinkSceneType = pa_proplist_gets(i->proplist, "scene.type");
        const char *sinkSceneMode = pa_proplist_gets(i->proplist, "scene.mode");
        const char *sinkSpatializationEnabled = pa_proplist_gets(i->proplist, "spatialization.enabled");
        bool existFlag = EffectChainManagerExist(sinkSceneType, sinkSceneMode, sinkSpatializationEnabled);
        if (!existFlag && sinkChannels > PRIMARY_CHANNEL_NUM) {
            return true;
        }
    }
    return false;
}

static bool InputIsPrimary(pa_sink_input *i)
{
    const bool isOffload = InputIsOffload(i);
    const bool isMultiChannel = InputIsMultiChannel(i); // add func is hd
    const bool isRunning = i->thread_info.state == PA_SINK_INPUT_RUNNING;
    return !isOffload && !isMultiChannel && isRunning;
}

static unsigned GetInputsInfo(enum HdiInputType type, bool isRun, pa_sink *s, pa_mix_info *info, unsigned maxinfo)
{
    pa_sink_input *i;
    unsigned n = 0;
    void *state = NULL;

    pa_sink_assert_ref(s);
    pa_sink_assert_io_context(s);
    pa_assert(is);

    while ((i = pa_hashmap_iterate(s->thread_info.inputs, &state, NULL)) && maxinfo > 0) {
        pa_sink_input_assert_ref(i);

        bool flag = false;
        const bool isOffload = InputIsOffload(i);
        const bool isHD = false; // add func is hd
        const bool isRunning = i->thread_info.state == PA_SINK_INPUT_RUNNING;
        if (isRun && !isRunning) {
            continue;
        }
        switch (type) {
            case HDI_INPUT_TYPE_PRIMARY:
                flag = !isOffload && !isHD;
                break;
            case HDI_INPUT_TYPE_OFFLOAD:
                flag = isOffload;
                break;
            case HDI_INPUT_TYPE_MULTICHANNEL:
                flag = isHD;
                break;
            default:
                break;
        }
        if (flag) {
            info->userdata = pa_sink_input_ref(i);
        } else {
            continue;
        }

        info++;
        n++;
        maxinfo--;
    }
    return n;
}

static int32_t GetInputsType(pa_sink *s, unsigned *nPrimary, unsigned *nOffload,
    unsigned *nMultiChannel, bool isRunning)
{
    int ret;
    struct Userdata *u;
    pa_assert_se(u = s->userdata);
    if ((ret = pthread_mutex_lock(&u->mutexPa2)) != 0) {
        AUDIO_WARNING_LOG("GetInputsType pthread_mutex_lock ret %d", ret);
    }
    pa_sink_input *i;
    void *state = NULL;
    *nPrimary = 0;
    *nOffload = 0;
    *nMultiChannel = 0;
    int n = 0;

    pa_sink_assert_ref(s);
    pa_sink_assert_io_context(s);
    pa_assert(is);
    while ((i = pa_hashmap_iterate(s->thread_info.inputs, &state, NULL))) {
        pa_sink_input_assert_ref(i);
        if (isRunning && i->thread_info.state != PA_SINK_INPUT_RUNNING) {
            continue;
        }
        n++;
        if (InputIsOffload(i)) {
            (*nOffload)++;
        } else if (!strcmp(u->sink->name, "Speaker") && InputIsMultiChannel(i)) {
            (*nMultiChannel)++;
        } else {
            (*nPrimary)++;
        }
    }
    if ((ret = pthread_mutex_unlock(&u->mutexPa2)) != 0) {
        AUDIO_WARNING_LOG("GetInputsType pthread_mutex_unlock ret %d", ret);
    }
    return n;
}

static size_t GetOffloadRenderLength(struct Userdata *u, pa_sink_input *i, bool *wait)
{
    size_t length;
    playback_stream *ps = i->userdata;
    const bool b = (bool)ps->sink_input->thread_info.resampler;
    const pa_sample_spec sampleSpecIn = b ? ps->sink_input->thread_info.resampler->i_ss : ps->sink_input->sample_spec;
    const pa_sample_spec sampleSpecOut = b ? ps->sink_input->thread_info.resampler->o_ss : ps->sink_input->sample_spec;
    const int statePolicy = GetInputPolicyState(i);
    u->offload.prewrite = (statePolicy == OFFLOAD_INACTIVE_BACKGROUND ?
                           OFFLOAD_HDI_CACHE2_PLUS : OFFLOAD_HDI_CACHE1_PLUS) * PA_USEC_PER_MSEC;
    size_t sizeFrame = pa_frame_align(pa_usec_to_bytes(OFFLOAD_FRAME_SIZE * PA_USEC_PER_MSEC, &sampleSpecOut),
        &sampleSpecOut);
    size_t tlengthHalfResamp = pa_frame_align(pa_usec_to_bytes(pa_bytes_to_usec(pa_memblockq_get_tlength(
        ps->memblockq) / 2, &sampleSpecIn), &sampleSpecOut), &sampleSpecOut); // 2 for half
    size_t sizeTgt = PA_MIN(sizeFrame, tlengthHalfResamp);
    const size_t bql = pa_memblockq_get_length(ps->memblockq);
    const size_t bqlResamp = pa_usec_to_bytes(pa_bytes_to_usec(bql, &sampleSpecIn), &sampleSpecOut);
    const size_t bqlRend = pa_memblockq_get_length(i->thread_info.render_memblockq);
    const size_t bqlAlin = pa_frame_align(bqlResamp + bqlRend, &sampleSpecOut);

    if (ps->drain_request) {
        if (i->thread_info.render_memblockq->maxrewind != 0) {
            pa_sink_input_update_max_rewind(i, 0);
        }
        const uint64_t hdiPos = u->offload.hdiPos + (pa_rtclock_now() - u->offload.hdiPosTs);
        *wait = u->offload.pos > hdiPos + HDI_MIN_MS_MAINTAIN * PA_USEC_PER_MSEC ? true : false;
        length = u->offload.pos > hdiPos + HDI_MIN_MS_MAINTAIN * PA_USEC_PER_MSEC ? 0 : sizeFrame;
    } else {
        bool waitable = false;
        const uint64_t hdiPos = u->offload.hdiPos + (pa_rtclock_now() - u->offload.hdiPosTs);
        if (u->offload.pos > hdiPos + 50 * PA_USEC_PER_MSEC) { // if hdi cache < 50ms, indicate no enough data
            // hdi left 100ms is triggered process_complete_msg, it leads to kartun. Could be stating time leads it.
            waitable = true;
        }
        length = PA_MIN(bqlAlin, sizeTgt);
        *wait = false;
        if (length < sizeTgt && u->offload.firstWrite == true) {
            *wait = true;
            length = 0;
        } else if (length < sizeTgt) {
            *wait = waitable || length == 0;
            length = waitable ? 0 : (length == 0 ? sizeFrame : length);
            if (ps->memblockq->missing > 0) {
                playback_stream_request_bytes(ps);
            } else if (ps->memblockq->missing < 0 && ps->memblockq->requested > (int64_t)ps->memblockq->minreq) {
                pa_sink_input_send_event(i, "signal_mainloop", NULL);
            }
        }
    }
    return length;
}

static void InputsDropFromInputs2(pa_mix_info *info, unsigned n)
{
    for (; n > 0; info++, n--) {
        if (info->userdata) {
            pa_sink_input_unref(info->userdata);
            info->userdata = NULL;
        }
        if (info->chunk.memblock) {
            pa_memblock_unref(info->chunk.memblock);
        }
    }
}

static void InputsDropFromInputs(pa_mix_info *infoInputs, unsigned nInputs, pa_mix_info *info, unsigned n,
    pa_memchunk *result)
{
    unsigned p = 0;
    unsigned ii = 0;
    unsigned nUnreffed = 0;
    pa_assert(result && result->memblock && result->length > 0);
    /* We optimize for the case where the order of the inputs has not changed */

    for (ii = 0; ii < nInputs; ++ii) {
        pa_sink_input *i = infoInputs[ii].userdata;
        unsigned j;
        pa_mix_info *m = NULL;

        pa_sink_input_assert_ref(i);

        /* Let's try to find the matching entyr info the pa_mix_info array */
        for (j = 0; j < n; j++) {
            if (info[p].userdata == i) {
                m = info + p;
                break;
            }

            p++;
            if (p >= n) {
                p = 0;
            }
        }

        /* Drop read data */
        pa_sink_input_drop(i, result->length);

        if (m) {
            if (m->chunk.memblock) {
                pa_memblock_unref(m->chunk.memblock);
                pa_memchunk_reset(&m->chunk);
            }

            pa_sink_input_unref(m->userdata);
            m->userdata = NULL;

            nUnreffed += 1;
        }
    }

    /* Now drop references to entries that are included in the
     * pa_mix_info array but don't exist anymore */

    if (nUnreffed < n) {
        for (; n > 0; info++, n--) {
            if (info->userdata)
                pa_sink_input_unref(info->userdata);
            if (info->chunk.memblock) {
                pa_memblock_unref(info->chunk.memblock);
            }
        }
    }
}

static void PaSinkRenderIntoOffload(pa_sink *s, pa_mix_info *infoInputs, unsigned nInputs, pa_memchunk *target)
{
    size_t length;
    size_t blockSizeMax;
    unsigned n = 0;
    unsigned ii = 0;
    pa_mix_info info[MAX_MIX_CHANNELS];
    pa_sink_assert_ref(s);
    pa_sink_assert_io_context(s);

    length = target->length;
    size_t mixlength = length;
    blockSizeMax = pa_mempool_block_size_max(s->core->mempool);
    if (length > blockSizeMax)
        length = pa_frame_align(blockSizeMax, &s->sample_spec);

    pa_assert(length > 0);
    for (ii = 0; ii < nInputs; ++ii) {
        pa_sink_input *i = infoInputs[ii].userdata;
        pa_sink_input_assert_ref(i);

        AUTO_CTRACE("hdi_sink::Offload:pa_sink_input_peek:%d len:%zu", i->index, length);
        pa_sink_input_peek(i, length, &info[n].chunk, &info[n].volume);
        if (mixlength == 0 || info[n].chunk.length < mixlength)
            mixlength = info[n].chunk.length;

        if (pa_memblock_is_silence(info[n].chunk.memblock)) {
            pa_memblock_unref(info[n].chunk.memblock);
            continue;
        }

        info[n].userdata = pa_sink_input_ref(i);

        pa_assert(info[n].chunk.memblock);
        pa_assert(info[n].chunk.length > 0);

        n++;
    }
    if (mixlength > 0) {
        length = mixlength;
    }

    pa_assert(n == 1 || n == 0);
    if (n == 0) {
        if (target->length > length)
            target->length = length;

        pa_silence_memchunk(target, &s->sample_spec);
    } else if (n == 1) {
        if (target->length > length)
            target->length = length;

        pa_memchunk vchunk;
        vchunk = info[0].chunk;

        if (vchunk.length > length)
            vchunk.length = length;
        // if target lead pa_memblock_new memory leak, fixed chunk length can solve it.
        pa_memchunk_memcpy(target, &vchunk);
    }

    InputsDropFromInputs(infoInputs, nInputs, info, n, target);
}

static void OffloadReset(struct Userdata *u)
{
    u->offload.sessionID = -1;
    u->offload.pos = 0;
    u->offload.hdiPos = 0;
    u->offload.hdiPosTs = pa_rtclock_now();
    u->offload.prewrite = OFFLOAD_HDI_CACHE1_PLUS * PA_USEC_PER_MSEC;
    u->offload.minWait = 0;
    u->offload.firstWrite = true;
    u->offload.firstWriteHdi = true;
    pa_atomic_store(&u->offload.hdistate, 0);
    u->offload.fullTs = 0;
}

static int32_t RenderWriteOffloadFunc(struct Userdata *u, size_t length, pa_mix_info *infoInputs, unsigned nInputs,
    int32_t *writen)
{
    pa_sink_input *i = infoInputs[0].userdata;

    pa_assert(length != 0);
    pa_memchunk *chunk = &(u->offload.chunk);
    chunk->index = 0;
    chunk->length = length;
    int64_t l;
    int64_t d;
    l = chunk->length;
    size_t blockSize = pa_memblock_get_length(u->offload.chunk.memblock);
    blockSize = PA_MAX(blockSize, pa_usec_to_bytes(0.6 * OFFLOAD_HDI_CACHE1 * PA_USEC_PER_MSEC, // 0.6 40% is hdi limit
        &u->sink->sample_spec));
    d = 0;
    while (l > 0) {
        pa_memchunk tchunk;
        tchunk = *chunk;
        tchunk.index += d;
        tchunk.length = PA_MIN(length, blockSize - tchunk.index);

        PaSinkRenderIntoOffload(i->sink, infoInputs, nInputs, &tchunk);
        d += tchunk.length;
        l -= tchunk.length;
    }
    if (l < 0) {
        chunk->length += -l;
    }

    int ret = RenderWriteOffload(u, i, chunk);
    *writen = ret == 0 ? chunk->length : 0;
    if (ret == 1) { // 1 indicates full
        const int hdistate = pa_atomic_load(&u->offload.hdistate);
        if (hdistate == 0) {
            pa_atomic_store(&u->offload.hdistate, 1);
        }
        if (GetInputPolicyState(i) == OFFLOAD_INACTIVE_BACKGROUND) {
            u->offload.fullTs = pa_rtclock_now();
        }
        pa_memblockq_rewind(i->thread_info.render_memblockq, chunk->length);
    }

    u->offload.pos += pa_bytes_to_usec(*writen, &u->sink->sample_spec);
    InputsDropFromInputs2(infoInputs, nInputs);
    return ret;
}

static int32_t ProcessRenderUseTimingOffload(struct Userdata *u, bool *wait, int32_t *nInput, int32_t *writen)
{
    *wait = true;
    pa_sink *s = u->sink;
    pa_mix_info infoInputs[MAX_MIX_CHANNELS];
    unsigned nInputs;

    pa_sink_assert_ref(s);
    pa_sink_assert_io_context(s);
    pa_assert(PA_SINK_IS_LINKED(s->thread_info.state));

    pa_assert(!s->thread_info.rewind_requested);
    pa_assert(s->thread_info.rewind_nbytes == 0);

    if (s->thread_info.state == PA_SINK_SUSPENDED) {
        return 0;
    }

    pa_sink_ref(s);

    nInputs = GetInputsInfo(HDI_INPUT_TYPE_OFFLOAD, true, s, infoInputs, MAX_MIX_CHANNELS);
    *nInput = (int32_t)nInputs;

    if (nInputs == 0) {
        pa_sink_unref(s);
        return 0;
    } else if (nInputs > 1) {
        AUDIO_WARNING_LOG("GetInputsInfo offload input != 1");
    }

    pa_sink_input *i = infoInputs[0].userdata;
    CheckInputChangeToOffload(u, i);
    size_t length = GetOffloadRenderLength(u, i, wait);
    if (*wait && length == 0) {
        InputsDropFromInputs2(infoInputs, nInputs);
        pa_sink_unref(s);
        return 0;
    }
    if (u->offload.firstWrite == true) { // first length > 0
        u->offload.firstWrite = false;
    }
    int ret = RenderWriteOffloadFunc(u, length, infoInputs, nInputs, writen);
    pa_sink_unref(s);
    return ret;
}

static int32_t UpdatePresentationPosition(struct Userdata *u)
{
    uint64_t frames;
    int64_t timeSec;
    int64_t timeNanoSec;
    int ret = u->offload.sinkAdapter->RendererSinkGetPresentationPosition(
        u->offload.sinkAdapter, &frames, &timeSec, &timeNanoSec);
    if (ret != 0) {
        AUDIO_ERR_LOG("RendererSinkGetPresentationPosition fail, ret %d", ret);
        return ret;
    }
    u->offload.hdiPos = frames;
    u->offload.hdiPosTs = timeSec * USEC_PER_SEC + timeNanoSec / PA_NSEC_PER_USEC;
    return 0;
}

static void OffloadRewindAndFlush(struct Userdata *u, pa_sink_input *i, bool afterRender)
{
    pa_sink_input_assert_ref(i);
    playback_stream *ps = i->userdata;
    pa_assert(ps);

    int ret = UpdatePresentationPosition(u);
    if (ret == 0) {
        uint64_t cacheLenInHdi = u->offload.pos > u->offload.hdiPos ? u->offload.pos - u->offload.hdiPos : 0;
        if (cacheLenInHdi != 0) {
            uint64_t bufSizeInRender = pa_usec_to_bytes(cacheLenInHdi, &i->sink->sample_spec);
            const pa_sample_spec sampleSpecIn = i->thread_info.resampler ? i->thread_info.resampler->i_ss
                                                                         : i->sample_spec;
            uint64_t bufSizeInInput = pa_usec_to_bytes(cacheLenInHdi, &sampleSpecIn);
            bufSizeInInput += pa_usec_to_bytes(pa_bytes_to_usec(
                pa_memblockq_get_length(i->thread_info.render_memblockq), &i->sink->sample_spec), &sampleSpecIn);
            uint64_t rewindSize = afterRender ? bufSizeInRender : bufSizeInInput;
            uint64_t maxRewind = afterRender ? i->thread_info.render_memblockq->maxrewind : ps->memblockq->maxrewind;
            if (rewindSize > maxRewind) {
                AUDIO_WARNING_LOG("OffloadRewindAndFlush, rewindSize(%" PRIu64 ") > maxrewind(%u), afterRender(%d)",
                    rewindSize, (uint32_t)(afterRender ? i->thread_info.render_memblockq->maxrewind :
                                           ps->memblockq->maxrewind), afterRender);
                rewindSize = maxRewind;
            }

            if (afterRender) {
                pa_memblockq_rewind(i->thread_info.render_memblockq, rewindSize);
            } else {
                pa_memblockq_rewind(ps->memblockq, rewindSize);
                pa_memblockq_flush_read(i->thread_info.render_memblockq);
            }
        }
    }
    u->offload.sinkAdapter->RendererSinkFlush(u->offload.sinkAdapter);
    OffloadReset(u);
}

static void GetSinkInputName(pa_sink_input *i, char *str, int len)
{
    const char *streamUid = safeProplistGets(i->proplist, "stream.client.uid", "NULL");
    const char *streamPid = safeProplistGets(i->proplist, "stream.client.pid", "NULL");
    const char *streamType = safeProplistGets(i->proplist, "stream.type", "NULL");
    const char *sessionID = safeProplistGets(i->proplist, "stream.sessionID", "NULL");
    int ret = sprintf_s(str, len, "%s_%s_%s_%s_of%d", streamType, streamUid, streamPid, sessionID, InputIsOffload(i));
    if (ret < 0) {
        AUDIO_ERR_LOG("sprintf_s fail! ret %d", ret);
    }
}

static int32_t getSinkInputSessionID(pa_sink_input *i)
{
    const char *res = pa_proplist_gets(i->proplist, "stream.sessionID");
    if (res == NULL) {
        return -1;
    } else {
        return atoi(res);
    }
}

static void OffloadLock(struct Userdata *u)
{
    if (!u->offload.runninglocked) {
        u->offload.sinkAdapter->RendererSinkOffloadRunningLockLock(u->offload.sinkAdapter);
        u->offload.runninglocked = true;
    } else {
    }
}

static void OffloadUnlock(struct Userdata *u)
{
    if (u->offload.runninglocked) {
        u->offload.sinkAdapter->RendererSinkOffloadRunningLockUnlock(u->offload.sinkAdapter);
        u->offload.runninglocked = false;
    } else {
    }
}

static void offloadSetMaxRewind(struct Userdata *u, pa_sink_input *i)
{
    pa_sink_input_assert_ref(i);
    pa_sink_input_assert_io_context(i);
    pa_assert(PA_SINK_INPUT_IS_LINKED(i->thread_info.state));

    size_t blockSize = pa_memblock_get_length(u->offload.chunk.memblock);
    pa_memblockq_set_maxrewind(i->thread_info.render_memblockq, blockSize);

    size_t nbytes = pa_usec_to_bytes(MAX_REWIND, &i->sink->sample_spec);

    if (i->update_max_rewind) {
        i->update_max_rewind(i, i->thread_info.resampler ?
                                pa_resampler_request(i->thread_info.resampler, nbytes) : nbytes);
    }
}

static void CheckInputChangeToOffload(struct Userdata *u, pa_sink_input *i)
{
    if (InputIsOffload(i) && pa_memblockq_get_maxrewind(i->thread_info.render_memblockq) == 0) {
        offloadSetMaxRewind(u, i);
        pa_sink_input *it;
        void *state = NULL;
        while ((it = pa_hashmap_iterate(u->sink->thread_info.inputs, &state, NULL))) {
            if (it != i && pa_memblockq_get_maxrewind(it->thread_info.render_memblockq) != 0) {
                pa_sink_input_update_max_rewind(it, 0);
                drop_backlog(it->thread_info.render_memblockq);
                playback_stream *ps = it->userdata;
                drop_backlog(ps->memblockq);
            }
        }
    }
}

static void StartOffloadHdi(struct Userdata *u, pa_sink_input *i)
{
    CheckInputChangeToOffload(u, i);
    int32_t sessionID = getSinkInputSessionID(i);
    if (u->offload.isHDISinkStarted) {
        AUDIO_INFO_LOG("StartOffloadHdi, sessionID : %{public}d -> %{public}d", u->offload.sessionID, sessionID);
        if (sessionID != u->offload.sessionID) {
            if (u->offload.sessionID != -1) {
                u->offload.sinkAdapter->RendererSinkReset(u->offload.sinkAdapter);
                OffloadReset(u);
            }
            u->offload.sessionID = sessionID;
        }
    } else {
        AUDIO_INFO_LOG("StartOffloadHdi, Restart offload with rate:%{public}d, channels:%{public}d",
            u->ss.rate, u->ss.channels);
        if (u->offload.sinkAdapter->RendererSinkStart(u->offload.sinkAdapter)) {
            AUDIO_WARNING_LOG("StartOffloadHdi, audiorenderer control start failed!");
        } else {
            RegOffloadCallback(u);
            u->offload.isHDISinkStarted = true;
            AUDIO_INFO_LOG("StartOffloadHdi, Successfully restarted offload HDI renderer");
            OffloadLock(u);
            u->offload.sessionID = sessionID;
            OffloadSetHdiVolumeBufferSize(i);
        }
    }
}

static void PaInputStateChangeCbOffload(struct Userdata *u, pa_sink_input *i, pa_sink_input_state_t state)
{
    const bool corking = i->thread_info.state == PA_SINK_INPUT_RUNNING && state == PA_SINK_INPUT_CORKED;
    const bool starting = i->thread_info.state == PA_SINK_INPUT_CORKED && state == PA_SINK_INPUT_RUNNING;
    const bool stopping = state == PA_SINK_INPUT_UNLINKED;

    if (starting) {
        StartOffloadHdi(u, i);
    } else if (corking) {
        pa_atomic_store(&u->offload.hdistate, 2); // 2 indicates corking
        OffloadRewindAndFlush(u, i, false);
    } else if (stopping) {
        u->offload.sinkAdapter->RendererSinkFlush(u->offload.sinkAdapter);
        OffloadReset(u);
    }
}

static void PaInputStateChangeCbPrimary(struct Userdata *u, pa_sink_input *i, pa_sink_input_state_t state)
{
    const bool starting = i->thread_info.state == PA_SINK_INPUT_CORKED && state == PA_SINK_INPUT_RUNNING;

    if (starting) {
        u->primary.timestamp = pa_rtclock_now();
        if (u->primary.isHDISinkStarted) {
            return;
        }
        AUDIO_INFO_LOG("PaInputStateChangeCb, Restart with rate:%{public}d,channels:%{public}d, format:%{public}d",
            u->ss.rate, u->ss.channels, (int)pa_sample_size_of_format(u->format));
        if (u->primary.sinkAdapter->RendererSinkStart(u->primary.sinkAdapter)) {
            AUDIO_ERR_LOG("PaInputStateChangeCb, audiorenderer control start failed!");
            u->primary.sinkAdapter->RendererSinkDeInit(u->primary.sinkAdapter);
        } else {
            u->primary.isHDISinkStarted = true;
            u->writeCount = 0;
            u->renderCount = 0;
            AUDIO_INFO_LOG("PaInputStateChangeCb, Successfully restarted HDI renderer");
        }
    }
}

static void StartPrimaryHdiIfRunning(struct Userdata *u)
{
    AUTO_CTRACE("hdi_sink::StartPrimaryHdiIfRunning");
    if (u->primary.isHDISinkStarted) {
        return;
    }

    unsigned nPrimary;
    unsigned nOffload;
    unsigned nMultiChannel;
    GetInputsType(u->sink, &nPrimary, &nOffload, &nMultiChannel, true);
    if (nPrimary == 0) {
        return;
    }

    if (u->primary.sinkAdapter->RendererSinkStart(u->primary.sinkAdapter)) {
        AUDIO_ERR_LOG("StartPrimaryHdiIfRunning, audiorenderer control start failed!");
        u->primary.sinkAdapter->RendererSinkDeInit(u->primary.sinkAdapter);
    } else {
        u->primary.isHDISinkStarted = true;
        u->writeCount = 0;
        u->renderCount = 0;
        AUDIO_INFO_LOG("StartPrimaryHdiIfRunning, Successfully restarted HDI renderer");
    }
}

static void ResetMultiChannelHdiState(struct Userdata *u, int32_t sinkChannels, uint64_t sinkChannelLayout)
{
    if (u->multiChannel.isHDISinkInited) {
        if (u->multiChannel.sample_attrs.channel != (uint32_t)sinkChannels) {
            u->multiChannel.sinkAdapter->RendererSinkStop(u->multiChannel.sinkAdapter);
            u->multiChannel.isHDISinkStarted = false;
            u->multiChannel.sinkAdapter->RendererSinkDeInit(u->multiChannel.sinkAdapter);
            u->multiChannel.isHDISinkInited = false;
            u->multiChannel.sample_attrs.adapterName = "primary";
            u->multiChannel.sample_attrs.channel = sinkChannels;
            u->multiChannel.sample_attrs.channelLayout = sinkChannelLayout;
            u->multiChannel.sinkAdapter->RendererSinkInit(u->multiChannel.sinkAdapter, &u->multiChannel.sample_attrs);
            u->multiChannel.isHDISinkInited = true;
        } else {
            if (u->multiChannel.isHDISinkStarted) {
                AUDIO_INFO_LOG("ResetMultiChannelHdiState inited and started");
                return;
            }
        }
    } else {
        u->multiChannel.sample_attrs.adapterName = "primary";
        u->multiChannel.sample_attrs.channel = sinkChannels;
        u->multiChannel.sample_attrs.channelLayout = sinkChannelLayout;
        u->multiChannel.sinkAdapter->RendererSinkInit(u->multiChannel.sinkAdapter, &u->multiChannel.sample_attrs);
        u->multiChannel.isHDISinkInited = true;
    }
    if (u->multiChannel.sinkAdapter->RendererSinkStart(u->multiChannel.sinkAdapter)) {
        u->multiChannel.isHDISinkStarted = false;
        u->multiChannel.sinkAdapter->RendererSinkDeInit(u->multiChannel.sinkAdapter);
        u->multiChannel.isHDISinkInited = false;
        AUDIO_INFO_LOG("ResetMultiChannelHdiState deinit success");
    } else {
        u->multiChannel.isHDISinkStarted = true;
        AUDIO_INFO_LOG("ResetMultiChannelHdiState start success");
        u->writeCount = 0;
        u->renderCount = 0;
    }
}

static void StartMultiChannelHdiIfRunning(struct Userdata *u)
{
    uint32_t sinkChannel = DEFAULT_MULTICHANNEL_NUM;
    uint64_t sinkChannelLayout = DEFAULT_MULTICHANNEL_CHANNELLAYOUT;
    EffectChainManagerReturnMultiChannelInfo(&sinkChannel, &sinkChannelLayout);

    ResetMultiChannelHdiState(u, sinkChannel, sinkChannelLayout);
}

static void StopPrimaryHdiIfNoRunning(struct Userdata *u)
{
    if (!u->primary.isHDISinkStarted) {
        return;
    }

    unsigned nPrimary;
    unsigned nOffload;
    unsigned nMultiChannel;
    GetInputsType(u->sink, &nPrimary, &nOffload, &nMultiChannel, true);
    if (nPrimary > 0) {
        return;
    }

    // Continuously dropping data clear counter on entering suspended state.
    if (u->bytes_dropped != 0) {
        AUDIO_INFO_LOG("StopPrimaryHdiIfNoRunning, HDI-sink continuously dropping data - clear statistics "
                       "(%zu -> 0 bytes dropped)", u->bytes_dropped);
        u->bytes_dropped = 0;
    }

    u->primary.sinkAdapter->RendererSinkStop(u->primary.sinkAdapter);
    AUDIO_INFO_LOG("StopPrimaryHdiIfNoRunning, Stopped HDI renderer");
    u->primary.isHDISinkStarted = false;
}

static void PaInputStateChangeCbMultiChannel(struct Userdata *u, pa_sink_input *i, pa_sink_input_state_t state)
{
    const bool corking = i->thread_info.state == PA_SINK_INPUT_RUNNING && state == PA_SINK_INPUT_CORKED;
    const bool starting = i->thread_info.state == PA_SINK_INPUT_CORKED && state == PA_SINK_INPUT_RUNNING;
    const bool stopping = state == PA_SINK_INPUT_UNLINKED;
    if (starting) {
        u->multiChannel.timestamp = pa_rtclock_now();
        uint32_t sinkChannel = DEFAULT_MULTICHANNEL_NUM;
        uint64_t sinkChannelLayout = DEFAULT_MULTICHANNEL_CHANNELLAYOUT;
        EffectChainManagerReturnMultiChannelInfo(&sinkChannel, &sinkChannelLayout);
        ResetMultiChannelHdiState(u, sinkChannel, sinkChannelLayout);
    } else if (stopping) {
        // Continuously dropping data clear counter on entering suspended state.
        if (u->bytes_dropped != 0) {
            AUDIO_INFO_LOG("PaInputStateChangeCbMultiChannel, HDI-sink continuously dropping data - clear statistics "
                           "(%zu -> 0 bytes dropped)", u->bytes_dropped);
            u->bytes_dropped = 0;
        }
        u->multiChannel.sinkAdapter->RendererSinkStop(u->multiChannel.sinkAdapter);
        u->multiChannel.sinkAdapter->RendererSinkDeInit(u->multiChannel.sinkAdapter);
        AUDIO_INFO_LOG("PaInputStateChangeCbMultiChannel, deinit mch renderer");
        u->multiChannel.isHDISinkStarted = false;
        u->multiChannel.isHDISinkInited = false;
    } else if (corking) {
        u->multiChannel.sinkAdapter->RendererSinkStop(u->multiChannel.sinkAdapter);
        u->multiChannel.sinkAdapter->RendererSinkDeInit(u->multiChannel.sinkAdapter);
        u->multiChannel.isHDISinkStarted = false;
        u->multiChannel.isHDISinkInited = false;
    }
}

static void PaInputStateChangeCb(pa_sink_input *i, pa_sink_input_state_t state)
{
    struct Userdata *u = NULL;

    pa_assert(i);
    pa_sink_input_assert_ref(i);
    pa_assert(i->sink);
    pa_assert_se(u = i->sink->userdata);

    char str[SPRINTF_STR_LEN] = {0};
    GetSinkInputName(i, str, SPRINTF_STR_LEN);
    AUDIO_INFO_LOG(
        "PaInputStateChangeCb, Sink[%{public}s]->SinkInput[%{public}s] state change:[%{public}s]-->[%{public}s]",
        GetDeviceClass(u->primary.sinkAdapter->deviceClass), str, GetInputStateInfo(i->thread_info.state),
        GetInputStateInfo(state));

    if (i->thread_info.state == state) {
        return;
    }

    {
        pa_proplist *propList = pa_proplist_new();
        pa_proplist_sets(propList, "old_state", GetInputStateInfo(i->thread_info.state));
        pa_proplist_sets(propList, "new_state", GetInputStateInfo(state));
        pa_sink_input_send_event(i, "state_changed", propList);
        pa_proplist_free(propList);
    }

    const bool corking = i->thread_info.state == PA_SINK_INPUT_RUNNING && state == PA_SINK_INPUT_CORKED;
    const bool starting = i->thread_info.state == PA_SINK_INPUT_CORKED && state == PA_SINK_INPUT_RUNNING;
    const bool stopping = state == PA_SINK_INPUT_UNLINKED;

    if (!corking && !starting && !stopping) {
        AUDIO_WARNING_LOG("PaInputStateChangeCb, input state change: invalid");
        return;
    }

    if (u->offload_enable && InputIsOffload(i)) {
        PaInputStateChangeCbOffload(u, i, state);
    } else if (u->multichannel_enable && InputIsMultiChannel(i)) {
        PaInputStateChangeCbMultiChannel(u, i, state);
    } else {
        PaInputStateChangeCbPrimary(u, i, state);
    }
}

static void PaInputVolumeChangeCb(pa_sink_input *i)
{
    struct Userdata *u;

    pa_sink_input_assert_ref(i);
    pa_assert_se(u = i->sink->userdata);

    if (u->offload_enable && InputIsOffload(i)) {
        float left;
        float right;
        u->offload.sinkAdapter->RendererSinkGetVolume(u->offload.sinkAdapter, &left, &right);

        pa_cvolume volume;
        pa_sw_cvolume_multiply(&volume, &i->sink->thread_info.soft_volume, &i->volume);
        float volumeResult;
        if (i->sink->thread_info.soft_muted || pa_cvolume_is_muted(&volume) || pa_cvolume_is_norm(&volume)) {
            volumeResult = 0;
        } else {
            volumeResult = (float)pa_sw_volume_to_linear(pa_cvolume_avg(&volume));
        }

        u->offload.sinkAdapter->RendererSinkSetVolume(u->offload.sinkAdapter, volumeResult, 0);

        char str[SPRINTF_STR_LEN] = {0};
        GetSinkInputName(i, str, SPRINTF_STR_LEN);
        AUDIO_INFO_LOG("PaInputVolumeChangeCb, Sink[%{public}s]->SinkInput[%{public}s] "
                       "offload hdi volume change:[%{public}f,%{public}f]-->[%{public}f]",
            GetDeviceClass(u->primary.sinkAdapter->deviceClass), str, left, right, volumeResult);
    }
}

static void ThreadFuncRendererTimerOffloadProcess(struct Userdata *u, pa_usec_t now, int64_t *sleepForUsec)
{
    const uint64_t pos = u->offload.pos;
    const uint64_t hdiPos = u->offload.hdiPos + (pa_rtclock_now() - u->offload.hdiPosTs);
    const uint64_t pw = u->offload.prewrite;
    int64_t blockTime = pa_bytes_to_usec(u->sink->thread_info.max_request, &u->sink->sample_spec);

    int32_t nInput = -1;
    const int hdistate = (int)pa_atomic_load(&u->offload.hdistate);
    if (pos <= hdiPos + pw && hdistate == 0) {
        bool wait;
        int32_t writen = -1;
        int ret = ProcessRenderUseTimingOffload(u, &wait, &nInput, &writen);
        if (ret < 0) {
            u->offload.minWait = now + 1 * PA_USEC_PER_MSEC; // 1ms for min wait
        } else if (wait) {
            if (u->offload.firstWrite == true) {
                blockTime = -1;
            } else {
                u->offload.minWait = now + 1 * PA_USEC_PER_MSEC; // 1ms for min wait
            }
        } else {
            blockTime = 1 * PA_USEC_PER_MSEC; // 1ms for min wait
        }
    } else if (hdistate == 1) {
        blockTime = (int64_t)(pos - hdiPos - HDI_MIN_MS_MAINTAIN * PA_USEC_PER_MSEC);
        if (blockTime < 0) {
            blockTime = OFFLOAD_FRAME_SIZE * PA_USEC_PER_MSEC; // block for one frame
        }
        u->offload.minWait = now + 1 * PA_USEC_PER_MSEC; // 1ms for min wait
    }
    if (pos < hdiPos) {
        if (pos != 0) {
            AUDIO_DEBUG_LOG("ThreadFuncRendererTimerOffload hdiPos wrong need sync, pos %" PRIu64 ", hdiPos %" PRIu64,
                pos, hdiPos);
        }
        if (u->offload.hdiPosTs + 300 * PA_USEC_PER_MSEC < now) { // 300ms for update pos
            UpdatePresentationPosition(u);
        }
    }
    if (nInput != 0 && blockTime != -1) {
        *sleepForUsec = (int64_t)PA_MAX(blockTime, 0) - (int64_t)(pa_rtclock_now() - now);
        *sleepForUsec = PA_MAX(*sleepForUsec, 0);
    }
}

static void ThreadFuncRendererTimerOffloadFlag(struct Userdata *u, pa_usec_t now, bool *flagOut, int64_t *sleepForUsec)
{
    bool flag = PA_SINK_IS_RUNNING(u->sink->thread_info.state);
    if (flag) {
        int64_t delta = u->offload.minWait - now;
        if (delta > 0) {
            flag = false;
            *sleepForUsec = delta;
        } else {
            unsigned nPrimary;
            unsigned nOffload;
            unsigned nMultiChannel;
            GetInputsType(u->sink, &nPrimary, &nOffload, &nMultiChannel, true);
            if (nOffload == 0) {
                flag = false;
            }
        }
    } else if (!PA_SINK_IS_OPENED(u->sink->thread_info.state)) {
        OffloadUnlock(u);
    }
    *flagOut = flag;
}

static int32_t WaitMsg(const char *prefix, const char *suffix, struct Userdata *u, pa_asyncmsgq *msgq)
{
    int32_t ret;
    int32_t code = 0;

    if ((ret = pthread_rwlock_unlock(&u->rwlockSleep)) != 0) {
        AUDIO_WARNING_LOG("WaitMsg pthread_rwlock_unlock ret %d", ret);
    }
    pa_assert_se(pa_asyncmsgq_get(msgq, NULL, &code, NULL, NULL, NULL, 1) == 0);
    if ((ret = pthread_rwlock_rdlock(&u->rwlockSleep)) != 0) {
        AUDIO_WARNING_LOG("WaitMsg pthread_rwlock_rdlock ret %d", ret);
    }

    pa_asyncmsgq_done(msgq, 0);

    if (code == QUIT) {
        AUDIO_INFO_LOG("Thread %s(use timing %s) shutting down, pid %d, tid %d", prefix, suffix, getpid(), gettid());
        pthread_rwlock_unlock(&u->rwlockSleep);
        return -1;
    }
    return 0;
}

static void ThreadFuncRendererTimerOffload(void *userdata)
{
    ScheduleReportData(getpid(), gettid(), "audio_server"); // set audio thread priority

    struct Userdata *u = userdata;

    pa_assert(u);

    const char *deviceClass = GetDeviceClass(u->primary.sinkAdapter->deviceClass);
    AUDIO_INFO_LOG("Thread %s(use timing offload) starting up, pid %d, tid %d", deviceClass, getpid(), gettid());
    pa_thread_mq_install(&u->thread_mq);

    OffloadReset(u);

    u->offload.sinkAdapter->RendererSinkOffloadRunningLockInit(u->offload.sinkAdapter);
    pthread_rwlock_rdlock(&u->rwlockSleep);

    while (true) {
        int32_t ret;
        if ((ret = pthread_mutex_unlock(&u->mutexPa)) != 0) {
            AUDIO_WARNING_LOG("ThreadFuncRendererTimerOffload pthread_mutex_unlock ret %d", ret);
        }
        if (WaitMsg(deviceClass, "offload", u, u->offload.msgq) == -1) {
            break;
        }
        if ((ret = pthread_mutex_lock(&u->mutexPa)) != 0) {
            AUDIO_WARNING_LOG("ThreadFuncRendererTimerOffload pthread_mutex_lock ret %d", ret);
        }

        // start process
        pa_usec_t now = pa_rtclock_now();
        int64_t sleepForUsec = -1;
        bool flag;
        ThreadFuncRendererTimerOffloadFlag(u, now, &flag, &sleepForUsec);

        if (flag) {
            ThreadFuncRendererTimerOffloadProcess(u, now, &sleepForUsec);
        }
        if (u->offload.fullTs != 0) {
            if (u->offload.fullTs + 10 * PA_USEC_PER_MSEC > now) { // 10 is min checking size
                const int64_t s = (u->offload.fullTs + 10 * PA_USEC_PER_MSEC) - now;
                sleepForUsec = sleepForUsec == -1 ? s : PA_MIN(s, sleepForUsec);
            } else if (pa_atomic_load(&u->offload.hdistate) == 1) {
                u->offload.fullTs = 0;
                OffloadUnlock(u);
                StopPrimaryHdiIfNoRunning(u);
            } else {
                u->offload.fullTs = 0;
            }
        }

        if (sleepForUsec != -1) {
            if (u->timestampSleep == -1) {
                u->timestampSleep = (int64_t)pa_rtclock_now() + sleepForUsec;
            } else {
                u->timestampSleep = PA_MIN(u->timestampSleep, (int64_t)pa_rtclock_now() + sleepForUsec);
            }
        }
    }
}

static void SinkRenderMultiChannelProcess(pa_sink *si, size_t length, pa_memchunk *chunkIn)
{
    struct Userdata *u;
    pa_assert_se(u = si->userdata);

    bool a2dpFlag = EffectChainManagerCheckA2dpOffload();
    if (!a2dpFlag) {
        return;
    }
    uint32_t sinkChannel = DEFAULT_MULTICHANNEL_NUM;
    uint64_t sinkChannelLayout = DEFAULT_MULTICHANNEL_CHANNELLAYOUT;
    EffectChainManagerReturnMultiChannelInfo(&sinkChannel, &sinkChannelLayout);

    chunkIn->memblock = pa_memblock_new(si->core->mempool, length * IN_CHANNEL_NUM_MAX / DEFAULT_IN_CHANNEL_NUM);
    size_t tmpLength = length * sinkChannel / DEFAULT_IN_CHANNEL_NUM;
    chunkIn->index = 0;
    chunkIn->length = tmpLength;
    SinkRenderMultiChannelGetData(si, chunkIn);
    chunkIn->index = 0;
    chunkIn->length = tmpLength;
}

static void SinkRenderMultiChannel(pa_sink *si, size_t length, pa_memchunk *chunkIn)
{
    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(PA_SINK_IS_LINKED(si->thread_info.state));
    pa_assert(length > 0);
    pa_assert(pa_frame_aligned(length, &si->sample_spec));
    pa_assert(chunkIn);

    pa_assert(!si->thread_info.rewind_requested);
    pa_assert(si->thread_info.rewind_nbytes == 0);

    pa_sink_ref(si);

    size_t blockSizeMax;

    pa_sink_assert_ref(si);
    pa_sink_assert_io_context(si);
    pa_assert(PA_SINK_IS_LINKED(si->thread_info.state));
    pa_assert(pa_frame_aligned(length, &si->sample_spec));
    pa_assert(chunkIn);

    pa_assert(!si->thread_info.rewind_requested);
    pa_assert(si->thread_info.rewind_nbytes == 0);

    if (si->thread_info.state == PA_SINK_SUSPENDED) {
        chunkIn->memblock = pa_memblock_ref(si->silence.memblock);
        chunkIn->index = si->silence.index;
        chunkIn->length = PA_MIN(si->silence.length, length);
        return;
    }

    if (length == 0)
        length = pa_frame_align(MIX_BUFFER_LENGTH, &si->sample_spec);

    blockSizeMax = pa_mempool_block_size_max(si->core->mempool);
    if (length > blockSizeMax)
        length = pa_frame_align(blockSizeMax, &si->sample_spec);

    pa_assert(length > 0);

    SinkRenderMultiChannelProcess(si, length, chunkIn);

    pa_sink_unref(si);
}

static void ProcessRenderUseTimingMultiChannel(struct Userdata *u, pa_usec_t now)
{
    pa_assert(u);

    // Fill the buffer up the latency size
    pa_memchunk chunk;

    // Change from pa_sink_render to pa_sink_render_full for alignment issue in 3516
    SinkRenderMultiChannel(u->sink, u->sink->thread_info.max_request, &chunk);
    pa_assert(chunk.length > 0);

    StartMultiChannelHdiIfRunning(u);
    pa_asyncmsgq_post(u->multiChannel.dq, NULL, HDI_RENDER, NULL, 0, &chunk, NULL);
    u->multiChannel.timestamp += pa_bytes_to_usec(u->sink->thread_info.max_request, &u->sink->sample_spec);
}

static bool ThreadFuncRendererTimerMultiChannelFlagJudge(struct Userdata *u)
{
    pa_assert(u);
    bool flag = (u->render_in_idle_state && PA_SINK_IS_OPENED(u->sink->thread_info.state)) ||
        (!u->render_in_idle_state && PA_SINK_IS_RUNNING(u->sink->thread_info.state)) ||
        (u->sink->state == PA_SINK_IDLE && u->sink->monitor_source &&
        PA_SOURCE_IS_RUNNING(u->sink->monitor_source->thread_info.state));
    pa_sink_input *i;
    void *state = NULL;
    int nMultiChannel = 0;
    while ((i = pa_hashmap_iterate(u->sink->thread_info.inputs, &state, NULL))) {
        pa_sink_input_assert_ref(i);
        if (InputIsMultiChannel(i)) {
            nMultiChannel++;
        }
    }
    flag &= nMultiChannel > 0;
    return flag;
}

static void ThreadFuncRendererTimerMultiChannel(void *userdata)
{
    ScheduleReportData(getpid(), gettid(), "audio_server"); // set audio thread priority

    struct Userdata *u = userdata;

    pa_assert(u);

    const char *deviceClass = GetDeviceClass(u->primary.sinkAdapter->deviceClass);
    AUDIO_INFO_LOG("Thread %s(use timing offload) starting up, pid %d, tid %d", deviceClass, getpid(), gettid());
    pa_thread_mq_install(&u->thread_mq);

    u->multiChannel.timestamp = pa_rtclock_now();
    const uint64_t pw = u->multiChannel.prewrite;
    pthread_rwlock_rdlock(&u->rwlockSleep);
    while (true) {
        int32_t ret;
        if ((ret = pthread_mutex_unlock(&u->mutexPa)) != 0) {
            AUDIO_WARNING_LOG("ThreadFuncRendererTimerMultiChannel pthread_mutex_unlock ret %d", ret);
        }
        if (WaitMsg(deviceClass, "multichannel", u, u->multiChannel.msgq) == -1) {
            break;
        }
        if ((ret = pthread_mutex_lock(&u->mutexPa)) != 0) {
            AUDIO_WARNING_LOG("ThreadFuncRendererTimerMultiChannel pthread_mutex_lock ret %d", ret);
        }

        pa_usec_t now = 0;

        int64_t sleepForUsec = -1;

        bool flag = ThreadFuncRendererTimerMultiChannelFlagJudge(u);
        if (flag) {
            now = pa_rtclock_now();
        }

        if (PA_UNLIKELY(u->sink->thread_info.rewind_requested)) {
            pa_sink_process_rewind(u->sink, 0);
        }

        if (flag) {
            if (u->multiChannel.timestamp <= now + pw && pa_atomic_load(&u->multiChannel.dflag) == 0) {
                pa_atomic_add(&u->multiChannel.dflag, 1);
                ProcessRenderUseTimingMultiChannel(u, now);
            }
            pa_usec_t blockTime = pa_bytes_to_usec(u->sink->thread_info.max_request, &u->sink->sample_spec);
            sleepForUsec = PA_MIN(blockTime - (pa_rtclock_now() - now), u->multiChannel.writeTime);
            sleepForUsec = PA_MAX(sleepForUsec, 0);
        }

        if (sleepForUsec != -1) {
            if (u->timestampSleep == -1) {
                u->timestampSleep = (int64_t)pa_rtclock_now() + sleepForUsec;
            } else {
                u->timestampSleep = PA_MIN(u->timestampSleep, (int64_t)pa_rtclock_now() + sleepForUsec);
            }
        }
    }
}

static void SetHdiParam(struct Userdata *userdata)
{
    pa_sink_input *i;
    void *state = NULL;
    int sessionIDMax = -1;
    char *sinkSceneTypeMax = "";
    char *sinkSceneModeMax = "";
    bool spatialEnabledMax = false;
    while ((i = pa_hashmap_iterate(userdata->sink->thread_info.inputs, &state, NULL))) {
        pa_sink_input_assert_ref(i);
        const char *clientUid = pa_proplist_gets(i->proplist, "stream.client.uid");
        const char *bootUpMusic = "1003";
        if (pa_safe_streq(clientUid, bootUpMusic)) {
            return;
        }
        const char *sinkSceneType = pa_proplist_gets(i->proplist, "scene.type");
        const char *sinkSceneMode = pa_proplist_gets(i->proplist, "scene.mode");
        const char *sinkSpatialization = pa_proplist_gets(i->proplist, "spatialization.enabled");
        const char *sinkSessionStr = pa_proplist_gets(i->proplist, "stream.sessionID");
        bool spatializationEnabled = pa_safe_streq(sinkSpatialization, "1") ? true : false;
        bool effectEnabled = pa_safe_streq(sinkSceneMode, "EFFECT_DEFAULT") ? true : false;
        bool spatialEnabled = spatializationEnabled && effectEnabled;
        int sessionID = atoi(sinkSessionStr == NULL ? "-1" : sinkSessionStr);
        if (sessionID > sessionIDMax && sinkSceneType && sinkSceneMode && sinkSpatialization) {
            sessionIDMax = sessionID;
            sinkSceneTypeMax = (char *)sinkSceneType;
            sinkSceneModeMax = (char *)sinkSceneMode;
            spatialEnabledMax = spatialEnabled;
        }
    }

    if (userdata == NULL || userdata->sinkSceneType == NULL || userdata->sinkSceneMode == NULL) {
        AUDIO_DEBUG_LOG("SetHdiParam userdata null pointer");
        return;
    }

    if (!pa_safe_streq(userdata->sinkSceneType, sinkSceneTypeMax) ||
        !pa_safe_streq(userdata->sinkSceneMode, sinkSceneModeMax) ||
        (userdata->spatialEnabled != spatialEnabledMax)) {
            userdata->sinkSceneMode = sinkSceneModeMax;
            userdata->sinkSceneType = sinkSceneTypeMax;
            userdata->spatialEnabled = spatialEnabledMax;
            EffectChainManagerSetHdiParam(userdata->sinkSceneType, userdata->sinkSceneMode, userdata->spatialEnabled);
        }
}

static void ThreadFuncRendererTimerLoop(struct Userdata *u, int64_t *sleepForUsec)
{
    pa_usec_t now = 0;

    bool flag = ((u->render_in_idle_state && PA_SINK_IS_OPENED(u->sink->thread_info.state)) ||
                (!u->render_in_idle_state && PA_SINK_IS_RUNNING(u->sink->thread_info.state)) ||
                (u->sink->state == PA_SINK_IDLE && monitorLinked(u->sink, true))) &&
                !(u->sink->state == PA_SINK_IDLE && u->primary.previousState == PA_SINK_SUSPENDED);
    unsigned nPrimary;
    unsigned nOffload;
    unsigned nHd;
    int32_t n = GetInputsType(u->sink, &nPrimary, &nOffload, &nHd, true);
    if (n != 0 && !monitorLinked(u->sink, true)) {
        flag &= nPrimary > 0;
    }
    if (flag) {
        now = pa_rtclock_now();
    }

    if (PA_UNLIKELY(u->sink->thread_info.rewind_requested)) {
        pa_sink_process_rewind(u->sink, 0);
    }

    if (flag) {
        if (u->primary.timestamp <= now + u->primary.prewrite && pa_atomic_load(&u->primary.dflag) == 0) {
            pa_atomic_add(&u->primary.dflag, 1);
            ProcessRenderUseTiming(u, now);
        }
        pa_usec_t blockTime = pa_bytes_to_usec(u->sink->thread_info.max_request, &u->sink->sample_spec);
        *sleepForUsec = blockTime - (pa_rtclock_now() - now);
        if (u->primary.timestamp <= now + u->primary.prewrite) {
            *sleepForUsec = PA_MIN(*sleepForUsec, u->primary.writeTime);
        }
        *sleepForUsec = PA_MAX(*sleepForUsec, 0);
    }
}

static void ThreadFuncRendererTimer(void *userdata)
{
    ScheduleReportData(getpid(), gettid(), "audio_server"); // set audio thread priority

    struct Userdata *u = userdata;

    pa_assert(u);

    const char *deviceClass = GetDeviceClass(u->primary.sinkAdapter->deviceClass);
    AUDIO_INFO_LOG("Thread %s(use timing primary) starting up, pid %d, tid %d", deviceClass, getpid(), gettid());
    pa_thread_mq_install(&u->thread_mq);

    u->primary.timestamp = pa_rtclock_now();
    pthread_rwlock_rdlock(&u->rwlockSleep);

    while (true) {
        int32_t ret;
        if ((ret = pthread_mutex_unlock(&u->mutexPa)) != 0) {
            AUDIO_WARNING_LOG("ThreadFuncRendererTimer pthread_mutex_unlock ret %d", ret);
        }
        if (WaitMsg(deviceClass, "primary", u, u->primary.msgq) == -1) {
            break;
        }
        if ((ret = pthread_mutex_lock(&u->mutexPa)) != 0) {
            AUDIO_WARNING_LOG("ThreadFuncRendererTimer pthread_mutex_lock ret %d", ret);
        }

        int64_t sleepForUsec = -1;

        ThreadFuncRendererTimerLoop(u, &sleepForUsec);

        if (sleepForUsec != -1) {
            if (u->timestampSleep == -1) {
                u->timestampSleep = (int64_t)pa_rtclock_now() + sleepForUsec;
            } else {
                u->timestampSleep = PA_MIN(u->timestampSleep, (int64_t)pa_rtclock_now() + sleepForUsec);
            }
        }
    }
}

static void ThreadFuncRendererTimerBusSendMsgq(struct Userdata *u)
{
    unsigned nPrimary;
    unsigned nOffload;
    unsigned nMultiChannel;
    int32_t n = GetInputsType(u->sink, &nPrimary, &nOffload, &nMultiChannel, false);

    if (u->timestampSleep < (int64_t)pa_rtclock_now()) {
        u->timestampSleep = -1;
    }

    pthread_rwlock_unlock(&u->rwlockSleep);

    bool primaryFlag = n == 0 || monitorLinked(u->sink, true);
    if ((nPrimary > 0 && u->primary.msgq) || primaryFlag) {
        pa_asyncmsgq_send(u->primary.msgq, NULL, 0, NULL, 0, NULL);
    }
    if (u->offload_enable && nOffload > 0 && u->offload.msgq) {
        pa_asyncmsgq_send(u->offload.msgq, NULL, 0, NULL, 0, NULL);
    }
    if (nMultiChannel > 0 && u->multiChannel.msgq) {
        pa_asyncmsgq_send(u->multiChannel.msgq, NULL, 0, NULL, 0, NULL);
    }
}

static void ThreadFuncRendererTimerBus(void *userdata)
{
    // set audio thread priority
    ScheduleReportData(getpid(), gettid(), "audio_server");

    struct Userdata *u = userdata;

    pa_assert(u);

    const char *deviceClass = GetDeviceClass(u->primary.sinkAdapter->deviceClass);
    AUDIO_INFO_LOG("Thread %s(use timing bus) starting up, pid %d, tid %d", deviceClass, getpid(), gettid());
    pa_thread_mq_install(&u->thread_mq);

    while (true) {
        int ret;
        pthread_rwlock_wrlock(&u->rwlockSleep);

        int64_t sleepForUsec;

        if (u->timestampSleep == -1) {
            pa_rtpoll_set_timer_disabled(u->rtpoll); // sleep forever
        } else if ((sleepForUsec = u->timestampSleep - pa_rtclock_now()) <= 0) {
            pa_rtpoll_set_timer_relative(u->rtpoll, 0);
        } else {
            pa_rtpoll_set_timer_relative(u->rtpoll, sleepForUsec);
        }

        // Hmm, nothing to do. Let's sleep
        if ((ret = pa_rtpoll_run(u->rtpoll)) < 0) {
            AUDIO_ERR_LOG("Thread (use timing bus) shutting down, error %d, pid %d, tid %d",
                ret, getpid(), gettid());
            // If this was no regular exit from the loop we have to continue
            // processing messages until we received PA_MESSAGE_SHUTDOWN
            pa_asyncmsgq_post(u->thread_mq.outq, PA_MSGOBJECT(u->core), PA_CORE_MESSAGE_UNLOAD_MODULE,
                u->module, 0, NULL, NULL);
            pa_asyncmsgq_wait_for(u->thread_mq.inq, PA_MESSAGE_SHUTDOWN);
            pthread_rwlock_unlock(&u->rwlockSleep);
            break;
        }

        if (ret == 0) {
            AUDIO_INFO_LOG("Thread (use timing bus) shutting down, pid %d, tid %d", getpid(), gettid());
            pthread_rwlock_unlock(&u->rwlockSleep);
            break;
        }

        SetHdiParam(u);

        ThreadFuncRendererTimerBusSendMsgq(u);
    }
}

static void ThreadFuncWriteHDIMultiChannel(void *userdata)
{
    AUDIO_DEBUG_LOG("ThreadFuncWriteHDIMultiChannel start");
    // set audio thread priority
    ScheduleReportData(getpid(), gettid(), "audio_server");

    struct Userdata *u = userdata;
    pa_assert(u);

    int32_t quit = 0;

    do {
        int32_t code = 0;
        pa_memchunk chunk;

        pa_assert_se(pa_asyncmsgq_get(u->multiChannel.dq, NULL, &code, NULL, NULL, &chunk, 1) == 0);

        switch (code) {
            case HDI_RENDER: {
                pa_usec_t now = pa_rtclock_now();
                if (RenderWrite(u->multiChannel.sinkAdapter, &chunk) < 0) {
                    AUDIO_DEBUG_LOG("ThreadFuncWriteHDIMultiChannel RenderWrite");
                    u->bytes_dropped += chunk.length;
                }
                if (pa_atomic_load(&u->multiChannel.dflag) == 1) {
                    pa_atomic_sub(&u->multiChannel.dflag, 1);
                }
                u->multiChannel.writeTime = pa_rtclock_now() - now;
                break;
            }
            case QUIT:
                quit = 1;
                break;
            default:
                break;
        }
        pa_asyncmsgq_done(u->multiChannel.dq, 0);
    } while (!quit);
}

static void ThreadFuncWriteHDI(void *userdata)
{
    // set audio thread priority
    ScheduleReportData(getpid(), gettid(), "audio_server");

    struct Userdata *u = userdata;
    pa_assert(u);

    int32_t quit = 0;

    do {
        int32_t code = 0;
        pa_memchunk chunk;

        pa_assert_se(pa_asyncmsgq_get(u->primary.dq, NULL, &code, NULL, NULL, &chunk, 1) == 0);

        switch (code) {
            case HDI_RENDER: {
                pa_usec_t now = pa_rtclock_now();
                if (!u->primary.isHDISinkStarted && now - u->timestampLastLog > USEC_PER_SEC) {
                    u->timestampLastLog = now;
                    const char *deviceClass = GetDeviceClass(u->primary.sinkAdapter->deviceClass);
                    AUDIO_DEBUG_LOG("HDI not started, skip RenderWrite, wait sink[%s] suspend", deviceClass);
                    pa_memblock_unref(chunk.memblock);
                } else if (!u->primary.isHDISinkStarted) {
                    pa_memblock_unref(chunk.memblock);
                } else if (RenderWrite(u->primary.sinkAdapter, &chunk) < 0) {
                    u->bytes_dropped += chunk.length;
                    AUDIO_ERR_LOG("RenderWrite failed");
                }
                if (pa_atomic_load(&u->primary.dflag) == 1) {
                    pa_atomic_sub(&u->primary.dflag, 1);
                }
                u->primary.writeTime = pa_rtclock_now() - now;
                break;
            }
            case QUIT:
                quit = 1;
                break;
            default:
                break;
        }
        pa_asyncmsgq_done(u->primary.dq, 0);
    } while (!quit);
}

static void TestModeThreadFuncWriteHDI(void *userdata)
{
    // set audio thread priority
    ScheduleReportData(getpid(), gettid(), "audio_server");

    struct Userdata *u = userdata;
    pa_assert(u);

    int32_t quit = 0;

    do {
        int32_t code = 0;
        pa_memchunk chunk;

        pa_assert_se(pa_asyncmsgq_get(u->primary.dq, NULL, &code, NULL, NULL, &chunk, 1) == 0);

        switch (code) {
            case HDI_RENDER:
                if (TestModeRenderWrite(u, &chunk) < 0) {
                    u->bytes_dropped += chunk.length;
                    AUDIO_ERR_LOG("TestModeRenderWrite failed");
                }
                if (pa_atomic_load(&u->primary.dflag) == 1) {
                    pa_atomic_sub(&u->primary.dflag, 1);
                }
                break;
            case QUIT:
                quit = 1;
                break;
            default:
                break;
        }
        pa_asyncmsgq_done(u->primary.dq, 0);
    } while (!quit);
}

static void SinkUpdateRequestedLatencyCb(pa_sink *s)
{
    struct Userdata *u = NULL;
    size_t nbytes;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);

    u->block_usec = pa_sink_get_requested_latency_within_thread(s);

    if (u->block_usec == (pa_usec_t) - 1)
        u->block_usec = s->thread_info.max_latency;

    nbytes = pa_usec_to_bytes(u->block_usec, &s->sample_spec);
    pa_sink_set_max_request_within_thread(s, nbytes);
}

static int32_t SinkProcessMsg(pa_msgobject *o, int32_t code, void *data, int64_t offset,
    pa_memchunk *chunk)
{
    AUDIO_DEBUG_LOG("SinkProcessMsg: code: %{public}d", code);
    struct Userdata *u = PA_SINK(o)->userdata;
    pa_assert(u);

    switch (code) {
        case PA_SINK_MESSAGE_GET_LATENCY: {
            if (u->sink_latency) {
                *((uint64_t *)data) = u->sink_latency * PA_USEC_PER_MSEC;
            } else {
                uint64_t latency;
                uint32_t hdiLatency;

                // Tries to fetch latency from HDI else will make an estimate based
                // on samples to be rendered based on the timestamp and current time
                if (u->primary.sinkAdapter->RendererSinkGetLatency(u->primary.sinkAdapter, &hdiLatency) == 0) {
                    latency = (PA_USEC_PER_MSEC * hdiLatency);
                } else {
                    pa_usec_t now = pa_rtclock_now();
                    latency = (now - u->primary.timestamp);
                }

                *((uint64_t *)data) = latency;
            }
            return 0;
        }
        default:
            break;
    }
    return pa_sink_process_msg(o, code, data, offset, chunk);
}

static char *GetStateInfo(pa_sink_state_t state)
{
    switch (state) {
        case PA_SINK_INVALID_STATE:
            return "INVALID";
        case PA_SINK_RUNNING:
            return "RUNNING";
        case PA_SINK_IDLE:
            return "IDLE";
        case PA_SINK_SUSPENDED:
            return "SUSPENDED";
        case PA_SINK_INIT:
            return "INIT";
        case PA_SINK_UNLINKED:
            return "UNLINKED";
        default:
            return "error state";
    }
}

static char *GetInputStateInfo(pa_sink_input_state_t state)
{
    switch (state) {
        case PA_SINK_INPUT_INIT:
            return "INIT";
        case PA_SINK_INPUT_RUNNING:
            return "RUNNING";
        case PA_SINK_INPUT_CORKED:
            return "CORKED";
        case PA_SINK_INPUT_UNLINKED:
            return "UNLINKED";
        default:
            return "UNKNOWN";
    }
}

static int32_t RemoteSinkStateChange(pa_sink *s, pa_sink_state_t newState)
{
    struct Userdata *u = s->userdata;
    if (s->thread_info.state == PA_SINK_INIT && newState == PA_SINK_IDLE) {
        AUDIO_INFO_LOG("First start.");
    }

    if (s->thread_info.state == PA_SINK_SUSPENDED && PA_SINK_IS_OPENED(newState)) {
        u->primary.timestamp = pa_rtclock_now();
        if (u->primary.isHDISinkStarted) {
            return 0;
        }

        if (u->primary.sinkAdapter->RendererSinkStart(u->primary.sinkAdapter)) {
            AUDIO_ERR_LOG("audiorenderer control start failed!");
        } else {
            u->primary.isHDISinkStarted = true;
            u->render_in_idle_state = 1; // enable to reduce noise from idle to running.
            u->writeCount = 0;
            u->renderCount = 0;
            AUDIO_INFO_LOG("Successfully restarted remote renderer");
        }
    }
    if (PA_SINK_IS_OPENED(s->thread_info.state) && newState == PA_SINK_SUSPENDED) {
        // Continuously dropping data (clear counter on entering suspended state.
        if (u->bytes_dropped != 0) {
            AUDIO_INFO_LOG("HDI-sink continuously dropping data - clear statistics (%zu -> 0 bytes dropped)",
                           u->bytes_dropped);
            u->bytes_dropped = 0;
        }

        if (u->primary.isHDISinkStarted) {
            u->primary.sinkAdapter->RendererSinkStop(u->primary.sinkAdapter);
            AUDIO_INFO_LOG("Stopped HDI renderer");
            u->primary.isHDISinkStarted = false;
        }
    }

    return 0;
}

static int32_t SinkSetStateInIoThreadCbStartPrimary(struct Userdata *u, pa_sink_state_t newState)
{
    u->primary.previousState = u->sink->thread_info.state;
    if (!PA_SINK_IS_OPENED(newState)) {
        return 0;
    }

    u->primary.timestamp = pa_rtclock_now();
    if (u->primary.isHDISinkStarted) {
        return 0;
    }

    if (u->sink->thread_info.state == PA_SINK_SUSPENDED && newState == PA_SINK_IDLE) {
        AUDIO_INFO_LOG("Primary sink from suspend to idle");
        return 0;
    }

    if (u->primary.sinkAdapter->RendererSinkStart(u->primary.sinkAdapter)) {
        AUDIO_ERR_LOG("audiorenderer control start failed!");
        u->primary.sinkAdapter->RendererSinkDeInit(u->primary.sinkAdapter);
    } else {
        u->primary.isHDISinkStarted = true;
        u->writeCount = 0;
        u->renderCount = 0;
        AUDIO_INFO_LOG("SinkSetStateInIoThreadCbStartPrimary, Successfully restarted HDI renderer");
    }
    return 0;
}

static int32_t SinkSetStateInIoThreadCbStartMultiChannel(struct Userdata *u, pa_sink_state_t newState)
{
    if (!PA_SINK_IS_OPENED(newState)) {
        return 0;
    }

    u->multiChannel.timestamp = pa_rtclock_now();

    uint32_t sinkChannel = DEFAULT_MULTICHANNEL_NUM;
    uint64_t sinkChannelLayout = DEFAULT_MULTICHANNEL_CHANNELLAYOUT;
    EffectChainManagerReturnMultiChannelInfo(&sinkChannel, &sinkChannelLayout);
    ResetMultiChannelHdiState(u, sinkChannel, sinkChannelLayout);
    return 0;
}

static void OffloadSinkStateChangeCb(pa_sink *sink, pa_sink_state_t newState)
{
    pa_sink *s;
    uint32_t idx;
    struct Userdata *u = NULL;
    int32_t nOpened = 0;
    pa_core *c = ((struct Userdata *)(sink->userdata))->core;
    PA_IDXSET_FOREACH(s, c->sinks, idx) {
        if (u == NULL || !u->offload_enable) {
            u = s->userdata;
        }

        if (s != sink && s->thread_info.state != PA_SINK_SUSPENDED) {
            nOpened += 1;
        }
    }
    if (u == NULL || !u->offload_enable) {
        return;
    }

    const bool starting = PA_SINK_IS_OPENED(newState);
    const bool stopping = newState == PA_SINK_SUSPENDED;
    if (!u->offload.inited && starting) {
        if (PrepareDeviceOffload(u) < 0) {
            return;
        }
        u->offload.inited = true;
        return;
    }

    if (stopping && nOpened == 0) {
        if (u->offload.isHDISinkStarted) {
            u->offload.sinkAdapter->RendererSinkStop(u->offload.sinkAdapter);
            AUDIO_INFO_LOG("Stopped Offload HDI renderer, DeInit later");
            u->offload.isHDISinkStarted = false;
        }
        OffloadReset(u);
        OffloadUnlock(u);
        if (u->offload.inited) {
            u->offload.inited = false;
            u->offload.sinkAdapter->RendererSinkDeInit(u->offload.sinkAdapter);
            AUDIO_INFO_LOG("DeInited Offload HDI renderer");
        }
        return;
    }
}

// Called from the IO thread.
static int32_t SinkSetStateInIoThreadCb(pa_sink *s, pa_sink_state_t newState, pa_suspend_cause_t newSuspendCause)
{
    struct Userdata *u = NULL;

    pa_assert(s);
    pa_assert_se(u = s->userdata);

    AUDIO_INFO_LOG("Sink[%{public}s] state change:[%{public}s]-->[%{public}s]",
        GetDeviceClass(u->primary.sinkAdapter->deviceClass), GetStateInfo(s->thread_info.state),
        GetStateInfo(newState));

    if (!strcmp(GetDeviceClass(u->primary.sinkAdapter->deviceClass), DEVICE_CLASS_REMOTE)) {
        return RemoteSinkStateChange(s, newState);
    }

    if (u->offload_enable) {
        OffloadSinkStateChangeCb(s, newState);
    }

    if (s->thread_info.state == PA_SINK_SUSPENDED || s->thread_info.state == PA_SINK_INIT ||
        newState == PA_SINK_RUNNING) {
        if (EffectChainManagerCheckA2dpOffload() && (!strcmp(u->sink->name, "Speaker"))) {
            SinkSetStateInIoThreadCbStartMultiChannel(u, newState);
        }
        return SinkSetStateInIoThreadCbStartPrimary(u, newState);
    } else if (PA_SINK_IS_OPENED(s->thread_info.state)) {
        if (newState != PA_SINK_SUSPENDED) {
            return 0;
        }
        // Continuously dropping data (clear counter on entering suspended state.
        if (u->bytes_dropped != 0) {
            AUDIO_INFO_LOG("HDI-sink continuously dropping data - clear statistics (%zu -> 0 bytes dropped)",
                           u->bytes_dropped);
            u->bytes_dropped = 0;
        }

        if (u->primary.isHDISinkStarted) {
            u->primary.sinkAdapter->RendererSinkStop(u->primary.sinkAdapter);
            AUDIO_INFO_LOG("Stopped HDI renderer");
            u->primary.isHDISinkStarted = false;
        }
    }

    return 0;
}

static pa_hook_result_t SinkInputMoveStartCb(pa_core *core, pa_sink_input *i, struct Userdata *u)
{
    pa_sink_input_assert_ref(i);
    if (u->offload_enable) {
        const bool maybeOffload = pa_memblockq_get_maxrewind(i->thread_info.render_memblockq) != 0;
        if (maybeOffload || InputIsOffload(i)) {
            OffloadRewindAndFlush(u, i, false);
            pa_sink_input_update_max_rewind(i, 0);
        }
    }
    return PA_HOOK_OK;
}

static pa_hook_result_t SinkInputStateChangedCb(pa_core *core, pa_sink_input *i, struct Userdata *u)
{
    pa_sink_input_assert_ref(i);
    if (u->offload_enable && InputIsOffload(i)) {
        if (i->state == PA_SINK_INPUT_CORKED) {
            pa_atomic_store(&u->offload.hdistate, 0);
        }
    }
    return PA_HOOK_OK;
}

static pa_hook_result_t SinkInputPutCb(pa_core *core, pa_sink_input *i, struct Userdata *u)
{
    pa_sink_input_assert_ref(i);
    i->state_change = PaInputStateChangeCb;
    if (u->offload_enable) {
        i->volume_changed = PaInputVolumeChangeCb;
    }
    return PA_HOOK_OK;
}

static enum HdiAdapterFormat ConvertPaToHdiAdapterFormat(pa_sample_format_t format)
{
    enum HdiAdapterFormat adapterFormat;
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

static int32_t PrepareDevice(struct Userdata *u, const char *filePath)
{
    SinkAttr sample_attrs;
    int32_t ret;

    sample_attrs.format = ConvertPaToHdiAdapterFormat(u->ss.format);
    sample_attrs.adapterName = u->adapterName;
    sample_attrs.openMicSpeaker = u->open_mic_speaker;
    sample_attrs.sampleRate = u->ss.rate;
    sample_attrs.channel = u->ss.channels;
    sample_attrs.volume = MAX_SINK_VOLUME_LEVEL;
    sample_attrs.filePath = filePath;
    sample_attrs.deviceNetworkId = u->deviceNetworkId;
    sample_attrs.deviceType =  u->deviceType;

    ret = u->primary.sinkAdapter->RendererSinkInit(u->primary.sinkAdapter, &sample_attrs);
    if (ret != 0) {
        AUDIO_ERR_LOG("audiorenderer Init failed!");
        return -1;
    }

    // call start in io thread for remote device.
    if (strcmp(GetDeviceClass(u->primary.sinkAdapter->deviceClass), DEVICE_CLASS_REMOTE)) {
        ret = u->primary.sinkAdapter->RendererSinkStart(u->primary.sinkAdapter);
    }

    if (ret != 0) {
        AUDIO_ERR_LOG("audiorenderer control start failed!");
        u->primary.sinkAdapter->RendererSinkDeInit(u->primary.sinkAdapter);
        return -1;
    }

    return 0;
}

static int32_t PrepareDeviceOffload(struct Userdata *u)
{
    const char *adapterName = safeProplistGets(u->sink->proplist, PA_PROP_DEVICE_STRING, "");
    const char *filePath = safeProplistGets(u->sink->proplist, "filePath", "");
    const char *deviceNetworkId = safeProplistGets(u->sink->proplist, "NetworkId", "");
    AUDIO_INFO_LOG("PrepareDeviceOffload enter, deviceClass %d, filePath %s",
        u->offload.sinkAdapter->deviceClass, filePath);
    SinkAttr sample_attrs;
    int32_t ret;

    enum HdiAdapterFormat format = ConvertPaToHdiAdapterFormat(u->ss.format);
    sample_attrs.format = format;
    AUDIO_INFO_LOG("PrepareDeviceOffload audiorenderer format: %d ,adapterName %s",
        sample_attrs.format, GetDeviceClass(u->offload.sinkAdapter->deviceClass));
    sample_attrs.adapterName = adapterName;
    sample_attrs.openMicSpeaker = u->open_mic_speaker;
    sample_attrs.sampleRate = u->ss.rate;
    sample_attrs.channel = u->ss.channels;
    sample_attrs.volume = MAX_SINK_VOLUME_LEVEL;
    sample_attrs.filePath = filePath;
    sample_attrs.deviceNetworkId = deviceNetworkId;
    sample_attrs.deviceType = u->deviceType;

    ret = u->offload.sinkAdapter->RendererSinkInit(u->offload.sinkAdapter, &sample_attrs);
    if (ret != 0) {
        AUDIO_ERR_LOG("PrepareDeviceOffload audiorenderer Init failed!");
        return -1;
    }
    
    return 0;
}

static int32_t PrepareDeviceMultiChannel(struct Userdata *u, struct RendererSinkAdapter *sinkAdapter,
    const char *filePath)
{
    int32_t ret;

    enum HdiAdapterFormat format = ConvertPaToHdiAdapterFormat(u->ss.format);

    u->multiChannel.sample_attrs.format = format;
    u->multiChannel.sample_attrs.sampleRate = u->ss.rate;
    AUDIO_INFO_LOG("PrepareDeviceMultiChannel format: %d ,adapterName %s",
        u->multiChannel.sample_attrs.format, GetDeviceClass(sinkAdapter->deviceClass));
    u->multiChannel.sample_attrs.adapterName = u->adapterName;
    u->multiChannel.sample_attrs.openMicSpeaker = u->open_mic_speaker;
    u->multiChannel.sample_attrs.sampleRate = u->ss.rate;
    u->multiChannel.sample_attrs.channel = DEFAULT_MULTICHANNEL_NUM;
    u->multiChannel.sample_attrs.channelLayout = DEFAULT_MULTICHANNEL_CHANNELLAYOUT;
    u->multiChannel.sample_attrs.volume = MAX_SINK_VOLUME_LEVEL;
    u->multiChannel.sample_attrs.filePath = filePath;
    u->multiChannel.sample_attrs.deviceNetworkId = u->deviceNetworkId;
    u->multiChannel.sample_attrs.deviceType =  u->deviceType;

    ret = sinkAdapter->RendererSinkInit(sinkAdapter, &u->multiChannel.sample_attrs);
    if (ret != 0) {
        AUDIO_ERR_LOG("PrepareDeviceMultiChannel Init failed!");
        return -1;
    }
    u->multiChannel.isHDISinkInited = true;
    AUDIO_DEBUG_LOG("PrepareDeviceMultiChannel init success");
    // call start in io thread for remote device.
    if (strcmp(GetDeviceClass(sinkAdapter->deviceClass), DEVICE_CLASS_REMOTE)) {
        ret = sinkAdapter->RendererSinkStart(sinkAdapter);
    }

    if (ret != 0) {
        AUDIO_ERR_LOG("PrepareDeviceMultiChannel control start failed!");
        sinkAdapter->RendererSinkDeInit(sinkAdapter);
        return -1;
    }
    AUDIO_DEBUG_LOG("PrepareDeviceMultiChannel start success");
    return 0;
}

static void PaHdiSinkUserdataInit(struct Userdata *u)
{
    u->format = u->ss.format;
    u->processLen = IN_CHANNEL_NUM_MAX * DEFAULT_FRAMELEN;
    u->processSize = u->processLen * sizeof(float);
    u->bufferAttr = pa_xnew0(BufferAttr, 1);
    pa_assert_se(u->bufferAttr->bufIn = (float *)malloc(u->processSize));
    pa_assert_se(u->bufferAttr->bufOut = (float *)malloc(u->processSize));
    pa_assert_se(u->bufferAttr->tempBufIn = (float *)malloc(u->processSize));
    pa_assert_se(u->bufferAttr->tempBufOut = (float *)malloc(u->processSize));
    u->bufferAttr->samplingRate = u->ss.rate;
    u->bufferAttr->frameLen = DEFAULT_FRAMELEN;
    u->bufferAttr->numChanIn = u->ss.channels;
    u->bufferAttr->numChanOut = u->ss.channels;
    u->sinkSceneMode = "";
    u->sinkSceneType = "";
    u->spatialEnabled = false;
}

static pa_sink *PaHdiSinkInit(struct Userdata *u, pa_modargs *ma, const char *driver)
{
    // set audio thread priority
    ScheduleReportData(getpid(), gettid(), "audio_server");

    pa_sink_new_data data;
    pa_module *m;
    pa_sink *sink = NULL;

    m = u->module;
    u->ss = m->core->default_sample_spec;
    u->map = m->core->default_channel_map;
    if (pa_modargs_get_sample_spec_and_channel_map(ma, &u->ss, &u->map, PA_CHANNEL_MAP_DEFAULT) < 0) {
        AUDIO_ERR_LOG("Failed to parse sample specification and channel map");
        goto fail;
    }

    AUDIO_INFO_LOG("Initializing HDI rendering device with rate: %{public}d, channels: %{public}d",
        u->ss.rate, u->ss.channels);
    if (PrepareDevice(u, pa_modargs_get_value(ma, "file_path", "")) < 0) { goto fail; }

    u->primary.prewrite = 0;
    if (u->offload_enable && !strcmp(GetDeviceClass(u->primary.sinkAdapter->deviceClass), DEVICE_CLASS_PRIMARY)) {
        u->primary.prewrite = u->block_usec * 7; // 7 frame, set cache len in hdi, avoid pop
    }

    u->primary.isHDISinkStarted = true;
    AUDIO_DEBUG_LOG("Initialization of HDI rendering device[%{public}s] completed", u->adapterName);
    pa_sink_new_data_init(&data);
    data.driver = driver;
    data.module = m;

    PaHdiSinkUserdataInit(u);
    pa_sink_new_data_set_name(&data, pa_modargs_get_value(ma, "sink_name", DEFAULT_SINK_NAME));
    pa_sink_new_data_set_sample_spec(&data, &u->ss);
    pa_sink_new_data_set_channel_map(&data, &u->map);
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_STRING,
        (u->adapterName ? u->adapterName : DEFAULT_AUDIO_DEVICE_NAME));
    pa_proplist_setf(data.proplist, PA_PROP_DEVICE_DESCRIPTION, "HDI sink is %s",
        (u->adapterName ? u->adapterName : DEFAULT_AUDIO_DEVICE_NAME));
    pa_proplist_sets(data.proplist, "filePath", pa_modargs_get_value(ma, "file_path", ""));
    pa_proplist_sets(data.proplist, "networkId", pa_modargs_get_value(ma, "network_id", DEFAULT_DEVICE_NETWORKID));

    if (pa_modargs_get_proplist(ma, "sink_properties", data.proplist, PA_UPDATE_REPLACE) < 0) {
        AUDIO_ERR_LOG("Invalid properties");
        pa_sink_new_data_done(&data);
        goto fail;
    }

    sink = pa_sink_new(m->core, &data,
                       PA_SINK_HARDWARE | PA_SINK_LATENCY | PA_SINK_DYNAMIC_LATENCY);
    pa_sink_new_data_done(&data);

    return sink;

fail:
    return NULL;
}

static int32_t PaHdiSinkNewInitThreadMultiChannel(pa_module *m, pa_modargs *ma, struct Userdata *u)
{
    int ret;
    char *paThreadName = NULL;
    pa_atomic_store(&u->multiChannel.dflag, 0);
    u->multiChannel.msgq = pa_asyncmsgq_new(0);
    u->multiChannel.dq = pa_asyncmsgq_new(0);
    ret = LoadSinkAdapter(DEVICE_CLASS_MULTICHANNEL, "LocalDevice", &u->multiChannel.sinkAdapter);
    if (ret) {
        AUDIO_ERR_LOG("Load mch adapter failed");
        return -1;
    }
    if (PrepareDeviceMultiChannel(u, u->multiChannel.sinkAdapter, pa_modargs_get_value(ma, "file_path", "")) < 0) {
        return -1;
    }

    u->multiChannel.used = true;

    u->multiChannel.chunk.memblock = pa_memblock_new(u->sink->core->mempool, -1); // -1 == pa_mempool_block_size_max

    paThreadName = "OS_write-pa-mch";
    if (!(u->multiChannel.thread = pa_thread_new(paThreadName, ThreadFuncRendererTimerMultiChannel, u))) {
        AUDIO_ERR_LOG("Failed to write-pa-multiChannel thread.");
        return -1;
    }
    AUDIO_DEBUG_LOG("multichannel pa_thread_new success");

    return 0;
}

static int32_t PaHdiSinkNewInitThread(pa_module *m, pa_modargs *ma, struct Userdata *u)
{
    char *paThreadName = NULL;

    paThreadName = "OS_WriteBus";
    if (!(u->thread = pa_thread_new(paThreadName, ThreadFuncRendererTimerBus, u))) {
        AUDIO_ERR_LOG("Failed to create bus thread.");
        return -1;
    }

    paThreadName = "OS_WritePrimary";
    if (!(u->primary.thread = pa_thread_new(paThreadName, ThreadFuncRendererTimer, u))) {
        AUDIO_ERR_LOG("Failed to create primary thread.");
        return -1;
    }

    if (!strcmp(u->sink->name, "Speaker")) {
        PaHdiSinkNewInitThreadMultiChannel(m, ma, u);
        u->multichannel_enable = true;
        pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SINK_INPUT_PUT], PA_HOOK_EARLY,
            (pa_hook_cb_t)SinkInputPutCb, u);
    } else {
        u->multichannel_enable = false;
    }

    // offload
    const char *deviceClass = GetDeviceClass(u->primary.sinkAdapter->deviceClass);
    if (u->offload_enable) {
        AUDIO_DEBUG_LOG("PaHdiSinkNew device[%s] sink[%s] init offload thread", deviceClass, u->sink->name);
        int32_t ret = LoadSinkAdapter(DEVICE_CLASS_OFFLOAD, "LocalDevice", &u->offload.sinkAdapter);
        if (ret) {
            AUDIO_ERR_LOG("Load adapter failed");
            return -1;
        }

        u->offload.msgq = pa_asyncmsgq_new(0);
        pa_atomic_store(&u->offload.hdistate, 0);
        u->offload.chunk.memblock = pa_memblock_new(u->sink->core->mempool,
            pa_usec_to_bytes(200 * PA_USEC_PER_MSEC, &u->sink->sample_spec)); // 200ms for max len once offload render
        pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SINK_INPUT_MOVE_START], PA_HOOK_LATE,
            (pa_hook_cb_t)SinkInputMoveStartCb, u);
        pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SINK_INPUT_STATE_CHANGED], PA_HOOK_NORMAL,
            (pa_hook_cb_t)SinkInputStateChangedCb, u);
        paThreadName = "OS_WriteOffload";
        if (!(u->offload.thread = pa_thread_new(paThreadName, ThreadFuncRendererTimerOffload, u))) {
            AUDIO_ERR_LOG("Failed to create offload thread.");
            return -1;
        }
    } else {
        AUDIO_DEBUG_LOG("PaHdiSinkNew device[%s] sink[%s] skip offload thread", deviceClass, u->sink->name);
    }
    return 0;
}

static int32_t PaHdiSinkNewInitUserData(pa_module *m, pa_modargs *ma, struct Userdata *u)
{
    u->core = m->core;
    u->module = m;

    pa_memchunk_reset(&u->memchunk);
    u->rtpoll = pa_rtpoll_new();
    u->primary.msgq = pa_asyncmsgq_new(0);
    pthread_rwlock_init(&u->rwlockSleep, NULL);
    pthread_mutex_init(&u->mutexPa, NULL);
    pthread_mutex_init(&u->mutexPa2, NULL);

    if (pa_thread_mq_init(&u->thread_mq, m->core->mainloop, u->rtpoll) < 0) {
        AUDIO_ERR_LOG("pa_thread_mq_init() failed.");
        return -1;
    }

    AUDIO_DEBUG_LOG("Load sink adapter");
    int32_t ret = LoadSinkAdapter(pa_modargs_get_value(ma, "device_class", DEFAULT_DEVICE_CLASS),
        pa_modargs_get_value(ma, "network_id", DEFAULT_DEVICE_NETWORKID), &u->primary.sinkAdapter);
    if (ret) {
        AUDIO_ERR_LOG("Load adapter failed");
        return -1;
    }
    if (pa_modargs_get_value_u32(ma, "fixed_latency", &u->fixed_latency) < 0) {
        AUDIO_ERR_LOG("Failed to parse fixed latency argument.");
        return -1;
    }
    if (pa_modargs_get_value_s32(ma, "device_type", &u->deviceType) < 0) {
        AUDIO_ERR_LOG("Failed to parse deviceType argument.");
        return -1;
    }

    u->adapterName = pa_modargs_get_value(ma, "adapter_name", DEFAULT_DEVICE_CLASS);
    u->sink_latency = 0;
    if (pa_modargs_get_value_u32(ma, "sink_latency", &u->sink_latency) < 0) {
        AUDIO_ERR_LOG("No sink_latency argument.");
    }

    u->deviceNetworkId = pa_modargs_get_value(ma, "network_id", DEFAULT_DEVICE_NETWORKID);

    if (pa_modargs_get_value_u32(ma, "render_in_idle_state", &u->render_in_idle_state) < 0) {
        AUDIO_ERR_LOG("Failed to parse render_in_idle_state  argument.");
        return -1;
    }

    if (pa_modargs_get_value_u32(ma, "open_mic_speaker", &u->open_mic_speaker) < 0) {
        AUDIO_ERR_LOG("Failed to parse open_mic_speaker argument.");
        return -1;
    }

    u->test_mode_on = false;
    if (pa_modargs_get_value_boolean(ma, "test_mode_on", &u->test_mode_on) < 0) {
        AUDIO_INFO_LOG("No test_mode_on arg. Normal mode it is.");
    }

    return 0;
}

static int32_t PaHdiSinkNewInitUserDataAndSink(pa_module *m, pa_modargs *ma, const char *driver, struct Userdata *u)
{
    if (pa_modargs_get_value_boolean(ma, "offload_enable", &u->offload_enable) < 0) {
        AUDIO_ERR_LOG("Failed to parse offload_enable argument.");
        return -1;
    }

    pa_atomic_store(&u->primary.dflag, 0);
    u->primary.dq = pa_asyncmsgq_new(0);

    u->sink = PaHdiSinkInit(u, ma, driver);
    if (!u->sink) {
        AUDIO_ERR_LOG("Failed to create sink object");
        return -1;
    }

    u->sink->parent.process_msg = SinkProcessMsg;
    u->sink->set_state_in_io_thread = SinkSetStateInIoThreadCb;
    if (!u->fixed_latency) {
        u->sink->update_requested_latency = SinkUpdateRequestedLatencyCb;
    }
    u->sink->userdata = u;

    pa_sink_set_asyncmsgq(u->sink, u->thread_mq.inq);
    pa_sink_set_rtpoll(u->sink, u->rtpoll);

    u->bytes_dropped = 0;
    u->buffer_size = DEFAULT_BUFFER_SIZE;
    if (pa_modargs_get_value_u32(ma, "buffer_size", &u->buffer_size) < 0) {
        AUDIO_ERR_LOG("Failed to parse buffer_size argument.");
        return -1;
    }

    u->block_usec = pa_bytes_to_usec(u->buffer_size, &u->sink->sample_spec);

    if (u->fixed_latency) {
        pa_sink_set_fixed_latency(u->sink, u->block_usec);
    } else {
        pa_sink_set_latency_range(u->sink, 0, u->block_usec);
    }

    pa_sink_set_max_request(u->sink, u->buffer_size);

    return 0;
}

pa_sink *PaHdiSinkNew(pa_module *m, pa_modargs *ma, const char *driver)
{
    struct Userdata *u = NULL;
    char *hdiThreadName = NULL;
    char *hdiThreadNameMch = NULL;

    pa_assert(m);
    pa_assert(ma);

    u = pa_xnew0(struct Userdata, 1);
    pa_assert(u);

    if (PaHdiSinkNewInitUserData(m, ma, u) < 0) {
        goto fail;
    }

    if (PaHdiSinkNewInitUserDataAndSink(m, ma, driver, u) < 0) {
        goto fail;
    }

    int32_t ret = PaHdiSinkNewInitThread(m, ma, u);
    if (ret) {
        AUDIO_ERR_LOG("PaHdiSinkNewInitThread failed");
        goto fail;
    }

    if (u->test_mode_on) {
        u->writeCount = 0;
        u->renderCount = 0;
        hdiThreadName = "OS_WriteHdiTest";
        if (!(u->primary.thread_hdi = pa_thread_new(hdiThreadName, TestModeThreadFuncWriteHDI, u))) {
            AUDIO_ERR_LOG("Failed to test-mode-write-hdi thread.");
            goto fail;
        }
    } else {
        hdiThreadName = "OS_WriteHdi";
        if (!(u->primary.thread_hdi = pa_thread_new(hdiThreadName, ThreadFuncWriteHDI, u))) {
            AUDIO_ERR_LOG("Failed to write-hdi-primary thread.");
            goto fail;
        }
        if (!strcmp(u->sink->name, "Speaker")) {
            hdiThreadNameMch = "OS_WriteHdiMch";
            if (!(u->multiChannel.thread_hdi = pa_thread_new(hdiThreadNameMch, ThreadFuncWriteHDIMultiChannel, u))) {
                AUDIO_ERR_LOG("Failed to write-hdi-multichannel thread.");
                goto fail;
            }
        }
    }

    u->primary.writeTime = DEFAULT_WRITE_TIME;
    u->multiChannel.writeTime = DEFAULT_WRITE_TIME;
    pa_sink_put(u->sink);

    return u->sink;
fail:
    UserdataFree(u);

    return NULL;
}

static void UserdataFreeOffload(struct Userdata *u)
{
    if (u->offload.msgq) {
        pa_asyncmsgq_unref(u->offload.msgq);
    }

    if (u->offload.sinkAdapter) {
        u->offload.sinkAdapter->RendererSinkStop(u->offload.sinkAdapter);
        u->offload.sinkAdapter->RendererSinkDeInit(u->offload.sinkAdapter);
        UnLoadSinkAdapter(u->offload.sinkAdapter);
    }

    if (u->offload.chunk.memblock) {
        pa_memblock_unref(u->offload.chunk.memblock);
    }
}

static void UserdataFreeMultiChannel(struct Userdata *u)
{
    AUDIO_DEBUG_LOG("UserdataFreeMultiChannel");
    if (u->multiChannel.msgq) {
        pa_asyncmsgq_unref(u->multiChannel.msgq);
    }

    if (u->multiChannel.dq) {
        pa_asyncmsgq_unref(u->multiChannel.dq);
    }

    if (u->multiChannel.sinkAdapter) {
        u->multiChannel.sinkAdapter->RendererSinkStop(u->multiChannel.sinkAdapter);
        u->multiChannel.sinkAdapter->RendererSinkDeInit(u->multiChannel.sinkAdapter);
        UnLoadSinkAdapter(u->multiChannel.sinkAdapter);
    }

    if (u->multiChannel.chunk.memblock) {
        pa_memblock_unref(u->multiChannel.chunk.memblock);
    }
}

static void UserdataFreeThread(struct Userdata *u)
{
    if (u->thread) {
        pa_asyncmsgq_send(u->thread_mq.inq, NULL, PA_MESSAGE_SHUTDOWN, NULL, 0, NULL);
        pa_thread_free(u->thread);
    }

    if (u->offload.thread) {
        pa_asyncmsgq_send(u->offload.msgq, NULL, QUIT, NULL, 0, NULL);
        pa_thread_free(u->offload.thread);
    }

    if (u->multiChannel.thread) {
        pa_asyncmsgq_send(u->multiChannel.msgq, NULL, QUIT, NULL, 0, NULL);
        pa_thread_free(u->multiChannel.thread);
    }

    if (u->multiChannel.thread_hdi) {
        pa_asyncmsgq_post(u->multiChannel.dq, NULL, QUIT, NULL, 0, NULL, NULL);
        pa_thread_free(u->multiChannel.thread_hdi);
    }

    if (u->primary.thread) {
        pa_asyncmsgq_send(u->primary.msgq, NULL, QUIT, NULL, 0, NULL);
        pa_thread_free(u->primary.thread);
    }

    if (u->primary.thread_hdi) {
        pa_asyncmsgq_post(u->primary.dq, NULL, QUIT, NULL, 0, NULL, NULL);
        pa_thread_free(u->primary.thread_hdi);
    }

    pa_thread_mq_done(&u->thread_mq);
}

static void UserdataFree(struct Userdata *u)
{
    pa_assert(u);

    if (u->sink) {
        pa_sink_unlink(u->sink);
    }

    UserdataFreeThread(u);

    if (u->sink) {
        pa_sink_unref(u->sink);
    }

    if (u->memchunk.memblock) {
        pa_memblock_unref(u->memchunk.memblock);
    }

    if (u->rtpoll) {
        pa_rtpoll_free(u->rtpoll);
    }

    if (u->primary.msgq) {
        pa_asyncmsgq_unref(u->primary.msgq);
    }

    if (u->primary.dq) {
        pa_asyncmsgq_unref(u->primary.dq);
    }

    if (u->primary.sinkAdapter) {
        u->primary.sinkAdapter->RendererSinkStop(u->primary.sinkAdapter);
        u->primary.sinkAdapter->RendererSinkDeInit(u->primary.sinkAdapter);
        UnLoadSinkAdapter(u->primary.sinkAdapter);
    }

    UserdataFreeOffload(u);
    UserdataFreeMultiChannel(u);
    pa_xfree(u);
    AUDIO_DEBUG_LOG("UserdataFree done");
}

void PaHdiSinkFree(pa_sink *s)
{
    struct Userdata *u = NULL;

    pa_sink_assert_ref(s);
    pa_assert_se(u = s->userdata);

    UserdataFree(u);
}
