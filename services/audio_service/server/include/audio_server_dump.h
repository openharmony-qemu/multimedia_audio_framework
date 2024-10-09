/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef AUDIO_SERVER_DUMP_H
#define AUDIO_SERVER_DUMP_H

#include <vector>
#include <list>
#include <queue>
#include <map>
#include <pwd.h>
#include "securec.h"
#include "nocopyable.h"

#include <pulse/pulseaudio.h>

#include "audio_service_log.h"
#include "audio_timer.h"
#include "audio_errors.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

static const int32_t AUDIO_DUMP_SUCCESS = 0;
static const int32_t AUDIO_DUMP_INIT_ERR = -1;

typedef struct {
    std::string name;
    pa_sample_spec sampleSpec;
} SinkSourceInfo;

typedef struct {
    uint32_t userId;
    uint32_t corked;                   // status
    std::string sessionId;
    std::string sessionStartTime;
    std::string applicationName;
    std::string processId;
    std::string privacyType;
    pa_sample_spec sampleSpec;
}InputOutputInfo;

typedef struct {
    std::vector<SinkSourceInfo> sinkDevices;
    std::vector<SinkSourceInfo> sourceDevices;
    std::vector<InputOutputInfo> sinkInputs;
    std::vector<InputOutputInfo> sourceOutputs;
} StreamData;

class AudioServerDump : public AudioTimer {
public:
    DISALLOW_COPY_AND_MOVE(AudioServerDump);

    AudioServerDump();
    ~AudioServerDump();
    int32_t Initialize();
    void AudioDataDump(std::string &dumpString, std::queue<std::u16string>& argQue);
    virtual void OnTimeOut();

private:
    pa_threaded_mainloop *mainLoop;
    pa_mainloop_api *api;
    pa_context *context;
    std::mutex ctrlMutex_;

    bool isMainLoopStarted_;
    bool isContextConnected_;
    StreamData streamData_;

    int32_t ConnectStreamToPA();
    void ResetPAAudioDump();

    void PlaybackSinkDump(std::string &dumpString);
    void RecordSourceDump(std::string &dumpString);
    void HDFModulesDump(std::string &dumpString);
    void PolicyHandlerDump(std::string &dumpString);
    void ArgDataDump(std::string &dumpString, std::queue<std::u16string>& argQue);
    void ServerDataDump(std::string &dumpString);
    void InitDumpFuncMap();
    void HelpInfoDump(std::string& dumpString);
    static bool IsEndWith(const std::string &mainStr, const std::string &toMatch);
    static bool IsValidModule(const std::string moduleName);

    // Callbacks
    static void PAContextStateCb(pa_context *context, void *userdata);
    static void PASinkInfoCallback(pa_context *c, const pa_sink_info *i, int eol, void *userdata);
    static void PASinkInputInfoCallback(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata);
    static void PASourceInfoCallback(pa_context *c, const pa_source_info *i, int eol, void *userdata);
    static void PASourceOutputInfoCallback(pa_context *c, const pa_source_output_info *i, int eol, void *userdata);

    using DumpFunc = void(AudioServerDump::*)(std::string &dumpString);
    std::map<std::u16string, DumpFunc> dumpFuncMap;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SERVER_DUMP_H
