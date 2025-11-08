/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioSchedule"
#endif

#include "audio_schedule.h"
#include "audio_schedule_guard.h"

#include <unistd.h>
#include <sys/types.h>
#include <cstring>
#include <unordered_map>
#include <set>

#ifdef RESSCHE_ENABLE
#include "res_type.h"
#include "res_sched_client.h"
#endif
#include "qos.h"
#include "concurrent_task_client.h"

#include "audio_utils.h"
#include "audio_common_log.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace OHOS::AudioStandard;

#ifdef RESSCHE_ENABLE
const uint32_t AUDIO_QOS_LEVEL = 7;
const int32_t DEFAULT_QOS_LEVEL = -1;
const uint32_t REPORTDATA_TIMEOUT = 8;
static std::mutex g_rssMutex;
static std::set<uint32_t> g_tidToReport = {};
constexpr uint32_t g_type = OHOS::ResourceSchedule::ResType::RES_TYPE_THREAD_QOS_CHANGE;
constexpr int64_t g_value = 0;
constexpr int32_t AUDIO_PROC_QOS_TABLE = 7;

void ConfigPayload(pid_t pid, pid_t tid, const char *bundleName, int32_t qosLevel,
    std::unordered_map<std::string, std::string> &mapPayload)
{
    std::string strBundleName = bundleName;
    std::string strPid = std::to_string(pid);
    std::string strTid = std::to_string(tid);
    std::string strQos = std::to_string(qosLevel);
    mapPayload["pid"] = strPid;
    mapPayload[strTid] = strQos;
    mapPayload["bundleName"] = strBundleName;
}

static void ScheduleReportDataInner(pid_t pid, pid_t tid, const char *bundleName, int32_t qosLevel)
{
    std::unordered_map<std::string, std::string> mapPayload;
    ConfigPayload(pid, tid, bundleName, qosLevel, mapPayload);
    OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(g_type, g_value, mapPayload);
}

