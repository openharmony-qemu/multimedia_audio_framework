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
#ifndef LOG_TAG
#define LOG_TAG "DfxMsgManager"
#endif

#include <map>
#include <algorithm>

#include "dfx_msg_manager.h"
#include "hisysevent.h"
#include "audio_common_log.h"

namespace OHOS {
namespace AudioStandard {

static constexpr int32_t DEFAULT_DFX_REPORT_INTERVAL_MIN = 24 * 60;
static constexpr int32_t DFX_MSG_QUEUE_CAPACITY = 100;
static constexpr int32_t MAX_DFX_MSG_MEMBER_SIZE = 100;
static constexpr int32_t MAX_DFX_REPORT_APP_COUNT = 10;
static constexpr int64_t DFX_CHECK_REPORT_MSG_TIME_MS = 60 * 1000;
static constexpr uint32_t BIT_2_OFFSET = 1;
static constexpr uint32_t BIT_3_OFFSET = 2;
static constexpr int8_t INDEXES_TUPLE_INDEX_RENDER_INFO = 0;
static constexpr int8_t INDEXES_TUPLE_INDEX_CAPTURE_INFO = 1;
static constexpr int8_t INDEXES_TUPLE_INDEX_INTERRUPT_INFO = 2;
static constexpr int8_t INDEXES_TUPLE_INDEX_INTERRUPT_EFFECT = 3;

DfxMsgHandler::DfxMsgHandler(IHandler* handler) : handler_(handler) {}

void DfxMsgHandler::OnHandle(uint32_t code, int64_t data)
{
    CHECK_AND_RETURN_LOG(handler_ != nullptr, "handler is nullptr");
    handler_->OnHandle(code, data);
}

DfxMsgManager& DfxMsgManager::GetInstance()
{
    static DfxMsgManager instance;
    return instance;
}

DfxMsgManager::DfxMsgManager() : msgQueue_(DFX_MSG_QUEUE_CAPACITY)
{
}

void DfxMsgManager::OnHandle(uint32_t code, int64_t data)
{
    switch (code) {
        case DFX_CHECK_REPORT_MSG:
            CheckReportDfxMsg();
            break;
        default:
            break;
    }
}

void DfxMsgManager::SafeSendCallBackEvent(uint32_t eventCode, int64_t data, int64_t delayTime)
{
    Trace trace("DfxMsgManager::SafeSendCallBackEvent");
    CHECK_AND_RETURN_LOG(callbackHandler_ != nullptr, "Runner is Release");
    std::lock_guard<std::mutex> lock(runnerMutex_);
    callbackHandler_->SendCallbackEvent(eventCode, data, delayTime);
}

void DfxMsgManager::CheckReportDfxMsg()
{
    Trace trace("DfxMsgManager::CheckReportDfxMsg");
    SafeSendCallBackEvent(DFX_CHECK_REPORT_MSG, 0, DFX_CHECK_REPORT_MSG_TIME_MS);

    std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    auto interval = std::chrono::system_clock::from_time_t(now - lastReportTime_).time_since_epoch();
    int intervalMin = std::chrono::duration_cast<std::chrono::minutes>(interval).count();

    std::lock_guard<std::mutex> lock(mutexLock_);
    if (intervalMin >= DEFAULT_DFX_REPORT_INTERVAL_MIN) {
        AUDIO_INFO_LOG("time is up, report msg size=%{public}d", static_cast<int32_t>(reportQueue_.size()));
        for (auto &item : reportQueue_) {
            HandleToHiSysEvent(item.second);
        }
        reportQueue_.clear();
        isFull_ = false;
        reportedCnt_ = 0;
        lastReportTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        cvReachLimit_.notify_all();
        std::lock_guard<std::mutex> appInfoLock(appInfoMutex_);
        appInfo_.clear();
        indexesInfo_.clear();
    }

    for (auto it = reportQueue_.begin(); it != reportQueue_.end();) {
        if (reportedCnt_ >= MAX_DFX_REPORT_APP_COUNT) {
            reportQueue_.clear();
            break;
        }
        if (IsMsgReady(it->second)) {
            HandleToHiSysEvent(it->second);
            it = reportQueue_.erase(it);
        } else {
            ++it;
        }
    }
    if (reportedCnt_ >= MAX_DFX_REPORT_APP_COUNT) {
        AUDIO_WARNING_LOG("dfx report reach maximum size");
        isFull_ = true;
    }
}

bool DfxMsgManager::IsMsgReady(const DfxMessage &msg)
{
    return (msg.interruptInfo.size() == MAX_DFX_MSG_MEMBER_SIZE &&
            (msg.captureInfo.size() == MAX_DFX_MSG_MEMBER_SIZE ||
            msg.renderInfo.size() == MAX_DFX_MSG_MEMBER_SIZE));
}

void DfxMsgManager::Init()
{
    lastReportTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    timeThread_ = std::make_unique<std::thread>(&DfxMsgManager::TimeFunc, this);
    pthread_setname_np(timeThread_->native_handle(), "AudioServerDFXTiming");

    std::unique_lock<std::mutex> lock(runnerMutex_);
    if (callbackHandler_ == nullptr) {
        handler_ = std::make_shared<DfxMsgHandler>(this);
        callbackHandler_ = CallbackHandler::GetInstance(handler_, "OS_DfxMsgCB");
        AUDIO_INFO_LOG("init handler success");
    }
    lock.unlock();

    SafeSendCallBackEvent(DFX_CHECK_REPORT_MSG, 0, DFX_CHECK_REPORT_MSG_TIME_MS);
}

DfxMsgManager::~DfxMsgManager()
{
    AUDIO_INFO_LOG("DfxMsgManager deconstructor");
    HandleThreadExit();
    std::lock_guard<std::mutex> lock(runnerMutex_);
    if (callbackHandler_ != nullptr) {
        AUDIO_INFO_LOG("runner move");
        callbackHandler_->ReleaseEventRunner();
        callbackHandler_ = nullptr;
    }
}

void DfxMsgManager::HandleThreadExit()
{
    startThread_.store(false, std::memory_order_release);
    if (timeThread_ && timeThread_->joinable()) {
        timeThread_->detach();
    }

    msgQueue_.PushNoWait({});
    isFull_ = false;
    cvReachLimit_.notify_all();
}

void DfxMsgManager::TimeFunc()
{
    while (startThread_.load(std::memory_order_acquire)) {
        DfxMessage msg;
        while (!isFull_ && startThread_) {
            msg = msgQueue_.Pop();
            if (!ProcessCheck(msg)) {
                continue;
            } else {
                Process(msg);
            }
        }

        std::unique_lock<std::mutex> lock(mutexLock_);
        while (isFull_) {
            AUDIO_INFO_LOG("today report reach max count, wait...");
            cvReachLimit_.wait(lock, [&] { return !isFull_; });
        }
    }
    AUDIO_INFO_LOG("TimeFunc exit");
}

bool DfxMsgManager::ProcessCheck(const DfxMessage &msg)
{
    if (msg.appUid == DFX_INVALID_APP_UID ||
        msg.renderInfo.size() >= MAX_DFX_MSG_MEMBER_SIZE ||
        msg.interruptInfo.size() >= MAX_DFX_MSG_MEMBER_SIZE ||
        msg.captureInfo.size() >= MAX_DFX_MSG_MEMBER_SIZE) {
        AUDIO_INFO_LOG("invalid msg, renderInfo=%{public}d" \
            ", interruptInfo=%{public}d, captureInfo=%{public}d",
            static_cast<int32_t>(msg.renderInfo.size()), static_cast<int32_t>(msg.interruptInfo.size()),
            static_cast<int32_t>(msg.captureInfo.size()));
        return false;
    }

    if (isFull_) {
        AUDIO_INFO_LOG("dfx report reach maximum size, discard msg.appUid=%{public}d", msg.appUid);
        return false;
    }
    return true;
}

bool DfxMsgManager::Process(DfxMessage &msg)
{
    std::lock_guard<std::mutex> lock(mutexLock_);
    if (reportQueue_.count(msg.appUid) == 0) {
        InsertReportQueue(msg);
        return true;
    }

    bool processed = false;
    auto range = reportQueue_.equal_range(msg.appUid);
    for (auto it = range.first; it != range.second; ++it) {
        auto nextIt = it;
        ++nextIt;
        bool isLast = (nextIt == range.second);
        if (IsMsgReady(it->second) && !isLast) {
            continue;
        }
        if (processed) {
            break;
        }
        if (ProcessInner(msg.appUid, msg.renderInfo, it->second.renderInfo) ||
            ProcessInner(msg.appUid, msg.interruptInfo, it->second.interruptInfo) ||
            ProcessInner(msg.appUid, msg.captureInfo, it->second.captureInfo)) {
            processed = true;
        }
    }
    return processed;
}

void DfxMsgManager::InsertReportQueue(const DfxMessage &msg)
{
    if (reportQueue_.size() == MAX_DFX_REPORT_APP_COUNT) {
        Trace trace("reportQueue_ reach maximum size, can not insert");
        return;
    }

    if (reportQueue_.count(msg.appUid) == 0) {
        reportQueue_.insert(std::make_pair(msg.appUid, msg));
        return;
    }

    auto upper = reportQueue_.upper_bound(msg.appUid);
    auto lastIt = --upper;
    if (lastIt->second.renderInfo.empty() && !msg.renderInfo.empty()) {
        std::copy(msg.renderInfo.begin(), msg.renderInfo.end(), std::back_inserter(lastIt->second.renderInfo));
    }

    if (lastIt->second.interruptInfo.empty() && !msg.interruptInfo.empty()) {
        std::copy(msg.interruptInfo.begin(), msg.interruptInfo.end(),
            std::back_inserter(lastIt->second.interruptInfo));
    }

    if (lastIt->second.captureInfo.empty() && !msg.captureInfo.empty()) {
        std::copy(msg.captureInfo.begin(), msg.captureInfo.end(), std::back_inserter(lastIt->second.captureInfo));
    }
}

bool DfxMsgManager::ProcessInner(int32_t index,
    std::list<RenderDfxInfo> &dfxInfo, std::list<RenderDfxInfo> &curDfxInfo)
{
    bool processed = false;
    int32_t size = static_cast<int32_t>(dfxInfo.size());
    if (size != 0) {
        processed = true;
        int32_t vacancy = MAX_DFX_MSG_MEMBER_SIZE - static_cast<int32_t>(curDfxInfo.size());
        vacancy = std::max(vacancy, 0);
        if (vacancy == 0) {
            InsertReportQueue({.appUid = index, .renderInfo = dfxInfo});
            return processed;
        }
        if (vacancy < size) {
            auto start = std::next(dfxInfo.begin(), vacancy);
            std::list<RenderDfxInfo> split1{dfxInfo.begin(), start};
            std::list<RenderDfxInfo> split2{start, dfxInfo.end()};
            std::copy(split1.begin(), split1.end(), std::back_inserter(curDfxInfo));
            InsertReportQueue({.appUid = index, .renderInfo = split2});
        } else {
            std::copy(dfxInfo.begin(), dfxInfo.end(), std::back_inserter(curDfxInfo));
        }
    }
    return processed;
}

bool DfxMsgManager::ProcessInner(int32_t index,
    std::list<InterruptDfxInfo> &dfxInfo, std::list<InterruptDfxInfo> &curDfxInfo)
{
    bool processed = false;
    int32_t size = static_cast<int32_t>(dfxInfo.size());
    if (size != 0) {
        processed = true;
        int32_t vacancy = MAX_DFX_MSG_MEMBER_SIZE - static_cast<int32_t>(curDfxInfo.size());
        vacancy = std::max(vacancy, 0);
        if (vacancy == 0) {
            InsertReportQueue({.appUid = index, .interruptInfo = dfxInfo});
            return processed;
        }
        if (vacancy < size) {
            auto start = std::next(dfxInfo.begin(), vacancy);
            std::list<InterruptDfxInfo> split1{dfxInfo.begin(), start};
            std::list<InterruptDfxInfo> split2{start, dfxInfo.end()};
            std::copy(split1.begin(), split1.end(), std::back_inserter(curDfxInfo));
            InsertReportQueue({.appUid = index, .interruptInfo = split2});
        } else {
            std::copy(dfxInfo.begin(), dfxInfo.end(), std::back_inserter(curDfxInfo));
        }
    }
    return processed;
}

bool DfxMsgManager::ProcessInner(int32_t index,
    std::list<CapturerDfxInfo> &dfxInfo, std::list<CapturerDfxInfo> &curDfxInfo)
{
    bool processed = false;
    int32_t size = static_cast<int32_t>(dfxInfo.size());
    if (size != 0) {
        processed = true;
        int32_t vacancy = MAX_DFX_MSG_MEMBER_SIZE - static_cast<int32_t>(curDfxInfo.size());
        vacancy = std::max(vacancy, 0);
        if (vacancy == 0) {
            InsertReportQueue({.appUid = index, .captureInfo = dfxInfo});
            return processed;
        }
        if (vacancy < size) {
            auto start = std::next(dfxInfo.begin(), vacancy);
            std::list<CapturerDfxInfo> split1{dfxInfo.begin(), start};
            std::list<CapturerDfxInfo> split2{start, dfxInfo.end()};
            std::copy(split1.begin(), split1.end(), std::back_inserter(curDfxInfo));
            InsertReportQueue({.appUid = index, .captureInfo = split2});
        } else {
            std::copy(dfxInfo.begin(), dfxInfo.end(), std::back_inserter(curDfxInfo));
        }
    }
    return processed;
}

bool DfxMsgManager::Enqueue(const DfxMessage &msg)
{
    if (isFull_) {
        AUDIO_WARNING_LOG("queue is full,");
        Trace trace("queue is full, discard msg, appUid=" + std::to_string(msg.appUid));
        return false;
    }

    if (CheckoutSystemAppUtil::CheckoutSystemApp(msg.appUid)) {
        Trace trace("skip system app dfx msg.., appuid=" + std::to_string(msg.appUid));
        AUDIO_WARNING_LOG("skip system app dfx msg.., appuid=%{public}d", msg.appUid);
        return false;
    }

    return msgQueue_.PushNoWait(msg);
}

void DfxMsgManager::HandleToHiSysEvent(DfxMessage &msg)
{
    if (reportedCnt_ >= MAX_DFX_REPORT_APP_COUNT) {
        AUDIO_WARNING_LOG("dfx report reach maximum size");
        return;
    }
    reportedCnt_++;
    auto dfxResult = std::make_unique<DfxReportResult>();
    Trace trace("renderInfoSize=" + std::to_string(msg.renderInfo.size()) +
        ", interruptInfoSize=" + std::to_string(msg.interruptInfo.size()) +
        ", captureInfosize=" + std::to_string(msg.captureInfo.size()));
    WriteRenderMsg(msg, dfxResult);
    WriteInterruptMsg(msg, dfxResult);
    WriteCapturerMsg(msg, dfxResult);
    WriteRunningAppMsg(msg, dfxResult);

    LogDfxResult(dfxResult);
    WritePlayAudioStatsEvent(dfxResult);
}

void DfxMsgManager::LogDfxResult(const std::unique_ptr<DfxReportResult> &result)
{
    CHECK_AND_RETURN_LOG(result != nullptr, "result is null");
    AUDIO_INFO_LOG("[HandleToHiSysEvent] appName=%{public}s", result->appName.c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] appVersion=%{public}s", result->appVersion.c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] summary=%{public}d", static_cast<int32_t>(result->summary));
    AUDIO_INFO_LOG("[HandleToHiSysEvent] rendererActions=%{public}s",
        DfxUtils::SerializeToJSONString(result->rendererActions).c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] renderInfo=%{public}s",
        DfxUtils::SerializeToJSONString(result->renderInfo).c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] renderTimestamp=%{public}s",
        DfxUtils::SerializeToJSONString(result->renderTimestamp).c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] rendererStats=%{public}s",
        DfxUtils::SerializeToJSONString(result->rendererStats).c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] interruptActions=%{public}s",
        DfxUtils::SerializeToJSONString(result->interruptActions).c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] interruptTimestamp=%{public}s",
        DfxUtils::SerializeToJSONString(result->interruptTimestamp).c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] interruptEffect=%{public}s",
        DfxUtils::SerializeToJSONString(result->interruptEffect).c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] interruptInfo=%{public}s",
        DfxUtils::SerializeToJSONString(result->interruptInfo).c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] capturerActions=%{public}s",
        DfxUtils::SerializeToJSONString(result->capturerActions).c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] capturerInfo=%{public}s",
        DfxUtils::SerializeToJSONString(result->capturerInfo).c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] capturerTimestamp=%{public}s",
        DfxUtils::SerializeToJSONString(result->capturerTimestamp).c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] capturerStat=%{public}s",
        DfxUtils::SerializeToJSONString(result->capturerStat).c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] appState=%{public}s",
        DfxUtils::SerializeToJSONString(result->appState).c_str());
    AUDIO_INFO_LOG("[HandleToHiSysEvent] appStateTimestamp=%{public}s",
        DfxUtils::SerializeToJSONString(result->appStateTimestamp).c_str());
}

