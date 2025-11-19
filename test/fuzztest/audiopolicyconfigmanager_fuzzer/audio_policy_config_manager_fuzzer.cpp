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

#include "audio_policy_config_manager.h"
#include "audio_ec_manager.h"
#include "iaudio_policy_client.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
typedef void (*TestFuncs)();

void UpdateAndClearStreamPropInfoFuzztest()
{
    std::string adapterName = "adapterName";
    std::string pipeName = "pipeName";
    std::list<DeviceStreamInfo> deviceStreamInfo;
    DeviceStreamInfo streamInfo = { SAMPLE_RATE_48000, ENCODING_PCM, SAMPLE_S16LE, CH_LAYOUT_STEREO };
    deviceStreamInfo.push_back(streamInfo);
    std::list<std::string> supportDevices;
    supportDevices.push_back("supportDevices");
    AudioPolicyConfigManager::GetInstance().UpdateStreamPropInfo(adapterName, pipeName,
                                                                 deviceStreamInfo, supportDevices);
    AudioPolicyConfigManager::GetInstance().ClearStreamPropInfo(adapterName, pipeName);
}

void UpdateDynamicCapturerConfigFuzztest()
{
    AudioModuleInfo moduleInfo;
    AudioPolicyConfigManager::GetInstance().GetAdapterInfoFlag();
    AudioPolicyConfigManager::GetInstance().UpdateDynamicCapturerConfig(g_fuzzUtils.GetData<ClassType>(), moduleInfo);
}

void GetMaxCapturersInstancesFuzzTest()
{
    PolicyGlobalConfigs globalConfigs;
    PolicyConfigInfo policyConfigInfo;
    if (g_fuzzUtils.GetData<bool>()) {
        policyConfigInfo.name_ = "maxCapturers";
        policyConfigInfo.value_ = "-0";
    }
    globalConfigs.commonConfigs_.push_back(policyConfigInfo);
    AudioPolicyConfigManager::GetInstance().OnGlobalConfigsParsed(globalConfigs);
    AudioPolicyConfigManager::GetInstance().GetMaxCapturersInstances();
}

void GetMaxFastRenderersInstancesFuzzTest()
{
    PolicyGlobalConfigs globalConfigs;
    PolicyConfigInfo policyConfigInfo;
    if (g_fuzzUtils.GetData<bool>()) {
        policyConfigInfo.name_ = "maxFastRenderers";
        policyConfigInfo.value_ = "-0";
    }
    globalConfigs.commonConfigs_.push_back(policyConfigInfo);
    AudioPolicyConfigManager::GetInstance().OnGlobalConfigsParsed(globalConfigs);
    AudioPolicyConfigManager::GetInstance().GetMaxFastRenderersInstances();
}

void GetVoipRendererFlagFuzzTest()
{
    AudioPolicyConfigManager::GetInstance().SetNormalVoipFlag(g_fuzzUtils.GetData<bool>());
    AudioPolicyConfigManager::GetInstance().GetNormalVoipFlag();
    AudioPolicyConfigManager::GetInstance().OnVoipConfigParsed(g_fuzzUtils.GetData<bool>());
    AudioPolicyConfigManager::GetInstance().GetVoipConfig();
    AudioPolicyConfigManager::GetInstance().GetVoipRendererFlag("Speaker", "LocalDevice",
                                                                g_fuzzUtils.GetData<AudioSamplingRate>());
}

void SetAndGetAudioLatencyFromXmlFuzzTest()
{
    AudioPolicyConfigManager::GetInstance().OnAudioLatencyParsed(g_fuzzUtils.GetData<uint64_t>());
    AudioPolicyConfigManager::GetInstance().GetAudioLatencyFromXml();
}

void GetAdapterInfoByTypeFuzzTest()
{
    std::shared_ptr<PolicyAdapterInfo> info = nullptr;
    AudioPolicyConfigManager::GetInstance().GetAdapterInfoByType(g_fuzzUtils.GetData<AudioAdapterType>(), info);
}

void GetStreamPropInfoSizeFuzzTest()
{
    AudioPolicyConfigManager::GetInstance().GetStreamPropInfoSize("primary", "");
}