void ScheduleReportData(pid_t pid, pid_t tid, const char *bundleName)
{
    AudioXCollie audioXcollie("RSS::ReportData with qos level 7, pid " + std::to_string(pid) +
        ", tid " + std::to_string(tid), REPORTDATA_TIMEOUT,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    Trace trace ("Rss::ReportData with qos level 7");
    AUDIO_INFO_LOG("Report tid %{public}u", tid);
    ScheduleReportDataInner(pid, tid, bundleName, AUDIO_QOS_LEVEL);
}

void ScheduleReportDataWithQosLevel(pid_t pid, pid_t tid, const char *bundleName, int32_t qosLevel)
{
    AudioXCollie audioXcollie("RSS::ReportData with qos level " + std::to_string(qosLevel) +
        ", pid " + std::to_string(pid) + ", tid " + std::to_string(tid), REPORTDATA_TIMEOUT,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    AUDIO_INFO_LOG("Report tid %{public}u to qosLevel %{public}d", tid, qosLevel);
    ScheduleReportDataInner(pid, tid, bundleName, qosLevel);
}

void UnscheduleReportData(pid_t pid, pid_t tid, const char* bundleName)
{
    AudioXCollie audioXcollie("RSS::ReportData with qos level -1, pid " + std::to_string(pid) +
        ", tid " + std::to_string(tid), REPORTDATA_TIMEOUT,
         nullptr, nullptr, AUDIO_XCOLLIE_FLAG_LOG | AUDIO_XCOLLIE_FLAG_RECOVERY);
    Trace trace ("Rss::ReportData with qos level -1");
    std::unordered_map<std::string, std::string> mapPayload;
    ConfigPayload(pid, tid, bundleName, DEFAULT_QOS_LEVEL, mapPayload);
    OHOS::ResourceSchedule::ResSchedClient::GetInstance().ReportData(g_type, g_value, mapPayload);
}

void UnscheduleThreadInServer(pid_t pid, pid_t tid)
{
    std::lock_guard<std::mutex> lock(g_rssMutex);
    if (g_tidToReport.find(tid) != g_tidToReport.end()) {
        AUDIO_INFO_LOG("Remove tid in server %{public}u", tid);
        g_tidToReport.erase(tid);
    }
    UnscheduleReportData(pid, tid, "audio_server");
}

void ScheduleThreadInServer(pid_t pid, pid_t tid)
{
    std::lock_guard<std::mutex> lock(g_rssMutex);
    if (g_tidToReport.find(tid) == g_tidToReport.end()) {
        AUDIO_INFO_LOG("Add tid in server %{public}u", tid);
        g_tidToReport.insert(tid);
    }
    ScheduleReportData(pid, tid, "audio_server");
}

void SetProcessDataThreadPriority(int32_t priority)
{
    struct sched_param param = {0};
    // setPriority = 50 + priority
    param.sched_priority = priority;
    int32_t res = sched_setscheduler(0, SCHED_FIFO | SCHED_RESET_ON_FORK, &param);
    if (res != 0) {
        AUDIO_ERR_LOG("Set thread 50 + %{public}d priority fail : %{public}d", param.sched_priority, res);
        return;
    }
    AUDIO_INFO_LOG("Set thread 50 + %{public}d priority success", param.sched_priority);
    return;
}

void ResetProcessDataThreadPriority()
{
    struct sched_param param = {0};
    param.sched_priority = 0;
    int32_t res = sched_setscheduler(0, SCHED_OTHER, &param);
    if (res != 0) {
        AUDIO_ERR_LOG("Reset thread priority fail : %{public}d", res);
        return;
    }
    AUDIO_INFO_LOG("Reset thread priority success");
    return;
}

void OnAddResSchedService(uint32_t audioServerPid)
{
    std::lock_guard<std::mutex> lock(g_rssMutex);
    for (auto tid : g_tidToReport) {
        AUDIO_INFO_LOG("On add rss, report %{public}u", tid);
        ScheduleReportData(audioServerPid, tid, "audio_server");
    }
}

bool SetEndpointThreadPriority()
{
    Trace trace("SetEndpointThreadPriority");
    bool res = false;
    std::unordered_map<std::string, std::string> payload;
    payload["groupId"] = std::to_string(AUDIO_PROC_QOS_TABLE);
    payload["pid"] = std::to_string(getpid());
    OHOS::ConcurrentTask::ConcurrentTaskClient::GetInstance().RequestAuth(payload);
    int32_t ret = OHOS::QOS::SetThreadQos(OHOS::QOS::QosLevel::QOS_KEY_BACKGROUND);
    if (ret == 0) {
        res = true;
    }
    AUDIO_INFO_LOG("set thread qos %{public}s", ret ? "failed" : "success");
    return res;
}

bool ResetEndpointThreadPriority()
{
    struct sched_param param = {0};
    param.sched_priority = 0;
    auto res = sched_setscheduler(0, SCHED_OTHER, &param);
    if (res != 0) {
        AUDIO_ERR_LOG("Reset thread priority fail : %{public}d", res);
        return false;
    }
    AUDIO_INFO_LOG("Reset thread priority success");
    return true;
};
#else
void ScheduleReportData(uint32_t /* pid */, uint32_t /* tid */, const char* /* bundleName*/) {};
void ScheduleThreadInServer(pid_t pid, pid_t tid) {};
void UnscheduleThreadInServer(pid_t tid) {};
void OnAddResSchedService(uint32_t audioServerPid) {};
void SetProcessDataThreadPriority(int32_t priority) {};
void ResetProcessDataThreadPriority() {};
void UnscheduleReportData(uint32_t /* pid */, uint32_t /* tid */, const char* /* bundleName*/) {};
bool SetEndpointThreadPriority() { return false; };
bool ResetEndpointThreadPriority() { return false; };
#endif

#ifdef __cplusplus
}
#endif

namespace OHOS {
namespace AudioStandard {
namespace {
static constexpr unsigned int WAIT_TIMEOUT_SECONDS = 5;
}
std::map<std::pair<pid_t, pid_t>,
    std::weak_ptr<SharedAudioScheduleGuard>> SharedAudioScheduleGuard::guardMap_;
std::mutex SharedAudioScheduleGuard::mutex_;
std::condition_variable SharedAudioScheduleGuard::cv_;

AudioScheduleGuard::AudioScheduleGuard(pid_t pid, pid_t tid, const std::string &bundleName)
    : pid_(pid), tid_(tid), bundleName_(bundleName)
{
    ScheduleReportData(pid, tid, bundleName.c_str());
    isReported_ = true;
}

AudioScheduleGuard::AudioScheduleGuard(AudioScheduleGuard&& audioScheduleGuard)
    : pid_(audioScheduleGuard.pid_), tid_(audioScheduleGuard.tid_),
    bundleName_(std::move(audioScheduleGuard.bundleName_)), isReported_(audioScheduleGuard.isReported_)
{
    audioScheduleGuard.isReported_ = false;
}

bool AudioScheduleGuard::operator==(const AudioScheduleGuard&) const = default;

AudioScheduleGuard::~AudioScheduleGuard()
{
    if (isReported_) {
        UnscheduleReportData(pid_, tid_, bundleName_.c_str());
    }
}

std::shared_ptr<SharedAudioScheduleGuard> SharedAudioScheduleGuard::Create(pid_t pid, pid_t tid,
    const std::string &bundleName)
{
    std::shared_ptr<SharedAudioScheduleGuard> sharedGuard = nullptr;
    std::unique_lock lock(mutex_);
    bool isTimeout = !cv_.wait_for(lock, std::chrono::seconds(WAIT_TIMEOUT_SECONDS), [pid, tid, &sharedGuard] () {
        if (guardMap_.contains({pid, tid})) {
            sharedGuard = guardMap_.at({pid, tid}).lock();
            if (sharedGuard != nullptr) {
                return true;
            }
            AUDIO_INFO_LOG("wait");
            // if contains but sharedGuard is null, wait last object destroy.
            return false;
        } else {
            return true;
        }
    });
    CHECK_AND_RETURN_RET_LOG(!isTimeout, nullptr, "timeout");

    if (sharedGuard) {
        AUDIO_INFO_LOG("ret exist obj");
        return sharedGuard;
    }

    if (!guardMap_.contains({pid, tid})) {
        sharedGuard = std::make_shared<SharedAudioScheduleGuard>(pid, tid, bundleName);
        CHECK_AND_RETURN_RET_LOG(sharedGuard, nullptr, "no mem");
        guardMap_.insert({{pid, tid}, sharedGuard});
        return sharedGuard;
    }

    AUDIO_ERR_LOG("unknow err");
    return nullptr;
}

SharedAudioScheduleGuard::~SharedAudioScheduleGuard()
{
    std::lock_guard lock(mutex_);
    // unreport must guard by mutex
    AudioScheduleGuard tempGuard(std::move(guard_));
    guardMap_.erase({pid_, tid_});
    cv_.notify_all();
}
} // namespace AudioStandard
} // namespace OHOS
