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

#include <sstream>
#include <iostream>
#include <cstddef>
#include <cstdint>
#include "audio_info.h"
#include "id_handler.h"
#include "hdi_adapter_manager.h"
#include "pro_audio_service_adapter_impl.h"
#include "audio_device_info.h"
#include "i_hpae_manager.h"
#include "audio_effect.h"
#include "audio_log.h"

using namespace std;
using namespace OHOS::AudioStandard::HPAE;
namespace OHOS {
namespace AudioStandard {
std::shared_ptr<ProAudioServiceAdapterImpl> impl_ = nullptr;
static std::string g_rootPath = "/data/";
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
typedef void (*TestPtr)(const uint8_t *, size_t);
std::mutex lock_;

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);
    if (RAW_DATA == nullptr || objectSize > g_dataSize - g_pos) {
        return object;
    }
    errno_t ret = memcpy_s(&object, objectSize, RAW_DATA + g_pos, objectSize);
    if (ret != EOK) {
        return {};
    }
    g_pos += objectSize;
    return object;
}

template<class T>
uint32_t GetArrLength(T &arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

class AudioServiceAdapterCallbackTest : public AudioServiceAdapterCallback {
public:
    void OnAudioStreamRemoved(const uint64_t sessionID) override
    {
        return;
    }
    void OnSetVolumeDbCb() override
    {
        return;
    }
};

static AudioModuleInfo InitSinkAudioModeInfo()
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-sink.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = "Speaker_File";
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "7680";
    audioModuleInfo.format = "s32le";
    audioModuleInfo.fixedLatency = "1";
    audioModuleInfo.offloadEnable = "0";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.fileName = g_rootPath + audioModuleInfo.adapterName + "_" + audioModuleInfo.rate + "_" +\
        audioModuleInfo.channels + "_" + audioModuleInfo.format + ".pcm";
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

static AudioModuleInfo InitSourceAudioModeInfo()
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-source.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = "mic";
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "3840";
    audioModuleInfo.format = "s16le";
    audioModuleInfo.fixedLatency = "1";
    audioModuleInfo.offloadEnable = "0";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.fileName = g_rootPath + audioModuleInfo.adapterName + "_" + audioModuleInfo.rate + "_" +
        audioModuleInfo.channels + "_" + audioModuleInfo.format + ".pcm";
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_SPEAKER);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

void SetUp()
{
    lock_guard<mutex> lock(lock_);
    if (impl_ != nullptr) {
        return;
    }
    IdHandler::GetInstance();
    HdiAdapterManager::GetInstance();
    std::unique_ptr<AudioServiceAdapterCallbackTest> cb = std::make_unique<AudioServiceAdapterCallbackTest>();
    impl_ = std::static_pointer_cast<ProAudioServiceAdapterImpl>(
        OHOS::AudioStandard::AudioServiceAdapter::CreateAudioAdapter(std::move(cb), true));
    impl_->Connect();
    HPAE::IHpaeManager::GetHpaeManager().Init();
}

void OpenAudioPortFuzzTest()
{
    SetUp();
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
}

void CloseAudioPortFuzzTest()
{
    SetUp();
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    impl_->CloseAudioPort(portId);
}

void SetDefaultSinkFuzzTest()
{
    SetUp();
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    impl_->SetDefaultSink(moduleInfo.name);
}

void SetDefaultSourceFuzzTest()
{
    SetUp();
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    impl_->SetDefaultSource(moduleInfo.name);
}

void SuspendAudioDeviceFuzzTest()
{
    SetUp();
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    impl_->SuspendAudioDevice(moduleInfo.name, true);
}

void SetSinkMuteFuzzTest()
{
    SetUp();
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    impl_->SetSinkMute(moduleInfo.name, true);
}

void GetAllSinkInputsFuzzTest()
{
    SetUp();
    impl_->GetAllSinkInputs();
}

