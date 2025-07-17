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

#include "audio_global_config_manager.h"

#include "audio_policy_log.h"
#include "audio_policy_global_parser.h"

using namespace std;

namespace OHOS {
namespace AudioStandard {

static std::string MAX_INNER_CAPTURE_KEY_NAME = "MAX_INNER_CAPTURE";

void AudioGlobalConfigManager::ParseGlobalConfigXml()
{
    unique_ptr<AudioPolicyGlobalParser> globalConfigParser = make_unique<AudioPolicyGlobalParser>();
    if (globalConfigParser->LoadConfiguration()) {
        AUDIO_INFO_LOG("Audio global config manager load configuration successfully.");
        globalConfigParser->Parse();
    }
    globalConfigParser->GetConfigByKeyName(MAX_INNER_CAPTURE_KEY_NAME, innerCapLimit_);
}

} // namespace AudioStandard
} // namespace OHOS

