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
#include "token_setproc.h"
#include <fuzzer/FuzzedDataProvider.h>
#include "../fuzz_utils.h"

#include "audio_workgroup_callback_impl.h"
using namespace std;
static int32_t NUM_32 = 32;
namespace OHOS {
namespace AudioStandard {

void OnWorkgroupChange(FuzzedDataProvider &provider)
{
    AudioWorkgroupCallbackImpl audioWorkgroupCallbackImpl;
    AudioWorkgroupChangeInfoIpc info;
    audioWorkgroupCallbackImpl.workgroupCb_ = nullptr;
    audioWorkgroupCallbackImpl.OnWorkgroupChange(info);
}

void FuzzTest(FuzzedDataProvider &provider)
{
    auto func = provider.PickValueInArray({
        OnWorkgroupChange,
    });
    func(provider);
}
}
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    if (SetSelfTokenID(718336240uLL | (1uLL << NUM_32)) < 0) {
        return -1;
    }
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::FuzzTest(fdp);
    return 0;
}