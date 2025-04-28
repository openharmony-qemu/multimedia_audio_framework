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

#ifndef HPAE_CAPTURE_MOVE_INFO_H
#define HPAE_CAPTURE_MOVE_INFO_H
#include "hpae_define.h"
#include "hpae_source_output_node.h"

namespace OHOS {
namespace AudioStandard {
namespace HPAE {
struct HpaeCaptureMoveInfo {
    uint32_t sessionId;
    std::shared_ptr<HpaeSourceOutputNode> sourceOutputNode;
    HpaeCapturerSessionInfo sessionInfo;
};
}  // namespace HPAE
}  // namespace AudioStandard
}  // namespace OHOS
#endif // HPAE_CAPTURE_MOVE_INFO_H