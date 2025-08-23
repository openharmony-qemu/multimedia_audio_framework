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
#include "audio_interrupt_service.h"
#include "audio_policy_server.h"
#include "audio_session_info.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "access_token.h"
#include "i_hpae_manager.h"
#include "manager/hdi_adapter_manager.h"
#include "util/id_handler.h"
using namespace std;

namespace OHOS {
namespace AudioStandard {
using namespace std;
bool g_hasPermission = false;
bool g_hasServerInit = false;
const int32_t SYSTEM_ABILITY_ID = 3009;
const bool RUN_ON_CREATE = false;
const std::u16string FORMMGR_INTERFACE_TOKEN = u"IAudioPolicy";
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

void AudioFuzzTestGetPermission()
{
    if (!g_hasPermission) {
        uint64_t tokenId;
        constexpr int perNum = 10;
        const char *perms[perNum] = {
            "ohos.permission.MICROPHONE",
            "ohos.permission.MANAGE_INTELLIGENT_VOICE",
            "ohos.permission.MANAGE_AUDIO_CONFIG",
            "ohos.permission.MICROPHONE_CONTROL",
            "ohos.permission.MODIFY_AUDIO_SETTINGS",
            "ohos.permission.ACCESS_NOTIFICATION_POLICY",
            "ohos.permission.USE_BLUETOOTH",
            "ohos.permission.CAPTURE_VOICE_DOWNLINK_AUDIO",
            "ohos.permission.RECORD_VOICE_CALL",
            "ohos.permission.MANAGE_SYSTEM_AUDIO_EFFECTS",
        };

        NativeTokenInfoParams infoInstance = {
            .dcapsNum = 0,
            .permsNum = 10,
            .aclsNum = 0,
            .dcaps = nullptr,
            .perms = perms,
            .acls = nullptr,
            .processName = "audiofuzztest",
            .aplStr = "system_basic",
        };
        tokenId = GetAccessTokenId(&infoInstance);
        SetSelfTokenID(tokenId);
        OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
        g_hasPermission = true;
    }
}

void MoreFuzzTest()
{
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    interruptService->GetAudioServerProxy();
    interruptService->WriteServiceStartupError();

    int32_t pid = GetData<int32_t>();
    interruptService->OnSessionTimeout(pid);
    interruptService->HandleSessionTimeOutEvent(pid);
}

void AddAudioSessionFuzzTest()
{
    int32_t sessionStrategy = 0;

    AudioInterrupt incomingInterrupt;
    incomingInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    incomingInterrupt.audioFocusType.sourceType = SOURCE_TYPE_VOICE_COMMUNICATION;
    AudioInterrupt activeInterrupt;
    activeInterrupt.audioFocusType.streamType = STREAM_MUSIC;
    AudioFocusEntry focusEntry;
    focusEntry.isReject = false;

    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    interruptService->CanMixForSession(incomingInterrupt, activeInterrupt, focusEntry);
    interruptService->CanMixForIncomingSession(incomingInterrupt, activeInterrupt, focusEntry);
    interruptService->CanMixForActiveSession(incomingInterrupt, activeInterrupt, focusEntry);
    interruptService->IsIncomingStreamLowPriority(focusEntry);
    interruptService->IsActiveStreamLowPriority(focusEntry);
}

void AddSetAudioManagerInterruptCallbackFuzzTest()
{
    MessageParcel data;
    data.WriteInterfaceToken(FORMMGR_INTERFACE_TOKEN);
    data.WriteBuffer(RAW_DATA, g_dataSize);
    data.RewindRead(0);
    sptr<IRemoteObject> object = data.ReadRemoteObject();
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    interruptService->GetAudioServerProxy();
    if (object == nullptr) {
        return;
    }
    interruptService->SetAudioManagerInterruptCallback(object);

    int32_t zoneId = GetData<int32_t>();
    uint32_t sessionId = GetData<uint32_t>();
    uint32_t uid = GetData<uint32_t>();
    interruptService->SetAudioInterruptCallback(zoneId, sessionId, object, uid);
}

void ClearAudioFocusInfoListOnAccountsChangedFuzzTest()
{
    int id = GetData<int>();
    std::shared_ptr<AudioInterruptService> interruptService = std::make_shared<AudioInterruptService>();
    interruptService->ClearAudioFocusInfoListOnAccountsChanged(id);
}

typedef void (*TestFuncs[4])();

TestFuncs g_testFuncs = {
    MoreFuzzTest,
    AddAudioSessionFuzzTest,
    AddSetAudioManagerInterruptCallbackFuzzTest,
    ClearAudioFocusInfoListOnAccountsChangedFuzzTest,
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

    OHOS::AudioStandard::AudioFuzzTestGetPermission();
    OHOS::AudioStandard::FuzzTest(data, size);
    return 0;
}