/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef AUDIO_SCHEDULE_GUARD_H
#define AUDIO_SCHEDULE_GUARD_H

#include <inttypes.h>
#include <string>
#include <memory>
#include <map>
#include <mutex>
#include <condition_variable>
#include <sys/types.h>
#include "audio_schedule.h"

namespace OHOS {
namespace AudioStandard {
class AudioScheduleGuard {
public:
    AudioScheduleGuard(pid_t pid, pid_t tid, const std::string &bundleName = "audio_server");

    AudioScheduleGuard(const AudioScheduleGuard&) = delete;

    AudioScheduleGuard operator=(const AudioScheduleGuard&) = delete;

    AudioScheduleGuard(AudioScheduleGuard&& audioScheduleGuard);

    bool operator==(const AudioScheduleGuard&) const;

    AudioScheduleGuard& operator=(AudioScheduleGuard&& audioScheduleGuard) = delete;

    ~AudioScheduleGuard();
private:
    pid_t pid_;
    pid_t tid_;
    std::string bundleName_;
    bool isReported_ = false;
};

class SharedAudioScheduleGuard {
public:
    static std::shared_ptr<SharedAudioScheduleGuard> Create(
        pid_t pid, pid_t tid, const std::string &bundleName = "audio_server");

    SharedAudioScheduleGuard(const SharedAudioScheduleGuard&) = delete;
    SharedAudioScheduleGuard(SharedAudioScheduleGuard&&) = delete;
    SharedAudioScheduleGuard operator=(const SharedAudioScheduleGuard&) = delete;
    SharedAudioScheduleGuard& operator=(SharedAudioScheduleGuard&&) = delete;

    SharedAudioScheduleGuard(pid_t pid, pid_t tid, const std::string &bundleName = "audio_server")
        : guard_(pid, tid, bundleName), pid_(pid), tid_(tid) {};

    ~SharedAudioScheduleGuard();
private:
    AudioScheduleGuard guard_;

    pid_t pid_;
    pid_t tid_;

    static std::map<std::pair<pid_t, pid_t>,
        std::weak_ptr<SharedAudioScheduleGuard>> guardMap_;

    static std::mutex mutex_;

    static std::condition_variable cv_;
};
} // namespace AudioStandard
} // namespace OHOS

#endif // AUDIO_SCHEDULE_GUARD_H