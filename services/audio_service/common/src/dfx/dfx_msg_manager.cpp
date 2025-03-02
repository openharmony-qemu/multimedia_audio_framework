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
static constexpr int32_t DFX_QUEUE_CHECK_INTERVAL_MIN = 60;
static constexpr int32_t MAX_DFX_REPORT_APP_COUNT = 20;

DfxMsgManager& DfxMsgManager::GetInstance()
{
    static DfxMsgManager instance;
    return instance;
}

DfxMsgManager::DfxMsgManager() : msgQueue_(DFX_MSG_QUEUE_CAPACITY)
{
}

void DfxMsgManager::Init()
{
    lastReportTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    timeThread_ = std::make_unique<std::thread>(&DfxMsgManager::TimeFunc, this);
    pthread_setname_np(timeThread_->native_handle(), "AudioServerDFXTiming");
}

DfxMsgManager::~DfxMsgManager()
{
    AUDIO_INFO_LOG("DfxMsgManager deconstructor");
    startThread_.store(false, std::memory_order_release);
}

void DfxMsgManager::TimeFunc()
{
    while (startThread_.load(std::memory_order_acquire)) {
        std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

        DfxMessage msg;
        while (!isFull_ && msgQueue_.PopNotWait(msg)) {
            if (!ProcessCheck(msg)) {
                continue;
            } else {
                Process(msg);
            }
        }
        auto interval = std::chrono::system_clock::from_time_t(now - lastReportTime_).time_since_epoch();
        int intervalMin = std::chrono::duration_cast<std::chrono::minutes>(interval).count();
        if (intervalMin >= DEFAULT_DFX_REPORT_INTERVAL_MIN) {
            AUDIO_INFO_LOG("time is up, report msg size=%{public}d,", reportQueue_.size());
            for (const auto &item : reportQueue_) {
                HandleToHiSysEvent(item.second);
            }
            reportQueue_.clear();
            isFull_ = false;
            lastReportTime_ = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        }
        msgQueue_.WaitNotEmptyFor(std::chrono::minutes(DFX_QUEUE_CHECK_INTERVAL_MIN));
    }
}

bool DfxMsgManager::ProcessCheck(const DfxMessage& msg)
{
    if (msg.renderInfo.size() >= MAX_DFX_MSG_MEMBER_SIZE ||
        msg.interruptInfo.size() >= MAX_DFX_MSG_MEMBER_SIZE ||
        msg.captureInfo.size() >= MAX_DFX_MSG_MEMBER_SIZE) {
        AUDIO_INFO_LOG("size exceed maximum, renderInfo=%{public}d, " \
            "interruptInfo=%{public}d, captureInfo=%{public}d,",
            msg.renderInfo.size(), msg.interruptInfo.size(), msg.captureInfo.size());
        return false;
    }

    if (!isFull_ && reportQueue_.size() == MAX_DFX_REPORT_APP_COUNT) {
        auto lower = reportQueue_.lower_bound(msg.appUid);
        auto upper = reportQueue_.upper_bound(msg.appUid);
        if (lower == upper) {
            AUDIO_INFO_LOG("dfx report reach maximum size, discard msg.appUid=%{public}d", msg.appUid);
            return false;
        }

        auto lastIt = --upper;
        int renderVacancy = MAX_DFX_MSG_MEMBER_SIZE - lastIt->second.renderInfo.size();
        renderVacancy = std::max(renderVacancy, 0);
        int interruptVacancy = MAX_DFX_MSG_MEMBER_SIZE - lastIt->second.interruptInfo.size();
        interruptVacancy = std::max(renderVacancy, 0);
        int capturerVacancy = MAX_DFX_MSG_MEMBER_SIZE - lastIt->second.captureInfo.size();
        capturerVacancy = std::max(renderVacancy, 0);
        if ((renderVacancy == 0 && interruptVacancy == 0) ||
            (capturerVacancy == 0 && interruptVacancy == 0)) {
            isFull_ = true;
        }
    }

    if (isFull_) {
        AUDIO_INFO_LOG("dfx report reach maximum size, discard msg.appUid=%{public}d", msg.appUid);
        return false;
    }
    return true;
}

