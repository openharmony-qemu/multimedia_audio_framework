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

#include "audio_core_config_manager.h"
#include "iaudio_policy_client.h"
#include "audio_device_utils.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
const std::string DEFAULT_MAC_ADDRESS = "00:11:22:33:44:55";
const int32_t MAX_TEST_UID = 10000;
const uint32_t DEVICE_SUPPORT_MMAP = 1;
typedef void (*TestFuncs)(FuzzedDataProvider&);

const vector<DeviceType> g_testOutputDeviceTypes = {
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_NEARLINK,
};

const vector<DeviceType> g_testInputDeviceTypes = {
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_NEARLINK_IN,
};

const vector<DeviceCategory> g_testDeviceCategories = {
    CATEGORY_DEFAULT,
    BT_HEADPHONE,
    BT_SOUNDBOX,
    BT_CAR,
    BT_GLASSES,
};

template<typename T>
T ConsumeEnum(FuzzedDataProvider& fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

template<typename T>
T PickValue(FuzzedDataProvider& fdp, const vector<T>& values)
{
    return values[fdp.ConsumeIntegralInRange<size_t>(0, values.size() - 1)];
}

std::string ConsumeString(FuzzedDataProvider& fdp, const std::string& fallback)
{
    std::string value = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    return value.empty() ? fallback : value;
}

std::shared_ptr<AudioDeviceDescriptor> MakeDeviceDescriptor(FuzzedDataProvider& fdp, DeviceRole role)
{
    const auto& deviceTypes = (role == INPUT_DEVICE) ? g_testInputDeviceTypes : g_testOutputDeviceTypes;
    auto desc = std::make_shared<AudioDeviceDescriptor>(PickValue(fdp, deviceTypes), role);
    desc->networkId_ = ConsumeString(fdp, LOCAL_NETWORK_ID);
    desc->macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    desc->deviceName_ = ConsumeString(fdp, "audio_fuzz_device");
    desc->deviceCategory_ = PickValue(fdp, g_testDeviceCategories);
    desc->deviceId_ = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_TEST_UID);
    desc->SetDeviceSupportMmap(fdp.ConsumeBool() ? DEVICE_SUPPORT_MMAP : 0);
    return desc;
}

void UpdateAndClearStreamPropInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::string adapterName = ConsumeString(fdp, "adapterName");
    std::string pipeName = ConsumeString(fdp, "pipeName");
    std::string networkId = ConsumeString(fdp, "fuzz_network_id");
    std::list<DeviceStreamInfo> deviceStreamInfo;
    DeviceStreamInfo streamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, CH_LAYOUT_STEREO };
    deviceStreamInfo.push_back(streamInfo);
    std::list<std::string> supportDevices;
    supportDevices.push_back(ConsumeString(fdp, "supportDevices"));
    AudioCoreConfigManager::GetInstance().UpdateStreamPropInfo(adapterName, pipeName,
        deviceStreamInfo, supportDevices, networkId);
    AudioCoreConfigManager::GetInstance().ClearStreamPropInfo(adapterName, pipeName, networkId);
}

void UpdateDynamicCapturerConfigFuzzTest(FuzzedDataProvider& fdp)
{
    AudioModuleInfo moduleInfo;
    AudioCoreConfigManager::GetInstance().GetAdapterInfoFlag();
    AudioCoreConfigManager::GetInstance().UpdateDynamicCapturerConfig(
        ConsumeEnum<ClassType>(fdp), moduleInfo);
}

void GetMaxCapturersInstancesFuzzTest(FuzzedDataProvider& fdp)
{
    PolicyGlobalConfigs globalConfigs;
    PolicyConfigInfo policyConfigInfo;
    if (fdp.ConsumeBool()) {
        policyConfigInfo.name_ = ConsumeString(fdp, "maxCapturers");
        policyConfigInfo.value_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    }
    globalConfigs.commonConfigs_.push_back(policyConfigInfo);
    AudioCoreConfigManager::GetInstance().OnGlobalConfigsParsed(globalConfigs);
    AudioCoreConfigManager::GetInstance().GetMaxCapturersInstances();
}

