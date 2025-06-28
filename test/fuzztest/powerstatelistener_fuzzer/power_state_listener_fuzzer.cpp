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

#include <iostream>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include "audio_info.h"
#include "audio_policy_server.h"
#include "audio_policy_service.h"
#include "audio_device_info.h"
#include "audio_utils.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "audio_channel_blend.h"
#include "volume_ramp.h"
#include "audio_speed.h"

#include "audio_policy_utils.h"
#include "audio_stream_descriptor.h"
#include "audio_limiter_manager.h"
#include "dfx_msg_manager.h"

#include "audio_source_clock.h"
#include "capturer_clock_manager.h"
#include "hpae_policy_manager.h"
#include "audio_policy_state_monitor.h"
#include "audio_device_info.h"
#include "audio_spatialization_service.h"
#include "suspend/sync_sleep_callback_ipc_interface_code.h"
#include "hibernate/sync_hibernate_callback_ipc_interface_code.h"

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 6;

typedef void (*TestFuncs)();

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

void PowerStateListenerControlAudioFocusFuzzTest()
{
    sptr<AudioPolicyServer> audioPolicyServer;
    auto powerStateListener = std::make_shared<PowerStateListener>(audioPolicyServer);
    powerStateListener->audioPolicyServer_ = nullptr;
    powerStateListener->ControlAudioFocus(true);
}

void OnSyncWakeupCallbackStubFuzzTest()
{
    sptr<AudioPolicyServer> audioPolicyServer;
    std::shared_ptr<SyncHibernateListenerStub> syncHibernateListenerStub =
        std::make_shared<SyncHibernateListener>(audioPolicyServer);
    syncHibernateListenerStub->OnSyncWakeupCallbackStub();
}

void OnSyncHibernateCallbackStubFuzzTest()
{
    sptr<AudioPolicyServer> audioPolicyServer;
    std::shared_ptr<SyncHibernateListenerStub> syncHibernateListenerStub =
        std::make_shared<SyncHibernateListener>(audioPolicyServer);
    syncHibernateListenerStub->OnSyncHibernateCallbackStub();
}

void PowerStateListenerStubOnRemoteRequestFuzzTest()
{
    sptr<AudioPolicyServer> audioPolicyServer;
    shared_ptr<PowerStateListenerStub> powerStateListenerStub = make_shared<PowerStateListener>(audioPolicyServer);
    vector<PowerMgr::SyncSleepCallbackInterfaceCode> codeList = {
        PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_SLEEP,
        PowerMgr::SyncSleepCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP,
    };
    uint32_t codeCount = GetData<uint32_t>() % codeList.size();
    uint32_t code = static_cast<int32_t>(codeList[codeCount]);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(PowerStateListenerStub::GetDescriptor());
    powerStateListenerStub->OnRemoteRequest(code, data, reply, option);
}

void SyncHibernateListenerControlAudioFocusFuzzTest()
{
    sptr<AudioPolicyServer> audioPolicyServer;
    auto syncHibernateListener = std::make_shared<SyncHibernateListener>(audioPolicyServer);
    syncHibernateListener->audioPolicyServer_ = nullptr;
    syncHibernateListener->ControlAudioFocus(true);
}

void SyncHibernateListenerStubOnRemoteRequestFuzzTest()
{
    sptr<AudioPolicyServer> audioPolicyServer;
    std::shared_ptr<SyncHibernateListenerStub> syncHibernateListenerStub =
        std::make_shared<SyncHibernateListener>(audioPolicyServer);
    vector<PowerMgr::SyncHibernateCallbackInterfaceCode> codeList = {
        PowerMgr::SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_HIBERNATE,
        PowerMgr::SyncHibernateCallbackInterfaceCode::CMD_ON_SYNC_WAKEUP,
    };
    uint32_t codeCount = GetData<uint32_t>() % codeList.size();
    uint32_t code = static_cast<int32_t>(codeList[codeCount]);
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteInterfaceToken(SyncHibernateListenerStub::GetDescriptor());
    syncHibernateListenerStub->OnRemoteRequest(code, data, reply, option);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    PowerStateListenerControlAudioFocusFuzzTest,
    OnSyncWakeupCallbackStubFuzzTest,
    OnSyncHibernateCallbackStubFuzzTest,
    PowerStateListenerStubOnRemoteRequestFuzzTest,
    SyncHibernateListenerControlAudioFocusFuzzTest,
    SyncHibernateListenerStubOnRemoteRequestFuzzTest,
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