void GetAllSourceOutputsFuzzTest()
{
    SetUp();
    impl_->GetAllSourceOutputs();
}

void DisconnectFuzzTest()
{
    SetUp();
    impl_->Disconnect();
}

void GetTargetSinksFuzzTest()
{
    SetUp();
    std::string adapterName = "adapterNameFuzzTest";
    impl_->GetTargetSinks(adapterName);
}

void GetAllSinksFuzzTest()
{
    SetUp();
    std::string adapterName = "adapterNameFuzzTest";
    impl_->GetAllSinks();
}

void SetLocalDefaultSinkFuzzTest()
{
    SetUp();
    std::string name = "SinkName";
    impl_->SetLocalDefaultSink(name);
}

void MoveSinkInputByIndexOrNameFuzzTest()
{
    SetUp();
    uint32_t sinkInputId = GetData<uint32_t>();
    uint32_t sinkIndex = GetData<uint32_t>();
    std::string sinkName = "SinkInputName";
    impl_->MoveSinkInputByIndexOrName(sinkInputId, sinkIndex, sinkName);
}

void MoveSourceOutputByIndexOrNameFuzzTest()
{
    SetUp();
    uint32_t sinkInputId = GetData<uint32_t>();
    uint32_t sinkIndex = GetData<uint32_t>();
    std::string sinkName = "SourceOutputName";
    impl_->MoveSourceOutputByIndexOrName(sinkInputId, sinkIndex, sinkName);
}

void GetAudioEffectPropertyV3FuzzTest()
{
    SetUp();
    AudioEffectPropertyArrayV3 propertyArray;
    impl_->GetAudioEffectProperty(propertyArray);
}

void GetAudioEffectPropertyFuzzTest()
{
    SetUp();
    AudioEffectPropertyArray propertyArray;
    impl_->GetAudioEffectProperty(propertyArray);
}

void GetAudioEnhancePropertyV3FuzzTest()
{
    SetUp();
    AudioEffectPropertyArrayV3 propertyArray;
    impl_->GetAudioEnhanceProperty(propertyArray);
}

void GetAudioEnhancePropertyFuzzTest()
{
    SetUp();
    AudioEnhancePropertyArray propertyArray;
    impl_->GetAudioEnhanceProperty(propertyArray);
}

void OnOpenAudioPortCbFuzzTest()
{
    SetUp();
    AudioModuleInfo moduleInfo = InitSinkAudioModeInfo();
    int32_t portId = impl_->OpenAudioPort(moduleInfo.lib, moduleInfo);
    impl_->OnOpenAudioPortCb(portId);
}

void OnCloseAudioPortCbFuzzTest()
{
    SetUp();
    int32_t result = GetData<int32_t>();
    impl_->OnCloseAudioPortCb(result);
}

void OnSetSinkMuteCbFuzzTest()
{
    SetUp();
    int32_t result = GetData<int32_t>();
    impl_->OnSetSinkMuteCb(result);
}

void OnSetSourceOutputMuteCbFuzzTest()
{
    SetUp();
    int32_t result = GetData<int32_t>();
    impl_->OnSetSourceOutputMuteCb(result);
}

void OnGetAllSinkInputsCbFuzzTest()
{
    SetUp();
    std::vector<SinkInput> sinkInputs = impl_->GetAllSinkInputs();
    int32_t result = GetData<int32_t>();
    impl_->OnGetAllSinkInputsCb(result, sinkInputs);
}

void OnGetAllSourceOutputsCbFuzzTest()
{
    SetUp();
    std::vector<SourceOutput> sourceOutputs = impl_->GetAllSourceOutputs();
    int32_t result = GetData<int32_t>();
    impl_->OnGetAllSourceOutputsCb(result, sourceOutputs);
}

void OnGetAllSinksCbFuzzTest()
{
    SetUp();
    std::vector<SinkInfo> sinks;
    int32_t result = GetData<int32_t>();
    impl_->OnGetAllSinksCb(result, sinks);
}

