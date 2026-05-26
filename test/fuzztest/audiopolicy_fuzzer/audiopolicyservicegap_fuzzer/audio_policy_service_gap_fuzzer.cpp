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

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include <fuzzer/FuzzedDataProvider.h>

#include "audio_device_info.h"
#include "audio_effect.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"

namespace OHOS {
namespace AudioStandard {
namespace {
constexpr int32_t SYSTEM_ABILITY_ID = 3009;
constexpr bool RUN_ON_CREATE = false;
constexpr size_t THRESHOLD = sizeof(uint64_t);
constexpr size_t MAX_STRING_LEN = 64;

bool g_hasServerInit = false;

template<typename T>
T ConsumeEnum(FuzzedDataProvider &fdp)
{
    return static_cast<T>(fdp.ConsumeIntegral<int32_t>());
}

sptr<AudioPolicyServer> GetServerPtr()
{
    static sptr<AudioPolicyServer> server = sptr<AudioPolicyServer>::MakeSptr(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit && server != nullptr) {
        server->OnStart();
        server->OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server->OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server->OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server->OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server->OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server->audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        g_hasServerInit = true;
    }
    return server;
}

void AudioPolicyServiceOutputMuteGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    auto &service = server->audioPolicyService_;

    service.SetAncoSourceOutputStreamMute(fdp.ConsumeIntegral<int32_t>(), fdp.ConsumeBool());
    service.SetSourceOutputStreamMuteByStreamId(fdp.ConsumeIntegral<int32_t>(), fdp.ConsumeBool());
}

void AudioPolicyServiceRoutingGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    auto &service = server->audioPolicyService_;

    service.IsEnhancedRoutingSupported();
    service.GetAudioSceneFromAllZones();
    service.MuteMediaWhenAccountsChanged();
}

void AudioPolicyServiceEffectGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    auto &service = server->audioPolicyService_;

    service.LoadEffectLibrary();
    service.IsVoipStreamUsage(ConsumeEnum<StreamUsage>(fdp));
    std::vector<AudioEffectProperty> properties;
    service.CheckVoipAnrOn(properties);
}

void AudioPolicyServiceDeviceGapFuzzTest(FuzzedDataProvider &fdp)
{
    auto server = GetServerPtr();
    if (server == nullptr) {
        return;
    }
    AudioDeviceDescriptor desc;
    desc.deviceType_ = ConsumeEnum<DeviceType>(fdp);
    desc.deviceRole_ = ConsumeEnum<DeviceRole>(fdp);
    desc.macAddress_ = fdp.ConsumeRandomLengthString(MAX_STRING_LEN);

    server->audioPolicyService_.OnConnectFailed(desc);
    server->audioPolicyService_.RegiestPolicy();
}

void Test(FuzzedDataProvider &fdp)
{
    auto func = fdp.PickValueInArray({
        AudioPolicyServiceOutputMuteGapFuzzTest,
        AudioPolicyServiceRoutingGapFuzzTest,
        AudioPolicyServiceEffectGapFuzzTest,
        AudioPolicyServiceDeviceGapFuzzTest,
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
