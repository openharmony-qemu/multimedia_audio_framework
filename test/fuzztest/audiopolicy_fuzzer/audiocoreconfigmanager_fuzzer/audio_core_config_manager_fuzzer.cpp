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
#include "../../fuzz_utils.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
typedef void (*TestFuncs)();

void UpdateAndClearStreamPropInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::string adapterName = "adapterName";
    std::string pipeName = "pipeName";
    std::list<DeviceStreamInfo> deviceStreamInfo;
    DeviceStreamInfo streamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, CH_LAYOUT_STEREO };
    deviceStreamInfo.push_back(streamInfo);
    std::list<std::string> supportDevices;
    supportDevices.push_back("supportDevices");
    AudioCoreConfigManager::GetInstance().UpdateStreamPropInfo(adapterName, pipeName,
        deviceStreamInfo, supportDevices);
    AudioCoreConfigManager::GetInstance().ClearStreamPropInfo(adapterName, pipeName);
}

void UpdateDynamicCapturerConfigFuzzTest(FuzzedDataProvider& fdp)
{
    AudioModuleInfo moduleInfo;
    AudioCoreConfigManager::GetInstance().GetAdapterInfoFlag();
    AudioCoreConfigManager::GetInstance().UpdateDynamicCapturerConfig(g_fuzzUtils.GetData<ClassType>(), moduleInfo);
}

void GetMaxCapturersInstancesFuzzTest(FuzzedDataProvider& fdp)
{
    PolicyGlobalConfigs globalConfigs;
    PolicyConfigInfo policyConfigInfo;
    if (g_fuzzUtils.GetData<bool>()) {
        policyConfigInfo.name_ = "maxCapturers";
        policyConfigInfo.value_ = "-0";
    }
    globalConfigs.commonConfigs_.push_back(policyConfigInfo);
    AudioCoreConfigManager::GetInstance().OnGlobalConfigsParsed(globalConfigs);
    AudioCoreConfigManager::GetInstance().GetMaxCapturersInstances();
}

void GetMaxFastRenderersInstancesFuzzTest(FuzzedDataProvider& fdp)
{
    PolicyGlobalConfigs globalConfigs;
    PolicyConfigInfo policyConfigInfo;
    if (g_fuzzUtils.GetData<bool>()) {
        policyConfigInfo.name_ = "maxFastRenderers";
        policyConfigInfo.value_ = "-0";
    }
    globalConfigs.commonConfigs_.push_back(policyConfigInfo);
    AudioCoreConfigManager::GetInstance().OnGlobalConfigsParsed(globalConfigs);
    AudioCoreConfigManager::GetInstance().GetMaxFastRenderersInstances();
}

void GetVoipRendererFlagFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCoreConfigManager::GetInstance().SetNormalVoipFlag(g_fuzzUtils.GetData<bool>());
    AudioCoreConfigManager::GetInstance().GetNormalVoipFlag();
    AudioCoreConfigManager::GetInstance().OnVoipConfigParsed(g_fuzzUtils.GetData<bool>());
    AudioCoreConfigManager::GetInstance().GetVoipConfig();
    AudioCoreConfigManager::GetInstance().GetVoipRendererFlag("Speaker", "LocalDevice",
        g_fuzzUtils.GetData<AudioSamplingRate>());
}

void SetAndGetAudioLatencyFromXmlFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCoreConfigManager::GetInstance().OnAudioLatencyParsed(g_fuzzUtils.GetData<uint64_t>());
    AudioCoreConfigManager::GetInstance().GetAudioLatencyFromXml();
}

void GetAdapterInfoByTypeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<PolicyAdapterInfo> info = nullptr;
    AudioCoreConfigManager::GetInstance().GetAdapterInfoByType(g_fuzzUtils.GetData<AudioAdapterType>(), info);
}

void GetStreamPropInfoSizeFuzzTest(FuzzedDataProvider& fdp)
{
    AudioCoreConfigManager::GetInstance().GetStreamPropInfoSize("primary", "");
}

