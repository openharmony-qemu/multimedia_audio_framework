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
#undef LOG_TAG
#define LOG_TAG "ModuleHdiSource"

#include <pa_config.h>
#include <pulsecore/log.h>
#include <pulsecore/modargs.h>
#include <pulsecore/module.h>
#include <pulsecore/source.h>
#include <stddef.h>
#include <stdbool.h>

#include <pulsecore/core.h>
#include <pulsecore/core-util.h>
#include <pulsecore/namereg.h>

#include "securec.h"
#include "audio_hdi_log.h"
#include "audio_enhance_chain_adapter.h"
#include "source_userdata.h"
#include "common/hdi_adapter_info.h"

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
    "ec_type",
    "ec_adapter",
    "ec_sampling_rate",
    "ec_format",
    "ec_channels",
    "open_mic_ref",
    "mic_ref_rate",
    "mic_ref_format",
    "mic_ref_channels",
    "default_adapter_enable",
    NULL
};

static void IncreaseScenekeyCount(pa_hashmap *sceneMap, const char *key)
{
    if (sceneMap == NULL) {
        return;
    }
    uint32_t *num = NULL;
    if ((num = (uint32_t *)pa_hashmap_get(sceneMap, key)) != NULL) {
        (*num)++;
    } else {
        char *sceneKey;
        sceneKey = strdup(key);
        if (sceneKey != NULL) {
            num = pa_xnew0(uint32_t, 1);
            *num = 1;
            if (pa_hashmap_put(sceneMap, sceneKey, num) != 0) {
                AUDIO_ERR_LOG("pa_hashmap_put failed");
                free(sceneKey);
                pa_xfree(num);
            }
        }
    }
}

static bool DecreaseScenekeyCount(pa_hashmap *sceneMap, const char *key)
{
    if (sceneMap == NULL) {
        return false;
    }
    uint32_t *num = NULL;
    if ((num = (uint32_t *)pa_hashmap_get(sceneMap, key)) != NULL) {
        (*num)--;
        if (*num == 0) {
            pa_hashmap_remove_and_free(sceneMap, key);
            return true;
        }
    }
    return false;
}

static enum AudioSampleFormatIntf ConvertPaFormat(pa_sample_format_t paFormat)
{
    enum AudioSampleFormatIntf format;
    switch (paFormat) {
        case PA_SAMPLE_U8:
            format = SAMPLE_U8;
            break;
        case PA_SAMPLE_S16LE:
        case PA_SAMPLE_S16BE:
            format = SAMPLE_S16;
            break;
        case PA_SAMPLE_S24LE:
        case PA_SAMPLE_S24BE:
            format = SAMPLE_S24;
            break;
        case PA_SAMPLE_S32LE:
        case PA_SAMPLE_S32BE:
            format = SAMPLE_S32;
            break;
        default:
            format = SAMPLE_S16;
            break;
    }

    return format;
}