void DfxMsgManager::WriteRenderMsg(DfxMessage &msg, const std::unique_ptr<DfxReportResult> &result)
{
    CHECK_AND_RETURN_LOG(result != nullptr, "result is null");
    std::vector<uint32_t> rendererActions{};
    std::vector<uint32_t> rendererInfos{};
    std::vector<uint64_t> timestamps{};
    std::vector<std::string> rendererStatVec{};

    UpdateAction(msg.appUid, msg.renderInfo);
    for (auto &item : msg.renderInfo) {
        auto rendererAction = DfxUtils::SerializeToUint32(item.rendererAction);

        rendererActions.push_back(static_cast<uint32_t>(rendererAction));
        timestamps.push_back(item.rendererAction.timestamp);

        if (static_cast<RendererStage>(item.rendererAction.fourthByte) == RendererStage::RENDERER_STAGE_STOP_OK ||
            static_cast<RendererStage>(item.rendererAction.fourthByte) == RENDERER_STAGE_STOP_BY_RELEASE) {
            auto rendererStat = DfxUtils::SerializeToJSONString(item.rendererStat);
            rendererStatVec.push_back(rendererStat);
        }

        if (static_cast<RendererStage>(item.rendererAction.fourthByte) == RendererStage::RENDERER_STAGE_START_OK ||
            static_cast<RendererStage>(item.rendererAction.fourthByte) == RendererStage::RENDERER_STAGE_START_FAIL) {
            auto rendererInfo = DfxUtils::SerializeToUint32(item.rendererInfo);
            rendererInfos.push_back(rendererInfo);
        }
    }

    result->rendererActions = std::move(rendererActions);
    result->renderTimestamp = std::move(timestamps);
    result->renderInfo = std::move(rendererInfos);
    result->rendererStats = std::move(rendererStatVec);
}

