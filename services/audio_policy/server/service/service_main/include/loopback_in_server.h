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

#ifndef LOOPBACK_IN_SERVER_H
#define LOOPBACK_IN_SERVER_H

#include <memory>

#include "loopback_stub.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {

class AudioLoopbackManager;

class LoopbackInServer : public LoopbackStub {
public:
    LoopbackInServer(AudioLoopbackMode mode, LoopbackType type, int32_t pid,
        std::shared_ptr<AudioLoopbackManager> manager);
    ~LoopbackInServer();

    int32_t Enable(bool enable, int32_t &ret) override;
    int32_t GetStatus(int32_t &status, int32_t &ret) override;
    int32_t GetVolume(float &volume, int32_t &ret) override;

    AudioLoopbackMode GetMode() const { return mode_; }
    LoopbackType GetType() const { return type_; }
    int32_t GetPid() const { return pid_; }

private:
    AudioLoopbackMode mode_;
    LoopbackType type_;
    int32_t pid_;
    std::weak_ptr<AudioLoopbackManager> manager_;
};

} // namespace AudioStandard
} // namespace OHOS

#endif // LOOPBACK_IN_SERVER_H