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

#include <string>
#include <thread>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <streambuf>
#include <algorithm>
#include <unistd.h>
#include "audio_errors.h"
#include "test_case_common.h"
#include "hpae_manager_impl.h"
#include "i_hpae_soft_link.h"
#include "hpae_manager_stream_fuzzer.h"
#include "id_handler.h"
#include "hdi_adapter_manager.h"
namespace OHOS {
namespace AudioStandard {
using namespace OHOS::AudioStandard::HPAE;
static const uint8_t *RAW_DATA = nullptr;
static std::string g_rootPath = "/data/";
static size_t g_dataSize = 0;
static size_t g_pos;
static const size_t THRESHOLD = 10;
static const uint32_t DEFAULT_SLEEP_TIME = 200; // 200ms

typedef void (*TestFuncs)();

template<class T>
T GetData()
{
    T object {};
    size_t objectSize = sizeof(object);

    if (g_dataSize <= g_pos) {
        return object;
    }

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
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

static AudioModuleInfo GetSinkAudioModeInfo(std::string name = "Speaker_File")
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-sink.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = name;
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "7680";
    audioModuleInfo.format = "s32le";
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

static AudioModuleInfo GetSourceAudioModeInfo(std::string name = "mic")
{
    AudioModuleInfo audioModuleInfo;
    audioModuleInfo.lib = "libmodule-hdi-source.z.so";
    audioModuleInfo.channels = "2";
    audioModuleInfo.rate = "48000";
    audioModuleInfo.name = name;
    audioModuleInfo.adapterName = "file_io";
    audioModuleInfo.className = "file_io";
    audioModuleInfo.bufferSize = "3840";
    audioModuleInfo.format = "s16le";
    audioModuleInfo.fixedLatency = "1";
    audioModuleInfo.offloadEnable = "0";
    audioModuleInfo.networkId = "LocalDevice";
    audioModuleInfo.fileName = g_rootPath + "source_" + audioModuleInfo.adapterName + "_" + audioModuleInfo.rate + "_" +
                               audioModuleInfo.channels + "_" + audioModuleInfo.format + ".pcm";
    std::stringstream typeValue;
    typeValue << static_cast<int32_t>(DEVICE_TYPE_FILE_SOURCE);
    audioModuleInfo.deviceType = typeValue.str();
    return audioModuleInfo;
}

void SetUp()
{
    IdHandler::GetInstance();
    HdiAdapterManager::GetInstance();
    IHpaeManager::GetHpaeManager().Init();
    std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_SLEEP_TIME));
}

void CreateSoftLinkFuzzTest()
{
    SetUp();
    std::shared_ptr<HpaeAudioServiceCallbackFuzzTest> callback = std::make_shared<HpaeAudioServiceCallbackFuzzTest>();
    IHpaeManager::GetHpaeManager().RegisterSerivceCallback(callback);
    std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_SLEEP_TIME));

    IHpaeManager::GetHpaeManager().OpenAudioPort(GetSinkAudioModeInfo());
    std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_SLEEP_TIME));
    int32_t sinkPortId = callback->GetPortId();

    IHpaeManager::GetHpaeManager().OpenAudioPort(GetSourceAudioModeInfo());
    std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_SLEEP_TIME));
    int32_t sourcePortId = callback->GetPortId();

    std::shared_ptr<IHpaeSoftLink> softLink =
        IHpaeSoftLink::CreateSoftLink(sinkPortId, sourcePortId, SoftLinkMode::HEARING_AID);
    if (softLink) {
        softLink->Start();
        std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_SLEEP_TIME));
        softLink->Stop();
        softLink->Release();
    }
    softLink = nullptr;
    IHpaeManager::GetHpaeManager().CloseAudioPort(sinkPortId);
    std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_SLEEP_TIME));
    IHpaeManager::GetHpaeManager().CloseAudioPort(sourcePortId);
    std::this_thread::sleep_for(std::chrono::milliseconds(DEFAULT_SLEEP_TIME));
}

TestFuncs g_testFuncs[1] = {
    CreateSoftLinkFuzzTest,
};

bool FuzzTest(const uint8_t* rawData, size_t size)
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
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
