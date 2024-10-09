/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#define LOG_TAG "AudioServerDump"
#endif

#include "audio_server_dump.h"
#include "audio_utils.h"
#include "audio_service.h"
#include "pa_adapter_tools.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

AudioServerDump::AudioServerDump() : mainLoop(nullptr),
    api(nullptr),
    context(nullptr),
    isMainLoopStarted_(false),
    isContextConnected_(false)
{
    AUDIO_DEBUG_LOG("AudioServerDump construct");
    InitDumpFuncMap();
}

AudioServerDump::~AudioServerDump()
{
    ResetPAAudioDump();
}

void AudioServerDump::InitDumpFuncMap()
{
    dumpFuncMap[u"-h"] = &AudioServerDump::HelpInfoDump;
    dumpFuncMap[u"-p"] = &AudioServerDump::PlaybackSinkDump;
    dumpFuncMap[u"-r"] = &AudioServerDump::RecordSourceDump;
    dumpFuncMap[u"-m"] = &AudioServerDump::HDFModulesDump;
    dumpFuncMap[u"-ep"] = &AudioServerDump::PolicyHandlerDump;
}

void AudioServerDump::ResetPAAudioDump()
{
    lock_guard<mutex> lock(ctrlMutex_);
    if (mainLoop && (isMainLoopStarted_ == true)) {
        pa_threaded_mainloop_stop(mainLoop);
    }

    if (context) {
        pa_context_set_state_callback(context, nullptr, nullptr);
        if (isContextConnected_ == true) {
            AUDIO_INFO_LOG("[AudioServerDump] disconnect context!");
            pa_context_disconnect(context);
        }
        pa_context_unref(context);
    }

    if (mainLoop) {
        pa_threaded_mainloop_free(mainLoop);
    }

    isMainLoopStarted_  = false;
    isContextConnected_ = false;
    mainLoop = nullptr;
    context  = nullptr;
    api      = nullptr;
}

int32_t AudioServerDump::Initialize()
{
    mainLoop = pa_threaded_mainloop_new();
    if (mainLoop == nullptr) {
        return AUDIO_DUMP_INIT_ERR;
    }

    api = pa_threaded_mainloop_get_api(mainLoop);
    if (api == nullptr) {
        ResetPAAudioDump();
        return AUDIO_DUMP_INIT_ERR;
    }

    context = pa_context_new(api, "AudioServerDump");
    if (context == nullptr) {
        ResetPAAudioDump();
        return AUDIO_DUMP_INIT_ERR;
    }

    pa_context_set_state_callback(context, PAContextStateCb, mainLoop);

    if (pa_context_connect(context, nullptr, PA_CONTEXT_NOFAIL, nullptr) < 0) {
        int error = pa_context_errno(context);
        AUDIO_ERR_LOG("context connect error: %{public}s", pa_strerror(error));
        ResetPAAudioDump();
        return AUDIO_DUMP_INIT_ERR;
    }

    isContextConnected_ = true;
    PaLockGuard lock(mainLoop);

    if (pa_threaded_mainloop_start(mainLoop) < 0) {
        AUDIO_ERR_LOG("Audio Service not started");
        ResetPAAudioDump();
        return AUDIO_DUMP_INIT_ERR;
    }

    isMainLoopStarted_ = true;
    while (isMainLoopStarted_) {
        pa_context_state_t state = pa_context_get_state(context);
        if (state == PA_CONTEXT_READY) {
            break;
        }

        if (!PA_CONTEXT_IS_GOOD(state)) {
            int error = pa_context_errno(context);
            AUDIO_ERR_LOG("context bad state error: %{public}s", pa_strerror(error));
            ResetPAAudioDump();
            return AUDIO_DUMP_INIT_ERR;
        }

        pa_threaded_mainloop_wait(mainLoop);
    }

    return AUDIO_DUMP_SUCCESS;
}

void AudioServerDump::OnTimeOut()
{
    PaLockGuard lock(mainLoop);
    pa_threaded_mainloop_signal(mainLoop, 0);
}

