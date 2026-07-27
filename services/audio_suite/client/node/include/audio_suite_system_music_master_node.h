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

#ifndef AUDIO_SUITE_SYSTEM_MUSIC_MASTER_NODE_H
#define AUDIO_SUITE_SYSTEM_MUSIC_MASTER_NODE_H

#include "audio_suite_system_node.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

class AudioSuiteSystemMusicMasterNode : public AudioSuiteSystemNode {
public:
    AudioSuiteSystemMusicMasterNode()
        : AudioSuiteSystemNode(NODE_TYPE_SYSTEM_MUSIC_MASTER)
    {}

    ~AudioSuiteSystemMusicMasterNode() override = default;

protected:
    std::string GetEffectChainName() const override
    {
        return nodeName_;
    }

private:
    std::string nodeName_ = "music_master";
};

}
}
}
#endif
