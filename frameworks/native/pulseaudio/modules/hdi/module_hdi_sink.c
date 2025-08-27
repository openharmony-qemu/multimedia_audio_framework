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

#include <pa_config.h>
#include <pulsecore/log.h>
#include <pulsecore/modargs.h>
#include <pulsecore/module.h>
#include <pulsecore/sink.h>
#include <stddef.h>
#include <stdbool.h>

#include <pulsecore/core-util.h>
#include <pulsecore/core.h>
#include <pulsecore/namereg.h>
#include "audio_effect_chain_adapter.h"
#include "audio_hdi_log.h"
#include "playback_capturer_adapter.h"
#include "sink_userdata.h"

pa_sink *PaHdiSinkNew(pa_module *m, pa_modargs *ma, const char *driver);
void PaHdiSinkFree(pa_sink *s);
void PaInputVolumeChangeCb(pa_sink_input *i);

PA_MODULE_AUTHOR("OpenHarmony");
PA_MODULE_DESCRIPTION("OpenHarmony HDI Sink");
PA_MODULE_VERSION(PACKAGE_VERSION);
PA_MODULE_LOAD_ONCE(false);
PA_MODULE_USAGE(
        "sink_name=<name for the sink> "
        "device_class=<name for the device class> "
        "sink_properties=<properties for the sink> "
        "format=<sample format> "
        "rate=<sample rate> "
        "channels=<number of channels> "
        "channel_map=<channel map> "
        "buffer_size=<custom buffer size>"
        "file_path=<file path for data writing>"
        "adapter_name=<primary>"
        "fixed_latency=<latency measure>"
        "sink_latency=<hdi latency>"
        "render_in_idle_state<renderer state>"
        "open_mic_speaker<open mic and speaker>"
        "test_mode_on<is test mode on>"
        "network_id<device network id>"
        "device_type<device type or port>"
        "offload_enable<if device support offload>"
        "default_adapter_enable<if set default adapter>"
        );

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
    NULL
};

static pa_hook_result_t SinkInputNewCb(const pa_core *c, pa_sink_input *si)
{
    CHECK_AND_RETURN_RET_LOG(c != NULL, PA_HOOK_OK, "pa core is null");

    const char *flush = pa_proplist_gets(si->proplist, "stream.flush");
    const char *sceneMode = pa_proplist_gets(si->proplist, "scene.mode");
    const char *sceneType = pa_proplist_gets(si->proplist, "scene.type");
    const char *deviceString = pa_proplist_gets(si->sink->proplist, PA_PROP_DEVICE_STRING);
    const char *sessionID = pa_proplist_gets(si->proplist, "stream.sessionID");
    const uint32_t channels = si->sample_spec.channels;
    const char *channelLayout = pa_proplist_gets(si->proplist, "stream.channelLayout");
    const char *spatializationEnabled = pa_proplist_gets(si->proplist, "spatialization.enabled");
    const char *streamUsage = pa_proplist_gets(si->proplist, "stream.usage");
    const char *systemVolumeType = pa_proplist_gets(si->proplist, "systemVolume.type");
    if (pa_safe_streq(deviceString, "remote")) {
        EffectChainManagerReleaseCb(sceneType, sessionID);
        return PA_HOOK_OK;
    }

    const char *appUser = pa_proplist_gets(si->proplist, "application.process.user");
    if (pa_safe_streq(appUser, "daudio")) {
        return PA_HOOK_OK;
    }

    const char *clientUid = pa_proplist_gets(si->proplist, "stream.client.uid");
    const char *bootUpMusic = "1003";
    if (!pa_safe_streq(clientUid, bootUpMusic)) {
        if (!pa_safe_streq(sceneMode, "EFFECT_NONE") && pa_safe_streq(flush, "true")) {
            EffectChainManagerInitCb(sceneType);
        }
        EffectChainManagerCreateCb(sceneType, sessionID);
        SessionInfoPack pack = {channels, channelLayout, sceneMode, spatializationEnabled, streamUsage,
            systemVolumeType};
        if (si->thread_info.state == PA_SINK_INPUT_RUNNING &&
            !EffectChainManagerAddSessionInfo(sceneType, sessionID, pack)) {
            EffectChainManagerMultichannelUpdate(sceneType);
            EffectChainManagerEffectUpdate();
            EffectChainManagerStreamUsageUpdate();
        }
    }
    return PA_HOOK_OK;
}