static void SetResampler(pa_source_output *so, const char *sceneKey, const struct Userdata *u,
    const struct AlgoSpecs *algoSpecs)
{
    pa_hashmap *preResamplerMap = (pa_hashmap *)u->sceneToPreResamplerMap;
    pa_hashmap *ecResamplerMap = (pa_hashmap *)u->sceneToEcResamplerMap;
    pa_hashmap *micRefResamplerMap = (pa_hashmap *)u->sceneToMicRefResamplerMap;
    if (!pa_sample_spec_equal(&so->source->sample_spec, &algoSpecs->micSpec)) {
        AUDIO_INFO_LOG("SOURCE spec:%{public}u_%{public}u_%{public}u ALGO spec:%{public}u_%{public}u_%{public}u",
            so->source->sample_spec.rate, (uint32_t)so->source->sample_spec.channels,
            (uint32_t)ConvertPaFormat(so->source->sample_spec.format),
            algoSpecs->micSpec.rate,  (uint32_t)algoSpecs->micSpec.channels,
            (uint32_t)ConvertPaFormat(algoSpecs->micSpec.format));
        pa_resampler *preResampler = pa_resampler_new(so->source->core->mempool,
            &so->source->sample_spec, &so->source->channel_map,
            &algoSpecs->micSpec, &so->source->channel_map,
            so->source->core->lfe_crossover_freq,
            PA_RESAMPLER_AUTO, PA_RESAMPLER_VARIABLE_RATE);
        pa_hashmap_put(preResamplerMap, pa_xstrdup(sceneKey), preResampler);
        pa_resampler *postResampler = pa_resampler_new(so->source->core->mempool,
            &algoSpecs->micSpec, &so->source->channel_map,
            &so->sample_spec, &so->source->channel_map,
            so->source->core->lfe_crossover_freq,
            PA_RESAMPLER_AUTO, PA_RESAMPLER_VARIABLE_RATE);
        so->thread_info.resampler = postResampler;
    }
    if ((u->ecType != EC_NONE) && (algoSpecs->ecSpec.rate != 0) &&
        (!pa_sample_spec_equal(&u->ecSpec, &algoSpecs->ecSpec))) {
        AUDIO_INFO_LOG("EC SOURCE spec:%{public}u_%{public}u_%{public}u ALGO spec:%{public}u_%{public}u_%{public}u",
            u->ecSpec.rate, (uint32_t)u->ecSpec.channels, (uint32_t)ConvertPaFormat(u->ecSpec.format),
            algoSpecs->ecSpec.rate,  (uint32_t)algoSpecs->ecSpec.channels,
            (uint32_t)ConvertPaFormat(algoSpecs->ecSpec.format));
        pa_resampler *ecResampler = pa_resampler_new(so->source->core->mempool,
            &u->ecSpec, &so->source->channel_map,
            &algoSpecs->ecSpec, &so->source->channel_map,
            so->source->core->lfe_crossover_freq,
            PA_RESAMPLER_AUTO, PA_RESAMPLER_VARIABLE_RATE);
        pa_hashmap_put(ecResamplerMap, pa_xstrdup(sceneKey), ecResampler);
    }
    if ((u->micRef == REF_ON) && (algoSpecs->micRefSpec.rate != 0) &&
        (!pa_sample_spec_equal(&u->micRefSpec, &algoSpecs->micRefSpec))) {
        AUDIO_INFO_LOG("MIC REF SOURCE spec:%{public}u_%{public}u_%{public}u ALGO:%{public}u_%{public}u_%{public}u",
            u->micRefSpec.rate, (uint32_t)u->micRefSpec.channels, (uint32_t)ConvertPaFormat(u->micRefSpec.format),
            algoSpecs->micRefSpec.rate,  (uint32_t)algoSpecs->micRefSpec.channels,
            (uint32_t)ConvertPaFormat(algoSpecs->micRefSpec.format));
        pa_resampler *micRefResampler = pa_resampler_new(so->source->core->mempool,
            &u->micRefSpec, &so->source->channel_map,
            &algoSpecs->micRefSpec, &so->source->channel_map,
            so->source->core->lfe_crossover_freq,
            PA_RESAMPLER_AUTO, PA_RESAMPLER_VARIABLE_RATE);
        pa_hashmap_put(micRefResamplerMap, pa_xstrdup(sceneKey), micRefResampler);
    }
}

static void SetDefaultResampler(pa_source_output *so, const pa_sample_spec *algoConfig, struct Userdata *u)
{
    if (!pa_sample_spec_equal(&so->source->sample_spec, algoConfig)) {
        if (u->defaultSceneResampler) {
            pa_resampler_free(u->defaultSceneResampler);
        }
        AUDIO_INFO_LOG("default SOURCE:%{public}u_%{public}u_%{public}u ALGO:%{public}u_%{public}u_%{public}u",
            so->source->sample_spec.rate, (uint32_t)so->source->sample_spec.channels,
            (uint32_t)ConvertPaFormat(so->source->sample_spec.format),
            algoConfig->rate,  (uint32_t)algoConfig->channels,
            (uint32_t)ConvertPaFormat(algoConfig->format));
        u->defaultSceneResampler = pa_resampler_new(so->source->core->mempool,
            &so->source->sample_spec, &so->source->channel_map,
            algoConfig, &so->source->channel_map,
            so->source->core->lfe_crossover_freq,
            PA_RESAMPLER_AUTO, PA_RESAMPLER_VARIABLE_RATE);
        pa_resampler *postResampler = pa_resampler_new(so->source->core->mempool,
            algoConfig, &so->source->channel_map,
            &so->sample_spec, &so->source->channel_map,
            so->source->core->lfe_crossover_freq,
            PA_RESAMPLER_AUTO, PA_RESAMPLER_VARIABLE_RATE);
        so->thread_info.resampler = postResampler;
    }
}