bool AudioServerDump::IsEndWith(const std::string &mainStr, const std::string &toMatch)
{
    if (mainStr.size() >= toMatch.size() &&
        mainStr.compare(mainStr.size() - toMatch.size(), toMatch.size(), toMatch) == 0) {
        return true;
    }
    return false;
}

bool AudioServerDump::IsValidModule(const std::string moduleName)
{
    if (moduleName.rfind("fifo", 0) == SUCCESS) {
        return false; // Module starts with fifo, Not valid module
    }

    if (IsEndWith(moduleName, "monitor")) {
        return false; // Module ends with monitor, Not valid module
    }
    return true;
}

void AudioServerDump::ServerDataDump(string &dumpString)
{
    PlaybackSinkDump(dumpString);
    RecordSourceDump(dumpString);
    HDFModulesDump(dumpString);
    PolicyHandlerDump(dumpString);
}

void AudioServerDump::ArgDataDump(std::string &dumpString, std::queue<std::u16string>& argQue)
{
    dumpString += "AudioServer Data Dump:\n\n";
    if (argQue.empty()) {
        ServerDataDump(dumpString);
        return;
    }
    while (!argQue.empty()) {
        std::u16string para = argQue.front();
        if (para == u"-h") {
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

void AudioServerDump::HelpInfoDump(string &dumpString)
{
    AppendFormat(dumpString, "usage:\n");
    AppendFormat(dumpString, "  -h\t\t\t|help text for hidumper audio\n");
    AppendFormat(dumpString, "  -p\t\t\t|dump pa playback streams\n");
    AppendFormat(dumpString, "  -r\t\t\t|dump pa record streams\n");
    AppendFormat(dumpString, "  -m\t\t\t|dump hdf input modules\n");
    AppendFormat(dumpString, "  -ep\t\t\t|dump policyhandler info\n");
}

void AudioServerDump::AudioDataDump(string &dumpString, std::queue<std::u16string>& argQue)
{
    if (mainLoop == nullptr || context == nullptr) {
        AUDIO_ERR_LOG("Audio Service Not running");
        return;
    }

    PaLockGuard lock(mainLoop);
    pa_operation *operation = nullptr;
    operation = pa_context_get_sink_info_list(context,
        AudioServerDump::PASinkInfoCallback, reinterpret_cast<void *>(this));

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mainLoop);
    }

    pa_operation_unref(operation);
    operation = pa_context_get_sink_input_info_list(context,
        AudioServerDump::PASinkInputInfoCallback, reinterpret_cast<void *>(this));

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mainLoop);
    }

    pa_operation_unref(operation);
    operation = pa_context_get_source_info_list(context,
        AudioServerDump::PASourceInfoCallback, reinterpret_cast<void *>(this));

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mainLoop);
    }

    pa_operation_unref(operation);
    operation = pa_context_get_source_output_info_list(context,
        AudioServerDump::PASourceOutputInfoCallback, reinterpret_cast<void *>(this));

    while (pa_operation_get_state(operation) == PA_OPERATION_RUNNING) {
        pa_threaded_mainloop_wait(mainLoop);
    }

    pa_operation_unref(operation);

    ArgDataDump(dumpString, argQue);

    return;
}

void AudioServerDump::PAContextStateCb(pa_context *context, void *userdata)
{
    pa_threaded_mainloop *mainLoop = reinterpret_cast<pa_threaded_mainloop *>(userdata);

    switch (pa_context_get_state(context)) {
        case PA_CONTEXT_READY:
        case PA_CONTEXT_TERMINATED:
        case PA_CONTEXT_FAILED:
            pa_threaded_mainloop_signal(mainLoop, 0);
            break;

        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
        default:
            break;
    }
    return;
}

