/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include <cstdint>
#include <memory>

#include <fuzzer/FuzzedDataProvider.h>

#include "audio_errors.h"
#include "audio_info.h"
#include "audio_loopback_manager.h"
#include "loopback_callback_stub.h"
#include "loopback_in_server.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t BASE_PID = 1000;
const int32_t MAX_CALL_COUNT = 16;
const size_t THRESHOLD = sizeof(uint32_t) + sizeof(uint16_t) + sizeof(uint8_t);

enum class ManagerState {
    ALIVE,
    NULLPTR,
    EXPIRED,
    WITH_HANDLER,
};

enum class RandomOperation {
    ENABLE,
    GET_STATUS,
    GET_VOLUME,
    GET_MODE,
    GET_TYPE,
    GET_PID,
};

class FuzzLoopbackCallback : public LoopbackCallbackStub {
public:
    int32_t OnCommandResult(int32_t command, bool success, int32_t errorCode) override
    {
        lastCommand_ = command;
        lastSuccess_ = success;
        lastErrorCode_ = errorCode;
        return SUCCESS;
    }

    int32_t OnStatusNotify(int32_t status, int32_t reason) override
    {
        lastStatus_ = status;
        lastReason_ = reason;
        return SUCCESS;
    }

private:
    int32_t lastCommand_ = 0;
    bool lastSuccess_ = false;
    int32_t lastErrorCode_ = 0;
    int32_t lastStatus_ = 0;
    int32_t lastReason_ = 0;
};

AudioLoopbackMode GetMode(FuzzedDataProvider &fdp)
{
    return fdp.PickValueInArray<AudioLoopbackMode>({
        LOOPBACK_HARDWARE,
        static_cast<AudioLoopbackMode>(fdp.ConsumeIntegral<int32_t>()),
    });
}

LoopbackType GetType(FuzzedDataProvider &fdp)
{
    return fdp.PickValueInArray<LoopbackType>({
        LOOPBACK_TYPE_NORMAL,
        LOOPBACK_TYPE_GLOBAL_HANDLER,
        LOOPBACK_TYPE_GLOBAL_CONTROL,
        static_cast<LoopbackType>(fdp.ConsumeIntegral<int32_t>()),
    });
}

int32_t GetPid(FuzzedDataProvider &fdp)
{
    return BASE_PID + static_cast<int32_t>(fdp.ConsumeIntegral<uint16_t>());
}

sptr<FuzzLoopbackCallback> MakeCallback()
{
    return new FuzzLoopbackCallback();
}

struct ServerHandle {
    sptr<LoopbackInServer> server;
    std::shared_ptr<AudioLoopbackManager> manager;
};

ServerHandle MakeServer(FuzzedDataProvider &fdp, ManagerState state)
{
    ServerHandle handle;
    AudioLoopbackMode mode = GetMode(fdp);
    LoopbackType type = GetType(fdp);
    int32_t pid = GetPid(fdp);

    switch (state) {
        case ManagerState::NULLPTR:
            handle.server = new LoopbackInServer(mode, type, pid, nullptr);
            break;
        case ManagerState::EXPIRED:
            handle.manager = std::make_shared<AudioLoopbackManager>();
            handle.server = new LoopbackInServer(mode, type, pid, handle.manager);
            handle.manager.reset();
            break;
        case ManagerState::WITH_HANDLER: {
            handle.manager = std::make_shared<AudioLoopbackManager>();
            sptr<IRemoteObject> handlerLoopback;
            handle.manager->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
                MakeCallback(), handlerLoopback, GetPid(fdp));
            handle.server = new LoopbackInServer(mode, type, pid, handle.manager);
            break;
        }
        case ManagerState::ALIVE:
        default:
            handle.manager = std::make_shared<AudioLoopbackManager>();
            handle.server = new LoopbackInServer(mode, type, pid, handle.manager);
            break;
    }
    return handle;
}

ManagerState PickManagerState(FuzzedDataProvider &fdp)
{
    return fdp.PickValueInArray<ManagerState>({
        ManagerState::ALIVE,
        ManagerState::NULLPTR,
        ManagerState::EXPIRED,
        ManagerState::WITH_HANDLER,
    });
}

void EnableFuzzTest(FuzzedDataProvider &fdp)
{
    auto handle = MakeServer(fdp, PickManagerState(fdp));
    if (handle.server == nullptr) {
        return;
    }
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    bool enable = fdp.ConsumeBool();
    int32_t result = handle.server->Enable(enable, ret);
    (void)result;
    (void)ret;
}

void GetStatusFuzzTest(FuzzedDataProvider &fdp)
{
    auto handle = MakeServer(fdp, PickManagerState(fdp));
    if (handle.server == nullptr) {
        return;
    }
    int32_t status = fdp.ConsumeIntegral<int32_t>();
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    int32_t result = handle.server->GetStatus(status, ret);
    (void)result;
    (void)status;
    (void)ret;
}