static uint32_t GetByteSizeByFormat(enum AudioSampleFormatIntf format)
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

static void InitDeviceAttrAdapter(struct DeviceAttrAdapter *deviceAttrAdapter, struct Userdata *u)
{
    deviceAttrAdapter->micRate = u->attrs.sampleRate;
    deviceAttrAdapter->micChannels = u->attrs.channel;
    deviceAttrAdapter->micFormat = GetByteSizeByFormat(u->attrs.format);
    deviceAttrAdapter->needEc = false;
    deviceAttrAdapter->needMicRef = false;
    if (u->ecType != EC_NONE) {
        deviceAttrAdapter->needEc = true;
        deviceAttrAdapter->ecRate = u->ecSamplingRate;
        deviceAttrAdapter->ecChannels = u->ecChannels;
        deviceAttrAdapter->ecFormat = GetByteSizeByFormat(u->ecFormat);
    } else {
        deviceAttrAdapter->ecRate = 0;
        deviceAttrAdapter->ecChannels = 0;
        deviceAttrAdapter->ecFormat = 0;
    }
    if (u->micRef == REF_ON) {
        deviceAttrAdapter->needMicRef = true;
        deviceAttrAdapter->micRefRate = u->micRefRate;
        deviceAttrAdapter->micRefChannels = u->micRefChannels;
        deviceAttrAdapter->micRefFormat = GetByteSizeByFormat(u->micRefFormat);
    } else {
        deviceAttrAdapter->micRefRate = 0;
        deviceAttrAdapter->micRefChannels = 0;
        deviceAttrAdapter->micRefFormat = 0;
    }
}

static pa_hook_result_t GetAlgoSpecs(uint64_t sceneKeyCode, struct AlgoSpecs *algoSpecs)
{
    pa_sample_spec_init(&algoSpecs->micSpec);
    pa_sample_spec_init(&algoSpecs->ecSpec);
    pa_sample_spec_init(&algoSpecs->micRefSpec);

    if (EnhanceChainManagerGetAlgoConfig(sceneKeyCode, &algoSpecs->micSpec, &algoSpecs->ecSpec,
        &algoSpecs->micRefSpec) != 0) {
        AUDIO_ERR_LOG("Get algo config failed");
        return PA_HOOK_OK;
    }
    return PA_HOOK_OK;
}

