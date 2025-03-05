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

#include "dfx_stat.h"
#include "dfx_utils.h"
#include "audio_info.h"
#include "audio_utils.h"
#include "event_bean.h"
#include "audio_safe_block_queue.h"

namespace OHOS {
namespace AudioStandard {

struct DfxMessage {
    uint32_t appUid{};
    std::list<RenderDfxInfo> renderInfo{};
    std::list<InterruptDfxInfo> interruptInfo{};
    std::list<CapturerDfxInfo> captureInfo{};
    bool ready = false;
};

class DfxMsgManager {
public:
    static DfxMsgManager& GetInstance();
    bool Enqueue(const DfxMessage &msg);
    bool HasAppInfo(uint32_t appUid);
    void Init();
    void SaveAppInfo(const DfxBundleInfo info);
private:
    DfxMsgManager();
    virtual ~DfxMsgManager();
    void TimeFunc();
    bool ProcessCheck(const DfxMessage& msg);
    bool Process(DfxMessage& msg);

    void InsertReportQueue(const DfxMessage& msg);
    bool ProcessInner(uint32_t index, std::list<RenderDfxInfo> &dfxInfo, std::list<RenderDfxInfo> &curDfxInfo);
    bool ProcessInner(uint32_t index, std::list<InterruptDfxInfo> &dfxInfo, std::list<InterruptDfxInfo> &curDfxInfo);
    bool ProcessInner(uint32_t index, std::list<CapturerDfxInfo> &dfxInfo, std::list<CapturerDfxInfo> &curDfxInfo);

    void WriteInterruptMsg(const DfxMessage &msg, std::shared_ptr<Media::MediaMonitor::EventBean> &bean);
    void WriteRenderMsg(const DfxMessage &msg, std::shared_ptr<Media::MediaMonitor::EventBean> &bean);
    void WriteCapturerMsg(const DfxMessage &msg, std::shared_ptr<Media::MediaMonitor::EventBean> &bean);
    void HandleToHiSysEvent(const DfxMessage &msg);
    void WritePlayAudioStatsEvent(std::shared_ptr<Media::MediaMonitor::EventBean> &bean);

    AudioSafeBlockQueue<DfxMessage> msgQueue_;
    std::multimap<uint32_t, DfxMessage> reportQueue_;
    std::unique_ptr<std::thread> timeThread_ = nullptr;
    std::atomic_bool startThread_ = true;
    std::time_t lastReportTime_{};
    std::map<uint32_t, DfxBundleInfo> bundleInfo_;
    std::atomic_bool isFull_ = false;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // ST_DFX_MESSAGE_MGR_H