static pa_hook_result_t SinkInputUnlinkCb(const pa_core *c, pa_sink_input *si, void *u)
{
    CHECK_AND_RETURN_RET_LOG(c != NULL, PA_HOOK_OK, "pa core is null");

    const char *sceneType = pa_proplist_gets(si->proplist, "scene.type");
    const char *deviceString = pa_proplist_gets(si->sink->proplist, PA_PROP_DEVICE_STRING);
    if (pa_safe_streq(deviceString, "remote")) {
        return PA_HOOK_OK;
    }

    const char *appUser = pa_proplist_gets(si->proplist, "application.process.user");
    if (pa_safe_streq(appUser, "daudio")) {
        return PA_HOOK_OK;
    }

    const char *clientUid = pa_proplist_gets(si->proplist, "stream.client.uid");
    const char *bootUpMusic = "1003";
    if (!pa_safe_streq(clientUid, bootUpMusic)) {
        const char *sessionID = pa_proplist_gets(si->proplist, "stream.sessionID");
        EffectChainManagerReleaseCb(sceneType, sessionID);
        if (si->thread_info.state == PA_SINK_INPUT_RUNNING &&
            !EffectChainManagerDeleteSessionInfo(sceneType, sessionID)) {
            EffectChainManagerMultichannelUpdate(sceneType);
            EffectChainManagerEffectUpdate();
            EffectChainManagerStreamUsageUpdate();
        }
    }
    return PA_HOOK_OK;
}

static pa_hook_result_t SinkInputStateChangedCb(const pa_core *c, pa_sink_input *si, void *u)
{
    CHECK_AND_RETURN_RET_LOG(c != NULL, PA_HOOK_OK, "pa core is null");
    CHECK_AND_RETURN_RET_LOG(si != NULL, PA_HOOK_OK, "sink input is null");

    const char *sceneMode = pa_proplist_gets(si->proplist, "scene.mode");
    const char *sceneType = pa_proplist_gets(si->proplist, "scene.type");
    const char *sessionID = pa_proplist_gets(si->proplist, "stream.sessionID");
    const uint32_t channels = si->sample_spec.channels;
    const char *channelLayout = pa_proplist_gets(si->proplist, "stream.channelLayout");
    const char *spatializationEnabled = pa_proplist_gets(si->proplist, "spatialization.enabled");
    const char *streamUsage = pa_proplist_gets(si->proplist, "stream.usage");
    const char *clientUid = pa_proplist_gets(si->proplist, "stream.client.uid");
    const char *systemVolumeType = pa_proplist_gets(si->proplist, "systemVolume.type");
    const char *bootUpMusic = "1003";

    if (si->thread_info.state == PA_SINK_INPUT_RUNNING && si->sink &&
        !pa_safe_streq(clientUid, bootUpMusic)) {
        SessionInfoPack pack = {channels, channelLayout, sceneMode, spatializationEnabled, streamUsage,
            systemVolumeType};
        if (!EffectChainManagerAddSessionInfo(sceneType, sessionID, pack)) {
            EffectChainManagerMultichannelUpdate(sceneType);
            EffectChainManagerVolumeUpdate(sessionID);
            EffectChainManagerEffectUpdate();
            EffectChainManagerStreamUsageUpdate();
        }
    }

    if ((si->thread_info.state == PA_SINK_INPUT_CORKED || si->thread_info.state == PA_SINK_INPUT_UNLINKED) &&
        si->sink && !pa_safe_streq(clientUid, bootUpMusic)) {
        if (!EffectChainManagerDeleteSessionInfo(sceneType, sessionID)) {
            EffectChainManagerMultichannelUpdate(sceneType);
            EffectChainManagerVolumeUpdate(sessionID);
            EffectChainManagerEffectUpdate();
            EffectChainManagerStreamUsageUpdate();
        }
    }
    return PA_HOOK_OK;
}

int pa__init(pa_module *m)
{
    pa_modargs *ma = NULL;

    CHECK_AND_RETURN_RET_LOG(m != NULL, 0, "pa module is null");

    if (!(ma = pa_modargs_new(m->argument, VALID_MODARGS))) {
        pa_log("Failed to parse module arguments");
        goto fail;
    }

    if (!(m->userdata = PaHdiSinkNew(m, ma, __FILE__))) {
        goto fail;
    }
    pa_sink *sink = (pa_sink*)m->userdata;
    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SINK_INPUT_PROPLIST_CHANGED], PA_HOOK_LATE,
        (pa_hook_cb_t)SinkInputNewCb, sink->userdata);
    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SINK_INPUT_UNLINK], PA_HOOK_LATE,
        (pa_hook_cb_t)SinkInputUnlinkCb, sink->userdata);
    // SourceOutputStateChangedCb will be replaced by UpdatePlaybackCaptureConfig in CapturerInServer
    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SINK_INPUT_STATE_CHANGED], PA_HOOK_LATE,
        (pa_hook_cb_t)SinkInputStateChangedCb, sink->userdata);

    pa_modargs_free(ma);

    return 0;

fail:
    if (ma) {
        pa_modargs_free(ma);
    }

    pa__done(m);

    return -1;
}

int pa__get_n_used(pa_module *m)
{
    CHECK_AND_RETURN_RET_LOG(m != NULL, 0, "pa module is null");

    pa_sink *sink = m->userdata;

    CHECK_AND_RETURN_RET_LOG(sink != NULL, 0, "sink is null");

    return pa_sink_linked_by(sink);
}

void pa__done(pa_module *m)
{
    AUDIO_INFO_LOG("In");
    CHECK_AND_RETURN_LOG(m != NULL, "pa module is null");

    pa_sink *sink = NULL;

    if ((sink = m->userdata)) {
        PaHdiSinkFree(sink);
    }
}
