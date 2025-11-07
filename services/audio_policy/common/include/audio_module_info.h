/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_CONFIG_H
#define ST_AUDIO_CONFIG_H

#include <list>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "audio_info.h"
#include "audio_policy_log.h"

namespace OHOS {
namespace AudioStandard {
namespace {
static const char* PRIMARY_CLASS = "primary";
static const char* A2DP_CLASS = "a2dp";
static const char* HEARING_AID_CLASS = "hearing_aid";
static const char* USB_CLASS = "usb";
static const char* DP_CLASS = "dp";
static const char* DP_MCH_CLASS = "dp_multichannel";
static const char* ACCESSORY_CLASS = "accessory";
static const char* FILE_CLASS = "file_io";
static const char* REMOTE_CLASS = "remote";
static const char* OFFLOAD_CLASS = "offload";
static const char* MCH_CLASS = "multichannel";
static const char* INVALID_CLASS = "invalid";
static const char* DIRECT_VOIP_CLASS = "primary_direct_voip";
static const char* MMAP_VOIP_CLASS = "primary_mmap_voip";
static const char* MMAP_CLASS = "primary_mmap";
static const char* DIRECT_CLASS = "primary_direct";
static const char* A2DP_FAST_CLASS = "a2dp_fast";
static const char* BLUETOOTH_SPEAKER = "Bt_Speaker";
static const char* BLUETOOTH_A2DP_FAST = "bt_a2dp_fast";
static const char* HEARING_AID_SPEAKER = "Hearing_Aid_Speaker";
static const char* BLUETOOTH_MIC = "Bt_Mic";
static const char* PRIMARY_SPEAKER = "Speaker";
static const char* OFFLOAD_PRIMARY_SPEAKER = "Offload_Speaker";
static const char* MCH_PRIMARY_SPEAKER = "MCH_Speaker";
static const char* USB_SPEAKER = "Usb_arm_speaker";
static const char* DP_SINK = "DP_speaker";
static const char* USB_MIC = "Usb_arm_mic";
static const char* PRIMARY_MIC = "Built_in_mic";
static const char* PRIMARY_WAKEUP_MIC = "Built_in_wakeup";
static const char* PRIMARY_AI_MIC = "Built_in_ai";
static const char* FILE_SINK = "file_sink";
static const char* FILE_SOURCE = "file_source";
static const char* PIPE_SINK = "fifo_output";
static const char* PIPE_SOURCE = "fifo_input";
static const char* INTERNAL_PORT = "internal";
static const char* ROLE_SOURCE = "source";
static const char* ROLE_SINK = "sink";
static const char* PORT_NONE = "none";
static const char* PRIMARY_DIRECT_VOIP = "direct_voip";
static const char* PRIMARY_MMAP_VOIP = "mmap_voip";
static const char* PRIMARY_MMAP = "mmap";
static const char* PRIMARY_DIRECT = "direct";
static const char* ACCESSORY_SOURCE = "accessory_mic";
static const char* VIRTUAL_AUDIO = "virtual_audio";
}

enum NodeName {
    DEVICE_CLASS,
    MODULES,
    MODULE,
    PORTS,
    PORT,
    AUDIO_INTERRUPT_ENABLE,
    UPDATE_ROUTE_SUPPORT,
    AUDIO_LATENCY,
    SINK_LATENCY,
    VOLUME_GROUP_CONFIG,
    INTERRUPT_GROUP_CONFIG,
    UNKNOWN
};

enum ClassType {
    TYPE_PRIMARY,
    TYPE_A2DP,
    TYPE_USB,
    TYPE_FILE_IO,
    TYPE_REMOTE_AUDIO,
    TYPE_DP,
    TYPE_ACCESSORY,
    TYPE_HEARING_AID,
    TYPE_INVALID
};

enum AudioPipeRole {
    PIPE_ROLE_OUTPUT = 0,
    PIPE_ROLE_INPUT,
    PIPE_ROLE_NONE,
};

struct AudioModuleInfo {
    std::string className;
    std::string name;
    std::string adapterName;
    std::string id;
    std::string lib;
    std::string role;

    std::string rate;

    std::set<uint32_t> supportedRate_;
    std::set<uint64_t> supportedChannelLayout_;

    std::string format;
    std::string channels;
    std::string channelLayout;
    std::string bufferSize;
    std::string fixedLatency;
    std::string sinkLatency;
    std::string renderInIdleState;
    std::string OpenMicSpeaker;
    std::string fileName;
    std::string networkId;
    std::string macAddress;
    std::string deviceType;
    std::string sceneName;
    std::string sourceType;
    std::string offloadEnable;
    std::string defaultAdapterEnable;

    std::string ecType;
    std::string ecAdapter;
    std::string ecSamplingRate;
    std::string ecFormat;
    std::string ecChannels;
    std::string openMicRef;
    std::string micRefRate;
    std::string micRefFormat;
    std::string micRefChannels;
    uint32_t suspendIdleTimeout = DEFAULT_SUSPEND_TIME_IN_MS;

    std::list<AudioModuleInfo> ports;
    std::string extra;
    AudioPipeRole pipeRole;
    /**
     * split stream, sent a few empty chunk when stream pause or stop
     */
    std::optional<bool> needEmptyChunk;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_CONFIG_H
