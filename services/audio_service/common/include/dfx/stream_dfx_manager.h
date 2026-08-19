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

#ifndef DFX_STREAM_DFX_MANAGER_H
#define DFX_STREAM_DFX_MANAGER_H

#include "audio_info.h"
#include "audio_errors.h"
#include <unordered_map>
#include <mutex>
#include <vector>

namespace OHOS {
namespace AudioStandard {
struct StreamRecord {
    AudioProcessConfig processConfig;
    int64_t startTime = 0;
    int64_t lastUploadTime = 0;
};

constexpr int32_t DFX_INVALID_STREAM_INFO = 0;
constexpr int32_t DFX_INVALID_APP_INFO = 0;
constexpr int32_t DFX_INVALID_EXCEPTION_INFO = 0;
constexpr int32_t DFX_INVALID_DEVICE_INFO = 0;
constexpr int32_t DFX_INVALID_SYSTEM_INFO = 0;

enum PlayOtherErrorCode {
    TYPE_BACKGROUND_PLAYBACK,
    TYPE_OFFLOAD_NOT_ENOUGH_DATA,
    TYPE_GET_TIMESTAMP_TOO_FREQUENTLY,
    TYPE_FAST_ABUSE,
    TYPE_FAST_PREFERRED_FRAME_SIZE_OUT_OF_RANGE,
};

class StreamDfxManager {
public:
    static StreamDfxManager& GetInstance();
    void CheckStreamOccupancy(uint32_t sessionId, const AudioProcessConfig &processConfig, bool isStart);
    void SendAudioErrorEvent(int32_t uid, int32_t errorCode, const std::string &erroDesc, bool isClient);
    static void GetErrorInfosFromMedia(std::vector<AudioFaultRecord> &errorInfos);

private:
    StreamDfxManager() = default;
    ~StreamDfxManager() = default;
    bool IsInterphoneStream(const AudioProcessConfig &processConfig);
    void ReportStreamOccupancyTimeout(uint32_t sessionId, int64_t startTime, int64_t currentTime);
    int32_t BuildAudioErrorCode(uint32_t errorCode, uint32_t interfaceCode);
    std::mutex streamMutex_;
    std::unordered_map<uint32_t, StreamRecord> streamRecordMap_{};
};
} // namespace AudioStandard
} // namespace OHOS
#endif //DFX_STREAM_DFX_MANAGER_H