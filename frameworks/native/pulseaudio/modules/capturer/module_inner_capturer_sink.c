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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifndef LOG_TAG
#define LOG_TAG "ModuleInnerCapturerSink"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>

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

#include "securec.h"
#include "audio_common_log.h"
#include "audio_utils_c.h"
#include "audio_volume_c.h"

PA_MODULE_AUTHOR("OpenHarmony");
PA_MODULE_DESCRIPTION(_("Inner Capturer Sink"));
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

#define DEFAULT_SINK_NAME "InnerCapturer"
#define DEFAULT_BUFFER_SIZE 8192  // same as HDI Sink
#define PA_ERR (-1)
const char *SINK_NAME_INNER_CAPTURER = "InnerCapturerSink";

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
};

static const char * const VALID_MODARGS[] = {
    "sink_name",
    "sink_properties",
    "format",
    "rate",
    "channels",
    "channel_map",
    "buffer_size",
    "formats",
    NULL
};

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

    pa_assert(s);
    pa_assert_se(u = s->userdata);

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
    struct userdata *u = s->userdata;

    pa_assert(u);

    pa_idxset_free(u->formats, (pa_free_cb_t) pa_format_info_free);
    u->formats = pa_idxset_copy(formats, (pa_copy_func_t) pa_format_info_copy);

    return true;
}

static pa_idxset* SinkGetFormatsCb(pa_sink *s)
{
    struct userdata *u = s->userdata;

    pa_assert(u);

    return pa_idxset_copy(u->formats, (pa_copy_func_t) pa_format_info_copy);
}

