/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef AUDIO_DEFINITION_POLICY_CONFIG_H
#define AUDIO_DEFINITION_POLICY_CONFIG_H

#include <list>
#include <set>
#include <unordered_map>
#include <string>

#include "audio_module_info.h"
#include "audio_info.h"
#include "audio_policy_log.h"
#include "audio_definition_policy_utils.h"
#include "audio_pipe_info.h"

namespace OHOS {
namespace AudioStandard {
static const char* STR_INITED = "";

static const char* ADAPTER_TYPE_PRIMARY = "primary";
static const char* ADAPTER_TYPE_A2DP = "a2dp";
static const char* ADAPTER_TYPE_HEARING_AID = "hearing_aid";
static const char* ADAPTER_TYPE_REMOTE = "remote";
static const char* ADAPTER_TYPE_FILE = "file";
static const char* ADAPTER_TYPE_USB = "usb";
static const char* ADAPTER_TYPE_DP = "dp";
static const char* ADAPTER_TYPE_ACCESSORY = "accessory";
static const char* ADAPTER_TYPE_SLE = "sle";

struct PairHash {
    template <class T1, class T2>
    std::size_t operator() (const std::pair<T1, T2> &tp) const
    {
        return std::hash<T1>()(std::get<0>(tp)) ^ std::hash<T2>()(std::get<1>(tp));
    }
};

enum class PolicyXmlNodeType {
    ADAPTERS,
    XML_UNKNOWN,
    VOLUME_GROUPS,
    INTERRUPT_GROUPS,
    GLOBAL_CONFIGS,
};

enum class AudioAdapterType {
    TYPE_PRIMARY,
    TYPE_A2DP,
    TYPE_USB,
    TYPE_FILE_IO,
    TYPE_REMOTE_AUDIO,
    TYPE_DP,
    TYPE_ACCESSORY,
    TYPE_SLE,
    TYPE_HEARING_AID,
    TYPE_INVALID
};

enum class AdapterInfoType {
    PIPES,
    DEVICES,
    UNKNOWN
};

enum class PipeInfoType {
    PA_PROP,
    STREAM_PROP,
    ATTRIBUTE,
    UNKNOWN
};

enum class PolicyGlobalConfigType {
    DEFAULT_OUTPUT,
    COMMON_CONFIGS,
    PA_CONFIGS,
    UNKNOWN
};

enum class PolicyPAConfigType {
    FAST_FORMAT,
    AUDIO_LATENCY,
    SINK_LATENCY,
    UNKNOWN
};

struct AttributeInfo {
    std::string name_ = STR_INITED;
    std::string value_ = STR_INITED;
};

struct PaPropInfo {
    std::string lib_ = STR_INITED;
    std::string role_ = STR_INITED;
    std::string moduleName_ = STR_INITED;
    std::string fixedLatency_ = STR_INITED;
    std::string renderInIdleState_ = STR_INITED;
};

class PipeStreamPropInfo;
class AdapterPipeInfo;
class AdapterDeviceInfo;
class PolicyAdapterInfo;

class PipeStreamPropInfo {
public:
    void SelfCheck();

    AudioSampleFormat format_ = INVALID_WIDTH;
    uint32_t sampleRate_ = 0;
    AudioChannelLayout channelLayout_ = CH_LAYOUT_UNKNOWN;
    AudioChannel channels_ = CHANNEL_UNKNOW;
    uint32_t bufferSize_ = 0;

    std::weak_ptr<AdapterPipeInfo> pipeInfo_;
    std::list<std::string> supportDevices_ {};
    std::unordered_map<DeviceType, std::shared_ptr<AdapterDeviceInfo>> supportDeviceMap_ {};
};

class AdapterPipeInfo {
public:
    void SelfCheck();

    void UpdateDynamicStreamProps(const std::list<std::shared_ptr<PipeStreamPropInfo>> &streamProps);
    void ClearDynamicStreamProps();

    std::string name_ = STR_INITED;
    AudioPipeRole role_ = PIPE_ROLE_NONE;
    PaPropInfo paProp_ {};

    AudioPreloadType preloadAttr_ = PRELOAD_TYPE_UNKNOWN;
    uint32_t supportFlags_ = AUDIO_FLAG_NONE;
    int32_t audioUsage_ = AUDIO_USAGE_NORMAL;
    bool supportEncodingEac3_ = false;

    std::weak_ptr<PolicyAdapterInfo> adapterInfo_;
    std::list<std::shared_ptr<PipeStreamPropInfo>> streamPropInfos_ {};
    std::list<std::shared_ptr<AttributeInfo>> attributeInfos_ {};

