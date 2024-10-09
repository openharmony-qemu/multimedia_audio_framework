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

#ifndef PA_ADAPTER_TOOLS_H
#define PA_ADAPTER_TOOLS_H

#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>

namespace OHOS {
namespace AudioStandard {
// PaLockGuard is used to auto-call unlock.
class PaLockGuard {
public:
    PaLockGuard(pa_threaded_mainloop *mainloop, bool mainloopCheck = false) : mainloop_(mainloop)
    {
        if (mainloopCheck) {
            isInMainloop_ = pa_threaded_mainloop_in_thread(mainloop_) ? true : false;
        } else {
            isInMainloop_ = false;
        }
        if (!isInMainloop_) {
            pa_threaded_mainloop_lock(mainloop_);
        }
    }

    ~PaLockGuard()
    {
        Unlock();
    }

    void Unlock()
    {
        if (!isInMainloop_ && !isUnlocked_) {
            pa_threaded_mainloop_unlock(mainloop_);
            isUnlocked_ = true;
        }
    }

    void Relock()
    {
        if (!isInMainloop_ && isUnlocked_) {
            isUnlocked_ = false;
            pa_threaded_mainloop_lock(mainloop_);
        }
    }
private:
    bool isUnlocked_ = false;
    bool isInMainloop_ = false;
    pa_threaded_mainloop *mainloop_ = nullptr;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // PA_ADAPTER_TOOLS_H
