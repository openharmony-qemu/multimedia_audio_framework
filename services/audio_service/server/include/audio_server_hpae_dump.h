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
#ifndef AUDIO_SERVER_HPAE_DUMP_H
#define AUDIO_SERVER_HPAE_DUMP_H

#include <vector>
#include <list>
#include <queue>
#include <map>
#include <pwd.h>
#include "securec.h"
#include "nocopyable.h"
#include "audio_service_hpae_dump_callback.h"
#include "audio_service_log.h"
#include "audio_timer.h"
#include "audio_errors.h"

namespace OHOS {
namespace AudioStandard {

class AudioServerHpaeDump : public AudioTimer, public AudioServiceHpaeDumpCallback {
public:
    DISALLOW_COPY_AND_MOVE(AudioServerHpaeDump);

    AudioServerHpaeDump();
    ~AudioServerHpaeDump();
    int32_t Initialize();
    void AudioDataDump(std::string &dumpString, std::queue<std::u16string> &argQue);
    void OnDumpSinkInfoCb(std::string &dumpStr, int32_t result) override;
    void OnDumpSourceInfoCb(std::string &dumpStr, int32_t result) override;
private:
    void InitDumpFuncMap();
    void HelpInfoDump(std::string &dumpString);
    void ArgDataDump(std::string &dumpString, std::queue<std::u16string> &argQue);
    void ServerDataDump(std::string &dumpString);
    void PlaybackSinkDump(std::string &dumpString);
    void GetDeviceSinkInfo(std::string &dumpString, std::string deviceName);
    void RecordSourceDump(std::string &dumpString);
    void GetDeviceSourceInfo(std::string &dumpString, std::string deviceName);
    
    using DumpFunc = void(AudioServerHpaeDump::*)(std::string &dumpString);
    std::map<std::u16string, DumpFunc> dumpFuncMap;
    std::string dumpHpaeSinkInfo_;
    std::string dumpHpaeSourceInfo_;
    std::mutex lock_;
    // for status operation wait and notify
    std::mutex callbackMutex_;
    std::condition_variable callbackCV_;
    bool isFinishGetSinkInfo_ = false;
    bool isFinishGetSourceInfo_ = false;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SERVER_DUMP_H
