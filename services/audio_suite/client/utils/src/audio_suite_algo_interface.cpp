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

#include "audio_suite_algo_interface.h"
#include "audio_suite_eq_algo_interface_impl.h"
#include "audio_suite_nr_algo_interface_impl.h"
#include "audio_suite_soundfield_algo_interface_impl.h"
#include "audio_suite_aiss_algo_interface_impl.h"
#include "audio_suite_voice_beautifier_algo_interface_impl.h"

namespace OHOS {
namespace AudioStandard {
namespace AudioSuite {

std::shared_ptr<AudioSuiteAlgoInterface> AudioSuiteAlgoInterface::CreateAlgoInterface(
    AlgoType algoType, NodeCapability &nc)
{
    switch (algoType) {
        case AlgoType::AUDIO_NODE_TYPE_EQUALIZER:
            return std::make_shared<AudioSuiteEqAlgoInterfaceImpl>(nc);
        case AlgoType::AUDIO_NODE_TYPE_NOISE_REDUCTION:
            return std::make_shared<AudioSuiteNrAlgoInterfaceImpl>(nc);
        case AlgoType::AUDIO_NODE_TYPE_SOUND_FIELD:
            return std::make_shared<AudioSuiteSoundFieldAlgoInterfaceImpl>(nc);
        case AlgoType::AUDIO_NODE_TYPE_AUDIO_SEPARATION:
            return std::make_shared<AudioSuiteAissAlgoInterfaceImpl>(nc);
        case AlgoType::AUDIO_NODE_TYPE_VOICE_BEAUTIFIER:
            return std::make_shared<AudioSuiteVoiceBeautifierAlgoInterfaceImpl>(nc);
        default:
            return nullptr;
    }
}

}  // namespace AudioSuite
}  // namespace AudioStandard
}  // namespace OHOS