void GetTargetSourceTypeAndMatchingFlagFuzzTest()
{
    bool useMatchingPropInfo = false;
    AudioPolicyConfigManager::GetInstance().GetTargetSourceTypeAndMatchingFlag(g_fuzzUtils.GetData<SourceType>(),
                                                                               useMatchingPropInfo);
}

void ParseFormatFuzzTest()
{
    std::string format = "";
    if (g_fuzzUtils.GetData<bool>()) {
        format = "s16le";
    }
    AudioPolicyConfigManager::GetInstance().ParseFormat(format);
}

void CheckDynamicCapturerConfigFuzzTest()
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> deviceDesc =
        std::make_shared<AudioDeviceDescriptor>(DEVICE_TYPE_USB_ARM_HEADSET);
    desc->newDeviceDescs_.push_back(deviceDesc);
    std::shared_ptr<PipeStreamPropInfo> info = std::make_shared<PipeStreamPropInfo>();
    AudioPolicyConfigManager::GetInstance().CheckDynamicCapturerConfig(desc, info);
}

void GetStreamPropInfoForRecordFuzzTest()
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
    AudioEcManager::GetInstance().Init(g_fuzzUtils.GetData<int32_t>(), g_fuzzUtils.GetData<int32_t>());
    AudioPolicyConfigManager::GetInstance().OnUpdateRouteSupport(g_fuzzUtils.GetData<bool>());
    AudioPolicyConfigManager::GetInstance().GetStreamPropInfoForRecord(desc, adapterPipeInfo, info, tempStreamInfo);
}

void GetNormalRecordAdapterInfoFuzzTest()
{
    std::shared_ptr<AudioStreamDescriptor> desc = std::make_shared<AudioStreamDescriptor>();
    CHECK_AND_RETURN(desc != nullptr);
    std::shared_ptr<AudioDeviceDescriptor> device = std::make_shared<AudioDeviceDescriptor>();
    desc->AddNewDevice(device);
    std::shared_ptr<AdapterPipeInfo> info = AudioPolicyConfigManager::GetInstance().GetNormalRecordAdapterInfo(desc);
}

void UpdateBasicStreamInfoFuzzTest()
{
    std::shared_ptr<AudioStreamDescriptor> desc = nullptr;
    std::shared_ptr<AdapterPipeInfo> pipeInfo = nullptr;
    AudioStreamInfo streamInfo;
    streamInfo.format = g_fuzzUtils.GetData<AudioSampleFormat>();
    AudioPolicyConfigManager::GetInstance().UpdateBasicStreamInfo(desc, pipeInfo, streamInfo);
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
    AudioPolicyConfigManager::GetInstance().UpdateBasicStreamInfo(desc, pipeInfo, streamInfo);
}

void GetDynamicStreamPropInfoFromPipeFuzzTest()
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
    AudioPolicyConfigManager::GetInstance().SupportImplicitConversion(g_fuzzUtils.GetData<AudioFlag>());
    AudioStreamInfo streamInfo(static_cast<AudioSamplingRate>(sampleRate), AudioEncodingType::ENCODING_PCM, format,
        channels);
    std::shared_ptr<PipeStreamPropInfo> ret =
        AudioPolicyConfigManager::GetInstance().GetDynamicStreamPropInfoFromPipe(info, streamInfo);
}

void IsStreamPropMatchFuzzTest()
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
    AudioPolicyConfigManager::GetInstance().IsStreamPropMatch(streamInfo, infos);
}

vector<TestFuncs> g_testFuncs = {
    UpdateAndClearStreamPropInfoFuzztest,
    UpdateDynamicCapturerConfigFuzztest,
    GetMaxCapturersInstancesFuzzTest,
    GetMaxFastRenderersInstancesFuzzTest,
    GetVoipRendererFlagFuzzTest,
    SetAndGetAudioLatencyFromXmlFuzzTest,
    GetAdapterInfoByTypeFuzzTest,
    GetStreamPropInfoSizeFuzzTest,
    CheckDynamicCapturerConfigFuzzTest,
    GetStreamPropInfoForRecordFuzzTest,
    GetNormalRecordAdapterInfoFuzzTest,
    GetDynamicStreamPropInfoFromPipeFuzzTest,
    IsStreamPropMatchFuzzTest,
};

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
