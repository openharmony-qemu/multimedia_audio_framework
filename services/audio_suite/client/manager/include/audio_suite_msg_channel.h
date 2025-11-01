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
#ifndef AUDIO_SUITE_MSG_CHANNEL_H
#define AUDIO_SUITE_MSG_CHANNEL_H

#include <any>


namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

enum PipelineMsgCode {
    START_PIPELINE,
    STOP_PIPELINE,
    GET_PIPELINE_STATE,
    CREATE_NODE,
    DESTROY_NODE,
    SET_BYPASS_STATUS,
    GET_BYPASS_STATUS,
    SET_AUDIO_FORMAT,
    SET_WRITEDATA_CALLBACK,
    CONNECT_NODES,
    DISCONNECT_NODES,
    INSTALL_NODE_TAP,
    REMOVE_NODE_TAP,
    RENDER_FRAME,
    MULTI_RENDER_FRAME,
    GET_OPTIONS,
};

class ISendMsgCallback {
public:
    virtual void Invoke(PipelineMsgCode cmdID, const std::any &args) = 0;
};

class CallbackSender {
protected:
    std::weak_ptr<ISendMsgCallback> weakCallback_;

public:
    void RegisterSendMsgCallback(std::weak_ptr<ISendMsgCallback> cb)
    {
        weakCallback_ = cb;
    }

    template <typename... Args>
    void TriggerCallback(PipelineMsgCode cmdID, Args &&...args)
    {
        if (auto callback = weakCallback_.lock()) {
            // pack the arguments into a tuple
            auto packed = std::make_tuple(std::forward<Args>(args)...);
            callback->Invoke(cmdID, packed);
        }
    }
};

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS

#endif