/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "get_server_util.h"

namespace OHOS {
namespace AudioStandard {

const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
bool g_hasServerInit = false;
const int32_t DEFAULT_SYSTEM_ABILITY_ID = -1000;

void InitA2dpOffloadManager(AudioPolicyServer* server)
{
    server->audioPolicyService_.audioA2dpOffloadManager_ = std::make_shared<AudioA2dpOffloadManager>();
    if (server->audioPolicyService_.audioA2dpOffloadManager_) {
        server->audioPolicyService_.audioA2dpOffloadManager_->Init();
    }
}

AudioPolicyServer* GetServerUtil::GetServerPtr()
{
    static AudioPolicyServer server(SYSTEM_ABILITY_ID, RUN_ON_CREATE);
    if (!g_hasServerInit) {
        AUDIO_INFO_LOG("AudioPolicyServiceUnitTest::GetServerPtr  server.OnStart()");
        InitA2dpOffloadManager(&server);
        server.OnStart();
        server.OnDump();
        server.OnAddSystemAbility(AUDIO_DISTRIBUTED_SERVICE_ID, "");
#ifdef FEATURE_MULTIMODALINPUT_INPUT
        server.OnAddSystemAbility(MULTIMODAL_INPUT_SERVICE_ID, "");
#endif
        server.OnAddSystemAbility(BLUETOOTH_HOST_SYS_ABILITY_ID, "");
        server.OnAddSystemAbility(POWER_MANAGER_SERVICE_ID, "");
        server.OnAddSystemAbility(SUBSYS_ACCOUNT_SYS_ABILITY_ID_BEGIN, "");
        server.audioPolicyService_.SetDefaultDeviceLoadFlag(true);
        server.OnAddSystemAbility(DEFAULT_SYSTEM_ABILITY_ID, "");
        server.OnAddSystemAbility(MEMORY_MANAGER_SA_ID, "");
        g_hasServerInit = true;
    }
    return &server;
}

static AudioPolicyServer* GetServerPtr()
{
    return GetServerUtil::GetServerPtr();
}
} // namespace AudioStandard
} // namespace OHOS