void GetMaxFastRenderersInstancesFuzzTest(FuzzedDataProvider& fdp)
{
    PolicyGlobalConfigs globalConfigs;
    PolicyConfigInfo policyConfigInfo;
    if (fdp.ConsumeBool()) {
        policyConfigInfo.name_ = ConsumeString(fdp, "maxFastRenderers");
        policyConfigInfo.value_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    }
    globalConfigs.commonConfigs_.push_back(policyConfigInfo);
    AudioCoreConfigManager::GetInstance().OnGlobalConfigsParsed(globalConfigs);
    AudioCoreConfigManager::GetInstance().GetMaxFastRenderersInstances();
}

void GetVoipRendererFlagFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCoreConfigManager::GetInstance().SetNormalVoipFlag(fdp.ConsumeBool());
    AudioCoreConfigManager::GetInstance().GetNormalVoipFlag();
    AudioCoreConfigManager::GetInstance().OnVoipConfigParsed(fdp.ConsumeBool());
    AudioCoreConfigManager::GetInstance().GetVoipConfig();
    AudioCoreConfigManager::GetInstance().GetVoipRendererFlag(ConsumeString(fdp, "Speaker"),
        ConsumeString(fdp, "LocalDevice"), ConsumeEnum<AudioSamplingRate>(fdp));
}

void SetAndGetAudioLatencyFromXmlFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCoreConfigManager::GetInstance().OnAudioLatencyParsed(fdp.ConsumeIntegral<uint64_t>());
    AudioCoreConfigManager::GetInstance().GetAudioLatencyFromXml();
}

void GetAdapterInfoByTypeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<PolicyAdapterInfo> info = nullptr;
    AudioCoreConfigManager::GetInstance().GetAdapterInfoByType(
        ConsumeEnum<AudioAdapterType>(fdp), info);
}

void GetStreamPropInfoSizeFuzzTest(FuzzedDataProvider& fdp)
{
    std::string adapterName = ConsumeString(fdp, "primary");
    std::string pipeName = ConsumeString(fdp, "");
    std::string networkId = ConsumeString(fdp, LOCAL_NETWORK_ID);
    AudioCoreConfigManager::GetInstance().GetStreamPropInfoSize(adapterName, pipeName, networkId);
}

void GetTargetSourceTypeAndMatchingFlagFuzzTest(FuzzedDataProvider& fdp)
{
    bool useMatchingPropInfo = fdp.ConsumeBool();
    AudioCoreConfigManager::GetInstance().GetTargetSourceTypeAndMatchingFlag(
        ConsumeEnum<SourceType>(fdp), useMatchingPropInfo);
}

void ParseFormatFuzzTest(FuzzedDataProvider& fdp)
{
    std::string format = "";
    if (fdp.ConsumeBool()) {
        format = ConsumeString(fdp, "s16le");
    }
    AudioCoreConfigManager::GetInstance().ParseFormat(format);
}

void CheckDynamicCapturerConfigFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = MakeDeviceDescriptor(fdp, INPUT_DEVICE);
    desc->newDeviceDescs_.push_back(deviceDesc);
    std::shared_ptr<PipeStreamPropInfo> info = std::make_shared<PipeStreamPropInfo>();
    AudioCoreConfigManager::GetInstance().CheckDynamicCapturerConfig(desc, info);
}

void GetStreamPropInfoForRecordFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    desc->SetAudioFlag(AUDIO_OUTPUT_FLAG_NORMAL);
    desc->streamInfo_.format = ConsumeEnum<AudioSampleFormat>(fdp);
    desc->streamInfo_.samplingRate = ConsumeEnum<AudioSamplingRate>(fdp);
    desc->capturerInfo_.sourceType = ConsumeEnum<SourceType>(fdp);
    std::shared_ptr<AdapterPipeInfo> adapterPipeInfo = std::make_shared<AdapterPipeInfo>();
    CHECK_AND_RETURN(adapterPipeInfo != nullptr);
    std::shared_ptr<PipeStreamPropInfo> info = nullptr;
    adapterPipeInfo->dynamicStreamPropInfosMap_[ConsumeString(fdp, "fuzz_network_id")].push_back(info);
    adapterPipeInfo->streamPropInfos_.push_back(info);
    AudioStreamInfo tempStreamInfo = {};
    tempStreamInfo.samplingRate = ConsumeEnum<AudioSamplingRate>(fdp);
    tempStreamInfo.channels = ConsumeEnum<AudioChannel>(fdp);
    AudioCoreConfigManager::GetInstance().OnUpdateRouteSupport(fdp.ConsumeBool());
    AudioCoreConfigManager::GetInstance().GetStreamPropInfoForRecord(desc, adapterPipeInfo, info, tempStreamInfo);
}

void GetNormalRecordAdapterInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> device = MakeDeviceDescriptor(fdp, INPUT_DEVICE);
    desc->AddNewDevice(device);
    std::shared_ptr<AdapterPipeInfo> info = AudioCoreConfigManager::GetInstance().GetNormalRecordAdapterInfo(desc);
}

void UpdateBasicStreamInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> desc = nullptr;
    std::shared_ptr<AdapterPipeInfo> pipeInfo = nullptr;
    AudioStreamInfo streamInfo;
    streamInfo.format = ConsumeEnum<AudioSampleFormat>(fdp);
    AudioCoreConfigManager::GetInstance().UpdateBasicStreamInfo(desc, pipeInfo, streamInfo);
    desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    pipeInfo = std::make_shared<AdapterPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);
    std::vector<uint32_t> routeFlag = {
        (AUDIO_INPUT_FLAG_VOIP | AUDIO_INPUT_FLAG_FAST),
        (AUDIO_OUTPUT_FLAG_VOIP | AUDIO_OUTPUT_FLAG_FAST),
        AUDIO_INPUT_FLAG_FAST,
        AUDIO_OUTPUT_FLAG_FAST,
    };
    desc->SetRoute(PickValue(fdp, routeFlag));
    if (fdp.ConsumeBool()) {
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
        pipeInfo->streamPropInfos_.push_back(fdp.ConsumeBool() ? streamPropInfo : nullptr);
    }
    AudioCoreConfigManager::GetInstance().UpdateBasicStreamInfo(desc, pipeInfo, streamInfo);
}

void GetDynamicStreamPropInfoFromPipeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AdapterPipeInfo> info = std::make_shared<AdapterPipeInfo>();
    CHECK_AND_RETURN(info != nullptr);
    std::shared_ptr<PipeStreamPropInfo> pipeStreamPropInfo = std::make_shared<PipeStreamPropInfo>();
    CHECK_AND_RETURN(pipeStreamPropInfo != nullptr);
    pipeStreamPropInfo->sampleRate_ = fdp.ConsumeIntegral<uint32_t>();
    std::list<std::shared_ptr<PipeStreamPropInfo>> streamProps = {pipeStreamPropInfo};
    std::string networkId = ConsumeString(fdp, "fuzz_network_id");
    info->UpdateDynamicStreamProps(streamProps, networkId);
    AudioSampleFormat format = ConsumeEnum<AudioSampleFormat>(fdp);
    uint32_t sampleRate = fdp.ConsumeIntegral<uint32_t>();
    AudioChannel channels = ConsumeEnum<AudioChannel>(fdp);
    AudioCoreConfigManager::GetInstance().SupportImplicitConversion(ConsumeEnum<AudioFlag>(fdp));
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(sampleRate), AudioEncodingType::ENCODING_PCM, format,
        channels);
    std::shared_ptr<PipeStreamPropInfo> ret =
        AudioCoreConfigManager::GetInstance().GetDynamicStreamPropInfoFromPipe(info, streamInfo, networkId);
}

void IsStreamPropMatchFuzzTest(FuzzedDataProvider& fdp)
{
    AudioStreamInfo streamInfo;
    streamInfo.format = ConsumeEnum<AudioSampleFormat>(fdp);
    streamInfo.samplingRate = ConsumeEnum<AudioSamplingRate>(fdp);
    streamInfo.channels = ConsumeEnum<AudioChannel>(fdp);
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos;
    if (fdp.ConsumeBool()) {
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
        streamPropInfo->format_ = streamInfo.format;
        streamPropInfo->sampleRate_ = streamInfo.samplingRate;
        streamPropInfo->channels_ = streamInfo.channels;
        infos.push_back(streamPropInfo);
    }
    AudioCoreConfigManager::GetInstance().IsStreamPropMatch(streamInfo, infos);
}

