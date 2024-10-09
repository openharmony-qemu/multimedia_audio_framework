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

#ifndef AUDIO_HEAD_TRACKER_H
#define AUDIO_HEAD_TRACKER_H

#include <cstdint>
#include <mutex>

#ifdef SENSOR_ENABLE
#include "sensor_agent.h"
#endif

namespace OHOS {
namespace AudioStandard {

#ifdef SENSOR_ENABLE
const uint32_t NONE_SPATIALIZER_ENGINE = 0;
const uint32_t ARM_SPATIALIZER_ENGINE = 1;
const uint32_t DSP_SPATIALIZER_ENGINE = 2;

class HeadTracker {
public:
    HeadTracker();
    ~HeadTracker();
    int32_t SensorInit();
    int32_t SensorSetConfig(int32_t spatializerEngineState);
    int32_t SensorActive();
    int32_t SensorDeactive();
    int32_t SensorUnsubscribe();
    HeadPostureData GetHeadPostureData();
    void SetHeadPostureData(HeadPostureData headPostureData);
private:
    static void HeadPostureDataProcCb(SensorEvent *event);
    static HeadPostureData headPostureData_;
    SensorUser sensorUser_ = {};
    int64_t sensorSamplingInterval_ = 30000000; // 30000000 ns = 30 ms
    static std::mutex headTrackerMutex_;
};
#endif
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_HEAD_TRACKER_H