    // for dynamic
    std::mutex dynamicMtx_;
    std::list<std::shared_ptr<PipeStreamPropInfo>> dynamicStreamPropInfos_ {};
    std::list<std::string> supportDevices_ {};
};

class AdapterDeviceInfo {
public:
    void SelfCheck();

    std::string name_ = STR_INITED;
    DeviceType type_ = DEVICE_TYPE_NONE;
    AudioPin pin_ = AUDIO_PIN_NONE;
    DeviceRole role_ = DEVICE_ROLE_NONE;

    std::weak_ptr<PolicyAdapterInfo> adapterInfo_;
    std::list<std::string> supportPipes_ {};
    std::unordered_map<uint32_t, std::shared_ptr<AdapterPipeInfo>> supportPipeMap_ {}; // flag <-> pipeInfo
};

class PolicyAdapterInfo {
public:
    PolicyAdapterInfo();
    ~PolicyAdapterInfo();
    void SelfCheck();

    static AudioAdapterType GetAdapterType(const std::string &adapterName);
    AudioAdapterType GetTypeEnum();
    std::shared_ptr<AdapterDeviceInfo> GetDeviceInfoByType(DeviceType deviceType, DeviceRole role);
    std::shared_ptr<AdapterPipeInfo> GetPipeInfoByName(const std::string &pipeName);

    std::string adapterName = STR_INITED;
    std::string adapterSupportScene = STR_INITED;
    std::list<std::shared_ptr<AdapterDeviceInfo>> deviceInfos;
    std::list<std::shared_ptr<AdapterPipeInfo>> pipeInfos;
};

struct PolicyConfigInfo {
    std::string name_ = STR_INITED;
    std::string value_ = STR_INITED;
    std::string type_ = STR_INITED;
};

struct PolicyGlobalPaConfigs {
    std::string audioLatency_ = STR_INITED;
    std::string sinkLatency_ = STR_INITED;
};

struct PolicyGlobalConfigs {
    std::string adapter_ = STR_INITED;
    std::string pipe_ = STR_INITED;
    std::string device_ = STR_INITED;
    std::list<PolicyConfigInfo> commonConfigs_ {};
    bool updateRouteSupport_ = false;
    PolicyGlobalPaConfigs globalPaConfigs_;
};

class AudioPolicyConfigData {
public:
    static AudioPolicyConfigData&  GetInstance();
    void Reorganize();
    void SelfCheck();

    void SetVersion(const std::string &version);

    std::string GetVersion();
    std::shared_ptr<AdapterDeviceInfo> GetAdapterDeviceInfo(DeviceType type_, DeviceRole role_,
        const std::string &networkId_, uint32_t flags, int32_t a2dpOffloadFlag = 0);

    void UpdateDynamicStreamProps(const std::string adapterName, const std::string &pipeName,
        const std::list<std::shared_ptr<PipeStreamPropInfo>> &streamProps);
    void ClearDynamicStreamProps(const std::string adapterName, const std::string &pipeName);
    uint32_t GetConfigStreamPropsSize(const std::string adapterName, const std::string &pipeName) const;
    uint32_t GetDynamicStreamPropsSize(const std::string adapterName, const std::string &pipeName) const;

    std::weak_ptr<AdapterPipeInfo> pipeInfo_;
    std::list<std::string> supportDevices_ {};

    std::unordered_map<AudioAdapterType, std::shared_ptr<PolicyAdapterInfo>> adapterInfoMap {};
    std::unordered_map<std::pair<DeviceType, DeviceRole>,
        std::set<std::shared_ptr<AdapterDeviceInfo>>, PairHash> deviceInfoMap {};
private:
    AudioPolicyConfigData() = default;
    AudioPolicyConfigData(const AudioPolicyConfigData&) = delete;
    AudioPolicyConfigData& operator=(const AudioPolicyConfigData&) = delete;

    void SetDeviceInfoMap(std::list<std::shared_ptr<AdapterDeviceInfo>> &deviceInfos,
        std::unordered_map<std::string, std::shared_ptr<AdapterDeviceInfo>> &tmpDeviceInfoMap_);
    void SetSupportDeviceAndPipeMap(std::shared_ptr<AdapterPipeInfo> &pipeInfo_,
        std::unordered_map<std::string, std::shared_ptr<AdapterDeviceInfo>> &tmpDeviceInfoMap_);

    std::string version_ = STR_INITED;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_DEFINITION_POLICY_CONFIG_H