void OnMoveSinkInputByIndexOrNameCbFuzzTest()
{
    SetUp();
    int32_t result = GetData<int32_t>();
    impl_->OnMoveSinkInputByIndexOrNameCb(result);
}

void OnMoveSourceOutputByIndexOrNameCbFuzzTest()
{
    SetUp();
    int32_t result = GetData<int32_t>();
    impl_->OnMoveSourceOutputByIndexOrNameCb(result);
}

void OnGetAudioEffectPropertyCbV3FuzzTest()
{
    SetUp();
    int32_t result = GetData<int32_t>();
    impl_->OnGetAudioEffectPropertyCbV3(result);
}

void OnGetAudioEffectPropertyCbFuzzTest()
{
    SetUp();
    int32_t result = GetData<int32_t>();
    impl_->OnGetAudioEffectPropertyCb(result);
}

void OnGetAudioEnhancePropertyCbV3FuzzTest()
{
    SetUp();
    int32_t result = GetData<int32_t>();
    impl_->OnGetAudioEnhancePropertyCbV3(result);
}

void OnGetAudioEnhancePropertyCbFuzzTest()
{
    SetUp();
    int32_t result = GetData<int32_t>();
    impl_->OnGetAudioEnhancePropertyCb(result);
}

void HandleSourceAudioStreamRemovedFuzzTest()
{
    SetUp();
    uint32_t sessionId = GetData<uint32_t>();
    impl_->HandleSourceAudioStreamRemoved(sessionId);
}

typedef void (*TestFuncs[32])();

TestFuncs g_testFuncs = {
    OpenAudioPortFuzzTest,
    CloseAudioPortFuzzTest,
    SetDefaultSinkFuzzTest,
    SetDefaultSourceFuzzTest,
    SuspendAudioDeviceFuzzTest,
    SetSinkMuteFuzzTest,
    GetAllSinkInputsFuzzTest,
    GetAllSourceOutputsFuzzTest,
    DisconnectFuzzTest,
    GetTargetSinksFuzzTest,
    GetAllSinksFuzzTest,
    SetLocalDefaultSinkFuzzTest,
    MoveSinkInputByIndexOrNameFuzzTest,
    MoveSourceOutputByIndexOrNameFuzzTest,
    GetAudioEffectPropertyV3FuzzTest,
    GetAudioEffectPropertyFuzzTest,
    GetAudioEnhancePropertyV3FuzzTest,
    GetAudioEnhancePropertyFuzzTest,
    OnOpenAudioPortCbFuzzTest,
    OnCloseAudioPortCbFuzzTest,
    OnSetSinkMuteCbFuzzTest,
    OnSetSourceOutputMuteCbFuzzTest,
    OnGetAllSinkInputsCbFuzzTest,
    OnGetAllSourceOutputsCbFuzzTest,
    OnGetAllSinksCbFuzzTest,
    OnMoveSinkInputByIndexOrNameCbFuzzTest,
    OnMoveSourceOutputByIndexOrNameCbFuzzTest,
    OnGetAudioEffectPropertyCbV3FuzzTest,
    OnGetAudioEffectPropertyCbFuzzTest,
    OnGetAudioEnhancePropertyCbV3FuzzTest,
    OnGetAudioEnhancePropertyCbFuzzTest,
    HandleSourceAudioStreamRemovedFuzzTest,
};

bool FuzzTest(const uint8_t *rawData, size_t size)
{
    if (rawData == nullptr) {
        return false;
    }

    // initialize data
    RAW_DATA = rawData;
    g_dataSize = size;
    g_pos = 0;

    uint32_t code = GetData<uint32_t>();
    uint32_t len = GetArrLength(g_testFuncs);
    if (len > 0) {
        g_testFuncs[code % len]();
    } else {
        AUDIO_INFO_LOG("%{public}s: The len length is equal to 0", __func__);
    }

    return true;
}

} // namespace AudioStandard
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
