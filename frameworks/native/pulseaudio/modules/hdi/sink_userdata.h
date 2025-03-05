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

#ifndef SINK_USERDATA_H
#define SINK_USERDATA_H

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
#include <pulsecore/memblock.h>
#include <pulsecore/mix.h>
#include <pulse/volume.h>
#include <pulsecore/protocol-native.h>
#include <pulsecore/memblockq.h>

#include "renderer_sink_adapter.h"

struct Userdata {
    const char *adapterName;
    uint32_t buffer_size;
    uint32_t fixed_latency;
    uint32_t sink_latency;
    uint32_t render_in_idle_state;
    uint32_t open_mic_speaker;
    bool render_full_enable;
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
    int32_t sinkSceneType;
    int32_t sinkSceneMode;
    pthread_mutex_t mutexPa;
    pthread_mutex_t mutexPa2;
    pthread_rwlock_t rwlockSleep;
    int64_t timestampSleep;
    pa_usec_t timestampLastLog;
    int8_t spatializationFadingState; // for indicating the fading state, =0:no fading, >0:fading in, <0:fading out
    int8_t spatializationFadingCount; // for indicating the fading rate
    bool actualSpatializationEnabled; // the spatialization state that actually applies effect
    bool isFirstStarted;
    bool isEffectBufferAllocated;
    uint64_t lastRecodedLatency;
    uint32_t continuesGetLatencyErrCount;
    uint32_t streamAvailable;
    uint32_t lastStreamAvailable;
    pa_hashmap *streamAvailableMap;
    struct {
        int32_t sessionID;
        bool firstWriteHdi; // for set volume onstart, avoid mute
        pa_usec_t pos;
        pa_usec_t hdiPos;
        pa_usec_t hdiPosTs;
        pa_usec_t prewrite;
        pa_thread *thread;
        pa_asyncmsgq *msgq;
        bool isHDISinkStarted;
        struct RendererSinkAdapter *sinkAdapter;
        pa_atomic_t hdistate; // 0:need_data 1:wait_consume 2:flushing
        pa_usec_t fullTs;
        bool runninglocked;
        pa_memchunk chunk;
        bool inited;
        int32_t setHdiBufferSizeNum; // for set hdi buffer size count
    } offload;
    struct {
        pa_usec_t timestamp;
        pa_usec_t lastProcessDataTime; // The timestamp from the last time the data was prepared to HDI
        pa_thread *thread;
        pa_thread *thread_hdi;
        pa_asyncmsgq *msgq;
        pa_atomic_t isHDISinkStarted;
        struct RendererSinkAdapter *sinkAdapter;
        pa_asyncmsgq *dq;
        pa_atomic_t dflag;
        pa_usec_t writeTime;
        pa_usec_t prewrite;
        pa_sink_state_t previousState;
        pa_atomic_t fadingFlagForPrimary; // 1：do fade in, 0: no need
        int32_t primaryFadingInDone;
        int32_t primarySinkInIndex;
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
        pa_atomic_t fadingFlagForMultiChannel; // 1：do fade in, 0: no need
        int32_t multiChannelFadingInDone;
        int32_t multiChannelSinkInIndex;
        int32_t multiChannelTmpSinkInIndex;
        uint32_t sinkChannel;
        uint64_t sinkChannelLayout;
    } multiChannel;
};

#endif // SINK_USERDATA_H