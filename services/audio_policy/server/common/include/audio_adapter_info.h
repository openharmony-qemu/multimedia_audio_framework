/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_POLICY_CONFIG_H
#define ST_AUDIO_POLICY_CONFIG_H

#include <list>
#include <set>
#include <unordered_map>
#include <string>

#include "audio_module_info.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const char* STR_INIT = "";

static const char* ADAPTER_PRIMARY_TYPE = "primary";
static const char* ADAPTER_A2DP_TYPE = "a2dp";
static const char* ADAPTER_HEARING_AID_TYPE = "hearing_aid";
static const char* ADAPTER_REMOTE_TYPE = "remote";
static const char* ADAPTER_FILE_TYPE = "file";
static const char* ADAPTER_USB_TYPE = "usb";
static const char* ADAPTER_DP_TYPE = "dp";
static const char* ADAPTER_ACCESSORY_TYPE = "accessory";

static const char* ADAPTER_DEVICE_PRIMARY_SPEAKER = "Speaker";
static const char* ADAPTER_DEVICE_PRIMARY_EARPIECE = "Earpicece";
static const char* ADAPTER_DEVICE_PRIMARY_MIC = "Built-In Mic";
static const char* ADAPTER_DEVICE_PRIMARY_WIRE_HEADSET = "Wired Headset";
static const char* ADAPTER_DEVICE_PRIMARY_WIRE_HEADPHONE = "Wired Headphones";
static const char* ADAPTER_DEVICE_PRIMARY_BT_SCO = "Bt Sco";
static const char* ADAPTER_DEVICE_PRIMARY_BT_OFFLOAD = "Bt Offload";
static const char* ADAPTER_DEVICE_PRIMARY_BT_HEADSET_HIFI = "Usb Headset Hifi";
static const char* ADAPTER_DEVICE_A2DP_BT_A2DP = "Bt A2dp";
static const char* ADAPTER_DEVICE_HEARING_AID = "Hearing Aid";
static const char* ADAPTER_DEVICE_REMOTE_SINK = "Remote Sink";
static const char* ADAPTER_DEVICE_REMOTE_SOURCE = "Remote Source";
static const char* ADAPTER_DEVICE_FILE_SINK = "File Sink";
static const char* ADAPTER_DEVICE_FILE_SOURCE = "File Source";
static const char* ADAPTER_DEVICE_USB_HEADSET_ARM = "Usb Headset Arm";
static const char* ADAPTER_DEVICE_USB_SPEAKER = "Usb_arm_speaker";
static const char* ADAPTER_DEVICE_USB_MIC = "Usb_arm_mic";
static const char* ADAPTER_DEVICE_PIPE_SINK = "fifo_output";
static const char* ADAPTER_DEVICE_PIPE_SOURCE = "fifo_input";
static const char* ADAPTER_DEVICE_WAKEUP = "Built_in_wakeup";
static const char* ADAPTER_DEVICE_NONE = "none";
static const char* ADAPTER_DEVICE_DP = "DP Sink";
static const char* ADAPTER_DEVICE_ACCESSORY = "accessory source";

static const char* MODULE_TYPE_SINK = "sink";
static const char* MODULE_TYPE_SOURCE = "source";
static const char* MODULE_SINK_OFFLOAD = "offload";
static const char* MODULE_SINK_LIB = "libmodule-hdi-sink.z.so";
static const char* MODULE_SOURCE_LIB = "libmodule-hdi-source.z.so";
static const char* MODULE_FILE_SINK_FILE = "/data/data/.pulse_dir/file_sink.pcm";
static const char* MODULE_FILE_SOURCE_FILE = "/data/data/.pulse_dir/file_source.pcm";

static const char* CONFIG_TYPE_PRELOAD = "preload";
static const char* CONFIG_TYPE_MAXINSTANCES = "maxinstances";

static const uint32_t DEFAULT_PERIOD_IN_MS = 20; // 20ms
}

enum class XmlNodeType {
    ADAPTERS,
    VOLUME_GROUPS,
    INTERRUPT_GROUPS,
    GLOBAL_CONFIGS,
    XML_UNKNOWN
};

enum class AdaptersType {
    TYPE_PRIMARY,
    TYPE_A2DP,
    TYPE_USB,
    TYPE_FILE_IO,
    TYPE_REMOTE_AUDIO,
    TYPE_DP,
    TYPE_ACCESSORY,
    TYPE_INVALID
};