void GetTargetSourceTypeAndMatchingFlagFuzzTest(FuzzedDataProvider& fdp)
{
    bool useMatchingPropInfo = false;
    AudioCoreConfigManager::GetInstance().GetTargetSourceTypeAndMatchingFlag(g_fuzzUtils.GetData<SourceType>(),
        useMatchingPropInfo);
}

void ParseFormatFuzzTest(FuzzedDataProvider& fdp)
{
    std::string format = "";
    if (g_fuzzUtils.GetData<bool>()) {
        format = "s16le";
    }
    AudioCoreConfigManager::GetInstance().ParseFormat(format);
}

void CheckDynamicCapturerConfigFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_USB_ARM_HEADSET);
    desc->newDeviceDescs_.push_back(deviceDesc);
    std::shared_ptr<PipeStreamPropInfo> info = std::make_shared<PipeStreamPropInfo>();
    AudioCoreConfigManager::GetInstance().CheckDynamicCapturerConfig(desc, info);
}

void GetStreamPropInfoForRecordFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    desc->SetAudioFlag(AUDIO_OUTPUT_FLAG_NORMAL);
    desc->streamInfo_.format = g_fuzzUtils.GetData<AudioSampleFormat>();
    desc->streamInfo_.samplingRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    desc->capturerInfo_.sourceType = g_fuzzUtils.GetData<SourceType>();
    std::shared_ptr<AdapterPipeInfo> adapterPipeInfo = std::make_shared<AdapterPipeInfo>();
    CHECK_AND_RETURN(adapterPipeInfo != nullptr);
    std::shared_ptr<PipeStreamPropInfo> info = nullptr;
    adapterPipeInfo->dynamicStreamPropInfos_.push_back(info);
    adapterPipeInfo->streamPropInfos_.push_back(info);
    AudioStreamInfo tempStreamInfo = {};
    tempStreamInfo.samplingRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    tempStreamInfo.channels = g_fuzzUtils.GetData<AudioChannel>();
    AudioCoreConfigManager::GetInstance().OnUpdateRouteSupport(g_fuzzUtils.GetData<bool>());
    AudioCoreConfigManager::GetInstance().GetStreamPropInfoForRecord(desc, adapterPipeInfo, info, tempStreamInfo);
}

void GetNormalRecordAdapterInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>();
    desc->AddNewDevice(device);
    std::shared_ptr<AdapterPipeInfo> info = AudioCoreConfigManager::GetInstance().GetNormalRecordAdapterInfo(desc);
}

void UpdateBasicStreamInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> desc = nullptr;
    std::shared_ptr<AdapterPipeInfo> pipeInfo = nullptr;
    AudioStreamInfo streamInfo;
    streamInfo.format = g_fuzzUtils.GetData<AudioSampleFormat>();
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
    desc->SetRoute(routeFlag[g_fuzzUtils.GetData<uint32_t>() % routeFlag.size()]);
    if (g_fuzzUtils.GetData<bool>()) {
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
        pipeInfo->streamPropInfos_.push_back(g_fuzzUtils.GetData<bool>() ? streamPropInfo : nullptr);
    }
    AudioCoreConfigManager::GetInstance().UpdateBasicStreamInfo(desc, pipeInfo, streamInfo);
}

void GetDynamicStreamPropInfoFromPipeFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AdapterPipeInfo> info = std::make_shared<AdapterPipeInfo>();
    CHECK_AND_RETURN(info != nullptr);
    std::shared_ptr<PipeStreamPropInfo> pipeStreamPropInfo = std::make_shared<PipeStreamPropInfo>();
    CHECK_AND_RETURN(pipeStreamPropInfo != nullptr);
    pipeStreamPropInfo->sampleRate_ = g_fuzzUtils.GetData<uint32_t>();
    std::list<std::shared_ptr<PipeStreamPropInfo>> streamProps = {pipeStreamPropInfo};
    info->UpdateDynamicStreamProps(streamProps);
    AudioSampleFormat format = g_fuzzUtils.GetData<AudioSampleFormat>();
    uint32_t sampleRate = g_fuzzUtils.GetData<uint32_t>();
    AudioChannel channels = g_fuzzUtils.GetData<AudioChannel>();
    AudioCoreConfigManager::GetInstance().SupportImplicitConversion(g_fuzzUtils.GetData<AudioFlag>());
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(sampleRate), AudioEncodingType::ENCODING_PCM, format,
        channels);
    std::shared_ptr<PipeStreamPropInfo> ret =
        AudioCoreConfigManager::GetInstance().GetDynamicStreamPropInfoFromPipe(info, streamInfo);
}

void IsStreamPropMatchFuzzTest(FuzzedDataProvider& fdp)
{
    AudioStreamInfo streamInfo;
    streamInfo.format = AudioSampleFormat::SAMPLE_F32LE;
    streamInfo.samplingRate = AudioSamplingRate::SAMPLE_RATE_11025;
    streamInfo.channels = AudioChannel::STEREO;
    std::list<std::shared_ptr<PipeStreamPropInfo>> infos;
    if (g_fuzzUtils.GetData<bool>()) {
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
        streamPropInfo->format_ = AudioSampleFormat::SAMPLE_F32LE;
        streamPropInfo->sampleRate_ = AudioSamplingRate::SAMPLE_RATE_11025;
        streamPropInfo->channels_ = AudioChannel::STEREO;
        infos.push_back(streamPropInfo);
    }
    AudioCoreConfigManager::GetInstance().IsStreamPropMatch(streamInfo, infos);
}

void GetFormatOnUpdateAnahsSupportFuzzTest(FuzzedDataProvider& fdp)
{
    std::string anahsShowType = g_fuzzUtils.GetData<std::string>();
    AudioCoreConfigManager::GetInstance().GetFastFormat();
    AudioCoreConfigManager::GetInstance().OnUpdateAnahsSupport(anahsShowType);
}

void IsSupportInnerCaptureOffloadFuzzTest(FuzzedDataProvider& fdp)
{
    PolicyGlobalConfigs globalConfigs;
    PolicyConfigInfo policyConfigInfo;
    if (g_fuzzUtils.GetData<bool>()) {
        policyConfigInfo.name_ = "offloadInnerCaptureSupport";
        policyConfigInfo.value_ = "-0";
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
    desc->audioFlag_ = static_cast<AudioFlag>(g_fuzzUtils.GetData<uint32_t>() % AUDIO_FLAG_MAX);
    AudioCoreConfigManager::GetInstance().PreferMultiChannelPipe(desc);
}

void UpdateStreamSampleInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    desc->routeFlag_ = g_fuzzUtils.GetData<uint32_t>();
    desc->streamInfo_.samplingRate = g_fuzzUtils.GetData<AudioSamplingRate>();
    AudioStreamInfo streamInfo = desc->streamInfo_;
    AudioCoreConfigManager::GetInstance().UpdateStreamSampleInfo(desc, streamInfo);
}

void MatchStreamPropInfoFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<PipeStreamPropInfo> info = nullptr;
    std::shared_ptr<AdapterPipeInfo> adapterPipeInfo = std::make_shared<AdapterPipeInfo>();
    CHECK_AND_RETURN(adapterPipeInfo != nullptr);
    if (g_fuzzUtils.GetData<bool>()) {
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
        adapterPipeInfo->streamPropInfos_.push_back(streamPropInfo);
    }
    if (g_fuzzUtils.GetData<bool>()) {
        std::shared_ptr<PipeStreamPropInfo> dynamicStreamPropInfo = std::make_shared<PipeStreamPropInfo>();
        std::list<std::shared_ptr<PipeStreamPropInfo>> dynamicProps = {dynamicStreamPropInfo};
        adapterPipeInfo->UpdateDynamicStreamProps(dynamicProps);
    }
    AudioStreamInfo streamInfo(g_fuzzUtils.GetData<AudioSamplingRate>(), g_fuzzUtils.GetData<AudioEncodingType>(),
        g_fuzzUtils.GetData<AudioSampleFormat>(), g_fuzzUtils.GetData<AudioChannel>(),
        g_fuzzUtils.GetData<AudioChannelLayout>());
    AudioCoreConfigManager::GetInstance().MatchStreamPropInfo(info, adapterPipeInfo, streamInfo);
}

void GetStreamPropInfoForMultiChannelFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    desc->streamInfo_.encoding = g_fuzzUtils.GetData<AudioEncodingType>();
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc = std::make_shared<AudioDeviceDescriptor>();
    desc->newDeviceDescs_.push_back(deviceDesc);
    desc->audioFlag_ = AUDIO_OUTPUT_FLAG_MULTICHANNEL;
    std::shared_ptr<AdapterPipeInfo> info = std::make_shared<AdapterPipeInfo>();
    CHECK_AND_RETURN(info != nullptr);
    if (g_fuzzUtils.GetData<bool>()) {
        std::shared_ptr<PipeStreamPropInfo> streamPropInfo = std::make_shared<PipeStreamPropInfo>();
        streamPropInfo->channelLayout_ = g_fuzzUtils.GetData<AudioChannelLayout>();
        info->streamPropInfos_.push_back(streamPropInfo);
    }
    AudioChannelLayout channelLayout = g_fuzzUtils.GetData<AudioChannelLayout>();
    AudioCoreConfigManager::GetInstance().GetStreamPropInfoForMultiChannel(desc, info, channelLayout);
}

void GetDynamicStreamPropInfoFromPipeForViVidFuzzTest(FuzzedDataProvider& fdp)
{
    std::shared_ptr<AdapterPipeInfo> info = std::make_shared<AdapterPipeInfo>();
    CHECK_AND_RETURN(info != nullptr);
    std::shared_ptr<PipeStreamPropInfo> pipeStreamPropInfo = std::make_shared<PipeStreamPropInfo>();
    CHECK_AND_RETURN(pipeStreamPropInfo != nullptr);
    pipeStreamPropInfo->channelLayout_ = g_fuzzUtils.GetData<AudioChannelLayout>();
    std::list<std::shared_ptr<PipeStreamPropInfo>> streamProps = {pipeStreamPropInfo};
    info->UpdateDynamicStreamProps(streamProps);
    AudioSampleFormat format = g_fuzzUtils.GetData<AudioSampleFormat>();
    uint32_t sampleRate = g_fuzzUtils.GetData<uint32_t>();
    AudioChannel channels = g_fuzzUtils.GetData<AudioChannel>();
    AudioChannelLayout channelLayout = g_fuzzUtils.GetData<AudioChannelLayout>();
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(sampleRate), AudioEncodingType::ENCODING_AUDIOVIVID,
        format, channels, channelLayout);
    AudioCoreConfigManager::GetInstance().GetDynamicStreamPropInfoFromPipeForViVid(info, streamInfo);
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
    pipeInfo->needUidCheck_ = g_fuzzUtils.GetData<bool>();

    std::shared_ptr<AdapterDeviceInfo> deviceInfo = std::make_shared<AdapterDeviceInfo>();
    CHECK_AND_RETURN(deviceInfo != nullptr);
    AudioFlag routeFlag = static_cast<AudioFlag>(AUDIO_OUTPUT_FLAG_LOWPOWER |
        (g_fuzzUtils.GetData<bool>() ? AUDIO_OUTPUT_FLAG_DIRECT : AUDIO_FLAG_NONE));
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
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
extern "C" int LLVMFuzzerInitialize(const uint8_t* data, size_t size)
{
    OHOS::AudioStandard::Init();
    return 0;
}