static pa_hook_result_t HandleSourceOutputPut(pa_source_output *so, struct Userdata *u)
{
    const char *sceneType = pa_proplist_gets(so->proplist, "scene.type");
    if (sceneType == NULL) {
        sceneType = "";
    }
    const char *sceneBypass = pa_proplist_gets(so->proplist, "scene.bypass");
    if (pa_safe_streq(sceneBypass, DEFAULT_SCENE_BYPASS)) {
        AUDIO_INFO_LOG("scene:%{public}s has been set to bypass", sceneType);
        return PA_HOOK_OK;
    }
    uint32_t captureId = u->captureId;
    uint32_t renderId = u->renderId;
    uint64_t sceneTypeCode = 0;
    if (GetSceneTypeCode(sceneType, &sceneTypeCode) != 0) {
        AUDIO_ERR_LOG("scenetype:%{public}s GetSceneTypeCode failed", sceneType);
        pa_proplist_sets(so->proplist, "scene.bypass", DEFAULT_SCENE_BYPASS);
        return PA_HOOK_OK;
    }
    uint64_t sceneKeyCode = 0;
    AUDIO_INFO_LOG("sceneTypeCode = %{public}" PRIu64 ", captureId = %{public}u, renderId = %{public}u",
        sceneTypeCode, captureId, renderId);
    sceneKeyCode = (sceneTypeCode << SCENE_TYPE_OFFSET) + (captureId << CAPTURER_ID_OFFSET) + renderId;
    struct DeviceAttrAdapter deviceAttrAdapter;
    InitDeviceAttrAdapter(&deviceAttrAdapter, u);
    int32_t ret = EnhanceChainManagerCreateCb(sceneKeyCode, &deviceAttrAdapter);
    if (ret < 0) {
        AUDIO_INFO_LOG("Create EnhanceChain failed, set to bypass");
        pa_proplist_sets(so->proplist, "scene.bypass", DEFAULT_SCENE_BYPASS);
        return PA_HOOK_OK;
    }
    struct AlgoSpecs algoSpecs;
    GetAlgoSpecs(sceneKeyCode, &algoSpecs);
    // default chain
    if (ret > 0) {
        SetDefaultResampler(so, &algoSpecs.micSpec, u);
        pa_proplist_sets(so->proplist, "scene.default", "1");
    } else {
        char sceneKey[MAX_SCENE_NAME_LEN];
        if (sprintf_s(sceneKey, sizeof(sceneKey), "%lu", sceneKeyCode) < 0) {
            AUDIO_ERR_LOG("sprintf from sceneKeyCode to sceneKey failed");
            return PA_HOOK_OK;
        }
        IncreaseScenekeyCount(u->sceneToCountMap, sceneKey);
        SetResampler(so, sceneKey, u, &algoSpecs);
    }
    EnhanceChainManagerInitEnhanceBuffer();
    return PA_HOOK_OK;
}

static void HandleSourceOutputUnlinkForRemote(struct Userdata *u)
{
    CHECK_AND_RETURN_LOG(u != NULL, "Get Userdata failed! userdata is NULL");
    CHECK_AND_RETURN_LOG(u->sourceAdapter != NULL, "sourceAdapter is null");
    CHECK_AND_RETURN_LOG(u->sourceAdapter->deviceClass != NULL, "deviceClass is null");
    CHECK_AND_RETURN_LOG(u->source != NULL, "source is null");

    if (strcmp(u->sourceAdapter->deviceClass, "remote") != 0) {
        AUDIO_INFO_LOG("HandleSourceOutputUnlinkForRemote: not remote source");
    } else {
        void *state = NULL;
        pa_source_output *sourceOutput;
        bool noStreamRunning = true;
        while ((sourceOutput = pa_hashmap_iterate(u->source->thread_info.outputs, &state, NULL))) {
            if (sourceOutput->thread_info.state == PA_SOURCE_OUTPUT_RUNNING) {
                noStreamRunning = false;
                break;
            }
        }
        if (!noStreamRunning) {
            AUDIO_INFO_LOG("HandleSourceOutputUnlinkForRemote: source has running stream");
        } else {
            u->sourceAdapter->SourceAdapterStop(u->sourceAdapter);
        }
    }
}

static pa_hook_result_t HandleSourceOutputUnlink(pa_source_output *so, struct Userdata *u)
{
    // If remote source has no running stream, stop source
    HandleSourceOutputUnlinkForRemote(u);

    const char *sceneType = pa_proplist_gets(so->proplist, "scene.type");
    if (sceneType == NULL) {
        sceneType = "";
    }
    const char *sceneBypass = pa_proplist_gets(so->proplist, "scene.bypass");
    if (pa_safe_streq(sceneBypass, DEFAULT_SCENE_BYPASS)) {
        AUDIO_INFO_LOG("scene:%{public}s has been set to bypass, do not need release", sceneType);
        return PA_HOOK_OK;
    }
    uint32_t captureId = u->captureId;
    uint32_t renderId = u->renderId;
    uint64_t sceneTypeCode = 0;
    if (GetSceneTypeCode(sceneType, &sceneTypeCode) != 0) {
        AUDIO_ERR_LOG("scenetype:%{public}s GetSceneTypeCode failed", sceneType);
        return PA_HOOK_OK;
    }
    uint64_t sceneKeyCode = 0;
    sceneKeyCode = (sceneTypeCode << SCENE_TYPE_OFFSET) + (captureId << CAPTURER_ID_OFFSET) + renderId;
    EnhanceChainManagerReleaseCb(sceneKeyCode);
    char sceneKey[MAX_SCENE_NAME_LEN];
    if (sprintf_s(sceneKey, sizeof(sceneKey), "%lu", sceneKeyCode) < 0) {
        AUDIO_ERR_LOG("sprintf from sceneKeyCode to sceneKey failed");
        return PA_HOOK_OK;
    }
    if (!pa_safe_streq(pa_proplist_gets(so->proplist, "scene.default"), "1") &&
        DecreaseScenekeyCount(u->sceneToCountMap, sceneKey)) {
        pa_hashmap_remove_and_free(u->sceneToPreResamplerMap, sceneKey);
        pa_hashmap_remove_and_free(u->sceneToEcResamplerMap, sceneKey);
        pa_hashmap_remove_and_free(u->sceneToMicRefResamplerMap, sceneKey);
    }
    return PA_HOOK_OK;
}

