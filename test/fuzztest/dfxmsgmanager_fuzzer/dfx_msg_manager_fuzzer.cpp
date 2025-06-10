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

namespace OHOS {
namespace AudioStandard {
using namespace std;

static const uint8_t* RAW_DATA = nullptr;
static size_t g_dataSize = 0;
static size_t g_pos;
const size_t THRESHOLD = 10;
const uint8_t TESTSIZE = 15;
const int32_t COUNT = 100;
const int32_t NUM_2 = 2;

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

void SaveAppInfoFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxRunningAppInfo info;
    info.appUid = 1;
    dfxMsgManager.SaveAppInfo(info);
}

void ProcessCheckFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    RenderDfxInfo renderInfo;
    InterruptDfxInfo interruptInfo;
    CapturerDfxInfo captureInfo;
    msg.appUid = 1;
    
    for (int i = 0; i < COUNT; i++) {
        msg.renderInfo.push_back(renderInfo);
    }
    dfxMsgManager.ProcessCheck(msg);
    msg.renderInfo.clear();

    dfxMsgManager.isFull_ = GetData<uint32_t>() % NUM_2;
    dfxMsgManager.ProcessCheck(msg);
}

void ProcessFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    msg.appUid = GetData<int32_t>();
    dfxMsgManager.Process(msg);
}

void ProcessInnerFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    uint32_t index = 0;
    std::list<RenderDfxInfo> dfxInfo;
    std::list<RenderDfxInfo> curDfxInfo;
    RenderDfxInfo renderInfo;
    dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
    for (int i = 0; i < COUNT; i++) {
        dfxInfo.push_back(renderInfo);
    }
    dfxMsgManager.ProcessInner(index, dfxInfo, curDfxInfo);
}

void EnqueueFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    dfxMsgManager.isFull_ = GetData<uint32_t>() % NUM_2;
    dfxMsgManager.Enqueue(msg);
}

void HandleToHiSysEventFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    dfxMsgManager.reportedCnt_ = GetData<uint32_t>();
    dfxMsgManager.reportQueue_.clear();
    dfxMsgManager.HandleToHiSysEvent(msg);
}

void GetAdapterNameBySessionIdFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    RenderDfxInfo renderInfo;
    std::unique_ptr<DfxReportResult> bean = std::make_unique<DfxReportResult>();
    msg.appUid = 1;
    dfxMsgManager.reportQueue_.clear();
    for (int i = 0; i < COUNT; i++) {
        msg.renderInfo.push_back(renderInfo);
    }
    dfxMsgManager.WriteRenderMsg(msg, bean);
}

void WriteInterruptMsgFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    InterruptDfxInfo interruptInfo;
    std::unique_ptr<DfxReportResult> bean = std::make_unique<DfxReportResult>();
    msg.appUid = 1;
    for (int i = 0; i < COUNT; i++) {
        msg.interruptInfo.push_back(interruptInfo);
    }
    dfxMsgManager.WriteInterruptMsg(msg, bean);
}

void WritePlayAudioStatsEventFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    std::unique_ptr<DfxReportResult> result = nullptr;
    dfxMsgManager.WritePlayAudioStatsEvent(result);
    result = std::make_unique<DfxReportResult>();
    dfxMsgManager.WritePlayAudioStatsEvent(result);
}

void OnHandleFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    uint32_t code = 0;
    int64_t data = 0;
    dfxMsgManager.OnHandle(code, data);
    code = GetData<uint32_t>();
    dfxMsgManager.OnHandle(code, data);
}

void CheckReportDfxMsgFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    msg.appUid = 1;
    dfxMsgManager.InsertReportQueue(msg);
    time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    dfxMsgManager.lastReportTime_ = now;
    dfxMsgManager.InsertReportQueue(msg);
    dfxMsgManager.reportedCnt_ = GetData<uint32_t>();
    dfxMsgManager.CheckReportDfxMsg();
}

void IsMsgReadyFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    RenderDfxInfo renderInfo;
    InterruptDfxInfo interruptInfo;
    CapturerDfxInfo captureInfo;
    msg.appUid = 1;
    for (int i = 0; i < COUNT; i++) {
        msg.interruptInfo.push_back(interruptInfo);
    }
    for (int i = 0; i < COUNT; i++) {
        msg.renderInfo.push_back(renderInfo);
    }
    dfxMsgManager.IsMsgReady(msg);
}

void HandleThreadExitFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    std::unique_ptr<DfxReportResult> result = std::make_unique<DfxReportResult>();
    result->appName = "appName";
    result->appVersion = "1.0";
    result->summary = GetData<uint32_t>();
    dfxMsgManager.LogDfxResult(result);
    dfxMsgManager.HandleThreadExit();
}

void WriteRunningAppMsgFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    DfxMessage msg;
    std::unique_ptr<DfxReportResult> result = std::make_unique<DfxReportResult>();
    DfxRunningAppInfo appinfo;
    msg.appUid = 1;
    appinfo.appUid = 1;
    appinfo.appName = "appName";
    appinfo.versionName = "1.0";
    appinfo.appStateVec.push_back(1);
    appinfo.appStateTimeStampVec.push_back(1);
    dfxMsgManager.WriteRunningAppMsg(msg, result);
}

void CheckCanAddAppInfoFuzzTest()
{
    DfxMsgManager &dfxMsgManager = DfxMsgManager::GetInstance();
    int32_t appUid = GetData<int32_t>();
    dfxMsgManager.CheckCanAddAppInfo(appUid);
    appUid = static_cast<int32_t>(getuid());
    dfxMsgManager.CheckCanAddAppInfo(appUid);
}

TestFuncs g_testFuncs[TESTSIZE] = {
    SaveAppInfoFuzzTest,
    ProcessCheckFuzzTest,
    ProcessFuzzTest,
    ProcessInnerFuzzTest,
    EnqueueFuzzTest,
    HandleToHiSysEventFuzzTest,
    GetAdapterNameBySessionIdFuzzTest,
    WriteInterruptMsgFuzzTest,
    WritePlayAudioStatsEventFuzzTest,
    OnHandleFuzzTest,
    CheckReportDfxMsgFuzzTest,
    IsMsgReadyFuzzTest,
    HandleThreadExitFuzzTest,
    WriteRunningAppMsgFuzzTest,
    CheckCanAddAppInfoFuzzTest,
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
