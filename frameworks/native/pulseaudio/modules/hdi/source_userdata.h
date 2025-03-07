/*
 * Copyright (c) 2024-2024 Huawei Device Co., Ltd.
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
#ifndef SOURCE_USERDATA_H
#define SOURCE_USERDATA_H

#include <sys/eventfd.h>
#include <poll.h>
#include <pulsecore/core.h>
#include <pulsecore/log.h>
#include <pulsecore/module.h>
#include <pulsecore/source.h>
#include <pulsecore/thread-mq.h>
#include <pulsecore/thread.h>
#include <pulsecore/hashmap.h>

#include "common/hdi_adapter_info.h"
#include "intf_def.h"

#define DEFAULT_SCENE_BYPASS "scene.bypass"
#define MAX_SCENE_NAME_LEN 100
#define SCENE_TYPE_OFFSET 32
#define CAPTURER_ID_OFFSET 16
#define BYTE_SIZE_SAMPLE_U8 1
#define BYTE_SIZE_SAMPLE_S16 2
#define BYTE_SIZE_SAMPLE_S24 3
#define BYTE_SIZE_SAMPLE_S32 4
#define BASE_TEN 10

struct Userdata {
    pa_core *core;
    pa_module *module;
    pa_source *source;
    pa_thread *thread;
    pa_thread_mq threadMq;
    pa_thread *threadCap;
    pa_asyncmsgq *CaptureMq;
    pa_rtpoll *rtpoll;
    uint32_t bufferSize;
    uint32_t openMicSpeaker;
    pa_usec_t blockUsec;
    pa_usec_t timestamp;
    struct SourceAdapterAttr attrs;
    bool isCapturerStarted;
    pa_atomic_t captureFlag;
    pa_atomic_t quitCaptureFlag;
    EcType ecType;
    const char *ecAdapaterName;
    uint32_t ecSamplingRate;
    int32_t ecFormat;
    uint32_t ecChannels;
    pa_sample_spec ecSpec;
    MicRefSwitch micRef;
    uint32_t micRefRate;
    int32_t micRefFormat;
    uint32_t micRefChannels;
    pa_sample_spec micRefSpec;
    struct SourceAdapter *sourceAdapter;
    pa_hashmap *sceneToCountMap;
    pa_hashmap *sceneToPreResamplerMap;
    pa_hashmap *sceneToEcResamplerMap;
    pa_hashmap *sceneToMicRefResamplerMap;
    struct SourceAdapter *sourceAdapterEc;
    struct SourceAdapter *sourceAdapterMicRef;
    uint64_t requestBytesEc;
    uint64_t requestBytesMicRef;
    void *bufferEc;
    void *bufferMicRef;
    uint32_t captureId;
    uint32_t renderId;
    pa_resampler *defaultSceneResampler;
    pa_rtpoll_item *rtpollItem;
    int eventFd;
};

struct AlgoSpecs {
    pa_sample_spec micSpec;
    pa_sample_spec ecSpec;
    pa_sample_spec micRefSpec;
};

#endif // SOURCE_USERDATA_H