void GetFormatOnUpdateAnahsSupportFuzzTest(FuzzedDataProvider& fdp)
{
    std::string anahsShowType = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    AudioCoreConfigManager::GetInstance().GetFastFormat();
    AudioCoreConfigManager::GetInstance().OnUpdateAnahsSupport(anahsShowType);
}

void IsSupportInnerCaptureOffloadFuzzTest(FuzzedDataProvider& fdp)
{
    PolicyGlobalConfigs globalConfigs;
    PolicyConfigInfo policyConfigInfo;
    if (fdp.ConsumeBool()) {
        policyConfigInfo.name_ = ConsumeString(fdp, "offloadInnerCaptureSupport");
        policyConfigInfo.value_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);
    }
    globalConfigs.commonConfigs_.push_back(policyConfigInfo);
    AudioCoreConfigManager::GetInstance().OnGlobalConfigsParsed(globalConfigs);
    AudioCoreConfigManager::GetInstance().IsSupportInnerCaptureOffload();
}

void PreferMultiChannelPipeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    desc->newDeviceDescs_.push_back(deviceDesc);
    desc->audioFlag_ = ConsumeEnum<AudioFlag>(fdp);
    AudioCoreConfigManager::GetInstance().PreferMultiChannelPipe(desc);
}

void UpdateStreamSampleInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    desc->routeFlag_ = fdp.ConsumeIntegral<uint32_t>();
    desc->streamInfo_.samplingRate = ConsumeEnum<AudioSamplingRate>(fdp);
    AudioStreamInfo streamInfo = desc->streamInfo_;
    AudioCoreConfigManager::GetInstance().UpdateStreamSampleInfo(desc, streamInfo);
}

void MatchStreamPropInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<PipeStreamPropInfo> info = nullptr;
    std::shared_ptr<AdapterPipeInfo> adapterPipeInfo = std::make_shared<AdapterPipeInfo>();
    CHECK_AND_RETURN(adapterPipeInfo != nullptr);
    std::string networkId = ConsumeString(fdp, "fuzz_network_id");
    if (fdp.ConsumeBool()) {
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
        adapterPipeInfo->streamPropInfos_.push_back(streamPropInfo);
    }
    if (fdp.ConsumeBool()) {
        std::shared_ptr<PipeStreamPropInfo> dynamicStreamPropInfo = std::make_shared<PipeStreamPropInfo>();
        std::list<std::shared_ptr<PipeStreamPropInfo>> dynamicProps = {dynamicStreamPropInfo};
        adapterPipeInfo->UpdateDynamicStreamProps(dynamicProps, networkId);
    }
    AudioStreamInfo streamInfo(ConsumeEnum<AudioSamplingRate>(fdp), ConsumeEnum<AudioEncodingType>(fdp),
        ConsumeEnum<AudioSampleFormat>(fdp), ConsumeEnum<AudioChannel>(fdp),
        ConsumeEnum<AudioChannelLayout>(fdp));
    AudioCoreConfigManager::GetInstance().MatchStreamPropInfo(info, adapterPipeInfo, streamInfo, networkId);
}

void GetStreamPropInfoForMultiChannelFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    desc->streamInfo_.encoding = ConsumeEnum<AudioEncodingType>(fdp);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    desc->newDeviceDescs_.push_back(deviceDesc);
    desc->audioFlag_ = AUDIO_OUTPUT_FLAG_MULTICHANNEL;
    std::shared_ptr<AdapterPipeInfo> info = std::make_shared<AdapterPipeInfo>();
    CHECK_AND_RETURN(info != nullptr);
    if (fdp.ConsumeBool()) {
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
        streamPropInfo->channelLayout_ = ConsumeEnum<AudioChannelLayout>(fdp) + 1;
        streamPropInfo->channels_ = ConvertLayoutToAudioChannel(streamPropInfo->channelLayout_);
        info->streamPropInfos_.push_back(streamPropInfo);
    }
    AudioChannelLayout channelLayout = ConsumeEnum<AudioChannelLayout>(fdp);
    AudioCoreConfigManager::GetInstance().GetStreamPropInfoForMultiChannel(desc, info, channelLayout);
}