void DfxMsgManager::UpdateAction(int32_t appUid, std::list<RenderDfxInfo> &renderInfo)
{
    uint8_t &infoIdx = GetDfxIndexByType(appUid, DfxMsgIndexType::DFX_MSG_IDX_TYPE_RENDER_INFO);
    for (auto &item : renderInfo) {
        if (static_cast<RendererStage>(item.rendererAction.fourthByte) == RendererStage::RENDERER_STAGE_START_OK ||
            static_cast<RendererStage>(item.rendererAction.fourthByte) == RendererStage::RENDERER_STAGE_START_FAIL) {
            infoIdx++;
        }
        if (item.rendererAction.firstByte != 0 && item.rendererAction.firstByte < infoIdx) {
            item.rendererAction.firstByte = infoIdx;
        }
    }

    renderInfo.sort([](const auto &item1, const auto &item2) {
        return item1.rendererAction.timestamp < item2.rendererAction.timestamp;
    });
}

void DfxMsgManager::UpdateAction(int32_t appUid, std::list<CapturerDfxInfo> &capturerInfo)
{
    uint8_t &infoIdx = GetDfxIndexByType(appUid, DfxMsgIndexType::DFX_MSG_IDX_TYPE_CAPTURE_INFO);
    for (auto &item : capturerInfo) {
        if (static_cast<CapturerStage>(item.capturerAction.fourthByte) == CapturerStage::CAPTURER_STAGE_START_OK ||
            static_cast<CapturerStage>(item.capturerAction.fourthByte) == CapturerStage::CAPTURER_STAGE_START_FAIL) {
            infoIdx++;
        }
        if (item.capturerAction.firstByte != 0 && item.capturerAction.firstByte < infoIdx) {
            item.capturerAction.firstByte = infoIdx;
        }
    }

    capturerInfo.sort([](const auto &item1, const auto &item2) {
        return item1.capturerAction.timestamp < item2.capturerAction.timestamp;
    });
}

