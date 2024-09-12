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
#define LOG_TAG "ModuleHdiSource"
#endif

#include <config.h>
#include <pulsecore/log.h>
#include <pulsecore/modargs.h>
#include <pulsecore/module.h>
#include <pulsecore/source.h>
#include <stddef.h>
#include <stdbool.h>

#include <pulsecore/core.h>
#include <pulsecore/core-util.h>
#include <pulsecore/namereg.h>

#include "audio_log.h"
#include "audio_enhance_chain_adapter.h"

pa_source *PaHdiSourceNew(pa_module *m, pa_modargs *ma, const char *driver);
void PaHdiSourceFree(pa_source *s);

PA_MODULE_AUTHOR("OpenHarmony");
PA_MODULE_DESCRIPTION("OpenHarmony HDI Source");
PA_MODULE_VERSION(PACKAGE_VERSION);
PA_MODULE_LOAD_ONCE(false);
PA_MODULE_USAGE(
        "source_name=<name for the source> "
        "device_class=<name for the device class> "
        "source_properties=<properties for the source> "
        "format=<sample format> "
        "rate=<sample rate> "
        "channels=<number of channels> "
        "channel_map=<channel map>"
        "buffer_size=<custom buffer size>"
        "file_path=<file path for data reading>"
        "adapter_name=<primary>"
        "open_mic_speaker<open mic>"
        "network_id<device network id>"
        "device_type<device type or port>"
        "source_type<source type or port>"
    );

static const char * const VALID_MODARGS[] = {
    "source_name",
    "device_class",
    "source_properties",
    "format",
    "rate",
    "channels",
    "channel_map",
    "buffer_size",
    "file_path",
    "adapter_name",
    "open_mic_speaker",
    "network_id",
    "device_type",
    "source_type",
    NULL
};

static pa_hook_result_t SourceOutputProplistChangedCb(pa_core *c, pa_source_output *so)
{
    AUDIO_DEBUG_LOG("Trigger SourceOutputProplistChangedCb");
    pa_assert(c);
    const char *sceneMode = pa_proplist_gets(so->proplist, "scene.mode");
    const char *sceneType = pa_proplist_gets(so->proplist, "scene.type");
    const char *upDevice = pa_proplist_gets(so->proplist, "device.up");
    const char *downDevice = pa_proplist_gets(so->proplist, "device.down");
    EnhanceChainManagerCreateCb(sceneType, sceneMode, upDevice, downDevice);
    return PA_HOOK_OK;
}

static pa_hook_result_t SourceOutputPutCb(pa_core *c, pa_source_output *so)
{
    AUDIO_DEBUG_LOG("Trigger SourceOutputPutCb");
    pa_assert(c);
    const char *sceneType = pa_proplist_gets(so->proplist, "scene.type");
    const char *sceneMode = pa_proplist_gets(so->proplist, "scene.mode");
    const char *upDevice = pa_proplist_gets(so->proplist, "device.up");
    const char *downDevice = pa_proplist_gets(so->proplist, "device.down");
    int32_t ret = EnhanceChainManagerCreateCb(sceneType, sceneMode, upDevice, downDevice);
    if (ret != 0) {
        return PA_HOOK_OK;
    }
    EnhanceChainManagerInitEnhanceBuffer();
    return PA_HOOK_OK;
}

static pa_hook_result_t SourceOutputUnlinkCb(pa_core *c, pa_source_output *so)
{
    AUDIO_DEBUG_LOG("Trigger SourceOutputUnlinkCb");
    pa_assert(c);
    const char *sceneType = pa_proplist_gets(so->proplist, "scene.type");
    const char *upDevice = pa_proplist_gets(so->proplist, "device.up");
    const char *downDevice = pa_proplist_gets(so->proplist, "device.down");
    EnhanceChainManagerReleaseCb(sceneType, upDevice, downDevice);
    return PA_HOOK_OK;
}

int pa__init(pa_module *m)
{
    pa_modargs *ma = NULL;

    pa_assert(m);

    if (!(ma = pa_modargs_new(m->argument, VALID_MODARGS))) {
        pa_log("Failed to parse module arguments");
        goto fail;
    }

    if (!(m->userdata = PaHdiSourceNew(m, ma, __FILE__))) {
        goto fail;
    }

    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SOURCE_OUTPUT_PROPLIST_CHANGED], PA_HOOK_LATE,
        (pa_hook_cb_t)SourceOutputProplistChangedCb, NULL);
    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SOURCE_OUTPUT_PUT], PA_HOOK_LATE,
        (pa_hook_cb_t)SourceOutputPutCb, NULL);
    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SOURCE_OUTPUT_UNLINK], PA_HOOK_LATE,
        (pa_hook_cb_t)SourceOutputUnlinkCb, NULL);

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
    pa_source *source = NULL;

    pa_assert(m);
    pa_assert_se(source = m->userdata);

    return pa_source_linked_by(source);
}

void pa__done(pa_module *m)
{
    pa_source *source = NULL;

    pa_assert(m);

    if ((source = m->userdata)) {
        PaHdiSourceFree(source);
    }
}