void GetDynamicStreamPropInfoFromPipeForViVidFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AdapterPipeInfo> info = std::make_shared<AdapterPipeInfo>();
    CHECK_AND_RETURN(info != nullptr);
    std::shared_ptr<PipeStreamPropInfo> pipeStreamPropInfo = std::make_shared<PipeStreamPropInfo>();
    CHECK_AND_RETURN(pipeStreamPropInfo != nullptr);
    pipeStreamPropInfo->channelLayout_ = ConsumeEnum<AudioChannelLayout>(fdp);
    std::list<std::shared_ptr<PipeStreamPropInfo>> streamProps = {pipeStreamPropInfo};
    std::string networkId = ConsumeString(fdp, "fuzz_network_id");
    info->UpdateDynamicStreamProps(streamProps, networkId);
    AudioSampleFormat format = ConsumeEnum<AudioSampleFormat>(fdp);
    uint32_t sampleRate = fdp.ConsumeIntegral<uint32_t>();
    AudioChannel channels = ConsumeEnum<AudioChannel>(fdp);
    AudioChannelLayout channelLayout = ConsumeEnum<AudioChannelLayout>(fdp);
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(sampleRate), AudioEncodingType::ENCODING_AUDIOVIVID,
        format, channels, channelLayout);
    AudioCoreConfigManager::GetInstance().GetDynamicStreamPropInfoFromPipeForViVid(info, streamInfo, networkId);
}

void NeedUidCheckForOffloadPipeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    CHECK_AND_RETURN(deviceDesc != nullptr);
    desc->newDeviceDescs_.push_back(deviceDesc);

    AudioPolicyConfigData &configData = AudioPolicyConfigData::GetInstance();
    configData.Reorganize();
    std::shared_ptr<AdapterPipeInfo> pipeInfo = std::make_shared<AdapterPipeInfo>();
    CHECK_AND_RETURN(pipeInfo != nullptr);
    pipeInfo->needUidCheck_ = fdp.ConsumeBool();

    std::shared_ptr<AdapterDeviceInfo> deviceInfo = std::make_shared<AdapterDeviceInfo>();
    CHECK_AND_RETURN(deviceInfo != nullptr);
    AudioFlag routeFlag = static_cast<AudioFlag>(AUDIO_OUTPUT_FLAG_LOWPOWER |
        (fdp.ConsumeBool() ? AUDIO_OUTPUT_FLAG_DIRECT : AUDIO_FLAG_NONE));
    deviceInfo->supportPipeMap_.insert({routeFlag, pipeInfo});

    std::set<std::shared_ptr<AdapterDeviceInfo>> deviceInfoSet = {deviceInfo};
    auto deviceKey = std::make_pair<DeviceType, DeviceRole>(DEVICE_TYPE_SPEAKER, OUTPUT_DEVICE);
    configData.deviceInfoMap[deviceKey] = deviceInfoSet;
    AudioCoreConfigManager::GetInstance().NeedUidCheckForOffloadPipe(desc);
    configData.deviceInfoMap.erase(deviceKey);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    UpdateAndClearStreamPropInfoFuzzTest,
    UpdateDynamicCapturerConfigFuzzTest,
    GetMaxCapturersInstancesFuzzTest,
    GetMaxFastRenderersInstancesFuzzTest,
    GetVoipRendererFlagFuzzTest,
    SetAndGetAudioLatencyFromXmlFuzzTest,
    GetAdapterInfoByTypeFuzzTest,
    GetStreamPropInfoSizeFuzzTest,
    GetTargetSourceTypeAndMatchingFlagFuzzTest,
    ParseFormatFuzzTest,
    CheckDynamicCapturerConfigFuzzTest,
    GetStreamPropInfoForRecordFuzzTest,
    GetNormalRecordAdapterInfoFuzzTest,
    UpdateBasicStreamInfoFuzzTest,
    GetDynamicStreamPropInfoFromPipeFuzzTest,
    IsStreamPropMatchFuzzTest,
    GetFormatOnUpdateAnahsSupportFuzzTest,
    IsSupportInnerCaptureOffloadFuzzTest,
    PreferMultiChannelPipeFuzzTest,
    UpdateStreamSampleInfoFuzzTest,
    MatchStreamPropInfoFuzzTest,
    GetStreamPropInfoForMultiChannelFuzzTest,
    GetDynamicStreamPropInfoFromPipeForViVidFuzzTest,
    NeedUidCheckForOffloadPipeFuzzTest,
    });
    func(fdp);
}
void Init()
{
}
} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