void DfxMsgManager::UpdateAction(int32_t appUid, std::list<InterruptDfxInfo> &interruptInfo)
{
    uint8_t &infoIdx = GetDfxIndexByType(appUid, DfxMsgIndexType::DFX_MSG_IDX_TYPE_INTERRUPT_INFO);
    uint8_t &effectIdx = GetDfxIndexByType(appUid, DfxMsgIndexType::DFX_MSG_IDX_TYPE_INTERRUPT_EFFECT);
    for (auto &item : interruptInfo) {
        if (!item.interruptEffectVec.empty()) {
            effectIdx++;
        }

        auto stage = static_cast<InterruptStage>(item.interruptAction.fourthByte);
        if (stage == InterruptStage::INTERRUPT_STAGE_START ||
            stage == InterruptStage::INTERRUPT_STAGE_RESTART) {
            infoIdx++;
        }

        if (item.interruptAction.firstByte != 0 && item.interruptAction.firstByte < infoIdx) {
            item.interruptAction.firstByte = infoIdx;
        }

        if (item.interruptAction.secondByte != 0 && item.interruptAction.secondByte < effectIdx) {
            item.interruptAction.secondByte = effectIdx;
        }
    }

    interruptInfo.sort([](const auto &item1, const auto &item2) {
        return item1.interruptAction.timestamp < item2.interruptAction.timestamp;
    });
}

