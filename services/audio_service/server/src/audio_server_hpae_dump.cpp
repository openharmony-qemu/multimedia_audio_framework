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
#define LOG_TAG "AudioServerHpaeDump"
#endif

#include "audio_server_hpae_dump.h"
#include <sstream>
#include "audio_utils.h"
#include "audio_errors.h"
#include "audio_service.h"
#include "audio_dump_pcm.h"
#include "audio_performance_monitor.h"
#include "manager/hdi_adapter_manager.h"
#include "i_hpae_manager.h"

using namespace std;
using namespace OHOS::AudioStandard::HPAE;
namespace OHOS {
namespace AudioStandard {
static const int32_t OPERATION_TIMEOUT_IN_MS = 1000;  // 1000ms

AudioServerHpaeDump::AudioServerHpaeDump()
{
    AUDIO_DEBUG_LOG("AudioServerHpaeDump construct");
    InitDumpFuncMap();
}

AudioServerHpaeDump::~AudioServerHpaeDump()
{}

void AudioServerHpaeDump::InitDumpFuncMap()
{
    dumpFuncMap[u"-h"] = &AudioServerHpaeDump::HelpInfoDump;
    dumpFuncMap[u"-p"] = &AudioServerHpaeDump::PlaybackSinkDump;
    dumpFuncMap[u"-r"] = &AudioServerHpaeDump::RecordSourceDump;
    dumpFuncMap[u"-m"] = &AudioServerHpaeDump::HDFModulesDump;
    dumpFuncMap[u"-ep"] = &AudioServerHpaeDump::PolicyHandlerDump;
    dumpFuncMap[u"-ct"] = &AudioServerHpaeDump::AudioCacheTimeDump;
    dumpFuncMap[u"-cm"] = &AudioServerHpaeDump::AudioCacheMemoryDump;
    dumpFuncMap[u"-pm"] = &AudioServerHpaeDump::AudioPerformMonitorDump;
    dumpFuncMap[u"-ha"] = &AudioServerHpaeDump::HdiAdapterDump;
}

void AudioServerHpaeDump::AudioDataDump(std::string &dumpString, std::queue<std::u16string> &argQue)
{
    ArgDataDump(dumpString, argQue);
}

void AudioServerHpaeDump::ServerDataDump(string &dumpString)
{
    PlaybackSinkDump(dumpString);
    RecordSourceDump(dumpString);
    HDFModulesDump(dumpString);
    PolicyHandlerDump(dumpString);
}

void AudioServerHpaeDump::GetDeviceSinkInfo(std::string &dumpString, std::string deviceName)
{
    lock_guard<mutex> lock(lock_);
    AUDIO_INFO_LOG("GetDeviceSinkInfo %{public}s start.", deviceName.c_str());
    isFinishGetSinkInfo_ = false;
    IHpaeManager::GetHpaeManager().DumpSinkInfo(deviceName);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    dumpHpaeSinkInfo_.clear();
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetSinkInfo_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("GetDeviceSinkInfo timeout!");
        return;
    }
    AUDIO_INFO_LOG("GetDeviceSinkInfo %{public}s end.", deviceName.c_str());
    dumpString += dumpHpaeSinkInfo_;
}

void AudioServerHpaeDump::PlaybackSinkDump(std::string &dumpString)
{
    dumpString += "Hpae AudioServer Playback sink Dump:\n\n";
    for (auto it = devicesInfo_.sinkInfos.begin(); it != devicesInfo_.sinkInfos.end(); it++) {
        dumpString += it->deviceName + ":\n";
        GetDeviceSinkInfo(dumpString, it->deviceName);
        dumpString += "\n";
    }
    dumpString += "\n";
    PlaybackSinkInputDump(dumpString);
}

void AudioServerHpaeDump::OnDumpSinkInfoCb(std::string &dumpStr, int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    dumpHpaeSinkInfo_ = dumpStr;
    isFinishGetSinkInfo_ = true;
    AUDIO_INFO_LOG(
        "AudioServerHpaeDump OnDumpSinkInfoCb %{public}s, result %{public}d", dumpHpaeSinkInfo_.c_str(), result);
    callbackCV_.notify_all();
}

void AudioServerHpaeDump::GetDeviceSourceInfo(std::string &dumpString, std::string deviceName)
{
    lock_guard<mutex> lock(lock_);
    AUDIO_INFO_LOG("GetDeviceSourceInfo %{public}s start.", deviceName.c_str());
    isFinishGetSourceInfo_ = false;
    IHpaeManager::GetHpaeManager().DumpSourceInfo(deviceName);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    dumpHpaeSourceInfo_.clear();
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetSourceInfo_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("GetDeviceSourceInfo timeout!");
        return;
    }
    AUDIO_INFO_LOG("GetDeviceSourceInfo %{public}s end.", deviceName.c_str());
    dumpString += dumpHpaeSourceInfo_;
}

