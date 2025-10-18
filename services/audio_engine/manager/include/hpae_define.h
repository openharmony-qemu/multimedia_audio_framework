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

#ifndef HPAE_DEFINE_H
#define HPAE_DEFINE_H
#include "hpae_msg_channel.h"
#include "i_stream.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
constexpr uint32_t MILLISECOND_PER_SECOND = 1000;
constexpr uint32_t FRAME_LEN_20MS = 20; // 20ms

struct HpaeSessionInfo {
    HpaeStreamInfo streamInfo;
    HpaeSessionState state = HPAE_SESSION_NEW;
    std::weak_ptr<IStreamStatusCallback> statusCallback;
    int32_t offloadType = OFFLOAD_DEFAULT;
    bool offloadEnable = false;
    float speed = 1.0f;
    uint64_t startTime; // create time
};


constexpr int32_t SCENE_TYPE_NUM = 9;

struct HpaeRenderSessionInfo {
    HpaeProcessorType sceneType = HPAE_SCENE_DEFAULT;
    HpaeSessionState state = HPAE_SESSION_NEW;
    bool isMoveAble = true;
    bool bypass = false;
};

struct HpaeSinkInputInfo {
    HpaeRenderSessionInfo rendererSessionInfo;
    HpaeNodeInfo nodeInfo;
};

struct HpaeCapturerSessionInfo {
    HpaeProcessorType sceneType = HPAE_SCENE_DEFAULT;
    HpaeSessionState state = HPAE_SESSION_NEW;
    bool isMoveAble = true;
};

struct HpaeSourceOutputInfo {
    HpaeCapturerSessionInfo capturerSessionInfo;
    HpaeNodeInfo nodeInfo;
};

enum HpaeBufferType {
    HPAE_BUFFER_TYPE_DEFAULT = 0,
    HPAE_BUFFER_TYPE_COBUFFER
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS

#endif // HPAE_DEFINE_H