void AudioServerDump::PASinkInfoCallback(pa_context *c, const pa_sink_info *i, int eol, void *userdata)
{
    AudioServerDump *asDump = reinterpret_cast<AudioServerDump *>(userdata);
    CHECK_AND_RETURN_LOG(asDump != nullptr, "Failed to get sink information");

    pa_threaded_mainloop *mainLoop = reinterpret_cast<pa_threaded_mainloop *>(asDump->mainLoop);

    CHECK_AND_RETURN_LOG(eol >= 0, "Failed to get sink information: %{public}s", pa_strerror(pa_context_errno(c)));

    if (eol) {
        pa_threaded_mainloop_signal(mainLoop, 0);
        return;
    }

    SinkSourceInfo sinkInfo;

    if (i->name != nullptr) {
        string sinkName(i->name);
        if (IsValidModule(sinkName)) {
            (sinkInfo.name).assign(sinkName);
            sinkInfo.sampleSpec = i->sample_spec;
            asDump->streamData_.sinkDevices.push_back(sinkInfo);
        }
    }
}

void AudioServerDump::PASinkInputInfoCallback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata)
{
    AudioServerDump *asDump = reinterpret_cast<AudioServerDump *>(userdata);
    CHECK_AND_RETURN_LOG(asDump != nullptr, "Failed to get sink input information");
    pa_threaded_mainloop *mainLoop = reinterpret_cast<pa_threaded_mainloop *>(asDump->mainLoop);
    CHECK_AND_RETURN_LOG(eol >= 0, "Failed to get sink input information: %{public}s",
        pa_strerror(pa_context_errno(c)));
    if (eol) {
        pa_threaded_mainloop_signal(mainLoop, 0);
        return;
    }
    InputOutputInfo sinkInputInfo;
    sinkInputInfo.sampleSpec = i->sample_spec;
    sinkInputInfo.corked = i->corked;
    if (i->proplist != nullptr) {
        const char *applicationname = pa_proplist_gets(i->proplist, "application.name");
        const char *processid = pa_proplist_gets(i->proplist, "application.process.id");
        const char *user = pa_proplist_gets(i->proplist, "application.process.user");
        const char *sessionid = pa_proplist_gets(i->proplist, "stream.sessionID");
        const char *sessionstarttime = pa_proplist_gets(i->proplist, "stream.startTime");
        const char *privacytype = pa_proplist_gets(i->proplist, "stream.privacyType");
        if (applicationname != nullptr) {
            string applicationName(applicationname);
            (sinkInputInfo.applicationName).assign(applicationName);
        }
        if (processid != nullptr) {
            string processId(processid);
            (sinkInputInfo.processId).assign(processId);
        }
        if (user != nullptr) {
            struct passwd *p;
            if ((p = getpwnam(user)) != nullptr) {
                sinkInputInfo.userId = uint32_t(p->pw_uid);
            }
        }
        if (sessionid != nullptr) {
            string sessionId(sessionid);
            (sinkInputInfo.sessionId).assign(sessionId);
        }
        if (sessionstarttime != nullptr) {
            string sessionStartTime(sessionstarttime);
            (sinkInputInfo.sessionStartTime).assign(sessionStartTime);
        }
        if (privacytype != nullptr) {
            string privacyType(privacytype);
            (sinkInputInfo.privacyType).assign(privacyType);
        }
    }
    asDump->streamData_.sinkInputs.push_back(sinkInputInfo);
}

void AudioServerDump::PASourceInfoCallback(pa_context *c, const pa_source_info *i, int eol, void *userdata)
{
    AudioServerDump *asDump = reinterpret_cast<AudioServerDump *>(userdata);
    CHECK_AND_RETURN_LOG(asDump != nullptr, "Failed to get source information");

    pa_threaded_mainloop *mainLoop = reinterpret_cast<pa_threaded_mainloop *>(asDump->mainLoop);
    CHECK_AND_RETURN_LOG(eol >= 0, "Failed to get source information: %{public}s",
        pa_strerror(pa_context_errno(c)));

    if (eol) {
        pa_threaded_mainloop_signal(mainLoop, 0);
        return;
    }

    SinkSourceInfo sourceInfo;

    if (i->name != nullptr) {
        string sourceName(i->name);
        if (IsValidModule(sourceName)) {
            (sourceInfo.name).assign(sourceName);
            sourceInfo.sampleSpec = i->sample_spec;
            asDump->streamData_.sourceDevices.push_back(sourceInfo);
        }
    }
}

