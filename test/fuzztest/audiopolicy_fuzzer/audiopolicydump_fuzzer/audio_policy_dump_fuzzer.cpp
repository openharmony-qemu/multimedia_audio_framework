/*
 * Copyright (c) 2025-2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/LICENSE/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <memory>
#include <vector>
#include <string>
#include "audio_info.h"
#include "audio_policy_dump.h"
#include "audio_policy_log.h"
#include "audio_utils.h"
#include "audio_volume_config.h"
#include "audio_adapter_manager.h"
#include "boot_animation_state_info.h"
#include <fuzzer/FuzzedDataProvider.h>

namespace OHOS {
namespace AudioStandard {
using namespace std;

const size_t THRESHOLD = 10;
const size_t MAX_STRING_LEN = 64;
const int32_t MAX_TEST_UID = 10000;
const int32_t MAX_VALUE_DATA = 100;
const int32_t MAX_VALUE_DATA_15 = 15;
const uint32_t MAX_SESSION_ID = 100000;
const std::string DEFAULT_MAC_ADDRESS = "00:11:22:33:44:55";

const vector<DeviceType> g_outputDeviceTypes = {
    DEVICE_TYPE_EARPIECE,
    DEVICE_TYPE_SPEAKER,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_WIRED_HEADPHONES,
    DEVICE_TYPE_BLUETOOTH_A2DP,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_NEARLINK,
};

const vector<DeviceType> g_inputDeviceTypes = {
    DEVICE_TYPE_MIC,
    DEVICE_TYPE_WIRED_HEADSET,
    DEVICE_TYPE_BLUETOOTH_SCO,
    DEVICE_TYPE_USB_HEADSET,
    DEVICE_TYPE_BLUETOOTH_A2DP_IN,
    DEVICE_TYPE_NEARLINK_IN,
};

const vector<DeviceFlag> g_deviceFlags = {
    NONE_DEVICES_FLAG,
    OUTPUT_DEVICES_FLAG,
    INPUT_DEVICES_FLAG,
    ALL_DEVICES_FLAG,
    DISTRIBUTED_OUTPUT_DEVICES_FLAG,
    DISTRIBUTED_INPUT_DEVICES_FLAG,
    ALL_DISTRIBUTED_DEVICES_FLAG,
    ALL_L_D_DEVICES_FLAG,
};

const vector<AudioRingerMode> g_ringerModes = {
    RINGER_MODE_NORMAL,
    RINGER_MODE_SILENT,
    RINGER_MODE_VIBRATE,
};

const vector<AudioSamplingRate> g_samplingRates = {
    SAMPLE_RATE_8000,
    SAMPLE_RATE_16000,
    SAMPLE_RATE_44100,
    SAMPLE_RATE_48000,
};

const vector<AudioSampleFormat> g_sampleFormats = {
    SAMPLE_U8,
    SAMPLE_S16LE,
    SAMPLE_S24LE,
    SAMPLE_S32LE,
    SAMPLE_F32LE,
};

const vector<AudioChannelLayout> g_channelLayouts = {
    CH_LAYOUT_MONO,
    CH_LAYOUT_STEREO,
    CH_LAYOUT_QUAD,
    CH_LAYOUT_5POINT1,
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
    const auto& deviceTypes = (role == INPUT_DEVICE) ? g_inputDeviceTypes : g_outputDeviceTypes;
    auto desc = std::make_shared<AudioDeviceDescriptor>(PickValue(fdp, deviceTypes), role);
    desc->networkId_ = ConsumeString(fdp, LOCAL_NETWORK_ID);
    desc->macAddress_ = ConsumeString(fdp, DEFAULT_MAC_ADDRESS);
    desc->deviceName_ = ConsumeString(fdp, "audio_fuzz_device");
    desc->deviceId_ = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_TEST_UID);
    return desc;
}

DeviceStreamInfo MakeDeviceStreamInfo(FuzzedDataProvider& fdp)
{
    DeviceStreamInfo streamInfo;
    streamInfo.samplingRate = {PickValue(fdp, g_samplingRates)};
    streamInfo.format = PickValue(fdp, g_sampleFormats);
    streamInfo.channelLayout = {PickValue(fdp, g_channelLayouts)};
    return streamInfo;
}

std::shared_ptr<AudioRendererChangeInfo> MakeRendererChangeInfo(FuzzedDataProvider& fdp)
{
    auto info = std::make_shared<AudioRendererChangeInfo>();
    info->rendererState = ConsumeEnum<RendererState>(fdp);
    info->clientUID = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_TEST_UID);
    info->sessionId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    info->outputDeviceInfo.deviceType_ = PickValue(fdp, g_outputDeviceTypes);
    info->rendererInfo.contentType = ConsumeEnum<ContentType>(fdp);
    info->rendererInfo.streamUsage = ConsumeEnum<StreamUsage>(fdp);
    info->rendererInfo.samplingRate = PickValue(fdp, g_samplingRates);
    info->rendererInfo.pipeType = ConsumeEnum<AudioPipeType>(fdp);
    info->appVolume = fdp.ConsumeIntegral<int32_t>();
    return info;
}

std::shared_ptr<AudioCapturerChangeInfo> MakeCapturerChangeInfo(FuzzedDataProvider& fdp)
{
    auto info = std::make_shared<AudioCapturerChangeInfo>();
    info->clientUID = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_TEST_UID);
    info->sessionId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
    info->muted = fdp.ConsumeBool();
    info->inputDeviceInfo.deviceType_ = PickValue(fdp, g_inputDeviceTypes);
    info->capturerInfo.samplingRate = PickValue(fdp, g_samplingRates);
    info->capturerInfo.pipeType = ConsumeEnum<AudioPipeType>(fdp);
    info->capturerState = ConsumeEnum<CapturerState>(fdp);
    return info;
}

void AllDeviceVolumeInfoDumpEmptyFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    dump->audioVolumeManager_.audioConnectedDevice_.connectedDevices_.clear();
    std::string dumpString;
    dump->AllDeviceVolumeInfoDump(dumpString);
}

void GetDumpDeviceInfoDistributedFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    dump->audioVolumeManager_.audioConnectedDevice_.connectedDevices_.clear();
    auto desc = MakeDeviceDescriptor(fdp, fdp.ConsumeBool() ? INPUT_DEVICE : OUTPUT_DEVICE);
    desc->audioStreamInfo_.clear();
    desc->audioStreamInfo_.push_back(MakeDeviceStreamInfo(fdp));
    dump->audioVolumeManager_.audioConnectedDevice_.AddConnectedDevice(desc);

    std::string dumpString;
    DeviceFlag flag = fdp.ConsumeBool() ? DISTRIBUTED_INPUT_DEVICES_FLAG : DISTRIBUTED_OUTPUT_DEVICES_FLAG;
    dump->GetDumpDeviceInfo(dumpString, flag);
}

void GetDumpDevicesNoPermissionFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    dump->audioVolumeManager_.audioConnectedDevice_.connectedDevices_.clear();
    auto desc = MakeDeviceDescriptor(fdp, fdp.ConsumeBool() ? INPUT_DEVICE : OUTPUT_DEVICE);
    dump->audioVolumeManager_.audioConnectedDevice_.AddConnectedDevice(desc);

    vector<DeviceFlag> permissionFlags = {
        NONE_DEVICES_FLAG,
        DISTRIBUTED_OUTPUT_DEVICES_FLAG,
        DISTRIBUTED_INPUT_DEVICES_FLAG,
        ALL_DISTRIBUTED_DEVICES_FLAG,
        ALL_L_D_DEVICES_FLAG,
    };
    dump->GetDumpDevices(PickValue(fdp, permissionFlags));
}

void GetDumpDevicesClearNetworkIdFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    dump->audioVolumeManager_.audioConnectedDevice_.connectedDevices_.clear();
    auto desc = MakeDeviceDescriptor(fdp, fdp.ConsumeBool() ? INPUT_DEVICE : OUTPUT_DEVICE);
    desc->networkId_ = ConsumeString(fdp, LOCAL_NETWORK_ID);
    dump->audioVolumeManager_.audioConnectedDevice_.AddConnectedDevice(desc);

    vector<DeviceFlag> localFlags = {INPUT_DEVICES_FLAG, OUTPUT_DEVICES_FLAG};
    dump->GetDumpDevices(PickValue(fdp, localFlags));
}

void GetRingerModeDumpFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    AudioRingerMode mode = PickValue(fdp, g_ringerModes);
    if (fdp.ConsumeBool()) {
        mode = static_cast<AudioRingerMode>(fdp.ConsumeIntegralInRange<int32_t>(-1, MAX_VALUE_DATA));
    }
    dump->audioPolicyManager_.SetRingerMode(mode);
    std::string dumpString;
    dump->GetRingerModeDump(dumpString);
}

void GetRingerModeInfoDumpFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    AudioRingerMode ringMode = PickValue(fdp, g_ringerModes);
    std::string callerName = ConsumeString(fdp, "TestCaller");
    std::string invocationTime = ConsumeString(fdp, "2025-01-01");
    dump->audioPolicyManager_.SaveRingerModeInfo(ringMode, callerName, invocationTime);
    std::string dumpString;
    dump->GetRingerModeInfoDump(dumpString);
}

void StreamVolumesDumpFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    if (fdp.ConsumeBool()) {
        dump->audioVolumeManager_.volumeKeyRegistrations_->Clear();
    } else {
        dump->audioVolumeManager_.SaveVolumeKeyRegistrationInfo(
            fdp.ConsumeBool() ? "VOLUME_UP" : "VOLUME_DOWN",
            ConsumeString(fdp, "2025-01-01"),
            fdp.ConsumeIntegralInRange<int32_t>(0, MAX_TEST_UID),
            fdp.ConsumeBool());
    }
    std::string dumpString;
    dump->StreamVolumesDump(dumpString);
}

void GetVolumeConfigDumpFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    auto& mgrRef = AudioAdapterManager::GetInstance();
    auto *mgr = static_cast<AudioAdapterManager*>(&mgrRef);
    mgr->streamVolumeInfos_.clear();

    auto streamVolumeInfoPtr = std::make_shared<StreamVolumeInfo>();
    streamVolumeInfoPtr->streamType = fdp.ConsumeBool() ? AudioVolumeType::STREAM_ALL : AudioVolumeType::STREAM_MUSIC;
    streamVolumeInfoPtr->minLevel = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA_15);
    streamVolumeInfoPtr->maxLevel = fdp.ConsumeIntegralInRange<int32_t>(MAX_VALUE_DATA_15, MAX_VALUE_DATA);
    streamVolumeInfoPtr->defaultLevel = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA_15);
    mgr->streamVolumeInfos_.insert({streamVolumeInfoPtr->streamType, streamVolumeInfoPtr});
    std::string dumpString;
    dump->GetVolumeConfigDump(dumpString);
    mgr->streamVolumeInfos_.clear();
}

void GetAdjustVolumeDumpFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    dump->audioVolumeManager_.systemVolumeLevelInfo_->Clear();
    AudioStreamType streamType = ConsumeEnum<AudioStreamType>(fdp);
    int32_t volumeLevel = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_VALUE_DATA);
    int32_t appUid = fdp.ConsumeIntegralInRange<int32_t>(0, MAX_TEST_UID);
    std::string invocationTime = ConsumeString(fdp, "2025-01-01");
    dump->audioVolumeManager_.SaveSystemVolumeLevelInfo(streamType, volumeLevel, appUid, invocationTime);
    std::string dumpString;
    dump->GetAdjustVolumeDump(dumpString);
}

void AdjustVolumeAppendFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    vector<AdjustStreamVolumeInfo> adjustInfo;
    if (fdp.ConsumeBool()) {
        AdjustStreamVolumeInfo info;
        info.volume = fdp.ConsumeFloatingPoint<float>();
        info.sessionId = fdp.ConsumeIntegralInRange<uint32_t>(0, MAX_SESSION_ID);
        info.invocationTime = ConsumeString(fdp, "2025-01-01");
        adjustInfo.push_back(info);
    }
    std::string dumpString;
    dump->AdjustVolumeAppend(adjustInfo, dumpString);
}

void AudioStreamDumpFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    dump->streamCollector_.audioRendererChangeInfos_.clear();
    int32_t count = fdp.ConsumeIntegralInRange<int32_t>(1, 3);
    for (int32_t i = 0; i < count; i++) {
        auto info = MakeRendererChangeInfo(fdp);
        info->rendererInfo.rendererFlags = fdp.PickValueInArray({0, 1, 2});
        dump->streamCollector_.audioRendererChangeInfos_.push_back(std::move(info));
    }
    std::string dumpString;
    dump->AudioStreamDump(dumpString);
}

void GetCapturerStreamDumpFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    dump->streamCollector_.audioCapturerChangeInfos_.clear();
    int32_t count = fdp.ConsumeIntegralInRange<int32_t>(1, 3);
    for (int32_t i = 0; i < count; i++) {
        auto info = MakeCapturerChangeInfo(fdp);
        info->capturerInfo.capturerFlags = fdp.PickValueInArray({0, 1, 2});
        dump->streamCollector_.audioCapturerChangeInfos_.push_back(std::move(info));
    }
    std::string dumpString;
    dump->GetCapturerStreamDump(dumpString);
}

void GetRingerModeTypeFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    AudioRingerMode mode = PickValue(fdp, g_ringerModes);
    if (fdp.ConsumeBool()) {
        mode = static_cast<AudioRingerMode>(fdp.ConsumeIntegralInRange<int32_t>(-1, MAX_VALUE_DATA));
    }
    dump->GetRingerModeType(mode);
}

void RecordBootAnimationClientStateTimeFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    BootAnimationInfo info(static_cast<size_t>(BootAnimationState::MAX), 0);
    for (size_t i = 0; i < static_cast<size_t>(BootAnimationState::MAX); i++) {
        info[i] = fdp.ConsumeIntegral<int64_t>();
    }
    dump->RecordBootAnimationClientStateTime(info);
}

void RecordBootStateTimeBootAnimationFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    int32_t stateStart = static_cast<int32_t>(BootAnimationState::CREATE_RENDERER_BEGIN);
    int32_t stateEnd = static_cast<int32_t>(BootAnimationState::MAX) - 1;
    BootAnimationState state =
        static_cast<BootAnimationState>(fdp.ConsumeIntegralInRange<int32_t>(stateStart, stateEnd));
    int32_t uid = fdp.ConsumeBool() ? BOOT_ANIMATION_UID : fdp.ConsumeIntegralInRange<int32_t>(0, MAX_TEST_UID);
    int64_t curTime = fdp.ConsumeIntegral<int64_t>();
    dump->RecordBootStateTime(state, uid, curTime);
}

void AudioPolicyBootDumpFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    int32_t stateStart = static_cast<int32_t>(AudioPolicyServerBootState::BEGIN);
    int32_t stateEnd = static_cast<int32_t>(AudioPolicyServerBootState::MAX) - 1;
    int32_t recordCount = fdp.ConsumeIntegralInRange<int32_t>(1, 5);
    for (int32_t i = 0; i < recordCount; i++) {
        AudioPolicyServerBootState state =
            static_cast<AudioPolicyServerBootState>(fdp.ConsumeIntegralInRange<int32_t>(stateStart, stateEnd));
        int64_t curTime = fdp.ConsumeIntegral<int64_t>();
        dump->RecordBootStateTime(state, curTime);
    }
    BootAnimationState baState =
        static_cast<BootAnimationState>(fdp.ConsumeIntegralInRange<int32_t>(
            static_cast<int32_t>(BootAnimationState::CREATE_RENDERER_BEGIN),
            static_cast<int32_t>(BootAnimationState::MAX) - 1));
    dump->RecordBootStateTime(baState, BOOT_ANIMATION_UID, fdp.ConsumeIntegral<int64_t>());
    std::string dumpString;
    dump->AudioPolicyBootDump(dumpString);
}

void DevicesInfoDumpFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    dump->audioVolumeManager_.audioConnectedDevice_.connectedDevices_.clear();
    if (fdp.ConsumeBool()) {
        auto desc = MakeDeviceDescriptor(fdp, fdp.ConsumeBool() ? INPUT_DEVICE : OUTPUT_DEVICE);
        desc->audioStreamInfo_.clear();
        desc->audioStreamInfo_.push_back(MakeDeviceStreamInfo(fdp));
        dump->audioVolumeManager_.audioConnectedDevice_.AddConnectedDevice(desc);
    }
    std::string dumpString;
    dump->DevicesInfoDump(dumpString);
}

void AudioModeDumpFuzzTest(FuzzedDataProvider& fdp)
{
    auto dump = std::make_shared<AudioPolicyDump>();
    CHECK_AND_RETURN(dump != nullptr);
    AudioRingerMode mode = PickValue(fdp, g_ringerModes);
    dump->audioPolicyManager_.SetRingerMode(mode);
    AudioRingerMode ringMode = PickValue(fdp, g_ringerModes);
    std::string callerName = ConsumeString(fdp, "TestCaller");
    std::string invocationTime = ConsumeString(fdp, "2025-01-01");
    dump->audioPolicyManager_.SaveRingerModeInfo(ringMode, callerName, invocationTime);
    std::string dumpString;
    dump->AudioModeDump(dumpString);
}

void Test(FuzzedDataProvider& fdp)
{
    auto func = fdp.PickValueInArray({
    AllDeviceVolumeInfoDumpEmptyFuzzTest,
    GetDumpDeviceInfoDistributedFuzzTest,
    GetDumpDevicesNoPermissionFuzzTest,
    GetDumpDevicesClearNetworkIdFuzzTest,
    GetRingerModeDumpFuzzTest,
    GetRingerModeInfoDumpFuzzTest,
    StreamVolumesDumpFuzzTest,
    GetVolumeConfigDumpFuzzTest,
    GetAdjustVolumeDumpFuzzTest,
    AdjustVolumeAppendFuzzTest,
    AudioStreamDumpFuzzTest,
    GetCapturerStreamDumpFuzzTest,
    GetRingerModeTypeFuzzTest,
    RecordBootAnimationClientStateTimeFuzzTest,
    RecordBootStateTimeBootAnimationFuzzTest,
    AudioPolicyBootDumpFuzzTest,
    DevicesInfoDumpFuzzTest,
    AudioModeDumpFuzzTest,
    });
    func(fdp);
}
} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    (void)argc;
    (void)argv;
    return 0;
}