uint8_t& DfxMsgManager::GetDfxIndexByType(int32_t appUid, DfxMsgIndexType type)
{
    auto iter = indexesInfo_.find(appUid);
    if (iter == indexesInfo_.end()) {
        indexesInfo_.insert({appUid, {0, 0, 0, 0}});
    }
    auto &indexes = indexesInfo_[appUid];
    if (type == DfxMsgIndexType::DFX_MSG_IDX_TYPE_RENDER_INFO) {
        return std::get<INDEXES_TUPLE_INDEX_RENDER_INFO>(indexes);
    } else if (type == DfxMsgIndexType::DFX_MSG_IDX_TYPE_CAPTURE_INFO) {
        return std::get<INDEXES_TUPLE_INDEX_CAPTURE_INFO>(indexes);
    } else if (type == DfxMsgIndexType::DFX_MSG_IDX_TYPE_INTERRUPT_INFO) {
        return std::get<INDEXES_TUPLE_INDEX_INTERRUPT_INFO>(indexes);
    } else if (type == DfxMsgIndexType::DFX_MSG_IDX_TYPE_INTERRUPT_EFFECT) {
        return std::get<INDEXES_TUPLE_INDEX_INTERRUPT_EFFECT>(indexes);
    }

    return std::get<INDEXES_TUPLE_INDEX_RENDER_INFO>(indexes);
}

