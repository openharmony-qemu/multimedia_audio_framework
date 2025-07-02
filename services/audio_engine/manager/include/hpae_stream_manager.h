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
#ifndef HPAE_STREAM_MANAGER_H
#define HPAE_STREAM_MANAGER_H
#include <stdint.h>
#include "audio_stream_info.h"
#include "hpae_define.h"
#include "i_stream.h"
namespace OHOS {
namespace AudioStandard {
namespace HPAE {
class HpaeStreamManager : public CallbackSender,
                          public INodeCallback,
                          public std::enable_shared_from_this<HpaeStreamManager> {
public:
    virtual ~HpaeStreamManager()
    {}
    virtual int32_t CreateStream(const HpaeStreamInfo &streamInfo) = 0;
    virtual int32_t DestroyStream(uint32_t sessionId) = 0;
    virtual int32_t Start(uint32_t sessionId) = 0;
    virtual int32_t Pause(uint32_t sessionId) = 0;
    virtual int32_t Flush(uint32_t sessionId) = 0;
    virtual int32_t Drain(uint32_t sessionId) = 0;
    virtual int32_t Stop(uint32_t sessionId) = 0;
    virtual int32_t Release(uint32_t sessionId) = 0;
    virtual int32_t MoveStream(uint32_t sessionId, const std::string &sinkName) = 0;
    virtual int32_t MoveAllStream(const std::string &name, const std::vector<uint32_t>& sessionIds,
        MoveSessionType moveType = MOVE_ALL) = 0;
    virtual int32_t SetMute(bool isMute) = 0;
    virtual void Process() = 0;
    virtual void HandleMsg() = 0;
    virtual int32_t Init(bool isReload = false) = 0;
    virtual int32_t DeInit(bool isMoveDefault = false) = 0;
    virtual bool IsInit() = 0;
    virtual bool IsRunning() = 0;
    virtual bool IsMsgProcessing() = 0;
    virtual bool DeactivateThread() = 0;
    virtual std::string GetThreadName() = 0;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif