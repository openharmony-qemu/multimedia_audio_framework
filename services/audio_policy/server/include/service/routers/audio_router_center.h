/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef ST_AUDIO_ROUTER_CENTER_H
#define ST_AUDIO_ROUTER_CENTER_H

#include "router_base.h"
#include "user_select_router.h"
#include "privacy_priority_router.h"
#include "public_priority_router.h"
#include "package_filter_router.h"
#include "stream_filter_router.h"
#include "cockpit_phone_router.h"
#include "pair_device_router.h"
#include "default_router.h"
#include "audio_stream_collector.h"
#include "audio_strategy_router_parser.h"
#include "audio_usage_strategy_parser.h"

namespace OHOS {
namespace AudioStandard {
class AudioRouterCenter {
public:
    static AudioRouterCenter& GetAudioRouterCenter()
    {
        static AudioRouterCenter audioRouterCenter;
        return audioRouterCenter;
    }
    std::vector<std::unique_ptr<AudioDeviceDescriptor>> FetchOutputDevices(StreamUsage streamUsage,
        int32_t clientUID, const RouterType &bypassType = RouterType::ROUTER_TYPE_NONE);
    std::unique_ptr<AudioDeviceDescriptor> FetchInputDevice(SourceType sourceType, int32_t clientUID);

    bool isCallRenderRouter(StreamUsage streamUsage);

    int32_t SetAudioDeviceRefinerCallback(const sptr<IRemoteObject> &object);

    int32_t UnsetAudioDeviceRefinerCallback();

private:
    AudioRouterCenter()
    {
        unique_ptr<AudioStrategyRouterParser> audioStrategyRouterParser = make_unique<AudioStrategyRouterParser>();
        if (audioStrategyRouterParser->LoadConfiguration()) {
            AUDIO_INFO_LOG("audioStrategyRouterParser load configuration successfully.");
            audioStrategyRouterParser->Parse();
            for (auto &mediaRounter : audioStrategyRouterParser->mediaRenderRouters_) {
                AUDIO_INFO_LOG("mediaRenderRouters_, class %{public}s", mediaRounter->GetClassName().c_str());
                mediaRenderRouters_.push_back(std::move(mediaRounter));
            }
            for (auto &callRenderRouter : audioStrategyRouterParser->callRenderRouters_) {
                AUDIO_INFO_LOG("callRenderRouters_, class %{public}s", callRenderRouter->GetClassName().c_str());
                callRenderRouters_.push_back(std::move(callRenderRouter));
            }
            for (auto &callCaptureRouter : audioStrategyRouterParser->callCaptureRouters_) {
                AUDIO_INFO_LOG("callCaptureRouters_, class %{public}s", callCaptureRouter->GetClassName().c_str());
                callCaptureRouters_.push_back(std::move(callCaptureRouter));
            }
            for (auto &ringRenderRouter : audioStrategyRouterParser->ringRenderRouters_) {
                AUDIO_INFO_LOG("ringRenderRouters_, class %{public}s", ringRenderRouter->GetClassName().c_str());
                ringRenderRouters_.push_back(std::move(ringRenderRouter));
            }
            for (auto &toneRenderRouter : audioStrategyRouterParser->toneRenderRouters_) {
                AUDIO_INFO_LOG("toneRenderRouters_, class %{public}s", toneRenderRouter->GetClassName().c_str());
                toneRenderRouters_.push_back(std::move(toneRenderRouter));
            }
            for (auto &recordCaptureRouter : audioStrategyRouterParser->recordCaptureRouters_) {
                AUDIO_INFO_LOG("recordCaptureRouters_, class %{public}s", recordCaptureRouter->GetClassName().c_str());
                recordCaptureRouters_.push_back(std::move(recordCaptureRouter));
            }
            for (auto &voiceMessageRouter : audioStrategyRouterParser->voiceMessageRouters_) {
                AUDIO_INFO_LOG("voiceMessageRouters_, class %{public}s", voiceMessageRouter->GetClassName().c_str());
                voiceMessageRouters_.push_back(std::move(voiceMessageRouter));
            }
        }

        unique_ptr<AudioUsageStrategyParser> audioUsageStrategyParser = make_unique<AudioUsageStrategyParser>();
        if (audioUsageStrategyParser->LoadConfiguration()) {
            AUDIO_INFO_LOG("AudioUsageStrategyParser load configuration successfully.");
            audioUsageStrategyParser->Parse();
            renderConfigMap_ = audioUsageStrategyParser->renderConfigMap_;
            capturerConfigMap_ = audioUsageStrategyParser->capturerConfigMap_;
            for (auto &renderConfig : renderConfigMap_) {
                AUDIO_INFO_LOG("streamusage:%{public}d, routername:%{public}s",
                    renderConfig.first, renderConfig.second.c_str());
            }
            for (auto &capturerConfig : capturerConfigMap_) {
                AUDIO_INFO_LOG("sourceType:%{public}d, sourceTypeName:%{public}s",
                    capturerConfig.first, capturerConfig.second.c_str());
            }
        }
    }

    ~AudioRouterCenter() {}

    unique_ptr<AudioDeviceDescriptor> FetchMediaRenderDevice(StreamUsage streamUsage, int32_t clientUID,
        RouterType &routerType, const RouterType &bypassType = RouterType::ROUTER_TYPE_NONE);
    unique_ptr<AudioDeviceDescriptor> FetchCallRenderDevice(StreamUsage streamUsage, int32_t clientUID,
        RouterType &routerType, const RouterType &bypassType = RouterType::ROUTER_TYPE_NONE);
    bool HasScoDevice();
    vector<unique_ptr<AudioDeviceDescriptor>> FetchRingRenderDevices(StreamUsage streamUsage, int32_t clientUID,
        RouterType &routerType);
    void DealRingRenderRouters(std::vector<std::unique_ptr<AudioDeviceDescriptor>> &descs,
        StreamUsage streamUsage, int32_t clientUID, RouterType &routerType);

    std::vector<std::unique_ptr<RouterBase>> mediaRenderRouters_;
    std::vector<std::unique_ptr<RouterBase>> callRenderRouters_;
    std::vector<std::unique_ptr<RouterBase>> callCaptureRouters_;
    std::vector<std::unique_ptr<RouterBase>> ringRenderRouters_;
    std::vector<std::unique_ptr<RouterBase>> toneRenderRouters_;
    std::vector<std::unique_ptr<RouterBase>> recordCaptureRouters_;
    std::vector<std::unique_ptr<RouterBase>> voiceMessageRouters_;

    unordered_map<StreamUsage, string> renderConfigMap_;
    unordered_map<SourceType, string> capturerConfigMap_;

    sptr<IStandardAudioRoutingManagerListener> audioDeviceRefinerCb_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_ROUTER_CENTER_H