void AudioServerHpaeDump::RecordSourceDump(std::string &dumpString)
{
    dumpString += "Hpae AudioServer Record source Dump:\n\n";
    for (auto it = devicesInfo_.sourceInfos.begin(); it != devicesInfo_.sourceInfos.end(); it++) {
        dumpString += it->deviceName + ":\n";
        GetDeviceSourceInfo(dumpString, it->deviceName);
        dumpString += "\n";
    }
    dumpString += "\n";
    RecordSourceOutputDump(dumpString);
}

void AudioServerHpaeDump::OnDumpSourceInfoCb(std::string &dumpStr, int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    dumpHpaeSourceInfo_ = dumpStr;
    isFinishGetSourceInfo_ = true;
    AUDIO_INFO_LOG(
        "AudioServerHpaeDump OnDumpSourceInfoCb %{public}s, result %{public}d", dumpHpaeSourceInfo_.c_str(), result);
    callbackCV_.notify_all();
}

void AudioServerHpaeDump::ArgDataDump(std::string &dumpString, std::queue<std::u16string> &argQue)
{
    CHECK_AND_RETURN(GetDevicesInfo());
    dumpString += "Hpae AudioServer Data Dump:\n\n";
    if (argQue.empty()) {
        ServerDataDump(dumpString);
        return;
    }
    while (!argQue.empty()) {
        std::u16string para = argQue.front();
        if (para == u"-h" || para == u"-p") {
            dumpString.clear();
            (this->*dumpFuncMap[para])(dumpString);
            return;
        } else if (dumpFuncMap.count(para) == 0) {
            dumpString.clear();
            AppendFormat(dumpString, "Please input correct param:\n");
            HelpInfoDump(dumpString);
            return;
        } else {
            (this->*dumpFuncMap[para])(dumpString);
        }
        argQue.pop();
    }
}

void AudioServerHpaeDump::HelpInfoDump(string &dumpString)
{
    AppendFormat(dumpString, "usage:\n");
    AppendFormat(dumpString, "  -h\t\t\t|help text for hidumper audio\n");
    AppendFormat(dumpString, "  -p\t\t\t|dump hpae playback streams\n");
    AppendFormat(dumpString, "  -r\t\t\t|dump hpae record streams\n");
}

int32_t AudioServerHpaeDump::Initialize()
{
    AUDIO_INFO_LOG("AudioServerHpaeDump Initialize");
    IHpaeManager::GetHpaeManager().RegisterHpaeDumpCallback(weak_from_this());
    return SUCCESS;
}

void AudioServerHpaeDump::HDFModulesDump(std::string &dumpString)
{
    lock_guard<mutex> lock(lock_);
    dumpHdfModulesInfo_ += "\nHDF Input Modules\n";
    AppendFormat(dumpHdfModulesInfo_, "- %zu HDF Input Modules (s) available:\n", devicesInfo_.sourceInfos.size());

    for (auto it = devicesInfo_.sourceInfos.begin(); it != devicesInfo_.sourceInfos.end(); it++) {
        HpaeSinkSourceInfo &sourceInfo = *it;
        AppendFormat(dumpHdfModulesInfo_, "  Module %d\n", it - devicesInfo_.sourceInfos.begin() + 1);
        AppendFormat(dumpHdfModulesInfo_, "  - Module Name: %s\n", (sourceInfo.deviceName).c_str());
        AppendFormat(dumpHdfModulesInfo_, "  - Module Configuration: %s\n\n", sourceInfo.config.c_str());
    }

    dumpHdfModulesInfo_ += "HDF Output Modules\n";
    AppendFormat(dumpHdfModulesInfo_, "- %zu HDF Output Modules (s) available:\n", devicesInfo_.sinkInfos.size());

    for (auto it = devicesInfo_.sinkInfos.begin(); it != devicesInfo_.sinkInfos.end(); it++) {
        HpaeSinkSourceInfo &sinkInfo = *it;
        AppendFormat(dumpHdfModulesInfo_, "  Module %d\n", it - devicesInfo_.sinkInfos.begin() + 1);
        AppendFormat(dumpHdfModulesInfo_, "  - Module Name: %s\n", (sinkInfo.deviceName).c_str());
        AppendFormat(dumpHdfModulesInfo_, "  - Module Configuration: %s\n\n", sinkInfo.config.c_str());
    }

    AUDIO_INFO_LOG("HDFModulesDump : \n%{public}s end", dumpHdfModulesInfo_.c_str());
    dumpString += dumpHdfModulesInfo_;
}