void DfxMsgManager::WriteInterruptMsg(DfxMessage &msg, const std::unique_ptr<DfxReportResult> &result)
{
    CHECK_AND_RETURN_LOG(result != nullptr, "result is null");
    std::vector<uint32_t> interruptActions{};
    std::vector<uint64_t> timestamps{};
    std::vector<std::string> interruptEffectVec{};
    std::vector<uint32_t> interruptInfoVec{};

    uint8_t interruptOthersFlag = 0;
    uint8_t interruptedFlag = 0;

    UpdateAction(msg.appUid, msg.interruptInfo);
    for (auto &item : msg.interruptInfo) {
        auto interruptAction = DfxUtils::SerializeToUint32(item.interruptAction);
        interruptActions.push_back(interruptAction);
        timestamps.push_back(item.interruptAction.timestamp);
        auto stage = static_cast<InterruptStage>(item.interruptAction.fourthByte);
        if (!item.interruptEffectVec.empty()) {
            auto interruptEffect = DfxUtils::SerializeToJSONString(item.interruptEffectVec);
            interruptEffectVec.push_back(interruptEffect);
            interruptOthersFlag = 1;
        }
        interruptedFlag = CheckIsInterrupted(stage) ?  1 : interruptedFlag;

        if (stage == InterruptStage::INTERRUPT_STAGE_START ||
            stage == InterruptStage::INTERRUPT_STAGE_RESTART) {
            auto interruptInfo = DfxUtils::SerializeToUint32(item.interruptInfo);
            interruptInfoVec.push_back(interruptInfo);
        }
    }

    uint8_t interruptBackgroundFlag = 0;
    std::lock_guard<std::mutex> lock(appInfoMutex_);
    if (appInfo_.count(msg.appUid) != 0) {
        auto &item = appInfo_[msg.appUid];
        auto iter = std::find_if(item.appStateVec.begin(), item.appStateVec.end(), [](const auto &item) {
            return static_cast<DfxAppState>(item) == DFX_APP_STATE_BACKGROUND;
        });
        if (iter != item.appStateVec.end()) {
            interruptBackgroundFlag = 1;
        }
    }

    result->interruptActions = std::move(interruptActions);
    result->interruptTimestamp = std::move(timestamps);
    result->interruptInfo = std::move(interruptInfoVec);
    result->interruptEffect = std::move(interruptEffectVec);
    uint8_t summaryInt = (interruptBackgroundFlag << BIT_3_OFFSET) | (interruptedFlag << BIT_2_OFFSET) |
            interruptOthersFlag;
    result->summary = DfxUtils::SerializeToUint32({0, 0, 0, summaryInt});
}

