/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "oh_audio_suite_tool_test.h"
#include "OHAudioSuiteEngine.h"
#include "native_audio_suite_engine.h"
#include "audio_suite_base.h"

namespace OHOS {
namespace AudioStandard {

bool AllNodeTypesSupported()
{
    for (const auto& nodeTypeEntry : AudioSuite::NODETYPE_TOSTRING_MAP) {
        if (nodeTypeEntry.first == AudioSuite::NODE_TYPE_EMPTY) {
            continue;
        }
        OH_AudioNode_Type nodeType = static_cast<OH_AudioNode_Type>(nodeTypeEntry.first);
        bool isCurrentSupported = false;
        int32_t ret = OH_AudioSuiteEngine_IsNodeTypeSupported(nodeType, &isCurrentSupported);
        if (ret != AUDIOSUITE_SUCCESS || !isCurrentSupported) {
            return false;
        }
    }
    return true;
}

} // namespace AudioStandard
} // namespace OHOS