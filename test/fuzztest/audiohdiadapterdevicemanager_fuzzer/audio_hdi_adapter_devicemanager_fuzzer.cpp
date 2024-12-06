/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_hdiadapter_info.h"
#include "audio_device_info.h"
#include "i_audio_device_manager.h"
#include "audio_device_manager_impl.h"
#include "audio_device_adapter_impl.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;
const uint32_t ENUMSIZE = 4;
const string DEFAULTNAME = "DEFAULT";
static const uint8_t *RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;

/*
* describe: get data from outside untrusted data(RAW_DATA) which size is according to sizeof(T)
* tips: only support basic type
*/
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
uint32_t GetArrLength(T& arr)
{
    if (arr == nullptr) {
        AUDIO_INFO_LOG("%{public}s: The array length is equal to 0", __func__);
        return 0;
    }
    return sizeof(arr) / sizeof(arr[0]);
}

void AudioDeviceManagerFactoryFuzzTest()
{
    AudioDeviceManagerFactory &audioDeviceManagerFactory = AudioDeviceManagerFactory::GetInstance();
    uint32_t audioDeviceManagerTypeInt = GetData<uint32_t>();
    audioDeviceManagerTypeInt = audioDeviceManagerTypeInt % ENUMSIZE;
    AudioDeviceManagerType audioDeviceManagerType = static_cast<AudioDeviceManagerType>(audioDeviceManagerTypeInt);
    audioDeviceManagerFactory.CreatDeviceManager(audioDeviceManagerType);
    audioDeviceManagerFactory.DestoryDeviceManager(audioDeviceManagerType);
}

void AudioDeviceManagerImlFuzzTest()
{
    shared_ptr<AudioDeviceManagerImpl> audioDeviceManagerIml = nullptr;
    uint32_t audioDeviceManagerTypeInt = GetData<uint32_t>();
    audioDeviceManagerTypeInt = audioDeviceManagerTypeInt % ENUMSIZE;
    AudioDeviceManagerType audioDeviceManagerType = static_cast<AudioDeviceManagerType>(audioDeviceManagerTypeInt);
#ifdef FEATURE_DISTRIBUTE_AUDIO
    sptr<IAudioManager> audioMgr = IAudioManager::Get("daudio_primary_service", false);
#else
    sptr<IAudioManager> audioMgr = nullptr;
#endif // FEATURE_DISTRIBUTE_AUDIO
    audioDeviceManagerIml = make_shared<AudioDeviceManagerImpl>(audioDeviceManagerType, audioMgr);
    if (audioDeviceManagerIml == nullptr) {
        return;
    }
    audioDeviceManagerIml->GetAllAdapters();
    bool isMmap = GetData<bool>();
    const string adapterName = "";
    audioDeviceManagerIml->LoadAdapters(adapterName, isMmap);
    audioDeviceManagerIml->UnloadAdapter(adapterName);
    audioDeviceManagerIml->LoadAdapters(DEFAULTNAME, isMmap);
    audioDeviceManagerIml->UnloadAdapter(DEFAULTNAME);
    audioDeviceManagerIml->Release();
    struct AudioAdapterDescriptor *desc = audioDeviceManagerIml->GetTargetAdapterDesc(DEFAULTNAME, isMmap);
    if (desc == nullptr || audioMgr == nullptr) {
        return;
    }
    AudioAdapterDescriptor descriptor = {
        .adapterName = desc->adapterName,
    };
    sptr<IAudioAdapter> audioAdapter = nullptr;
    audioMgr->LoadAdapter(descriptor, audioAdapter);

    auto audioDevAdp = std::make_shared<AudioDeviceAdapterImpl>(std::string(desc->adapterName), audioAdapter);
    audioDevAdp->SetParamCallback(audioDevAdp);
    audioDevAdp = AudioDeviceAdapterImpl::GetParamCallback();
    audioDevAdp->Init();
    audioDevAdp->RegExtraParamObserver();
    AudioParamKey audioParamKey = GetData<AudioParamKey>();
    const string condition = "";
    const string value = "";
    audioDevAdp->SetAudioParameter(audioParamKey, condition, value);
    audioDevAdp->GetAudioParameter(audioParamKey, condition);
    audioDevAdp->Release();
}

typedef void (*TestFuncs[2])();

TestFuncs g_testFuncs = {
    AudioDeviceManagerFactoryFuzzTest,
    AudioDeviceManagerImlFuzzTest,
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
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    if (size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}