enum class AdapterType {
    PIPES,
    DEVICES,
    UNKNOWN
};

enum class PipeType {
    PA_PROP,
    STREAM_PROP,
    CONFIGS,
    UNKNOWN
};

enum class GlobalConfigType {
    DEFAULT_OUTPUT,
    COMMON_CONFIGS,
    PA_CONFIGS,
    DEFAULT_MAX_CON_CURRENT_INSTANCE,
    UNKNOWN
};

enum class PAConfigType {
    AUDIO_LATENCY,
    SINK_LATENCY,
    UNKNOWN
};

enum class DefaultMaxInstanceType {
    OUTPUT,
    INPUT,
    UNKNOWN
};

enum class StreamType {
    NORMAL,
    FAST,
    UNKNOWN
};

struct ConfigInfo {
    std::string name_ = STR_INIT;
    std::string value_ = STR_INIT;
    std::string type_ = STR_INIT;
};

struct ProfileInfo {
    std::string rate_ = STR_INIT;
    std::string channels_ = STR_INIT;
    std::string format_ = STR_INIT;
    std::string bufferSize_ = STR_INIT;
};

struct AudioAdapterDeviceInfo {
    std::string name_ = STR_INIT;
    std::string type_ = STR_INIT;
    std::string role_ = STR_INIT;
};

struct StreamPropInfo {
    std::string format_ = STR_INIT;
    uint32_t sampleRate_ = 0;
    uint32_t periodInMs_ = DEFAULT_PERIOD_IN_MS;
    uint32_t channelLayout_ = 0;
    uint32_t bufferSize_ = 0;
};

struct PipeInfo {
    std::string name_ = STR_INIT;
    std::string pipeRole_ = STR_INIT;
    std::string pipeFlags_ = STR_INIT;
    std::string moduleName_ = STR_INIT;

    std::string lib_ = STR_INIT;
    std::string paPropRole_ = STR_INIT;
    std::string fixedLatency_ = STR_INIT;
    std::string renderInIdleState_ = STR_INIT;

    int32_t audioFlag_ = AUDIO_FLAG_NORMAL;
    int32_t audioUsage_ = AUDIO_USAGE_NORMAL;

    std::list<StreamPropInfo> streamPropInfos_ {};
    std::list<uint32_t> sampleRates_ {};
    std::list<uint32_t> channelLayouts_ {};
    std::list<ConfigInfo> configInfos_ {};
};

struct AudioPipeDeviceInfo {
    std::string name_ = STR_INIT;
    std::string type_ = STR_INIT;
    std::string pin_ = STR_INIT;
    std::string role_ = STR_INIT;
    std::list<std::string> supportPipes_ {};
};

struct ModuleInfo {
    std::string moduleType_ = STR_INIT;

    std::string name_ = STR_INIT;
    std::string lib_ = STR_INIT;
    std::string role_ = STR_INIT;
    std::string fixedLatency_ = STR_INIT;
    std::string renderInIdleState_ = STR_INIT;
    std::string profile_ = STR_INIT;
    std::string file_ = STR_INIT;

    std::list<ConfigInfo> configInfos_ {};
    std::list<ProfileInfo> profileInfos_ {};
    std::list<std::string> devices_ {};
};

struct AudioAdapterInfo {
    PipeInfo *GetPipeByName(const std::string &pipeName);
    AudioPipeDeviceInfo *GetDeviceInfoByDeviceType(DeviceType deviceType);

    std::string adapterName_ = STR_INIT;
    std::string adaptersupportScene_ = STR_INIT;
    std::list<AudioPipeDeviceInfo> deviceInfos_ {};
    std::list<PipeInfo> pipeInfos_ {};
};

struct GlobalPaConfigs {
    std::string audioLatency_ = STR_INIT;
    std::string sinkLatency_ = STR_INIT;
};

struct GlobalConfigs {
    std::string adapter_ = STR_INIT;
    std::string pipe_ = STR_INIT;
    std::string device_ = STR_INIT;
    std::list<ConfigInfo> commonConfigs_ {};
    bool updateRouteSupport_ = false;
    GlobalPaConfigs globalPaConfigs_;
    std::list<ConfigInfo> outputConfigInfos_ {};
    std::list<ConfigInfo> inputConfigInfos_ {};
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_POLICY_CONFIG_H
