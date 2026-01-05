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
#include "i_stream_listener.h"
#include "ipc_stream_listener_impl.h"
using namespace std;

static int32_t NUM_32 = 32;
namespace OHOS {
namespace AudioStandard {
constexpr int32_t SUCCESS = 0;
class IStreamListenerI : IStreamListener {
    ~IStreamListenerI() {};
    int32_t OnOperationHandled(Operation operation, int64_t result) override;
};
int32_t IStreamListenerI::OnOperationHandled(Operation operation, int64_t result)
{
    return SUCCESS;
}
std::shared_ptr<IpcStreamListenerImpl> ipcStreamListenerImpl = nullptr;

void IpcStreamListenerImplInit()
{
    std::shared_ptr<IStreamListenerI> iStreamListenerI = std::make_shared<IStreamListenerI>();
    ipcStreamListenerImpl = std::make_shared<IpcStreamListenerImpl>(iStreamListenerI);
}

void OnOperationHandled(FuzzedDataProvider &provider)
{
    int32_t operation = provider.ConsumeIntegral<int32_t>();
    int64_t result = provider.ConsumeIntegral<int64_t>();
    ipcStreamListenerImpl->OnOperationHandled(operation, result);
}

void OnOperationHandledLazy(FuzzedDataProvider &provider)
{
    int32_t operation = provider.ConsumeIntegral<int32_t>();
    int64_t result = provider.ConsumeIntegral<int64_t>();
    ipcStreamListenerImpl->OnOperationHandledLazy(operation, result);
}

void FuzzTest(FuzzedDataProvider &provider)
{
    if (ipcStreamListenerImpl == nullptr) {
        return;
    }
    auto func = provider.PickValueInArray({
        OnOperationHandled,
        OnOperationHandledLazy,
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
    OHOS::AudioStandard::IpcStreamListenerImplInit();
    return 0;
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::FuzzTest(fdp);
    return 0;
}