void DfxMsgManager::WriteCapturerMsg(DfxMessage &msg, const std::unique_ptr<DfxReportResult> &result)
{
    CHECK_AND_RETURN_LOG(result != nullptr, "result is null");
    std::vector<uint32_t> capturerActions{};
    std::vector<uint32_t> capturerInfos{};
    std::vector<uint64_t> timestamps{};
    std::vector<std::string> capturerStatVec{};

    UpdateAction(msg.appUid, msg.captureInfo);
    for (auto &item : msg.captureInfo) {
        auto capturerAction = DfxUtils::SerializeToUint32(item.capturerAction);

        capturerActions.push_back(static_cast<uint32_t>(capturerAction));
        timestamps.push_back(item.capturerAction.timestamp);
        if (static_cast<CapturerStage>(item.capturerAction.fourthByte) == CapturerStage::CAPTURER_STAGE_STOP_OK ||
            static_cast<CapturerStage>(item.capturerAction.fourthByte) == CapturerStage::CAPTURER_STAGE_PAUSE_OK ||
            static_cast<CapturerStage>(item.capturerAction.fourthByte) == CAPTURER_STAGE_STOP_BY_RELEASE) {
            auto capturerStat = DfxUtils::SerializeToJSONString(item.capturerStat);
            capturerStatVec.push_back(capturerStat);
        }

        if (static_cast<CapturerStage>(item.capturerAction.fourthByte) == CapturerStage::CAPTURER_STAGE_START_OK ||
            static_cast<CapturerStage>(item.capturerAction.fourthByte) == CapturerStage::CAPTURER_STAGE_START_FAIL) {
            auto capturerInfo = DfxUtils::SerializeToUint32(item.capturerInfo);
            capturerInfos.push_back(capturerInfo);
        }
    }

    result->capturerActions = std::move(capturerActions);
    result->capturerTimestamp = std::move(timestamps);
    result->capturerInfo = std::move(capturerInfos);
    result->capturerStat = std::move(capturerStatVec);
}

void DfxMsgManager::WriteRunningAppMsg(DfxMessage &msg, const std::unique_ptr<DfxReportResult> &result)
{
    CHECK_AND_RETURN_LOG(result != nullptr, "result is null");
    std::lock_guard<std::mutex> lock(appInfoMutex_);
    if (appInfo_.count(msg.appUid) == 0) {
        AUDIO_ERR_LOG("unknown appUid=%{public}d", msg.appUid);
        return;
    }

    auto &item = appInfo_[msg.appUid];
    result->appName = item.appName;
    result->appVersion = item.versionName;
    for (auto item : item.appStateVec) {
        result->appState.push_back(item);
    }

    for (auto item : item.appStateTimeStampVec) {
        result->appStateTimestamp.push_back(item);
    }

    item.appStateVec.clear();
    item.appStateTimeStampVec.clear();
}

