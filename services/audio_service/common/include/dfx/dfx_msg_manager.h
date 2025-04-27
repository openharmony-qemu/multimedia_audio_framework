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

#ifndef ST_DFX_MESSAGE_MGR_H
#define ST_DFX_MESSAGE_MGR_H

#include <map>
#include <list>
#include <thread>
#include <chrono>
#include <atomic>
#include <tuple>

#include "dfx_stat.h"
#include "dfx_utils.h"
#include "audio_info.h"
#include "audio_utils.h"
#include "audio_safe_block_queue.h"
#include "event_handler.h"
#include "callback_handler.h"

namespace OHOS {
namespace AudioStandard {

struct DfxMessage {
    int32_t appUid{};
    std::list<RenderDfxInfo> renderInfo{};
    std::list<InterruptDfxInfo> interruptInfo{};
    std::list<CapturerDfxInfo> captureInfo{};
    bool ready = false;
};

enum class DfxMsgIndexType {
    DFX_MSG_IDX_TYPE_RENDER_INFO,
    DFX_MSG_IDX_TYPE_CAPTURE_INFO,
    DFX_MSG_IDX_TYPE_INTERRUPT_INFO,
    DFX_MSG_IDX_TYPE_INTERRUPT_EFFECT,
};

struct DfxReportResult {
    std::string appName{};
    std::string appVersion{};
    std::vector<uint32_t> rendererActions{};
    std::vector<uint32_t> renderInfo{};
    std::vector<uint64_t> renderTimestamp{};
    std::vector<std::string> rendererStats{};
    std::vector<uint32_t> interruptActions{};
    std::vector<uint64_t> interruptTimestamp{};
    std::vector<std::string> interruptEffect{};
    std::vector<uint32_t> interruptInfo{};
    std::vector<uint32_t> capturerActions{};
    std::vector<uint32_t> capturerInfo{};
    std::vector<uint64_t> capturerTimestamp{};
    std::vector<std::string> capturerStat{};
    std::vector<uint8_t> appState{};
    std::vector<uint64_t> appStateTimestamp{};
    uint64_t summary{};
};

class DfxMsgHandler : public IHandler {
public:
    DfxMsgHandler(IHandler* handler);
    void OnHandle(uint32_t code, int64_t data) override;
private:
    IHandler *handler_ = nullptr;
};

class DfxMsgManager : public IHandler {
public:
    static DfxMsgManager& GetInstance();
    void OnHandle(uint32_t code, int64_t data) override;

    bool Enqueue(const DfxMessage &msg);
    bool CheckCanAddAppInfo(int32_t appUid);
    void Init();
    void SaveAppInfo(const DfxRunningAppInfo info);
    void UpdateAppState(int32_t appUid, DfxAppState appstate, bool forceUpdate = false);
private:
    DfxMsgManager();
    virtual ~DfxMsgManager();
    void TimeFunc();
    bool ProcessCheck(const DfxMessage &msg);
    bool Process(DfxMessage &msg);
    void SafeSendCallBackEvent(uint32_t eventCode, int64_t data, int64_t delayTime);
    void CheckReportDfxMsg();
    bool IsMsgReady(const DfxMessage &msg);
    void HandleThreadExit();
    void InsertReportQueue(const DfxMessage &msg);
    bool ProcessInner(int32_t index, std::list<RenderDfxInfo> &dfxInfo, std::list<RenderDfxInfo> &curDfxInfo);
    bool ProcessInner(int32_t index, std::list<InterruptDfxInfo> &dfxInfo, std::list<InterruptDfxInfo> &curDfxInfo);
    bool ProcessInner(int32_t index, std::list<CapturerDfxInfo> &dfxInfo, std::list<CapturerDfxInfo> &curDfxInfo);

    void WriteInterruptMsg(DfxMessage &msg, const std::unique_ptr<DfxReportResult> &result);
    void WriteRenderMsg(DfxMessage &msg, const std::unique_ptr<DfxReportResult> &result);
    void WriteCapturerMsg(DfxMessage &msg, const std::unique_ptr<DfxReportResult> &result);
    void WriteRunningAppMsg(DfxMessage &msg, const std::unique_ptr<DfxReportResult> &result);
    void HandleToHiSysEvent(DfxMessage &msg);
    void WritePlayAudioStatsEvent(const std::unique_ptr<DfxReportResult> &result);
    void UpdateAction(int32_t appUid, std::list<RenderDfxInfo> &renderInfo);
    void UpdateAction(int32_t appUid, std::list<CapturerDfxInfo> &capturerInfo);
    void UpdateAction(int32_t appUid, std::list<InterruptDfxInfo> &interruptInfo);
    uint8_t& GetDfxIndexByType(int32_t appUid, DfxMsgIndexType type);
    bool CheckIsInterrupted(InterruptStage stage);

    void LogDfxResult(const std::unique_ptr<DfxReportResult> &result);

    AudioSafeBlockQueue<DfxMessage> msgQueue_;
    std::multimap<int32_t, DfxMessage> reportQueue_;
    std::unique_ptr<std::thread> timeThread_ = nullptr;
    std::atomic_bool startThread_ = true;
    std::time_t lastReportTime_{};
    std::map<int32_t, DfxRunningAppInfo> appInfo_;
    std::atomic_bool isFull_ = false;
    std::atomic_int32_t reportedCnt_ = 0;
    std::shared_ptr<DfxMsgHandler> handler_ = nullptr;
    std::mutex runnerMutex_;
    std::shared_ptr<CallbackHandler> callbackHandler_ = nullptr;
    std::condition_variable cvReachLimit_;
    std::mutex mutexLock_;
    std::map<int32_t, std::tuple<uint8_t, uint8_t, uint8_t, uint8_t>> indexesInfo_;

    enum {
        DFX_CHECK_REPORT_MSG = 0,
    };
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_DFX_MESSAGE_MGR_H