bool AudioServerHpaeDump::GetDevicesInfo()
{
    lock_guard<mutex> lock(lock_);
    IHpaeManager::GetHpaeManager().DumpAllAvailableDevice(devicesInfo_);
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetHdfModulesInfo_ = false;
    dumpHdfModulesInfo_.clear();
    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetHdfModulesInfo_;  // will be true when got notified.
    });
    CHECK_AND_RETURN_RET_LOG(stopWaiting, false, "DumpAllAvailableDevice timeout!");
    return true;
}

void AudioServerHpaeDump::OnDumpAllAvailableDeviceCb(int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    isFinishGetHdfModulesInfo_ = true;
    AUDIO_INFO_LOG(
        "sink count %{public}zu, source count %{public}zu, result %{public}d",
        devicesInfo_.sinkInfos.size(), devicesInfo_.sourceInfos.size(), result);
    callbackCV_.notify_all();
}

void AudioServerHpaeDump::PolicyHandlerDump(std::string &dumpString)
{
    AUDIO_INFO_LOG("PolicyHandlerDump");
    AudioService::GetInstance()->Dump(dumpString);
}

void AudioServerHpaeDump::AudioCacheTimeDump(std::string &dumpString)
{
    AUDIO_INFO_LOG("AudioCacheTimeDump");
    dumpString += "\nAudioCached Time\n";

    int64_t startTime = 0;
    int64_t endTime = 0;
    AudioCacheMgr::GetInstance().GetCachedDuration(startTime, endTime);
    dumpString += "Call dump get time: [ " + ClockTime::NanoTimeToString(startTime) + " ~ " +
        ClockTime::NanoTimeToString(endTime) + " ], cur: [ " +
        ClockTime::NanoTimeToString(ClockTime::GetRealNano()) + " ] \n";
}

void AudioServerHpaeDump::AudioCacheMemoryDump(std::string &dumpString)
{
    AUDIO_INFO_LOG("AudioCacheMemoryDump");
    dumpString += "\nAudioCached Memory\n";

    size_t dataLength = 0;
    size_t bufferLength = 0;
    size_t structLength = 0;
    AudioCacheMgr::GetInstance().GetCurMemoryCondition(dataLength, bufferLength, structLength);
    dumpString += "dataLength: " + std::to_string(dataLength / BYTE_TO_KB_SIZE) + " KB, " +
                    "bufferLength: " + std::to_string(bufferLength / BYTE_TO_KB_SIZE) + " KB, " +
                    "structLength: " + std::to_string(structLength / BYTE_TO_KB_SIZE) + " KB \n";
}

void AudioServerHpaeDump::AudioPerformMonitorDump(std::string &dumpString)
{
    AUDIO_INFO_LOG("AudioPerformMonitorDump");
    dumpString += "\n Dump Audio Performance Monitor Record Infos\n";
    AudioPerformanceMonitor::GetInstance().DumpMonitorInfo(dumpString);
}

void AudioServerHpaeDump::HdiAdapterDump(std::string &dumpString)
{
    AUDIO_INFO_LOG("HdiAdapterDump");
    dumpString += "\nHdiAdapter Info\n";
    HdiAdapterManager::GetInstance().DumpInfo(dumpString);
}

void AudioServerHpaeDump::PlaybackSinkInputDump(std::string &dumpString)
{
    lock_guard<mutex> lock(lock_);
    AUDIO_INFO_LOG("get sinkinputs dump info");
    isFinishGetStreamInfo_ = false;
    dumpSinkInputsInfo_.clear();
    IHpaeManager::GetHpaeManager().DumpSinkInputsInfo();
    std::unique_lock<std::mutex> waitLock(callbackMutex_);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetStreamInfo_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("PlaybackSinkInputDump timeout!");
        return;
    }
    AUDIO_INFO_LOG("PlaybackSinkInputDump SUCCESS, info : \n%{public}s", dumpSinkInputsInfo_.c_str());
    dumpString += dumpSinkInputsInfo_;
}

void AudioServerHpaeDump::RecordSourceOutputDump(std::string &dumpString)
{
    lock_guard<mutex> lock(lock_);
    AUDIO_INFO_LOG("get sourceoutputs dump info");
    isFinishGetStreamInfo_ = false;
    dumpSourceOutputsInfo_.clear();
    IHpaeManager::GetHpaeManager().DumpSourceOutputsInfo();
    std::unique_lock<std::mutex> waitLock(callbackMutex_);

    bool stopWaiting = callbackCV_.wait_for(waitLock, std::chrono::milliseconds(OPERATION_TIMEOUT_IN_MS), [this] {
        return isFinishGetStreamInfo_;  // will be true when got notified.
    });
    if (!stopWaiting) {
        AUDIO_ERR_LOG("RecordSourceOutputDump timeout!");
        return;
    }
    AUDIO_INFO_LOG("RecordSourceOutputDump SUCCESS, info : \n%{public}s", dumpSourceOutputsInfo_.c_str());
    dumpString += dumpSourceOutputsInfo_;
}

static std::string TransTimeToString(uint64_t timetamp)
{
    auto tp = std::chrono::system_clock::time_point(std::chrono::milliseconds(timetamp));
    time_t time = std::chrono::system_clock::to_time_t(tp);
    struct tm *timeinfo = localtime(&time);
    if (!timeinfo) {
        return "Invalid time";
    }
    char buffer[80];
    CHECK_AND_RETURN_RET_LOG(strftime(buffer, sizeof(buffer), "%a %b %d %H:%M:%S %Y", timeinfo) != 0, "error time",
        "strftime failed");
    return buffer;
}

static void TransHpaeInputOutputInfoToStr(const HpaeInputOutputInfo &info, const size_t &idx, std::string &tempDumpStr)
{
    std::ostringstream oss;
    oss << "  Stream " << idx << "\n"
        << "  - Stream Id: " << info.sessionId << "\n"
        << "  - Device Name: " << info.deviceName << "\n"
        << "  - Application Name: " << GetBundleNameByToken(info.tokenId) << "\n"
        << "  - Process Id: " << info.pid << "\n"
        << "  - User Id: " << info.uid << "\n"
        << "  - Offload Enable: " << (info.offloadEnable ? "true" : "false") << "\n"
        << "  - stream can be captured: " << (info.privacyType == 0 ? "true" : "false") << "\n"
        << "  - Stream Configuration: " << info.config << "\n"
        << "  - Status: " << (info.state == HPAE_SESSION_RUNNING ? "RUNNING" : "STOPPED/PAUSED") << "\n"
        << "  - Stream Start Time: " << TransTimeToString(info.startTime) << "\n\n";
    tempDumpStr += oss.str();
}

void AudioServerHpaeDump::OnDumpSinkInputsInfoCb(std::vector<HpaeInputOutputInfo> &sinkInputs, int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    if (result == SUCCESS) {
        dumpSinkInputsInfo_ += "Playback Streams\n";
        AppendFormat(dumpSinkInputsInfo_, "- %zu Playback stream (s) available:\n", sinkInputs.size());
        for (auto it = sinkInputs.begin(); it != sinkInputs.end(); it++) {
            HpaeInputOutputInfo info = *it;
            TransHpaeInputOutputInfoToStr(info, it - sinkInputs.begin() + 1, dumpSinkInputsInfo_);
        }
        dumpSinkInputsInfo_ += "\n";
    }
    isFinishGetStreamInfo_ = true;
    AUDIO_INFO_LOG("AudioServerHpaeDump OnDumpSinkInputsInfoCb result %{public}d", result);
    callbackCV_.notify_all();
}

void AudioServerHpaeDump::OnDumpSourceOutputsInfoCb(std::vector<HpaeInputOutputInfo> &sourceOutputs, int32_t result)
{
    std::unique_lock<std::mutex> waitLock(callbackMutex_);
    if (result == SUCCESS) {
        dumpSourceOutputsInfo_ += "Record Streams\n";
        AppendFormat(dumpSourceOutputsInfo_, "- %zu Record stream (s) available:\n", sourceOutputs.size());
        for (auto it = sourceOutputs.begin(); it != sourceOutputs.end(); it++) {
            HpaeInputOutputInfo info = *it;
            TransHpaeInputOutputInfoToStr(info, it - sourceOutputs.begin() + 1, dumpSourceOutputsInfo_);
        }
        dumpSourceOutputsInfo_ += "\n";
    }
    isFinishGetStreamInfo_ = true;
    AUDIO_INFO_LOG("AudioServerHpaeDump OnDumpSourceOutputsInfoCb result %{public}d", result);
    callbackCV_.notify_all();
}
}  // namespace AudioStandard
}  // namespace OHOS