void DfxMsgManager::WritePlayAudioStatsEvent(const std::unique_ptr<DfxReportResult> &result)
{
    CHECK_AND_RETURN_LOG(result != nullptr, "result is null");
    auto ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "PLAY_AUDIO_STATS",
        HiviewDFX::HiSysEvent::EventType::STATISTIC,
        "APP_NAME", result->appName,
        "APP_VERSION", result->appVersion,
        "INTERRUPT_ACTION", result->interruptActions,
        "INTERRUPT_TIMESTAMP", result->interruptTimestamp,
        "INTERRUPT_INFO", result->interruptInfo,
        "INTERRUPT_EFFECT", result->interruptEffect,
        "RENDERER_ACTION", result->rendererActions,
        "RENDERER_TIMESTAMP", result->renderTimestamp,
        "RENDERER_INFO", result->renderInfo,
        "RENDERER_STATS", result->rendererStats,
        "RECORDER_ACTION", result->capturerActions,
        "RECORDER_TIMESTAMP", result->capturerTimestamp,
        "RECORDER_INFO", result->capturerInfo,
        "RECORDER_STATS", result->capturerStat,
        "APP_STATE", result->appState,
        "APP_STATE_TIMESTAMP", result->appStateTimestamp,
        "SUMMARY", result->summary);
    if (ret) {
        AUDIO_ERR_LOG("write event fail: PLAY_AUDIO_STATS, ret = %{public}d", ret);
    }
}

bool DfxMsgManager::CheckCanAddAppInfo(int32_t appUid)
{
    std::lock_guard<std::mutex> lock(appInfoMutex_);
    bool ret = false;
    if (CheckoutSystemAppUtil::CheckoutSystemApp(appUid)) {
        Trace trace("skip system app dfx msg.., appuid=" + std::to_string(appUid));
        AUDIO_WARNING_LOG("skip system app dfx msg.., appuid=%{public}d", appUid);
        return ret;
    }

    ret = appInfo_.count(appUid) == 0;
    return ret;
}

void DfxMsgManager::SaveAppInfo(const DfxRunningAppInfo info)
{
    std::lock_guard<std::mutex> lock(appInfoMutex_);
    if (appInfo_.count(info.appUid) == 0) {
        appInfo_.insert(std::make_pair(info.appUid, info));
    }
}

void DfxMsgManager::UpdateAppState(int32_t appUid, DfxAppState appState, bool forceUpdate)
{
    std::lock_guard<std::mutex> lock(appInfoMutex_);
    if (appInfo_.count(appUid) != 0) {
        auto &item = appInfo_[appUid];
        DfxAppState recentAppState = !item.appStateVec.empty() ?
            static_cast<DfxAppState>(item.appStateVec.back()) : DFX_APP_STATE_UNKNOWN;
        if (!forceUpdate && recentAppState == DFX_APP_STATE_START) {
            AUDIO_WARNING_LOG("discard unstarted audio stream app state");
            return;
        }
        if (recentAppState == appState) {
            AUDIO_WARNING_LOG("discard repeated app state");
            return;
        }
        DfxStatAction dfxAppState = {appState, 0, 0, 0};
        item.appStateVec.push_back(static_cast<uint8_t>(appState));
        item.appStateTimeStampVec.push_back(dfxAppState.timestamp);
    }
}

bool DfxMsgManager::CheckIsInterrupted(InterruptStage stage)
{
    bool ret = (stage == INTERRUPT_STAGE_STOPPED ||
        stage == INTERRUPT_STAGE_DUCK_BEGIN ||
        stage == INTERRUPT_STAGE_PAUSED ||
        stage == INTERRUPT_STAGE_RESUMED ||
        stage == INTERRUPT_STAGE_DUCK_END);
    return ret;
}
} // namespace AudioStandard
} // namespace OHOS
