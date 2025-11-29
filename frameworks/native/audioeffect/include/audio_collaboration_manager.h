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

#include <cstdio>
#include <cstdint>
#include <cassert>
#include <cstdint>
#include <cstddef>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <mutex>
#include <set>

#include "audio_effect.h"
#include "audio_effect_chain.h"
#include "audio_info.h"
#include "audio_effect_chain_manager.h"

#ifdef SENSOR_ENABLE
#include "audio_head_tracker.h"
#endif

#ifdef WINDOW_MANAGER_ENABLE
#include "audio_effect_rotation.h"
#endif
#include "audio_effect_volume.h"

namespace OHOS {
namespace AudioStandard {

class AudioCollaborationManager {
public:
    AudioCollaborationManager();
    ~AudioCollaborationManager();
    static AudioEffectChainManager *GetInstance();
    void updateCollaborativeProductId(const std::string &productId);
    void LoadCollaborationConfig();

private:
    void updateLatencyInner();
    void LoadCollaborationConfigInner();
    std::mutex collaborationMutex_;
    int32_t latencyMs_ = 0;
    std::string productId_ = "default";
    AudioTwsMode twsMode_ = TWS_MODE_DEFAULT;
    AudioEarphoneProduct earphoneProduct_ = EARPHONE_PRODUCT_NONE;
    std::unordered_map<std::string, std::map<AudioTwsMode, int32_t>> collaborativeLatencyConfig_;
};

}  // namespace AudioStandard
}  // namespace OHOS