static pa_hook_result_t CheckIfAvailSource(pa_source_output *so, struct Userdata *u)
{
    pa_source *soSource = so->source;
    pa_source *thisSource = u->source;
    if (soSource == NULL || thisSource == NULL) {
        return PA_HOOK_CANCEL;
    }
    if (soSource->index != thisSource->index) {
        AUDIO_INFO_LOG("NOT correspondant SOURCE %{public}s AND %{public}s.", soSource->name, thisSource->name);
        return PA_HOOK_CANCEL;
    }
    return PA_HOOK_OK;
}

static pa_hook_result_t SourceOutputPutCb(const pa_core *c, pa_source_output *so, struct Userdata *u)
{
    CHECK_AND_RETURN_RET_LOG(u != NULL, PA_HOOK_OK, "Get Userdata failed! userdata is NULL");
    CHECK_AND_RETURN_RET_LOG(c != NULL, PA_HOOK_OK, "pa core is null");
    CHECK_AND_RETURN_RET_LOG(so != NULL, PA_HOOK_OK, "so is NULL");

    const char *sessionID = pa_proplist_gets(so->proplist, "stream.sessionID");
    if (sessionID == NULL) {
        sessionID = "";
    }
    AUDIO_INFO_LOG("Trigger SourceOutputPutCb sessionID:%{public}s", sessionID);

    if (CheckIfAvailSource(so, u) == PA_HOOK_CANCEL) {
        return PA_HOOK_OK;
    }
    return HandleSourceOutputPut(so, u);
}

static pa_hook_result_t SourceOutputUnlinkCb(const pa_core *c, pa_source_output *so, struct Userdata *u)
{
    CHECK_AND_RETURN_RET_LOG(u != NULL, PA_HOOK_OK, "Get Userdata failed! userdata is NULL");
    CHECK_AND_RETURN_RET_LOG(c != NULL, PA_HOOK_OK, "pa core is null");
    CHECK_AND_RETURN_RET_LOG(so != NULL, PA_HOOK_OK, "so is NULL");

    const char *sessionID = pa_proplist_gets(so->proplist, "stream.sessionID");
    if (sessionID == NULL) {
        sessionID = "";
    }
    AUDIO_INFO_LOG("Trigger SourceOutputUnlinkCb sessionID:%{public}s", sessionID);

    if (CheckIfAvailSource(so, u) == PA_HOOK_CANCEL) {
        return PA_HOOK_OK;
    }
    return HandleSourceOutputUnlink(so, u);
}

static pa_hook_result_t SourceOutputMoveStartCb(const pa_core *c, pa_source_output *so, struct Userdata *u)
{
    CHECK_AND_RETURN_RET_LOG(u != NULL, PA_HOOK_OK, "Get Userdata failed! userdata is NULL");
    CHECK_AND_RETURN_RET_LOG(c != NULL, PA_HOOK_OK, "pa core is null");
    CHECK_AND_RETURN_RET_LOG(so != NULL, PA_HOOK_OK, "so is NULL");

    const char *sessionID = pa_proplist_gets(so->proplist, "stream.sessionID");
    if (sessionID == NULL) {
        sessionID = "";
    }
    AUDIO_INFO_LOG("Trigger SourceOutputMoveStartCb sessionID:%{public}s", sessionID);

    if (CheckIfAvailSource(so, u) == PA_HOOK_CANCEL) {
        return PA_HOOK_OK;
    }
    return HandleSourceOutputUnlink(so, u);
}

