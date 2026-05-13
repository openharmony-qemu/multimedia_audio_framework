/*
 * Copyright (c) 2026-2026 Huawei Device Co., Ltd.
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
#define LOG_TAG "MixerProcessorManager"
#endif

#include "audio_errors.h"
#include "audio_suite_log.h"
#include "audio_suite_mixer_processor.h"
#include "audio_suite_mixer_processor_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

std::unique_ptr<MixerProcessor> MixerProcessorManager::Create(const AudioFormat &format, bool enableLimiter)
{
    AUDIO_INFO_LOG("Mixer processor create, enableLimiter: %{public}d", enableLimiter);
    std::unique_ptr<MixerProcessorNode> node = std::make_unique<MixerProcessorNode>();
    CHECK_AND_RETURN_RET_LOG(node != nullptr, nullptr, "create MixerProcessorNode failed");

    int32_t ret = node->Init(format, enableLimiter);
    CHECK_AND_RETURN_RET_LOG(ret == SUCCESS, nullptr, "MixerProcessorNode::Init failed, ret: %{public}d", ret);

    return node;
}

} // namespace AudioSuite
} // namespace AudioStandard
} // namespace OHOS