void AudioServerDump::PASourceOutputInfoCallback(pa_context *c, const pa_source_output_info *i, int eol,
    void *userdata)
{
    AudioServerDump *asDump = reinterpret_cast<AudioServerDump *>(userdata);
    CHECK_AND_RETURN_LOG(asDump != nullptr, "Failed to get source output information");
    pa_threaded_mainloop *mainLoop = reinterpret_cast<pa_threaded_mainloop *>(asDump->mainLoop);
    CHECK_AND_RETURN_LOG(eol >= 0, "Failed to get source output information: %{public}s",
        pa_strerror(pa_context_errno(c)));
    if (eol) {
        pa_threaded_mainloop_signal(mainLoop, 0);
        return;
    }
    InputOutputInfo sourceOutputInfo;
    sourceOutputInfo.sampleSpec = i->sample_spec;
    sourceOutputInfo.corked = i->corked;
    if (i->proplist != nullptr) {
        const char *applicationname = pa_proplist_gets(i->proplist, "application.name");
        const char *processid = pa_proplist_gets(i->proplist, "application.process.id");
        const char *user = pa_proplist_gets(i->proplist, "application.process.user");
        const char *sessionid = pa_proplist_gets(i->proplist, "stream.sessionID");
        const char *sessionstarttime = pa_proplist_gets(i->proplist, "stream.startTime");
        const char *privacytype = pa_proplist_gets(i->proplist, "stream.privacyType");
        if (applicationname != nullptr) {
            string applicationName(applicationname);
            (sourceOutputInfo.applicationName).assign(applicationName);
        }
        if (processid != nullptr) {
            string processId(processid);
            (sourceOutputInfo.processId).assign(processId);
        }
        if (user != nullptr) {
            struct passwd *p;
            if ((p = getpwnam(user)) != nullptr) {
                sourceOutputInfo.userId = uint32_t(p->pw_uid);
            }
        }
        if (sessionid != nullptr) {
            string sessionId(sessionid);
            (sourceOutputInfo.sessionId).assign(sessionId);
        }
        if (sessionstarttime != nullptr) {
            string sessionStartTime(sessionstarttime);
            (sourceOutputInfo.sessionStartTime).assign(sessionStartTime);
        }
        if (privacytype != nullptr) {
            string privacyType(privacytype);
            (sourceOutputInfo.privacyType).assign(privacyType);
        }
    }
    asDump->streamData_.sourceOutputs.push_back(sourceOutputInfo);
}

void AudioServerDump::PlaybackSinkDump(std::string &dumpString)
{
    AUDIO_INFO_LOG("PlaybackSinkDump enter");
    char s[PA_SAMPLE_SPEC_SNPRINT_MAX];

    dumpString += "Playback Streams\n";

    AppendFormat(dumpString, "- %zu Playback stream (s) available:\n", streamData_.sinkInputs.size());

    for (auto it = streamData_.sinkInputs.begin(); it != streamData_.sinkInputs.end(); it++) {
        InputOutputInfo sinkInputInfo = *it;

        AppendFormat(dumpString, "  Stream %d\n", it - streamData_.sinkInputs.begin() + 1);
        AppendFormat(dumpString, "  - Stream Id: %s\n", (sinkInputInfo.sessionId).c_str());
        AppendFormat(dumpString, "  - Application Name: %s\n", ((sinkInputInfo.applicationName).c_str()));
        AppendFormat(dumpString, "  - Process Id: %s\n", (sinkInputInfo.processId).c_str());
        AppendFormat(dumpString, "  - User Id: %u\n", sinkInputInfo.userId);
        AppendFormat(dumpString, "  - stream can be captured: %s\n",
            sinkInputInfo.privacyType == "0" ? "true" : "false");

        char *inputSampleSpec = pa_sample_spec_snprint(s, sizeof(s), &(sinkInputInfo.sampleSpec));
        AppendFormat(dumpString, "  - Stream Configuration: %s\n", inputSampleSpec);
        dumpString += "  - Status:";
        dumpString += (sinkInputInfo.corked) ? "STOPPED/PAUSED" : "RUNNING";
        AppendFormat(dumpString, "\n  - Stream Start Time: %s\n", (sinkInputInfo.sessionStartTime).c_str());
        dumpString += "\n";
    }
}

