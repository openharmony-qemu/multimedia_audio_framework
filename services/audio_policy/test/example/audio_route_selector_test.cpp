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
#ifndef LOG_TAG
#define LOG_TAG "AudioRouteSelectorTest"
#endif

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <thread>
#include <unistd.h>

#include "audio_errors.h"
#include "audio_policy_interface.h"
#include "audio_routing_client_manager.h"

using namespace OHOS;
using namespace OHOS::AudioStandard;

namespace {
std::atomic<bool> g_keepRunning = true;
constexpr int32_t MIN_ARG_COUNT = 2;
constexpr int32_t MAX_ARG_COUNT = 3;
constexpr int32_t SELECT_RESULT_ARG_INDEX = 1;
constexpr int32_t CALLBACK_RET_ARG_INDEX = 2;
constexpr int32_t DECIMAL_BASE = 10;

void PrintUsage(const char *programName)
{
    std::cout << "Usage: " << programName << " <select_result> [callback_ret]" << std::endl;
    std::cout << "  select_result: 0=success 1=reject_create 2=fallback_normal" << std::endl;
    std::cout << "  callback_ret : callback return code, default 0" << std::endl;
    std::cout << "Run the tool as cockpit_sa/root, then trigger route selection from another client." << std::endl;
}

void HandleSignal(int signo)
{
    (void)signo;
    g_keepRunning = false;
}

class FixedAudioRouteSelector : public AudioRouteSelector {
public:
    FixedAudioRouteSelector(int32_t callbackRet, int32_t selectResult)
        : callbackRet_(callbackRet), selectResult_(selectResult) {}
    ~FixedAudioRouteSelector() override = default;

    int32_t OnAudioRouteSelect(const std::shared_ptr<AudioRouteSelectInfo> &routeSelectInfo,
        int32_t &selectResult) override
    {
        selectResult = selectResult_;
        if (routeSelectInfo != nullptr) {
            std::cout << "route callback:"
                << " mode=" << static_cast<int32_t>(routeSelectInfo->audioMode)
                << " sampleRate=" << routeSelectInfo->streamInfo.GetEffectiveSampleRate()
                << " channels=" << static_cast<int32_t>(routeSelectInfo->streamInfo.channels)
                << " format=" << static_cast<int32_t>(routeSelectInfo->streamInfo.format)
                << " usage=" << static_cast<int32_t>(routeSelectInfo->streamUsage)
                << " source=" << static_cast<int32_t>(routeSelectInfo->sourceType)
                << " routeFlag=" << static_cast<uint32_t>(routeSelectInfo->routeFlag)
                << " uid=" << routeSelectInfo->appInfo.appUid
                << " pid=" << routeSelectInfo->appInfo.appPid
                << " bundle=" << routeSelectInfo->bundleName
                << std::endl;
        } else {
            std::cout << "route callback: routeSelectInfo is nullptr" << std::endl;
        }
        std::cout << "route callback return ret=" << callbackRet_
            << " selectResult=" << selectResult_ << std::endl;
        return callbackRet_;
    }

private:
    int32_t callbackRet_ = SUCCESS;
    int32_t selectResult_ = ROUTE_SELECT_RESULT_SUCCESS;
};
} // namespace

int main(int argc, char *argv[])
{
    if (argc < MIN_ARG_COUNT || argc > MAX_ARG_COUNT) {
        PrintUsage(argv[0]);
        return ERR_INVALID_PARAM;
    }

    char *end = nullptr;
    int32_t selectResult = static_cast<int32_t>(std::strtol(argv[SELECT_RESULT_ARG_INDEX], &end, DECIMAL_BASE));
    if (end == nullptr || *end != '\0' ||
        selectResult < AudioRouteSelector::ROUTE_SELECT_RESULT_SUCCESS ||
        selectResult > AudioRouteSelector::ROUTE_SELECT_RESULT_FALLBACK_NORMAL) {
        PrintUsage(argv[0]);
        return ERR_INVALID_PARAM;
    }

    int32_t callbackRet = SUCCESS;
    if (argc == MAX_ARG_COUNT) {
        callbackRet = static_cast<int32_t>(std::strtol(argv[CALLBACK_RET_ARG_INDEX], &end, DECIMAL_BASE));
        if (end == nullptr || *end != '\0') {
            PrintUsage(argv[0]);
            return ERR_INVALID_PARAM;
        }
    }

    std::signal(SIGINT, HandleSignal);
    std::signal(SIGTERM, HandleSignal);

    auto routeSelector = std::make_shared<FixedAudioRouteSelector>(callbackRet, selectResult);
    int32_t ret = AudioRoutingClientManager::GetInstance().SetAudioRouteSelectorCallback(routeSelector);
    if (ret != SUCCESS) {
        std::cerr << "SetAudioRouteSelectorCallback failed, ret=" << ret << std::endl;
        return ret;
    }

    std::cout << "Audio route selector registered."
        << " callbackRet=" << callbackRet
        << " selectResult=" << selectResult
        << ". Press Ctrl+C to exit." << std::endl;

    while (g_keepRunning.load()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    ret = AudioRoutingClientManager::GetInstance().UnsetAudioRouteSelectorCallback();
    if (ret != SUCCESS) {
        std::cerr << "UnsetAudioRouteSelectorCallback failed, ret=" << ret << std::endl;
        return ret;
    }
    std::cout << "Audio route selector unregistered." << std::endl;
    return SUCCESS;
}