void GetVolumeFuzzTest(FuzzedDataProvider &fdp)
{
    auto handle = MakeServer(fdp, PickManagerState(fdp));
    if (handle.server == nullptr) {
        return;
    }
    float volume = fdp.ConsumeFloatingPoint<float>();
    int32_t ret = fdp.ConsumeIntegral<int32_t>();
    int32_t result = handle.server->GetVolume(volume, ret);
    (void)result;
    (void)volume;
    (void)ret;
}

void AllMethodsFuzzTest(FuzzedDataProvider &fdp)
{
    auto handle = MakeServer(fdp, PickManagerState(fdp));
    if (handle.server == nullptr) {
        return;
    }
    int32_t enableRet = 0;
    handle.server->Enable(fdp.ConsumeBool(), enableRet);
    int32_t status = 0;
    int32_t statusRet = 0;
    handle.server->GetStatus(status, statusRet);
    float volume = 0.0f;
    int32_t volumeRet = 0;
    handle.server->GetVolume(volume, volumeRet);
}

void LifecycleFuzzTest(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<AudioLoopbackManager>();
    sptr<IRemoteObject> handlerLoopback;
    if (fdp.ConsumeBool()) {
        manager->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
            MakeCallback(), handlerLoopback, GetPid(fdp));
    }
    LoopbackType type = GetType(fdp);
    int32_t pid = GetPid(fdp);
    sptr<LoopbackInServer> server = new LoopbackInServer(LOOPBACK_HARDWARE, type, pid, manager);
    if (server == nullptr) {
        return;
    }

    int32_t ret = 0;
    int32_t status = 0;
    float volume = 0.0f;
    server->Enable(fdp.ConsumeBool(), ret);
    server->GetStatus(status, ret);
    server->GetVolume(volume, ret);

    manager.reset();
    server->Enable(fdp.ConsumeBool(), ret);
    server->GetStatus(status, ret);
    server->GetVolume(volume, ret);
}

void GettersFuzzTest(FuzzedDataProvider &fdp)
{
    auto handle = MakeServer(fdp, PickManagerState(fdp));
    if (handle.server == nullptr) {
        return;
    }
    (void)handle.server->GetMode();
    (void)handle.server->GetType();
    (void)handle.server->GetPid();
}

void ExecuteRandomOp(sptr<LoopbackInServer> &server, RandomOperation operation, FuzzedDataProvider &fdp)
{
    switch (operation) {
        case RandomOperation::ENABLE: {
            int32_t ret = 0;
            server->Enable(fdp.ConsumeBool(), ret);
            break;
        }
        case RandomOperation::GET_STATUS: {
            int32_t status = 0;
            int32_t ret = 0;
            server->GetStatus(status, ret);
            break;
        }
        case RandomOperation::GET_VOLUME: {
            float volume = 0.0f;
            int32_t ret = 0;
            server->GetVolume(volume, ret);
            break;
        }
        case RandomOperation::GET_MODE:
            (void)server->GetMode();
            break;
        case RandomOperation::GET_TYPE:
            (void)server->GetType();
            break;
        case RandomOperation::GET_PID:
            (void)server->GetPid();
            break;
    }
}

void RandomSequenceFuzzTest(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<AudioLoopbackManager>();
    sptr<IRemoteObject> handlerLoopback;
    if (fdp.ConsumeBool()) {
        manager->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER,
            MakeCallback(), handlerLoopback, GetPid(fdp));
    }
    sptr<LoopbackInServer> server = new LoopbackInServer(LOOPBACK_HARDWARE, GetType(fdp), GetPid(fdp), manager);
    if (server == nullptr) {
        return;
    }
    int32_t callCount = fdp.ConsumeIntegralInRange<int32_t>(1, MAX_CALL_COUNT);
    for (int32_t i = 0; i < callCount; i++) {
        ExecuteRandomOp(server, fdp.PickValueInArray<RandomOperation>({
            RandomOperation::ENABLE, RandomOperation::GET_STATUS, RandomOperation::GET_VOLUME,
            RandomOperation::GET_MODE, RandomOperation::GET_TYPE, RandomOperation::GET_PID,
        }), fdp);
        if (fdp.ConsumeBool()) {
            manager.reset();
        }
    }
}

void Test(FuzzedDataProvider &fdp)
{
    auto func = fdp.PickValueInArray({
        EnableFuzzTest,
        GetStatusFuzzTest,
        GetVolumeFuzzTest,
        AllMethodsFuzzTest,
        LifecycleFuzzTest,
        GettersFuzzTest,
        RandomSequenceFuzzTest,
    });
    func(fdp);
}
} // namespace
} // namespace AudioStandard
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    if (data == nullptr || size < OHOS::AudioStandard::THRESHOLD) {
        return 0;
    }
    FuzzedDataProvider fdp(data, size);
    OHOS::AudioStandard::Test(fdp);
    return 0;
}