void AudioServerDump::RecordSourceDump(std::string &dumpString)
{
    char s[PA_SAMPLE_SPEC_SNPRINT_MAX];
    dumpString += "Record Streams \n";
    AppendFormat(dumpString, "- %zu Record stream (s) available:\n", streamData_.sourceOutputs.size());

    for (auto it = streamData_.sourceOutputs.begin(); it != streamData_.sourceOutputs.end(); it++) {
        InputOutputInfo sourceOutputInfo = *it;
        AppendFormat(dumpString, "  Stream %d\n", it - streamData_.sourceOutputs.begin() + 1);
        AppendFormat(dumpString, "  - Stream Id: %s\n", (sourceOutputInfo.sessionId).c_str());
        AppendFormat(dumpString, "  - Application Name: %s\n", (sourceOutputInfo.applicationName).c_str());
        AppendFormat(dumpString, "  - Process Id: %s\n", sourceOutputInfo.processId.c_str());
        AppendFormat(dumpString, "  - User Id: %u\n", sourceOutputInfo.userId);

        char *outputSampleSpec = pa_sample_spec_snprint(s, sizeof(s), &(sourceOutputInfo.sampleSpec));
        AppendFormat(dumpString, "  - Stream Configuration: %s\n", outputSampleSpec);
        dumpString += "  - Status:";
        dumpString += (sourceOutputInfo.corked) ? "STOPPED/PAUSED" : "RUNNING";
        AppendFormat(dumpString, "\n  - Stream Start Time: %s\n", (sourceOutputInfo.sessionStartTime).c_str());
        dumpString += "\n";
    }
}

void AudioServerDump::HDFModulesDump(std::string &dumpString)
{
    char s[PA_SAMPLE_SPEC_SNPRINT_MAX];

    dumpString += "\nHDF Input Modules\n";
    AppendFormat(dumpString, "- %zu HDF Input Modules (s) available:\n", streamData_.sourceDevices.size());

    for (auto it = streamData_.sourceDevices.begin(); it != streamData_.sourceDevices.end(); it++) {
        SinkSourceInfo sourceInfo = *it;

        AppendFormat(dumpString, "  Module %d\n", it - streamData_.sourceDevices.begin() + 1);
        AppendFormat(dumpString, "  - Module Name: %s\n", (sourceInfo.name).c_str());
        char *hdfOutSampleSpec = pa_sample_spec_snprint(s, sizeof(s), &(sourceInfo.sampleSpec));
        AppendFormat(dumpString, "  - Module Configuration: %s\n\n", hdfOutSampleSpec);
    }

    dumpString += "HDF Output Modules\n";
    AppendFormat(dumpString, "- %zu HDF Output Modules (s) available:\n", streamData_.sinkDevices.size());

    for (auto it = streamData_.sinkDevices.begin(); it != streamData_.sinkDevices.end(); it++) {
        SinkSourceInfo sinkInfo = *it;
        AppendFormat(dumpString, "  Module %d\n", it - streamData_.sinkDevices.begin() + 1);
        AppendFormat(dumpString, "  - Module Name: %s\n", (sinkInfo.name).c_str());
        char *hdfInSampleSpec = pa_sample_spec_snprint(s, sizeof(s), &(sinkInfo.sampleSpec));
        AppendFormat(dumpString, "  - Module Configuration: %s\n\n", hdfInSampleSpec);
    }
}

void AudioServerDump::PolicyHandlerDump(std::string &dumpString)
{
    AUDIO_INFO_LOG("PolicyHandlerDump");
    AudioService::GetInstance()->Dump(dumpString);
}
} // namespace AudioStandard
} // namespace OHOS
