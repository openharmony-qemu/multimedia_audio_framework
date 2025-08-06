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

#include <securec.h>

#include "audio_log.h"
#include "microphone_descriptor.h"
#include "../fuzz_utils.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

FuzzUtils &g_fuzzUtils = FuzzUtils::GetInstance();
const size_t FUZZ_INPUT_SIZE_THRESHOLD = 10;

typedef void (*TestFuncs)();

void MicrophoneDescriptor1FuzzTest()
{
    int32_t id = g_fuzzUtils.GetData<int32_t>();
    DeviceType deviceType = g_fuzzUtils.GetData<DeviceType>();
    int32_t groupId = g_fuzzUtils.GetData<int32_t>();
    int32_t sensitivity = g_fuzzUtils.GetData<int32_t>();
    MicrophoneDescriptor microphoneDescriptor(id, deviceType, groupId, sensitivity);
}

void MicrophoneDescriptor2FuzzTest()
{
    sptr<MicrophoneDescriptor> micDesc = new (std::nothrow) MicrophoneDescriptor();
    if (micDesc == nullptr) {
        return;
    }
    micDesc->micId_ = g_fuzzUtils.GetData<int32_t>();
    MicrophoneDescriptor microphoneDescriptor(micDesc);
}

void OperatorFuzzTest()
{
    int32_t id = g_fuzzUtils.GetData<int32_t>();
    DeviceType deviceType = g_fuzzUtils.GetData<DeviceType>();

    int32_t groupId = g_fuzzUtils.GetData<int32_t>();
    int32_t sensitivity = g_fuzzUtils.GetData<int32_t>();
    MicrophoneDescriptor microphoneDescriptor(id, deviceType, groupId, sensitivity);
    MicrophoneDescriptor microphoneDescriptor2(microphoneDescriptor);
}

void MarshallingFuzzTest()
{
    int32_t id = g_fuzzUtils.GetData<int32_t>();
    DeviceType deviceType = g_fuzzUtils.GetData<DeviceType>();

    int32_t groupId = g_fuzzUtils.GetData<int32_t>();
    int32_t sensitivity = g_fuzzUtils.GetData<int32_t>();
    MicrophoneDescriptor microphoneDescriptor(id, deviceType, groupId, sensitivity);
    Parcel parcel;
    microphoneDescriptor.Marshalling(parcel);
}

void UnmarshallingFuzzTest()
{
    int32_t id = g_fuzzUtils.GetData<int32_t>();
    DeviceType deviceType = g_fuzzUtils.GetData<DeviceType>();

    int32_t groupId = g_fuzzUtils.GetData<int32_t>();
    int32_t sensitivity = g_fuzzUtils.GetData<int32_t>();
    MicrophoneDescriptor microphoneDescriptor(id, deviceType, groupId, sensitivity);
    Parcel parcel;
    microphoneDescriptor.Marshalling(parcel);
    if (MicrophoneDescriptor::Unmarshalling(parcel) == nullptr) {
        return;
    }
    auto micDescShared = std::shared_ptr<MicrophoneDescriptor>(MicrophoneDescriptor::Unmarshalling(parcel));
}

vector<TestFuncs> g_testFuncs = {
    MicrophoneDescriptor1FuzzTest,
    MicrophoneDescriptor2FuzzTest,
    OperatorFuzzTest,
    MarshallingFuzzTest,
    UnmarshallingFuzzTest,
};

} // namespace AudioStandard
} // namesapce OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (size < OHOS::AudioStandard::FUZZ_INPUT_SIZE_THRESHOLD) {
        return 0;
    }

    OHOS::AudioStandard::g_fuzzUtils.fuzzTest(data, size, OHOS::AudioStandard::g_testFuncs);
    return 0;
}