bool DfxMsgManager::Process(DfxMessage& msg)
{
    bool ret = true;
    if (reportQueue_.count(msg.appUid) == 0) {
        reportQueue_.insert(std::make_pair(msg.appUid, msg));
        return ret;
    }

    bool processed = false;
    auto range = reportQueue_.equal_range(msg.appUid);
    for (auto it = range.first; it != range.second; ++it) {
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

void DfxMsgManager::InsertReportQueue(const DfxMessage& msg)
{
    if (reportQueue_.size() == MAX_DFX_REPORT_APP_COUNT) {
        return;
    }
    reportQueue_.insert(std::make_pair(msg.appUid, msg));
}

bool DfxMsgManager::ProcessInner(uint32_t index,
    std::list<RenderDfxInfo> &dfxInfo, std::list<RenderDfxInfo> &curDfxInfo)
{
    bool processed = false;
    auto size = dfxInfo.size();
    if (size != 0) {
        processed = true;
        int vacancy = MAX_DFX_MSG_MEMBER_SIZE - curDfxInfo.size();
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

bool DfxMsgManager::ProcessInner(uint32_t index,
    std::list<InterruptDfxInfo> &dfxInfo, std::list<InterruptDfxInfo> &curDfxInfo)
{
    bool processed = false;
    auto size = dfxInfo.size();
    if (size != 0) {
        processed = true;
        int vacancy = MAX_DFX_MSG_MEMBER_SIZE - curDfxInfo.size();
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

bool DfxMsgManager::ProcessInner(uint32_t index,
    std::list<CapturerDfxInfo> &dfxInfo, std::list<CapturerDfxInfo> &curDfxInfo)
{
    bool processed = false;
    auto size = dfxInfo.size();
    if (size != 0) {
        processed = true;
        int vacancy = MAX_DFX_MSG_MEMBER_SIZE - curDfxInfo.size();
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
        return false;
    }

    return msgQueue_.PushNoWait(msg);
}

void DfxMsgManager::HandleToHiSysEvent(const DfxMessage &msg)
{
    std::shared_ptr<Media::MediaMonitor::EventBean> bean = std::make_shared<Media::MediaMonitor::EventBean>(
        Media::MediaMonitor::ModuleId::AUDIO, Media::MediaMonitor::EventId::UNKNOW_EVENTID,
        Media::MediaMonitor::EventType::BEHAVIOR_EVENT);
    
    WriteRenderMsg(msg, bean);
    WriteInterruptMsg(msg, bean);
    WriteCapturerMsg(msg, bean);

    auto logMsgInt = bean->GetIntMap();
    for (auto item : logMsgInt) {
        AUDIO_INFO_LOG("[HandleToHiSysEvent] logMsgInt=%{public}s===>%{public}d",
            item.first.c_str(), item.second);
    }
    auto logMsgStr = bean->GetStringMap();
    for (auto item : logMsgStr) {
        AUDIO_INFO_LOG("[HandleToHiSysEvent] logMsgStr=%{public}s===>%{public}s",
            item.first.c_str(), item.second.c_str());
    }

    WritePlayAudioStatsEvent(bean);
}

void DfxMsgManager::WriteRenderMsg(const DfxMessage &msg, std::shared_ptr<Media::MediaMonitor::EventBean> &bean)
{
    std::vector<std::string> rendererActions{};
    std::vector<std::string> rendererInfos{};
    std::vector<std::string> timestamps{};
    std::vector<std::string> rendererStatVec{};

    for (auto &item : msg.renderInfo) {
        auto rendererAction = DfxUtils::SerializeToUint32(item.rendererAction);

        rendererActions.push_back(std::to_string(rendererAction));
        timestamps.push_back(std::to_string(item.rendererAction.timestamp));

        if (static_cast<RendererStage>(item.rendererAction.fourthByte) == RendererStage::RENDERER_STAGE_STOP_OK) {
            auto rendererStat = DfxUtils::SerializeToJSONString(item.rendererStat);
            rendererStatVec.push_back(rendererStat);
        }

        if (static_cast<RendererStage>(item.rendererAction.fourthByte) == RendererStage::RENDERER_STAGE_START_OK ||
            static_cast<RendererStage>(item.rendererAction.fourthByte) == RendererStage::RENDERER_STAGE_START_FAIL) {
            auto rendererInfo = DfxUtils::SerializeToUint32(item.rendererInfo);
            rendererInfos.push_back(std::to_string(rendererInfo));
        }
    }
    auto rendererAction = DfxUtils::SerializeToJSONString(rendererActions);
    auto rendererTimestamp = DfxUtils::SerializeToJSONString(timestamps);
    auto rendererInfoStr = DfxUtils::SerializeToJSONString(rendererInfos);
    auto rendererStatStr = DfxUtils::SerializeToJSONString(rendererStatVec);

    if (bundleInfo_.count(msg.appUid) != 0) {
        auto info = bundleInfo_[msg.appUid];
        bean->Add("APP_NAME", info.appName);
        bean->Add("APP_VERSION", std::to_string(info.versionCode));
    }

    bean->Add("RENDERER_ACTION", rendererAction);
    bean->Add("RENDERER_TIMESTAMP", rendererTimestamp);
    bean->Add("RENDERER_INFO", rendererInfoStr);
    bean->Add("RENDERER_STATS", rendererStatStr);
}

void DfxMsgManager::WriteInterruptMsg(const DfxMessage &msg, std::shared_ptr<Media::MediaMonitor::EventBean> &bean)
{
    std::vector<std::string> interruptActions{};
    std::vector<std::string> timestamps{};
    std::vector<std::string> interruptEffectVec{};
    std::vector<std::string> interruptInfoVec{};
    std::vector<std::string> appStateVec{};
    std::vector<std::string> appStateTimestampVec{};

    uint8_t interruptOthersFlag = 0;
    uint8_t interruptedFlag = 0;
    uint8_t interruptBackgroundFlag = 0;
    for (auto &item : msg.interruptInfo) {
        auto interruptAction = DfxUtils::SerializeToUint32(item.interruptAction);
        interruptActions.push_back(std::to_string(interruptAction));
        timestamps.push_back(std::to_string(item.interruptAction.timestamp));
        auto stage = static_cast<InterruptStage>(item.interruptAction.fourthByte);
        if (!item.interruptEffectVec.empty()) {
            auto interruptEffect = DfxUtils::SerializeToJSONString(item.interruptEffectVec);
            interruptEffectVec.push_back(interruptEffect);
            interruptOthersFlag = 1;
        }
        interruptedFlag = stage == InterruptStage::INTERRUPT_STAGE_STOPPED ?  1 : interruptedFlag;

        if (stage == InterruptStage::INTERRUPT_STAGE_START ||
            stage == InterruptStage::INTERRUPT_STAGE_RESTART) {
            auto interruptInfo = DfxUtils::SerializeToUint32(item.interruptInfo);
            interruptInfoVec.push_back(std::to_string(interruptInfo));
        }

        std::for_each(item.appStateVec.begin(), item.appStateVec.end(),
            [&interruptBackgroundFlag, &appStateVec, &appStateTimestampVec](const auto& item) {
                interruptBackgroundFlag = item.firstByte == INTERRUPT_APP_STATE_BACKGROUND ?
                    1 : interruptBackgroundFlag;
                appStateVec.push_back(std::to_string(item.firstByte));
                appStateTimestampVec.push_back(std::to_string(item.timestamp));
        });
    }

    auto interruptActionStr = DfxUtils::SerializeToJSONString(interruptActions);
    auto interruptTimestampStr = DfxUtils::SerializeToJSONString(timestamps);
    auto interruptinfoStr = DfxUtils::SerializeToJSONString(interruptInfoVec);
    auto interruptEffectStr = DfxUtils::SerializeToJSONString(interruptEffectVec);
    auto appStateStr = DfxUtils::SerializeToJSONString(appStateVec);
    auto appStateTimestampStr = DfxUtils::SerializeToJSONString(appStateTimestampVec);

    bean->Add("INTERRUPT_ACTION", interruptActionStr);
    bean->Add("INTERRUPT_TIMESTAMP", interruptTimestampStr);
    bean->Add("INTERRUPT_INFO", interruptinfoStr);
    bean->Add("INTERRUPT_EFFECT", interruptEffectStr);
    bean->Add("APP_STATE", appStateStr);
    bean->Add("APP_STATE_TIMESTAMP", appStateTimestampStr);

    DfxStatInt32 summary{interruptOthersFlag, interruptedFlag, interruptBackgroundFlag, 0};
    auto summaryInt = DfxUtils::SerializeToUint32(summary);
    bean->Add("SUMMARY", static_cast<int32_t>(summaryInt));
}

void DfxMsgManager::WriteCapturerMsg(const DfxMessage &msg, std::shared_ptr<Media::MediaMonitor::EventBean> &bean)
{
    std::vector<std::string> capturerActions{};
    std::vector<std::string> capturerInfos{};
    std::vector<std::string> timestamps{};
    std::vector<std::string> capturerStatVec{};

    for (auto &item : msg.captureInfo) {
        auto capturerAction = DfxUtils::SerializeToUint32(item.capturerAction);

        capturerActions.push_back(std::to_string(capturerAction));
        timestamps.push_back(std::to_string(item.capturerAction.timestamp));

        if (static_cast<CapturerStage>(item.capturerAction.fourthByte) == CapturerStage::CAPTURER_STAGE_STOP_OK) {
            auto capturerStat = DfxUtils::SerializeToJSONString(item.capturerStat);
            capturerStatVec.push_back(capturerStat);
        }

        if (static_cast<CapturerStage>(item.capturerAction.fourthByte) == CapturerStage::CAPTURER_STAGE_START_OK ||
            static_cast<CapturerStage>(item.capturerAction.fourthByte) == CapturerStage::CAPTURER_STAGE_START_FAIL) {
            auto capturerInfo = DfxUtils::SerializeToUint32(item.capturerInfo);
            capturerInfos.push_back(std::to_string(capturerInfo));
        }
    }
    auto capturerAction = DfxUtils::SerializeToJSONString(capturerActions);
    auto capturerTimestamp = DfxUtils::SerializeToJSONString(timestamps);
    auto capturerInfoStr = DfxUtils::SerializeToJSONString(capturerInfos);
    auto capturerStatStr = DfxUtils::SerializeToJSONString(capturerStatVec);

    bean->Add("CLIENT_UID", static_cast<int32_t>(msg.appUid));
    bean->Add("CAPTURER_ACTION", capturerAction);
    bean->Add("CAPTURER_TIMESTAMP", capturerTimestamp);
    bean->Add("CAPTURER_INFO", capturerInfoStr);
    bean->Add("CAPTURER_STATS", capturerStatStr);
}

void DfxMsgManager::WritePlayAudioStatsEvent(std::shared_ptr<Media::MediaMonitor::EventBean> &bean)
{
    if (bean == nullptr) {
        AUDIO_ERR_LOG("eventBean is nullptr");
        return;
    }
    auto ret = HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::AUDIO, "PLAY_AUDIO_STATS",
        HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
        "APP_NAME", bean->GetStringValue("APP_NAME"),
        "APP_VERSION", bean->GetStringValue("APP_VERSION"),
        "INTERRUPT_ACTION", bean->GetStringValue("INTERRUPT_ACTION"),
        "INTERRUPT_TIMESTAMP", bean->GetStringValue("INTERRUPT_TIMESTAMP"),
        "INTERRUPT_INFO", bean->GetStringValue("INTERRUPT_INFO"),
        "INTERRUPT_EFFECT", bean->GetStringValue("INTERRUPT_EFFECT"),
        "RENDERER_ACTION", bean->GetStringValue("RENDERER_ACTION"),
        "RENDERER_TIMESTAMP", bean->GetStringValue("RENDERER_TIMESTAMP"),
        "RENDERER_INFO", bean->GetStringValue("RENDERER_INFO"),
        "RENDERER_STATS", bean->GetStringValue("RENDERER_STATS"),
        "RECORDER_ACTION", bean->GetStringValue("RECORDER_ACTION"),
        "RECORDER_TIMESTAMP", bean->GetStringValue("RECORDER_TIMESTAMP"),
        "RECORDER_INFO", bean->GetStringValue("RECORDER_INFO"),
        "RECORDER_STATS", bean->GetStringValue("RECORDER_STATS"),
        "APP_STATE", bean->GetStringValue("APP_STATE"),
        "APP_STATE_TIMESTAMP", bean->GetStringValue("APP_STATE_TIMESTAMP"),
        "SUMMARY", bean->GetIntValue("SUMMARY")
        );
    if (ret) {
        AUDIO_ERR_LOG("write event fail: PLAY_AUDIO_STATS, ret = %{public}d", ret);
    }
}

bool DfxMsgManager::HasAppInfo(uint32_t appUid)
{
    return bundleInfo_.count(appUid) == 0;
}

void DfxMsgManager::SaveAppInfo(const DfxBundleInfo info)
{
    if (bundleInfo_.count(info.appUid) == 0) {
        bundleInfo_.insert(std::make_pair(info.appUid, info));
    }
}

} // namespace AudioStandard
} // namespace OHOS
