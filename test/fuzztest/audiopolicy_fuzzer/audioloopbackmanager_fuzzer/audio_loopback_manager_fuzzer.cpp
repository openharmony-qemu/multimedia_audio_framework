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
#include "audio_loopback_manager.h"
#include "loopback_callback_stub.h"

namespace OHOS {
namespace AudioStandard {
namespace {
const int32_t BASE_PID = 1000;
const int32_t MAX_OPERATION_COUNT = 12;
const size_t THRESHOLD = sizeof(uint64_t) + sizeof(uint16_t);

enum class RandomOperation {
    CREATE_LOOPBACK,
    DESTROY_LOOPBACK,
    ENABLE_LOOPBACK,
    GET_STATUS,
    GET_VOLUME,
    CONTROL_DIED,
    NORMAL_DIED,
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

void CreateLoopbackFuzzTest(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<AudioLoopbackManager>();
    sptr<IRemoteObject> loopback;
    auto callback = MakeCallback();
    manager->CreateLoopback(GetMode(fdp), GetType(fdp), callback, loopback, GetPid(fdp));
}

void GlobalLifecycleFuzzTest(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<AudioLoopbackManager>();
    int32_t handlerPid = GetPid(fdp);
    int32_t controlPid = GetPid(fdp);
    sptr<IRemoteObject> handler;
    sptr<IRemoteObject> control;
    auto handlerCallback = MakeCallback();
    auto controlCallback = MakeCallback();

    manager->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, handlerCallback, handler, handlerPid);
    manager->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, controlCallback, control, controlPid);
    manager->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, MakeCallback(), control, GetPid(fdp));

    bool enable = fdp.ConsumeBool();
    manager->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, enable, controlPid);
    manager->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, enable, GetPid(fdp));
    manager->Enable(LOOPBACK_TYPE_GLOBAL_CONTROL, !enable, controlPid);
    manager->Enable(LOOPBACK_TYPE_GLOBAL_HANDLER, fdp.ConsumeBool(), handlerPid);

    int status = fdp.ConsumeIntegral<int>();
    float volume = fdp.ConsumeFloatingPoint<float>();
    manager->GetStatus(LOOPBACK_TYPE_GLOBAL_CONTROL, status, controlPid);
    manager->GetVolume(LOOPBACK_TYPE_GLOBAL_CONTROL, volume, controlPid);
    manager->GetStatus(GetType(fdp), status, GetPid(fdp));
    manager->GetVolume(GetType(fdp), volume, GetPid(fdp));

    if (fdp.ConsumeBool()) {
        manager->OnControlDied(controlPid);
    }
    if (fdp.ConsumeBool()) {
        manager->OnHandlerDied();
    }

    manager->DestroyLoopback(GetType(fdp), GetPid(fdp));
    manager->DestroyLoopback(LOOPBACK_TYPE_GLOBAL_CONTROL, controlPid);
    manager->DestroyLoopback(LOOPBACK_TYPE_GLOBAL_HANDLER, handlerPid);
}

void NormalLifecycleFuzzTest(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<AudioLoopbackManager>();
    int32_t pid = GetPid(fdp);
    sptr<IRemoteObject> loopback;
    manager->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_NORMAL, MakeCallback(), loopback, pid);
    manager->Enable(LOOPBACK_TYPE_NORMAL, fdp.ConsumeBool(), pid);
    int status = 0;
    float volume = 0.0f;
    manager->GetStatus(LOOPBACK_TYPE_NORMAL, status, pid);
    manager->GetVolume(LOOPBACK_TYPE_NORMAL, volume, pid);
    if (fdp.ConsumeBool()) {
        manager->OnNormalDied(pid);
    }
    manager->DestroyLoopback(LOOPBACK_TYPE_NORMAL, pid);
}

void RandomOperationSequenceFuzzTest(FuzzedDataProvider &fdp)
{
    auto manager = std::make_shared<AudioLoopbackManager>();
    int32_t handlerPid = BASE_PID;
    int32_t controlPid = BASE_PID + 1;
    sptr<IRemoteObject> loopback;

    if (fdp.ConsumeBool()) {
        manager->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_HANDLER, MakeCallback(), loopback, handlerPid);
    }
    if (fdp.ConsumeBool()) {
        manager->CreateLoopback(LOOPBACK_HARDWARE, LOOPBACK_TYPE_GLOBAL_CONTROL, MakeCallback(), loopback, controlPid);
    }

    int32_t operationCount = fdp.ConsumeIntegralInRange<int32_t>(1, MAX_OPERATION_COUNT);
    for (int32_t i = 0; i < operationCount; i++) {
        LoopbackType type = GetType(fdp);
        int32_t pid = fdp.ConsumeBool() ? controlPid : GetPid(fdp);
        auto operation = fdp.PickValueInArray<RandomOperation>({
            RandomOperation::CREATE_LOOPBACK, RandomOperation::DESTROY_LOOPBACK, RandomOperation::ENABLE_LOOPBACK,
            RandomOperation::GET_STATUS, RandomOperation::GET_VOLUME,
            RandomOperation::CONTROL_DIED, RandomOperation::NORMAL_DIED,
        });
        switch (operation) {
            case RandomOperation::CREATE_LOOPBACK:
                manager->CreateLoopback(GetMode(fdp), type, MakeCallback(), loopback, pid);
                break;
            case RandomOperation::DESTROY_LOOPBACK:
                manager->DestroyLoopback(type, pid);
                break;
            case RandomOperation::ENABLE_LOOPBACK:
                manager->Enable(type, fdp.ConsumeBool(), pid);
                break;
            case RandomOperation::GET_STATUS: {
                int status = fdp.ConsumeIntegral<int>();
                manager->GetStatus(type, status, pid);
                break;
            }
            case RandomOperation::GET_VOLUME: {
                float volume = fdp.ConsumeFloatingPoint<float>();
                manager->GetVolume(type, volume, pid);
                break;
            }
            case RandomOperation::CONTROL_DIED:
                manager->OnControlDied(pid);
                break;
            case RandomOperation::NORMAL_DIED:
                manager->OnNormalDied(pid);
                break;
        }
    }
}

void DeathRecipientFuzzTest(FuzzedDataProvider &fdp)
{
    bool called = false;
    sptr<IRemoteObject::DeathRecipient> deathRecipient = new LoopbackDeathRecipient([&called]() { called = true; });
    wptr<IRemoteObject> remote;
    deathRecipient->OnRemoteDied(remote);
    if (fdp.ConsumeBool()) {
        deathRecipient->OnRemoteDied(remote);
    }
    (void)called;
}

void Test(FuzzedDataProvider &fdp)
{
    auto func = fdp.PickValueInArray({
        CreateLoopbackFuzzTest,
        GlobalLifecycleFuzzTest,
        NormalLifecycleFuzzTest,
        RandomOperationSequenceFuzzTest,
        DeathRecipientFuzzTest,
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