static pa_hook_result_t SourceOutputMoveFinishCb(const pa_core *c, pa_source_output *so, struct Userdata *u)
{
    CHECK_AND_RETURN_RET_LOG(u != NULL, PA_HOOK_OK, "Get Userdata failed! userdata is NULL");
    CHECK_AND_RETURN_RET_LOG(c != NULL, PA_HOOK_OK, "pa core is null");
    CHECK_AND_RETURN_RET_LOG(so != NULL, PA_HOOK_OK, "so is NULL");

    const char *sessionID = pa_proplist_gets(so->proplist, "stream.sessionID");
    if (sessionID == NULL) {
        sessionID = "";
    }
    AUDIO_INFO_LOG("Trigger SourceOutputMoveFinishCb sessionID:%{public}s", sessionID);

    if (CheckIfAvailSource(so, u) == PA_HOOK_CANCEL) {
        return PA_HOOK_OK;
    }
    return HandleSourceOutputPut(so, u);
}

int pa__init(pa_module *m)
{
    pa_modargs *ma = NULL;

    CHECK_AND_RETURN_RET_LOG(m != NULL, PA_HOOK_OK, "pa core is null");

    if (!(ma = pa_modargs_new(m->argument, VALID_MODARGS))) {
        AUDIO_ERR_LOG("Failed to parse module arguments");
        goto fail;
    }

    if (!(m->userdata = PaHdiSourceNew(m, ma, __FILE__))) {
        AUDIO_ERR_LOG("Failed to PaHdiSourceNew");
        goto fail;
    }
    pa_source *source = (pa_source *)m->userdata;

    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SOURCE_OUTPUT_PUT], PA_HOOK_LATE,
        (pa_hook_cb_t)SourceOutputPutCb, source->userdata);
    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SOURCE_OUTPUT_UNLINK], PA_HOOK_LATE,
        (pa_hook_cb_t)SourceOutputUnlinkCb, source->userdata);
    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SOURCE_OUTPUT_MOVE_FINISH], PA_HOOK_LATE,
        (pa_hook_cb_t)SourceOutputMoveFinishCb, source->userdata);
    pa_module_hook_connect(m, &m->core->hooks[PA_CORE_HOOK_SOURCE_OUTPUT_MOVE_START], PA_HOOK_LATE,
        (pa_hook_cb_t)SourceOutputMoveStartCb, source->userdata);

    pa_source_put(source);
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
    CHECK_AND_RETURN_RET_LOG(m != NULL, PA_HOOK_OK, "pa core is null");

    pa_source *source = m->userdata;

    CHECK_AND_RETURN_RET_LOG(source != NULL, 0, "source is null");

    return pa_source_linked_by(source);
}

static void ReleaseAllChains(struct Userdata *u)
{
    void *state = NULL;
    uint32_t *sceneKeyNum;
    const void *sceneKey;
    while ((sceneKeyNum = pa_hashmap_iterate(u->sceneToCountMap, &state, &sceneKey))) {
        uint64_t sceneKeyCode = (uint64_t)strtoul((char *)sceneKey, NULL, BASE_TEN);
        for (uint32_t count = 0; count < *sceneKeyNum; count++) {
            EnhanceChainManagerReleaseCb(sceneKeyCode);
        }
    }
}

void pa__done(pa_module *m)
{
    pa_source *source = NULL;

    CHECK_AND_RETURN_LOG(m != NULL, "pa core is null");

    if ((source = m->userdata)) {
        struct Userdata *u = (struct Userdata *)source->userdata;
        if (u != NULL) {
            AUDIO_INFO_LOG("Release all enhChains on [%{public}s]", source->name);
            ReleaseAllChains(u);
        }
        PaHdiSourceFree(source);
        m->userdata = NULL;
    }
}