static void ProcessRewind(struct userdata *u, pa_usec_t now)
{
    size_t rewindNbytes;
    size_t inBuffer;
    pa_usec_t delay;

    pa_assert(u);

    rewindNbytes = u->sink->thread_info.rewind_nbytes;
    if (!PA_SINK_IS_OPENED(u->sink->thread_info.state) || rewindNbytes <= 0) {
        goto do_nothing;
    }
    AUDIO_DEBUG_LOG("Requested to rewind %lu bytes.", (unsigned long) rewindNbytes);

    if (u->timestamp <= now) {
        goto do_nothing;
    }

    delay = u->timestamp - now;
    inBuffer = pa_usec_to_bytes(delay, &u->sink->sample_spec);
    if (inBuffer <= 0) {
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

static const char *SafeProplistGets(const pa_proplist *p, const char *key, const char *defstr)
{
    const char *res = pa_proplist_gets(p, key);
    if (res == NULL) {
        return defstr;
    }
    return res;
}

static void SetSinkVolumeBySinkName(pa_sink *s)
{
    pa_assert(s);
    void *state = NULL;
    pa_sink_input *input;
    while ((input = pa_hashmap_iterate(s->thread_info.inputs, &state, NULL))) {
        pa_sink_input_assert_ref(input);
        if (input->thread_info.state != PA_SINK_INPUT_RUNNING) {
            continue;
        }
        const char *streamType = SafeProplistGets(input->proplist, "stream.type", "NULL");
        const char *sessionIDStr = SafeProplistGets(input->proplist, "stream.sessionID", "NULL");
        uint32_t sessionID = sessionIDStr != NULL ? atoi(sessionIDStr) : 0;
        float volumeFloat = 1.0f;
        if (!strcmp(s->name, SINK_NAME_INNER_CAPTURER)) { // inner capturer only stream volume
            volumeFloat = GetStreamVolume(sessionID);
        } else {
            struct VolumeValues volumes = {0.0f, 0.0f, 0.0f};
            volumeFloat = GetCurVolume(sessionID, streamType, s->name, &volumes);
        }
        uint32_t volume = pa_sw_volume_from_linear(volumeFloat);
        pa_cvolume_set(&input->thread_info.soft_volume, input->thread_info.soft_volume.channels, volume);
    }
}

static void UnsetSinkVolume(pa_sink *s)
{
    pa_assert(s);
    void *state = NULL;
    pa_sink_input *input;
    while ((input = pa_hashmap_iterate(s->thread_info.inputs, &state, NULL))) {
        pa_sink_input_assert_ref(input);
        if (input->thread_info.state != PA_SINK_INPUT_RUNNING) {
            continue;
        }
        uint32_t volume = pa_sw_volume_from_linear(1.0f);
        pa_cvolume_set(&input->thread_info.soft_volume, input->thread_info.soft_volume.channels, volume);
    }
}

static void ProcessRender(struct userdata *u, pa_usec_t now)
{
    size_t ate = 0;

    pa_assert(u);

    // update use volume
    SetSinkVolumeBySinkName(u->sink);

    /* This is the configured latency. Sink inputs connected to us
    might not have a single frame more than the maxrequest value
    queued. Hence: at maximum read this many bytes from the sink
    inputs. */

    /* Fill the buffer up the latency size */
    while (u->timestamp < now + u->block_usec) {
        pa_memchunk chunk;

        pa_sink_render(u->sink, u->sink->thread_info.max_request, &chunk);
        AUTO_CTRACE("inner_capturer_sink: ProcessRender len %zu, max_request %zu", chunk.length,
            u->sink->thread_info.max_request);
        pa_memblock_unref(chunk.memblock);

        u->timestamp += pa_bytes_to_usec(chunk.length, &u->sink->sample_spec);

        ate += chunk.length;
        if (ate >= u->sink->thread_info.max_request) {
            break;
        }
    }

    UnsetSinkVolume(u->sink);
}

static void ThreadFunc(void *userdata)
{
    struct userdata *u = userdata;

    pa_assert(u);

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

        if (PA_UNLIKELY(u->sink->thread_info.rewind_requested)) {
            ProcessRewind(u, now);
        }

        /* Render some data and drop it immediately */
        if (PA_SINK_IS_OPENED(u->sink->thread_info.state)) {
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

int InitFailed(pa_module *m, pa_modargs *ma)
{
    AUDIO_ERR_LOG("Inner Capturer Sink Init Failed");
    if (ma)
        pa_modargs_free(ma);

    pa__done(m);

    return PA_ERR;
}

int CreateSink(pa_module *m, pa_modargs *ma, struct userdata *u)
{
    pa_sample_spec ss;
    pa_channel_map map;
    pa_sink_new_data data;
    pa_format_info *format;

    pa_assert(m);

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
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_DESCRIPTION, _("Null Output"));
    pa_proplist_sets(data.proplist, PA_PROP_DEVICE_STRING, "innercapturer");

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

int pa__init(pa_module *m)
{
    struct userdata *u = NULL;
    pa_modargs *ma = NULL;
    size_t nbytes;
    int mq;
    int mg;

    pa_assert(m);

    ma = pa_modargs_new(m->argument, VALID_MODARGS);
    CHECK_AND_RETURN_RET_LOG(ma != NULL, InitFailed(m, ma), "Failed to parse module arguments:%{public}s", m->argument);

    m->userdata = u = pa_xnew0(struct userdata, 1);
    u->core = m->core;
    u->module = m;
    u->rtpoll = pa_rtpoll_new();

    mq = pa_thread_mq_init(&u->thread_mq, m->core->mainloop, u->rtpoll);
    CHECK_AND_RETURN_RET_LOG(mq >=0, InitFailed(m, ma), "pa_thread_mq_init() failed.");

    if (CreateSink(m, ma, u) != 0) {
        return InitFailed(m, ma);
    }

    pa_sink_set_asyncmsgq(u->sink, u->thread_mq.inq);
    pa_sink_set_rtpoll(u->sink, u->rtpoll);

    u->buffer_size = DEFAULT_BUFFER_SIZE;

    mg = pa_modargs_get_value_u32(ma, "buffer_size", &u->buffer_size);
    CHECK_AND_RETURN_RET_LOG(mg >= 0, InitFailed(m, ma),
        "Failed to parse buffer_size arg in capturer sink");

    u->block_usec = pa_bytes_to_usec(u->buffer_size, &u->sink->sample_spec);
    nbytes = pa_usec_to_bytes(u->block_usec, &u->sink->sample_spec);

    pa_sink_set_max_rewind(u->sink, nbytes);

    pa_sink_set_max_request(u->sink, u->buffer_size);

    if (!(u->thread = pa_thread_new("OS_InnerCap", ThreadFunc, u))) {
        AUDIO_ERR_LOG("Failed to create thread.");
        return InitFailed(m, ma);
    }
    pa_sink_set_latency_range(u->sink, 0, u->block_usec);

    pa_sink_put(u->sink);

    pa_modargs_free(ma);

    return 0;
}

int pa__get_n_used(pa_module *m)
{
    struct userdata *u;

    pa_assert(m);
    pa_assert_se(u = m->userdata);

    return pa_sink_linked_by(u->sink);
}

void pa__done(pa_module*m)
{
    struct userdata *u;

    pa_assert(m);

    if (!(u = m->userdata)) {
        return;
    }

    if (u->sink) {
        pa_sink_unlink(u->sink);
    }

    if (u->thread) {
        pa_asyncmsgq_send(u->thread_mq.inq, NULL, PA_MESSAGE_SHUTDOWN, NULL, 0, NULL);
        pa_thread_free(u->thread);
    }

    pa_thread_mq_done(&u->thread_mq);

    if (u->sink) {
        pa_sink_unref(u->sink);
    }

    if (u->rtpoll) {
        pa_rtpoll_free(u->rtpoll);
    }

    if (u->formats) {
        pa_idxset_free(u->formats, (pa_free_cb_t) pa_format_info_free);
    }

    pa_xfree(u);
    m->userdata = NULL;
}
